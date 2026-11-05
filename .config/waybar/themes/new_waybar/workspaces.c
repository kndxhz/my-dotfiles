/* Dynamic workspace buttons using Waybar's CFFI ABI v2. */
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <signal.h>
#include <stdlib.h>

#include "responsive-media.h"

const size_t wbcffi_version = 2;
typedef struct {
    void *obj;
    const char *waybar_version;
    GtkContainer *(*get_root_widget)(void *);
    void (*queue_update)(void *);
} InitInfo;
typedef struct { const char *key, *value; } ConfigEntry;
typedef struct {
    GtkWidget *box, *header, *revealer, *content;
    int first, last;
} WorkspaceGroup;
typedef struct {
    GtkWidget *box;
    GList *buttons;
    GPtrArray *groups;
    GIOChannel *stream;
    GPid pid;
    guint watch;
    guint media_install_source;
    gint64 last_scroll;
    gint64 last_event;
    guint cooldown_ms;
    double accumulated;
    int active_workspace;
} Workspaces;

static void focus(const char *target) {
    char *expression = g_strdup_printf("hl.dsp.focus({workspace = [[%s]]})", target);
    char *argv[] = {"hyprctl", "dispatch", expression, NULL};
    GError *error = NULL;
    if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL,
                       NULL, NULL, NULL, &error)) {
        g_warning("Workspace switch: %s", error->message);
        g_clear_error(&error);
    }
    g_free(expression);
}

static void clicked(GtkButton *button, gpointer unused) {
    (void)unused;
    focus(gtk_button_get_label(button));
}

static int adjacent_workspace(Workspaces *self, int direction) {
    int target = 0;
    gboolean current_occupied = FALSE;
    if (self->active_workspace <= 0) return 0;
    for (GList *it = self->buttons; it; it = it->next) {
        int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(it->data), "workspace-id"));
        gboolean occupied = gtk_style_context_has_class(
            gtk_widget_get_style_context(GTK_WIDGET(it->data)), "occupied");
        if (id == self->active_workspace) current_occupied = occupied;
        /* Fixed empty slots are display placeholders, not swipe destinations. */
        if (!occupied) continue;
        if (direction > 0 && id > self->active_workspace && (!target || id < target))
            target = id;
        if (direction < 0 && id < self->active_workspace && (!target || id > target))
            target = id;
    }
    /* Like native swipes: one empty workspace after the last occupied one. */
    if (!target && direction > 0 && current_occupied && self->active_workspace < G_MAXINT)
        target = self->active_workspace + 1;
    return target;
}

static gboolean scrolled(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    (void)widget;
    Workspaces *self = data;
    gint64 now = g_get_monotonic_time();
    if (now - self->last_scroll < (gint64)self->cooldown_ms * 1000)
        return TRUE;
    int direction = 0;
    if (event->direction == GDK_SCROLL_UP) direction = -1;
    else if (event->direction == GDK_SCROLL_DOWN) direction = 1;
    else if (event->direction == GDK_SCROLL_SMOOTH) {
        if (now - self->last_event > 1000000) self->accumulated = 0;
        self->last_event = now;
        self->accumulated += event->delta_y;
        if (self->accumulated >= 1.0) direction = 1;
        else if (self->accumulated <= -1.0) direction = -1;
    }
    if (direction) {
        self->last_scroll = now;
        self->accumulated = 0;
        int target = adjacent_workspace(self, direction);
        if (target) {
            char *name = g_strdup_printf("%d", target);
            focus(name);
            g_free(name);
        }
    }
    return TRUE;
}

static void set_class(GtkWidget *widget, const char *name, gboolean enabled) {
    GtkStyleContext *style = gtk_widget_get_style_context(widget);
    if (enabled) gtk_style_context_add_class(style, name);
    else gtk_style_context_remove_class(style, name);
}

static void group_toggled(GtkToggleButton *header, gpointer data) {
    WorkspaceGroup *group = data;
    gboolean expanded = gtk_toggle_button_get_active(header);
    gtk_revealer_set_reveal_child(GTK_REVEALER(group->revealer), expanded);
    char *label = group->first == group->last
        ? g_strdup_printf("%d %s", group->first, expanded ? "▾" : "▸")
        : g_strdup_printf("%d–%d %s", group->first, group->last, expanded ? "▾" : "▸");
    gtk_button_set_label(GTK_BUTTON(header), label);
    g_free(label);
}

static void free_group(gpointer data) {
    WorkspaceGroup *group = data;
    gtk_widget_destroy(group->box);
    g_free(group);
}

static void add_group(Workspaces *self) {
    WorkspaceGroup *group = g_new0(WorkspaceGroup, 1);
    group->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    group->header = gtk_toggle_button_new();
    group->revealer = gtk_revealer_new();
    group->content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    set_class(group->box, "workspace-group", TRUE);
    set_class(group->header, "workspace-group-header", TRUE);
    set_class(group->content, "workspace-group-content", TRUE);
    gtk_widget_set_can_focus(group->header, FALSE);
    gtk_revealer_set_transition_type(GTK_REVEALER(group->revealer),
                                     GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_revealer_set_transition_duration(GTK_REVEALER(group->revealer), 180);
    gtk_container_add(GTK_CONTAINER(group->revealer), group->content);
    gtk_box_pack_start(GTK_BOX(group->box), group->header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(group->box), group->revealer, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self->box), group->box, FALSE, FALSE, 0);
    g_signal_connect(group->header, "toggled", G_CALLBACK(group_toggled), group);
    gtk_widget_add_events(group->header, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);
    g_signal_connect(group->header, "scroll-event", G_CALLBACK(scrolled), self);
    g_ptr_array_add(self->groups, group);
    gtk_widget_show_all(group->box);
}

static void place_button(GtkWidget *button, GtkWidget *parent, guint position) {
    GtkWidget *old_parent = gtk_widget_get_parent(button);
    if (old_parent != parent) {
        g_object_ref(button);
        if (old_parent) gtk_container_remove(GTK_CONTAINER(old_parent), button);
        gtk_box_pack_start(GTK_BOX(parent), button, FALSE, FALSE, 0);
        g_object_unref(button);
    }
    gtk_box_reorder_child(GTK_BOX(parent), button, (gint)position);
}

static void render(Workspaces *self, const char *line) {
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, line, -1, NULL)) {
        g_object_unref(parser);
        return;
    }
    JsonObject *root = json_node_get_object(json_parser_get_root(parser));
    int active = json_object_get_int_member(root, "active");
    self->active_workspace = active;
    JsonArray *items = json_object_get_array_member(root, "workspaces");
    /* Chunk the existing displayed sequence; never filter or sort here. */
    guint count = json_array_get_length(items);
    guint workspace_count = json_object_has_member(root, "workspace-count")
        ? (guint)json_object_get_int_member(root, "workspace-count") : count;
    guint group_count = workspace_count >= 10 ? (count + 4) / 5 : 0;
    while (self->groups->len < group_count) add_group(self);
    set_class(self->box, "grouped", group_count > 0);
    for (GList *it = self->buttons; it; it = it->next)
        g_object_set_data(G_OBJECT(it->data), "keep", NULL);
    for (guint i = 0; i < json_array_get_length(items); i++) {
        JsonObject *item = json_array_get_object_element(items, i);
        int id = json_object_get_int_member(item, "id");
        int windows = json_object_get_int_member(item, "windows");
        GtkWidget *button = NULL;
        for (GList *it = self->buttons; it; it = it->next) {
            if (GPOINTER_TO_INT(g_object_get_data(G_OBJECT(it->data), "workspace-id")) == id)
                button = it->data;
        }
        if (!button) {
            char *label = g_strdup_printf("%d", id);
            button = gtk_button_new_with_label(label);
            g_free(label);
            gtk_widget_set_can_focus(button, FALSE);
            g_object_set_data(G_OBJECT(button), "workspace-id", GINT_TO_POINTER(id));
            g_signal_connect(button, "clicked", G_CALLBACK(clicked), NULL);
            gtk_widget_add_events(button, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);
            g_signal_connect(button, "scroll-event", G_CALLBACK(scrolled), self);
            gtk_box_pack_start(GTK_BOX(self->box), button, FALSE, FALSE, 0);
            self->buttons = g_list_append(self->buttons, button);
            gtk_widget_show(button);
        }
        GtkWidget *parent = group_count
            ? ((WorkspaceGroup *)g_ptr_array_index(self->groups, i / 5))->content
            : self->box;
        place_button(button, parent, group_count ? i % 5 : i);
        g_object_set_data(G_OBJECT(button), "keep", GINT_TO_POINTER(1));
        set_class(button, "active", id == active);
        set_class(button, "occupied", windows > 0);
        set_class(button, "empty", windows == 0);
        gboolean urgent = json_object_has_member(item, "urgent") &&
                          json_object_get_boolean_member(item, "urgent") && id != active;
        set_class(button, "urgent", urgent);
        char *tooltip = g_strdup_printf("工作区 %d · %d 个窗口%s", id, windows,
                                       urgent ? " · 有新提醒" : "");
        gtk_widget_set_tooltip_text(button, tooltip);
        g_free(tooltip);
    }
    for (GList *it = self->buttons, *next; it; it = next) {
        next = it->next;
        if (!g_object_get_data(G_OBJECT(it->data), "keep")) {
            gtk_widget_destroy(it->data);
            self->buttons = g_list_delete_link(self->buttons, it);
        }
    }
    /* Move surviving buttons before destroying surplus group containers. */
    g_ptr_array_set_size(self->groups, group_count);
    for (guint g = 0; g < group_count; g++) {
        WorkspaceGroup *group = g_ptr_array_index(self->groups, g);
        guint end = MIN((g + 1) * 5, count);
        group->first = json_object_get_int_member(json_array_get_object_element(items, g * 5), "id");
        group->last = json_object_get_int_member(json_array_get_object_element(items, end - 1), "id");
        gboolean active_group = FALSE, urgent_group = FALSE;
        GString *tooltip = g_string_new("点击展开/收起 · 工作区：");
        for (guint i = g * 5; i < end; i++) {
            JsonObject *item = json_array_get_object_element(items, i);
            int id = json_object_get_int_member(item, "id");
            active_group |= id == active;
            urgent_group |= id != active && json_object_has_member(item, "urgent") &&
                            json_object_get_boolean_member(item, "urgent");
            g_string_append_printf(tooltip, "%s%d", i == g * 5 ? "" : "、", id);
        }
        set_class(group->header, "active", active_group);
        set_class(group->header, "urgent", urgent_group);
        gtk_widget_set_tooltip_text(group->header, tooltip->str);
        g_string_free(tooltip, TRUE);
        group_toggled(GTK_TOGGLE_BUTTON(group->header), group);
        gtk_box_reorder_child(GTK_BOX(self->box), group->box, (gint)g);
    }
    g_object_unref(parser);
}

static gboolean receive(GIOChannel *channel, GIOCondition condition, gpointer data) {
    Workspaces *self = data;
    char *line = NULL;
    GIOStatus status;
    while ((status = g_io_channel_read_line(channel, &line, NULL, NULL, NULL)) == G_IO_STATUS_NORMAL) {
        render(self, line);
        g_free(line);
        line = NULL;
    }
    g_free(line);
    if (status == G_IO_STATUS_EOF || status == G_IO_STATUS_ERROR || (condition & (G_IO_HUP | G_IO_ERR))) {
        self->watch = 0;
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

static void child_exited(GPid pid, gint status, gpointer unused) {
    (void)status; (void)unused;
    g_spawn_close_pid(pid);
}

static gboolean install_media_layout(gpointer data) {
    Workspaces *self = data;
    if (!media_install(self->box))
        g_warning("Responsive media: could not find lyrics and window modules");
    self->media_install_source = 0;
    return G_SOURCE_REMOVE;
}

void *wbcffi_init(const InitInfo *info, const ConfigEntry *entries, size_t count) {
    Workspaces *self = g_new0(Workspaces, 1);
    self->cooldown_ms = 300;
    for (size_t i = 0; i < count; i++)
        if (g_str_equal(entries[i].key, "scroll-cooldown-ms"))
            self->cooldown_ms = CLAMP(atoi(entries[i].value), 100, 2000);
    char *script = g_build_filename(g_get_home_dir(), ".config/waybar/themes/new_waybar/workspace-status.py", NULL);
    char *argv[] = {"python3", "-u", script, NULL};
    gint output;
    GError *error = NULL;
    if (!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                                  NULL, NULL, &self->pid, NULL, &output, NULL, &error)) {
        g_warning("Workspace status: %s", error->message);
        g_clear_error(&error);
        g_free(script); g_free(self);
        return NULL;
    }
    g_free(script);
    g_child_watch_add(self->pid, child_exited, NULL);
    self->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->groups = g_ptr_array_new_with_free_func(free_group);
    gtk_widget_set_name(self->box, "workspaces");
    gtk_container_add(info->get_root_widget(info->obj), self->box);
    gtk_widget_show(self->box);
    self->stream = g_io_channel_unix_new(output);
    g_io_channel_set_close_on_unref(self->stream, TRUE);
    g_io_channel_set_flags(self->stream, G_IO_FLAG_NONBLOCK, NULL);
    self->watch = g_io_add_watch(self->stream, G_IO_IN | G_IO_HUP | G_IO_ERR, receive, self);
    self->media_install_source = g_idle_add(install_media_layout, self);
    return self;
}

void wbcffi_deinit(void *instance) {
    Workspaces *self = instance;
    if (self->watch) g_source_remove(self->watch);
    if (self->media_install_source) g_source_remove(self->media_install_source);
    kill(self->pid, SIGTERM);
    g_io_channel_unref(self->stream);
    g_list_free(self->buttons);
    g_ptr_array_unref(self->groups);
    gtk_widget_destroy(self->box);
    g_free(self);
}

void wbcffi_update(void *instance) { (void)instance; }
void wbcffi_refresh(void *instance, int signal_id) { (void)instance; (void)signal_id; }
void wbcffi_doaction(void *instance, const char *action) { (void)instance; (void)action; }

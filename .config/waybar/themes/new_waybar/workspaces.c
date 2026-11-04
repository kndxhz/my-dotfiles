/* Dynamic workspace buttons using Waybar's CFFI ABI v2. */
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <signal.h>
#include <stdlib.h>

const size_t wbcffi_version = 2;
typedef struct {
    void *obj;
    const char *waybar_version;
    GtkContainer *(*get_root_widget)(void *);
    void (*queue_update)(void *);
} InitInfo;
typedef struct { const char *key, *value; } ConfigEntry;
typedef struct {
    GtkWidget *box;
    GIOChannel *stream;
    GPid pid;
    guint watch;
    gint64 last_scroll;
    gint64 last_event;
    guint cooldown_ms;
    double accumulated;
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
        focus(direction > 0 ? "e+1" : "e-1");
    }
    return TRUE;
}

static void set_class(GtkWidget *widget, const char *name, gboolean enabled) {
    GtkStyleContext *style = gtk_widget_get_style_context(widget);
    if (enabled) gtk_style_context_add_class(style, name);
    else gtk_style_context_remove_class(style, name);
}

static void render(Workspaces *self, const char *line) {
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, line, -1, NULL)) {
        g_object_unref(parser);
        return;
    }
    JsonObject *root = json_node_get_object(json_parser_get_root(parser));
    int active = json_object_get_int_member(root, "active");
    JsonArray *items = json_object_get_array_member(root, "workspaces");
    GList *children = gtk_container_get_children(GTK_CONTAINER(self->box));
    for (GList *it = children; it; it = it->next)
        g_object_set_data(G_OBJECT(it->data), "keep", NULL);
    for (guint i = 0; i < json_array_get_length(items); i++) {
        JsonObject *item = json_array_get_object_element(items, i);
        int id = json_object_get_int_member(item, "id");
        int windows = json_object_get_int_member(item, "windows");
        GtkWidget *button = NULL;
        for (GList *it = children; it; it = it->next) {
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
            gtk_widget_show(button);
        }
        gtk_box_reorder_child(GTK_BOX(self->box), button, (gint)i);
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
    for (GList *it = children; it; it = it->next)
        if (!g_object_get_data(G_OBJECT(it->data), "keep")) gtk_widget_destroy(it->data);
    g_list_free(children);
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
    gtk_widget_set_name(self->box, "workspaces");
    gtk_container_add(info->get_root_widget(info->obj), self->box);
    gtk_widget_show(self->box);
    self->stream = g_io_channel_unix_new(output);
    g_io_channel_set_close_on_unref(self->stream, TRUE);
    g_io_channel_set_flags(self->stream, G_IO_FLAG_NONBLOCK, NULL);
    self->watch = g_io_add_watch(self->stream, G_IO_IN | G_IO_HUP | G_IO_ERR, receive, self);
    return self;
}

void wbcffi_deinit(void *instance) {
    Workspaces *self = instance;
    if (self->watch) g_source_remove(self->watch);
    kill(self->pid, SIGTERM);
    g_io_channel_unref(self->stream);
    gtk_widget_destroy(self->box);
    g_free(self);
}

void wbcffi_update(void *instance) { (void)instance; }
void wbcffi_refresh(void *instance, int signal_id) { (void)instance; (void)signal_id; }
void wbcffi_doaction(void *instance, const char *action) { (void)instance; (void)action; }

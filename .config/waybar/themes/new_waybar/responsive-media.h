/* Share the remaining bar width: shrink the title first, then hide lyrics.
 * Child visibility is independent of module visibility, so fresh lyrics
 * cannot accidentally show a module that currently does not fit. */
typedef struct {
    GtkBox parent;
    GtkWidget *lyrics, *title;
} ResponsiveMedia;
typedef struct { GtkBoxClass parent; } ResponsiveMediaClass;
G_DEFINE_TYPE(ResponsiveMedia, responsive_media, GTK_TYPE_BOX)

static void media_width(GtkWidget *child, gint *minimum, gint *natural) {
    *minimum = *natural = 0;
    if (child && gtk_widget_get_visible(child))
        gtk_widget_get_preferred_width(child, minimum, natural);
}

static void responsive_media_width(GtkWidget *widget, gint *minimum, gint *natural) {
    ResponsiveMedia *self = (ResponsiveMedia *)widget;
    gint lm, ln, tm, tn;
    media_width(self->lyrics, &lm, &ln);
    media_width(self->title, &tm, &tn);
    /* Lyrics must never increase the bar's minimum width. */
    *minimum = tm;
    *natural = MAX(tm, ln + tn);
}

static void responsive_media_height(GtkWidget *widget, gint *minimum, gint *natural) {
    ResponsiveMedia *self = (ResponsiveMedia *)widget;
    gint lm = 0, ln = 0, tm = 0, tn = 0;
    if (self->lyrics && gtk_widget_get_visible(self->lyrics))
        gtk_widget_get_preferred_height(self->lyrics, &lm, &ln);
    if (self->title && gtk_widget_get_visible(self->title))
        gtk_widget_get_preferred_height(self->title, &tm, &tn);
    *minimum = MAX(lm, tm);
    *natural = MAX(ln, tn);
}

static void responsive_media_width_for_height(GtkWidget *widget, gint height,
                                              gint *minimum, gint *natural) {
    (void)height;
    responsive_media_width(widget, minimum, natural);
}

static void responsive_media_height_for_width(GtkWidget *widget, gint width,
                                              gint *minimum, gint *natural) {
    (void)width;
    responsive_media_height(widget, minimum, natural);
}

static void responsive_media_allocate(GtkWidget *widget, GtkAllocation *allocation) {
    ResponsiveMedia *self = (ResponsiveMedia *)widget;
    gint lm, ln, tm, tn;
    media_width(self->lyrics, &lm, &ln);
    media_width(self->title, &tm, &tn);
    GTK_WIDGET_CLASS(responsive_media_parent_class)->size_allocate(widget, allocation);
    /* The expanded middle region runs from workspaces to the tray.
     * Keep the title at its midpoint even as lyrics shorten the title. */
    gint midpoint = allocation->width / 2;
    gint right_room = allocation->width - midpoint;
    gint title_floor = MAX(tm, MIN(tn, 80));
    gint with_lyrics = MAX(0, 2 * MIN(midpoint - ln, right_room));
    gboolean show_lyrics = ln > 0 && with_lyrics >= title_floor;
    gint lyrics_width = show_lyrics ? ln : 0;
    gint title_width = MIN(tn, MAX(0, 2 * MIN(midpoint - lyrics_width, right_room)));
    gboolean show_title = self->title && gtk_widget_get_visible(self->title) &&
                          title_width >= tm && title_width > 0;
    /* Lyrics occupy the blank region before the title, not the bar midpoint.
     * With no visible title, the entire available region is blank. */
    gint blank_width = show_title ? midpoint - title_width / 2 : allocation->width;
    if (!show_title) {
        show_lyrics = ln > 0 && ln <= blank_width;
        lyrics_width = show_lyrics ? ln : 0;
    }
    if (self->lyrics) {
        gtk_widget_set_child_visible(self->lyrics, show_lyrics);
        if (show_lyrics) {
            GtkAllocation part = *allocation;
            part.x += MAX(0, (blank_width - lyrics_width) / 2);
            part.width = lyrics_width;
            gtk_widget_size_allocate(self->lyrics, &part);
        }
    }
    if (self->title && gtk_widget_get_visible(self->title)) {
        gtk_widget_set_child_visible(self->title, title_width >= tm && title_width > 0);
        if (title_width >= tm && title_width > 0) {
            GtkAllocation part = *allocation;
            part.x += midpoint - title_width / 2;
            part.width = title_width;
            gtk_widget_size_allocate(self->title, &part);
        }
    }
    gtk_widget_set_clip(widget, allocation);
}

static void responsive_media_class_init(ResponsiveMediaClass *klass) {
    GtkWidgetClass *widget = GTK_WIDGET_CLASS(klass);
    widget->get_preferred_width = responsive_media_width;
    widget->get_preferred_height = responsive_media_height;
    widget->get_preferred_width_for_height = responsive_media_width_for_height;
    widget->get_preferred_height_for_width = responsive_media_height_for_width;
    widget->size_allocate = responsive_media_allocate;
}

static void responsive_media_init(ResponsiveMedia *self) {
    gtk_orientable_set_orientation(GTK_ORIENTABLE(self), GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_name(GTK_WIDGET(self), "responsive-media");
}

static GtkWidget *media_find(GtkWidget *widget, const char *name) {
    if (g_str_equal(gtk_widget_get_name(widget), name)) return widget;
    if (!GTK_IS_CONTAINER(widget)) return NULL;
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
    GtkWidget *found = NULL;
    for (GList *it = children; it && !found; it = it->next)
        found = media_find(it->data, name);
    g_list_free(children);
    return found;
}

static GtkWidget *media_module(GtkWidget *widget, GtkWidget *parent) {
    while (widget && gtk_widget_get_parent(widget) != parent)
        widget = gtk_widget_get_parent(widget);
    return widget;
}

static void media_move(GtkWidget *widget, GtkWidget *destination) {
    g_object_ref(widget);
    gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widget)), widget);
    gtk_box_pack_start(GTK_BOX(destination), widget, FALSE, TRUE, 0);
    g_object_unref(widget);
}

static void media_center_labels(GtkWidget *widget) {
    if (GTK_IS_LABEL(widget)) gtk_label_set_xalign(GTK_LABEL(widget), 0.5f);
    if (!GTK_IS_CONTAINER(widget)) return;
    GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
    for (GList *it = children; it; it = it->next) media_center_labels(it->data);
    g_list_free(children);
}

static gboolean media_install(GtkWidget *anchor) {
    GtkWidget *top = gtk_widget_get_toplevel(anchor);
    GtkWidget *lyrics = media_find(top, "custom-lyrics");
    GtkWidget *title = media_find(top, "window");
    if (!lyrics || !title) return FALSE;
    /* Both modules are configured in modules-center. Find their shared box. */
    GtkWidget *center = gtk_widget_get_parent(title);
    while (center && !gtk_widget_is_ancestor(lyrics, center))
        center = gtk_widget_get_parent(center);
    if (!GTK_IS_BOX(center)) return FALSE;
    GtkWidget *lyrics_module = media_module(lyrics, center);
    GtkWidget *title_module = media_module(title, center);
    if (!lyrics_module || !title_module || lyrics_module == title_module) return FALSE;
    ResponsiveMedia *media = g_object_new(responsive_media_get_type(), NULL);
    media->lyrics = lyrics_module;
    media->title = title_module;
    media_center_labels(title_module);
    media_center_labels(lyrics_module);
    /* Restore the full lyric width; allocation decides whether it fits. */
    if (GTK_IS_LABEL(lyrics)) {
        gtk_label_set_ellipsize(GTK_LABEL(lyrics), PANGO_ELLIPSIZE_NONE);
        gtk_label_set_max_width_chars(GTK_LABEL(lyrics), -1);
        gtk_label_set_single_line_mode(GTK_LABEL(lyrics), TRUE);
    }
    media_move(lyrics_module, GTK_WIDGET(media));
    media_move(title_module, GTK_WIDGET(media));
    gtk_box_pack_start(GTK_BOX(center), GTK_WIDGET(media), TRUE, TRUE, 0);
    gtk_box_reorder_child(GTK_BOX(center), GTK_WIDGET(media), 0);
    gtk_widget_show(GTK_WIDGET(media));
    g_printerr("Responsive media: title centered between workspaces and tray; title shrinks before lyrics hide\n");
    return TRUE;
}

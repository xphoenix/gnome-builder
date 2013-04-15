/* gb-source-pane.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gd/gd-revealer.h>
#include <glib/gi18n.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcegutter.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourcestylescheme.h>
#include <nautilus/nautilus-floating-bar.h>

#include "gb-search-provider.h"
#include "gb-source-gutter-renderer-compiler.h"
#include "gb-source-gutter-renderer-diff.h"
#include "gb-source-overlay.h"
#include "gb-source-pane.h"
#include "gb-source-view.h"

static void gb_search_provider_init (GbSearchProviderIface *iface);

G_DEFINE_TYPE_EXTENDED(GbSourcePane,
                       gb_source_pane,
                       GB_TYPE_WORKSPACE_PANE,
                       0,
                       G_IMPLEMENT_INTERFACE(GB_TYPE_SEARCH_PROVIDER,
                                             gb_search_provider_init))

struct _GbSourcePanePrivate
{
   GFile *file;

   GtkWidget *highlight;
   GtkWidget *overlay;
   GtkWidget *ruler;
   GtkWidget *scroller;
   GtkWidget *search_entry;
   GtkWidget *search_revealer;
   GtkWidget *view;

   gint insert_text_handler;
   gint delete_range_handler;
   gint mark_set_handler;
};

enum
{
   PROP_0,
   PROP_FILE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_source_pane_guess_language (GbSourcePane *pane,
                               GFile        *file)
{
   GtkSourceLanguageManager *lm;
   GbSourcePanePrivate *priv;
   GtkSourceLanguage *l;
   GtkTextBuffer *buffer;
   const gchar *content_type = NULL;
   GFileInfo *info;
   gchar *base;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(G_IS_FILE(file));

   priv = pane->priv;

   info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                            G_FILE_QUERY_INFO_NONE, NULL, NULL);
   if (info) {
      content_type = g_file_info_get_content_type(info);
   }

   base = g_file_get_basename(file);
   lm = gtk_source_language_manager_get_default();
   l = gtk_source_language_manager_guess_language(lm, base, content_type);

   if (l) {
      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view));
      gtk_source_buffer_set_language(GTK_SOURCE_BUFFER(buffer), l);
   }

   g_clear_object(&info);
   g_free(base);
}

static void
gb_source_pane_load_scheme (GbSourcePane *pane)
{
   static const gchar *schemes[] = {
      "solarized-light",
      "solarized-dark",
      "tango",
      NULL
   };

   GtkSourceStyleSchemeManager *sm;
   GtkSourceStyleScheme *s;
   GbSourcePanePrivate *priv;
   GtkTextBuffer *buffer;
   gint i;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));

   priv = pane->priv;

   sm = gtk_source_style_scheme_manager_get_default();
   for (i = 0; schemes[i]; i++) {
      s = gtk_source_style_scheme_manager_get_scheme(sm, schemes[i]);
      if (s) {
         buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view));
         gtk_source_buffer_set_style_scheme(GTK_SOURCE_BUFFER(buffer), s);
         break;
      }
   }
}

GFile *
gb_source_pane_get_file (GbSourcePane *pane)
{
   g_return_val_if_fail(GB_IS_SOURCE_PANE(pane), NULL);
   return pane->priv->file;
}

void
gb_source_pane_set_file (GbSourcePane *pane,
                         GFile        *file)
{
   GbSourcePanePrivate *priv;
   gchar *title;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(G_IS_FILE(file));

   priv = pane->priv;

   g_clear_object(&priv->file);
   priv->file = g_object_ref(file);

   title = g_file_get_basename(file);
   g_object_set(pane, "title", title, NULL);
   g_free(title);

   gb_source_pane_guess_language(pane, file);

   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_FILE]);
}

void
gb_source_pane_load_async (GbSourcePane        *pane,
                           GFile               *file,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data)
{
   GbSourcePanePrivate *priv;
   GtkTextBuffer *buffer;
   GError *error = NULL;
   gchar *contents;
   gchar *path;
   gsize length;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(G_IS_FILE(file));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));

   priv = pane->priv;

   /*
    * TODO: Make this async.
    *       Use GInputStream.
    *       Warn on files larger than 1 Mb.
    */

   if (!(path = g_file_get_path(file))) {
      g_warning("Only local files are currently supported.");
      return;
   }

   if (!g_file_get_contents(path, &contents, &length, &error)) {
      g_warning("%s: %s", path, error->message);
      g_error_free(error);
      g_free(path);
      return;
   }

   g_free(path);

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view));
   gtk_source_buffer_begin_not_undoable_action(GTK_SOURCE_BUFFER(buffer));
   gtk_text_buffer_set_text(buffer, contents, length);
   gtk_source_buffer_end_not_undoable_action(GTK_SOURCE_BUFFER(buffer));
   g_free(contents);

   gb_source_pane_set_file(pane, file);

   gtk_text_buffer_set_modified(buffer, FALSE);

   /* TODO: Remove after making async. */
   if (callback) callback(G_OBJECT(pane), NULL, user_data);
}

gboolean
gb_source_pane_load_finish (GbSourcePane  *pane,
                            GAsyncResult  *result,
                            GError       **error)
{
   return TRUE;
}

static void
gb_source_pane_emit_modified_changed (GbSourcePane *pane)
{
   GtkTextBuffer *buffer;
   gboolean modified;

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pane->priv->view));
   modified = gtk_text_buffer_get_modified(buffer);
   gb_workspace_pane_set_modified(GB_WORKSPACE_PANE(pane), modified);
}

static void
gb_source_pane_grab_focus (GtkWidget *widget)
{
   gtk_widget_grab_focus(GB_SOURCE_PANE(widget)->priv->view);
}

static gboolean
get_child_position (GtkOverlay   *overlay,
                    GtkWidget    *widget,
                    GdkRectangle *allocation,
                    GbSourcePane *pane)
{
   return FALSE;
}

static void
update_position (GbSourcePane  *pane,
                 GtkTextBuffer *buffer)
{
   GtkTextMark *mark;
   GtkTextIter iter;
   gchar *text;
   guint line;
   guint column;

   mark = gtk_text_buffer_get_insert(buffer);
   gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
   line = gtk_text_iter_get_line(&iter) + 1;
   column = gtk_text_iter_get_line_offset(&iter) + 1;
   text = g_strdup_printf(_("Line: %u  Column: %u"), line, column);
   g_object_set(pane->priv->ruler, "label", text, NULL);
   g_free(text);

   if (!gtk_widget_get_visible(pane->priv->ruler)) {
      gtk_widget_show(pane->priv->ruler);
   }
}

static void
on_mark_set (GtkTextBuffer *buffer,
             GtkTextIter   *location,
             GtkTextMark   *mark,
             GbSourcePane  *pane)
{
   if (mark == gtk_text_buffer_get_insert(buffer)) {
      update_position(pane, buffer);
   }
}

static void
on_insert_text (GtkTextBuffer *buffer,
                GtkTextIter   *location,
                gchar         *text,
                gint           length,
                GbSourcePane  *pane)
{
   update_position(pane, buffer);
}

static void
on_delete_range (GtkTextBuffer *buffer,
                 GtkTextIter   *begin,
                 GtkTextIter   *end,
                 GbSourcePane  *pane)
{
   update_position(pane, buffer);
}

static void
gb_source_pane_focus_search (GbSearchProvider *provider)
{
   GbSourcePane *pane = (GbSourcePane *)provider;

   g_assert(GB_IS_SOURCE_PANE(pane));

   g_object_set(pane->priv->search_revealer,
                "reveal-child", TRUE,
                NULL);
   gtk_widget_grab_focus(pane->priv->search_entry);
}

static void
gb_source_pane_dispose (GObject *object)
{
   GbSourcePanePrivate *priv = GB_SOURCE_PANE(object)->priv;
   GtkTextBuffer *buffer;

   if ((buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view)))) {
      if (priv->insert_text_handler) {
         g_signal_handler_disconnect(buffer, priv->insert_text_handler);
         priv->insert_text_handler = 0;
      }
      if (priv->delete_range_handler) {
         g_signal_handler_disconnect(buffer, priv->delete_range_handler);
         priv->delete_range_handler = 0;
      }
   }

   G_OBJECT_CLASS(gb_source_pane_parent_class)->dispose(object);
}

static gboolean
gb_source_pane_view_key_press (GtkTextView  *text_view,
                               GdkEventKey  *key,
                               GbSourcePane *pane)
{
   GbSourcePanePrivate *priv = pane->priv;

   if (key->keyval == GDK_KEY_Escape) {
      g_object_set(priv->highlight,
                   "visible", FALSE,
                   NULL);
      g_object_set(text_view,
                   "search-text", NULL,
                   NULL);
      gtk_widget_grab_focus(priv->view);
      return TRUE;
   }

   return FALSE;
}

static gboolean
gb_source_pane_search_entry_key_press (GtkSearchEntry *search_entry,
                                       GdkEventKey    *key,
                                       GbSourcePane   *pane)
{
   GbSourcePanePrivate *priv = pane->priv;

   if (key->keyval == GDK_KEY_Escape) {
      g_object_set(priv->search_revealer,
                   "reveal-child", FALSE,
                   NULL);
      g_object_set(priv->highlight,
                   "visible", FALSE,
                   NULL);
      gtk_widget_grab_focus(priv->view);
      return TRUE;
   }

   return FALSE;
}

static void
gb_source_pane_finalize (GObject *object)
{
   GbSourcePanePrivate *priv = GB_SOURCE_PANE(object)->priv;

   g_clear_object(&priv->file);

   G_OBJECT_CLASS(gb_source_pane_parent_class)->finalize(object);
}

static void
gb_source_pane_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   GbSourcePane *pane = GB_SOURCE_PANE(object);

   switch (prop_id) {
   case PROP_FILE:
      g_value_set_object(value, gb_source_pane_get_file(pane));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_pane_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   GbSourcePane *pane = GB_SOURCE_PANE(object);

   switch (prop_id) {
   case PROP_FILE:
      gb_source_pane_set_file(pane, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_pane_class_init (GbSourcePaneClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_source_pane_dispose;
   object_class->finalize = gb_source_pane_finalize;
   object_class->get_property = gb_source_pane_get_property;
   object_class->set_property = gb_source_pane_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourcePanePrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->grab_focus = gb_source_pane_grab_focus;

   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("The file to load from source."),
                          G_TYPE_FILE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);
}

static void
gb_source_pane_init (GbSourcePane *pane)
{
   GtkSourceGutterRenderer *renderer;
   GbSourcePanePrivate *priv;
   GtkSourceBuffer *buffer;
   GtkSourceGutter *gutter;
   GtkWidget *button;
   GtkWidget *frame_;
   GtkWidget *hbox;
   GtkWidget *image;

   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_SOURCE_PANE,
                                            GbSourcePanePrivate);

   priv = pane->priv;

   g_object_set(pane,
                "icon-name", GTK_STOCK_FILE,
                "title", _("Unnamed File"),
                NULL);

   priv->overlay = g_object_new(GTK_TYPE_OVERLAY,
                                "visible", TRUE,
                                NULL);
   g_signal_connect(priv->overlay, "get-child-position",
                    G_CALLBACK(get_child_position),
                    pane);
   gtk_container_add(GTK_CONTAINER(pane), priv->overlay);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "hexpand", TRUE,
                                 "vexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->overlay), priv->scroller);

   buffer = gtk_source_buffer_new(NULL);
   g_signal_connect_swapped(buffer,
                            "modified-changed",
                            G_CALLBACK(gb_source_pane_emit_modified_changed),
                            pane);
   priv->view = g_object_new(GB_TYPE_SOURCE_VIEW,
                             "buffer", buffer,
                             "visible", TRUE,
                             NULL);
   g_signal_connect(priv->view, "key-press-event",
                    G_CALLBACK(gb_source_pane_view_key_press),
                    pane);
   gtk_container_add(GTK_CONTAINER(priv->scroller), priv->view);
   g_object_unref(buffer);

   priv->highlight = g_object_new(GB_TYPE_SOURCE_OVERLAY,
                                  "hexpand", TRUE,
                                  "tag", "Tag::Search",
                                  "vexpand", TRUE,
                                  "visible", TRUE,
                                  "widget", priv->view,
                                  NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->highlight);

   priv->ruler = g_object_new(NAUTILUS_TYPE_FLOATING_BAR,
                              "label", _("Line: 1  Column: 1"),
                              "halign", GTK_ALIGN_END,
                              "valign", GTK_ALIGN_END,
                              "visible", TRUE,
                              NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->ruler);

   if (gtk_widget_get_direction(priv->view) == GTK_TEXT_DIR_RTL) {
      gtk_widget_set_halign(priv->ruler, GTK_ALIGN_START);
   }

   priv->search_revealer = g_object_new(GD_TYPE_REVEALER,
                                        "halign", GTK_ALIGN_END,
                                        "margin-right", 32,
                                        "reveal-child", FALSE,
                                        "valign", GTK_ALIGN_START,
                                        "vexpand", FALSE,
                                        "visible", TRUE,
                                        NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->search_revealer);

   frame_ = g_object_new(GTK_TYPE_FRAME,
                         "visible", TRUE,
                         NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(frame_),
                               "gedit-search-slider");
   gtk_container_add(GTK_CONTAINER(priv->search_revealer), frame_);

   /* HACK: Belongs in theme. */
   GdkRGBA rgba;
   gdk_rgba_parse(&rgba, "#eeeeec");
   gtk_widget_override_background_color(frame_, GTK_STATE_FLAG_NORMAL, &rgba);

   hbox = g_object_new(GTK_TYPE_BOX,
                       "border-width", 6,
                       "orientation", GTK_ORIENTATION_HORIZONTAL,
                       "vexpand", FALSE,
                       "visible", TRUE,
                       NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(hbox),
                               GTK_STYLE_CLASS_LINKED);
   gtk_container_add(GTK_CONTAINER(frame_), hbox);

   priv->search_entry = g_object_new(GTK_TYPE_SEARCH_ENTRY,
                                     "halign", GTK_ALIGN_END,
                                     "placeholder-text", _("Search"),
                                     "valign", GTK_ALIGN_START,
                                     "visible", TRUE,
                                     "width-chars", 25,
                                     NULL);
   g_signal_connect(priv->search_entry, "key-press-event",
                    G_CALLBACK(gb_source_pane_search_entry_key_press),
                    pane);
   gtk_container_add(GTK_CONTAINER(hbox), priv->search_entry);

   if (gtk_widget_get_direction(priv->search_entry) == GTK_TEXT_DIR_RTL) {
      gtk_widget_set_margin_left(priv->search_revealer, 32);
      gtk_widget_set_margin_right(priv->search_revealer, 0);
   }

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "icon-name", "go-up-symbolic",
                        "visible", TRUE,
                        NULL);
   button = g_object_new(GTK_TYPE_BUTTON,
                         "can-focus", FALSE,
                         "image", image,
                         "vexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   gtk_container_add(GTK_CONTAINER(hbox), button);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "icon-name", "go-down-symbolic",
                        "visible", TRUE,
                        NULL);
   button = g_object_new(GTK_TYPE_BUTTON,
                         "can-focus", FALSE,
                         "image", image,
                         "vexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   gtk_container_add(GTK_CONTAINER(hbox), button);

   gutter = gtk_source_view_get_gutter(GTK_SOURCE_VIEW(priv->view),
                                       GTK_TEXT_WINDOW_LEFT);

   renderer = g_object_new(GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER, NULL);
   gtk_source_gutter_renderer_set_padding(renderer, 2, 1);
   gtk_source_gutter_renderer_set_size(renderer, 14);
   gtk_source_gutter_insert(gutter, renderer, -30);

   renderer = g_object_new(GB_TYPE_SOURCE_GUTTER_RENDERER_DIFF, NULL);
   gtk_source_gutter_renderer_set_size(renderer, 1);
   gtk_source_gutter_renderer_set_padding(renderer, 2, 0);
   gtk_source_gutter_insert(gutter, renderer, -10);

   gb_source_pane_load_scheme(pane);

   pane->priv->insert_text_handler =
      g_signal_connect_after(buffer, "insert-text",
                             G_CALLBACK(on_insert_text), pane);

   pane->priv->delete_range_handler =
      g_signal_connect_after(buffer, "delete-range",
                             G_CALLBACK(on_delete_range), pane);

   pane->priv->mark_set_handler =
      g_signal_connect_after(buffer, "mark-set", G_CALLBACK(on_mark_set),
                             pane);

   g_object_bind_property(pane->priv->search_entry, "text",
                          pane->priv->view, "search-text",
                          G_BINDING_SYNC_CREATE);

   g_object_bind_property(pane->priv->view, "search-has-matches",
                          pane->priv->highlight, "visible",
                          G_BINDING_SYNC_CREATE);
}

static void
gb_search_provider_init (GbSearchProviderIface *iface)
{
   iface->focus_search = gb_source_pane_focus_search;
}

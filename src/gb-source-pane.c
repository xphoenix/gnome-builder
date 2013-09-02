/* gb-source-pane.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtksourceview/gtksource.h>
#include <nautilus/nautilus-floating-bar.h>

#include "gb-animation.h"
#include "gb-source-fullscreen-container.h"
#include "gb-search-provider.h"
#include "gb-source-diff.h"
#include "gb-source-gutter-renderer-compiler.h"
#include "gb-source-gutter-renderer-diff.h"
#include "gb-source-overlay.h"
#include "gb-source-pane.h"
#include "gb-source-snippet.h"
#include "gb-source-snippet-completion-provider.h"
#include "gb-source-snippets.h"
#include "gb-source-snippets-manager.h"
#include "gb-source-view.h"
#include "gb-zoomable.h"

static void gb_search_provider_init (GbSearchProviderIface *iface);
static void gb_zoomable_init        (GbZoomableInterface *iface);

G_DEFINE_TYPE_EXTENDED(GbSourcePane,
                       gb_source_pane,
                       GB_TYPE_WORKSPACE_PANE,
                       0,
                       G_IMPLEMENT_INTERFACE(GB_TYPE_SEARCH_PROVIDER,
                                             gb_search_provider_init)
                       G_IMPLEMENT_INTERFACE(GB_TYPE_ZOOMABLE,
                                             gb_zoomable_init))

struct _GbSourcePanePrivate
{
   GFile        *file;
   GbSourceDiff *diff;

   GbSourceSnippets *snippets;

   GtkWidget *highlight;
   GtkWidget *overlay;
   GtkWidget *progress;
   GtkWidget *ruler;
   GtkWidget *scroller;
   GtkWidget *search_bar;
   GtkWidget *search_entry;
   GtkWidget *view;

   GtkSourceBuffer *buffer;

   GtkSourceSearchContext  *search_context;
   GtkSourceSearchSettings *search_settings;

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
      "solarizeddark",
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

   gb_source_diff_set_file(priv->diff, file);

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
   GtkTextIter iter;
   GError *error = NULL;
   gchar *contents;
   gchar *path;
   gchar *uri;
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

   if ((uri = g_file_get_uri(file))) {
      gb_workspace_pane_set_uri(GB_WORKSPACE_PANE(pane), uri);
      g_free(uri);
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

   gtk_text_buffer_get_start_iter(buffer, &iter);
   gtk_text_buffer_select_range(buffer, &iter, &iter);

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
   gb_workspace_pane_set_can_save(GB_WORKSPACE_PANE(pane), modified);
}

static void
gb_source_pane_reload_snippets (GbSourcePane      *pane,
                                GtkSourceLanguage *language)
{
   GbSourceSnippetsManager *manager;
   GbSourceSnippets *snippets = NULL;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(!language || GTK_SOURCE_IS_LANGUAGE(language));

   manager = gb_source_snippets_manager_get_default();

   if (language) {
      snippets = gb_source_snippets_manager_get_for_language(manager, language);
      gb_source_snippets_clear(pane->priv->snippets);
      gb_source_snippets_merge(pane->priv->snippets, snippets);
   }
}

static void
gb_source_pane_language_changed (GbSourcePane      *pane,
                                 GParamSpec        *pspec,
                                 GtkSourceBuffer   *buffer)
{
   GtkSourceLanguage *language;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(pspec);
   g_return_if_fail(GTK_SOURCE_IS_BUFFER(buffer));

   language = gtk_source_buffer_get_language(buffer);
   gb_source_pane_reload_snippets(pane, language);
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
   if (mark && (mark == gtk_text_buffer_get_insert(buffer))) {
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

   g_object_set(pane->priv->search_bar,
                "search-mode-enabled", TRUE,
                NULL);
   gtk_widget_grab_focus(pane->priv->search_entry);
}

static void
gb_source_pane_dispose (GObject *object)
{
   GbSourcePanePrivate *priv = GB_SOURCE_PANE(object)->priv;
   GtkTextBuffer *buffer;

   g_clear_object(&priv->diff);

   if (priv->view) {
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
   }

   G_OBJECT_CLASS(gb_source_pane_parent_class)->dispose(object);
}

static gboolean
gb_source_pane_view_key_press (GtkTextView  *text_view,
                               GdkEventKey  *key,
                               GbSourcePane *pane)
{
   GbSourcePanePrivate *priv = pane->priv;
   gboolean was_enabled;

   if (key->keyval == GDK_KEY_Escape) {
      g_object_get(priv->search_bar,
                   "search-mode-enabled", &was_enabled,
                   NULL);
      if (was_enabled) {
         gtk_widget_hide(priv->highlight);
         g_object_set(priv->search_bar,
                      "search-mode-enabled", FALSE,
                      NULL);
         return TRUE;
      }
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
      gtk_widget_hide(priv->highlight);
      g_object_set(priv->search_bar,
                   "search-mode-enabled", FALSE,
                   NULL);
      gtk_widget_grab_focus(priv->view);
      return TRUE;
   } else if (key->keyval == GDK_KEY_Return) {
      gtk_widget_show(priv->highlight);
   }

   return FALSE;
}

static void
gb_source_pane_search_entry_changed (GtkEntry     *entry,
                                     GbSourcePane *pane)
{
   GbSourcePanePrivate *priv;
   GdkRectangle rect;
   GtkTextIter iter;
   GtkTextIter match_begin;
   GtkTextIter match_end;
   gboolean search_mode_enabled;
   const char *text;

   g_assert(GTK_IS_ENTRY(entry));
   g_assert(GB_IS_SOURCE_PANE(pane));

   priv = pane->priv;

   /*
    * Ignore the change if it is by the search bar hiding.
    */
   g_object_get(priv->search_bar,
                "search-mode-enabled", &search_mode_enabled,
                NULL);
   if (!search_mode_enabled) {
      return;
   }

   text = gtk_entry_get_text(entry);

   /*
    * Update the search query. It doesn't like "" much.
    */
   if (text && !*text) {
      text = NULL;
   }
   gtk_source_search_settings_set_search_text(priv->search_settings, text);

   /*
    * Make sure our overlay is visible.
    */
   gtk_widget_set_visible(priv->highlight, !!text);

   /*
    * Scroll to the first match if we can find one.
    */
   gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(priv->view), &rect);
   gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(priv->view),
                                      &iter,
                                      rect.x,
                                      rect.y);
   if (gtk_source_search_context_forward(priv->search_context,
                                         &iter,
                                         &match_begin,
                                         &match_end)) {
      gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(priv->view),
                                   &match_begin,
                                   0.25,
                                   TRUE,
                                   0.0,
                                   0.5);
   }
}

static void
gb_source_pane_save_async (GbWorkspacePane     *pane,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data)
{
   GbSourcePanePrivate *priv;
   GOutputStream *output_stream;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;
   GtkDialog *dialog;
   GtkWidget *toplevel;
   GError *error = NULL;
   gsize written;
   GFile *file;
   gchar *uri;
   gchar *text = NULL;
   gsize length = 0;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   /*
    * TODO: Make this all async.
    */

   priv = GB_SOURCE_PANE(pane)->priv;

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->progress), 0.0);
   gtk_widget_show(priv->progress);

   gb_workspace_pane_set_busy(pane, TRUE);

   /*
    * If we do not yet have a filename, we need to prompt the user for the
    * target filename.
    */
   if (!priv->file) {
      toplevel = gtk_widget_get_toplevel(GTK_WIDGET(pane));
      dialog = g_object_new(GTK_TYPE_FILE_CHOOSER_DIALOG,
                            "action", GTK_FILE_CHOOSER_ACTION_SAVE,
                            "transient-for", toplevel,
                            "title", _("Save File..."),
                            NULL);
      gtk_dialog_add_buttons(dialog,
                             _("Cancel"), GTK_RESPONSE_CANCEL,
                             _("Save"), GTK_RESPONSE_ACCEPT,
                             NULL);
      if (gtk_dialog_run(dialog) != GTK_RESPONSE_ACCEPT) {
         gtk_widget_destroy(GTK_WIDGET(dialog));
         gb_workspace_pane_set_busy(pane, FALSE);
         gtk_widget_hide(priv->progress);
         return;
      }
      uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
      file = g_file_new_for_uri(uri);
      gb_source_pane_set_file(GB_SOURCE_PANE(pane), file);
      g_free(uri);
      gtk_widget_destroy(GTK_WIDGET(dialog));
   } else {
      file = g_object_ref(priv->file);
   }

   output_stream = G_OUTPUT_STREAM(g_file_replace(file, NULL, TRUE,
                                                  G_FILE_CREATE_NONE,
                                                  NULL, &error));
   g_object_unref(file);

   if (!output_stream) {
      g_printerr("Failed to open file: %s\n", error->message);
      g_error_free(error);
      return;
   }

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->view));
   gtk_text_buffer_get_bounds(buffer, &begin, &end);
   text = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
   length = strlen(text);

   if (!g_output_stream_write_all(output_stream, text, length, &written,
                                  NULL, &error)) {
      g_printerr("Failed to write to file: %s\n", error->message);
      g_object_unref(output_stream);
      g_error_free(error);
      g_free(text);
      return;
   }

   g_free(text);
   g_object_unref(output_stream);

   gtk_text_buffer_set_modified(buffer, FALSE);

   gb_object_animate_full(priv->progress,
                          GB_ANIMATION_EASE_IN_OUT_QUAD,
                          2000,
                          NULL,
                          (GDestroyNotify)gtk_widget_hide,
                          priv->progress,
                          "fraction", 1.0,
                          NULL);

   gb_workspace_pane_set_busy(pane, FALSE);

   gb_source_diff_queue_parse(priv->diff);
}

static gboolean
gb_source_pane_save_finish (GbWorkspacePane  *pane,
                            GAsyncResult     *result,
                            GError          **error)
{
}

static void
gb_source_pane_zoom_by (GbSourcePane *pane,
                        gdouble       fraction)
{
   PangoFontDescription *font = NULL;
   GbSourcePanePrivate *priv;
   GtkStyleContext *style_context;
   gint font_size;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));

   priv = pane->priv;

   style_context = gtk_widget_get_style_context(priv->view);
   gtk_style_context_get(style_context, GTK_STATE_FLAG_NORMAL,
                         "font", &font,
                         NULL);

   font_size = pango_font_description_get_size(font);
   font_size *= fraction;
   pango_font_description_set_size(font, font_size);

   gtk_widget_override_font(priv->view, font);
   pango_font_description_free(font);
}

static void
gb_source_pane_zoom_in (GbZoomable *zoomable)
{
   gb_source_pane_zoom_by(GB_SOURCE_PANE(zoomable), PANGO_SCALE_LARGE);
}

static void
gb_source_pane_zoom_out (GbZoomable *zoomable)
{
   gb_source_pane_zoom_by(GB_SOURCE_PANE(zoomable), PANGO_SCALE_SMALL);
}

static void
setup_source_view (GbSourcePane  *pane,
                   GtkSourceView *source_view)
{
   GtkSourceCompletionProvider *provider;
   GtkSourceGutterRenderer *renderer;
   PangoFontDescription *font;
   GtkSourceCompletion *completion;
   GbSourcePanePrivate *priv;
   GtkSourceGutter *gutter;

   priv = pane->priv;

   g_object_set(source_view,
                "buffer", priv->buffer,
                "show-line-numbers", TRUE,
                "show-right-margin", TRUE,
                "right-margin-position", 80,
                NULL);

   completion = gtk_source_view_get_completion(source_view);

   g_object_set(completion,
                "show-headers", FALSE,
                NULL);

   provider = gb_source_snippet_completion_provider_new(
         GB_SOURCE_VIEW(source_view),
         priv->snippets);
   gtk_source_completion_add_provider(completion, provider, NULL);
   g_object_unref(provider);

   gutter = gtk_source_view_get_gutter(source_view,
                                       GTK_TEXT_WINDOW_LEFT);

   renderer = g_object_new(GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER, NULL);
   gtk_source_gutter_renderer_set_padding(renderer, 2, 1);
   gtk_source_gutter_renderer_set_size(renderer, 14);
   gtk_source_gutter_insert(gutter, renderer, -30);

   renderer = g_object_new(GB_TYPE_SOURCE_GUTTER_RENDERER_DIFF,
                           "diff", priv->diff,
                           NULL);
   gtk_source_gutter_renderer_set_size(renderer, 2);
   gtk_source_gutter_renderer_set_padding(renderer, 2, 0);
   gtk_source_gutter_insert(gutter, renderer, -10);

   font = pango_font_description_from_string("Monospace");
   gtk_widget_override_font(GTK_WIDGET(source_view), font);
   pango_font_description_free(font);
}

static void
place_over (GtkWidget *window,
            GtkWidget *other)
{
   GtkAllocation alloc;
   GdkWindow *gwin;
   gint x;
   gint y;

   gtk_widget_get_allocation(other, &alloc);
   gwin = gtk_widget_get_window(other);
   gdk_window_get_origin(gwin, &x, &y);

   x += alloc.x;
   y += alloc.y;

   gtk_window_set_default_size(GTK_WINDOW(window), alloc.width, alloc.height);
   gtk_window_move(GTK_WINDOW(window), x, y);
}

static gboolean
fullscreen (gpointer data)
{
   gtk_window_fullscreen(data);

   return FALSE;
}

static void
gb_source_pane_fullscreen (GbWorkspacePane *pane)
{
   PangoFontDescription *font;
   GbSourcePanePrivate *priv;
   GtkStyleContext *context;
   const gchar *uri;
   GtkWidget *container;
   GtkWidget *source_view;
   GtkWidget *window;

   priv = GB_SOURCE_PANE(pane)->priv;

   uri = gb_workspace_pane_get_uri(pane);

   window = g_object_new(GTK_TYPE_WINDOW,
                         "title", uri,
                         NULL);

   container = g_object_new(GB_TYPE_SOURCE_FULLSCREEN_CONTAINER,
                            "visible", TRUE,
                            NULL);
   gtk_container_add(GTK_CONTAINER(window), container);

   source_view = g_object_new(GB_TYPE_SOURCE_VIEW,
                              "visible", TRUE,
                              NULL);

   setup_source_view(GB_SOURCE_PANE(pane), GTK_SOURCE_VIEW(source_view));

   gtk_container_add(GTK_CONTAINER(container), source_view);

   place_over(window, priv->scroller);

   gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

   gtk_window_present(GTK_WINDOW(window));

   g_timeout_add(100, fullscreen, window);
}

static void
gb_source_pane_finalize (GObject *object)
{
   GbSourcePanePrivate *priv = GB_SOURCE_PANE(object)->priv;

   g_clear_object(&priv->buffer);
   g_clear_object(&priv->file);
   g_clear_object(&priv->diff);
   g_clear_object(&priv->search_context);
   g_clear_object(&priv->search_settings);
   g_clear_object(&priv->snippets);

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
   GbWorkspacePaneClass *workspace_pane_class;
   GtkWidgetClass *widget_class;
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_source_pane_dispose;
   object_class->finalize = gb_source_pane_finalize;
   object_class->get_property = gb_source_pane_get_property;
   object_class->set_property = gb_source_pane_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourcePanePrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->grab_focus = gb_source_pane_grab_focus;

   workspace_pane_class = GB_WORKSPACE_PANE_CLASS(klass);
   workspace_pane_class->save_async = gb_source_pane_save_async;
   workspace_pane_class->save_finish = gb_source_pane_save_finish;
   workspace_pane_class->fullscreen = gb_source_pane_fullscreen;

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
   GtkSourceGutter *gutter;

   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_SOURCE_PANE,
                                            GbSourcePanePrivate);

   priv = pane->priv;

   g_object_set(pane,
                "can-fullscreen", TRUE,
                "icon-name", "text-x-generic",
                "title", _("Unnamed File"),
                NULL);

   priv->snippets = gb_source_snippets_new();

   priv->search_bar = g_object_new(GTK_TYPE_SEARCH_BAR,
                                   "show-close-button", FALSE,
                                   "visible", TRUE,
                                   "vexpand", FALSE,
                                   "hexpand", TRUE,
                                   NULL);
   gtk_container_add(GTK_CONTAINER(pane), priv->search_bar);

   priv->search_entry = g_object_new(GTK_TYPE_SEARCH_ENTRY,
                                     "margin-left", 100,
                                     "margin-right", 100,
                                     "hexpand", TRUE,
                                     "visible", TRUE,
                                     "width-chars", 32,
                                     NULL);
   g_signal_connect(priv->search_entry, "key-press-event",
                    G_CALLBACK(gb_source_pane_search_entry_key_press),
                    pane);
   g_signal_connect(priv->search_entry, "changed",
                    G_CALLBACK(gb_source_pane_search_entry_changed),
                    pane);
   gtk_container_add(GTK_CONTAINER(priv->search_bar), priv->search_entry);
   gtk_search_bar_connect_entry(GTK_SEARCH_BAR(priv->search_bar),
                                GTK_ENTRY(priv->search_entry));

   priv->overlay = g_object_new(GTK_TYPE_OVERLAY,
                                "visible", TRUE,
                                NULL);
   g_signal_connect(priv->overlay, "get-child-position",
                    G_CALLBACK(get_child_position),
                    pane);
   gtk_container_add_with_properties(GTK_CONTAINER(pane), priv->overlay,
                                     "height", 1,
                                     "left-attach", 0,
                                     "top-attach", 1,
                                     "width", 1,
                                     NULL);

   priv->progress = gtk_progress_bar_new();
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->progress),
                               GTK_STYLE_CLASS_OSD);
   gtk_widget_set_halign(priv->progress, GTK_ALIGN_FILL);
   gtk_widget_set_valign(priv->progress, GTK_ALIGN_START);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->progress);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "hexpand", TRUE,
                                 "vexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->overlay), priv->scroller);

   priv->buffer = gtk_source_buffer_new(NULL);
   g_signal_connect_swapped(priv->buffer,
                            "modified-changed",
                            G_CALLBACK(gb_source_pane_emit_modified_changed),
                            pane);
   g_signal_connect_swapped(priv->buffer,
                            "notify::language",
                            G_CALLBACK(gb_source_pane_language_changed),
                            pane);
   priv->view = g_object_new(GB_TYPE_SOURCE_VIEW,
                             "buffer", priv->buffer,
                             "visible", TRUE,
                             NULL);
   g_object_add_weak_pointer(G_OBJECT(priv->view), (gpointer *)&priv->view);
   g_signal_connect(priv->view, "key-press-event",
                    G_CALLBACK(gb_source_pane_view_key_press),
                    pane);
   gtk_container_add(GTK_CONTAINER(priv->scroller), priv->view);

   priv->diff = gb_source_diff_new(NULL, GTK_TEXT_BUFFER(priv->buffer));

   setup_source_view(pane, GTK_SOURCE_VIEW(priv->view));

   priv->search_settings = g_object_new(GTK_SOURCE_TYPE_SEARCH_SETTINGS,
                                        "search-text", NULL,
                                        "regex-enabled", FALSE,
                                        NULL);

   priv->search_context = g_object_new(GTK_SOURCE_TYPE_SEARCH_CONTEXT,
                                       "buffer", priv->buffer,
                                       "highlight", TRUE,
                                       "settings", priv->search_settings,
                                       NULL);

   priv->highlight = g_object_new(GB_TYPE_SOURCE_OVERLAY,
                                  "hexpand", TRUE,
                                  "search-context", priv->search_context,
                                  "source-view", priv->view,
                                  "vexpand", TRUE,
                                  "visible", FALSE,
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

   gb_source_pane_load_scheme(pane);

   pane->priv->insert_text_handler =
      g_signal_connect_after(priv->buffer, "insert-text",
                             G_CALLBACK(on_insert_text), pane);

   pane->priv->delete_range_handler =
      g_signal_connect_after(priv->buffer, "delete-range",
                             G_CALLBACK(on_delete_range), pane);

   pane->priv->mark_set_handler =
      g_signal_connect_after(priv->buffer, "mark-set",
                             G_CALLBACK(on_mark_set), pane);
}

static void
gb_search_provider_init (GbSearchProviderIface *iface)
{
   iface->focus_search = gb_source_pane_focus_search;
}

static void
gb_zoomable_init (GbZoomableInterface *iface)
{
   iface->zoom_in = gb_source_pane_zoom_in;
   iface->zoom_out = gb_source_pane_zoom_out;
}

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

#include "gb-animation.h"
#include "gb-language-c-completion-provider.h"
#include "gb-source-diff.h"
#include "gb-source-gutter-renderer-diff.h"
#include "gb-source-overlay.h"
#include "gb-source-pane.h"
#include "gb-source-snippet.h"
#include "gb-source-snippet-completion-provider.h"
#include "gb-source-snippets.h"
#include "gb-source-snippets-manager.h"
#include "gb-source-typelib-completion-provider.h"
#include "gb-source-view.h"
#include "gb-zoomable.h"
#include "nautilus-floating-bar.h"

struct _GbSourcePanePrivate
{
   GFile *file;

   GtkSourceBuffer *buffer;

   GbSourceDiff            *diff;
   GtkSourceGutterRenderer *diff_renderer;

   GbSourceSnippets            *snippets;
   GtkSourceCompletionProvider *snippets_provider;

   GtkSourceCompletionProvider *typelib_provider;

   GtkWidget *floating_bar;
   GtkWidget *overlay;
   GtkWidget *progress;
   GtkWidget *scroller;
   GtkWidget *search_bar;
   GtkWidget *search_entry;
   GtkWidget *source_overlay;
   GtkWidget *view;

   GtkSourceSearchContext  *search_context;
   GtkSourceSearchSettings *search_settings;

   guint buffer_delete_range_handler;
   guint buffer_insert_text_handler;
   guint buffer_mark_set_handler;
   guint buffer_modified_changed_handler;
   guint buffer_notify_language_handler;

   guint view_focus_in_event_handler;
   guint view_focus_out_event_handler;
   guint view_key_press_event_handler;

   guint search_entry_key_press_event_handler;
   guint search_entry_changed_handler;
};

enum
{
   PROP_0,
   PROP_FILE,
   PROP_FONT,
   PROP_STYLE_SCHEME_NAME,
   LAST_PROP
};

static void gb_zoomable_init (GbZoomableInterface *iface);

G_DEFINE_TYPE_EXTENDED(GbSourcePane,
                       gb_source_pane,
                       GB_TYPE_WORKSPACE_PANE,
                       0,
                       G_IMPLEMENT_INTERFACE(GB_TYPE_ZOOMABLE,
                                             gb_zoomable_init))

static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_source_pane_set_style_scheme_name (GbSourcePane *pane,
                                      const gchar  *name)
{
   GtkSourceStyleSchemeManager *m;
   GtkSourceStyleScheme *s;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));

   if (!name) {
      name = "solarized-light";
   }

   m = gtk_source_style_scheme_manager_get_default();
   s = gtk_source_style_scheme_manager_get_scheme(m, name);
   gtk_source_buffer_set_style_scheme(pane->priv->buffer, s);

   g_object_notify_by_pspec(G_OBJECT(pane),
                            gParamSpecs[PROP_STYLE_SCHEME_NAME]);
}

static void
gb_source_pane_set_font (GbSourcePane *pane,
                         const gchar  *font)
{
   PangoFontDescription *font_desc = NULL;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));

   /*
    * TODO: This should take into account the current zoom.
    */

   if (font) {
      font_desc = pango_font_description_from_string(font);
   }

   gtk_widget_override_font(GTK_WIDGET(pane->priv->view), font_desc);

   if (font_desc) {
      pango_font_description_free(font_desc);
   }

   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_FONT]);
}

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

      if (!g_strcmp0(gtk_source_language_get_id(l), "c")) {
         GtkSourceCompletionProvider *provider;
         GtkSourceCompletion *completion;

         completion = gtk_source_view_get_completion(GTK_SOURCE_VIEW(priv->view));
         provider = g_object_new(GB_TYPE_LANGUAGE_C_COMPLETION_PROVIDER,
                                 NULL);
         gtk_source_completion_add_provider(completion, provider, NULL);
         g_object_unref(provider);
      }
   }

   /*
    * TODO: Based on language, update completion provider language
    *       so that it may provide the right format.
    */

   g_clear_object(&info);
   g_free(base);
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
buffer_modified_changed_cb (GbSourcePane *pane)
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
   GbSourceSnippets *snippets;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));
   g_return_if_fail(!language || GTK_SOURCE_IS_LANGUAGE(language));

   g_clear_object(&pane->priv->snippets);

   if (language) {
      manager = gb_source_snippets_manager_get_default();
      snippets = gb_source_snippets_manager_get_for_language(manager, language);
      pane->priv->snippets = snippets ? g_object_ref(snippets) : NULL;
      g_object_set(pane->priv->snippets_provider,
                   "snippets", snippets,
                   NULL);
   }
}

static void
buffer_notify_language_cb (GbSourcePane    *pane,
                           GParamSpec      *pspec,
                           GtkSourceBuffer *buffer)
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
   GbSourcePane *pane = (GbSourcePane *)widget;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));

   gtk_widget_grab_focus(GTK_WIDGET(pane->priv->view));
}

static void
gb_source_pane_update_cursor_position (GbSourcePane  *pane,
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
   g_object_set(pane->priv->floating_bar, "label", text, NULL);
   g_free(text);

   if (!gtk_widget_get_visible(pane->priv->floating_bar)) {
      gtk_widget_show(pane->priv->floating_bar);
   }
}

static void
buffer_mark_set_cb (GtkTextBuffer *buffer,
                    GtkTextIter   *location,
                    GtkTextMark   *mark,
                    GbSourcePane  *pane)
{
   g_assert(GTK_IS_TEXT_BUFFER(buffer));
   g_assert(GB_IS_SOURCE_PANE(pane));

   if (mark && (mark == gtk_text_buffer_get_insert(buffer))) {
      gb_source_pane_update_cursor_position(pane, buffer);
   }
}

static void
buffer_insert_text_cb (GtkTextBuffer *buffer,
                       GtkTextIter   *location,
                       gchar         *text,
                       gint           length,
                       GbSourcePane  *pane)
{
   g_assert(GTK_IS_TEXT_BUFFER(buffer));
   g_assert(GB_IS_SOURCE_PANE(pane));

   gb_source_pane_update_cursor_position(pane, buffer);
}

static void
buffer_delete_range_cb (GtkTextBuffer *buffer,
                        GtkTextIter   *begin,
                        GtkTextIter   *end,
                        GbSourcePane  *pane)
{
   g_assert(GTK_IS_TEXT_BUFFER(buffer));
   g_assert(GB_IS_SOURCE_PANE(pane));

   gb_source_pane_update_cursor_position(pane, buffer);
}

static void
gb_source_pane_focus_search (GbWorkspacePane *pane)
{
   GbSourcePanePrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_PANE(pane));

   priv = GB_SOURCE_PANE(pane)->priv;

   g_object_set(priv->search_bar,
                "search-mode-enabled", TRUE,
                NULL);
   gtk_widget_grab_focus(priv->search_entry);
}

static void
gb_source_pane_dispose (GObject *object)
{
   GbSourcePanePrivate *priv = GB_SOURCE_PANE(object)->priv;

#define DISCONNECT(object, field) \
   if (object && field) { \
      g_signal_handler_disconnect(object, field); \
      field = 0; \
   }

   DISCONNECT(priv->buffer, priv->buffer_delete_range_handler);
   DISCONNECT(priv->buffer, priv->buffer_insert_text_handler);
   DISCONNECT(priv->buffer, priv->buffer_mark_set_handler);
   DISCONNECT(priv->buffer, priv->buffer_modified_changed_handler);
   DISCONNECT(priv->buffer, priv->buffer_notify_language_handler);

   DISCONNECT(priv->view, priv->view_focus_in_event_handler);
   DISCONNECT(priv->view, priv->view_focus_out_event_handler);
   DISCONNECT(priv->view, priv->view_key_press_event_handler);

   DISCONNECT(priv->search_entry, priv->search_entry_key_press_event_handler);
   DISCONNECT(priv->search_entry, priv->search_entry_changed_handler);

#undef DISCONNECT

   g_clear_object(&priv->file);
   g_clear_object(&priv->buffer);
   g_clear_object(&priv->diff);
   g_clear_object(&priv->search_context);
   g_clear_object(&priv->search_settings);
   g_clear_object(&priv->snippets);
   g_clear_object(&priv->snippets_provider);
   g_clear_object(&priv->typelib_provider);

   G_OBJECT_CLASS(gb_source_pane_parent_class)->dispose(object);
}

static gboolean
view_key_press_event_cb (GtkTextView  *text_view,
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
         gtk_widget_hide(priv->source_overlay);
         g_object_set(priv->search_bar,
                      "search-mode-enabled", FALSE,
                      NULL);
         return TRUE;
      }
   }

   return FALSE;
}

static gboolean
search_entry_key_press_event_cb (GtkSearchEntry *search_entry,
                                 GdkEventKey    *key,
                                 GbSourcePane   *pane)
{
   GbSourcePanePrivate *priv = pane->priv;

   if (key->keyval == GDK_KEY_Escape) {
      gtk_widget_hide(priv->source_overlay);
      g_object_set(priv->search_bar,
                   "search-mode-enabled", FALSE,
                   NULL);
      gtk_widget_grab_focus(priv->view);
      return TRUE;
   } else if (key->keyval == GDK_KEY_Return) {
      gtk_widget_show(priv->source_overlay);
   }

   return FALSE;
}

static void
search_entry_changed_cb (GtkEntry     *entry,
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
   gtk_widget_set_visible(priv->source_overlay, !!text);

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
   g_return_if_fail(GB_IS_SOURCE_PANE(zoomable));

   gb_source_pane_zoom_by(GB_SOURCE_PANE(zoomable), PANGO_SCALE_LARGE);
}

static void
gb_source_pane_zoom_out (GbZoomable *zoomable)
{
   g_return_if_fail(GB_IS_SOURCE_PANE(zoomable));

   gb_source_pane_zoom_by(GB_SOURCE_PANE(zoomable), PANGO_SCALE_SMALL);
}

static gboolean
view_focus_out_event_cb (GtkSourceView       *source_view,
                         GdkEventFocus       *event,
                         GtkSourceCompletion *completion)
{
   g_return_if_fail(GTK_SOURCE_IS_COMPLETION(completion));

   gtk_source_completion_block_interactive(completion);

   return FALSE;
}

static gboolean
view_focus_in_event_cb (GtkSourceView       *source_view,
                        GdkEventFocus       *event,
                        GtkSourceCompletion *completion)
{
   g_return_if_fail(GTK_SOURCE_IS_COMPLETION(completion));

   gtk_source_completion_unblock_interactive(completion);

   return FALSE;
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
   case PROP_FONT:
      gb_source_pane_set_font(pane, g_value_get_string(value));
      break;
   case PROP_STYLE_SCHEME_NAME:
      gb_source_pane_set_style_scheme_name(pane, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_pane_class_init (GbSourcePaneClass *klass)
{
   GtkSourceStyleSchemeManager *sm;
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
   workspace_pane_class->focus_search = gb_source_pane_focus_search;
   workspace_pane_class->save_async = gb_source_pane_save_async;
   workspace_pane_class->save_finish = gb_source_pane_save_finish;

   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("The file to load from source."),
                          G_TYPE_FILE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);

   gParamSpecs[PROP_FONT] =
      g_param_spec_string("font",
                          _("Font"),
                          _("The font to use on the source view."),
                          "Monospace",
                          (G_PARAM_WRITABLE |
                           G_PARAM_CONSTRUCT |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FONT,
                                   gParamSpecs[PROP_FONT]);

   gParamSpecs[PROP_STYLE_SCHEME_NAME] =
      g_param_spec_string("style-scheme-name",
                          _("Style Scheme Name"),
                          _("The name of the style scheme to use."),
                          NULL,
                          (G_PARAM_WRITABLE |
                           G_PARAM_CONSTRUCT |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STYLE_SCHEME_NAME,
                                   gParamSpecs[PROP_STYLE_SCHEME_NAME]);

   sm = gtk_source_style_scheme_manager_get_default ();
   gtk_source_style_scheme_manager_append_search_path (sm, "resource://org/gnome/Builder/data/style-schemes");
}

static void
gb_source_pane_init (GbSourcePane *pane)
{
   GtkSourceGutterRenderer *renderer;
   GtkSourceCompletion *completion;
   GbSourcePanePrivate *priv;
   GtkSourceGutter *gutter;

   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_SOURCE_PANE,
                                            GbSourcePanePrivate);

   priv = pane->priv;

   g_object_set(pane,
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
                                     "width-chars", 48,
                                     NULL);
   gtk_container_add(GTK_CONTAINER(priv->search_bar), priv->search_entry);
   gtk_search_bar_connect_entry(GTK_SEARCH_BAR(priv->search_bar),
                                GTK_ENTRY(priv->search_entry));

   priv->search_entry_key_press_event_handler =
      g_signal_connect(priv->search_entry, "key-press-event",
                       G_CALLBACK(search_entry_key_press_event_cb),
                       pane);

   priv->search_entry_changed_handler =
      g_signal_connect(priv->search_entry, "changed",
                       G_CALLBACK(search_entry_changed_cb),
                       pane);

   priv->overlay = g_object_new(GTK_TYPE_OVERLAY,
                                "visible", TRUE,
                                NULL);
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

   priv->buffer_modified_changed_handler =
      g_signal_connect_swapped(priv->buffer,
                               "modified-changed",
                               G_CALLBACK(buffer_modified_changed_cb),
                               pane);

   priv->buffer_notify_language_handler =
      g_signal_connect_swapped(priv->buffer,
                               "notify::language",
                               G_CALLBACK(buffer_notify_language_cb),
                               pane);

   priv->view = g_object_new(GB_TYPE_SOURCE_VIEW,
                             "buffer", priv->buffer,
                             "insert-spaces-instead-of-tabs", TRUE,
                             "right-margin-position", 80,
                             "show-line-numbers", TRUE,
                             "show-right-margin", TRUE,
                             "tab-width", 3,
                             "visible", TRUE,
                             NULL);
   gtk_container_add(GTK_CONTAINER(priv->scroller), priv->view);

   completion = gtk_source_view_get_completion(GTK_SOURCE_VIEW(priv->view));
   g_object_set(completion,
                "show-headers", FALSE,
                NULL);

   priv->view_key_press_event_handler =
      g_signal_connect(priv->view, "key-press-event",
                       G_CALLBACK(view_key_press_event_cb),
                       pane);

   priv->view_focus_in_event_handler =
      g_signal_connect(priv->view,
                       "focus-in-event",
                       G_CALLBACK(view_focus_in_event_cb),
                       completion);

   priv->view_focus_out_event_handler =
      g_signal_connect(priv->view,
                       "focus-out-event",
                       G_CALLBACK(view_focus_out_event_cb),
                       completion);

   gtk_source_completion_block_interactive(completion);

   priv->snippets_provider =
      gb_source_snippet_completion_provider_new(GB_SOURCE_VIEW(priv->view),
                                                priv->snippets);
   gtk_source_completion_add_provider(completion,
                                      priv->snippets_provider,
                                      NULL);

   pane->priv->typelib_provider = gb_source_typelib_completion_provider_new();
   gtk_source_completion_add_provider(completion,
                                      priv->typelib_provider,
                                      NULL);

   priv->diff = gb_source_diff_new(NULL, GTK_TEXT_BUFFER(priv->buffer));

   gutter = gtk_source_view_get_gutter(GTK_SOURCE_VIEW(priv->view),
                                       GTK_TEXT_WINDOW_LEFT);

   priv->diff_renderer = gb_source_gutter_renderer_diff_new(priv->diff);
   gtk_source_gutter_renderer_set_size(priv->diff_renderer, 2);
   gtk_source_gutter_renderer_set_padding(priv->diff_renderer, 2, 0);
   gtk_source_gutter_insert(gutter, priv->diff_renderer, -10);

   priv->search_settings = g_object_new(GTK_SOURCE_TYPE_SEARCH_SETTINGS,
                                        "search-text", NULL,
                                        "regex-enabled", FALSE,
                                        NULL);

   priv->search_context = g_object_new(GTK_SOURCE_TYPE_SEARCH_CONTEXT,
                                       "buffer", priv->buffer,
                                       "highlight", TRUE,
                                       "settings", priv->search_settings,
                                       NULL);

   priv->source_overlay = g_object_new(GB_TYPE_SOURCE_OVERLAY,
                                       "hexpand", TRUE,
                                       "search-context", priv->search_context,
                                       "source-view", priv->view,
                                       "vexpand", TRUE,
                                       "visible", FALSE,
                                       NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->source_overlay);

   priv->floating_bar = g_object_new(NAUTILUS_TYPE_FLOATING_BAR,
                                     "label", _("Line: 1  Column: 1"),
                                     "halign", GTK_ALIGN_END,
                                     "valign", GTK_ALIGN_END,
                                     "visible", TRUE,
                                     NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->floating_bar);

   if (gtk_widget_get_direction(priv->view) == GTK_TEXT_DIR_RTL) {
      gtk_widget_set_halign(priv->floating_bar, GTK_ALIGN_START);
   }

   pane->priv->buffer_insert_text_handler =
      g_signal_connect_after(priv->buffer,
                             "insert-text",
                             G_CALLBACK(buffer_insert_text_cb),
                             pane);

   pane->priv->buffer_delete_range_handler =
      g_signal_connect_after(priv->buffer,
                             "delete-range",
                             G_CALLBACK(buffer_delete_range_cb),
                             pane);

   pane->priv->buffer_mark_set_handler =
      g_signal_connect_after(priv->buffer,
                             "mark-set",
                             G_CALLBACK(buffer_mark_set_cb),
                             pane);
}

static void
gb_zoomable_init (GbZoomableInterface *iface)
{
   iface->zoom_in = gb_source_pane_zoom_in;
   iface->zoom_out = gb_source_pane_zoom_out;
}

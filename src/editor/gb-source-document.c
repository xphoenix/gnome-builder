/* gb-source-document.c
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
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourceview.h>

#include "gb-buffer-tasks.h"
#include "gb-document.h"
#include "gb-gtk.h"
#include "gb-log.h"
#include "gb-source-document.h"
#include "gb-source-view.h"

#include "nautilus-floating-bar.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "source"

/**
 * SECTION:gb-source-document
 * @title: GbSourceDocument
 * @short_description: #GbDocument representing source files.
 *
 * The #GbSourceDocument class is used to implement the #GbDocument interface
 * for source code files.
 *
 * It provides save/load and brings together all the fancy features for a
 * single editor window.
 */

static void gb_document_init (GbDocumentInterface *klass);

G_DEFINE_TYPE_EXTENDED(GbSourceDocument,
                       gb_source_document,
                       GTK_TYPE_BIN,
                       0,
                       G_IMPLEMENT_INTERFACE(GB_TYPE_DOCUMENT,
                                             gb_document_init))

struct _GbSourceDocumentPrivate
{
   GtkTextBuffer *buffer;
   GSettings     *settings;
   GFile         *file;
   gchar         *title;

   guint          buffer_insert_text_handler;
   guint          buffer_delete_range_handler;
   guint          buffer_mark_set_handler;

   GtkWidget     *floating_bar;
   GtkWidget     *overlay;
   GtkWidget     *progress;
   GtkWidget     *scroller;
   GtkWidget     *view;
};

enum
{
   PROP_0,
   PROP_BUFFER,
   PROP_CAN_SAVE,
   PROP_FILE,
   PROP_LANGUAGE,
   PROP_STYLE_SCHEME,
   PROP_STYLE_SCHEME_NAME,
   PROP_TITLE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

static void
guess_and_apply_language (GbSourceDocument *document)
{
   GtkSourceLanguageManager *manager;
   GbSourceDocumentPrivate *priv;
   GtkSourceLanguage *lang;
   gboolean result_uncertain;
   gchar *filename;
   gchar *content_type;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   priv = document->priv;

   if (!priv->file) {
      return;
   }

   filename = g_file_get_path(priv->file);
   content_type = g_content_type_guess(filename, NULL, 0, &result_uncertain);
   if (result_uncertain) {
      g_free(content_type);
      content_type = NULL;
   }

   manager = gtk_source_language_manager_get_default();
   lang = gtk_source_language_manager_guess_language(manager,
                                                     filename,
                                                     content_type);

   gb_source_document_set_language(document, lang);

   g_free(content_type);
   g_free(filename);
}

static guint
get_last_edit_known_edit_offset (GFile *file)
{
   /*
    * TODO: Store the last edit position so when the document is
    *       reopened, we can place it at the same offset.
    */
   return 0;
}

GtkTextBuffer *
gb_source_document_get_buffer (GbSourceDocument *document)
{
   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   return document->priv->buffer;
}

GtkSourceLanguage *
gb_source_document_get_language (GbSourceDocument *document)
{
   GtkSourceBuffer *buffer;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   buffer = GTK_SOURCE_BUFFER(document->priv->buffer);
   return gtk_source_buffer_get_language(buffer);
}

void
gb_source_document_set_language (GbSourceDocument  *document,
                                 GtkSourceLanguage *language)
{
   GtkSourceBuffer *buffer;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   buffer = GTK_SOURCE_BUFFER(document->priv->buffer);
   gtk_source_buffer_set_language(buffer, language);
   g_object_notify_by_pspec(G_OBJECT(document), gParamSpecs[PROP_LANGUAGE]);
}

GtkSourceStyleScheme *
gb_source_document_get_style_scheme (GbSourceDocument *document)
{
   GtkSourceBuffer *buffer;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   buffer = GTK_SOURCE_BUFFER(document->priv->buffer);
   return gtk_source_buffer_get_style_scheme(buffer);
}

void
gb_source_document_set_style_scheme (GbSourceDocument     *document,
                                     GtkSourceStyleScheme *style_scheme)
{
   GtkSourceBuffer *buffer;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   buffer = GTK_SOURCE_BUFFER(document->priv->buffer);
   gtk_source_buffer_set_style_scheme(buffer, style_scheme);
   g_object_notify_by_pspec(G_OBJECT(document),
                            gParamSpecs[PROP_STYLE_SCHEME]);
}

void
gb_source_document_set_style_scheme_name (GbSourceDocument *document,
                                          const gchar      *style_scheme_name)
{
   GtkSourceStyleSchemeManager *manager;
   GtkSourceStyleScheme *style_scheme;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   manager = gtk_source_style_scheme_manager_get_default();
   style_scheme =
      gtk_source_style_scheme_manager_get_scheme(manager,
                                                 style_scheme_name);
   gb_source_document_set_style_scheme(document, style_scheme);
}

GFile *
gb_source_document_get_file (GbSourceDocument *document)
{
   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   return document->priv->file;
}

void
gb_source_document_set_file (GbSourceDocument *document,
                             GFile            *file)
{
   GbSourceDocumentPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));
   g_return_if_fail(!file || G_IS_FILE(file));

   priv = document->priv;

   g_clear_object(&priv->file);
   g_clear_pointer(&priv->title, g_free);

   priv->file = file ? g_object_ref(file) : NULL;

   g_object_notify_by_pspec(G_OBJECT(document), gParamSpecs[PROP_FILE]);
   g_object_notify_by_pspec(G_OBJECT(document), gParamSpecs[PROP_TITLE]);
}

static const gchar *
gb_source_document_get_title (GbDocument *document)
{
   GbSourceDocumentPrivate *priv;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   priv = GB_SOURCE_DOCUMENT(document)->priv;

   if (!priv->title && priv->file) {
      priv->title = g_file_get_basename(priv->file);
   }

   return priv->title ? priv->title : _("Unsaved Document");
}

static gboolean
gb_source_document_get_can_save (GbDocument *document)
{
   GbSourceDocumentPrivate *priv;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), FALSE);

   priv = GB_SOURCE_DOCUMENT(document)->priv;

   return gtk_text_buffer_get_modified(priv->buffer);
}

static GBytes *
gb_source_document_get_text_bytes (GbSourceDocument *document)
{
   GbSourceDocumentPrivate *priv;
   GtkTextIter begin;
   GtkTextIter end;
   gchar *text;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   priv = document->priv;

   gtk_text_buffer_get_bounds(priv->buffer, &begin, &end);
   text = gtk_text_buffer_get_text(priv->buffer, &begin, &end, TRUE);
   /*
    * I believe strlen() is safe on the following since we will get
    * a two-byte representation back for NUL bytes rather than \0.
    */
   return g_bytes_new_take(text, strlen(text));
}

static GFile *
gb_source_document_get_file_may_block (GbSourceDocument *document)
{
   GbSourceDocumentPrivate *priv;
   GtkWidget *toplevel;
   GtkDialog *dialog;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   priv = document->priv;

   if (!priv->file) {
      toplevel = gtk_widget_get_toplevel(GTK_WIDGET(document));
      dialog = g_object_new(GTK_TYPE_FILE_CHOOSER_DIALOG,
                            "action", GTK_FILE_CHOOSER_ACTION_SAVE,
                            "local-only", FALSE,
                            "select-multiple", FALSE,
                            "title", _("Save File"),
                            "transient-for", toplevel,
                            NULL);
      gtk_dialog_add_buttons(dialog,
                             _("Cancel"), GTK_RESPONSE_CANCEL,
                             _("Save"), GTK_RESPONSE_OK,
                             NULL);
      if (GTK_RESPONSE_OK == gtk_dialog_run(dialog)) {
         priv->file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
      }
      gtk_widget_destroy(GTK_WIDGET(dialog));
      if (priv->file) {
         g_object_notify_by_pspec(G_OBJECT(document), gParamSpecs[PROP_FILE]);
         g_object_notify_by_pspec(G_OBJECT(document), gParamSpecs[PROP_TITLE]);
      }
   }

   return priv->file ? g_object_ref(priv->file) : NULL;
}

static void
save_thread_func (GTask        *task,
                  gpointer      source_object,
                  gpointer      task_data,
                  GCancellable *cancellable)
{
   GFileProgressCallback progress;
   GFileOutputStream *stream;
   const guint8 *data;
   gboolean make_backup = TRUE;
   gpointer progress_data;
   GError *error = NULL;
   GBytes *bytes = task_data;
   gssize r;
   GFile *file;
   gsize towrite;
   gsize len;

   g_return_if_fail(G_IS_TASK(task));
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(source_object));
   g_return_if_fail(bytes);

   data = g_bytes_get_data(bytes, &len);
   file = g_object_get_data(G_OBJECT(task), "file");
   progress = g_object_get_data(G_OBJECT(task), "progress");
   progress_data = g_object_get_data(G_OBJECT(task), "progress_data");
   towrite = len;

   g_assert(data);
   g_assert(G_IS_FILE(file));

   /*
    * TODO: Plumb through setting for make_backup.
    */

#define CHECK_CANCELLABLE \
   if (g_cancellable_is_cancelled(cancellable)) return;

#define UPDATE_PROGRESS \
   if (progress) progress(len - towrite, len, progress_data)

   CHECK_CANCELLABLE;

   if (!(stream = g_file_replace(file, NULL, make_backup, G_FILE_CREATE_NONE,
                                 cancellable, &error))) {
      g_task_return_error(task, error);
      return;
   }

   for (towrite = len; towrite; towrite -= r) {
      UPDATE_PROGRESS;
      CHECK_CANCELLABLE;

      r = g_output_stream_write(G_OUTPUT_STREAM(stream),
                                data,
                                towrite,
                                cancellable,
                                &error);

      if (r == -1) {
         g_task_return_error(task, error);
         g_output_stream_close(G_OUTPUT_STREAM(stream), NULL, NULL);
         g_object_unref(stream);
         return;
      }
   }

   CHECK_CANCELLABLE;
   UPDATE_PROGRESS;

   if (!g_output_stream_close(G_OUTPUT_STREAM(stream), cancellable, &error)) {
      g_task_return_error(task, error);
      g_object_unref(stream);
      return;
   }

   UPDATE_PROGRESS;

#undef CHECK_CANCELLABLE
#undef UPDATE_PROGRESS

   g_object_unref(stream);

   g_task_return_boolean(task, TRUE);
}

static void
gb_source_document_save_async (GbDocument            *document,
                               GCancellable          *cancellable,
                               GAsyncReadyCallback    callback,
                               gpointer               user_data)
{
   GbSourceDocumentPrivate *priv;
   GbSourceDocument *sdoc = (GbSourceDocument *)document;
   GBytes *bytes;
   GTask *task;
   GFile *file;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(sdoc));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   priv = sdoc->priv;

   task = g_task_new(document, cancellable, callback, user_data);

   file = gb_source_document_get_file_may_block(sdoc);
   if (!file) {
      g_task_return_new_error(task,
                              G_IO_ERROR,
                              G_IO_ERROR_CANCELLED,
                              _("No filename was selected."));
      goto cleanup;
   }

   bytes = gb_source_document_get_text_bytes(sdoc);

   g_task_set_task_data(task, bytes, (GDestroyNotify)g_bytes_unref);

   g_object_set_data_full(G_OBJECT(task), "file", file, g_object_unref);

   g_object_set_data(G_OBJECT(task), "progress",
                     gb_gtk_progress_bar_file_progress_callback);
   g_object_set_data_full(G_OBJECT(task), "progress_data",
                          g_object_ref(priv->progress),
                          g_object_unref);

   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->progress), 0);
   gtk_widget_show(priv->progress);

   g_task_run_in_thread(task, save_thread_func);

cleanup:
   g_object_unref(task);
}

static gboolean
hide_progress_cb (gpointer data)
{
   GbSourceDocument *document = data;

   gtk_widget_hide(document->priv->progress);
   g_object_unref(document);

   return G_SOURCE_REMOVE;
}

static gboolean
gb_source_document_save_finish (GbDocument    *document,
                                GAsyncResult  *result,
                                GError       **error)
{
   GbSourceDocument *sdoc = (GbSourceDocument *)document;
   gboolean ret;
   GTask *task = (GTask *)result;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(sdoc), FALSE);
   g_return_val_if_fail(G_IS_TASK(result), FALSE);
   g_return_val_if_fail(g_task_is_valid(task, document), FALSE);

   g_timeout_add(250, hide_progress_cb, g_object_ref(document));

   if ((ret = g_task_propagate_boolean(task, error))) {
      gtk_text_buffer_set_modified(sdoc->priv->buffer, FALSE);
      guess_and_apply_language(sdoc);
   }

   return ret;
}

void
gb_source_document_load_file_async (GbSourceDocument    *document,
                                    GFile               *file,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data)
{
   GbSourceDocumentPrivate *priv;
   GConverter *decoder = NULL;
   GConverter *decompressor = NULL;
   GTask *task;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));
   g_return_if_fail(!file || G_IS_FILE(file));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   priv = document->priv;

   /*
    * TODO: Guess decompressor and encoder?
    */

   task = gb_buffer_load_task_new(document,
                                  file,
                                  decoder, decompressor,
                                  cancellable,
                                  gb_gtk_progress_bar_file_progress_callback,
                                  priv->progress,
                                  callback,
                                  user_data);

   g_object_unref(task);
}

gboolean
gb_source_document_load_file_finish (GbSourceDocument  *document,
                                     GAsyncResult      *result,
                                     GError           **error)
{
   GbSourceDocumentPrivate *priv;
   const gchar *text;
   GtkTextIter iter;
   GBytes *bytes;
   GTask *task = (GTask *)result;
   guint offset;
   gsize len;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), FALSE);
   g_return_val_if_fail(G_IS_TASK(task), FALSE);

   priv = document->priv;

   if (!(bytes = g_task_propagate_pointer(task, error))) {
      return FALSE;
   }

   text = g_bytes_get_data(bytes, &len);

   if (!g_utf8_validate(text, len, NULL)) {
      g_set_error(error,
                  G_IO_ERROR,
                  G_IO_ERROR_INVALID_DATA,
                  _("Failed to convert document to UTF-8."));
      g_bytes_unref(bytes);
      return FALSE;
   }

   gtk_source_buffer_begin_not_undoable_action(GTK_SOURCE_BUFFER(priv->buffer));
   gtk_text_buffer_set_text(priv->buffer, text, len);
   gtk_text_buffer_set_modified(priv->buffer, FALSE);
   gtk_source_buffer_end_not_undoable_action(GTK_SOURCE_BUFFER(priv->buffer));

   if (priv->file) {
      offset = get_last_edit_known_edit_offset(priv->file);
      gtk_text_buffer_get_iter_at_offset(priv->buffer, &iter, offset);
      gtk_text_buffer_select_range(priv->buffer, &iter, &iter);
   }

   guess_and_apply_language(document);

   g_bytes_unref(bytes);

   return TRUE;
}

static gboolean
is_stop_char (gunichar c)
{
   switch (c) {
   case '_':
      return FALSE;
   case ')':
   case '(':
   case '&':
   case '*':
   case '{':
   case '}':
   case ' ':
   case '\t':
   case '[':
   case ']':
   case '=':
   case '"':
   case '\'':
      return TRUE;
   default:
      return !g_unichar_isalnum(c);
   }
}

gchar *
gb_source_document_get_current_word (GbSourceDocument *document)
{
   GbSourceDocumentPrivate *priv;
   GtkTextMark *mark;
   GtkTextIter iter;
   GtkTextIter begin;
   GtkTextIter end;
   gboolean moved_back = FALSE;
   gboolean moved_forward = FALSE;
   gunichar c;
   gchar *text;

   g_return_val_if_fail(GB_IS_SOURCE_DOCUMENT(document), NULL);

   priv = document->priv;

   mark = gtk_text_buffer_get_insert(priv->buffer);
   gtk_text_buffer_get_iter_at_mark(priv->buffer, &iter, mark);
   gtk_text_iter_assign(&begin, &iter);
   gtk_text_iter_assign(&end, &iter);

   c = gtk_text_iter_get_char(&begin);
   while (gtk_text_iter_backward_char(&begin) &&
          (c = gtk_text_iter_get_char(&begin)) &&
          !is_stop_char(c)) {
      /* Do Nothing */
      moved_back = TRUE;
   }
   if (moved_back && is_stop_char(c)) {
      gtk_text_iter_forward_char(&begin);
   }

   c = gtk_text_iter_get_char(&end);
   while (gtk_text_iter_forward_char(&end) &&
          (c = gtk_text_iter_get_char(&end)) &&
          !is_stop_char(c)) {
      /* Do Nothing */
      moved_forward = TRUE;
   }
   if (moved_forward && is_stop_char(c)) {
      gtk_text_iter_backward_char(&end);
   }

   text = gtk_text_buffer_get_text(priv->buffer, &begin, &end, TRUE);

   return text;
}

static void
update_cursor_position (GbSourceDocument *document,
                        GtkTextBuffer *buffer)
{
   GbSourceDocumentPrivate *priv;
   GtkTextMark *mark;
   GtkTextIter iter;
   gboolean overwrite;
   gchar *text;
   guint line;
   guint column;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));

   priv = document->priv;

   overwrite = gtk_text_view_get_overwrite(GTK_TEXT_VIEW(priv->view));
   mark = gtk_text_buffer_get_insert(buffer);
   gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
   line = gtk_text_iter_get_line(&iter) + 1;
   column = gtk_source_view_get_visual_column(GTK_SOURCE_VIEW(priv->view),
                                              &iter) + 1;
   if (!overwrite) {
      text = g_strdup_printf(_("Ln %u, Col %u  INS"), line, column);
   } else {
      text = g_strdup_printf(_("Ln %u, Col %u  REP"), line, column);
   }
   g_object_set(priv->floating_bar, "label", text, NULL);
   g_free(text);

   if (!gtk_widget_get_visible(priv->floating_bar)) {
      gtk_widget_show(priv->floating_bar);
   }
}

static void
buffer_mark_set_cb (GtkTextBuffer    *buffer,
                    GtkTextIter      *location,
                    GtkTextMark      *mark,
                    GbSourceDocument *document)
{
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   if (mark == gtk_text_buffer_get_insert(buffer)) {
      update_cursor_position(document, buffer);
   }
}

static void
buffer_insert_text_cb (GtkTextBuffer *buffer,
                       GtkTextIter   *location,
                       gchar         *text,
                       gint           length,
                       GbSourceDocument *document)
{
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   update_cursor_position(document, buffer);
}

static void
buffer_delete_range_cb (GtkTextBuffer    *buffer,
                        GtkTextIter      *begin,
                        GtkTextIter      *end,
                        GbSourceDocument *document)
{
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   update_cursor_position(document, buffer);
}

static void
on_toggle_overwite (GtkTextView      *text_view,
                    GbSourceDocument *document)
{
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));
   g_return_if_fail(GTK_IS_TEXT_VIEW(text_view));

   update_cursor_position(document, document->priv->buffer);
}

static void
gb_source_document_grab_focus (GtkWidget *widget)
{
   GbSourceDocument *document = (GbSourceDocument *)widget;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   gtk_widget_grab_focus(document->priv->view);
}

static void
gb_source_document_modified_changed (GbSourceDocument *document,
                                     GtkTextBuffer    *buffer)
{
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));

   g_object_notify_by_pspec(G_OBJECT(document), gParamSpecs[PROP_CAN_SAVE]);
}

static void
gb_source_document_destroy (GtkWidget *widget)
{
   GbSourceDocumentPrivate *priv;
   GbSourceDocument *document = (GbSourceDocument *)widget;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   priv = document->priv;

   g_clear_object(&priv->buffer);
   g_clear_object(&priv->settings);
   g_clear_object(&priv->file);

   GTK_WIDGET_CLASS(gb_source_document_parent_class)->destroy(widget);

   EXIT;
}

static void
gb_source_document_finalize (GObject *object)
{
   GbSourceDocumentPrivate *priv;

   ENTRY;

   priv = GB_SOURCE_DOCUMENT(object)->priv;

   g_clear_object(&priv->buffer);
   g_clear_object(&priv->file);
   g_clear_object(&priv->settings);
   g_clear_pointer(&priv->title, g_free);

   G_OBJECT_CLASS(gb_source_document_parent_class)->finalize(object);

   EXIT;
}

static void
gb_source_document_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
   GbSourceDocument *document = GB_SOURCE_DOCUMENT(object);
   GbDocument *doc = GB_DOCUMENT(object);

   switch (prop_id) {
   case PROP_BUFFER:
      g_value_set_object(value, gb_source_document_get_buffer(document));
      break;
   case PROP_CAN_SAVE:
      g_value_set_boolean(value, gb_source_document_get_can_save(doc));
      break;
   case PROP_FILE:
      g_value_set_object(value, gb_source_document_get_file(document));
      break;
   case PROP_LANGUAGE:
      g_value_set_object(value, gb_source_document_get_language(document));
      break;
   case PROP_STYLE_SCHEME:
      g_value_set_object(value, gb_source_document_get_style_scheme(document));
      break;
   case PROP_TITLE:
      g_value_set_string(value, gb_source_document_get_title(doc));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_document_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
   GbSourceDocument *document = GB_SOURCE_DOCUMENT(object);

   switch (prop_id) {
   case PROP_FILE:
      gb_source_document_set_file(document, g_value_get_object(value));
      break;
   case PROP_LANGUAGE:
      gb_source_document_set_language(document, g_value_get_object(value));
      break;
   case PROP_STYLE_SCHEME:
      gb_source_document_set_style_scheme(document, g_value_get_object(value));
      break;
   case PROP_STYLE_SCHEME_NAME:
      gb_source_document_set_style_scheme_name(document,
                                               g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_document_class_init (GbSourceDocumentClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_document_finalize;
   object_class->get_property = gb_source_document_get_property;
   object_class->set_property = gb_source_document_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceDocumentPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->destroy = gb_source_document_destroy;
   widget_class->grab_focus = gb_source_document_grab_focus;

   gParamSpecs[PROP_BUFFER] =
      g_param_spec_object("buffer",
                          _("Buffer"),
                          _("The text buffer for the document."),
                          GTK_TYPE_TEXT_BUFFER,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_BUFFER,
                                   gParamSpecs[PROP_BUFFER]);

   gParamSpecs[PROP_CAN_SAVE] =
      g_param_spec_boolean("can-save",
                           _("Can Save"),
                           _("If the document can currently be saved."),
                           FALSE,
                           (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_SAVE,
                                   gParamSpecs[PROP_CAN_SAVE]);

   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("The file to save to."),
                          G_TYPE_FILE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);

   gParamSpecs[PROP_LANGUAGE] =
      g_param_spec_object("language",
                          _("Language"),
                          _("The language for the buffer."),
                          GTK_SOURCE_TYPE_LANGUAGE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_LANGUAGE,
                                   gParamSpecs[PROP_LANGUAGE]);

   gParamSpecs[PROP_STYLE_SCHEME] =
      g_param_spec_object("style-scheme",
                          _("Style Scheme"),
                          _("The style scheme for the buffer."),
                          GTK_SOURCE_TYPE_STYLE_SCHEME,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STYLE_SCHEME,
                                   gParamSpecs[PROP_STYLE_SCHEME]);

   gParamSpecs[PROP_STYLE_SCHEME_NAME] =
      g_param_spec_string("style-scheme-name",
                          _("Style Scheme Name"),
                          _("The name of the style scheme."),
                          NULL,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STYLE_SCHEME_NAME,
                                   gParamSpecs[PROP_STYLE_SCHEME_NAME]);

   gParamSpecs[PROP_TITLE] =
      g_param_spec_string("title",
                          _("Title"),
                          _("The title of the document."),
                          NULL,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TITLE,
                                   gParamSpecs[PROP_TITLE]);
}

static void
gb_source_document_init (GbSourceDocument *document)
{
   GbSourceDocumentPrivate *priv;

   document->priv = G_TYPE_INSTANCE_GET_PRIVATE(document,
                                                GB_TYPE_SOURCE_DOCUMENT,
                                                GbSourceDocumentPrivate);

   priv = document->priv;

   priv->buffer = GTK_TEXT_BUFFER(gtk_source_buffer_new(NULL));
   g_object_set(priv->buffer,
                "highlight-matching-brackets", FALSE,
                NULL);
   gtk_text_buffer_set_modified(priv->buffer, FALSE);
   g_signal_connect_object(priv->buffer,
                           "modified-changed",
                           G_CALLBACK(gb_source_document_modified_changed),
                           document,
                           G_CONNECT_SWAPPED);

   priv->overlay = g_object_new(GTK_TYPE_OVERLAY,
                                "visible", TRUE,
                                NULL);
   gtk_container_add(GTK_CONTAINER(document), priv->overlay);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->overlay), priv->scroller);

   priv->view = g_object_new(GB_TYPE_SOURCE_VIEW,
                             "buffer", priv->buffer,
                             "insert-spaces-instead-of-tabs", TRUE,
                             "right-margin-position", 80,
                             "show-line-numbers", TRUE,
                             "show-right-margin", TRUE,
                             "tab-width", 3,
                             "visible", TRUE,
                             NULL);
   g_signal_connect_object(priv->view,
                           "toggle-overwrite",
                           G_CALLBACK(on_toggle_overwite),
                           document,
                           G_CONNECT_AFTER);
   gtk_container_add(GTK_CONTAINER(priv->scroller), priv->view);

   priv->floating_bar = g_object_new(NAUTILUS_TYPE_FLOATING_BAR,
                                     "label", _("Ln 1, Col 1  INS"),
                                     "halign", GTK_ALIGN_END,
                                     "valign", GTK_ALIGN_END,
                                     "visible", TRUE,
                                     NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->floating_bar);

   if (gtk_widget_get_direction(priv->view) == GTK_TEXT_DIR_RTL) {
      gtk_widget_set_halign(priv->floating_bar, GTK_ALIGN_START);
   }

   priv->buffer_insert_text_handler =
      g_signal_connect_object(priv->buffer,
                              "insert-text",
                              G_CALLBACK(buffer_insert_text_cb),
                              document,
                              G_CONNECT_AFTER);

   priv->buffer_delete_range_handler =
      g_signal_connect_object(priv->buffer,
                              "delete-range",
                              G_CALLBACK(buffer_delete_range_cb),
                              document,
                              G_CONNECT_AFTER);

   priv->buffer_mark_set_handler =
      g_signal_connect_object(priv->buffer,
                              "mark-set",
                              G_CALLBACK(buffer_mark_set_cb),
                              document,
                              G_CONNECT_AFTER);

   priv->progress = g_object_new(GTK_TYPE_PROGRESS_BAR,
                                 "halign", GTK_ALIGN_FILL,
                                 "valign", GTK_ALIGN_START,
                                 "vexpand", FALSE,
                                 "visible", FALSE,
                                 NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->progress),
                               GTK_STYLE_CLASS_OSD);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->progress);

   /*
    * Bind application settings to the document.
    */
   priv->settings = g_settings_new("org.gnome.Builder.Editor.SourceDocument");
   g_settings_bind(priv->settings, "style-scheme",
                   document, "style-scheme-name",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "highlight-matching-brackets",
                   priv->buffer, "highlight-matching-brackets",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "highlight-current-line",
                   priv->view, "highlight-current-line",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "insert-spaces-instead-of-tabs",
                   priv->view, "insert-spaces-instead-of-tabs",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "right-margin-position",
                   priv->view, "right-margin-position",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "show-line-marks",
                   priv->view, "show-line-marks",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "show-right-margin",
                   priv->view, "show-right-margin",
                   G_SETTINGS_BIND_GET);
   g_settings_bind(priv->settings, "tab-width",
                   priv->view, "tab-width",
                   G_SETTINGS_BIND_GET);
}

static void
gb_document_init (GbDocumentInterface *klass)
{
   klass->get_can_save = gb_source_document_get_can_save;
   klass->get_title = gb_source_document_get_title;
   klass->save_async = gb_source_document_save_async;
   klass->save_finish = gb_source_document_save_finish;
}

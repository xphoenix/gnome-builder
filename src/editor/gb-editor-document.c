/* gb-editor-document.c
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

#include "gb-editor-document.h"
#include "gb-buffer-tasks.h"

struct _GbEditorDocumentPrivate
{
   GbEditorDocumentState state;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE(GbEditorDocument,
                        gb_editor_document,
                        GTK_SOURCE_TYPE_BUFFER,
                        G_ADD_PRIVATE(GbEditorDocument))

gboolean
gb_editor_document_load_from_file_finish (GbEditorDocument  *document,
                                          GAsyncResult      *result,
                                          GError           **error)
{
   GBytes *bytes;
   GTask *task = (GTask *)result;

   g_return_val_if_fail (GB_IS_EDITOR_DOCUMENT(document), FALSE);
   g_return_val_if_fail (G_IS_TASK(task), FALSE);

   if ((bytes = g_task_propagate_pointer(task, error))) {
      gtk_source_buffer_begin_not_undoable_action(GTK_SOURCE_BUFFER(document));
      gtk_text_buffer_set_text(GTK_TEXT_BUFFER(document),
                               (const gchar *)g_bytes_get_data(bytes, NULL),
                               g_bytes_get_size(bytes));
      gtk_source_buffer_end_not_undoable_action(GTK_SOURCE_BUFFER(document));
      g_bytes_unref(bytes);
   }

   return !!bytes;
}

void
gb_editor_document_load_from_file_async (GbEditorDocument      *document,
                                         GFile                 *file,
                                         GCancellable          *cancellable,
                                         GFileProgressCallback  progress_callback,
                                         gpointer               progress_data,
                                         GAsyncReadyCallback    callback,
                                         gpointer               user_data)
{
   GTask *task;

   g_return_if_fail(GB_IS_EDITOR_DOCUMENT(document));
   g_return_if_fail(G_IS_FILE(file));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   task = gb_buffer_load_task_new (document, file, NULL, NULL, cancellable,
                                   progress_callback, progress_data, callback,
                                   user_data);

   g_object_unref (task);
}

void
gb_editor_document_save_to_file_async (GbEditorDocument      *document,
                                       GFile                 *file,
                                       GCancellable          *cancellable,
                                       GFileProgressCallback  progress_callback,
                                       gpointer               progress_data,
                                       GAsyncReadyCallback    callback,
                                       gpointer               user_data)
{
   GTask *task;

   g_return_if_fail (GB_IS_EDITOR_DOCUMENT(document));
   g_return_if_fail (G_IS_FILE(file));
   g_return_if_fail (!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail (callback);

   task = gb_buffer_save_task_new (document, file, NULL, NULL, cancellable,
                                   progress_callback, progress_data,
                                   callback, user_data);

   g_object_unref (task);
}

gboolean
gb_editor_document_save_to_file_finish (GbEditorDocument  *document,
                                        GAsyncResult      *result,
                                        GError           **error)
{
   GTask *task = (GTask *)result;

   g_return_val_if_fail (GB_IS_EDITOR_DOCUMENT(document), FALSE);
   g_return_val_if_fail (G_IS_TASK(task), FALSE);

   return g_task_propagate_boolean (task, error);
}

GbEditorDocument *
gb_editor_document_new (void)
{
   return g_object_new(GB_TYPE_EDITOR_DOCUMENT, NULL);
}

static void
gb_editor_document_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_editor_document_parent_class)->finalize(object);
}

static void
gb_editor_document_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
   //GbEditorDocument *document = GB_EDITOR_DOCUMENT(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_editor_document_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
   //GbEditorDocument *document = GB_EDITOR_DOCUMENT(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_editor_document_class_init (GbEditorDocumentClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_editor_document_finalize;
   object_class->get_property = gb_editor_document_get_property;
   object_class->set_property = gb_editor_document_set_property;
}

static void
gb_editor_document_init (GbEditorDocument *document)
{
   document->priv = gb_editor_document_get_instance_private(document);
}

GQuark
gb_editor_document_error_quark (void)
{
   return g_quark_from_static_string("gb-editor-document-error");
}

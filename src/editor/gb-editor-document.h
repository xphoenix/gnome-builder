/* gb-editor-document.h
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

#ifndef GB_EDITOR_DOCUMENT_H
#define GB_EDITOR_DOCUMENT_H

#include <gtksourceview/gtksourcebuffer.h>

G_BEGIN_DECLS

#define GB_EDITOR_DOCUMENT_ERROR           (gb_editor_document_error_quark())
#define GB_TYPE_EDITOR_DOCUMENT            (gb_editor_document_get_type())
#define GB_EDITOR_DOCUMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_EDITOR_DOCUMENT, GbEditorDocument))
#define GB_EDITOR_DOCUMENT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_EDITOR_DOCUMENT, GbEditorDocument const))
#define GB_EDITOR_DOCUMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_EDITOR_DOCUMENT, GbEditorDocumentClass))
#define GB_IS_EDITOR_DOCUMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_EDITOR_DOCUMENT))
#define GB_IS_EDITOR_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_EDITOR_DOCUMENT))
#define GB_EDITOR_DOCUMENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_EDITOR_DOCUMENT, GbEditorDocumentClass))

typedef struct _GbEditorDocument        GbEditorDocument;
typedef struct _GbEditorDocumentClass   GbEditorDocumentClass;
typedef enum   _GbEditorDocumentError   GbEditorDocumentError;
typedef struct _GbEditorDocumentPrivate GbEditorDocumentPrivate;
typedef enum   _GbEditorDocumentState   GbEditorDocumentState;

struct _GbEditorDocument
{
   GtkSourceBuffer parent;

   /*< private >*/
   GbEditorDocumentPrivate *priv;
};

struct _GbEditorDocumentClass
{
   GtkSourceBufferClass parent_class;
};

enum _GbEditorDocumentState
{
   GB_EDITOR_STATE_READY,
   GB_EDITOR_STATE_LOADING,
   GB_EDITOR_STATE_SAVING,
};

enum _GbEditorDocumentError
{
   GB_EDITOR_DOCUMENT_ERROR_BUSY,
};

GQuark            gb_editor_document_error_quark           (void) G_GNUC_CONST;
GType             gb_editor_document_get_type              (void) G_GNUC_CONST;
void              gb_editor_document_load_from_file_async  (GbEditorDocument       *document,
                                                            GFile                  *file,
                                                            GCancellable           *cancellable,
                                                            GFileProgressCallback   progress_callback,
                                                            gpointer                progress_data,
                                                            GAsyncReadyCallback     callback,
                                                            gpointer                user_data);
gboolean          gb_editor_document_load_from_file_finish (GbEditorDocument       *document,
                                                            GAsyncResult           *result,
                                                            GError                **error);
void              gb_editor_document_save_to_file_async    (GbEditorDocument       *document,
                                                            GFile                  *file,
                                                            GCancellable           *cancellable,
                                                            GFileProgressCallback   progress_callback,
                                                            gpointer                progress_data,
                                                            GAsyncReadyCallback     callback,
                                                            gpointer                user_data);
gboolean          gb_editor_document_save_to_file_finish   (GbEditorDocument       *document,
                                                            GAsyncResult           *result,
                                                            GError                **error);
GbEditorDocument *gb_editor_document_new                   (void);

G_END_DECLS

#endif /* GB_EDITOR_DOCUMENT_H */

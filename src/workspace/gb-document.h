/* gb-document.h
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

#ifndef GB_DOCUMENT_H
#define GB_DOCUMENT_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define GB_TYPE_DOCUMENT             (gb_document_get_type())
#define GB_DOCUMENT(o)               (G_TYPE_CHECK_INSTANCE_CAST((o),    GB_TYPE_DOCUMENT, GbDocument))
#define GB_IS_DOCUMENT(o)            (G_TYPE_CHECK_INSTANCE_TYPE((o),    GB_TYPE_DOCUMENT))
#define GB_DOCUMENT_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE((o), GB_TYPE_DOCUMENT, GbDocumentInterface))

typedef struct _GbDocument          GbDocument;
typedef struct _GbDocumentInterface GbDocumentInterface;

struct _GbDocumentInterface
{
   GTypeInterface parent;

   gboolean     (*get_can_save)   (GbDocument             *document);
   const gchar *(*get_title)      (GbDocument             *document);
   void         (*save_async)     (GbDocument             *document,
                                   GCancellable           *cancellable,
                                   GAsyncReadyCallback     callback,
                                   gpointer                user_data);
   gboolean     (*save_finish)    (GbDocument             *document,
                                   GAsyncResult           *result,
                                   GError                **error);
   void         (*save_as_async)  (GbDocument             *document,
                                   GFile                  *file,
                                   GCancellable           *cancellable,
                                   GAsyncReadyCallback     callback,
                                   gpointer                user_data);
   gboolean     (*save_as_finish) (GbDocument             *document,
                                   GAsyncResult           *result,
                                   GError                **error);

};

void         gb_document_emit_changed   (GbDocument             *document);
GType        gb_document_get_type       (void) G_GNUC_CONST;
gboolean     gb_document_get_can_save   (GbDocument             *document);
const gchar *gb_document_get_title      (GbDocument             *document);
void         gb_document_save_async     (GbDocument             *document,
                                         GCancellable           *cancellable,
                                         GAsyncReadyCallback     callback,
                                         gpointer                user_data);
gboolean     gb_document_save_finish    (GbDocument             *document,
                                         GAsyncResult           *result,
                                         GError                **error);
void         gb_document_save_as_async  (GbDocument             *document,
                                         GFile                  *file,
                                         GCancellable           *cancellable,
                                         GAsyncReadyCallback     callback,
                                         gpointer                user_data);
gboolean    gb_document_save_as_finish  (GbDocument             *document,
                                         GAsyncResult           *result,
                                         GError                **error);

G_END_DECLS

#endif /* GB_DOCUMENT_H */

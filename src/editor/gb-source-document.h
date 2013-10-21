/* gb-source-document.h
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

#ifndef GB_SOURCE_DOCUMENT_H
#define GB_SOURCE_DOCUMENT_H

#include <gtk/gtk.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcestylescheme.h>

#include "gb-document.h"

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_DOCUMENT            (gb_source_document_get_type())
#define GB_SOURCE_DOCUMENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_DOCUMENT, GbSourceDocument))
#define GB_SOURCE_DOCUMENT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_DOCUMENT, GbSourceDocument const))
#define GB_SOURCE_DOCUMENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_DOCUMENT, GbSourceDocumentClass))
#define GB_IS_SOURCE_DOCUMENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_DOCUMENT))
#define GB_IS_SOURCE_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_DOCUMENT))
#define GB_SOURCE_DOCUMENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_DOCUMENT, GbSourceDocumentClass))

typedef struct _GbSourceDocument        GbSourceDocument;
typedef struct _GbSourceDocumentClass   GbSourceDocumentClass;
typedef struct _GbSourceDocumentPrivate GbSourceDocumentPrivate;

struct _GbSourceDocument
{
   GtkBin parent;

   /*< private >*/
   GbSourceDocumentPrivate *priv;
};

struct _GbSourceDocumentClass
{
   GtkBinClass parent_class;
};

GType                 gb_source_document_get_type          (void) G_GNUC_CONST;
void                  gb_source_document_load_file_async   (GbSourceDocument       *document,
                                                            GFile                  *file,
                                                            GCancellable           *cancellable,
                                                            GAsyncReadyCallback     callback,
                                                            gpointer                user_data);
gboolean              gb_source_document_load_file_finish  (GbSourceDocument       *document,
                                                            GAsyncResult           *result,
                                                            GError                **error);
GtkSourceLanguage    *gb_source_document_get_language      (GbSourceDocument       *document);
void                  gb_source_document_set_language      (GbSourceDocument       *document,
                                                            GtkSourceLanguage      *language);
GtkSourceStyleScheme *gb_source_document_get_style_scheme  (GbSourceDocument       *document);
void                  gb_source_document_set_style_scheme  (GbSourceDocument       *document,
                                                            GtkSourceStyleScheme   *style_scheme);
gchar                 *gb_source_document_get_current_word (GbSourceDocument       *document);

G_END_DECLS

#endif /* GB_SOURCE_DOCUMENT_H */

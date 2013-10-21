/* gb-document.c
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

#include "gb-document.h"

G_DEFINE_INTERFACE(GbDocument, gb_document, G_TYPE_OBJECT)

enum
{
   CHANGED,
   LAST_SIGNAL
};

static guint gSignals[LAST_SIGNAL];

gboolean
gb_document_get_can_save (GbDocument *document)
{
   g_return_val_if_fail(GB_IS_DOCUMENT(document), FALSE);

   return GB_DOCUMENT_GET_INTERFACE(document)->get_can_save(document);
}

const gchar *
gb_document_get_title (GbDocument *document)
{
   g_return_val_if_fail(GB_IS_DOCUMENT(document), NULL);

   return GB_DOCUMENT_GET_INTERFACE(document)->get_title(document);
}

void
gb_document_save_async (GbDocument            *document,
                        GCancellable          *cancellable,
                        GAsyncReadyCallback    callback,
                        gpointer               user_data)
{
   g_return_if_fail(GB_IS_DOCUMENT(document));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   GB_DOCUMENT_GET_INTERFACE(document)->
      save_async(document, cancellable, callback, user_data);
}

gboolean
gb_document_save_finish (GbDocument    *document,
                         GAsyncResult  *result,
                         GError       **error)
{
   g_return_val_if_fail(GB_IS_DOCUMENT(document), FALSE);
   g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

   return GB_DOCUMENT_GET_INTERFACE(document)->
      save_finish(document, result, error);
}

void
gb_document_save_as_async (GbDocument            *document,
                           GFile                 *file,
                           GCancellable          *cancellable,
                           GAsyncReadyCallback    callback,
                           gpointer               user_data)
{
   g_return_if_fail(GB_IS_DOCUMENT(document));
   g_return_if_fail(G_IS_FILE(file));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   GB_DOCUMENT_GET_INTERFACE(document)->
      save_as_async(document, file, cancellable, callback, user_data);
}

gboolean
gb_document_save_as_finish (GbDocument    *document,
                            GAsyncResult  *result,
                            GError       **error)
{
   g_return_val_if_fail(GB_IS_DOCUMENT(document), FALSE);

   return GB_DOCUMENT_GET_INTERFACE(document)->
      save_as_finish(document, result, error);
}

void
gb_document_emit_changed (GbDocument *document)
{
   g_return_if_fail(GB_IS_DOCUMENT(document));

   g_signal_emit(document, gSignals[CHANGED], 0);
}

static void
gb_document_default_init (GbDocumentInterface *klass)
{
   g_object_interface_install_property(klass,
                                       g_param_spec_boolean("can-save",
                                                            _("Can Save"),
                                                            _("If the document can currently be saved."),
                                                            FALSE,
                                                            (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

   g_object_interface_install_property(klass,
                                       g_param_spec_string("title",
                                                           _("Title"),
                                                           _("The title of the document."),
                                                           NULL,
                                                           (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS)));

   gSignals[CHANGED] = g_signal_new("changed",
                                    GB_TYPE_DOCUMENT,
                                    G_SIGNAL_RUN_LAST,
                                    0,
                                    NULL,
                                    NULL,
                                    g_cclosure_marshal_VOID__VOID,
                                    G_TYPE_NONE,
                                    0);
}

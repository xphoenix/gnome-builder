/* gb-source-snippets.c
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
#include <string.h>

#include "gb-source-snippet-chunk.h"
#include "gb-source-snippets.h"
#include "snippet-parser.h"

G_DEFINE_TYPE(GbSourceSnippets, gb_source_snippets, G_TYPE_OBJECT)

struct _GbSourceSnippetsPrivate
{
   GHashTable *snippets;
};

GbSourceSnippets *
gb_source_snippets_new (void)
{
   return g_object_new(GB_TYPE_SOURCE_SNIPPETS,
                       NULL);
}

void
gb_source_snippets_clear (GbSourceSnippets *snippets)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPETS(snippets));
   g_hash_table_remove_all(snippets->priv->snippets);
}

static void
on_snippet_parsed (GbSourceSnippet *snippet,
                   gpointer         user_data)
{
   GbSourceSnippets *snippets = user_data;

   g_assert(GB_IS_SOURCE_SNIPPETS(snippets));
   g_assert(GB_IS_SOURCE_SNIPPET(snippet));

   gb_source_snippets_add(snippets, snippet);
}

static gboolean
load_from_stream (GbSourceSnippets  *snippets,
                  GDataInputStream  *stream,
                  GError           **error)
{
   SnippetParser *parser;
   gboolean ret = TRUE;
   GError *local_error = NULL;
   gchar *line;
   gsize length;

   g_assert(GB_IS_SOURCE_SNIPPETS(snippets));
   g_assert(G_IS_DATA_INPUT_STREAM(stream));

   parser = snippet_parser_new(on_snippet_parsed, snippets);

   while ((line = g_data_input_stream_read_line(stream,
                                                &length,
                                                NULL,
                                                &local_error))) {
      snippet_parser_feed_line(parser, line);
      g_free(line);
   }

   if (local_error) {
      g_propagate_error(error, local_error);
      ret = FALSE;
   }

   if (ret) {
      snippet_parser_finish(parser);
   }

   snippet_parser_free(parser);

   return ret;
}

gboolean
gb_source_snippets_load_from_stream (GbSourceSnippets  *snippets,
                                     GInputStream      *stream,
                                     GError           **error)
{
   GDataInputStream *data_stream;
   gboolean ret;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPETS(snippets), FALSE);
   g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);

   data_stream = g_data_input_stream_new(stream);
   ret = load_from_stream(snippets, data_stream, error);
   g_object_unref(data_stream);

   return ret;
}

gboolean
gb_source_snippets_load_from_file (GbSourceSnippets  *snippets,
                                   GFile             *file,
                                   GError           **error)
{
   GFileInputStream *file_stream;
   gboolean ret = FALSE;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPETS(snippets), FALSE);
   g_return_val_if_fail(G_IS_FILE(file), FALSE);

   if ((file_stream = g_file_read(file, NULL, error))) {
      ret = gb_source_snippets_load_from_stream(snippets,
                                                G_INPUT_STREAM(file_stream),
                                                error);
      g_object_unref(file_stream);
   }

   return ret;
}

void
gb_source_snippets_add (GbSourceSnippets *snippets,
                        GbSourceSnippet  *snippet)
{
   const gchar *trigger;

   g_return_if_fail(GB_IS_SOURCE_SNIPPETS(snippets));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   trigger = gb_source_snippet_get_trigger(snippet);
   g_hash_table_insert(snippets->priv->snippets,
                       g_strdup(trigger),
                       g_object_ref(snippet));
}

void
gb_source_snippets_foreach (GbSourceSnippets *snippets,
                            GFunc             foreach_func,
                            gpointer          user_data)
{
   GHashTableIter iter;
   gpointer key;
   gpointer value;

   g_hash_table_iter_init(&iter, snippets->priv->snippets);
   while (g_hash_table_iter_next(&iter, &key, &value)) {
      foreach_func(value, user_data);
   }
}

static void
gb_source_snippets_finalize (GObject *object)
{
   GbSourceSnippetsPrivate *priv;

   priv = GB_SOURCE_SNIPPETS(object)->priv;

   g_clear_pointer(&priv->snippets, g_hash_table_unref);

   G_OBJECT_CLASS(gb_source_snippets_parent_class)->finalize(object);
}

static void
gb_source_snippets_class_init (GbSourceSnippetsClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_snippets_finalize;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetsPrivate));
}

static void
gb_source_snippets_init (GbSourceSnippets *snippets)
{
   snippets->priv = G_TYPE_INSTANCE_GET_PRIVATE(snippets,
                                                GB_TYPE_SOURCE_SNIPPETS,
                                                GbSourceSnippetsPrivate);

   snippets->priv->snippets =
      g_hash_table_new_full(g_str_hash,
                            g_str_equal,
                            g_free,
                            g_object_unref);
}

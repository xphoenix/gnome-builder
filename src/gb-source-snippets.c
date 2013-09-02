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

#include "gb-source-snippets.h"

G_DEFINE_TYPE(GbSourceSnippets, gb_source_snippets, G_TYPE_OBJECT)

struct _GbSourceSnippetsPrivate
{
   GHashTable *snippets;
};

enum
{
   PROP_0,
   LAST_PROP
};

GbSourceSnippets *
gb_source_snippets_new (void)
{
   return g_object_new(GB_TYPE_SOURCE_SNIPPETS,
                       NULL);
}

void
gb_source_snippets_add (GbSourceSnippets *snippets,
                        GbSourceSnippet  *snippet)
{
   gchar *key;

   key = g_strdup(gb_source_snippet_get_trigger(snippet));
   g_hash_table_insert(snippets->priv->snippets, key, g_object_ref(snippet));
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

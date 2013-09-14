/* fuzzy.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 * 
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <ctype.h>
#include <string.h>

#include "fuzzy.h"


typedef struct _FuzzyItem   FuzzyItem;
typedef struct _FuzzyLookup FuzzyLookup;


struct _Fuzzy
{
   GStringChunk *chunk;
   GPtrArray    *id_to_text;
   GPtrArray    *char_tables;
   gboolean      in_bulk_insert;
};


struct _FuzzyItem
{
   guint id  : 20;
   guint pos : 12;
};


G_STATIC_ASSERT(sizeof(FuzzyItem) == 4);


struct _FuzzyLookup
{
   Fuzzy        *fuzzy;
   GArray      **tables;
   gint         *state;
   guint         n_tables;
   gsize         max_matches;
   const gchar  *needle;
   GHashTable   *matches;
};


static gint
fuzzy_item_compare (gconstpointer a,
                    gconstpointer b)
{
   gint ret;

   const FuzzyItem *fa = a;
   const FuzzyItem *fb = b;

   if ((ret = fa->id - fb->id) == 0) {
      ret = fa->pos - fb->pos;
   }

   return ret;
}


static gint
fuzzy_match_compare (gconstpointer a,
                     gconstpointer b)
{
   const FuzzyMatch *ma = a;
   const FuzzyMatch *mb = b;

   if (ma->score < mb->score) {
      return 1;
   } else if (ma->score > mb->score) {
      return -1;
   }

   return g_strcmp0(ma->text, mb->text);
}


/**
 * fuzzy_new:
 *
 * Create a new #Fuzzy for fuzzy matching strings.
 *
 * Returns: A newly allocated #Fuzzy that should be freed with fuzzy_free().
 */
Fuzzy *
fuzzy_new (void)
{
   GArray *table;
   Fuzzy *fuzzy;
   gint i;

   fuzzy = g_new0(Fuzzy, 1);
   fuzzy->chunk = g_string_chunk_new(4096);
   fuzzy->id_to_text = g_ptr_array_new();
   fuzzy->char_tables = g_ptr_array_new();
   g_ptr_array_set_free_func(fuzzy->char_tables,
                             (GDestroyNotify)g_array_unref);

   for (i = 0; i < 256; i++) {
      table = g_array_new(FALSE, FALSE, sizeof(FuzzyItem));
      g_ptr_array_add(fuzzy->char_tables, table);
   }

   return fuzzy;
}


/**
 * fuzzy_begin_bulk_insert:
 * @fuzzy: (in): A #Fuzzy.
 *
 * Start a bulk insertion. @fuzzy is not ready for searching until
 * fuzzy_end_bulk_insert() has been called.
 *
 * This allows for inserting large numbers of strings and deferring
 * the final sort until fuzzy_end_bulk_insert().
 */
void
fuzzy_begin_bulk_insert (Fuzzy *fuzzy)
{
   g_return_if_fail(fuzzy);
   g_return_if_fail(!fuzzy->in_bulk_insert);

   fuzzy->in_bulk_insert = TRUE;
}


/**
 * fuzzy_end_bulk_insert:
 * @fuzzy: (in): A #Fuzzy.
 *
 * Complete a bulk insert and resort the index.
 */
void
fuzzy_end_bulk_insert (Fuzzy *fuzzy)
{
   GArray *table;
   gint i;

   g_return_if_fail(fuzzy);
   g_return_if_fail(fuzzy->in_bulk_insert);

   fuzzy->in_bulk_insert = FALSE;

   for (i = 0; i < fuzzy->char_tables->len; i++) {
      table = g_ptr_array_index(fuzzy->char_tables, i);
      g_array_sort(table, fuzzy_item_compare);
   }
}


/**
 * fuzzy_insert:
 * @fuzzy: (in): A #Fuzzy.
 * @text: (in): An ASCII string.
 *
 * Inserts a string into the fuzzy matcher.
 *
 * Note that @text MUST be an ascii string. UTF-8 is not supported.
 */
void
fuzzy_insert (Fuzzy       *fuzzy,
              const gchar *text)
{
   FuzzyItem item;
   GArray *table;
   guint idx;
   gint id;
   gint i;

   g_return_if_fail(fuzzy);
   g_return_if_fail(text);
   g_return_if_fail(fuzzy->id_to_text->len < ((1 << 20) - 1));

   if (!*text) {
      return;
   }

   g_ptr_array_add(fuzzy->id_to_text,
                   g_string_chunk_insert(fuzzy->chunk, text));

   id = fuzzy->id_to_text->len - 1;

   for (i = 0; text[i]; i++) {
      idx = text[i];
      table = g_ptr_array_index(fuzzy->char_tables, idx);

      item.id = id;
      item.pos = i;
      g_array_append_val(table, item);

      if (!fuzzy->in_bulk_insert) {
         g_array_sort(table, fuzzy_item_compare);
      }
   }
}


/**
 * fuzzy_free:
 * @fuzzy: (allow-none): A #Fuzzy.
 *
 * Frees resources associated with #Fuzzy. @fuzzy must not be used
 * after calling this.
 */
void
fuzzy_free (Fuzzy *fuzzy)
{
   if (fuzzy) {
      g_string_chunk_free(fuzzy->chunk);
      fuzzy->chunk = NULL;

      g_ptr_array_unref(fuzzy->id_to_text);
      fuzzy->id_to_text = NULL;

      g_ptr_array_unref(fuzzy->char_tables);
      fuzzy->char_tables = NULL;

      g_free(fuzzy);
   }
}


static gboolean
fuzzy_do_match (FuzzyLookup *lookup,
                FuzzyItem   *item,
                gint         table_index,
                gint         score)
{
   FuzzyItem *iter;
   gpointer key;
   GArray *table;
   gint *state;
   gint iter_score;

   g_assert(lookup);
   g_assert(item);
   g_assert(table_index);

   table = lookup->tables[table_index];
   state = &lookup->state[table_index];

   for (; state[0] < table->len; state[0]++) {
      iter = &g_array_index(table, FuzzyItem, state[0]);

      if ((iter->id < item->id) ||
          ((iter->id == item->id) && (iter->pos <= item->pos))) {
         continue;
      } else if (iter->id > item->id) {
         break;
      }

      iter_score = score + (iter->pos - item->pos);

      if ((table_index + 1) < lookup->n_tables) {
         if (fuzzy_do_match(lookup, iter, table_index + 1, iter_score)) {
            return TRUE;
         }
         continue;
      }

      key = GINT_TO_POINTER(iter->id);

      if (!g_hash_table_contains(lookup->matches, key) ||
          (iter_score < GPOINTER_TO_INT(g_hash_table_lookup(lookup->matches, key)))) {
         g_hash_table_insert(lookup->matches, key, GINT_TO_POINTER(iter_score));
      }

      return TRUE;
   }

   return FALSE;
}


/**
 * fuzzy_match:
 * @fuzzy: (in): A #Fuzzy.
 * @needle: (in): The needle to fuzzy search for.
 * @max_matches: (in): The max number of matches to return.
 *
 * Fuzzy searches within @fuzzy for strings that fuzzy match @needle.
 * Only up to @max_matches will be returned.
 *
 * @needle MUST be an ascii string.
 *
 * TODO: max_matches is not yet respected.
 *
 * Returns: (transfer full) (element-type FuzzyMatch): A newly allocated
 *   #GArray containing #FuzzyMatch elements. This should be freed when
 *   the caller is done with it using g_array_unref().
 *   It is a programming error to keep the structure around longer than
 *   the @fuzzy instance.
 */
GArray *
fuzzy_match (Fuzzy       *fuzzy,
             const gchar *needle,
             gsize        max_matches)
{
   FuzzyLookup lookup = { 0 };
   FuzzyMatch match;
   FuzzyItem *item;
   GArray *matches = NULL;
   GArray *root;
   gint i;

   g_return_val_if_fail(fuzzy, NULL);
   g_return_val_if_fail(!fuzzy->in_bulk_insert, NULL);
   g_return_val_if_fail(needle, NULL);

   matches = g_array_new(FALSE, FALSE, sizeof(FuzzyMatch));

   if (!*needle) {
      return matches;
   }

   lookup.fuzzy = fuzzy;
   lookup.n_tables = strlen(needle);
   lookup.state = g_new0(gint, lookup.n_tables);
   lookup.tables = g_new0(GArray*, lookup.n_tables);
   lookup.needle = needle;
   lookup.max_matches = max_matches;
   lookup.matches = g_hash_table_new(NULL, NULL);

   for (i = 0; needle[i]; i++) {
      lookup.tables[i] = g_ptr_array_index(fuzzy->char_tables,
                                           (guint)needle[i]);
   }

   root = g_ptr_array_index(fuzzy->char_tables, (guint)needle[0]);

   if (G_LIKELY(lookup.n_tables > 1)) {
      for (i = 0; i < root->len; i++) {
         item = &g_array_index(root, FuzzyItem, i);
         fuzzy_do_match(&lookup, item, 1, 0);
      }
   } else {
      for (i = 0; i < root->len; i++) {
         item = &g_array_index(root, FuzzyItem, i);
         match.text = g_ptr_array_index(fuzzy->id_to_text, item->id);
         match.score = 0;
         g_array_append_val(matches, match);
      }
      return matches;
   }

   {
      GHashTableIter iter;
      gpointer key;
      gpointer value;

      g_hash_table_iter_init(&iter, lookup.matches);
      while (g_hash_table_iter_next(&iter, &key, &value)) {
         match.text = g_ptr_array_index(fuzzy->id_to_text,
                                        GPOINTER_TO_INT(key));
         match.score = 1.0 / (strlen(match.text) + GPOINTER_TO_INT(value));
         g_array_append_val(matches, match);
      }

      g_array_sort(matches, fuzzy_match_compare);

      /*
       * TODO: We could be more clever here when inserting into the array
       *       only if it is a lower score than the end or < max items.
       */

      if (max_matches && (matches->len > max_matches)) {
         g_array_set_size(matches, max_matches);
      }
   }

   g_free(lookup.state);
   g_free(lookup.tables);
   g_hash_table_unref(lookup.matches);

   return matches;
}

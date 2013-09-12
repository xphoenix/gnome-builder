/* gb-search-fuzzy.c
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

#include <string.h>

#include "gb-search-fuzzy.h"

/*
 * NOTE: This only uses a class in case we want to add various
 *       parameters later to the algorithm.
 */

G_DEFINE_TYPE(GbSearchFuzzy, gb_search_fuzzy, G_TYPE_OBJECT)

GbSearchFuzzy *
gb_search_fuzzy_new (void)
{
   return g_object_new(GB_TYPE_SEARCH_FUZZY, NULL);
}

static void
_match_highlight_score (const gchar *haystack,
                        const gchar *haystack_casefold,
                        glong        haystack_chars,
                        const gchar *needle_casefold,
                        glong        needle_chars,
                        GString     *str,
                        gfloat      *score)
{
   const gchar *tmp;
   gfloat real_needle_chars = needle_chars;
   gchar *slice;
   gint slice_len;
   gint off;
   gint i;
   gint longest_run = 0;
   gint first_run = -1;

   g_assert(haystack);
   g_assert(haystack_casefold);
   g_assert(needle_casefold);
   g_assert(needle_chars);
   g_assert(str);

again:
   for (i = needle_chars; i > 0; i--) {
      slice = g_utf8_substring(needle_casefold, 0, i);

      if (!(tmp = strstr(haystack_casefold, slice))) {
         g_free(slice);
         continue;
      }

      off = (tmp - haystack_casefold);
      slice_len = strlen(slice);

      g_string_append_len(str, haystack, off);
      g_string_append(str, "<span color='#2e6bbf'>");
      g_string_append_len(str, haystack + off, slice_len);
      g_string_append_len(str, "</span>", 7);

      haystack += off + slice_len;
      haystack_casefold += off + slice_len;

      needle_casefold += slice_len;
      needle_chars = (needle_chars - i);

      longest_run = MAX(longest_run, i);

      if (first_run == -1) {
         first_run = i;
      }

      g_free(slice);

      goto again;
   }

   if (*needle_casefold) {
      *score = 0.0;
      return;
   }

   g_string_append(str, haystack);

   /*
    * Handle degenerate cases.
    */
   if (first_run == -1) {
      *score = 0.0;
      return;
   } else if (first_run == haystack_chars) {
      *score = 1.0;
      return;
   }

   /*
    * TODO: The following could use lots of tricks to catch matches after
    *       '_' as more important, etc.
    */

   /*
    * Start by scoring using our longest run of text in relation
    * to the input text.
    */
   *score = (longest_run / real_needle_chars);

   /*
    * Adjust the score by string length so shorter strings are better.
    */
   *score *= 0.7 + (real_needle_chars / (gfloat)haystack_chars * 0.3);
}

static gchar *
match_highlight_score (const gchar *haystack,
                       const gchar *needle,
                       gfloat      *score)
{
   GString *str;
   gchar *haystack_casefold;
   gchar *needle_casefold;

   g_return_val_if_fail(haystack, NULL);
   g_return_val_if_fail(needle, NULL);
   g_return_val_if_fail(score, NULL);

   *score = 0.0;

   str = g_string_new(NULL);
   haystack_casefold = g_utf8_casefold(haystack, -1);
   needle_casefold = g_utf8_casefold(needle, -1);

   _match_highlight_score(haystack,
                          haystack_casefold,
                          g_utf8_strlen(haystack_casefold, -1),
                          needle_casefold,
                          g_utf8_strlen(needle_casefold, -1),
                          str,
                          score);

   g_free(haystack_casefold);
   g_free(needle_casefold);

   return g_string_free(str, (*score == 0.0));
}

gchar *
gb_search_fuzzy_match_score_highlight (GbSearchFuzzy *fuzzy,
                                       const gchar   *haystack,
                                       const gchar   *needle,
                                       gfloat        *score)
{
   return match_highlight_score(haystack, needle, score);
}

static void
gb_search_fuzzy_class_init (GbSearchFuzzyClass *klass)
{
}

static void
gb_search_fuzzy_init (GbSearchFuzzy *fuzzy)
{
}

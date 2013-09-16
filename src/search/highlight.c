/* highlight.c
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

#include "highlight.h"

gchar *
highlight_substrings (const gchar *haystack,
                      const gchar *needle,
                      const gchar *prefix,
                      const gchar *suffix)
{
   GString *str;
   gint len;

   g_return_val_if_fail(haystack, NULL);
   g_return_val_if_fail(needle, NULL);

   str = g_string_new(NULL);

   /*
    * TODO: Be smarter about this and do runs of text within prefix/suffix.
    *       This does it for every match which is pretty lame.
    */

   for (; *haystack; haystack++) {
      if (*needle == *haystack) {
         g_string_append(str, prefix);
         g_string_append_c(str, *haystack);
         g_string_append(str, suffix);
         needle++;
      } else {
         g_string_append_c(str, *haystack);
      }
   }

   return g_string_free(str, FALSE);
}

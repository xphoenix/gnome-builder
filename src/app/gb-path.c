/* gb-path.c
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

#include "gb-path.h"

static GHashTable *gSuffixToIcon;

const gchar *
gb_path_get_icon_name (const gchar *path)
{
   static gsize initialized;
   const gchar *icon_name = NULL;
   const gchar *suffix;
   gchar **parts;
   gchar *name;
   gint length;

   if (g_once_init_enter(&initialized)) {
      gSuffixToIcon = g_hash_table_new_full(g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            g_free);

#define ADD_ICON_FOR_SUFFIX(icon, suffix) \
         g_hash_table_insert(gSuffixToIcon, (gchar *)suffix, (gchar *)icon)

      ADD_ICON_FOR_SUFFIX("text-x-csrc", "c");
      ADD_ICON_FOR_SUFFIX("text-x-csrc", "cc");
      ADD_ICON_FOR_SUFFIX("text-x-csrc", "cpp");
      ADD_ICON_FOR_SUFFIX("text-x-chdr", "h");
      ADD_ICON_FOR_SUFFIX("text-x-chdr", "hh");
      ADD_ICON_FOR_SUFFIX("text-x-chdr", "hpp");
      ADD_ICON_FOR_SUFFIX("gnome-mime-text-x-python", "py");
      ADD_ICON_FOR_SUFFIX("gnome-mime-application-x-python-bytecode", "pyc");

#undef ADD_ICON_FOR_SUFFIX

      g_once_init_leave(&initialized, TRUE);
   }

   name = g_path_get_basename(path);
   parts = g_strsplit(name, ".", 0);
   if ((length = g_strv_length(parts))) {
      suffix = parts[length - 1];
      icon_name = g_hash_table_lookup(gSuffixToIcon, suffix);
   }

   g_free(name);
   g_strfreev(parts);

   return icon_name;
}

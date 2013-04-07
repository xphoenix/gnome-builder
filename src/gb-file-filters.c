/* gb-file-filters.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gb-file-filters.h"

GtkFileFilter *
gb_file_filter_c_new (void)
{
   GtkFileFilter *filter;

   filter = gtk_file_filter_new();
   gtk_file_filter_set_name(filter, _("C/C++ Source"));
   gtk_file_filter_add_pattern(filter, "*.c");
   gtk_file_filter_add_pattern(filter, "*.cc");
   gtk_file_filter_add_pattern(filter, "*.cpp");
   return filter;
}

GtkFileFilter *
gb_file_filter_header_new (void)
{
   GtkFileFilter *filter;

   filter = gtk_file_filter_new();
   gtk_file_filter_set_name(filter, _("C/C++ Header"));
   gtk_file_filter_add_pattern(filter, "*.h");
   gtk_file_filter_add_pattern(filter, "*.hh");
   gtk_file_filter_add_pattern(filter, "*.hpp");
   return filter;
}

GtkFileFilter *
gb_file_filter_js_new (void)
{
   GtkFileFilter *filter;

   filter = gtk_file_filter_new();
   gtk_file_filter_set_name(filter, _("JavaScript"));
   gtk_file_filter_add_pattern(filter, "*.js");
   return filter;
}

/* gb-file-filters.h:
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

#ifndef GB_FILE_FILTERS_H
#define GB_FILE_FILTERS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkFileFilter *gb_file_filter_c_new       (void);
GtkFileFilter *gb_file_filter_header_new  (void);
GtkFileFilter *gb_file_filter_js_new      (void);
GtkFileFilter *gb_file_filter_text_new    (void);

G_END_DECLS

#endif /* GB_FILE_FILTERS_H */

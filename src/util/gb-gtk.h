/* gb-gtk.h
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

#ifndef GB_GTK_H
#define GB_GTK_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

gpointer gb_gtk_builder_load_and_get_object         (const gchar *resource_path,
                                                     const gchar *name);
void     gb_gtk_progress_bar_file_progress_callback (goffset      current_num_bytes,
                                                     goffset      total_num_bytes,
                                                     gpointer     user_data);


G_END_DECLS

#endif /* GB_GTK_H */

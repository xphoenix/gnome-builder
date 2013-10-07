/* gb-icon-theme.c
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

#include <gtk/gtk.h>

#include "gb-application-resource.h"
#include "gb-icon-theme.h"

void
gb_icon_theme_init (void)
{
   static gsize initialized;
   
   if (g_once_init_enter(&initialized)) {
      GdkPixbuf *pixbuf;

#define REGISTER_ICON(name, size, path) \
      G_STMT_START { \
         pixbuf = gdk_pixbuf_new_from_resource("/org/gnome/Builder/data/icons/"path, NULL); \
         gtk_icon_theme_add_builtin_icon(name, size, pixbuf); \
         g_object_unref(pixbuf); \
      } G_STMT_END

      REGISTER_ICON("gb-project", 16, "project-16x.png");
      REGISTER_ICON("text-x-chdr", 16, "text-x-chdr-16x.png");
      REGISTER_ICON("text-x-csrc", 16, "text-x-csrc-16x.png");

      g_once_init_leave(&initialized, TRUE);
   }
}

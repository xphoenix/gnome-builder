/* gb-zoomable.c
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

#include "gb-zoomable.h"

G_DEFINE_INTERFACE(GbZoomable, gb_zoomable, GTK_TYPE_WIDGET)

static void
gb_zoomable_default_init (GbZoomableInterface *iface)
{
}

void
gb_zoomable_zoom_in (GbZoomable *zoomable)
{
   GbZoomableInterface *iface;

   g_return_if_fail(GB_IS_ZOOMABLE(zoomable));

   iface = GB_ZOOMABLE_GET_INTERFACE(zoomable);
   if (iface->zoom_in) {
      iface->zoom_in(zoomable);
   }
}

void
gb_zoomable_zoom_out (GbZoomable *zoomable)
{
   GbZoomableInterface *iface;

   g_return_if_fail(GB_IS_ZOOMABLE(zoomable));

   iface = GB_ZOOMABLE_GET_INTERFACE(zoomable);
   if (iface->zoom_in) {
      iface->zoom_out(zoomable);
   }
}

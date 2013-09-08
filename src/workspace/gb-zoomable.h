/* gb-zoomable.h
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

#ifndef GB_ZOOMABLE_H
#define GB_ZOOMABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_ZOOMABLE             (gb_zoomable_get_type())
#define GB_ZOOMABLE(o)               (G_TYPE_CHECK_INSTANCE_CAST((o),    GB_TYPE_ZOOMABLE, GbZoomable))
#define GB_IS_ZOOMABLE(o)            (G_TYPE_CHECK_INSTANCE_TYPE((o),    GB_TYPE_ZOOMABLE))
#define GB_ZOOMABLE_GET_INTERFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE((o), GB_TYPE_ZOOMABLE, GbZoomableInterface))

typedef struct _GbZoomable          GbZoomable;
typedef struct _GbZoomableInterface GbZoomableInterface;

struct _GbZoomableInterface
{
   GTypeInterface parent;

   void (*zoom_in)  (GbZoomable *zoomable);
   void (*zoom_out) (GbZoomable *zoomable);
};

GType gb_zoomable_get_type (void) G_GNUC_CONST;
void  gb_zoomable_zoom_in  (GbZoomable *zoomable);
void  gb_zoomable_zoom_out (GbZoomable *zoomable);

G_END_DECLS

#endif /* GB_ZOOMABLE_H */

/* gb-source-overlay.h
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

#ifndef GB_SOURCE_OVERLAY_H
#define GB_SOURCE_OVERLAY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_OVERLAY            (gb_source_overlay_get_type())
#define GB_SOURCE_OVERLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_OVERLAY, GbSourceOverlay))
#define GB_SOURCE_OVERLAY_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_OVERLAY, GbSourceOverlay const))
#define GB_SOURCE_OVERLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_OVERLAY, GbSourceOverlayClass))
#define GB_IS_SOURCE_OVERLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_OVERLAY))
#define GB_IS_SOURCE_OVERLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_OVERLAY))
#define GB_SOURCE_OVERLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_OVERLAY, GbSourceOverlayClass))

typedef struct _GbSourceOverlay        GbSourceOverlay;
typedef struct _GbSourceOverlayClass   GbSourceOverlayClass;
typedef struct _GbSourceOverlayPrivate GbSourceOverlayPrivate;

struct _GbSourceOverlay
{
   GtkDrawingArea parent;

   /*< private >*/
   GbSourceOverlayPrivate *priv;
};

struct _GbSourceOverlayClass
{
   GtkDrawingAreaClass parent_class;
};

GType gb_source_overlay_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SOURCE_OVERLAY_H */

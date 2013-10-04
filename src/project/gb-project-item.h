/* gb-project-item.h
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
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

#ifndef GB_PROJECT_ITEM_H
#define GB_PROJECT_ITEM_H

#include <glib-object.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_ITEM            (gb_project_item_get_type())
#define GB_PROJECT_ITEM_ERROR           (gb_project_item_error_quark())
#define GB_PROJECT_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_ITEM, GbProjectItem))
#define GB_PROJECT_ITEM_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_ITEM, GbProjectItem const))
#define GB_PROJECT_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_ITEM, GbProjectItemClass))
#define GB_IS_PROJECT_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_ITEM))
#define GB_IS_PROJECT_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_ITEM))
#define GB_PROJECT_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_ITEM, GbProjectItemClass))

typedef struct _GbProjectItem        GbProjectItem;
typedef struct _GbProjectItemClass   GbProjectItemClass;
typedef struct _GbProjectItemPrivate GbProjectItemPrivate;
typedef enum   _GbProjectItemError   GbProjectItemError;

enum _GbProjectItemError
{
   GB_PROJECT_ITEM_ERROR_NO_PARENT = 1,
   GB_PROJECT_ITEM_ERROR_NO_DESERIALIZER,
};

struct _GbProjectItem
{
   GInitiallyUnowned parent;

   /*< private >*/
   GbProjectItemPrivate *priv;
};

struct _GbProjectItemClass
{
   GInitiallyUnownedClass parent_class;
};

GQuark         gb_project_item_error_quark  (void) G_GNUC_CONST;
GType          gb_project_item_get_type     (void) G_GNUC_CONST;
GbProjectItem *gb_project_item_get_parent   (GbProjectItem  *item);
GbProjectItem *gb_project_item_get_toplevel (GbProjectItem  *item);

G_END_DECLS

#endif /* GB_PROJECT_ITEM_H */

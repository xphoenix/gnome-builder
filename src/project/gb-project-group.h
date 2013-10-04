/* gb-project-group.h
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

#ifndef GB_PROJECT_GROUP_H
#define GB_PROJECT_GROUP_H

#include "gb-project-item.h"

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_GROUP            (gb_project_group_get_type())
#define GB_PROJECT_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_GROUP, GbProjectGroup))
#define GB_PROJECT_GROUP_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_GROUP, GbProjectGroup const))
#define GB_PROJECT_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_GROUP, GbProjectGroupClass))
#define GB_IS_PROJECT_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_GROUP))
#define GB_IS_PROJECT_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_GROUP))
#define GB_PROJECT_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_GROUP, GbProjectGroupClass))

typedef struct _GbProjectGroup        GbProjectGroup;
typedef struct _GbProjectGroupClass   GbProjectGroupClass;
typedef struct _GbProjectGroupPrivate GbProjectGroupPrivate;

struct _GbProjectGroup
{
   GbProjectItem parent;

   /*< private >*/
   GbProjectGroupPrivate *priv;
};

struct _GbProjectGroupClass
{
   GbProjectItemClass parent_class;
};

guint          gb_project_group_get_count     (GbProjectGroup *group);
GType          gb_project_group_get_item_type (GbProjectGroup *group);
GbProjectItem *gb_project_group_get_item      (GbProjectGroup *group,
                                               guint           index_);
GList         *gb_project_group_get_items     (GbProjectGroup *group);
GType          gb_project_group_get_type      (void) G_GNUC_CONST;
void           gb_project_group_append        (GbProjectGroup *group,
                                               GbProjectItem  *item);
void           gb_project_group_insert        (GbProjectGroup *group,
                                               guint           index_,
                                               GbProjectItem  *item);
void           gb_project_group_remove        (GbProjectGroup *group,
                                               GbProjectItem  *item);
void           gb_project_group_remove_index  (GbProjectGroup *group,
                                               guint           index_);

G_END_DECLS

#endif /* GB_PROJECT_GROUP_H */

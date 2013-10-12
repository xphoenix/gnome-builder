/* gb-tree-node-group.h
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

#ifndef GB_TREE_NODE_GROUP_H
#define GB_TREE_NODE_GROUP_H

#include "gb-tree-node.h"

G_BEGIN_DECLS

#define GB_TYPE_TREE_NODE_GROUP            (gb_tree_node_group_get_type())
#define GB_TREE_NODE_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TREE_NODE_GROUP, GbTreeNodeGroup))
#define GB_TREE_NODE_GROUP_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TREE_NODE_GROUP, GbTreeNodeGroup const))
#define GB_TREE_NODE_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_TREE_NODE_GROUP, GbTreeNodeGroupClass))
#define GB_IS_TREE_NODE_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_TREE_NODE_GROUP))
#define GB_IS_TREE_NODE_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_TREE_NODE_GROUP))
#define GB_TREE_NODE_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_TREE_NODE_GROUP, GbTreeNodeGroupClass))

typedef struct _GbTreeNodeGroup        GbTreeNodeGroup;
typedef struct _GbTreeNodeGroupClass   GbTreeNodeGroupClass;
typedef struct _GbTreeNodeGroupPrivate GbTreeNodeGroupPrivate;

struct _GbTreeNodeGroup
{
   GbTreeNode parent;

   /*< private >*/
   GbTreeNodeGroupPrivate *priv;
};

struct _GbTreeNodeGroupClass
{
   GbTreeNodeClass parent_class;
};

GType  gb_tree_node_group_get_type     (void) G_GNUC_CONST;
void   gb_tree_node_group_add          (GbTreeNodeGroup *group,
                                        GbTreeNode      *child);
void   gb_tree_node_group_remove       (GbTreeNodeGroup *group,
                                        GbTreeNode      *child);
GList *gb_tree_node_group_get_children (GbTreeNodeGroup *group);

G_END_DECLS

#endif /* GB_TREE_NODE_GROUP_H */

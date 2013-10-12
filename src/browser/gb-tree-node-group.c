/* gb-tree-node-group.c
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

#include <glib/gi18n.h>

#include "gb-tree-node-group.h"

G_DEFINE_TYPE(GbTreeNodeGroup, gb_tree_node_group, GB_TYPE_TREE_NODE)

struct _GbTreeNodeGroupPrivate
{
   GList *children;
};

GList *
gb_tree_node_group_get_children (GbTreeNodeGroup *group)
{
   g_return_val_if_fail(GB_IS_TREE_NODE_GROUP(group), NULL);
   return group->priv->children;
}

void
gb_tree_node_group_add (GbTreeNodeGroup *group,
                        GbTreeNode      *child)
{
   GbTreeNodeGroupPrivate *priv;

   g_return_if_fail(GB_IS_TREE_NODE_GROUP(group));
   g_return_if_fail(GB_IS_TREE_NODE(child));

   priv = group->priv;

   priv->children = g_list_append(priv->children, g_object_ref(child));
}

void
gb_tree_node_group_remove (GbTreeNodeGroup *group,
                           GbTreeNode      *child)
{
   GbTreeNodeGroupPrivate *priv;

   g_return_if_fail(GB_IS_TREE_NODE_GROUP(group));
   g_return_if_fail(GB_IS_TREE_NODE(child));

   priv = group->priv;

   if (g_list_find(priv->children, child)) {
      priv->children = g_list_remove(priv->children, child);
      g_object_unref(child);
   }
}

static void
gb_tree_node_group_finalize (GObject *object)
{
   GbTreeNodeGroupPrivate *priv;

   priv = GB_TREE_NODE_GROUP(object)->priv;

   g_list_foreach(priv->children, (GFunc)g_object_unref, NULL);
   g_list_free(priv->children);
   priv->children = NULL;

   G_OBJECT_CLASS(gb_tree_node_group_parent_class)->finalize(object);
}

static void
gb_tree_node_group_class_init (GbTreeNodeGroupClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_tree_node_group_finalize;
   g_type_class_add_private(object_class, sizeof(GbTreeNodeGroupPrivate));
}

static void
gb_tree_node_group_init (GbTreeNodeGroup *group)
{
   group->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(group,
                                  GB_TYPE_TREE_NODE_GROUP,
                                  GbTreeNodeGroupPrivate);
}

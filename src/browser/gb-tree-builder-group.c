/* gb-tree-builder-group.c
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

#include "gb-tree-builder-group.h"
#include "gb-tree-node-group.h"

G_DEFINE_TYPE(GbTreeBuilderGroup, gb_tree_builder_group, GB_TYPE_TREE_BUILDER)

static void
gb_tree_builder_group_build_node (GbTreeBuilder *builder,
                                  GbTreeNode    *node)
{
   GList *list;

   g_return_if_fail(GB_IS_TREE_BUILDER(builder));
   g_return_if_fail(GB_IS_TREE_NODE(node));

   list = gb_tree_node_group_get_children(GB_TREE_NODE_GROUP(node));

   for (; list; list = list->next) {
      gb_tree_node_append(node, list->data);
   }
}

static void
gb_tree_builder_group_class_init (GbTreeBuilderGroupClass *klass)
{
   GbTreeBuilderClass *builder_class;

   builder_class = GB_TREE_BUILDER_CLASS(klass);
   builder_class->build_node = gb_tree_builder_group_build_node;
}

static void
gb_tree_builder_group_init (GbTreeBuilderGroup *group)
{
}

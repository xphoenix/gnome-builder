/* gb-tree-builder-gir.c
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

#include "gb-tree-builder-gir.h"
#include "gb-tree-node-gir.h"

G_DEFINE_TYPE(GbTreeBuilderGir, gb_tree_builder_gir, GB_TYPE_TREE_BUILDER)

GbTreeBuilder *
gb_tree_builder_gir_new (void)
{
   return g_object_new(GB_TYPE_TREE_BUILDER_GIR, NULL);
}

static void
gb_tree_builder_gir_build_node (GbTreeBuilder *builder,
                                GbTreeNode    *node)
{
   GbTreeNodeGirMode mode;
   GIRepository *repository;
   GbTreeNode *child;
   gchar **namespaces;
   gint i;

   if (!GB_IS_TREE_NODE_GIR(node)) {
      return;
   }

   repository = g_irepository_get_default();
   mode = gb_tree_node_gir_get_mode(GB_TREE_NODE_GIR(node));

   if (mode == GB_TREE_NODE_GIR_MODE_NONE) {
      namespaces = g_irepository_get_loaded_namespaces(repository);
      for (i = 0; namespaces[i]; i++) {
         child = g_object_new(GB_TYPE_TREE_NODE_GIR,
                              "mode", GB_TREE_NODE_GIR_MODE_NAMESPACE,
                              "text", namespaces[i],
                              NULL);
         gb_tree_node_append(node, child);
      }
      g_strfreev(namespaces);
   } else if (mode == GB_TREE_NODE_GIR_MODE_NAMESPACE) {
      child = g_object_new(GB_TYPE_TREE_NODE_GIR,
                           "mode", GB_TREE_NODE_GIR_MODE_STRUCTS,
                           "text", _("Structs"),
                           NULL);
      gb_tree_node_append(node, child);

      child = g_object_new(GB_TYPE_TREE_NODE_GIR,
                           "mode", GB_TREE_NODE_GIR_MODE_ENUMS,
                           "text", _("Enums"),
                           NULL);
      gb_tree_node_append(node, child);
   }
}

static void
gb_tree_builder_gir_class_init (GbTreeBuilderGirClass *klass)
{
   GbTreeBuilderClass *builder_class;

   builder_class = GB_TREE_BUILDER_CLASS(klass);
   builder_class->build_node = gb_tree_builder_gir_build_node;

   g_irepository_require(g_irepository_get_default(), "Gtk", "3.0", 0, NULL);
}

static void
gb_tree_builder_gir_init (GbTreeBuilderGir *gir)
{
}

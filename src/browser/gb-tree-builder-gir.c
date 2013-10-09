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
   GIRepository *repository;
   const gchar *ns;
   GbTreeNode *child;
   GIBaseInfo *info;
   gchar **namespaces;
   gint i;
   gint n_info;

   if (!GB_IS_TREE_NODE_GIR(node)) {
      return;
   }

   repository = g_irepository_get_default();
   info = gb_tree_node_gir_get_info(GB_TREE_NODE_GIR(node));
   ns = gb_tree_node_gir_get_ns(GB_TREE_NODE_GIR(node));

   if (!info && !ns) {
      namespaces = g_irepository_get_loaded_namespaces(repository);
      for (i = 0; namespaces[i]; i++) {
         child = g_object_new(GB_TYPE_TREE_NODE_GIR,
                              "ns", namespaces[i],
                              "text", namespaces[i],
                              NULL);
         gb_tree_node_append(node, child);
      }
      g_strfreev(namespaces);
   } else if (ns) {
      n_info = g_irepository_get_n_infos(repository, ns);
      for (i = 0; i < n_info; i++) {
         info = g_irepository_get_info(repository, ns, i);
         child = g_object_new(GB_TYPE_TREE_NODE_GIR,
                              "info", info,
                              "text", g_base_info_get_name(info),
                              NULL);
         gb_tree_node_append(node, child);
         g_base_info_unref(info);
      }
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

/* gb-symbol-tree-builder-gir.c
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

#include "gb-symbol-tree-builder-gir.h"
#include "gb-symbol-tree-node-gir.h"
#include "gb-tree-node-group.h"

G_DEFINE_TYPE(GbSymbolTreeBuilderGir,
              gb_symbol_tree_builder_gir,
              GB_TYPE_TREE_BUILDER)

static void
gb_symbol_tree_builder_gir_build_node (GbTreeBuilder *builder,
                                       GbTreeNode    *node)
{
   GbSymbolTreeBuilderGir *builder_gir = (GbSymbolTreeBuilderGir *)builder;
   GbSymbolTreeNodeGir *node_gir;

   g_return_if_fail(GB_IS_SYMBOL_TREE_BUILDER_GIR(builder_gir));

   if (GB_IS_SYMBOL_TREE_NODE_GIR(node)) {
      node_gir = GB_SYMBOL_TREE_NODE_GIR(node);
      if (gb_symbol_tree_node_gir_is_repository(node_gir)) {
      }
   } else if (GB_IS_TREE_NODE_GROUP(node)) {
   }
}

static void
gb_symbol_tree_builder_gir_class_init (GbSymbolTreeBuilderGirClass *klass)
{
   GbTreeBuilderClass *builder_class;

   builder_class = GB_TREE_BUILDER_CLASS(klass);
   builder_class->build_node = gb_symbol_tree_builder_gir_build_node;
}

static void
gb_symbol_tree_builder_gir_init (GbSymbolTreeBuilderGir *gir)
{
}

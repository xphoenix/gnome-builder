/* gb-tree-node-gir.h
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

#ifndef GB_TREE_NODE_GIR_H
#define GB_TREE_NODE_GIR_H

#include <girepository.h>

#include "gb-tree-node.h"

G_BEGIN_DECLS

#define GB_TYPE_TREE_NODE_GIR            (gb_tree_node_gir_get_type())
#define GB_TREE_NODE_GIR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TREE_NODE_GIR, GbTreeNodeGir))
#define GB_TREE_NODE_GIR_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TREE_NODE_GIR, GbTreeNodeGir const))
#define GB_TREE_NODE_GIR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_TREE_NODE_GIR, GbTreeNodeGirClass))
#define GB_IS_TREE_NODE_GIR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_TREE_NODE_GIR))
#define GB_IS_TREE_NODE_GIR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_TREE_NODE_GIR))
#define GB_TREE_NODE_GIR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_TREE_NODE_GIR, GbTreeNodeGirClass))

typedef struct _GbTreeNodeGir        GbTreeNodeGir;
typedef struct _GbTreeNodeGirClass   GbTreeNodeGirClass;
typedef struct _GbTreeNodeGirPrivate GbTreeNodeGirPrivate;

struct _GbTreeNodeGir
{
   GbTreeNode parent;

   /*< private >*/
   GbTreeNodeGirPrivate *priv;
};

struct _GbTreeNodeGirClass
{
   GbTreeNodeClass parent_class;
};

GbTreeNode  *gb_tree_node_gir_new      (GIBaseInfo    *info);
GType        gb_tree_node_gir_get_type (void) G_GNUC_CONST;
GIBaseInfo  *gb_tree_node_gir_get_info (GbTreeNodeGir *gir);
void         gb_tree_node_gir_set_info (GbTreeNodeGir *gir,
                                        GIBaseInfo    *info);
const gchar *gb_tree_node_gir_get_ns   (GbTreeNodeGir *gir);

G_END_DECLS

#endif /* GB_TREE_NODE_GIR_H */

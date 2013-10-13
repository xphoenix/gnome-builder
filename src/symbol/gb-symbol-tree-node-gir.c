/* gb-symbol-tree-node-gir.c
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

#include <girepository.h>
#include <glib/gi18n.h>

#include "gb-symbol-tree-node-gir.h"

G_DEFINE_TYPE(GbSymbolTreeNodeGir, gb_symbol_tree_node_gir, GB_TYPE_TREE_NODE)

struct _GbSymbolTreeNodeGirPrivate
{
   GIRepository *repository;
   GIBaseInfo *info;
};

enum
{
   PROP_0,
   PROP_INFO,
   PROP_REPOSITORY,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

gboolean
gb_symbol_tree_node_gir_is_repository (GbSymbolTreeNodeGir *gir)
{
   g_return_val_if_fail(GB_IS_SYMBOL_TREE_NODE_GIR(gir), FALSE);
   return gir->priv->repository && !gir->priv->info;
}

GIBaseInfo *
gb_symbol_tree_node_gir_get_info (GbSymbolTreeNodeGir *gir)
{
   g_return_val_if_fail(GB_IS_SYMBOL_TREE_NODE_GIR(gir), NULL);
   return gir->priv->info;
}

void
gb_symbol_tree_node_gir_set_info (GbSymbolTreeNodeGir *gir,
                                  GIBaseInfo          *info)
{
   GbSymbolTreeNodeGirPrivate *priv;

   g_return_if_fail(GB_IS_SYMBOL_TREE_NODE_GIR(gir));

   priv = gir->priv;

   g_clear_pointer(&priv->info, g_base_info_unref);
   if (info) {
      priv->info = g_base_info_ref(info);
   }
}

GIRepository *
gb_symbol_tree_node_gir_get_repository (GbSymbolTreeNodeGir *gir)
{
   g_return_val_if_fail(GB_IS_SYMBOL_TREE_NODE_GIR(gir), NULL);
   return gir->priv->repository;
}

static void
gb_symbol_tree_node_gir_set_repository (GbSymbolTreeNodeGir *gir,
                                        GIRepository        *repository)
{
   GbSymbolTreeNodeGirPrivate *priv;

   g_return_if_fail(GB_IS_SYMBOL_TREE_NODE_GIR(gir));
   g_return_if_fail(G_IS_IREPOSITORY(repository));

   priv = gir->priv;

   g_clear_object(&priv->repository);

   if (repository) {
      priv->repository = g_object_ref(repository);
   }
}

static void
gb_symbol_tree_node_gir_finalize (GObject *object)
{
   GbSymbolTreeNodeGirPrivate *priv;

   priv = GB_SYMBOL_TREE_NODE_GIR(object)->priv;

   g_clear_object(&priv->repository);
   g_clear_pointer(&priv->info, g_base_info_unref);

   G_OBJECT_CLASS(gb_symbol_tree_node_gir_parent_class)->finalize(object);
}

static void
gb_symbol_tree_node_gir_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
   GbSymbolTreeNodeGir *gir = GB_SYMBOL_TREE_NODE_GIR(object);

   switch (prop_id) {
   case PROP_INFO:
      g_value_set_object(value, gb_symbol_tree_node_gir_get_info(gir));
      break;
   case PROP_REPOSITORY:
      g_value_set_object(value, gb_symbol_tree_node_gir_get_repository(gir));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_tree_node_gir_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
   GbSymbolTreeNodeGir *gir = GB_SYMBOL_TREE_NODE_GIR(object);

   switch (prop_id) {
   case PROP_INFO:
      gb_symbol_tree_node_gir_set_info(gir, g_value_get_boxed(value));
      break;
   case PROP_REPOSITORY:
      gb_symbol_tree_node_gir_set_repository(gir, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_tree_node_gir_class_init (GbSymbolTreeNodeGirClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_symbol_tree_node_gir_finalize;
   object_class->get_property = gb_symbol_tree_node_gir_get_property;
   object_class->set_property = gb_symbol_tree_node_gir_set_property;
   g_type_class_add_private(object_class, sizeof(GbSymbolTreeNodeGirPrivate));

   gParamSpecs[PROP_INFO] =
      g_param_spec_boxed("info",
                         _("Info"),
                         _("The GIBaseInfo to be visualized."),
                         GI_TYPE_BASE_INFO,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_INFO,
                                   gParamSpecs[PROP_INFO]);

   gParamSpecs[PROP_REPOSITORY] =
      g_param_spec_object("repository",
                          _("Repository"),
                          _("The repository for the node."),
                          G_TYPE_IREPOSITORY,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_REPOSITORY,
                                   gParamSpecs[PROP_REPOSITORY]);
}

static void
gb_symbol_tree_node_gir_init (GbSymbolTreeNodeGir *gir)
{
   gir->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(gir,
                                  GB_TYPE_SYMBOL_TREE_NODE_GIR,
                                  GbSymbolTreeNodeGirPrivate);
}

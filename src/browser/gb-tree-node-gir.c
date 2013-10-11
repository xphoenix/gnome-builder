/* gb-tree-node-gir.c
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

#include "gb-tree-node-gir.h"

G_DEFINE_TYPE(GbTreeNodeGir, gb_tree_node_gir, GB_TYPE_TREE_NODE)

struct _GbTreeNodeGirPrivate
{
   GIBaseInfo *info;
   GbTreeNodeGirMode mode;
};

enum
{
   PROP_0,
   PROP_INFO,
   PROP_MODE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbTreeNode *
gb_tree_node_gir_new (GIBaseInfo *info)
{
   return g_object_new(GB_TYPE_TREE_NODE_GIR, "info", info, NULL);
}

GbTreeNodeGirMode
gb_tree_node_gir_get_mode (GbTreeNodeGir *gir)
{
   g_return_val_if_fail(GB_IS_TREE_NODE_GIR(gir), 0);
   return gir->priv->mode;
}

void
gb_tree_node_gir_set_mode (GbTreeNodeGir     *gir,
                           GbTreeNodeGirMode  mode)
{
   g_return_if_fail(GB_IS_TREE_NODE_GIR(gir));
   gir->priv->mode = mode;
}

GIBaseInfo *
gb_tree_node_gir_get_info (GbTreeNodeGir *gir)
{
   g_return_val_if_fail(GB_IS_TREE_NODE_GIR(gir), NULL);
   return gir->priv->info;
}

void
gb_tree_node_gir_set_info (GbTreeNodeGir *gir,
                           GIBaseInfo    *info)
{
   GbTreeNodeGirPrivate *priv;

   g_return_if_fail(GB_IS_TREE_NODE_GIR(gir));

   priv = gir->priv;

   if (priv->info) {
      g_base_info_unref(priv->info);
      priv->info = NULL;
   }

   if (info) {
      priv->info = g_base_info_ref(info);
      g_object_set(gir,
                   "text", g_base_info_get_name(info),
                   "icon-name", NULL, /* TODO */
                   NULL);
   }

   g_object_notify_by_pspec(G_OBJECT(gir), gParamSpecs[PROP_INFO]);
}

static void
gb_tree_node_gir_finalize (GObject *object)
{
   GbTreeNodeGirPrivate *priv;

   priv = GB_TREE_NODE_GIR(object)->priv;

   if (priv->info) {
      g_base_info_unref(priv->info);
      priv->info = NULL;
   }

   G_OBJECT_CLASS(gb_tree_node_gir_parent_class)->finalize(object);
}

static void
gb_tree_node_gir_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
   GbTreeNodeGir *gir = GB_TREE_NODE_GIR(object);

   switch (prop_id) {
   case PROP_INFO:
      g_value_set_boxed(value, gb_tree_node_gir_get_info(gir));
      break;
   case PROP_MODE:
      g_value_set_enum(value, gb_tree_node_gir_get_mode(gir));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_tree_node_gir_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
   GbTreeNodeGir *gir = GB_TREE_NODE_GIR(object);

   switch (prop_id) {
   case PROP_INFO:
      gb_tree_node_gir_set_info(gir, g_value_get_boxed(value));
      break;
   case PROP_MODE:
      gb_tree_node_gir_set_mode(gir, g_value_get_enum(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_tree_node_gir_class_init (GbTreeNodeGirClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_tree_node_gir_finalize;
   object_class->get_property = gb_tree_node_gir_get_property;
   object_class->set_property = gb_tree_node_gir_set_property;
   g_type_class_add_private(object_class, sizeof(GbTreeNodeGirPrivate));

   gParamSpecs[PROP_INFO] =
      g_param_spec_boxed("info",
                         _("Info"),
                         _("The girepository info."),
                         GI_TYPE_BASE_INFO,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_INFO,
                                   gParamSpecs[PROP_INFO]);

   gParamSpecs[PROP_MODE] =
      g_param_spec_enum("mode",
                        _("Mode"),
                        _("The mode for the node."),
                        GB_TYPE_TREE_NODE_GIR_MODE,
                        GB_TREE_NODE_GIR_MODE_NONE,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MODE,
                                   gParamSpecs[PROP_MODE]);
}

static void
gb_tree_node_gir_init (GbTreeNodeGir *gir)
{
   gir->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(gir,
                                  GB_TYPE_TREE_NODE_GIR,
                                  GbTreeNodeGirPrivate);
}

GType
gb_tree_node_gir_mode_get_type (void)
{
   static gsize initialized;
   static GType type;
   static const GEnumValue values[] = {
      { GB_TREE_NODE_GIR_MODE_NONE, "GB_TREE_NODE_GIR_MODE_NONE", "NONE" },
      { GB_TREE_NODE_GIR_MODE_NAMESPACE, "GB_TREE_NODE_GIR_MODE_NAMESPACE", "NAMESPACE" },
      { GB_TREE_NODE_GIR_MODE_STRUCTS, "GB_TREE_NODE_GIR_MODE_STRUCTS", "STRUCTS" },
      { GB_TREE_NODE_GIR_MODE_ENUMS, "GB_TREE_NODE_GIR_MODE_ENUMS", "ENUMS" },
      { 0 }
   };

   if (g_once_init_enter(&initialized)) {
      type = g_enum_register_static("GbTreeNodeGirMode", values);
      g_once_init_leave(&initialized, TRUE);
   }

   return type;
}

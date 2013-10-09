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
   gchar *ns;
};

enum
{
   PROP_0,
   PROP_INFO,
   PROP_NS,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbTreeNode *
gb_tree_node_gir_new (GIBaseInfo *info)
{
   return g_object_new(GB_TYPE_TREE_NODE_GIR, "info", info, NULL);
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

const gchar *
gb_tree_node_gir_get_ns (GbTreeNodeGir *gir)
{
   g_return_val_if_fail(GB_IS_TREE_NODE_GIR(gir), NULL);
   return gir->priv->ns;
}

static void
gb_tree_node_gir_set_ns (GbTreeNodeGir *gir,
                         const gchar   *ns)
{
   g_return_if_fail(GB_IS_TREE_NODE_GIR(gir));

   g_free(gir->priv->ns);
   gir->priv->ns = g_strdup(ns);
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

   g_free(priv->ns);

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
   case PROP_NS:
      g_value_set_string(value, gb_tree_node_gir_get_ns(gir));
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
   case PROP_NS:
      gb_tree_node_gir_set_ns(gir, g_value_get_string(value));
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

   gParamSpecs[PROP_NS] =
      g_param_spec_string("ns",
                          _("NS"),
                          _("Namespace"),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NS,
                                   gParamSpecs[PROP_NS]);
}

static void
gb_tree_node_gir_init (GbTreeNodeGir *gir)
{
   gir->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(gir,
                                  GB_TYPE_TREE_NODE_GIR,
                                  GbTreeNodeGirPrivate);
}

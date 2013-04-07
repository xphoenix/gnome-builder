/* gb-symbol-pane.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gb-symbol-pane.h"
#include "gb-symbol-provider.h"

G_DEFINE_TYPE(GbSymbolPane, gb_symbol_pane, GB_TYPE_WORKSPACE_PANE)

struct _GbSymbolPanePrivate
{
   GbSymbolProvider *provider;
};

enum
{
   PROP_0,
   PROP_PROVIDER,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSymbolProvider *
gb_symbol_pane_get_provider (GbSymbolPane *pane)
{
   g_return_val_if_fail(GB_IS_SYMBOL_PANE(pane), NULL);
   return pane->priv->provider;
}

void
gb_symbol_pane_set_provider (GbSymbolPane     *pane,
                             GbSymbolProvider *provider)
{
   GbSymbolPanePrivate *priv;

   g_return_if_fail(GB_IS_SYMBOL_PANE(pane));
   g_return_if_fail(!provider || GB_IS_SYMBOL_PROVIDER(provider));

   priv = pane->priv;

   g_clear_object(&priv->provider);

   if (provider) {
      priv->provider = g_object_ref(provider);
   }

   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_PROVIDER]);
}

static void
gb_symbol_pane_finalize (GObject *object)
{
   GbSymbolPanePrivate *priv;

   priv = GB_SYMBOL_PANE(object)->priv;

   g_clear_object(&priv->provider);

   G_OBJECT_CLASS(gb_symbol_pane_parent_class)->finalize(object);
}

static void
gb_symbol_pane_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   GbSymbolPane *pane = GB_SYMBOL_PANE(object);

   switch (prop_id) {
   case PROP_PROVIDER:
      g_value_set_object(value, gb_symbol_pane_get_provider(pane));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_pane_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   GbSymbolPane *pane = GB_SYMBOL_PANE(object);

   switch (prop_id) {
   case PROP_PROVIDER:
      gb_symbol_pane_set_provider(pane, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_pane_class_init (GbSymbolPaneClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_symbol_pane_finalize;
   object_class->get_property = gb_symbol_pane_get_property;
   object_class->set_property = gb_symbol_pane_set_property;
   g_type_class_add_private(object_class, sizeof(GbSymbolPanePrivate));

   gParamSpecs[PROP_PROVIDER] =
      g_param_spec_object("provider",
                          _("Provider"),
                          _("The symbol provider to load symbols from."),
                          GB_TYPE_SYMBOL_PROVIDER,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_PROVIDER,
                                   gParamSpecs[PROP_PROVIDER]);
}

static void
gb_symbol_pane_init (GbSymbolPane *pane)
{
   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_SYMBOL_PANE,
                                            GbSymbolPanePrivate);
}

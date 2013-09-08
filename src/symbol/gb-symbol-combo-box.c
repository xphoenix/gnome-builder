/* gb-symbol-combo-box.c
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

#include "gb-symbol-combo-box.h"
#include "gb-symbol-provider.h"

G_DEFINE_TYPE(GbSymbolComboBox, gb_symbol_combo_box, GTK_TYPE_COMBO_BOX)

struct _GbSymbolComboBoxPrivate
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

static void
gb_symbol_combo_box_update (GbSymbolComboBox *combo_box)
{
}

GbSymbolProvider *
gb_symbol_combo_box_get_provider (GbSymbolComboBox *combo_box)
{
   g_return_val_if_fail(GB_IS_SYMBOL_COMBO_BOX(combo_box), NULL);
   return combo_box->priv->provider;
}

void
gb_symbol_combo_box_set_provider (GbSymbolComboBox *combo_box,
                                  GbSymbolProvider *provider)
{
   GbSymbolComboBoxPrivate *priv;

   g_return_if_fail(GB_IS_SYMBOL_COMBO_BOX(combo_box));
   g_return_if_fail(!provider || GB_IS_SYMBOL_PROVIDER(provider));

   priv = combo_box->priv;

   g_clear_object(&priv->provider);

   if (provider) {
      priv->provider = g_object_ref(provider);
   }

   gb_symbol_combo_box_update(combo_box);
}

static void
gb_symbol_combo_box_finalize (GObject *object)
{
   GbSymbolComboBoxPrivate *priv;

   priv = GB_SYMBOL_COMBO_BOX(object)->priv;

   g_clear_object(&priv->provider);

   G_OBJECT_CLASS(gb_symbol_combo_box_parent_class)->finalize(object);
}

static void
gb_symbol_combo_box_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
   GbSymbolComboBox *combo_box = GB_SYMBOL_COMBO_BOX(object);

   switch (prop_id) {
   case PROP_PROVIDER:
      g_value_set_object(value, gb_symbol_combo_box_get_provider(combo_box));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_combo_box_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
   GbSymbolComboBox *combo_box = GB_SYMBOL_COMBO_BOX(object);

   switch (prop_id) {
   case PROP_PROVIDER:
      gb_symbol_combo_box_set_provider(combo_box, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_combo_box_class_init (GbSymbolComboBoxClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_symbol_combo_box_finalize;
   object_class->get_property = gb_symbol_combo_box_get_property;
   object_class->set_property = gb_symbol_combo_box_set_property;
   g_type_class_add_private(object_class, sizeof(GbSymbolComboBoxPrivate));

   gParamSpecs[PROP_PROVIDER] =
      g_param_spec_object("provider",
                          _("Provider"),
                          _("The provider for symbols."),
                          GB_TYPE_SYMBOL_PROVIDER,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_PROVIDER,
                                   gParamSpecs[PROP_PROVIDER]);
}

static void
gb_symbol_combo_box_init (GbSymbolComboBox *combo_box)
{
   combo_box->priv = G_TYPE_INSTANCE_GET_PRIVATE(combo_box,
                                                 GB_TYPE_SYMBOL_COMBO_BOX,
                                                 GbSymbolComboBoxPrivate);
}

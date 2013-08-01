/* gb-symbol.c
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

#include "gb-symbol.h"

G_DEFINE_TYPE(GbSymbol, gb_symbol, G_TYPE_OBJECT)

struct _GbSymbolPrivate
{
   gchar *name;
};

enum
{
   PROP_0,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_symbol_get_name (GbSymbol *symbol)
{
   g_return_val_if_fail(GB_IS_SYMBOL(symbol), NULL);
   return symbol->priv->name;
}

static void
gb_symbol_set_name (GbSymbol    *symbol,
                    const gchar *name)
{
   g_return_if_fail(GB_IS_SYMBOL(symbol));
   g_free(symbol->priv->name);
   symbol->priv->name = g_strdup(name);
}

static void
gb_symbol_finalize (GObject *object)
{
   GbSymbolPrivate *priv;

   priv = GB_SYMBOL(object)->priv;

   g_clear_pointer(&priv->name, g_free);

   G_OBJECT_CLASS(gb_symbol_parent_class)->finalize(object);
}

static void
gb_symbol_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
   GbSymbol *symbol = GB_SYMBOL(object);

   switch (prop_id) {
   case PROP_NAME:
      g_value_set_string(value, gb_symbol_get_name(symbol));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
   GbSymbol *symbol = GB_SYMBOL(object);

   switch (prop_id) {
   case PROP_NAME:
      gb_symbol_set_name(symbol, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_class_init (GbSymbolClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_symbol_finalize;
   object_class->get_property = gb_symbol_get_property;
   object_class->set_property = gb_symbol_set_property;
   g_type_class_add_private(object_class, sizeof(GbSymbolPrivate));

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the symbol."),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS |
                           G_PARAM_CONSTRUCT_ONLY));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_symbol_init (GbSymbol *symbol)
{
   symbol->priv = G_TYPE_INSTANCE_GET_PRIVATE(symbol,
                                              GB_TYPE_SYMBOL,
                                              GbSymbolPrivate);
}

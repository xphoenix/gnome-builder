/* gb-symbol-provider-js.c
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

#include "gb-symbol-provider-js.h"

G_DEFINE_TYPE(GbSymbolProviderJs,
              gb_symbol_provider_js,
              GB_TYPE_SYMBOL_PROVIDER)

struct _GbSymbolProviderJsPrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_symbol_provider_js_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_symbol_provider_js_parent_class)->finalize(object);
}

static void
gb_symbol_provider_js_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
   //GbSymbolProviderJs *js = GB_SYMBOL_PROVIDER_JS(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_provider_js_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
   //GbSymbolProviderJs *js = GB_SYMBOL_PROVIDER_JS(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_symbol_provider_js_class_init (GbSymbolProviderJsClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_symbol_provider_js_finalize;
   object_class->get_property = gb_symbol_provider_js_get_property;
   object_class->set_property = gb_symbol_provider_js_set_property;
   g_type_class_add_private(object_class, sizeof(GbSymbolProviderJsPrivate));
}

static void
gb_symbol_provider_js_init (GbSymbolProviderJs *js)
{
   js->priv = G_TYPE_INSTANCE_GET_PRIVATE(js,
                                          GB_TYPE_SYMBOL_PROVIDER_JS,
                                          GbSymbolProviderJsPrivate);
}

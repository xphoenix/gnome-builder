/* gb-search-provider.c
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

#include "gb-search-provider.h"

G_DEFINE_ABSTRACT_TYPE(GbSearchProvider, gb_search_provider, G_TYPE_OBJECT)

struct _GbSearchProviderPrivate
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
gb_search_provider_get_name (GbSearchProvider *provider)
{
   g_return_val_if_fail(GB_IS_SEARCH_PROVIDER(provider), NULL);

   return provider->priv->name;
}

void
gb_search_provider_set_name (GbSearchProvider *provider,
                             const gchar      *name)
{
   g_return_if_fail(GB_IS_SEARCH_PROVIDER(provider));

   g_free(provider->priv->name);
   provider->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(provider), gParamSpecs[PROP_NAME]);
}

void
gb_search_provider_populate (GbSearchProvider *provider,
                             const gchar      *search_term,
                             GtkListStore     *store)
{
   GbSearchProviderClass *klass;

   g_return_if_fail(GB_IS_SEARCH_PROVIDER(provider));
   g_return_if_fail(search_term);
   g_return_if_fail(GTK_IS_LIST_STORE(store));

   klass = GB_SEARCH_PROVIDER_GET_CLASS(provider);

   if (klass->populate) {
      klass->populate(provider, search_term, store);
   }
}

void
gb_search_provider_activate (GbSearchProvider *provider,
                             GtkTreeModel     *model,
                             GtkTreeIter      *iter)
{
   GbSearchProviderClass *klass;

   g_return_if_fail(GB_IS_SEARCH_PROVIDER(provider));
   g_return_if_fail(GTK_IS_TREE_MODEL(model));
   g_return_if_fail(iter);

   klass = GB_SEARCH_PROVIDER_GET_CLASS(provider);

   if (klass->activate) {
      klass->activate(provider, model, iter);
   }
}

static void
gb_search_provider_finalize (GObject *object)
{
   GbSearchProviderPrivate *priv;

   priv = GB_SEARCH_PROVIDER(object)->priv;

   g_free(priv->name);

   G_OBJECT_CLASS(gb_search_provider_parent_class)->finalize(object);
}

static void
gb_search_provider_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
   GbSearchProvider *provider = GB_SEARCH_PROVIDER(object);

   switch (prop_id) {
   case PROP_NAME:
      g_value_set_string(value, gb_search_provider_get_name(provider));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_search_provider_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
   GbSearchProvider *provider = GB_SEARCH_PROVIDER(object);

   switch (prop_id) {
   case PROP_NAME:
      gb_search_provider_set_name(provider, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_search_provider_class_init (GbSearchProviderClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_search_provider_finalize;
   object_class->get_property = gb_search_provider_get_property;
   object_class->set_property = gb_search_provider_set_property;
   g_type_class_add_private(object_class, sizeof(GbSearchProviderPrivate));

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the provider."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_search_provider_init (GbSearchProvider *provider)
{
   provider->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(provider,
                                  GB_TYPE_SEARCH_PROVIDER,
                                  GbSearchProviderPrivate);
}

/* gb-docs-pane.c
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

#include "gb-docs-pane.h"
#include "gb-docs-provider.h"

G_DEFINE_TYPE(GbDocsPane, gb_docs_pane, GB_TYPE_WORKSPACE_PANE)

struct _GbDocsPanePrivate
{
   GbDocsProvider *provider;
};

enum
{
   PROP_0,
   PROP_PROVIDER,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GtkWidget *
gb_docs_pane_new (void)
{
   return g_object_new(GB_TYPE_DOCS_PANE, NULL);
}

/**
 * gb_docs_pane_get_provider:
 * @pane: A #GbDocsPane.
 *
 * Gets the "provider" property which is the documentation provider.
 *
 * Returns: (transfer none): A #GbDocsProvider.
 */
GbDocsProvider *
gb_docs_pane_get_provider (GbDocsPane *pane)
{
   g_return_val_if_fail(GB_IS_DOCS_PANE(pane), NULL);
   return pane->priv->provider;
}

void
gb_docs_pane_set_provider (GbDocsPane     *pane,
                           GbDocsProvider *provider)
{
   GbDocsPanePrivate *priv;

   g_return_if_fail(GB_IS_DOCS_PANE(pane));

   priv = pane->priv;

   g_clear_object(&priv->provider);

   if (provider) {
      priv->provider = g_object_ref(provider);
   }

   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_PROVIDER]);
}

static void
gb_docs_pane_finalize (GObject *object)
{
   GbDocsPanePrivate *priv;

   priv = GB_DOCS_PANE(object)->priv;

   g_clear_object(&priv->provider);

   G_OBJECT_CLASS(gb_docs_pane_parent_class)->finalize(object);
}

static void
gb_docs_pane_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
   GbDocsPane *pane = GB_DOCS_PANE(object);

   switch (prop_id) {
   case PROP_PROVIDER:
      g_value_set_object(value, gb_docs_pane_get_provider(pane));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_docs_pane_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
   GbDocsPane *pane = GB_DOCS_PANE(object);

   switch (prop_id) {
   case PROP_PROVIDER:
      gb_docs_pane_set_provider(pane, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_docs_pane_class_init (GbDocsPaneClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_docs_pane_finalize;
   object_class->get_property = gb_docs_pane_get_property;
   object_class->set_property = gb_docs_pane_set_property;
   g_type_class_add_private(object_class, sizeof(GbDocsPanePrivate));

   gParamSpecs[PROP_PROVIDER] =
      g_param_spec_object("provider",
                          _("Provider"),
                          _("The documentation provider."),
                          GB_TYPE_DOCS_PROVIDER,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_PROVIDER,
                                   gParamSpecs[PROP_PROVIDER]);
}

static void
gb_docs_pane_init (GbDocsPane *pane)
{
   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_DOCS_PANE,
                                            GbDocsPanePrivate);
}

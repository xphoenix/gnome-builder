/* gb-workspace-search-provider.c
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

/**
 * SECTION:gb-workspace-search-provider
 * @title: GbWorkspaceSearchProvider
 * @short_title: Search provider for workspace panes.
 *
 * #GbWorkspaceSearchProvider provides search results relating to workspace
 * panes that may be selected. It provides convenient access to jump between
 * them.
 */

#include <glib/gi18n.h>

#include "gb-search-completion.h"
#include "gb-workspace.h"
#include "gb-workspace-search-provider.h"

G_DEFINE_TYPE(GbWorkspaceSearchProvider,
              gb_workspace_search_provider,
              GB_TYPE_SEARCH_PROVIDER)

struct _GbWorkspaceSearchProviderPrivate
{
   GbWorkspace *workspace;
};

enum
{
   PROP_0,
   PROP_WORKSPACE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_workspace_search_provider_populate (GbSearchProvider *provider,
                                       const gchar      *search_term,
                                       GtkListStore     *store)
{
   GtkTreeIter iter;

   g_assert(GB_IS_WORKSPACE_SEARCH_PROVIDER(provider));
   g_assert(search_term);
   g_assert(GTK_IS_LIST_STORE(store));

   gtk_list_store_append(store, &iter);
   gtk_list_store_set(store, &iter,
                      GB_SEARCH_COMPLETION_COLUMN_MARKUP, "<b>test mark</b>up",
                      GB_SEARCH_COMPLETION_COLUMN_TEXT, "test markup",
                      -1);

   g_print("Request to populate: search_term=%s\n", search_term);
}

static void
gb_workspace_search_provider_set_workspace (GbWorkspaceSearchProvider *provider,
                                            GbWorkspace               *workspace)
{
   GbWorkspaceSearchProviderPrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE_SEARCH_PROVIDER(provider));
   g_return_if_fail(!workspace || GB_IS_WORKSPACE(workspace));

   priv = provider->priv;

   if (priv->workspace) {
      g_object_remove_weak_pointer(G_OBJECT(priv->workspace),
                                   (gpointer *)&priv->workspace);
      priv->workspace = NULL;
   }

   if (workspace) {
      priv->workspace = workspace;
      g_object_add_weak_pointer(G_OBJECT(priv->workspace),
                                (gpointer *)&priv->workspace);
   }
}

static void
gb_workspace_search_provider_finalize (GObject *object)
{
   GbWorkspaceSearchProviderPrivate *priv;

   priv = GB_WORKSPACE_SEARCH_PROVIDER(object)->priv;

   G_OBJECT_CLASS(gb_workspace_search_provider_parent_class)->finalize(object);
}

static void
gb_workspace_search_provider_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
   GbWorkspaceSearchProvider *provider = GB_WORKSPACE_SEARCH_PROVIDER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_search_provider_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
   GbWorkspaceSearchProvider *provider = GB_WORKSPACE_SEARCH_PROVIDER(object);

   switch (prop_id) {
   case PROP_WORKSPACE:
      gb_workspace_search_provider_set_workspace(provider,
                                                 g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_search_provider_class_init (GbWorkspaceSearchProviderClass *klass)
{
   GObjectClass *object_class;
   GbSearchProviderClass *provider_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_search_provider_finalize;
   object_class->get_property = gb_workspace_search_provider_get_property;
   object_class->set_property = gb_workspace_search_provider_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceSearchProviderPrivate));

   provider_class = GB_SEARCH_PROVIDER_CLASS(klass);
   provider_class->populate = gb_workspace_search_provider_populate;

   gParamSpecs[PROP_WORKSPACE] =
      g_param_spec_object("workspace",
                          _("Workspace"),
                          _("The workspace to complete."),
                          GB_TYPE_WORKSPACE,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_WORKSPACE,
                                   gParamSpecs[PROP_WORKSPACE]);
}

static void
gb_workspace_search_provider_init (GbWorkspaceSearchProvider *provider)
{
   provider->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(provider,
                                  GB_TYPE_WORKSPACE_SEARCH_PROVIDER,
                                  GbWorkspaceSearchProviderPrivate);

   gb_search_provider_set_name(GB_SEARCH_PROVIDER(provider), _("Workspace"));
}

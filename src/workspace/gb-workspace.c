/* gb-workspace.c
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

#include "gb-debugger-section.h"
#include "gb-designer-section.h"
#include "gb-docs-section.h"
#include "gb-editor-section.h"
#include "gb-git-section.h"
#include "gb-gtk.h"
#include "gb-workspace.h"
#include "gb-workspace-container.h"
#include "gb-workspace-greeter.h"

struct _GbWorkspacePrivate
{
   GbProject *project;
};

G_DEFINE_TYPE_WITH_CODE(GbWorkspace,
                        gb_workspace,
                        GTK_TYPE_APPLICATION_WINDOW,
                        G_ADD_PRIVATE(GbWorkspace))

enum
{
   PROP_0,
   PROP_PROJECT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GtkWidget *
gb_workspace_new (void)
{
   return g_object_new(GB_TYPE_WORKSPACE, NULL);
}

GbProject *
gb_workspace_get_project (GbWorkspace *workspace)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);

   return workspace->priv->project;
}

void
gb_workspace_set_project (GbWorkspace *workspace,
                          GbProject   *project)
{
   GbWorkspacePrivate *priv;
   GtkWidget *child;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   if (priv->project != project) {
      g_clear_object(&priv->project);
      priv->project = project ? g_object_ref(project) : NULL;
   }

   child = gtk_bin_get_child(GTK_BIN(workspace));
   if (child) {
      gtk_container_remove(GTK_CONTAINER(workspace), child);
   }

   if (priv->project) {
      child = g_object_new(GB_TYPE_WORKSPACE_CONTAINER,
                           "visible", TRUE,
                           NULL);
   } else {
      child = g_object_new(GB_TYPE_WORKSPACE_GREETER,
                           "visible", TRUE,
                           NULL);
   }

   gtk_container_add(GTK_CONTAINER(workspace), child);

   g_object_notify_by_pspec(G_OBJECT(workspace), gParamSpecs[PROP_PROJECT]);
}

static void
gb_workspace_finalize (GObject *object)
{
   GbWorkspacePrivate *priv = GB_WORKSPACE(object)->priv;

   g_clear_object(&priv->project);

   G_OBJECT_CLASS(gb_workspace_parent_class)->finalize(object);
}

static void
gb_workspace_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
   GbWorkspace *workspace = GB_WORKSPACE(object);

   switch (prop_id) {
   case PROP_PROJECT:
      g_value_set_object(value, gb_workspace_get_project(workspace));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
   GbWorkspace *workspace = GB_WORKSPACE(object);

   switch (prop_id) {
   case PROP_PROJECT:
      gb_workspace_set_project(workspace, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_class_init (GbWorkspaceClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_finalize;
   object_class->get_property = gb_workspace_get_property;
   object_class->set_property = gb_workspace_set_property;

   gParamSpecs[PROP_PROJECT] =
      g_param_spec_object("project",
                          _("Project"),
                          _("The project for the workspace."),
                          GB_TYPE_PROJECT,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PROJECT,
                                   gParamSpecs[PROP_PROJECT]);
}

static void
gb_workspace_init (GbWorkspace *workspace)
{
   workspace->priv = gb_workspace_get_instance_private(workspace);
}

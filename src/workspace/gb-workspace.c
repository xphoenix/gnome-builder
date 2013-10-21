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

#include "gb-workspace.h"
#include "gb-workspace-actions.h"
#include "gb-workspace-container.h"
#include "gb-workspace-container-private.h"
#include "gb-workspace-greeter.h"

G_DEFINE_TYPE(GbWorkspace, gb_workspace, GTK_TYPE_APPLICATION_WINDOW)

struct _GbWorkspacePrivate
{
   GbBackForwardList    *back_forward_list;
   GbCommandStack       *command_stack;
   GbProject            *project;

   GbWorkspaceContainer *container;
};

enum
{
   PROP_0,
   PROP_COMMAND_STACK,
   PROP_CURRENT_DOCUMENT,
   PROP_CURRENT_SECTION,
   PROP_EDITOR,
   PROP_PROJECT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbWorkspace *
gb_workspace_new (void)
{
   return g_object_new(GB_TYPE_WORKSPACE,
                       NULL);
}

GbWorkspaceDocs *
gb_workspace_get_docs (GbWorkspace *workspace)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);

   return workspace->priv->container ?
          GB_WORKSPACE_DOCS(workspace->priv->container->priv->docs) :
          NULL;
}

GbWorkspaceEditor *
gb_workspace_get_editor (GbWorkspace *workspace)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);

   return workspace->priv->container ?
          GB_WORKSPACE_EDITOR(workspace->priv->container->priv->edit) :
          NULL;
}

GbWorkspaceSection *
gb_workspace_get_current_section (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   GtkWidget *child;

   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);

   priv = workspace->priv;

   if (priv->container) {
      child = gtk_stack_get_visible_child(
            GTK_STACK(priv->container->priv->stack));
      return GB_WORKSPACE_SECTION(child);
   }

   return NULL;
}

void
gb_workspace_set_current_section (GbWorkspace        *workspace,
                                  GbWorkspaceSection *section)
{
   GbWorkspacePrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(GB_IS_WORKSPACE_SECTION(section));

   priv = workspace->priv;

   if (priv->container) {
      gtk_stack_set_visible_child(GTK_STACK(priv->container->priv->stack),
                                  GTK_WIDGET(section));
   }

   g_object_notify_by_pspec(G_OBJECT(workspace),
                            gParamSpecs[PROP_CURRENT_SECTION]);
}

GbDocument *
gb_workspace_get_current_document (GbWorkspace *workspace)
{
   GbWorkspaceSection *section;

   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);

   if ((section = gb_workspace_get_current_section(workspace))) {
      return gb_workspace_section_get_current_document(section);
   }

   return NULL;
}

GbBackForwardList *
gb_workspace_get_back_forward_list (GbWorkspace *workspace)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);
   return workspace->priv->back_forward_list;
}

GbCommandStack *
gb_workspace_get_command_stack (GbWorkspace *workspace)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), NULL);
   return workspace->priv->command_stack;
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

   g_clear_object(&priv->project);
   priv->project = project ? g_object_ref(project) : NULL;

   gtk_container_remove(GTK_CONTAINER(workspace),
                        gtk_bin_get_child(GTK_BIN(workspace)));

   if (priv->project) {
      child = g_object_new(GB_TYPE_WORKSPACE_CONTAINER,
                           "visible", TRUE,
                           NULL);
      priv->container = GB_WORKSPACE_CONTAINER(child);
   } else {
      child = g_object_new(GB_TYPE_WORKSPACE_GREETER,
                           "visible", TRUE,
                           NULL);
      priv->container = NULL;
   }

   gtk_container_add(GTK_CONTAINER(workspace), child);

   g_object_notify_by_pspec(G_OBJECT(workspace), gParamSpecs[PROP_PROJECT]);
}

static void
gb_workspace_finalize (GObject *object)
{
   GbWorkspacePrivate *priv;

   priv = GB_WORKSPACE(object)->priv;

   g_clear_object(&priv->back_forward_list);
   g_clear_object(&priv->command_stack);
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
   case PROP_COMMAND_STACK:
      g_value_set_object(value, gb_workspace_get_command_stack(workspace));
      break;
   case PROP_CURRENT_DOCUMENT:
      g_value_set_object(value, gb_workspace_get_current_document(workspace));
      break;
   case PROP_CURRENT_SECTION:
      g_value_set_object(value, gb_workspace_get_current_section(workspace));
      break;
   case PROP_EDITOR:
      g_value_set_object(value, gb_workspace_get_editor(workspace));
      break;
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
   case PROP_CURRENT_SECTION:
      gb_workspace_set_current_section(workspace, g_value_get_object(value));
      break;
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
   g_type_class_add_private(object_class, sizeof(GbWorkspacePrivate));

   gParamSpecs[PROP_COMMAND_STACK] =
      g_param_spec_object("command-stack",
                          _("Command Stack"),
                          _("The command stack for undo/redo."),
                          GB_TYPE_COMMAND_STACK,
                          (G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_COMMAND_STACK,
                                   gParamSpecs[PROP_COMMAND_STACK]);

   gParamSpecs[PROP_CURRENT_DOCUMENT] =
      g_param_spec_object("current-document",
                          _("Current Document"),
                          _("The current document being edited or viewed."),
                          GB_TYPE_DOCUMENT,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CURRENT_DOCUMENT,
                                   gParamSpecs[PROP_CURRENT_DOCUMENT]);

   gParamSpecs[PROP_CURRENT_SECTION] =
      g_param_spec_object("current-section",
                          _("Current Section"),
                          _("The current section, editor, docs, etc."),
                          GB_TYPE_WORKSPACE_SECTION,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CURRENT_SECTION,
                                   gParamSpecs[PROP_CURRENT_SECTION]);

   gParamSpecs[PROP_EDITOR] =
      g_param_spec_object("editor",
                          _("Editor"),
                          _("The source code editor."),
                          GB_TYPE_WORKSPACE_EDITOR,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_EDITOR,
                                   gParamSpecs[PROP_EDITOR]);

   gParamSpecs[PROP_PROJECT] =
      g_param_spec_object("project",
                          _("Project"),
                          _("The project for the workspace."),
                          GB_TYPE_PROJECT,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PROJECT,
                                   gParamSpecs[PROP_PROJECT]);
}

static void
gb_workspace_init (GbWorkspace *workspace)
{
   GtkWidget *greeter;

   workspace->priv = G_TYPE_INSTANCE_GET_PRIVATE(workspace,
                                                 GB_TYPE_WORKSPACE,
                                                 GbWorkspacePrivate);

   g_object_set(workspace,
                "default-width", 800,
                "default-height", 600,
                "title", _("Builder"),
                NULL);

   workspace->priv->command_stack = gb_command_stack_new();

   greeter = g_object_new(GB_TYPE_WORKSPACE_GREETER,
                          "visible", TRUE,
                          NULL);
   gtk_container_add(GTK_CONTAINER(workspace), greeter);

   _gb_workspace_actions_init(workspace);
}

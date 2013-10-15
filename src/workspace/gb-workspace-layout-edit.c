/* gb-workspace-layout-edit.c
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

#include "gb-source-pane.h"
#include "gb-terminal-pane.h"
#include "gb-tree.h"
#include "gb-tree-builder.h"
#include "gb-tree-builder-project.h"
#include "gb-tree-node.h"
#include "gb-tree-node-project.h"
#include "gb-workspace-layout-edit.h"
#include "gb-workspace-pane-group.h"

G_DEFINE_TYPE(GbWorkspaceLayoutEdit,
              gb_workspace_layout_edit,
              GB_TYPE_WORKSPACE_LAYOUT)

struct _GbWorkspaceLayoutEditPrivate
{
   GList *groups;

   GtkWidget *browser;
   GtkWidget *browser_scroller;

   gboolean is_fullscreen;
};

static void
gb_workspace_layout_edit_load (GbWorkspaceLayout *layout,
                               GbProject         *project)
{
   GbWorkspaceLayoutEditPrivate *priv;
   GbWorkspaceLayoutEdit *edit = (GbWorkspaceLayoutEdit *)layout;
   GbTreeNode *node;

   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT_EDIT(edit));

   priv = edit->priv;

   if (!project) {
      g_object_set(priv->browser, "root", NULL, NULL);
      return;
   }

   node = g_object_new(GB_TYPE_TREE_NODE_PROJECT,
                       "project", project,
                       NULL);
   g_object_set(priv->browser, "root", node, NULL);
   g_object_unref(node);

#if 0
   g_object_bind_property(project, "name", priv->toolbar, "title",
                          G_BINDING_SYNC_CREATE);
#endif
}

static void
gb_workspace_layout_edit_add (GtkContainer *container,
                              GtkWidget    *child)
{
   GbWorkspaceLayoutEditPrivate *priv;
   GbWorkspaceLayoutEdit *edit = (GbWorkspaceLayoutEdit *)container;

   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT_EDIT(edit));

   priv = edit->priv;

   if (GB_IS_WORKSPACE_PANE_GROUP(child)) {
      if (priv->is_fullscreen) {
         gb_workspace_pane_group_fullscreen(GB_WORKSPACE_PANE_GROUP(child));
      }
   }

   if (GB_IS_WORKSPACE_PANE(child)) {
      gtk_container_add(GTK_CONTAINER(priv->groups->data), child);
   } else {
      GTK_CONTAINER_CLASS(gb_workspace_layout_edit_parent_class)->
         add(container, child);
   }
}

static void
gb_workspace_layout_edit_grab_focus (GtkWidget *widget)
{
   GbWorkspaceLayoutEdit *edit = (GbWorkspaceLayoutEdit *)widget;
   GList *iter;

   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT_EDIT(edit));

   for (iter = edit->priv->groups; iter; iter = iter->next) {
      gtk_widget_grab_focus(iter->data);
      return;
   }

   GTK_WIDGET_CLASS(gb_workspace_layout_edit_parent_class)->grab_focus(widget);
}

static void
gb_workspace_layout_edit_fullscreen (GbWorkspaceLayout *layout)
{
   GbWorkspaceLayoutEdit *edit = (GbWorkspaceLayoutEdit *)layout;
   GList *iter;

   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT_EDIT(edit));

   edit->priv->is_fullscreen = TRUE;

   for (iter = edit->priv->groups; iter; iter = iter->next) {
      gb_workspace_pane_group_fullscreen(iter->data);
   }

   gtk_widget_hide(edit->priv->browser_scroller);
}

static void
gb_workspace_layout_edit_unfullscreen (GbWorkspaceLayout *layout)
{
   GbWorkspaceLayoutEdit *edit = (GbWorkspaceLayoutEdit *)layout;
   GList *iter;

   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT_EDIT(edit));

   edit->priv->is_fullscreen = FALSE;

   for (iter = edit->priv->groups; iter; iter = iter->next) {
      gb_workspace_pane_group_unfullscreen(iter->data);
   }

   gtk_widget_show(edit->priv->browser_scroller);
}

static void
gb_workspace_layout_edit_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_layout_edit_parent_class)->finalize(object);
}

static void
gb_workspace_layout_edit_class_init (GbWorkspaceLayoutEditClass *klass)
{
   GbWorkspaceLayoutClass *layout_class;
   GtkContainerClass *container_class;
   GtkWidgetClass *widget_class;
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_layout_edit_finalize;
   g_type_class_add_private(object_class,
                            sizeof(GbWorkspaceLayoutEditPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->grab_focus = gb_workspace_layout_edit_grab_focus;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_workspace_layout_edit_add;

   layout_class = GB_WORKSPACE_LAYOUT_CLASS(klass);
   layout_class->load = gb_workspace_layout_edit_load;
   layout_class->fullscreen = gb_workspace_layout_edit_fullscreen;
   layout_class->unfullscreen = gb_workspace_layout_edit_unfullscreen;
}

static void
gb_workspace_layout_edit_init (GbWorkspaceLayoutEdit *edit)
{
   GbTreeBuilder *builder;
   GtkWidget *group;
   GtkWidget *hpaned;

   edit->priv = G_TYPE_INSTANCE_GET_PRIVATE(edit,
                                            GB_TYPE_WORKSPACE_LAYOUT_EDIT,
                                            GbWorkspaceLayoutEditPrivate);

   hpaned = g_object_new(GTK_TYPE_PANED,
                         "orientation", GTK_ORIENTATION_HORIZONTAL,
                         "position", 275,
                         "visible", TRUE,
                         NULL);
   gtk_container_add(GTK_CONTAINER(edit), hpaned);

   edit->priv->browser_scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                               "visible", TRUE,
                                               NULL);
   gtk_container_add(GTK_CONTAINER(hpaned), edit->priv->browser_scroller);

   edit->priv->browser = g_object_new(GB_TYPE_TREE,
                                      "visible", TRUE,
                                      NULL);
   gtk_container_add(GTK_CONTAINER(edit->priv->browser_scroller),
                     edit->priv->browser);

   builder = gb_tree_builder_project_new();
   gb_tree_add_builder(GB_TREE(edit->priv->browser), builder);

   group = g_object_new(GB_TYPE_WORKSPACE_PANE_GROUP,
                        "hexpand", TRUE,
                        "vexpand", TRUE,
                        "visible", TRUE,
                        NULL);
   gtk_container_add(GTK_CONTAINER(hpaned), group);

   edit->priv->groups = g_list_append(edit->priv->groups, group);
}

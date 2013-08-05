/* gb-workspace-pane-group.c
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

#include "gb-workspace-pane.h"
#include "gb-workspace-pane-group.h"

G_DEFINE_TYPE(GbWorkspacePaneGroup, gb_workspace_pane_group, GTK_TYPE_GRID)

struct _GbWorkspacePaneGroupPrivate
{
   GtkWidget *notebook;
};

enum
{
   PROP_0,
   LAST_PROP
};

#if 0
static GParamSpec *gParamSpecs[LAST_PROP];
#endif

static void
gb_workspace_pane_group_close_clicked (GtkButton            *button,
                                       GbWorkspacePaneGroup *group)
{
   GbWorkspacePaneGroupPrivate *priv;
   GtkWidget *child;

   g_return_if_fail(GTK_IS_WIDGET(button));
   g_return_if_fail(GB_IS_WORKSPACE_PANE_GROUP(group));

   priv = group->priv;

   /*
    * TODO: Check can-save.
    */

   child = g_object_get_data(G_OBJECT(button), "child");
   gtk_container_remove(GTK_CONTAINER(priv->notebook), child);
}

static void
gb_workspace_pane_group_add (GtkContainer *container,
                             GtkWidget    *child)
{
   GbWorkspacePaneGroupPrivate *priv;
   GbWorkspacePaneGroup *group = (GbWorkspacePaneGroup *)container;
   GtkTreeIter iter;
   GtkWidget *hbox;
   GtkWidget *close_button;
   GtkWidget *icon;
   GtkWidget *label;
   GtkWidget *spinner;

   g_return_if_fail(GB_IS_WORKSPACE_PANE_GROUP(group));
   g_return_if_fail(GTK_IS_WIDGET(child));

   priv = group->priv;

   if (GB_IS_WORKSPACE_PANE(child)) {
      hbox = g_object_new(GTK_TYPE_BOX,
                          "orientation", GTK_ORIENTATION_HORIZONTAL,
                          "spacing", 0,
                          "visible", TRUE,
                          NULL);

      spinner = g_object_new(GTK_TYPE_SPINNER,
                             "hexpand", FALSE,
                             "vexpand", FALSE,
                             "visible", FALSE,
                             NULL);
      g_object_bind_property(child, "busy", spinner, "active",
                             G_BINDING_SYNC_CREATE);
      g_object_bind_property(child, "busy", spinner, "visible",
                             G_BINDING_SYNC_CREATE);
      gtk_container_add_with_properties(GTK_CONTAINER(hbox), spinner,
                                        "padding", 3,
                                        NULL);

      icon = g_object_new(GTK_TYPE_IMAGE,
                          "hexpand", FALSE,
                          "icon-name", "text-x-generic",
                          "icon-size", GTK_ICON_SIZE_MENU,
                          "visible", TRUE,
                          NULL);
      g_object_bind_property(child, "icon-name", icon, "icon-name",
                             G_BINDING_SYNC_CREATE);
      gtk_container_add_with_properties(GTK_CONTAINER(hbox), icon,
                                        "padding", 3,
                                        NULL);

      label = g_object_new(GTK_TYPE_LABEL,
                           "ellipsize", PANGO_ELLIPSIZE_END,
                           "hexpand", TRUE,
                           "single-line-mode", TRUE,
                           "visible", TRUE,
                           "xalign", 0.0,
                           "xpad", 0,
                           "yalign", 0.5,
                           "ypad", 0,
                           NULL);
      g_object_bind_property(child, "title", label, "label",
                             G_BINDING_SYNC_CREATE);
      gtk_container_add(GTK_CONTAINER(hbox), label);

      close_button = g_object_new(GTK_TYPE_BUTTON,
                                  "hexpand", FALSE,
                                  "focus-on-click", FALSE,
                                  "name", "nautilus-tab-close-button",
                                  "relief", GTK_RELIEF_NONE,
                                  "visible", TRUE,
                                  NULL);
      g_object_set_data(G_OBJECT(close_button), "child", child);
      g_signal_connect(close_button, "clicked",
                       G_CALLBACK(gb_workspace_pane_group_close_clicked),
                       group);
      gtk_container_add(GTK_CONTAINER(hbox), close_button);

      icon = g_object_new(GTK_TYPE_IMAGE,
                          "icon-name", "window-close-symbolic",
                          "icon-size", GTK_ICON_SIZE_MENU,
                          "tooltip-text", _("Close tab"),
                          "visible", TRUE,
                          NULL);
      gtk_container_add(GTK_CONTAINER(close_button), icon);

      gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook), child, hbox);
   } else {
      GTK_CONTAINER_CLASS(gb_workspace_pane_group_parent_class)->
         add(container, child);
   }
}

static void
gb_workspace_pane_group_grab_focus (GtkWidget *widget)
{
   GbWorkspacePaneGroup *group = (GbWorkspacePaneGroup *)widget;
   GList *children;
   GList *iter;

   g_return_if_fail(GB_IS_WORKSPACE_PANE_GROUP(group));

   children = gtk_container_get_children(GTK_CONTAINER(group->priv->notebook));
   for (iter = children; iter; iter = iter->next) {
      if (gtk_widget_get_visible(iter->data)) {
         gtk_widget_grab_focus(iter->data);
         break;
      }
   }

   g_list_free(children);
}

static void
gb_workspace_pane_group_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_pane_group_parent_class)->finalize(object);
}

static void
gb_workspace_pane_group_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
   //GbWorkspacePaneGroup *group = GB_WORKSPACE_PANE_GROUP(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_pane_group_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
   //GbWorkspacePaneGroup *group = GB_WORKSPACE_PANE_GROUP(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_pane_group_class_init (GbWorkspacePaneGroupClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_pane_group_finalize;
   object_class->get_property = gb_workspace_pane_group_get_property;
   object_class->set_property = gb_workspace_pane_group_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspacePaneGroupPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->grab_focus = gb_workspace_pane_group_grab_focus;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_workspace_pane_group_add;
}

static void
gb_workspace_pane_group_init (GbWorkspacePaneGroup *group)
{
   GbWorkspacePaneGroupPrivate *priv;

   group->priv = G_TYPE_INSTANCE_GET_PRIVATE(group,
                                             GB_TYPE_WORKSPACE_PANE_GROUP,
                                             GbWorkspacePaneGroupPrivate);

   priv = group->priv;

   priv->notebook = g_object_new(GTK_TYPE_NOTEBOOK,
                                 "hexpand", TRUE,
                                 "show-border", FALSE,
                                 "show-tabs", TRUE,
                                 "vexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(group), priv->notebook);
}

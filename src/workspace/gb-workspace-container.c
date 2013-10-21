/* gb-workspace-container.c
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

#include "gb-application.h"
#include "gb-gtk.h"
#include "gb-workspace.h"
#include "gb-workspace-container.h"
#include "gb-workspace-container-private.h"
#include "gb-workspace-docs.h"
#include "gb-workspace-editor.h"

#define UI_RESOURCE_PATH "/org/gnome/Builder/ui/gb-workspace-container.ui"

G_DEFINE_TYPE(GbWorkspaceContainer, gb_workspace_container, GTK_TYPE_GRID)

static void
gb_workspace_container_parent_set (GtkWidget *widget,
                                   GtkWidget *old_parent)
{
   GbWorkspaceContainerPrivate *priv;
   GtkWidget *toplevel;

   g_return_if_fail(GB_WORKSPACE_CONTAINER(widget));

   priv = GB_WORKSPACE_CONTAINER(widget)->priv;

   toplevel = gtk_widget_get_toplevel(widget);
   if (!GB_IS_WORKSPACE(toplevel)) {
      return;
   }

   gtk_window_set_titlebar(GTK_WINDOW(toplevel), priv->header_bar);
}

static void
gb_workspace_container_finalize (GObject *object)
{
   GbWorkspaceContainerPrivate *priv;

   priv = GB_WORKSPACE_CONTAINER(object)->priv;

   g_clear_object(&priv->header_bar);
   g_clear_object(&priv->menu_model);

   G_OBJECT_CLASS(gb_workspace_container_parent_class)->finalize(object);
}

static void
gb_workspace_container_class_init (GbWorkspaceContainerClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_container_finalize;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceContainerPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->parent_set = gb_workspace_container_parent_set;
}

static void
gb_workspace_container_init (GbWorkspaceContainer *container)
{
   GbWorkspaceContainerPrivate *priv;
   GtkWidget *icon;
   GtkWidget *hbox;
   gboolean rtl;

   container->priv = G_TYPE_INSTANCE_GET_PRIVATE(container,
                                                 GB_TYPE_WORKSPACE_CONTAINER,
                                                 GbWorkspaceContainerPrivate);

   priv = container->priv;

   rtl = gtk_widget_get_direction(GTK_WIDGET(container)) == GTK_TEXT_DIR_RTL;

   priv->stack = g_object_new(GTK_TYPE_STACK,
                              "hexpand", TRUE,
                              "transition-type",
                                 GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT,
                              "vexpand", TRUE,
                              "visible", TRUE,
                              NULL);
   gtk_container_add(GTK_CONTAINER(container), priv->stack);

   priv->stack_switcher = g_object_new(GTK_TYPE_STACK_SWITCHER,
                                       "stack", priv->stack,
                                       "visible", TRUE,
                                       NULL);

   priv->header_bar = g_object_new(GTK_TYPE_HEADER_BAR,
                                   "show-close-button", TRUE,
                                   "custom-title", priv->stack_switcher,
                                   "visible", TRUE,
                                   NULL);
   priv->header_bar = g_object_ref(priv->header_bar);

   priv->ui = g_object_new(GTK_TYPE_LABEL,
                           "label", "TODO: UI",
                           "visible", TRUE,
                           NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->stack), priv->ui,
                                     "name", "ui",
                                     "position", 0,
                                     "title", _("UI"),
                                     NULL);

   priv->edit = g_object_new(GB_TYPE_WORKSPACE_EDITOR,
                             "hexpand", TRUE,
                             "vexpand", TRUE,
                             "visible", TRUE,
                             NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->stack), priv->edit,
                                     "name", "edit",
                                     "position", 1,
                                     "title", _("Edit"),
                                     NULL);

   priv->debug = g_object_new(GTK_TYPE_LABEL,
                              "label", "TODO: Debug",
                              "visible", TRUE,
                              NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->stack), priv->debug,
                                     "name", "debug",
                                     "position", 2,
                                     "title", _("Debug"),
                                     NULL);

   priv->git = g_object_new(GTK_TYPE_LABEL,
                            "label", "TODO: Git",
                            "visible", TRUE,
                            NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->stack), priv->git,
                                     "name", "git",
                                     "position", 3,
                                     "title", _("Git"),
                                     NULL);

   priv->docs = g_object_new(GB_TYPE_WORKSPACE_DOCS,
                             "visible", TRUE,
                             NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->stack), priv->docs,
                                     "name", "docs",
                                     "position", 4,
                                     "title", _("Docs"),
                                     NULL);

   hbox = g_object_new(GTK_TYPE_BOX,
                       "orientation", GTK_ORIENTATION_HORIZONTAL,
                       "visible", TRUE,
                       NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(hbox), "linked");
   gtk_header_bar_pack_start(GTK_HEADER_BAR(priv->header_bar), hbox);

   priv->build = g_object_new(GTK_TYPE_BUTTON,
                              "label", _("Build"),
                              "visible", TRUE,
                              NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->build),
                               "suggested-action");
   gtk_container_add(GTK_CONTAINER(hbox), priv->build);

   icon = g_object_new(GTK_TYPE_IMAGE,
                       "icon-name", rtl ?
                                    "media-playback-start-rtl-symbolic" :
                                    "media-playback-start-symbolic",
                       "icon-size", GTK_ICON_SIZE_MENU,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       NULL);
   priv->run = g_object_new(GTK_TYPE_BUTTON,
                            "child", icon,
                            "visible", TRUE,
                            NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->run),
                               "suggested-action");
   gtk_container_add(GTK_CONTAINER(hbox), priv->run);

   icon = g_object_new(GTK_TYPE_IMAGE,
                       "icon-name", "edit-find-symbolic",
                       "icon-size", GTK_ICON_SIZE_MENU,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       NULL);
   priv->search = g_object_new(GTK_TYPE_TOGGLE_BUTTON,
                               "child", icon,
                               "visible", TRUE,
                               NULL);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header_bar), priv->search);

   priv->menu_model =
      gb_gtk_builder_load_and_get_object(UI_RESOURCE_PATH, "menu");

   icon = g_object_new(GTK_TYPE_IMAGE,
                       "icon-name", "emblem-system-symbolic",
                       "icon-size", GTK_ICON_SIZE_MENU,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       NULL);
   priv->menu_button = g_object_new(GTK_TYPE_MENU_BUTTON,
                                    "child", icon,
                                    "direction", GTK_ARROW_DOWN,
                                    "menu-model", priv->menu_model,
                                    "visible", TRUE,
                                    NULL);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header_bar),
                           priv->menu_button);
   g_object_set(gtk_menu_button_get_popup(GTK_MENU_BUTTON(priv->menu_button)),
                "halign", GTK_ALIGN_END,
                NULL);
}

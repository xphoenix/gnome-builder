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

#include "gb-debugger-section.h"
#include "gb-designer-section.h"
#include "gb-docs-section.h"
#include "gb-editor-section.h"
#include "gb-git-section.h"
#include "gb-gtk.h"
#include "gb-workspace-container.h"

#define MENU_UI_PATH "/org/gnome/Builder/ui/gb-workspace-menu.ui"
#define MENU_UI_NAME "menu"

struct _GbWorkspaceContainerPrivate
{
   GtkWidget *build;
   GtkWidget *debugger;
   GtkWidget *designer;
   GtkWidget *docs;
   GtkWidget *editor;
   GtkWidget *git;
   GtkWidget *header;
   GtkWidget *menu_button;
   GtkWidget *stack;
   GtkWidget *stack_switcher;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE(GbWorkspaceContainer,
                        gb_workspace_container,
                        GTK_TYPE_BIN,
                        G_ADD_PRIVATE(GbWorkspaceContainer))

static void
register_actions (GbWorkspaceSection *section,
                  const gchar        *name)
{
   GActionGroup *actions;
   GtkWidget *toplevel;

   g_return_if_fail(GB_IS_WORKSPACE_SECTION(section));

   actions = gb_workspace_section_get_actions(section);
   toplevel = gtk_widget_get_toplevel(GTK_WIDGET(section));

   if (GTK_IS_APPLICATION_WINDOW(toplevel)) {
      gtk_widget_insert_action_group(toplevel, name, actions);
   }
}

static void
gb_workspace_container_parent_set (GtkWidget *widget,
                                   GtkWidget *old_parent)
{
   GbWorkspaceContainerPrivate *priv;
   GtkWidget *toplevel;
   GtkWidget *child;

   g_return_if_fail(GB_IS_WORKSPACE_CONTAINER(widget));

   priv = GB_WORKSPACE_CONTAINER(widget)->priv;

   toplevel = gtk_widget_get_toplevel(widget);
   if (GTK_IS_WINDOW(toplevel)) {
      gtk_window_set_titlebar(GTK_WINDOW(toplevel), priv->header);
   }

   register_actions(GB_WORKSPACE_SECTION(priv->debugger), "debugger");
   register_actions(GB_WORKSPACE_SECTION(priv->designer), "designer");
   register_actions(GB_WORKSPACE_SECTION(priv->editor), "editor");
   register_actions(GB_WORKSPACE_SECTION(priv->git), "git");
   register_actions(GB_WORKSPACE_SECTION(priv->docs), "docs");

   child = gtk_stack_get_visible_child(GTK_STACK(priv->stack));
   register_actions(GB_WORKSPACE_SECTION(child), "section");
}

static void
on_section_changed (GbWorkspaceContainer *container,
                    GParamSpec           *pspec,
                    GtkStack             *stack)
{
   GtkWidget *child;

   g_return_if_fail(GB_IS_WORKSPACE_CONTAINER(container));
   g_return_if_fail(GTK_IS_STACK(stack));

   child = gtk_stack_get_visible_child(stack);

   if (child) {
      register_actions(GB_WORKSPACE_SECTION(child), "section");
   }
}

static void
gb_workspace_container_finalize (GObject *object)
{
   GbWorkspaceContainerPrivate *priv = GB_WORKSPACE_CONTAINER(object)->priv;

   g_clear_object(&priv->header);

   G_OBJECT_CLASS(gb_workspace_container_parent_class)->finalize(object);
}

static void
gb_workspace_container_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
   //GbWorkspaceContainer *container = GB_WORKSPACE_CONTAINER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_container_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
   //GbWorkspaceContainer *container = GB_WORKSPACE_CONTAINER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_container_class_init (GbWorkspaceContainerClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_container_finalize;
   object_class->get_property = gb_workspace_container_get_property;
   object_class->set_property = gb_workspace_container_set_property;

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->parent_set = gb_workspace_container_parent_set;
}

static void
gb_workspace_container_init (GbWorkspaceContainer *container)
{
   GbWorkspaceContainerPrivate *priv;
   GMenuModel *menu_model;
   GtkWidget *icon;
   GtkMenu *popup;

   container->priv = gb_workspace_container_get_instance_private(container);

   priv = container->priv;

   priv->stack =
      g_object_new(GTK_TYPE_STACK,
                   "transition-type", GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT,
                   "visible", TRUE,
                   NULL);
   gtk_container_add(GTK_CONTAINER(container), priv->stack);

   priv->stack_switcher = g_object_new(GTK_TYPE_STACK_SWITCHER,
                                       "stack", priv->stack,
                                       "visible", TRUE,
                                       NULL);
   g_signal_connect_object(priv->stack,
                           "notify::visible-child",
                           G_CALLBACK(on_section_changed),
                           container,
                           (G_CONNECT_SWAPPED | G_CONNECT_AFTER));

   priv->header = g_object_new(GTK_TYPE_HEADER_BAR,
                               "custom-title", priv->stack_switcher,
                               "show-close-button", TRUE,
                               "visible", TRUE,
                               NULL);
   priv->header = g_object_ref(priv->header);

   priv->build = g_object_new(GTK_TYPE_BUTTON,
                              "hexpand", FALSE,
                              "label", _("Build"),
                              "valign", GTK_ALIGN_CENTER,
                              "visible", TRUE,
                              NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->build),
                               "suggested-action");
   gtk_header_bar_pack_start(GTK_HEADER_BAR(priv->header), priv->build);

   icon = g_object_new(GTK_TYPE_IMAGE,
                       "icon-name", "emblem-system-symbolic",
                       "icon-size", GTK_ICON_SIZE_MENU,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       NULL);

   menu_model = gb_gtk_builder_load_and_get_object(MENU_UI_PATH, MENU_UI_NAME);
   priv->menu_button = g_object_new(GTK_TYPE_MENU_BUTTON,
                                    "child", icon,
                                    "direction", GTK_ARROW_DOWN,
                                    "menu-model", menu_model,
                                    "visible", TRUE,
                                    NULL);
   popup = gtk_menu_button_get_popup(GTK_MENU_BUTTON(priv->menu_button));
   gtk_widget_set_halign(GTK_WIDGET(popup), GTK_ALIGN_END);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header),
                           priv->menu_button);
   g_object_unref(menu_model);

   priv->editor = g_object_new(GB_TYPE_EDITOR_SECTION,
                             "visible", TRUE,
                             NULL);
   gtk_stack_add_titled(GTK_STACK(priv->stack), priv->editor, "editor", _("Edit"));
   gtk_stack_set_visible_child(GTK_STACK(priv->stack), priv->editor);

   priv->debugger = g_object_new(GB_TYPE_DEBUGGER_SECTION,
                                 "visible", TRUE,
                                 NULL);
   gtk_stack_add_titled(GTK_STACK(priv->stack), priv->debugger, "debug",
                        _("Debug"));

   priv->git = g_object_new(GB_TYPE_GIT_SECTION,
                             "visible", TRUE,
                             NULL);
   gtk_stack_add_titled(GTK_STACK(priv->stack), priv->git, "git", _("Git"));

   priv->docs = g_object_new(GB_TYPE_DOCS_SECTION,
                             "visible", TRUE,
                             NULL);
   gtk_stack_add_titled(GTK_STACK(priv->stack), priv->docs, "docs", _("Docs"));

   priv->designer = g_object_new(GB_TYPE_DESIGNER_SECTION,
                                 "visible", TRUE,
                                 NULL);
   gtk_stack_add_titled(GTK_STACK(priv->stack), priv->designer, "ui", _("UI"));
   gtk_container_child_set(GTK_CONTAINER(priv->stack),
                           priv->designer,
                           "position", 0,
                           NULL);
}

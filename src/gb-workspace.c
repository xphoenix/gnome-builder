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

#include <gd/gd-header-bar.h>
#include <glib/gi18n.h>

#include "gb-application.h"
#include "gb-search-provider.h"
#include "gb-workspace.h"
#include "gb-workspace-layout.h"
#include "gb-workspace-layout-edit.h"
#include "gb-workspace-layout-splash.h"
#include "gb-workspace-layout-switcher.h"
#include "gb-workspace-pane.h"

struct _GbWorkspacePrivate
{
   GbProject *project;

   GtkWidget *edit;
   GtkWidget *layout;
   GtkWidget *notebook;
   GtkWidget *splash;
   GtkWidget *switcher;
   GtkWidget *toolbar;
   GtkWidget *vbox;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

G_DEFINE_TYPE_WITH_PRIVATE(GbWorkspace, gb_workspace, GTK_TYPE_APPLICATION_WINDOW)

/**
 * gb_workspace_get_project:
 * @workspace: (in): A #GbWorkspace.
 *
 * Fetches the "project" property.
 *
 * Returns: (transfer none): A #GbProject or %NULL.
 */
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
   GbWorkspaceLayoutClass *klass = NULL;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(!project || GB_IS_PROJECT(project));

   priv = workspace->priv;

   if (priv->layout) {
      klass = GB_WORKSPACE_LAYOUT_GET_CLASS(priv->layout);
   }

   if (priv->layout && priv->project && klass->unload) {
      klass->unload(GB_WORKSPACE_LAYOUT(priv->layout), priv->project);
   }

   g_clear_object(&priv->project);

   if (project) {
      priv->project = g_object_ref(project);
   }

   if (priv->layout && priv->project && klass->load) {
      klass->load(GB_WORKSPACE_LAYOUT(priv->layout), priv->project);
   }
}

#if 0
void
gb_workspace_set_layout (GbWorkspace       *workspace,
                         GbWorkspaceLayout *layout)
{
   GbWorkspaceLayoutClass *klass;
   GbWorkspacePrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(!layout || GB_IS_WORKSPACE_LAYOUT(layout));

   priv = workspace->priv;

   g_object_set(priv->toolbar, "visible", !!layout, NULL);

   if (!layout) {
      layout = g_object_new(GB_TYPE_WORKSPACE_LAYOUT_SPLASH,
                            "visible", TRUE,
                            NULL);
   }

   if (priv->layout && priv->project) {
      klass = GB_WORKSPACE_LAYOUT_GET_CLASS(priv->layout);
      if (klass->unload) {
         klass->unload(GB_WORKSPACE_LAYOUT(priv->layout), priv->project);
      }
   }

   if (priv->layout) {
      gtk_container_remove(GTK_CONTAINER(priv->vbox), priv->layout);
   }

   priv->layout = GTK_WIDGET(layout);

   gtk_container_add(GTK_CONTAINER(priv->vbox), priv->layout);

   if (priv->project) {
      klass = GB_WORKSPACE_LAYOUT_GET_CLASS(layout);
      if (klass->load) {
         klass->load(layout, priv->project);
      }
   }
}
#endif

void
gb_workspace_set_mode (GbWorkspace     *workspace,
                       GbWorkspaceMode  mode)
{
   GbWorkspacePrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   gtk_notebook_set_current_page(GTK_NOTEBOOK(priv->notebook),
                                 (guint)mode);

   priv->layout = gtk_notebook_get_nth_page(GTK_NOTEBOOK(priv->notebook),
                                            (guint)mode);

   g_object_set(priv->toolbar,
                "visible", (mode != GB_WORKSPACE_SPLASH),
                NULL);
}

static void
gb_workspace_add (GtkContainer *container,
                  GtkWidget    *child)
{
   GbWorkspacePrivate *priv = GB_WORKSPACE(container)->priv;

   if (GB_IS_WORKSPACE_PANE(child)) {
      gtk_container_add(GTK_CONTAINER(priv->layout), child);
   } else {
      GTK_CONTAINER_CLASS(gb_workspace_parent_class)->add(container, child);
   }
}

static void
gb_workspace_pane_search (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
   GbWorkspace *workspace = user_data;
   GtkWidget *widget;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   for (widget = gtk_window_get_focus(GTK_WINDOW(workspace));
        widget;
        widget = gtk_widget_get_parent(widget)) {
      if (GB_IS_SEARCH_PROVIDER(widget)) {
         gb_search_provider_focus_search(GB_SEARCH_PROVIDER(widget));
         return;
      }
   }

   /*
    * If we failed to find a search provider in the current focus chain,
    * modify the focused widget and try again.
    */

   gtk_widget_grab_focus(workspace->priv->layout);

   for (widget = gtk_window_get_focus(GTK_WINDOW(workspace));
        widget;
        widget = gtk_widget_get_parent(widget)) {
      if (GB_IS_SEARCH_PROVIDER(widget)) {
         gb_search_provider_focus_search(GB_SEARCH_PROVIDER(widget));
         return;
      }
   }
}

static void
gb_workspace_finalize (GObject *object)
{
   GbWorkspacePrivate *priv;

   priv = GB_WORKSPACE(object)->priv;

   g_clear_object(&priv->project);

   G_OBJECT_CLASS(gb_workspace_parent_class)->finalize(object);
}

static void
gb_workspace_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
   //GbWorkspace *workspace = GB_WORKSPACE(object);

   switch (prop_id) {
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
   //GbWorkspace *workspace = GB_WORKSPACE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_class_init (GbWorkspaceClass *klass)
{
   GObjectClass *object_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_finalize;
   object_class->get_property = gb_workspace_get_property;
   object_class->set_property = gb_workspace_set_property;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_workspace_add;

   g_io_extension_point_register("org.gnome.Builder.Workspace");
}

static void
gb_workspace_init_actions (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   static const GActionEntry entries[] = {
      { "pane-search", gb_workspace_pane_search },
   };

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   g_action_map_add_action_entries(G_ACTION_MAP(workspace),
                                   entries,
                                   G_N_ELEMENTS(entries),
                                   workspace);
}

static void
gb_workspace_init (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   GMenuModel *menu;
   GtkBuilder *builder;
   GtkWidget *image;
   GtkWidget *button;

   workspace->priv = G_TYPE_INSTANCE_GET_PRIVATE(workspace,
                                                 GB_TYPE_WORKSPACE,
                                                 GbWorkspacePrivate);

   priv = workspace->priv;

   gb_workspace_init_actions(workspace);

   g_object_set(workspace,
                "hide-titlebar-when-maximized", TRUE,
                NULL);

   priv->vbox =
      g_object_new(GTK_TYPE_BOX,
                   "orientation", GTK_ORIENTATION_VERTICAL,
                   "vexpand", TRUE,
                   "visible", TRUE,
                   NULL);
   gtk_container_add(GTK_CONTAINER(workspace), priv->vbox);

   priv->switcher =
      g_object_new(GB_TYPE_WORKSPACE_LAYOUT_SWITCHER,
                   "visible", TRUE,
                   NULL);

   priv->toolbar =
      g_object_new(GD_TYPE_HEADER_BAR,
                   "custom-title", workspace->priv->switcher,
                   "name", "GbHeaderBar",
                   "height-request", 48,
                   "vexpand", FALSE,
                   "visible", TRUE,
                   NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->toolbar),
                               "builder-header");
   gtk_container_add(GTK_CONTAINER(priv->vbox), priv->toolbar);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-name", "edit-find-symbolic",
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "visible", TRUE,
                        NULL);
   button = g_object_new(GTK_TYPE_TOGGLE_BUTTON,
                         "image", image,
                         "hexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   gd_header_bar_pack_end(GD_HEADER_BAR(priv->toolbar), button);

   builder = gtk_builder_new();
   gtk_builder_add_from_string(builder,
                               "<interface>"
                               "  <menu id='menubar'>"
                               "    <section>"
                               "      <item>"
                               "        <attribute name='label' translatable='yes'>_Find</attribute>"
                               "        <attribute name='action'>win.pane-search</attribute>"
                               "        <attribute name='accel'>&lt;Primary&gt;f</attribute>"
                               "      </item>"
                               "    </section>"
                               "  </menu>"
                               "</interface>", -1, NULL);

   menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menubar"));

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-name", "emblem-system-symbolic",
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "visible", TRUE,
                        NULL);
   button = g_object_new(GTK_TYPE_MENU_BUTTON,
                         "direction", GTK_ARROW_DOWN,
                         "image", image,
                         "hexpand", FALSE,
                         "menu-model", menu,
                         "visible", TRUE,
                         NULL);
   gd_header_bar_pack_end(GD_HEADER_BAR(priv->toolbar), button);

   {
      GtkWidget *box;

      box = g_object_new(GTK_TYPE_BOX,
                         "hexpand", FALSE,
                         "orientation", GTK_ORIENTATION_HORIZONTAL,
                         "visible", TRUE,
                         NULL);
      gtk_style_context_add_class(gtk_widget_get_style_context(box),
                                  "linked");
      gd_header_bar_pack_start(GD_HEADER_BAR(priv->toolbar), box);

      button = g_object_new(GTK_TYPE_BUTTON,
                            "label", _("Build"),
                            "hexpand", FALSE,
                            "visible", TRUE,
                            "width-request", 75,
                            NULL);
      gtk_style_context_add_class(gtk_widget_get_style_context(button),
                                  "suggested-action");
      gtk_container_add(GTK_CONTAINER(box), button);

      image = g_object_new(GTK_TYPE_IMAGE,
                           "icon-name", "media-playback-start",
                           "icon-size", GTK_ICON_SIZE_MENU,
                           "visible", TRUE,
                           NULL);
      button = g_object_new(GTK_TYPE_BUTTON,
                            "child", image,
                            "hexpand", FALSE,
                            "visible", TRUE,
                            NULL);
      gtk_style_context_add_class(gtk_widget_get_style_context(button),
                                  "suggested-action");
      gtk_container_add(GTK_CONTAINER(box), button);
   }

   priv->notebook =
      g_object_new(GTK_TYPE_NOTEBOOK,
                   "hexpand", TRUE,
                   "show-tabs", FALSE,
                   "show-border", FALSE,
                   "vexpand", TRUE,
                   "visible", TRUE,
                   NULL);
   gtk_container_add(GTK_CONTAINER(priv->vbox), priv->notebook);

   priv->splash = g_object_new(GB_TYPE_WORKSPACE_LAYOUT_SPLASH,
                               "visible", TRUE,
                               NULL);
   gtk_container_add(GTK_CONTAINER(priv->notebook), priv->splash);

   priv->edit = g_object_new(GB_TYPE_WORKSPACE_LAYOUT_EDIT,
                             "visible", TRUE,
                             NULL);
   gtk_container_add(GTK_CONTAINER(priv->notebook), priv->edit);

   gb_workspace_set_mode(workspace, GB_WORKSPACE_SPLASH);

   g_object_unref(builder);
}

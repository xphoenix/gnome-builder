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

#include "gb-action.h"
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

   GtkWidget *build;
   GtkWidget *current_pane;
   GtkWidget *edit;
   GtkWidget *header;
   GtkWidget *layout;
   GtkWidget *menu;
   GtkWidget *notebook;
   GtkWidget *run;
   GtkWidget *search;
   GtkWidget *splash;
   GtkWidget *switcher;
   GtkWidget *vbox;
};

enum
{
   PROP_0,
   PROP_PROJECT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

G_DEFINE_TYPE_WITH_PRIVATE(GbWorkspace,
                           gb_workspace,
                           GTK_TYPE_APPLICATION_WINDOW)

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
      gtk_widget_show(priv->build);
      gtk_widget_show(priv->menu);
      gtk_widget_show(priv->run);
      gtk_widget_show(priv->search);
      gtk_header_bar_set_custom_title(GTK_HEADER_BAR(priv->header),
                                      priv->switcher);
   } else {
      gtk_widget_hide(priv->build);
      gtk_widget_hide(priv->menu);
      gtk_widget_hide(priv->run);
      gtk_widget_hide(priv->search);
      gtk_header_bar_set_title(GTK_HEADER_BAR(priv->header),
                               _("Select a Project"));
   }

   if (priv->layout && priv->project && klass->load) {
      klass->load(GB_WORKSPACE_LAYOUT(priv->layout), priv->project);
   }
}

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
gb_workspace_pane_save_cb (GbWorkspacePane *pane,
                           GAsyncResult    *result,
                           gpointer         user_data)
{
   GbWorkspace *workspace = user_data;
   GError *error = NULL;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   if (!gb_workspace_pane_save_finish(pane, result, &error)) {
      g_printerr("%s\n", error->message);
      g_error_free(error);
   }

   g_object_unref(workspace);
}

static void
gb_workspace_pane_save (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
   GbWorkspacePrivate *priv;
   GbWorkspacePane *pane;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   priv = workspace->priv;

   if (GB_IS_WORKSPACE_PANE(priv->current_pane)) {
      pane = GB_WORKSPACE_PANE(priv->current_pane);
      if (gb_workspace_pane_get_can_save(pane)) {
         gb_workspace_pane_save_async(pane,
                                      NULL, /* TODO: Cancellable */
                                      (GAsyncReadyCallback)gb_workspace_pane_save_cb,
                                      g_object_ref(workspace));
      }
   }
}

static void
gb_workspace_pane_search (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
   GbWorkspacePrivate *priv;
   GbSearchProvider *provider;
   GbWorkspace *workspace = user_data;
   GtkWidget *widget;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   priv = workspace->priv;

   if (GB_IS_WORKSPACE_PANE(priv->current_pane)) {
      if (GB_IS_SEARCH_PROVIDER(priv->current_pane)) {
         provider = GB_SEARCH_PROVIDER(priv->current_pane);
         gb_search_provider_focus_search(provider);
      }
   }
}

static void
gb_workspace_update_actions (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   GAction *action;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   action = g_action_map_lookup_action(G_ACTION_MAP(workspace), "pane-search");
   g_simple_action_set_enabled(G_SIMPLE_ACTION(action),
                               GB_IS_WORKSPACE_PANE(priv->current_pane));
}

static void
gb_workspace_set_focus (GtkWindow *window,
                        GtkWidget *widget)
{
   GbWorkspace *workspace = (GbWorkspace *)window;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   GTK_WINDOW_CLASS(gb_workspace_parent_class)->set_focus(window, widget);

   for (; widget; widget = gtk_widget_get_parent(widget)) {
      if (GB_IS_WORKSPACE_PANE(widget)) {
         workspace->priv->current_pane = widget;
         break;
      }
   }

   gb_workspace_update_actions(workspace);
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
   GtkWindowClass *window_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_finalize;
   object_class->get_property = gb_workspace_get_property;
   object_class->set_property = gb_workspace_set_property;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_workspace_add;

   window_class = GTK_WINDOW_CLASS(klass);
   window_class->set_focus = gb_workspace_set_focus;

   gParamSpecs[PROP_PROJECT] =
      g_param_spec_object("project",
                          _("Project"),
                          _("The project we are working with."),
                          GB_TYPE_PROJECT,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PROJECT,
                                   gParamSpecs[PROP_PROJECT]);

   g_io_extension_point_register("org.gnome.Builder.Workspace");
}

static void
gb_workspace_init_actions (GbWorkspace *workspace)
{
   static const GActionEntry entries[] = {
      { "pane-save", gb_workspace_pane_save },
      { "pane-search", gb_workspace_pane_search },
   };

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   g_action_map_add_action_entries(G_ACTION_MAP(workspace),
                                   entries,
                                   G_N_ELEMENTS(entries),
                                   workspace);

   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>f", "win.pane-search", NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>s", "win.pane-save", NULL);
}

static void
gb_workspace_init (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   GMenuModel *menu;
   GtkBuilder *builder;
   GtkWidget *box;
   GtkWidget *image;

   workspace->priv = G_TYPE_INSTANCE_GET_PRIVATE(workspace,
                                                 GB_TYPE_WORKSPACE,
                                                 GbWorkspacePrivate);

   priv = workspace->priv;

   gb_workspace_init_actions(workspace);

   gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(workspace), FALSE);

   priv->header = g_object_new(GTK_TYPE_HEADER_BAR,
                               "title", _("Select a Project"),
                               "name", "workspace-header-bar",
                               "height-request", 48,
                               "vexpand", FALSE,
                               "visible", TRUE,
                               NULL);
   gtk_window_set_titlebar(GTK_WINDOW(workspace), priv->header);

   priv->vbox = g_object_new(GTK_TYPE_BOX,
                             "orientation", GTK_ORIENTATION_VERTICAL,
                             "vexpand", TRUE,
                             "visible", TRUE,
                             NULL);
   gtk_container_add(GTK_CONTAINER(workspace), priv->vbox);

   priv->switcher = g_object_new(GB_TYPE_WORKSPACE_LAYOUT_SWITCHER,
                                 "visible", TRUE,
                                 NULL);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-name", "edit-find-symbolic",
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "visible", TRUE,
                        NULL);
   priv->search = g_object_new(GTK_TYPE_TOGGLE_BUTTON,
                               "image", image,
                               "hexpand", FALSE,
                               "visible", FALSE,
                               NULL);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header), priv->search);

   builder = gtk_builder_new();
   gtk_builder_add_from_string(builder,
                               "<interface>"
                               "  <menu id='menubar'>"
                               "    <section>"
                               "      <item>"
                               "        <attribute name='label' translatable='yes'>_Save</attribute>"
                               "        <attribute name='action'>win.pane-save</attribute>"
                               "        <attribute name='accel'>&lt;Primary&gt;s</attribute>"
                               "      </item>"
                               "    </section>"
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
   priv->menu = g_object_new(GTK_TYPE_MENU_BUTTON,
                             "direction", GTK_ARROW_DOWN,
                             "image", image,
                             "hexpand", FALSE,
                             "menu-model", menu,
                             "visible", FALSE,
                             NULL);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header), priv->menu);

   box = g_object_new(GTK_TYPE_BOX,
                      "hexpand", FALSE,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(box),
                               "linked");
   gtk_header_bar_pack_start(GTK_HEADER_BAR(priv->header), box);

   priv->build = g_object_new(GTK_TYPE_BUTTON,
                              "label", _("Build"),
                              "hexpand", FALSE,
                              "visible", FALSE,
                              "width-request", 75,
                              NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->build),
                               "suggested-action");
   gtk_container_add(GTK_CONTAINER(box), priv->build);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-name", "media-playback-start",
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "visible", TRUE,
                        NULL);
   priv->run = g_object_new(GTK_TYPE_BUTTON,
                            "child", image,
                            "hexpand", FALSE,
                            "visible", FALSE,
                            NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->run),
                               "suggested-action");
   gtk_container_add(GTK_CONTAINER(box), priv->run);

   priv->notebook = g_object_new(GTK_TYPE_NOTEBOOK,
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

/* gb-workspace.c
 *
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

#include "gb-animation.h"
#include "gb-application.h"
#include "gb-search-completion.h"
#include "gb-source-pane.h"
#include "gb-terminal-pane.h"
#include "gb-workspace.h"
#include "gb-workspace-layout.h"
#include "gb-workspace-layout-edit.h"
#include "gb-workspace-layout-docs.h"
#include "gb-workspace-layout-splash.h"
#include "gb-workspace-layout-switcher.h"
#include "gb-workspace-pane.h"
#include "gb-workspace-pane-group.h"
#include "gb-workspace-search-provider.h"
#include "gb-zoomable.h"

struct _GbWorkspacePrivate
{
   GbProject *project;

   gboolean is_fullscreen;

   GtkAdjustment *search_adj;

   GtkWidget *build;
   GtkWidget *current_pane;
   GtkWidget *docs;
   GtkWidget *edit;
   GtkWidget *header;
   GtkWidget *layout;
   GtkWidget *menu;
   GtkWidget *notebook;
   GtkWidget *run;
   GtkWidget *search;
   GtkWidget *search_entry;
   GtkWidget *splash;
   GtkWidget *switcher;
   GtkWidget *switcher_overlay;
   GtkWidget *vbox;
};

enum
{
   PROP_0,
   PROP_IS_FULLSCREEN,
   PROP_PROJECT,
   LAST_PROP
};

enum
{
   PANE_ADDED,
   PANE_REMOVED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

G_DEFINE_TYPE_WITH_PRIVATE(GbWorkspace,
                           gb_workspace,
                           GTK_TYPE_APPLICATION_WINDOW)

gboolean
gb_workspace_is_fullscreen (GbWorkspace *workspace)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), FALSE);
   return workspace->priv->is_fullscreen;
}

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
                                      priv->switcher_overlay);
   } else {
      gtk_widget_hide(priv->build);
      gtk_widget_hide(priv->menu);
      gtk_widget_hide(priv->run);
      gtk_widget_hide(priv->search);
      gtk_header_bar_set_title(GTK_HEADER_BAR(priv->header),
                               _("Select a Project"));
   }

   /*
    * TODO: Add other layouts here.
    */
   gb_workspace_layout_load(GB_WORKSPACE_LAYOUT(priv->edit), priv->project);

   g_object_notify_by_pspec(G_OBJECT(workspace), gParamSpecs[PROP_PROJECT]);
}

GbWorkspaceMode
gb_workspace_get_mode (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   gint page;

   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), 0);

   priv = workspace->priv;

   page = gtk_notebook_get_current_page(GTK_NOTEBOOK(priv->notebook));
   return (GbWorkspaceMode)page;
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

   if (priv->is_fullscreen) {
      gtk_widget_set_visible(priv->header,
                             GB_IS_WORKSPACE_LAYOUT_SPLASH(priv->layout));
   }
}

static void
gb_workspace_add (GtkContainer *container,
                  GtkWidget    *child)
{
   GbWorkspacePrivate *priv = GB_WORKSPACE(container)->priv;

   if (GB_IS_WORKSPACE_PANE(child)) {
      g_object_ref(child);
      gtk_container_add(GTK_CONTAINER(priv->layout), child);
      g_signal_emit(container, gSignals[PANE_ADDED], 0, child);
      g_object_unref(child);
   } else {
      GTK_CONTAINER_CLASS(gb_workspace_parent_class)->add(container, child);
   }
}

static void
gb_workspace_save_pane_cb (GbWorkspacePane *pane,
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
gb_workspace_close_pane (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
   GbWorkspacePrivate *priv;
   GbWorkspacePane *pane;
   GbWorkspace *workspace = user_data;
   GtkWidget *parent;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   priv = workspace->priv;

   if (GB_IS_WORKSPACE_PANE(priv->current_pane)) {
      pane = GB_WORKSPACE_PANE(priv->current_pane);
      for (parent = gtk_widget_get_parent(priv->current_pane);
           parent;
           parent = gtk_widget_get_parent(parent)) {
         if (GB_IS_WORKSPACE_PANE_GROUP(parent)) {
            gb_workspace_pane_group_close(GB_WORKSPACE_PANE_GROUP(parent),
                                          pane);
         }
      }
   }
}

static void
gb_workspace_save_pane (GSimpleAction *action,
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
                                      (GAsyncReadyCallback)gb_workspace_save_pane_cb,
                                      g_object_ref(workspace));
      }
   }
}

static void
gb_workspace_search_pane (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
   GbWorkspacePrivate *priv;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   priv = workspace->priv;

   if (GB_IS_WORKSPACE_PANE(priv->current_pane)) {
      gb_workspace_pane_focus_search(GB_WORKSPACE_PANE(priv->current_pane));
   }
}

static void
gb_workspace_zoom_pane_in (GSimpleAction *action,
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
      if (GB_IS_ZOOMABLE(pane)) {
         gb_zoomable_zoom_in(GB_ZOOMABLE(pane));
      }
   }
}

static void
gb_workspace_zoom_pane_out (GSimpleAction *action,
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
      if (GB_IS_ZOOMABLE(pane)) {
         gb_zoomable_zoom_out(GB_ZOOMABLE(pane));
      }
   }
}

static void
gb_workspace_new_file (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
   GbWorkspacePane *pane;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   pane = g_object_new(GB_TYPE_SOURCE_PANE,
                       "icon-name", "text-x-generic",
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(workspace), GTK_WIDGET(pane));
}

static void
gb_workspace_new_terminal (GSimpleAction *action,
                           GVariant      *parameter,
                           gpointer       user_data)
{
   GbWorkspacePane *pane;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(G_IS_ACTION(action));

   pane = g_object_new(GB_TYPE_TERMINAL_PANE,
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(workspace), GTK_WIDGET(pane));
}

void
gb_workspace_update_actions (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   GbWorkspacePane *pane;
   GActionMap *map;
   gboolean has_pane;
   gboolean is_modified = FALSE;
   gboolean is_zoomable;
   GAction *action;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   map = G_ACTION_MAP(workspace);
   has_pane = GB_IS_WORKSPACE_PANE(priv->current_pane);
   pane = has_pane ? GB_WORKSPACE_PANE(priv->current_pane) : NULL;
   is_zoomable = GB_IS_ZOOMABLE(priv->current_pane);

   if ((action = g_action_map_lookup_action(map, "search-pane"))) {
      g_simple_action_set_enabled(G_SIMPLE_ACTION(action), has_pane);
   }

   if ((action = g_action_map_lookup_action(map, "close-pane"))) {
      g_simple_action_set_enabled(G_SIMPLE_ACTION(action), has_pane);
   }

   if ((action = g_action_map_lookup_action(map, "zoom-pane-in"))) {
      g_simple_action_set_enabled(G_SIMPLE_ACTION(action), is_zoomable);
   }

   if ((action = g_action_map_lookup_action(map, "zoom-pane-out"))) {
      g_simple_action_set_enabled(G_SIMPLE_ACTION(action), is_zoomable);
   }

   if ((action = g_action_map_lookup_action(map, "save-pane"))) {
      if (has_pane) {
         is_modified = gb_workspace_pane_get_can_save(pane);
      }
      g_simple_action_set_enabled(G_SIMPLE_ACTION(action),
                                  has_pane && is_modified);
   }
}

static void
gb_workspace_set_focus (GtkWindow *window,
                        GtkWidget *widget)
{
   GbWorkspacePrivate *priv;
   GbWorkspace *workspace = (GbWorkspace *)window;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   GTK_WINDOW_CLASS(gb_workspace_parent_class)->set_focus(window, widget);

   for (; widget; widget = gtk_widget_get_parent(widget)) {
      if (GB_IS_WORKSPACE_PANE(widget)) {
         if (priv->current_pane) {
            g_object_remove_weak_pointer(G_OBJECT(priv->current_pane),
                                         (gpointer *)&priv->current_pane);
         }
         priv->current_pane = widget;
         g_object_add_weak_pointer(G_OBJECT(widget),
                                   (gpointer *)&priv->current_pane);
         break;
      }
   }

   if (widget) {
      gb_workspace_update_actions(workspace);
   }
}

static void
gb_workspace_docs_search (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
   GbWorkspace *workspace = user_data;
   GbWorkspaceMode mode;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   mode = gb_workspace_get_mode(workspace);
   if (mode != GB_WORKSPACE_DOCS) {
      gb_workspace_set_mode(workspace, GB_WORKSPACE_DOCS);
   }

   gtk_widget_grab_focus(workspace->priv->docs);
}

static void
gb_workspace_global_search (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
   GtkToggleButton *button;
   GbWorkspace *workspace = user_data;
   gboolean active;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   button = GTK_TOGGLE_BUTTON(workspace->priv->search);
   active = gtk_toggle_button_get_active(button);
   gtk_toggle_button_set_active(button, !active);
}

static void
gb_workspace_select_pane (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
   GbWorkspacePrivate *priv;
   GbWorkspace *workspace = user_data;
   GtkWidget *widget;
   gint idx = -1;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(parameter);
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   g_variant_get(parameter, "i", &idx);
   if (idx == -1) {
      return;
   }

   widget = priv->current_pane;

   for (widget = priv->current_pane;
        widget;
        widget = gtk_widget_get_parent(widget)) {
      if (GB_IS_WORKSPACE_PANE_GROUP(widget)) {
         gb_workspace_pane_group_set_page(
               GB_WORKSPACE_PANE_GROUP(widget),
               idx);
      }
   }
}

static void
gb_workspace_toggle_fullscreen (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data)
{
   GbWorkspace *workspace = user_data;

   if (gb_workspace_is_fullscreen(workspace)) {
      gtk_window_unfullscreen(GTK_WINDOW(workspace));
   } else {
      gtk_window_fullscreen(GTK_WINDOW(workspace));
   }
}

static void
gb_workspace_do_fullscreen_cb (GtkWidget *widget,
                               gpointer   user_data)
{
   if (GB_IS_WORKSPACE_LAYOUT(widget)) {
      gb_workspace_layout_fullscreen(GB_WORKSPACE_LAYOUT(widget));
   }
}

static void
gb_workspace_do_unfullscreen_cb (GtkWidget *widget,
                                 gpointer   user_data)
{
   if (GB_IS_WORKSPACE_LAYOUT(widget)) {
      gb_workspace_layout_unfullscreen(GB_WORKSPACE_LAYOUT(widget));
   }
}

static void
gb_workspace_do_fullscreen (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv = workspace->priv;

   gb_workspace_layout_fullscreen(GB_WORKSPACE_LAYOUT(priv->layout));

   gtk_container_foreach(GTK_CONTAINER(priv->notebook),
                         gb_workspace_do_fullscreen_cb,
                         NULL);

   if (!GB_IS_WORKSPACE_LAYOUT_SPLASH(priv->layout)) {
      gtk_widget_hide(priv->header);
   }
}

static void
gb_workspace_do_unfullscreen (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv = workspace->priv;

   gtk_widget_show(priv->header);

   gtk_container_foreach(GTK_CONTAINER(priv->notebook),
                         gb_workspace_do_unfullscreen_cb,
                         NULL);
}

static void
gb_workspace_window_state_event (GtkWidget           *widget,
                                 GdkEventWindowState *event,
                                 gpointer             user_data)
{
   GbWorkspacePrivate *priv;
   GbWorkspace *workspace = (GbWorkspace *)widget;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   if ((event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)) {
      priv->is_fullscreen = !priv->is_fullscreen;
      if (priv->is_fullscreen) {
         gb_workspace_do_fullscreen(workspace);
      } else {
         gb_workspace_do_unfullscreen(workspace);
      }
      g_object_notify_by_pspec(G_OBJECT(widget),
                               gParamSpecs[PROP_IS_FULLSCREEN]);
   }
}

static gboolean
show_global_search (GtkWidget *widget,
                    gpointer   user_data)
{
   GbWorkspace *workspace = user_data;
   GtkWidget *focus;
   gdouble value = 2.0;

   g_assert(GTK_IS_TOGGLE_BUTTON(widget));
   g_assert(GB_IS_WORKSPACE(workspace));

   focus = workspace->priv->layout;

   if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
      gtk_adjustment_set_value(workspace->priv->search_adj, 0.0);
      value = 1.0;
      focus = workspace->priv->search_entry;
   }

   gb_object_animate(workspace->priv->search_adj,
                     GB_ANIMATION_EASE_IN_OUT_QUAD, 250, NULL,
                     "value", value,
                     NULL);


   gtk_widget_grab_focus(focus);

   return FALSE;
}

static gboolean
get_child_position (GtkOverlay   *overlay,
                    GtkWidget    *widget,
                    GdkRectangle *alloc,
                    gpointer      user_data)
{
   GbWorkspacePrivate *priv;
   GbWorkspace *workspace = user_data;

   priv = workspace->priv;

   gtk_widget_get_allocation(GTK_WIDGET(overlay), alloc);

   alloc->x = 0;
   alloc->y = -alloc->height;
   alloc->y += gtk_adjustment_get_value(priv->search_adj) * alloc->height;

   return TRUE;
}

static void
hide_global_search (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   priv = workspace->priv;

   gtk_entry_set_text(GTK_ENTRY(priv->search_entry), "");
   g_object_set(priv->search,
                "active", FALSE,
                NULL);
}

static void
on_search_entry_activate (GbWorkspace *workspace,
                          GtkEntry    *entry)
{
   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(GTK_IS_ENTRY(entry));

   /*
    * TODO: Activate the selected completion.
    */

   hide_global_search(workspace);
}

static void
on_search_entry_changed (GbWorkspace *workspace,
                         GtkEntry    *entry)
{
   GtkEntryCompletion *completion;
   const gchar *text;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));
   g_return_if_fail(GTK_IS_ENTRY(entry));

   text = gtk_entry_get_text(entry);
   if (text && *text) {
      completion = gtk_entry_get_completion(entry);
      gb_search_completion_reload(GB_SEARCH_COMPLETION(completion), text);
      gb_search_completion_select_first(GB_SEARCH_COMPLETION(completion));
   }
}

static gboolean
on_search_completion_match_selected (GbWorkspace        *workspace,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter,
                                     GtkEntryCompletion *completion)
{
   g_return_val_if_fail(GB_IS_WORKSPACE(workspace), FALSE);
   g_return_val_if_fail(GTK_IS_TREE_MODEL(model), FALSE);
   g_return_val_if_fail(iter, FALSE);
   g_return_val_if_fail(GTK_IS_ENTRY_COMPLETION(completion), FALSE);

   hide_global_search(workspace);

   return FALSE;
}

static void
gb_workspace_finalize (GObject *object)
{
   GbWorkspacePrivate *priv;

   priv = GB_WORKSPACE(object)->priv;

   g_clear_object(&priv->search_adj);
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
   case PROP_IS_FULLSCREEN:
      g_value_set_boolean(value, gb_workspace_is_fullscreen(workspace));
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

   gParamSpecs[PROP_IS_FULLSCREEN] =
      g_param_spec_boolean("is-fullscreen",
                          _("Is Fullscreen"),
                          _("If the window is currently in fullscreen mode."),
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_READWRITE));
   g_object_class_install_property(object_class, PROP_IS_FULLSCREEN,
                                   gParamSpecs[PROP_IS_FULLSCREEN]);

   gParamSpecs[PROP_PROJECT] =
      g_param_spec_object("project",
                          _("Project"),
                          _("The project we are working with."),
                          GB_TYPE_PROJECT,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PROJECT,
                                   gParamSpecs[PROP_PROJECT]);

   gSignals[PANE_ADDED] = g_signal_new("pane-added",
                                       GB_TYPE_WORKSPACE,
                                       G_SIGNAL_RUN_FIRST,
                                       0,
                                       NULL,
                                       NULL,
                                       g_cclosure_marshal_VOID__OBJECT,
                                       G_TYPE_NONE,
                                       1,
                                       GB_TYPE_WORKSPACE_PANE);

   gSignals[PANE_REMOVED] = g_signal_new("pane-removed",
                                         GB_TYPE_WORKSPACE,
                                         G_SIGNAL_RUN_FIRST,
                                         0,
                                         NULL,
                                         NULL,
                                         g_cclosure_marshal_VOID__OBJECT,
                                         G_TYPE_NONE,
                                         1,
                                         GB_TYPE_WORKSPACE_PANE);

   g_io_extension_point_register("org.gnome.Builder.Workspace");
}

static void
gb_workspace_init_actions (GbWorkspace *workspace)
{
   static const GActionEntry entries[] = {
      { "close-pane", gb_workspace_close_pane },
      { "docs-search", gb_workspace_docs_search },
      { "global-search", gb_workspace_global_search },
      { "new-file", gb_workspace_new_file },
      { "new-terminal", gb_workspace_new_terminal },
      { "save-pane", gb_workspace_save_pane },
      { "search-pane", gb_workspace_search_pane },
      { "select-pane", gb_workspace_select_pane, "i" },
      { "toggle-fullscreen", gb_workspace_toggle_fullscreen },
      { "zoom-pane-in", gb_workspace_zoom_pane_in },
      { "zoom-pane-out", gb_workspace_zoom_pane_out },
   };

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   g_action_map_add_action_entries(G_ACTION_MAP(workspace),
                                   entries,
                                   G_N_ELEMENTS(entries),
                                   workspace);

   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>n", "win.new-file", NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>w", "win.close-pane", NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>f", "win.search-pane", NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>s", "win.save-pane", NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>plus", "win.zoom-pane-in", NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>minus", "win.zoom-pane-out",
                                   NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary><Shift>t", "win.new-terminal",
                                   NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Alt>Return", "win.toggle-fullscreen",
                                   NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>period", "win.global-search",
                                   NULL);
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                   "<Primary>k", "win.docs-search",
                                   NULL);

#define ADD_PANE_ACCEL(n, k) G_STMT_START { \
   GVariant *v; \
   v = g_variant_new("i", n); \
   gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT), \
                                   "<Alt>"#k, "win.select-pane", v); \
   g_variant_unref(v); \
} G_STMT_END

   ADD_PANE_ACCEL(0, 1);
   ADD_PANE_ACCEL(1, 2);
   ADD_PANE_ACCEL(2, 3);
   ADD_PANE_ACCEL(3, 4);
   ADD_PANE_ACCEL(4, 5);
   ADD_PANE_ACCEL(5, 6);
   ADD_PANE_ACCEL(6, 7);
   ADD_PANE_ACCEL(7, 8);
   ADD_PANE_ACCEL(8, 9);

#undef ADD_PANE_ACCEL
}

static void
gb_workspace_init (GbWorkspace *workspace)
{
   GbWorkspacePrivate *priv;
   GtkEntryCompletion *completion;
   GbSearchProvider *provider;
   GMenuModel *menu;
   GtkBuilder *builder;
   GtkWidget *box;
   GtkWidget *image;
   gboolean rtl;

   workspace->priv = G_TYPE_INSTANCE_GET_PRIVATE(workspace,
                                                 GB_TYPE_WORKSPACE,
                                                 GbWorkspacePrivate);

   priv = workspace->priv;

   gb_workspace_init_actions(workspace);

   rtl = gtk_widget_get_direction (GTK_WIDGET(workspace)) == GTK_TEXT_DIR_RTL;

   gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(workspace), FALSE);

   priv->header = g_object_new(GTK_TYPE_HEADER_BAR,
                               "title", _("Select a Project"),
                               "name", "workspace-header-bar",
                               "height-request", 48,
                               "show-close-button", TRUE,
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

   priv->switcher_overlay = g_object_new(GTK_TYPE_OVERLAY,
                                         "visible", TRUE,
                                         NULL);
   g_signal_connect(priv->switcher_overlay,
                    "get-child-position",
                    G_CALLBACK(get_child_position),
                    workspace);

   priv->switcher = g_object_new(GB_TYPE_WORKSPACE_LAYOUT_SWITCHER,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->switcher_overlay),
                     priv->switcher);

   priv->search_entry = g_object_new(GTK_TYPE_SEARCH_ENTRY,
                                     "visible", TRUE,
                                     NULL);
   g_signal_connect_object(priv->search_entry,
                           "changed",
                           G_CALLBACK(on_search_entry_changed),
                           workspace,
                           G_CONNECT_SWAPPED);
   g_signal_connect_object(priv->search_entry,
                           "activate",
                           G_CALLBACK(on_search_entry_activate),
                           workspace,
                           G_CONNECT_SWAPPED);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->switcher_overlay),
                           priv->search_entry);

   completion = gb_search_completion_new();
   g_signal_connect_object(completion,
                           "match-selected",
                           G_CALLBACK(on_search_completion_match_selected),
                           workspace,
                           (G_CONNECT_AFTER | G_CONNECT_SWAPPED));
   gtk_entry_set_completion(GTK_ENTRY(priv->search_entry), completion);

   provider = g_object_new(GB_TYPE_WORKSPACE_SEARCH_PROVIDER,
                           "name", _("Workspace"),
                           "workspace", workspace,
                           NULL);
   gb_search_completion_add_provider(GB_SEARCH_COMPLETION(completion),
                                     provider);

   g_object_unref(completion);

   priv->search_adj = g_object_new(GTK_TYPE_ADJUSTMENT,
                                   "value", 0.0,
                                   "lower", 0.0,
                                   "upper", 2.0,
                                   NULL);
   g_signal_connect_swapped(priv->search_adj,
                            "value-changed",
                            G_CALLBACK(gtk_widget_queue_resize),
                            priv->search_entry);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-name", "edit-find-symbolic",
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "visible", TRUE,
                        NULL);
   priv->search = g_object_new(GTK_TYPE_TOGGLE_BUTTON,
                               "image", image,
                               "valign", GTK_ALIGN_CENTER,
                               "hexpand", FALSE,
                               "visible", FALSE,
                               NULL);
   g_signal_connect(priv->search,
                    "clicked",
                    G_CALLBACK(show_global_search),
                    workspace);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header), priv->search);

   builder = gtk_builder_new();
   gtk_builder_add_from_string(builder,
         "<interface>"
         "  <menu id='menubar'>"
         "    <section>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>_New File</attribute>"
         "        <attribute name='action'>win.new-file</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;n</attribute>"
         "      </item>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>_Open...</attribute>"
         "        <attribute name='action'>app.open</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;o</attribute>"
         "      </item>"
         "    </section>"
         "    <section>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>_Save</attribute>"
         "        <attribute name='action'>win.save-pane</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;s</attribute>"
         "      </item>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>_Save As...</attribute>"
         "        <attribute name='action'>win.save-as-pane</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;&lt;Shift&gt;s</attribute>"
         "      </item>"
         "    </section>"
         "    <section>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>_Find</attribute>"
         "        <attribute name='action'>win.search-pane</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;f</attribute>"
         "      </item>"
         "    </section>"
         "    <section>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>Zoom In</attribute>"
         "        <attribute name='action'>win.zoom-pane-in</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;plus</attribute>"
         "      </item>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>Zoom Out</attribute>"
         "        <attribute name='action'>win.zoom-pane-out</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;minus</attribute>"
         "      </item>"
         "    </section>"
         "    <section>"
         "      <item>"
         "        <attribute name='label' translatable='yes'>_Close</attribute>"
         "        <attribute name='action'>win.close-pane</attribute>"
         "        <attribute name='accel'>&lt;Primary&gt;w</attribute>"
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
                             "valign", GTK_ALIGN_CENTER,
                             "hexpand", FALSE,
                             "menu-model", menu,
                             "visible", FALSE,
                             NULL);
   gtk_header_bar_pack_end(GTK_HEADER_BAR(priv->header), priv->menu);

   gtk_widget_set_halign(
         GTK_WIDGET(gtk_menu_button_get_popup(GTK_MENU_BUTTON(priv->menu))),
         GTK_ALIGN_END);

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
                        "icon-name", rtl ?  "media-playback-start-rtl-symbolic" :
                                            "media-playback-start-symbolic",
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

   /*
    * TODO: Replace with other layout implementations.
    */
   gtk_container_add(GTK_CONTAINER(priv->notebook), gtk_label_new(NULL));
   gtk_container_add(GTK_CONTAINER(priv->notebook), gtk_label_new(NULL));
   gtk_container_add(GTK_CONTAINER(priv->notebook), gtk_label_new(NULL));

   priv->docs = g_object_new(GB_TYPE_WORKSPACE_LAYOUT_DOCS,
                             "visible", TRUE,
                             NULL);
   gtk_container_add(GTK_CONTAINER(priv->notebook), priv->docs);

   gb_workspace_set_mode(workspace, GB_WORKSPACE_SPLASH);

   g_object_unref(builder);

   gb_workspace_update_actions(workspace);

   g_signal_connect(workspace,
                    "window-state-event",
                    G_CALLBACK(gb_workspace_window_state_event),
                    NULL);
}

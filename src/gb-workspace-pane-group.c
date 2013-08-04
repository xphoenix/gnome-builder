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
   GtkListStore *panes;

   GtkWidget *close;
   GtkWidget *combo;
   GtkWidget *next;
   GtkWidget *prev;
   GtkWidget *extra;
   GtkWidget *notebook;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_workspace_pane_group_add (GtkContainer *container,
                             GtkWidget    *child)
{
   GbWorkspacePaneGroupPrivate *priv;
   GbWorkspacePaneGroup *group = (GbWorkspacePaneGroup *)container;
   GtkTreeIter iter;

   g_return_if_fail(GB_IS_WORKSPACE_PANE_GROUP(group));
   g_return_if_fail(GTK_IS_WIDGET(child));

   priv = group->priv;

   if (GB_IS_WORKSPACE_PANE(child)) {
      gtk_container_add(GTK_CONTAINER(priv->notebook), child);
      gtk_list_store_append(priv->panes, &iter);
      gtk_list_store_set(priv->panes, &iter, 0, child, -1);
      gtk_widget_set_sensitive(priv->combo, TRUE);
      gtk_widget_set_sensitive(priv->close, TRUE);
      gtk_combo_box_set_active_iter(GTK_COMBO_BOX(priv->combo), &iter);
      g_signal_connect_swapped(child, "notify::can-save",
                               G_CALLBACK(gtk_widget_queue_draw),
                               priv->combo);
   } else {
      GTK_CONTAINER_CLASS(gb_workspace_pane_group_parent_class)->
         add(container, child);
   }
}

static void
combo_text_func (GtkCellLayout   *cell_layout,
                 GtkCellRenderer *cell,
                 GtkTreeModel    *tree_model,
                 GtkTreeIter     *iter,
                 gpointer         user_data)
{
   GbWorkspacePane *pane;
   gboolean modified;
   gchar *text;
   gchar *title;

   gtk_tree_model_get(tree_model, iter, 0, &pane, -1);
   g_object_get(pane,
                "title", &title,
                "can-save", &modified,
                NULL);

   if (modified) {
      text = g_strdup_printf("%s *", title);
      g_object_set(cell, "text", text, NULL);
      g_free(text);
   } else {
      g_object_set(cell, "text", title, NULL);
   }

   g_free(title);
   g_object_unref(pane);
}

static void
pixbuf_text_func (GtkCellLayout   *cell_layout,
                  GtkCellRenderer *cell,
                  GtkTreeModel    *tree_model,
                  GtkTreeIter     *iter,
                  gpointer         user_data)
{
   GbWorkspacePane *pane;
   const gchar *icon_name;

   gtk_tree_model_get(tree_model, iter, 0, &pane, -1);
   icon_name = gb_workspace_pane_get_icon_name(pane);
   g_object_set(cell, "icon-name", icon_name, NULL);
   g_object_unref(pane);
}

static void
combo_changed (GtkComboBox          *combo_box,
               GbWorkspacePaneGroup *group)
{
   GbWorkspacePaneGroupPrivate *priv;
   GtkTreeIter iter;
   GtkWidget *child;
   gint page;

   priv = group->priv;

   if (gtk_combo_box_get_active_iter(combo_box, &iter)) {
      gtk_tree_model_get(GTK_TREE_MODEL(priv->panes), &iter,
                         0, &child,
                         -1);
      page = gtk_notebook_page_num(GTK_NOTEBOOK(priv->notebook), child);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(priv->notebook), page);
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
   GbWorkspacePaneGroupPrivate *priv = GB_WORKSPACE_PANE_GROUP(object)->priv;

   g_clear_object(&priv->panes);

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
   GtkCellRenderer *cell;
   GtkWidget *image;

   group->priv = G_TYPE_INSTANCE_GET_PRIVATE(group,
                                             GB_TYPE_WORKSPACE_PANE_GROUP,
                                             GbWorkspacePaneGroupPrivate);

   priv = group->priv;

   gtk_style_context_add_class(
      gtk_widget_get_style_context(GTK_WIDGET(group)),
      "linked");

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "icon-name", "go-previous",
                        NULL);
   priv->prev = g_object_new(GTK_TYPE_BUTTON,
                             "hexpand", FALSE,
                             "image", image,
                             "sensitive", FALSE,
                             "visible", TRUE,
                             NULL);
   gtk_container_add(GTK_CONTAINER(group), priv->prev);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "icon-name", "go-next",
                        NULL);
   priv->next = g_object_new(GTK_TYPE_BUTTON,
                             "hexpand", FALSE,
                             "image", image,
                             "sensitive", FALSE,
                             "visible", TRUE,
                             NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(group), priv->next,
                                     "left-attach", 1,
                                     NULL);

   priv->combo = g_object_new(GTK_TYPE_COMBO_BOX,
                              "hexpand", TRUE,
                              "sensitive", FALSE,
                              "visible", TRUE,
                              NULL);
   gtk_style_context_add_class(gtk_widget_get_style_context(priv->combo),
                               "builder-header-sibling");
   g_signal_connect(priv->combo, "changed", G_CALLBACK(combo_changed), group);
   gtk_container_add_with_properties(GTK_CONTAINER(group), priv->combo,
                                     "left-attach", 2,
                                     NULL);

   priv->panes = gtk_list_store_new(1, GB_TYPE_WORKSPACE_PANE);
   gtk_combo_box_set_model(GTK_COMBO_BOX(priv->combo),
                           GTK_TREE_MODEL(priv->panes));

   cell = gtk_cell_renderer_pixbuf_new();
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(priv->combo), cell, FALSE);
   gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(priv->combo), cell,
                                      pixbuf_text_func, NULL, NULL);

   cell = gtk_cell_renderer_text_new();
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(priv->combo), cell, TRUE);
   gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(priv->combo), cell,
                                      combo_text_func, NULL, NULL);

   priv->extra = g_object_new(GTK_TYPE_BOX,
                              "hexpand", FALSE,
                              "orientation", GTK_ORIENTATION_HORIZONTAL,
                              "visible", TRUE,
                              NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(group), priv->extra,
                                     "left-attach", 3,
                                     NULL);

   image = g_object_new(GTK_TYPE_IMAGE,
                        "icon-size", GTK_ICON_SIZE_MENU,
                        "icon-name", "gtk-close",
                        NULL);
   priv->close = g_object_new(GTK_TYPE_BUTTON,
                              "hexpand", FALSE,
                              "image", image,
                              "sensitive", FALSE,
                              "visible", TRUE,
                              NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(group), priv->close,
                                     "left-attach", 4,
                                     NULL);

   priv->notebook = g_object_new(GTK_TYPE_NOTEBOOK,
                                 "hexpand", TRUE,
                                 "show-border", FALSE,
                                 "show-tabs", FALSE,
                                 "vexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(group), priv->notebook,
                                     "left-attach", 0,
                                     "top-attach", 1,
                                     "width", 5,
                                     NULL);
}

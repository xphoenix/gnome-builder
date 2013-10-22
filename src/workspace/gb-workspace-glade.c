/* gb-workspace-glade.c
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

#include <gladeui/glade.h>
#include <glib/gi18n.h>

#include "gb-workspace-glade.h"

G_DEFINE_TYPE(GbWorkspaceGlade, gb_workspace_glade, GB_TYPE_WORKSPACE_SECTION)

struct _GbWorkspaceGladePrivate
{
   GtkWidget *hpaned1;
   GtkWidget *hpaned2;
   GtkWidget *vpaned1;

   GtkWidget *design_view;
   GtkWidget *editor;
   GtkWidget *inspector;
   GtkWidget *palette;
};

enum
{
   PROP_0,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

static void
on_selection_changed (GladeInspector   *inspector,
                      GbWorkspaceGlade *glade)
{
   GbWorkspaceGladePrivate *priv;
   GList *list;

   g_return_if_fail(GB_IS_WORKSPACE_GLADE(glade));

   priv = glade->priv;

   g_print("Selection changed\n");

   if ((list = glade_inspector_get_selected_items(inspector))) {
      glade_editor_load_widget(GLADE_EDITOR(priv->editor), list->data);
   }

   g_list_free(list);
}

static void
gb_workspace_glade_finalize (GObject *object)
{
   //GbWorkspaceGladePrivate *priv;

   //priv = GB_WORKSPACE_GLADE(object)->priv;

   G_OBJECT_CLASS(gb_workspace_glade_parent_class)->finalize(object);
}

static void
gb_workspace_glade_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
   //GbWorkspaceGlade *glade = GB_WORKSPACE_GLADE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_glade_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
   //GbWorkspaceGlade *glade = GB_WORKSPACE_GLADE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_glade_class_init (GbWorkspaceGladeClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_glade_finalize;
   object_class->get_property = gb_workspace_glade_get_property;
   object_class->set_property = gb_workspace_glade_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceGladePrivate));
}

static void
gb_workspace_glade_init (GbWorkspaceGlade *glade)
{
   GbWorkspaceGladePrivate *priv;
   GladeProject *project;

   glade->priv = G_TYPE_INSTANCE_GET_PRIVATE(glade,
                                             GB_TYPE_WORKSPACE_GLADE,
                                             GbWorkspaceGladePrivate);

   priv = glade->priv;

   /*
    * XXX: This is just an experiment right now.
    */

   project = glade_project_new();

   priv->hpaned1 = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_paned_set_position(GTK_PANED(priv->hpaned1), 200);
   gtk_container_add(GTK_CONTAINER(glade), priv->hpaned1);
   gtk_widget_show(priv->hpaned1);

   priv->palette = glade_palette_new();
   g_object_set(priv->palette, "project", project, NULL);
   gtk_container_add(GTK_CONTAINER(priv->hpaned1), priv->palette);
   gtk_widget_show(priv->palette);

   priv->hpaned2 = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_paned_set_position(GTK_PANED(priv->hpaned2), 600);
   gtk_container_add(GTK_CONTAINER(priv->hpaned1), priv->hpaned2);
   gtk_widget_show(priv->hpaned2);

   priv->design_view = glade_design_view_new(project);
   gtk_container_add(GTK_CONTAINER(priv->hpaned2), priv->design_view);
   gtk_widget_show(priv->design_view);

   priv->vpaned1 = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
   gtk_container_add(GTK_CONTAINER(priv->hpaned2), priv->vpaned1);
   gtk_widget_show(priv->vpaned1);

   priv->inspector = glade_inspector_new();
   g_object_set(priv->inspector, "project", project, NULL);
   gtk_container_add(GTK_CONTAINER(priv->vpaned1), priv->inspector);
   g_signal_connect(priv->inspector, "selection-changed",
                    G_CALLBACK(on_selection_changed), glade);
   gtk_widget_show(priv->inspector);

   priv->editor = glade_editor_new();
   g_object_set(priv->editor, "project", project, NULL);
   gtk_container_add(GTK_CONTAINER(priv->vpaned1), priv->editor);
   gtk_widget_show(priv->editor);
}

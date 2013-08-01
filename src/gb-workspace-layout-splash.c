/* gb-workspace-layout-splash.c:
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

#include "gb-project.h"
#include "gb-workspace.h"
#include "gb-workspace-layout.h"
#include "gb-workspace-layout-edit.h"
#include "gb-workspace-layout-splash.h"

G_DEFINE_TYPE(GbWorkspaceLayoutSplash,
              gb_workspace_layout_splash,
              GB_TYPE_WORKSPACE_LAYOUT)

struct _GbWorkspaceLayoutSplashPrivate
{
   GtkWidget *list_box;
};

static void
gb_workspace_layout_splash_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_layout_splash_parent_class)->finalize(object);
}

static void
gb_workspace_layout_splash_class_init (GbWorkspaceLayoutSplashClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_layout_splash_finalize;
   g_type_class_add_private(object_class,
                            sizeof(GbWorkspaceLayoutSplashPrivate));
}

static void
gb_workspace_layout_splash_row_activated (GtkListBox              *list_box,
                                          GtkWidget               *row,
                                          GbWorkspaceLayoutSplash *splash)
{
   GbWorkspace *workspace;
   const gchar *path;
   GbProject *project;
   GtkWidget *child;

   g_assert(GTK_IS_LIST_BOX(list_box));
   g_assert(GTK_IS_LIST_BOX_ROW(row));
   g_assert(GB_IS_WORKSPACE_LAYOUT_SPLASH(splash));

   workspace = GB_WORKSPACE(gtk_widget_get_toplevel(row));
   child = gtk_bin_get_child(GTK_BIN(row));

   if (!(path = g_object_get_data(G_OBJECT(child), "path"))) {
      project = g_object_new(GB_TYPE_PROJECT,
                             "name", "Unnamed Project",
                             NULL);
      gb_workspace_set_project(workspace, project);
      gb_workspace_set_mode(workspace, GB_WORKSPACE_EDIT);
      g_object_unref(project);
   } else {
      g_print("open %s.\n", path);
   }
}

static void
update_separators (GtkListBoxRow *row,
                   GtkListBoxRow *before,
                   gpointer       user_data)
{
   GtkWidget *header;

   g_assert(GTK_IS_LIST_BOX_ROW(row));
   g_assert(!before || GTK_IS_LIST_BOX_ROW(before));
   g_assert(!user_data);

   header = gtk_list_box_row_get_header(row);

   if (!before) {
      if (header) {
         gtk_widget_destroy(header);
      }
      return;
   }

   if (!header) {
      header = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
      gtk_list_box_row_set_header(row, header);
      gtk_widget_show(header);
   }
}

static GtkWidget *
make_row (const gchar *name,
          const gchar *date,
          const gchar *path)
{
   GtkWidget *row;
   GtkWidget *l;
   gchar *str;

   row = g_object_new(GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);

   str = g_strdup_printf(
      path ? "<b>%s</b>" : "<span weight='bold' color='#999999'>%s</span>",
      name);
   l = g_object_new(GTK_TYPE_LABEL,
                    "hexpand", TRUE,
                    "label", str,
                    "margin-bottom", 12,
                    "margin-left", 12,
                    "margin-right", 12,
                    "margin-top", 12,
                    "use-markup", TRUE,
                    "visible", TRUE,
                    "xalign", 0.0f,
                    "yalign", 0.5f,
                    NULL);
   gtk_container_add(GTK_CONTAINER(row), l);
   g_free(str);

   str = g_strdup_printf("<span color='#999999' weight='bold'>%s</span>",
                         date ? date : "");
   l = g_object_new(GTK_TYPE_LABEL,
                    "label", str,
                    "margin-bottom", 12,
                    "margin-left", 12,
                    "margin-right", 12,
                    "margin-top", 12,
                    "use-markup", TRUE,
                    "visible", TRUE,
                    "xalign", 1.0f,
                    "yalign", 0.5f,
                    NULL);
   gtk_container_add(GTK_CONTAINER(row), l);
   g_free(str);

   g_object_set_data_full(G_OBJECT(row), "path", g_strdup(path), g_free);

   return row;
}

static void
gb_workspace_layout_splash_init (GbWorkspaceLayoutSplash *splash)
{
   GtkWidget *align;
   GtkWidget *frame_;
   GtkWidget *row;
   GtkWidget *viewport;
   GtkWidget *header;
   GtkWidget *sep;

   splash->priv = G_TYPE_INSTANCE_GET_PRIVATE(splash,
                                              GB_TYPE_WORKSPACE_LAYOUT_SPLASH,
                                              GbWorkspaceLayoutSplashPrivate);

   header = g_object_new(GD_TYPE_HEADER_BAR,
                         "height-request", 48,
                         "hexpand", FALSE,
                         "title", _("Select a Project"),
                         "vexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(splash), header,
                                     "left-attach", 0,
                                     "top-attach", 0,
                                     "width", 1,
                                     "height", 1,
                                     NULL);

   sep = g_object_new(GTK_TYPE_SEPARATOR,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(splash), sep,
                                     "left-attach", 0,
                                     "top-attach", 1,
                                     "width", 1,
                                     "height", 1,
                                     NULL);

   align = g_object_new(GTK_TYPE_ALIGNMENT,
                        "hexpand", TRUE,
                        "vexpand", TRUE,
                        "visible", TRUE,
                        "xalign", 0.5f,
                        "xscale", 0.0f,
                        "yalign", 0.5f,
                        "yscale", 0.0f,
                        NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(splash), align,
                                     "left-attach", 0,
                                     "top-attach", 2,
                                     "width", 1,
                                     "height", 1,
                                     NULL);

   frame_ = g_object_new(GTK_TYPE_FRAME,
                         "width-request", 500,
                         "shadow-type", GTK_SHADOW_IN,
                         "vexpand", FALSE,
                         "hexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   gtk_container_add(GTK_CONTAINER(align), frame_);

   viewport = g_object_new(GTK_TYPE_VIEWPORT,
                           "shadow-type", GTK_SHADOW_NONE,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(frame_), viewport);

   splash->priv->list_box = g_object_new(GTK_TYPE_LIST_BOX,
                                         "visible", TRUE,
                                         NULL);
   gtk_list_box_set_header_func(GTK_LIST_BOX(splash->priv->list_box),
                                update_separators, NULL, NULL);
   gtk_list_box_set_selection_mode(GTK_LIST_BOX(splash->priv->list_box),
                                   GTK_SELECTION_NONE);
   gtk_list_box_set_activate_on_single_click(
         GTK_LIST_BOX(splash->priv->list_box),
         TRUE);
   g_signal_connect(splash->priv->list_box,
                    "row-activated",
                    G_CALLBACK(gb_workspace_layout_splash_row_activated),
                    splash);
   gtk_container_add(GTK_CONTAINER(viewport), splash->priv->list_box);

   row = make_row("Gnome Builder", "Yesterday", "gbproject.gbproject");
   gtk_container_add(GTK_CONTAINER(splash->priv->list_box), row);

   row = make_row("New Project", NULL, NULL);
   gtk_container_add(GTK_CONTAINER(splash->priv->list_box), row);
}

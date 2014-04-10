/* gb-workspace-greeter.c
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
#include "gb-create-project-dialog.h"
#include "gb-project.h"
#include "gb-project-info.h"
#include "gb-project-service.h"
#include "gb-service.h"
#include "gb-workspace.h"
#include "gb-workspace-greeter.h"

G_DEFINE_TYPE(GbWorkspaceGreeter, gb_workspace_greeter, GTK_TYPE_GRID)

struct _GbWorkspaceGreeterPrivate
{
   GtkHeaderBar      *header;
   GtkListBox        *projects;
   GtkScrolledWindow *scroller;
};

enum
{
   PROJECT_SELECTED,
   LAST_SIGNAL
};

static guint gSignals[LAST_SIGNAL];

GList *
get_recent_projects (void)
{
   GbService *projects;
   GList *list = NULL;

   projects = gb_application_get_service(GB_APPLICATION_DEFAULT,
                                         GB_TYPE_PROJECT_SERVICE);
   if (projects) {
      list = gb_project_service_get_recent_projects(GB_PROJECT_SERVICE(projects));
      list = g_list_copy(list);
      g_list_foreach(list, (GFunc)g_object_ref, NULL);
   }

   return list;
}

GtkWidget *
gb_workspace_greeter_new (void)
{
   return g_object_new(GB_TYPE_WORKSPACE_GREETER, NULL);
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
make_row (GbProjectInfo *project_info)
{
   const gchar *name;
   GDateTime *dt;
   GtkWidget *box;
   GtkWidget *l;
   GtkWidget *row;
   gchar *date = NULL;
   gchar *str;

   if (!project_info) {
      name = N_("New Project");
   } else {
      name = gb_project_info_get_name(project_info);
      dt = gb_project_info_get_last_modified_at(project_info);
      if (dt) {
         date = g_date_time_format(dt, "%x");
      }
   }

   row = g_object_new(GTK_TYPE_LIST_BOX_ROW,
                      "visible", TRUE,
                      NULL);

   box = g_object_new(GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);
   gtk_container_add(GTK_CONTAINER(row), box);

   str = g_strdup_printf(
      project_info ? "<b>%s</b>" : "<span weight='bold' color='#999999'>%s</span>",
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
   gtk_container_add(GTK_CONTAINER(box), l);
   g_free(str);

   str = g_strdup_printf("<span color='#999999' weight='normal'>%s</span>",
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
   gtk_container_add(GTK_CONTAINER(box), l);
   g_free(str);

   g_object_set_data_full(G_OBJECT(row),
                          "project-info",
                          project_info ? g_object_ref(project_info) : NULL,
                          g_object_unref);

   g_free(date);

   return row;
}

static void
gb_workspace_greeter_parent_set (GtkWidget *widget,
                                 GtkWidget *old_parent)
{
   GbWorkspaceGreeter *greeter;
   GtkWidget *toplevel;

   g_return_if_fail(GB_IS_WORKSPACE_GREETER(widget));

   greeter = GB_WORKSPACE_GREETER(widget);
   toplevel = gtk_widget_get_toplevel(widget);

   if (GB_IS_WORKSPACE(toplevel)) {
      gtk_window_set_titlebar(GTK_WINDOW(toplevel),
                              GTK_WIDGET(greeter->priv->header));
   }

   gtk_widget_grab_focus(GTK_WIDGET(greeter->priv->projects));
}

static void
on_row_activated (GtkListBox         *list_box,
                  GtkListBoxRow      *list_box_row,
                  GbWorkspaceGreeter *greeter)
{
   GbProjectInfo *project_info;
   GtkWidget *dialog;
   GtkWidget *toplevel;
   GbProject *local_project = NULL;
   GbProject *project;

   g_return_if_fail(GTK_IS_LIST_BOX(list_box));
   g_return_if_fail(GTK_IS_LIST_BOX_ROW(list_box_row));
   g_return_if_fail(GB_IS_WORKSPACE_GREETER(greeter));

   toplevel = gtk_widget_get_toplevel(GTK_WIDGET(greeter));
   project_info = g_object_get_data(G_OBJECT(list_box_row), "project-info");

   if (!project_info) {
      dialog = g_object_new(GB_TYPE_CREATE_PROJECT_DIALOG,
                            "transient-for", toplevel,
                            "use-header-bar", TRUE,
                            NULL);
      if (GTK_RESPONSE_OK == gtk_dialog_run(GTK_DIALOG(dialog))) {
         project = local_project = gb_create_project_dialog_get_project(
               GB_CREATE_PROJECT_DIALOG(dialog));
      } else {
         gtk_widget_destroy(dialog);
         return;
      }
      gtk_widget_destroy(dialog);
   } else {
      /*
       * TODO: gb_project_info_load().
       */
      project = NULL;
   }

   g_object_set(toplevel, "project", project, NULL);

   g_clear_object(&local_project);
}

static void
gb_workspace_greeter_style_updated (GtkWidget *widget)
{
   GbWorkspaceGreeterPrivate *priv;
   GtkRequisition min_req;
   GtkRequisition nat_req;
   GtkWidget *fake_row;
   GList *list;
   gint length;
   gint min_height;

   priv = GB_WORKSPACE_GREETER(widget)->priv;

   if (GTK_WIDGET_CLASS(gb_workspace_greeter_parent_class)->style_updated) {
      GTK_WIDGET_CLASS(gb_workspace_greeter_parent_class)->
         style_updated(widget);
   }

   list = get_recent_projects();
   length = g_list_length(list);
   g_list_foreach(list, (GFunc)g_object_unref, list);
   g_list_free(list);
   list = NULL;

   fake_row = make_row(NULL);
   gtk_widget_get_preferred_size(fake_row, &min_req, &nat_req);
   gtk_widget_destroy(fake_row);

   length = CLAMP(length + 1, 1, 5);
   min_height = length * nat_req.height + ((length - 1) * 2);

   gtk_scrolled_window_set_min_content_height(
      GTK_SCROLLED_WINDOW(priv->scroller),
      min_height);
}

static void
gb_workspace_greeter_grab_focus (GtkWidget *widget)
{
   GbWorkspaceGreeterPrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE_GREETER(widget));

   priv = GB_WORKSPACE_GREETER(widget)->priv;

   gtk_widget_grab_focus(GTK_WIDGET(priv->projects));
}

static void
gb_workspace_greeter_finalize (GObject *object)
{
   GbWorkspaceGreeterPrivate *priv;

   priv = GB_WORKSPACE_GREETER(object)->priv;

   g_clear_object(&priv->header);

   G_OBJECT_CLASS(gb_workspace_greeter_parent_class)->finalize(object);
}

static void
gb_workspace_greeter_class_init (GbWorkspaceGreeterClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_greeter_finalize;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceGreeterPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->grab_focus = gb_workspace_greeter_grab_focus;
   widget_class->parent_set = gb_workspace_greeter_parent_set;
   widget_class->style_updated = gb_workspace_greeter_style_updated;

   gSignals[PROJECT_SELECTED] = g_signal_new("project-selected",
                                             GB_TYPE_WORKSPACE_GREETER,
                                             G_SIGNAL_RUN_LAST,
                                             0,
                                             NULL,
                                             NULL,
                                             g_cclosure_marshal_VOID__OBJECT,
                                             G_TYPE_NONE,
                                             1,
                                             GB_TYPE_PROJECT);
}

static void
gb_workspace_greeter_init (GbWorkspaceGreeter *greeter)
{
   GbWorkspaceGreeterPrivate *priv;
   GtkWidget *align;
   GtkWidget *frame_;
   GtkWidget *row;
   GList *iter;
   GList *list;

   greeter->priv = G_TYPE_INSTANCE_GET_PRIVATE(greeter,
                                               GB_TYPE_WORKSPACE_GREETER,
                                               GbWorkspaceGreeterPrivate);

   priv = greeter->priv;

   priv->header = g_object_new(GTK_TYPE_HEADER_BAR,
                               "hexpand", TRUE,
                               "show-close-button", TRUE,
                               "title", _("Select a Project"),
                               "visible", TRUE,
                               NULL);
   priv->header = g_object_ref(priv->header);

   align = g_object_new(GTK_TYPE_ALIGNMENT,
                        "border-width", 24,
                        "hexpand", TRUE,
                        "vexpand", TRUE,
                        "visible", TRUE,
                        "xalign", 0.5f,
                        "xscale", 0.0f,
                        "yalign", 0.5f,
                        "yscale", 0.5f,
                        NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(greeter), align,
                                     "left-attach", 0,
                                     "top-attach", 2,
                                     "width", 1,
                                     "height", 1,
                                     NULL);

   frame_ = g_object_new(GTK_TYPE_FRAME,
                         "width-request", 500,
                         "shadow-type", GTK_SHADOW_IN,
                         "valign", GTK_ALIGN_CENTER,
                         "hexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   gtk_container_add(GTK_CONTAINER(align), frame_);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "vexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(frame_), GTK_WIDGET(priv->scroller));

   priv->projects = g_object_new(GTK_TYPE_LIST_BOX,
                                 "can-focus", FALSE,
                                 "activate-on-single-click", TRUE,
                                 "selection-mode", GTK_SELECTION_NONE,
                                 "vexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_list_box_set_header_func(GTK_LIST_BOX(priv->projects),
                                update_separators, NULL, NULL);
   g_signal_connect(priv->projects,
                    "row-activated",
                    G_CALLBACK(on_row_activated),
                    greeter);
   gtk_container_add(GTK_CONTAINER(priv->scroller),
                     GTK_WIDGET(priv->projects));

   list = get_recent_projects();

   for (iter = list; iter; iter = iter->next) {
      row = make_row(iter->data);
      gtk_container_add(GTK_CONTAINER(priv->projects), row);
   }

   row = make_row(NULL);
   gtk_container_add(GTK_CONTAINER(priv->projects), row);

   g_list_foreach(list, (GFunc)g_object_unref, NULL);
   g_list_free(list);
}

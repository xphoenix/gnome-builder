/* gb-create-project-dialog.c
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

#include "gb-create-project-dialog.h"

G_DEFINE_TYPE(GbCreateProjectDialog, gb_create_project_dialog, GTK_TYPE_DIALOG)

struct _GbCreateProjectDialogPrivate
{
   GtkWidget *entry;
};

enum
{
   PROP_0,
   PROP_PROJECT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbProject *
gb_create_project_dialog_get_project (GbCreateProjectDialog *dialog)
{
   GbCreateProjectDialogPrivate *priv;
   const gchar *name;
   GbProject *project;

   g_return_val_if_fail(GB_IS_CREATE_PROJECT_DIALOG(dialog), NULL);

   priv = dialog->priv;

   name = gtk_entry_get_text(GTK_ENTRY(priv->entry));
   project = g_object_new(GB_TYPE_PROJECT,
                          "name", name,
                          NULL);

   return project;
}

static void
create_clicked (GtkWidget *button,
                GtkDialog *dialog)
{
   gtk_dialog_response(dialog, GTK_RESPONSE_OK);
}

static void
cancel_clicked (GtkWidget *button,
                GtkDialog *dialog)
{
   gtk_dialog_response(dialog, GTK_RESPONSE_CANCEL);
}

static void
gb_create_project_dialog_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_create_project_dialog_parent_class)->finalize(object);
}

static void
gb_create_project_dialog_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
   GbCreateProjectDialog *dialog = GB_CREATE_PROJECT_DIALOG(object);

   switch (prop_id) {
   case PROP_PROJECT:
      g_value_take_object(value, gb_create_project_dialog_get_project(dialog));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_create_project_dialog_class_init (GbCreateProjectDialogClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_create_project_dialog_finalize;
   object_class->get_property = gb_create_project_dialog_get_property;
   g_type_class_add_private(object_class, sizeof(GbCreateProjectDialogPrivate));

   gParamSpecs[PROP_PROJECT] =
      g_param_spec_object("project",
                          _("Project"),
                          _("The created project."),
                          GB_TYPE_PROJECT,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PROJECT,
                                   gParamSpecs[PROP_PROJECT]);
}

static void
gb_create_project_dialog_init (GbCreateProjectDialog *dialog)
{
   GbCreateProjectDialogPrivate *priv;
   GtkStyleContext *style_context;
   GtkWidget *header_bar;
   GtkWidget *box;
   GtkWidget *l;
   GtkWidget *button;
   GtkWidget *grid;
   gchar *path;

   dialog->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE(dialog,
                                  GB_TYPE_CREATE_PROJECT_DIALOG,
                                  GbCreateProjectDialogPrivate);

   g_object_set(dialog,
                "default-height", 375,
                "default-width", 600,
                "title", _("New Project"),
                NULL);

   box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

   header_bar = g_object_new(GTK_TYPE_HEADER_BAR,
                             "hexpand", TRUE,
                             "show-close-button", FALSE,
                             "title", _("New Project"),
                             "vexpand", FALSE,
                             "visible", TRUE,
                             NULL);
   gtk_window_set_titlebar(GTK_WINDOW(dialog), header_bar);

   button = g_object_new(GTK_TYPE_BUTTON,
                         "label", _("Cancel"),
                         "margin-top", 3,
                         "margin-left", 3,
                         "margin-bottom", 3,
                         "vexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   g_signal_connect(button, "clicked", G_CALLBACK(cancel_clicked), dialog);
   gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), button);

   button = g_object_new(GTK_TYPE_BUTTON,
                         "can-default", TRUE,
                         "label", _("Create"),
                         "margin-top", 3,
                         "margin-right", 3,
                         "margin-bottom", 3,
                         "vexpand", FALSE,
                         "visible", TRUE,
                         NULL);
   g_signal_connect(button, "clicked", G_CALLBACK(create_clicked), dialog);
   style_context = gtk_widget_get_style_context(button);
   gtk_style_context_add_class(style_context, "suggested-action");
   gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), button);
   gtk_window_set_default(GTK_WINDOW(dialog), button);

   grid = g_object_new(GTK_TYPE_GRID,
                       "border-width", 30,
                       "column-spacing", 8,
                       "row-spacing", 6,
                       "visible", TRUE,
                       "vexpand", TRUE,
                       "hexpand", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(box), grid);

   l = g_object_new(GTK_TYPE_LABEL,
                    "label", _("Name"),
                    "hexpand", TRUE,
                    "visible", TRUE,
                    "xalign", 1.0f,
                    NULL);
   gtk_container_add(GTK_CONTAINER(grid), l);

   priv->entry = g_object_new(GTK_TYPE_ENTRY,
                              "visible", TRUE,
                              "width-chars", 30,
                              NULL);
   gtk_entry_set_activates_default(GTK_ENTRY(priv->entry), TRUE);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), priv->entry,
                                     "left-attach", 1,
                                     NULL);
   gtk_widget_grab_focus(priv->entry);

   l = g_object_new(GTK_TYPE_LABEL,
                    "label", NULL,
                    "hexpand", TRUE,
                    "visible", TRUE,
                    NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), l,
                                     "left-attach", 2,
                                     NULL);

   l = g_object_new(GTK_TYPE_LABEL,
                    "label", _("Location"),
                    "hexpand", TRUE,
                    "visible", TRUE,
                    "xalign", 1.0f,
                    NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), l,
                                     "top-attach", 1,
                                     "left-attach", 0,
                                     NULL);

   button = g_object_new(GTK_TYPE_FILE_CHOOSER_BUTTON,
                         "action", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                         "create-folders", TRUE,
                         "visible", TRUE,
                         "width-chars", 30,
                         NULL);
   path = g_build_filename(g_get_home_dir(), "Projects", NULL);
   gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(button), path);
   g_free(path);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), button,
                                     "top-attach", 1,
                                     "left-attach", 1,
                                     NULL);

   l = g_object_new(GTK_TYPE_LABEL,
                    "label", NULL,
                    "hexpand", TRUE,
                    "visible", TRUE,
                    NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), l,
                                     "left-attach", 2,
                                     "top-attach", 1,
                                     NULL);

   l = g_object_new(GTK_TYPE_LABEL,
                    "label", _("GNOME Version"),
                    "hexpand", TRUE,
                    "visible", TRUE,
                    "xalign", 1.0f,
                    NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), l,
                                     "top-attach", 2,
                                     "left-attach", 0,
                                     NULL);

   button = g_object_new(GTK_TYPE_COMBO_BOX_TEXT,
                         "visible", TRUE,
                         NULL);
   gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(button), "3.10");
   gtk_combo_box_set_active(GTK_COMBO_BOX(button), 0);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), button,
                                     "top-attach", 2,
                                     "left-attach", 1,
                                     NULL);

   l = g_object_new(GTK_TYPE_LABEL,
                    "label", NULL,
                    "hexpand", TRUE,
                    "visible", TRUE,
                    NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(grid), l,
                                     "left-attach", 2,
                                     "top-attach", 2,
                                     NULL);
}

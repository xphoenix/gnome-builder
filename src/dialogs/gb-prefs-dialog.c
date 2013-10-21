/* gb-prefs-dialog.c
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

#include "gb-prefs-dialog.h"

struct _GbPrefsDialogPrivate
{
   GtkListStore *sidebar_model;
   GtkWidget    *sidebar;
};

G_DEFINE_TYPE_WITH_CODE(GbPrefsDialog,
                        gb_prefs_dialog,
                        GTK_TYPE_WINDOW,
                        G_ADD_PRIVATE(GbPrefsDialog))

GbPrefsDialog *
gb_prefs_dialog_get_default (void)
{
   static GbPrefsDialog *dialog;

   if (!dialog) {
      dialog = g_object_new(GB_TYPE_PREFS_DIALOG, NULL);
      g_object_add_weak_pointer(G_OBJECT(dialog), (gpointer *)&dialog);
   }

   return dialog;
}

static void
gb_prefs_dialog_finalize (GObject *object)
{
   GbPrefsDialogPrivate *priv = GB_PREFS_DIALOG(object)->priv;

   g_clear_object(&priv->sidebar_model);

   G_OBJECT_CLASS(gb_prefs_dialog_parent_class)->finalize(object);
}

static void
gb_prefs_dialog_class_init (GbPrefsDialogClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_prefs_dialog_finalize;

   widget_class = GTK_WIDGET_CLASS(klass);
   gtk_widget_class_set_template_from_resource(widget_class,
                                               "/org/gnome/Builder/ui/gb-prefs-dialog.ui");
   gtk_widget_class_bind_template_child_private(widget_class, GbPrefsDialog, sidebar);
}

static void
gb_prefs_dialog_init (GbPrefsDialog *dialog)
{
   GbPrefsDialogPrivate *priv;
   GtkTreeViewColumn *column;
   GtkCellRenderer *cell;
   GtkWidget *header;

   dialog->priv = gb_prefs_dialog_get_instance_private(dialog);

   priv = dialog->priv;

   header = g_object_new(GTK_TYPE_HEADER_BAR,
                         "show-close-button", TRUE,
                         "title", _("Preferences"),
                         "visible", TRUE,
                         NULL);
   gtk_window_set_titlebar(GTK_WINDOW(dialog), header);

   gtk_widget_init_template(GTK_WIDGET(dialog));

   priv->sidebar_model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
   gtk_tree_view_set_model(GTK_TREE_VIEW(priv->sidebar),
                           GTK_TREE_MODEL(priv->sidebar_model));

#define ADD_TO_SIDEBAR(name, page) \
   { \
      GtkTreeIter iter; \
      gtk_list_store_append(priv->sidebar_model, &iter); \
      gtk_list_store_set(priv->sidebar_model, &iter, 0, name, 1, page, -1); \
   }
   ADD_TO_SIDEBAR(_("Source Editor"), 0);
   ADD_TO_SIDEBAR(_("Git"), 1);
#undef ADD_TO_SIDEBAR

   column = gtk_tree_view_column_new();
   cell = g_object_new(GTK_TYPE_CELL_RENDERER_TEXT,
                       "xalign", 0.0f,
                       "xpad", 6,
                       "ypad", 6,
                       NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), cell, TRUE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), cell, "text", 0);
   gtk_tree_view_append_column(GTK_TREE_VIEW(priv->sidebar), column);
}

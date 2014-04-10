/* gb-notebook-group.c
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

#include "gb-notebook.h"
#include "gb-notebook-group.h"

struct _GbNotebookGroupPrivate
{
   GSList *notebooks;
};

G_DEFINE_TYPE_WITH_CODE(GbNotebookGroup,
                        gb_notebook_group,
                        GTK_TYPE_BIN,
                        G_ADD_PRIVATE(GbNotebookGroup))

static void
gb_notebook_group_notebook_destroyed (GbNotebookGroup *group,
                                      GbNotebook      *notebook)
{
   GbNotebookGroupPrivate *priv;

   g_return_if_fail(GB_IS_NOTEBOOK_GROUP(group));
   g_return_if_fail(GB_IS_NOTEBOOK(notebook));

   priv = group->priv;

   priv->notebooks = g_slist_remove(priv->notebooks, notebook);
}

static gint
gb_notebook_group_count_paneds (GbNotebookGroup *group)
{
   GtkWidget *child;
   gint count = 0;

   g_return_val_if_fail (GB_IS_NOTEBOOK_GROUP (group), 0);

   child = gtk_bin_get_child(GTK_BIN(group));

   while (GTK_IS_PANED(child)) {
      count++;

      if (GTK_IS_PANED(gtk_paned_get_child2(GTK_PANED(child)))) {
         child = gtk_paned_get_child2(GTK_PANED(child));
         continue;
      }

      return count;
   }

   return 0;
}

static void
gb_notebook_group_relayout (GbNotebookGroup *group)
{
   GbNotebookGroupPrivate *priv;
   GtkAllocation alloc;
   GtkWidget *paned;
   GSList *iter;
   gint position;
   gint count;

   g_return_if_fail(GB_IS_NOTEBOOK_GROUP(group));

   priv = group->priv;

   gtk_widget_get_allocation (GTK_WIDGET (group), &alloc);

   count = gb_notebook_group_count_paneds (group);

   if (!count || !alloc.width) {
      return;
   }

   position = alloc.width / (gdouble)count;

   for (iter = priv->notebooks; iter; iter = iter->next) {
      paned = gtk_widget_get_parent (iter->data);
      gtk_paned_set_position (GTK_PANED (paned), position);
   }
}

static GtkWidget *
gb_notebook_group_get_descendent (GbNotebookGroup *group)
{
   GtkWidget *child;

   g_return_val_if_fail (GB_IS_NOTEBOOK_GROUP (group), NULL);

   child = gtk_bin_get_child(GTK_BIN(group));

   while (GTK_IS_PANED(child)) {
      if (GTK_IS_PANED(gtk_paned_get_child2(GTK_PANED(child)))) {
         child = gtk_paned_get_child2(GTK_PANED(child));
         continue;
      }
      return child;
   }

   return NULL;
}

static void
gb_notebook_group_add_notebook (GbNotebookGroup *group,
                                GbNotebook      *notebook)
{
   GbNotebookGroupPrivate *priv;
   GtkWidget *paned;
   GtkWidget *parent;

   g_return_if_fail(GB_IS_NOTEBOOK_GROUP(group));

   priv = group->priv;

   gtk_widget_set_hexpand (GTK_WIDGET (notebook), TRUE);
   gtk_widget_set_vexpand (GTK_WIDGET (notebook), TRUE);

   paned = gb_notebook_group_get_descendent(group);

   parent = g_object_new(GTK_TYPE_PANED,
                         "orientation", GTK_ORIENTATION_HORIZONTAL,
                         "visible", TRUE,
                         NULL);

   if (!paned) {
      gtk_container_add(GTK_CONTAINER(group), parent);
   } else {
      gtk_paned_add2(GTK_PANED(paned), parent);
   }

   gtk_paned_add1(GTK_PANED(parent), GTK_WIDGET(notebook));

   gtk_container_child_set(GTK_CONTAINER(parent), GTK_WIDGET(notebook),
                           "resize", TRUE,
                           "shrink", TRUE,
                           NULL);

   priv->notebooks = g_slist_append(priv->notebooks, notebook);

   g_signal_connect_object(notebook,
                           "destroy",
                           G_CALLBACK(gb_notebook_group_notebook_destroyed),
                           group,
                           G_CONNECT_SWAPPED);
}

void
gb_notebook_group_add_tab (GbNotebookGroup *group,
                           GbTab           *tab,
                           GbTabLabel      *tab_label,
                           GbNotebook      *notebook,
                           gboolean         new_group)
{
   GbNotebookGroupPrivate *priv;
   gint page;

   g_return_if_fail (GB_IS_NOTEBOOK_GROUP (group));
   g_return_if_fail (GB_IS_TAB (tab));
   g_return_if_fail (GB_IS_TAB_LABEL (tab_label));

   priv = group->priv;

   gtk_widget_set_hexpand (GTK_WIDGET (tab), TRUE);
   gtk_widget_set_vexpand (GTK_WIDGET (tab), TRUE);

   if (new_group || !priv->notebooks) {
      notebook = g_object_new(GB_TYPE_NOTEBOOK,
                              "hexpand", TRUE,
                              "vexpand", TRUE,
                              "visible", TRUE,
                              NULL);
      gb_notebook_group_add_notebook(group, GB_NOTEBOOK(notebook));
   } else if (!notebook) {
      notebook = priv->notebooks->data;
   }

   page = gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                                    GTK_WIDGET (tab),
                                    GTK_WIDGET (tab_label));
   gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);

   gb_notebook_group_relayout (group);
}

static void
gb_notebook_group_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_notebook_group_parent_class)->finalize(object);
}

static void
gb_notebook_group_class_init (GbNotebookGroupClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_notebook_group_finalize;
}

static void
gb_notebook_group_init (GbNotebookGroup *group)
{
   GbNotebook *notebook;

   group->priv = gb_notebook_group_get_instance_private(group);

   notebook = g_object_new(GB_TYPE_NOTEBOOK,
                           "visible", TRUE,
                           NULL);
   gb_notebook_group_add_notebook(group, notebook);
}

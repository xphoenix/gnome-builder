/* gb-workspace-layout-docs.c
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
#include <webkit2/webkit2.h>

#include "gb-workspace-layout-docs.h"

G_DEFINE_TYPE(GbWorkspaceLayoutDocs,
              gb_workspace_layout_docs,
              GB_TYPE_WORKSPACE_LAYOUT)

struct _GbWorkspaceLayoutDocsPrivate
{
   GtkWidget *view;
};

static void
gb_workspace_layout_docs_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_layout_docs_parent_class)->finalize(object);
}

static void
gb_workspace_layout_docs_class_init (GbWorkspaceLayoutDocsClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_layout_docs_finalize;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceLayoutDocsPrivate));
}

static void
gb_workspace_layout_docs_init (GbWorkspaceLayoutDocs *docs)
{
   GtkTreeViewColumn *column;
   GtkCellRenderer *cell;
   GtkListStore *model;
   GtkTreeIter iter;
   GtkWidget *scroller;
   GtkWidget *paned;
   GtkWidget *vpaned;
   GtkWidget *treeview;
   GdkPixbuf *pixbuf;
   GError *error = NULL;

   docs->priv = G_TYPE_INSTANCE_GET_PRIVATE(docs,
                                            GB_TYPE_WORKSPACE_LAYOUT_DOCS,
                                            GbWorkspaceLayoutDocsPrivate);

   paned = g_object_new(GTK_TYPE_PANED,
                        "expand", TRUE,
                        "orientation", GTK_ORIENTATION_HORIZONTAL,
                        "position", 275,
                        "visible", TRUE,
                        NULL);
   gtk_container_add(GTK_CONTAINER(docs), paned);

   vpaned = g_object_new(GTK_TYPE_PANED,
                         "expand", TRUE,
                         "orientation", GTK_ORIENTATION_VERTICAL,
                         "position", 400,
                         "visible", TRUE,
                         NULL);
   gtk_container_add(GTK_CONTAINER(paned), vpaned);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(vpaned), scroller);

   model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

   gtk_list_store_append(model, &iter);
   gtk_list_store_set(model, &iter, 0, NULL, 1, "GObject", -1);

   treeview = g_object_new(GTK_TYPE_TREE_VIEW,
                           "model", model,
                           "headers-visible", FALSE,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(scroller), treeview);

   column = g_object_new(GTK_TYPE_TREE_VIEW_COLUMN,
                         NULL);
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

   cell = g_object_new(GTK_TYPE_CELL_RENDERER_PIXBUF,
                       "width", 16,
                       NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), cell, FALSE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), cell, "pixbuf", 0);

   cell = g_object_new(GTK_TYPE_CELL_RENDERER_TEXT,
                       NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), cell, TRUE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), cell, "text", 1);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(vpaned), scroller);

   model = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

   /*
    * TODO: an icon abstraction for getting pixbufs with overlays.
    */

   pixbuf = gdk_pixbuf_new_from_resource("/org/gnome/Builder/data/icons/class-16x.png", &error);
   if (!pixbuf) {
      g_error("%s", error->message);
   }

   gtk_list_store_append(model, &iter);
   gtk_list_store_set(model, &iter, 0, pixbuf, 1, "GObject", -1);
   gtk_list_store_append(model, &iter);
   gtk_list_store_set(model, &iter, 0, pixbuf, 1, "GInitiallyUnowned", -1);

   pixbuf = gdk_pixbuf_new_from_resource("/org/gnome/Builder/data/icons/enum-16x.png", &error);
   if (!pixbuf) {
      g_error("%s", error->message);
   }

   gtk_list_store_append(model, &iter);
   gtk_list_store_set(model, &iter, 0, pixbuf, 1, "GFileTest", -1);

   treeview = g_object_new(GTK_TYPE_TREE_VIEW,
                           "model", model,
                           "headers-visible", FALSE,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(scroller), treeview);

   column = g_object_new(GTK_TYPE_TREE_VIEW_COLUMN,
                         NULL);
   gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

   cell = g_object_new(GTK_TYPE_CELL_RENDERER_PIXBUF,
                       NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), cell, FALSE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), cell, "pixbuf", 0);

   cell = g_object_new(GTK_TYPE_CELL_RENDERER_TEXT,
                       NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(column), cell, TRUE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(column), cell, "text", 1);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "expand", TRUE,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(paned), scroller);

   docs->priv->view = g_object_new(WEBKIT_TYPE_WEB_VIEW,
                                   "visible", TRUE,
                                   NULL);
   gtk_container_add(GTK_CONTAINER(scroller), docs->priv->view);

#if 0
   {
      gchar *data;
      gsize len;

      /*
       * XXX: Temporary hack to test out ideas for docs.
       */
      if (g_file_get_contents("data/docs.html", &data, &len, NULL)) {
         webkit_web_view_load_html(WEBKIT_WEB_VIEW(docs->priv->view),
                                   data, NULL);
      }
   }
#endif
}

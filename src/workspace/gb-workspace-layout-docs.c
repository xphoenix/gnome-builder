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

#include <devhelp/devhelp.h>
#include <glib/gi18n.h>
#include <webkit2/webkit2.h>

#include "gb-workspace-layout-docs.h"

G_DEFINE_TYPE(GbWorkspaceLayoutDocs,
              gb_workspace_layout_docs,
              GB_TYPE_WORKSPACE_LAYOUT)

struct _GbWorkspaceLayoutDocsPrivate
{
   DhBookManager *books;
   GtkWidget *hpaned;
   GtkWidget *sidebar;
   GtkWidget *scroller;
   GtkWidget *webview;
};

static void
on_link_selected (DhSidebar *sidebar,
                  DhLink    *link_,
                  gpointer   user_data)
{
   GbWorkspaceLayoutDocs *docs = user_data;
   gchar *uri;

   g_return_if_fail(DH_IS_SIDEBAR(sidebar));
   g_return_if_fail(link_);
   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT_DOCS(docs));

   uri = dh_link_get_uri(link_);
   webkit_web_view_load_uri(WEBKIT_WEB_VIEW(docs->priv->webview), uri);
   g_free(uri);
}

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
   docs->priv = G_TYPE_INSTANCE_GET_PRIVATE(docs,
                                            GB_TYPE_WORKSPACE_LAYOUT_DOCS,
                                            GbWorkspaceLayoutDocsPrivate);

   docs->priv->books = dh_book_manager_new();
   dh_book_manager_populate(docs->priv->books);

   docs->priv->hpaned = g_object_new(GTK_TYPE_PANED,
                                     "expand", TRUE,
                                     "orientation", GTK_ORIENTATION_HORIZONTAL,
                                     "position", 325,
                                     "visible", TRUE,
                                     NULL);
   gtk_container_add(GTK_CONTAINER(docs), docs->priv->hpaned);

   docs->priv->sidebar = dh_sidebar_new(docs->priv->books);
   gtk_widget_set_margin_top(docs->priv->sidebar, 6);
   gtk_container_add(GTK_CONTAINER(docs->priv->hpaned),
                     docs->priv->sidebar);
   g_signal_connect(docs->priv->sidebar,
                    "link-selected",
                    G_CALLBACK(on_link_selected),
                    docs);
   gtk_widget_show(docs->priv->sidebar);

   docs->priv->scroller = gtk_scrolled_window_new(NULL, NULL);
   gtk_container_add(GTK_CONTAINER(docs->priv->hpaned),
                     docs->priv->scroller);
   gtk_widget_show(docs->priv->scroller);

   docs->priv->webview = g_object_new(WEBKIT_TYPE_WEB_VIEW,
                                      "visible", TRUE,
                                      NULL);
   gtk_container_add(GTK_CONTAINER(docs->priv->scroller),
                     docs->priv->webview);
}

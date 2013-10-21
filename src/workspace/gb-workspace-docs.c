/* gb-workspace-docs.c
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
#include <devhelp/devhelp.h>
#include <webkit2/webkit2.h>

#include "gb-workspace-docs.h"

G_DEFINE_TYPE(GbWorkspaceDocs, gb_workspace_docs, GB_TYPE_WORKSPACE_SECTION)

struct _GbWorkspaceDocsPrivate
{
   DhBookManager *book_manager;

   GtkWidget *paned;
   GtkWidget *sidebar;
   GtkWidget *scroller;
   GtkWidget *webview;
};

static void
gb_workspace_docs_show (GtkWidget *widget)
{
   GbWorkspaceDocs *docs = (GbWorkspaceDocs *)widget;

   g_return_if_fail(GB_IS_WORKSPACE_DOCS(docs));

   GTK_WIDGET_CLASS(gb_workspace_docs_parent_class)->show(widget);

   dh_book_manager_populate(docs->priv->book_manager);
}

static void
gb_workspace_docs_link_selected (GbWorkspaceDocs *docs,
                                 DhLink          *link_,
                                 DhSidebar       *sidebar)
{
   GbWorkspaceDocsPrivate *priv;
   gchar *uri;

   g_return_if_fail(GB_IS_WORKSPACE_DOCS(docs));
   g_return_if_fail(link_);
   g_return_if_fail(DH_IS_SIDEBAR(sidebar));

   priv = docs->priv;

   uri = dh_link_get_uri(link_);
   webkit_web_view_load_uri(WEBKIT_WEB_VIEW(priv->webview), uri);
   g_free(uri);
}

void
gb_workspace_docs_set_search_term (GbWorkspaceDocs *docs,
                                   const gchar     *search_term)
{
   g_return_if_fail(GB_IS_WORKSPACE_DOCS(docs));

   dh_sidebar_set_search_string(DH_SIDEBAR(docs->priv->sidebar),
                                search_term);
}

static void
gb_workspace_docs_grab_focus (GtkWidget *widget)
{
   GbWorkspaceDocs *docs = (GbWorkspaceDocs *)widget;

   g_return_if_fail(GB_IS_WORKSPACE_DOCS(docs));

   dh_sidebar_set_search_focus(DH_SIDEBAR(docs->priv->sidebar));
}

static void
gb_workspace_docs_finalize (GObject *object)
{
   GbWorkspaceDocsPrivate *priv = GB_WORKSPACE_DOCS(object)->priv;

   g_clear_object(&priv->book_manager);

   G_OBJECT_CLASS(gb_workspace_docs_parent_class)->finalize(object);
}

static void
gb_workspace_docs_class_init (GbWorkspaceDocsClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_docs_finalize;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceDocsPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->show = gb_workspace_docs_show;
   widget_class->grab_focus = gb_workspace_docs_grab_focus;
}

static void
gb_workspace_docs_init (GbWorkspaceDocs *docs)
{
   GbWorkspaceDocsPrivate *priv;

   docs->priv = G_TYPE_INSTANCE_GET_PRIVATE(docs,
                                            GB_TYPE_WORKSPACE_DOCS,
                                            GbWorkspaceDocsPrivate);

   priv = docs->priv;

   priv->paned = g_object_new(GTK_TYPE_PANED,
                              "orientation", GTK_ORIENTATION_HORIZONTAL,
                              "position", 325,
                              "visible", TRUE,
                              NULL);
   gtk_container_add(GTK_CONTAINER(docs), priv->paned);

   priv->book_manager = dh_book_manager_new();

   priv->sidebar = dh_sidebar_new(priv->book_manager);
   g_object_set(priv->sidebar, "margin-top", 3, NULL);
   g_signal_connect_object(priv->sidebar,
                           "link-selected",
                           G_CALLBACK(gb_workspace_docs_link_selected),
                           docs,
                           G_CONNECT_SWAPPED);
   gtk_container_add(GTK_CONTAINER(priv->paned), priv->sidebar);
   gtk_widget_show(priv->sidebar);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->paned), priv->scroller);

   priv->webview = g_object_new(WEBKIT_TYPE_WEB_VIEW,
                                "visible", TRUE,
                                NULL);
   gtk_container_add(GTK_CONTAINER(priv->scroller), priv->webview);
}

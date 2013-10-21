/* gb-workspace-editor.c
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

#include "gb-source-document.h"
#include "gb-tab-label.h"
#include "gb-workspace-editor.h"

G_DEFINE_TYPE(GbWorkspaceEditor,
              gb_workspace_editor,
              GB_TYPE_WORKSPACE_SECTION)

struct _GbWorkspaceEditorPrivate
{
   GtkNotebook *notebook;
};

static void
gb_workspace_editor_close_document (GbWorkspaceEditor *editor,
                                    GbDocument        *document)
{
   GbWorkspaceEditorPrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE_EDITOR(editor));
   g_return_if_fail(GB_IS_DOCUMENT(document));

   priv = editor->priv;

   /*
    * TODO: Check can-save and show dialog?
    */

   gtk_container_remove(GTK_CONTAINER(priv->notebook),
                        GTK_WIDGET(document));
}

static void
on_close_clicked (GbTabLabel        *tab_label,
                  GbWorkspaceEditor *editor)
{
   GbDocument *document;

   g_return_if_fail(GB_IS_TAB_LABEL(tab_label));
   g_return_if_fail(GB_IS_WORKSPACE_EDITOR(editor));

   document = g_object_get_data(G_OBJECT(tab_label), "view");
   if (!GB_IS_DOCUMENT(document)) {
      return;
   }

   gb_workspace_editor_close_document(editor, document);
}

static void
gb_workspace_editor_add_view (GbWorkspaceEditor *editor,
                              GtkWidget         *view)
{
   GbWorkspaceEditorPrivate *priv;
   GtkWidget *tab;

   g_return_if_fail(GB_IS_WORKSPACE_EDITOR(editor));
   g_return_if_fail(GTK_IS_WIDGET(view));

   priv = editor->priv;

   tab = g_object_new(GB_TYPE_TAB_LABEL,
                      "visible", TRUE,
                      NULL);
   g_object_bind_property(view, "title", tab, "title",
                          G_BINDING_SYNC_CREATE);
   g_object_bind_property(view, "can-save", tab, "modified",
                          G_BINDING_SYNC_CREATE);
   g_object_set_data(G_OBJECT(tab), "view", view);
   g_signal_connect_object(tab, "close-clicked",
                           G_CALLBACK(on_close_clicked),
                           editor,
                           0);

   gtk_notebook_append_page(priv->notebook, view, tab);
   gtk_container_child_set(GTK_CONTAINER(priv->notebook), view,
                           "tab-expand", TRUE,
                           "tab-fill", TRUE,
                           NULL);
}

static void
gb_workspace_editor_add (GtkContainer *container,
                         GtkWidget    *child)
{
   GbWorkspaceEditor *editor = (GbWorkspaceEditor *)container;

   g_return_if_fail(GB_IS_WORKSPACE_EDITOR(editor));
   g_return_if_fail(GTK_IS_WIDGET(child));

   if (GB_IS_SOURCE_DOCUMENT(child)) {
      gb_workspace_editor_add_view(editor, child);
   } else {
      GTK_CONTAINER_CLASS(gb_workspace_editor_parent_class)->
         add(container, child);
   }
}

void
gb_workspace_editor_set_current_document (GbWorkspaceEditor *editor,
                                          GbDocument        *document)
{
   GbWorkspaceEditorPrivate *priv;
   gint position = -1;

   g_return_if_fail(GB_IS_WORKSPACE_EDITOR(editor));
   g_return_if_fail(GB_IS_SOURCE_DOCUMENT(document));

   priv = editor->priv;

   gtk_container_child_get(GTK_CONTAINER(priv->notebook),
                           GTK_WIDGET(document),
                           "position", &position,
                           NULL);
   if (position >= 0) {
      gtk_notebook_set_current_page(priv->notebook, position);
   }
}

static void
gb_workspace_editor_switch_page (GtkNotebook       *notebook,
                                 GtkWidget         *page,
                                 guint              page_num,
                                 GbWorkspaceEditor *editor)
{
   g_return_if_fail(GTK_IS_NOTEBOOK(notebook));
   g_return_if_fail(GB_IS_DOCUMENT(page));
   g_return_if_fail(GB_IS_WORKSPACE_EDITOR(editor));

   gb_workspace_section_set_current_document(GB_WORKSPACE_SECTION(editor),
                                             GB_DOCUMENT(page));
}

static void
gb_workspace_editor_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_editor_parent_class)->finalize(object);
}

static void
gb_workspace_editor_class_init (GbWorkspaceEditorClass *klass)
{
   GObjectClass *object_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_editor_finalize;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceEditorPrivate));

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_workspace_editor_add;
}

static void
gb_workspace_editor_init (GbWorkspaceEditor *editor)
{
   GbWorkspaceEditorPrivate *priv;

   editor->priv = G_TYPE_INSTANCE_GET_PRIVATE(editor,
                                              GB_TYPE_WORKSPACE_EDITOR,
                                              GbWorkspaceEditorPrivate);

   priv = editor->priv;

   priv->notebook = g_object_new(GTK_TYPE_NOTEBOOK,
                                 "scrollable", TRUE,
                                 "enable-popup", TRUE,
                                 "visible", TRUE,
                                 NULL);
   g_signal_connect_object(priv->notebook,
                           "switch-page",
                           G_CALLBACK(gb_workspace_editor_switch_page),
                           editor,
                           G_CONNECT_AFTER);
   gtk_container_add(GTK_CONTAINER(editor), GTK_WIDGET(priv->notebook));
}

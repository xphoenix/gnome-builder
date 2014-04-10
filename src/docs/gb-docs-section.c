/* gb-docs-section.c
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

#include "gb-docs-section.h"
#include "gb-docs-tab.h"
#include "gb-tab-label.h"

struct _GbDocsSectionPrivate
{
   GSimpleActionGroup *actions;
   DhBookManager *book_manager;

   GtkWidget *notebook;
   GtkWidget *paned;
   GtkWidget *sidebar;
};

G_DEFINE_TYPE_WITH_CODE(GbDocsSection,
                        gb_docs_section,
                        GB_TYPE_WORKSPACE_SECTION,
                        G_ADD_PRIVATE(GbDocsSection))

static GActionGroup *
gb_docs_section_get_actions (GbWorkspaceSection *section)
{
   GbDocsSection *docs = (GbDocsSection *)section;

   g_return_val_if_fail(GB_IS_DOCS_SECTION(docs), NULL);

   return G_ACTION_GROUP(docs->priv->actions);
}

static void
gb_docs_section_update_style (GbDocsSection *section)
{
   GbDocsSectionPrivate *priv;
   gboolean show_tabs;
   gint n_pages;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));

   priv = section->priv;

   n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(priv->notebook));
   show_tabs = n_pages > 1;

   gtk_notebook_set_show_tabs(GTK_NOTEBOOK(priv->notebook), show_tabs);
}

static void
gb_docs_section_close_clicked (GbDocsSection *section,
                               GbTabLabel    *label)
{
   GbDocsSectionPrivate *priv;
   GbDocsTab *tab;
   gint n_pages;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));
   g_return_if_fail(GB_IS_TAB_LABEL(label));

   priv = section->priv;

   n_pages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(priv->notebook));

   if (n_pages > 1) {
      tab = GB_DOCS_TAB(gb_tab_label_get_tab(label));
      gtk_container_remove(GTK_CONTAINER(section->priv->notebook),
                           GTK_WIDGET(tab));
      gb_docs_section_update_style(section);
   }
}

static void
gb_docs_section_add_tab (GbDocsSection *section)
{
   GbDocsSectionPrivate *priv;
   GtkWidget *tab;
   GtkWidget *tab_label;
   gint page;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));

   priv = section->priv;

   tab = g_object_new(GB_TYPE_DOCS_TAB,
                      "visible", TRUE,
                      NULL);
   tab_label = g_object_new(GB_TYPE_TAB_LABEL,
                            "tab", tab,
                            "visible", TRUE,
                            NULL);
   g_signal_connect_object(tab_label,
                           "close-clicked",
                           G_CALLBACK(gb_docs_section_close_clicked),
                           section,
                           G_CONNECT_SWAPPED);
   page = gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook),
                                   tab,
                                   tab_label);
   gtk_notebook_set_current_page(GTK_NOTEBOOK(priv->notebook), page);

   gb_docs_section_update_style(section);
}

static void
on_link_selected (GbDocsSection *section,
                  DhLink        *link_,
                  DhSidebar     *sidebar)
{
   GbDocsSectionPrivate *priv;
   GtkWidget *tab;
   gint page;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));
   g_return_if_fail(link_);
   g_return_if_fail(DH_IS_SIDEBAR(sidebar));

   priv = section->priv;

   page = gtk_notebook_get_current_page(GTK_NOTEBOOK(priv->notebook));
   if (page < 0) {
      gb_docs_section_add_tab(section);
      page = 0;
   }

   tab = gtk_notebook_get_nth_page(GTK_NOTEBOOK(priv->notebook), page);
   g_assert(GB_IS_DOCS_TAB(tab));

   gb_docs_tab_set_link(GB_DOCS_TAB(tab), link_);
}

static void
on_new_tab_activate (GSimpleAction *action,
                     GVariant      *parameter,
                     gpointer       user_data)
{
   GbDocsSection *section = user_data;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));

   gb_docs_section_add_tab(section);
}

static void
on_focus_search_activate (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
   GbDocsSection *section = user_data;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));

   dh_sidebar_set_search_focus(DH_SIDEBAR(section->priv->sidebar));
}

static void
on_close_activate (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
   GbDocsSectionPrivate *priv;
   GbDocsSection *section = user_data;
   GtkNotebook *notebook;
   GtkWidget *child;
   gint page;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));

   priv = section->priv;

   notebook = GTK_NOTEBOOK(priv->notebook);

   if (gtk_notebook_get_n_pages(notebook) > 1) {
      page = gtk_notebook_get_current_page(notebook);
      if (page > -1) {
         child = gtk_notebook_get_nth_page(notebook, page);
         gtk_container_remove(GTK_CONTAINER(notebook), child);
         gb_docs_section_update_style(section);
      }
   }
}

static void
gb_docs_section_constructed (GObject *object)
{
   GbDocsSection *section = (GbDocsSection *)object;

   g_return_if_fail(GB_IS_DOCS_SECTION(section));

   dh_book_manager_populate(section->priv->book_manager);
}

static void
gb_docs_section_finalize (GObject *object)
{
   GbDocsSectionPrivate *priv = GB_DOCS_SECTION(object)->priv;

   g_clear_object(&priv->actions);
   g_clear_object(&priv->book_manager);

   G_OBJECT_CLASS(gb_docs_section_parent_class)->finalize(object);
}

static void
gb_docs_section_class_init (GbDocsSectionClass *klass)
{
   GObjectClass *object_class;
   GbWorkspaceSectionClass *section_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->constructed = gb_docs_section_constructed;
   object_class->finalize = gb_docs_section_finalize;

   section_class = GB_WORKSPACE_SECTION_CLASS(klass);
   section_class->get_actions = gb_docs_section_get_actions;
}

static void
gb_docs_section_init (GbDocsSection *section)
{
   static const GActionEntry entries[] = {
      { "focus-search", on_focus_search_activate },
      { "new-tab", on_new_tab_activate },
      { "close", on_close_activate },
   };
   GbDocsSectionPrivate *priv;

   priv = section->priv = gb_docs_section_get_instance_private(section);

   priv->actions = g_simple_action_group_new();
   g_action_map_add_action_entries(G_ACTION_MAP(priv->actions),
                                   entries,
                                   G_N_ELEMENTS(entries),
                                   section);

   priv->book_manager = dh_book_manager_new();

   priv->paned = g_object_new(GTK_TYPE_PANED,
                              "orientation", GTK_ORIENTATION_HORIZONTAL,
                              "position", 325,
                              "visible", TRUE,
                              NULL);
   gtk_container_add(GTK_CONTAINER(section), priv->paned);

   priv->sidebar = dh_sidebar_new(priv->book_manager);
   g_object_set(priv->sidebar, "margin-top", 3, NULL);
   g_signal_connect_object(priv->sidebar,
                           "link-selected",
                           G_CALLBACK(on_link_selected),
                           section,
                           G_CONNECT_SWAPPED);
   gtk_container_add(GTK_CONTAINER(priv->paned), priv->sidebar);
   gtk_widget_show(priv->sidebar);

   priv->notebook = g_object_new(GTK_TYPE_NOTEBOOK,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->paned), priv->notebook);

   gb_docs_section_add_tab(section);
}

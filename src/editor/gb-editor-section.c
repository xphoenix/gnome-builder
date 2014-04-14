/* gb-editor-section.c
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

#include "gb-editor-section.h"
#include "gb-editor-tab.h"
#include "gb-notebook.h"
#include "gb-notebook-group.h"
#include "gb-tab-label.h"

struct _GbEditorSectionPrivate
{
   GSimpleActionGroup *actions;

   GtkWidget *notebook_group;
   GtkWidget *last_tab;
};

G_DEFINE_TYPE_WITH_CODE(GbEditorSection,
                        gb_editor_section,
                        GB_TYPE_WORKSPACE_SECTION,
                        G_ADD_PRIVATE(GbEditorSection))

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_editor_section_action_set (GbEditorSection *section,
                              const gchar     *action_name,
                              const gchar     *first_property,
                              ...) G_GNUC_NULL_TERMINATED;

static void
gb_editor_section_action_set (GbEditorSection *section,
                              const gchar     *action_name,
                              const gchar     *first_property,
                              ...)
{
   GbEditorSectionPrivate *priv;
   GAction *action;
   va_list args;

   g_return_if_fail(GB_IS_EDITOR_SECTION(section));
   g_return_if_fail(action_name);
   g_return_if_fail(first_property);

   priv = section->priv;

   action = g_action_map_lookup_action (G_ACTION_MAP (priv->actions),
                                        action_name);

   if (!action) {
      g_warning ("No such action: \"%s\"", action_name);
      return;
   }

   va_start (args, first_property);
   g_object_set_valist (G_OBJECT (action), first_property, args);
   va_end (args);
}

static void
gb_editor_section_set_last_document (GbEditorSection *section,
                                     GbEditorTab     *tab)
{
   GbEditorSectionPrivate *priv;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));
   g_return_if_fail (!tab || GB_IS_EDITOR_TAB (tab));

   priv = section->priv;

   if (priv->last_tab) {
      g_object_remove_weak_pointer (G_OBJECT (priv->last_tab),
                                    (gpointer *)&priv->last_tab);
      priv->last_tab = NULL;
   }

   if (tab) {
      priv->last_tab = GTK_WIDGET (tab);
      g_object_add_weak_pointer (G_OBJECT (priv->last_tab),
                                 (gpointer *)&priv->last_tab);
   }

   gb_editor_section_action_set (section, "save-document",
                                 "enabled", !!tab,
                                 NULL);
}

static void
gb_editor_section_tab_focused (GbEditorSection *section,
                               GbEditorTab     *tab)
{
   g_return_if_fail (GB_IS_EDITOR_SECTION (section));
   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   gb_editor_section_set_last_document (section, tab);
}

static void
gb_editor_section_close_clicked (GbEditorSection *section,
                                 GbTabLabel      *tab_label)
{
   GtkWidget *notebook;
   GtkWidget *tab;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));
   g_return_if_fail (GB_IS_TAB_LABEL (tab_label));

   tab = gb_tab_label_get_tab (tab_label);

   if (gb_tab_get_can_save (GB_TAB (tab))) {
      /*
       * TODO: Ask if they want to save first.
       */
   }

   notebook = gtk_widget_get_parent (tab);

   gtk_container_remove (GTK_CONTAINER (notebook), tab);
}

static void
gb_editor_section_add_tab (GbEditorSection *section,
                           GbTab           *tab,
                           GbTabLabel      *tab_label,
                           gboolean         new_group)
{
   GbEditorSectionPrivate *priv;
   GbNotebook *notebook = NULL;
   GtkWidget *parent;
   GList *list;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));
   g_return_if_fail (GB_IS_TAB (tab));
   g_return_if_fail (GB_IS_TAB_LABEL (tab_label));

   priv = section->priv;

   if (priv->last_tab) {
      parent = priv->last_tab;

      do {
         parent = gtk_widget_get_parent (parent);
      } while (parent && !GB_IS_NOTEBOOK (parent));

      if (parent) {
         notebook = GB_NOTEBOOK (parent);
      }

      if (!new_group && notebook) {
         list = gtk_container_get_children (GTK_CONTAINER (notebook));
         if (g_list_length (list) == 1) {
            if (gb_editor_tab_get_is_empty (list->data)) {
               gtk_container_remove (GTK_CONTAINER (notebook), list->data);
            }
         }
         g_list_free (list);
      }
   }

   gb_notebook_group_add_tab (GB_NOTEBOOK_GROUP (priv->notebook_group), tab,
                              tab_label, notebook, new_group);

   gb_editor_section_set_last_document (section, GB_EDITOR_TAB (tab));

   g_signal_connect_object (tab,
                            "focused",
                            G_CALLBACK (gb_editor_section_tab_focused),
                            section,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   g_signal_connect_object (tab_label,
                            "close-clicked",
                            G_CALLBACK (gb_editor_section_close_clicked),
                            section,
                            G_CONNECT_SWAPPED);

   gtk_widget_grab_focus (GTK_WIDGET (tab));
}

static void
gb_editor_section_on_new_document (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       user_data)
{
   GbEditorSection *section = user_data;
   GbEditorTab *tab;
   GbTabLabel *tab_label;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));

   tab = g_object_new(GB_TYPE_EDITOR_TAB,
                      "title", _("Unsaved Document"),
                      "visible", TRUE,
                      NULL);

   tab_label = g_object_new(GB_TYPE_TAB_LABEL,
                            "tab", tab,
                            NULL);

   gb_editor_section_add_tab (section, GB_TAB (tab), tab_label, FALSE);
}

static void
gb_editor_section_on_close (GSimpleAction *action,
                            GVariant      *parameter,
                            gpointer       user_data)
{
   GbEditorSectionPrivate *priv;
   GbEditorSection *section = user_data;
   GtkWidget *notebook;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));

   priv = section->priv;

   if (priv->last_tab) {
      if (gb_tab_get_can_save (GB_TAB (priv->last_tab))) {
         /*
          * TODO: Check if we should save.
          */
      }

      notebook = gtk_widget_get_parent (priv->last_tab);
      gtk_container_remove (GTK_CONTAINER (notebook), priv->last_tab);
   }
}

static void
gb_editor_section_on_open_document (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
   GbEditorSection *section = user_data;
   GbTabLabel *tab_label;
   GtkWidget *dialog;
   GtkWidget *toplevel;
   GSList *files;
   GSList *iter;
   GbTab *tab;
   gint ret;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));

   toplevel = gtk_widget_get_toplevel(GTK_WIDGET(section));

   dialog = g_object_new(GTK_TYPE_FILE_CHOOSER_DIALOG,
                         "action", GTK_FILE_CHOOSER_ACTION_OPEN,
                         "local-only", FALSE,
                         "select-multiple", TRUE,
                         "title", _("Open"),
                         "transient-for", toplevel,
                         "visible", TRUE,
                         NULL);

   gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                          _("Cancel"), GTK_RESPONSE_CANCEL,
                          _("Open"), GTK_RESPONSE_OK,
                          NULL);
   gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

   ret = gtk_dialog_run(GTK_DIALOG(dialog));

   if (ret == GTK_RESPONSE_OK) {
      files = gtk_file_chooser_get_files (GTK_FILE_CHOOSER (dialog));

      for (iter = files; iter; iter = iter->next) {
         tab = g_object_new(GB_TYPE_EDITOR_TAB,
                            "visible", TRUE,
                            NULL);

         tab_label = g_object_new(GB_TYPE_TAB_LABEL,
                                  "tab", tab,
                                  "visible", TRUE,
                                  NULL);

         gb_editor_tab_open (GB_EDITOR_TAB (tab), iter->data);

         gb_editor_section_add_tab (section, tab, tab_label, FALSE);
      }

      g_slist_foreach (files, (GFunc)g_object_unref, NULL);
      g_slist_free (files);
   }

   gtk_widget_destroy(dialog);
}

static void
gb_editor_section_on_new_tab_group (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
   GbEditorSection *section = user_data;
   GbTabLabel *tab_label;
   GbTab *tab;

   g_return_if_fail(GB_IS_EDITOR_SECTION(section));

   tab = g_object_new(GB_TYPE_EDITOR_TAB,
                      "title", _("Unsaved Document"),
                      "visible", TRUE,
                      NULL);

   tab_label = g_object_new(GB_TYPE_TAB_LABEL,
                            "tab", tab,
                            "visible", TRUE,
                            NULL);

   gb_editor_section_add_tab (section, tab, tab_label, TRUE);
}

static void
gb_editor_section_on_save_document (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
   GbEditorSection *section = user_data;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));

   if (!section->priv->last_tab) {
      g_warning ("No document to save.");
      return;
   }

   gb_editor_tab_save (GB_EDITOR_TAB (section->priv->last_tab));
}

static void
gb_editor_section_on_find (GSimpleAction *action,
                           GVariant      *parameter,
                           gpointer       user_data)
{
   GbEditorSection *section = user_data;

   g_return_if_fail (GB_IS_EDITOR_SECTION (section));

   if (!section->priv->last_tab) {
      return;
   }

   gb_editor_tab_find (GB_EDITOR_TAB (section->priv->last_tab));
}

static GActionGroup *
gb_editor_section_get_actions (GbWorkspaceSection *section)
{
   g_return_val_if_fail(GB_IS_EDITOR_SECTION(section), NULL);

   return G_ACTION_GROUP(GB_EDITOR_SECTION(section)->priv->actions);
}

static void
gb_editor_section_finalize (GObject *object)
{
   GbEditorSectionPrivate *priv = GB_EDITOR_SECTION(object)->priv;

   g_clear_object(&priv->actions);

   G_OBJECT_CLASS(gb_editor_section_parent_class)->finalize(object);
}

static void
gb_editor_section_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
   //GbEditorSection *section = GB_EDITOR_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_editor_section_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
   //GbEditorSection *section = GB_EDITOR_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_editor_section_class_init (GbEditorSectionClass *klass)
{
   GObjectClass *object_class;
   GbWorkspaceSectionClass *section_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_editor_section_finalize;
   object_class->get_property = gb_editor_section_get_property;
   object_class->set_property = gb_editor_section_set_property;

   section_class = GB_WORKSPACE_SECTION_CLASS(klass);
   section_class->get_actions = gb_editor_section_get_actions;
}

static void
gb_editor_section_init (GbEditorSection *section)
{
   GbEditorSectionPrivate *priv;
   static const GActionEntry entries[] = {
      { "close", gb_editor_section_on_close },
      { "new-document", gb_editor_section_on_new_document },
      { "new-tab-group", gb_editor_section_on_new_tab_group },
      { "open-document", gb_editor_section_on_open_document },
      { "save-document", gb_editor_section_on_save_document },
      { "find", gb_editor_section_on_find },
   };

   priv = section->priv = gb_editor_section_get_instance_private(section);

   priv->actions = g_simple_action_group_new();
   g_action_map_add_action_entries(G_ACTION_MAP(priv->actions),
                                   entries,
                                   G_N_ELEMENTS(entries),
                                   section);

   priv->notebook_group = g_object_new(GB_TYPE_NOTEBOOK_GROUP,
                                       "visible", TRUE,
                                       NULL);
   gtk_container_add(GTK_CONTAINER(section), priv->notebook_group);

   gb_editor_section_action_set (section, "save-document",
                                 "enabled", FALSE,
                                 NULL);

   gb_editor_section_on_new_document (NULL, NULL, section);
}

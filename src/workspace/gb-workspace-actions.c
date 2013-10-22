/* gb-workspace-actions.c
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
#include "gb-prefs-dialog.h"
#include "gb-source-document.h"
#include "gb-workspace.h"
#include "gb-workspace-editor.h"
#include "gb-workspace-docs.h"

typedef struct
{
   const gchar *accel;
   const gchar *action;
} Accelerator;

static const Accelerator gAccelators[] = {
   { "<Primary>K",     "win.search-docs-by-word" },
   { "<Primary>N",     "win.new-document" },
   { "<Primary>O",     "win.open-document" },
   { "<Primary>S",     "win.save-document" },
   { "<Primary>comma", "win.preferences" },
   { "<Control>1",     "win.move-to-glade" },
   { "<Control>2",     "win.move-to-editor" },
   { "<Control>5",     "win.move-to-docs" },
   { NULL }
};

static void
gb_workspace_actions_add_accelerators (GbWorkspace *workspace)
{
   gint i;

   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   for (i = 0; gAccelators[i].accel; i++) {
      gtk_application_add_accelerator(GTK_APPLICATION(GB_APPLICATION_DEFAULT),
                                      gAccelators[i].accel,
                                      gAccelators[i].action,
                                      NULL);
   }
}

static void
gb_workspace_actions_new_document (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       user_data)
{
   GbWorkspaceEditor *editor;
   GbSourceDocument *document;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   /*
    * TODO: How should we add this to the proper section?
    * TODO: Should we add history point into navigation?
    * TODO: Focus the application on the new section.
    */

   if (!(editor = gb_workspace_get_editor(workspace))) {
      return;
   }

   gb_workspace_set_current_section(workspace, GB_WORKSPACE_SECTION(editor));

   document = g_object_new(GB_TYPE_SOURCE_DOCUMENT,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(editor), GTK_WIDGET(document));
   gb_workspace_editor_set_current_document(editor, GB_DOCUMENT(document));
   gtk_widget_grab_focus(GTK_WIDGET(document));
}

static void
gb_workspace_actions_close_document (GSimpleAction *action,
                                     GVariant      *parameter,
                                     gpointer       user_data)
{
   GbWorkspace *workspace = user_data;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   /*
    * TODO: Close the current panel.
    */
}

static void
save_cb (GObject      *object,
         GAsyncResult *result,
         gpointer      user_data)
{
   GbDocument *document = (GbDocument *)object;
   GError *error = NULL;

   if (!gb_document_save_finish(document, result, &error)) {
      /* TODO: Is there anything to do with the error from here?
       *       I think the GtkInfoBar will be used within the editor
       *       anyway.
       */
      g_warning("%s", error->message);
      g_error_free(error);
   }
}

static void
open_cb (GObject      *object,
         GAsyncResult *result,
         gpointer      user_data)
{
   GbWorkspaceEditor *editor = user_data;
   GbSourceDocument *document = (GbSourceDocument *)object;
   GError *error = NULL;

   if (!gb_source_document_load_file_finish(document, result, &error)) {
      g_warning("%s", error->message);
      g_error_free(error);
      goto cleanup;
   }

   gtk_widget_set_sensitive(GTK_WIDGET(document), TRUE);
   gtk_widget_grab_focus(GTK_WIDGET(document));

cleanup:
   g_object_unref(editor);
}

static void
gb_workspace_actions_open_document (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
   GbWorkspaceEditor *editor;
   GbWorkspace *workspace = user_data;
   GbDocument *document;
   GtkDialog *dialog;
   GSList *list;
   GSList *iter;

   /*
    * TODO: We need some sort of registry or callback to determine
    *       what GbDocument implementation to use for a file.
    */

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   if (!(editor = gb_workspace_get_editor(workspace))) {
      return;
   }

   dialog = g_object_new(GTK_TYPE_FILE_CHOOSER_DIALOG,
                         "action", GTK_FILE_CHOOSER_ACTION_OPEN,
                         "local-only", FALSE,
                         "select-multiple", TRUE,
                         "title", _("Open File"),
                         "transient-for", workspace,
                         NULL);
   gtk_dialog_add_buttons(dialog,
                          _("Cancel"), GTK_RESPONSE_CANCEL,
                          _("Open"), GTK_RESPONSE_OK,
                          NULL);

   if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
      gb_workspace_set_current_section(workspace,
                                       GB_WORKSPACE_SECTION(editor));
      list = gtk_file_chooser_get_files(GTK_FILE_CHOOSER(dialog));
      for (iter = list; iter; iter = iter->next) {
         document = g_object_new(GB_TYPE_SOURCE_DOCUMENT,
                                 "file", iter->data,
                                 "sensitive", FALSE,
                                 "visible", TRUE,
                                 NULL);
         gtk_container_add(GTK_CONTAINER(editor), GTK_WIDGET(document));
         gtk_widget_realize(GTK_WIDGET(document));
         gb_source_document_load_file_async(GB_SOURCE_DOCUMENT(document),
                                            iter->data,
                                            NULL,
                                            open_cb,
                                            g_object_ref(editor));
         gb_workspace_editor_set_current_document(editor,
                                                  GB_DOCUMENT(document));
      }
      g_slist_foreach(list, (GFunc)g_object_unref, NULL);
      g_slist_free(list);
   }

   gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void
gb_workspace_actions_save_document (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
   GbWorkspace *workspace = user_data;
   GbDocument *document;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   document = gb_workspace_get_current_document(workspace);
   if (document) {
      if (gb_document_get_can_save(document)) {
         gb_document_save_async(document, NULL, save_cb, NULL);
      }
   }
}

static void
gb_workspace_actions_move_to_glade (GSimpleAction *action,
                                    GVariant      *parameter,
                                    gpointer       user_data)
{
   GbWorkspaceGlade *glade;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   if ((glade = gb_workspace_get_glade(workspace))) {
      gb_workspace_set_current_section(workspace,
                                       GB_WORKSPACE_SECTION(glade));
   }
}

static void
gb_workspace_actions_move_to_docs (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       user_data)
{
   GbWorkspaceDocs *docs;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   if ((docs = gb_workspace_get_docs(workspace))) {
      gb_workspace_set_current_section(workspace,
                                       GB_WORKSPACE_SECTION(docs));
   }
}

static void
gb_workspace_actions_move_to_editor (GSimpleAction *action,
                                     GVariant      *parameter,
                                     gpointer       user_data)
{
   GbWorkspaceEditor *editor;
   GbWorkspace *workspace = user_data;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   if ((editor = gb_workspace_get_editor(workspace))) {
      gb_workspace_set_current_section(workspace,
                                       GB_WORKSPACE_SECTION(editor));
   }
}

static void
gb_workspace_actions_search_docs_by_word (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       user_data)
{
   GbWorkspaceDocs *docs;
   GbWorkspace *workspace = user_data;
   GbDocument *document;
   gchar *word;

   g_return_if_fail(G_IS_SIMPLE_ACTION(action));
   g_return_if_fail(GB_IS_WORKSPACE(workspace));

   docs = gb_workspace_get_docs(workspace);
   if (!docs) {
      return;
   }

   /*
    * TODO: Support more than GbSourceDocument?
    */

   document = gb_workspace_get_current_document(workspace);
   if (!GB_IS_SOURCE_DOCUMENT(document)) {
      return;
   }

   word = gb_source_document_get_current_word(GB_SOURCE_DOCUMENT(document));
   gb_workspace_docs_set_search_term(docs, word);
   g_free(word);

   gb_workspace_set_current_section(workspace,
                                    GB_WORKSPACE_SECTION(docs));

   gtk_widget_grab_focus(GTK_WIDGET(docs));
}

static void
gb_workspace_actions_preferences (GSimpleAction *action,
                                  GVariant      *parameter,
                                  gpointer       user_data)
{
   gtk_window_present(GTK_WINDOW(GB_PREFS_DIALOG_DEFAULT));
}

void
_gb_workspace_actions_init (GbWorkspace *workspace)
{
   static const GActionEntry entries[] = {
      { "new-document", gb_workspace_actions_new_document },
      { "close-document", gb_workspace_actions_close_document },
      { "move-to-docs", gb_workspace_actions_move_to_docs },
      { "move-to-editor", gb_workspace_actions_move_to_editor },
      { "move-to-glade", gb_workspace_actions_move_to_glade },
      { "open-document", gb_workspace_actions_open_document },
      { "preferences", gb_workspace_actions_preferences },
      { "save-document", gb_workspace_actions_save_document },
      { "search-docs-by-word", gb_workspace_actions_search_docs_by_word },
   };

   g_action_map_add_action_entries(G_ACTION_MAP(workspace),
                                   entries,
                                   G_N_ELEMENTS(entries),
                                   workspace);

   gb_workspace_actions_add_accelerators(workspace);
}

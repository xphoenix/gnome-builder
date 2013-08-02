/* gb-source-actions.c
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

#include "gb-action.h"
#include "gb-application.h"
#include "gb-search-provider.h"
#include "gb-workspace.h"

static gboolean
gb_source_actions_focus_search (GbAction  *action,
                                 GVariant  *parameter,
                                 GError   **error)
{
   GbWorkspace *workspace;
   GtkWidget *focus;

   workspace = gb_application_get_workspace(GB_APPLICATION_DEFAULT);
   focus = gtk_window_get_focus(GTK_WINDOW(workspace));

   for (; focus; focus = gtk_widget_get_parent(focus)) {
      if (GB_IS_SEARCH_PROVIDER(focus)) {
         gb_search_provider_focus_search(GB_SEARCH_PROVIDER(focus));
         break;
      }
   }
}


GB_ACTION_REGISTER_STATIC("focus-search", gb_source_actions_focus_search);

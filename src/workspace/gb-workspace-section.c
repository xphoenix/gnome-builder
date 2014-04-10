/* gb-workspace-section.c
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

#include "gb-workspace-section.h"

struct _GbWorkspaceSectionPrivate
{
   gpointer dummy;
};

G_DEFINE_ABSTRACT_TYPE_WITH_CODE(GbWorkspaceSection,
                                 gb_workspace_section,
                                 GTK_TYPE_BIN,
                                 G_ADD_PRIVATE(GbWorkspaceSection))

/**
 * gb_workspace_section_get_actions:
 * @section: (in): A #GbWorkspaceSection.
 *
 * Fetches a #GActionGroup for actions within this section.
 *
 * Returns: (transfer none): A #GActionGroup.
 */
GActionGroup *
gb_workspace_section_get_actions (GbWorkspaceSection *section)
{
   GbWorkspaceSectionClass *klass;

   g_return_val_if_fail(GB_IS_WORKSPACE_SECTION(section), NULL);

   klass = GB_WORKSPACE_SECTION_GET_CLASS(section);

   if (klass->get_actions) {
      return klass->get_actions(section);
   }

   return NULL;
}

void
gb_workspace_section_set_project (GbWorkspaceSection *section,
                                  GbProject          *project)
{
   GbWorkspaceSectionClass *klass;

   g_return_if_fail(GB_IS_WORKSPACE_SECTION(section));
   g_return_if_fail(!project || GB_IS_PROJECT(project));

   klass = GB_WORKSPACE_SECTION_GET_CLASS(section);

   if (klass->set_project) {
      klass->set_project(section, project);
   }
}

static void
gb_workspace_section_class_init (GbWorkspaceSectionClass *klass)
{
}

static void
gb_workspace_section_init (GbWorkspaceSection *section)
{
   section->priv = gb_workspace_section_get_instance_private(section);
}

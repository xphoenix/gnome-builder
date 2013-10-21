/* gb-workspace.h
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

#ifndef GB_WORKSPACE_H
#define GB_WORKSPACE_H

#include <gtk/gtk.h>

#include "gb-back-forward-list.h"
#include "gb-command-stack.h"
#include "gb-document.h"
#include "gb-project.h"
#include "gb-workspace-docs.h"
#include "gb-workspace-editor.h"

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE            (gb_workspace_get_type())
#define GB_WORKSPACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE, GbWorkspace))
#define GB_WORKSPACE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE, GbWorkspace const))
#define GB_WORKSPACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE, GbWorkspaceClass))
#define GB_IS_WORKSPACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE))
#define GB_IS_WORKSPACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE))
#define GB_WORKSPACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE, GbWorkspaceClass))

typedef struct _GbWorkspace        GbWorkspace;
typedef struct _GbWorkspaceClass   GbWorkspaceClass;
typedef struct _GbWorkspacePrivate GbWorkspacePrivate;

struct _GbWorkspace
{
   GtkApplicationWindow parent;

   /*< private >*/
   GbWorkspacePrivate *priv;
};

struct _GbWorkspaceClass
{
   GtkApplicationWindowClass parent_class;
};

GType               gb_workspace_get_type              (void) G_GNUC_CONST;
GbWorkspace        *gb_workspace_new                   (void);
GbBackForwardList  *gb_workspace_get_back_forward_list (GbWorkspace        *workspace);
GbCommandStack     *gb_workspace_get_command_stack     (GbWorkspace        *workspace);
GbProject          *gb_workspace_get_project           (GbWorkspace        *workspace);
void                gb_workspace_set_project           (GbWorkspace        *workspace,
                                                        GbProject          *project);
GbWorkspaceDocs    *gb_workspace_get_docs              (GbWorkspace        *workspace);
GbWorkspaceEditor  *gb_workspace_get_editor            (GbWorkspace        *workspace);
GbDocument         *gb_workspace_get_current_document  (GbWorkspace        *workspace);
GbWorkspaceSection *gb_workspace_get_current_section   (GbWorkspace        *workspace);
void                gb_workspace_set_current_section   (GbWorkspace        *workspace,
                                                        GbWorkspaceSection *section);

G_END_DECLS

#endif /* GB_WORKSPACE_H */

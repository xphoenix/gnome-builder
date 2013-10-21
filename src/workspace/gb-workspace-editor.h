/* gb-workspace-editor.h
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

#ifndef GB_WORKSPACE_EDITOR_H
#define GB_WORKSPACE_EDITOR_H

#include "gb-document.h"
#include "gb-workspace-section.h"

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_EDITOR            (gb_workspace_editor_get_type())
#define GB_WORKSPACE_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_EDITOR, GbWorkspaceEditor))
#define GB_WORKSPACE_EDITOR_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_EDITOR, GbWorkspaceEditor const))
#define GB_WORKSPACE_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_EDITOR, GbWorkspaceEditorClass))
#define GB_IS_WORKSPACE_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_EDITOR))
#define GB_IS_WORKSPACE_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_EDITOR))
#define GB_WORKSPACE_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_EDITOR, GbWorkspaceEditorClass))

typedef struct _GbWorkspaceEditor        GbWorkspaceEditor;
typedef struct _GbWorkspaceEditorClass   GbWorkspaceEditorClass;
typedef struct _GbWorkspaceEditorPrivate GbWorkspaceEditorPrivate;

struct _GbWorkspaceEditor
{
   GbWorkspaceSection parent;

   /*< private >*/
   GbWorkspaceEditorPrivate *priv;
};

struct _GbWorkspaceEditorClass
{
   GbWorkspaceSectionClass parent_class;
};

GType gb_workspace_editor_get_type             (void) G_GNUC_CONST;
void  gb_workspace_editor_set_current_document (GbWorkspaceEditor *editor,
                                                GbDocument        *document);

G_END_DECLS

#endif /* GB_WORKSPACE_EDITOR_H */

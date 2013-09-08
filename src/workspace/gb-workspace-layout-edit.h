/* gb-workspace-layout-edit.h
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

#ifndef GB_WORKSPACE_LAYOUT_EDIT_H
#define GB_WORKSPACE_LAYOUT_EDIT_H

#include "gb-workspace-layout.h"

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_LAYOUT_EDIT            (gb_workspace_layout_edit_get_type())
#define GB_WORKSPACE_LAYOUT_EDIT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_LAYOUT_EDIT, GbWorkspaceLayoutEdit))
#define GB_WORKSPACE_LAYOUT_EDIT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_LAYOUT_EDIT, GbWorkspaceLayoutEdit const))
#define GB_WORKSPACE_LAYOUT_EDIT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_LAYOUT_EDIT, GbWorkspaceLayoutEditClass))
#define GB_IS_WORKSPACE_LAYOUT_EDIT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_LAYOUT_EDIT))
#define GB_IS_WORKSPACE_LAYOUT_EDIT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_LAYOUT_EDIT))
#define GB_WORKSPACE_LAYOUT_EDIT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_LAYOUT_EDIT, GbWorkspaceLayoutEditClass))

typedef struct _GbWorkspaceLayoutEdit        GbWorkspaceLayoutEdit;
typedef struct _GbWorkspaceLayoutEditClass   GbWorkspaceLayoutEditClass;
typedef struct _GbWorkspaceLayoutEditPrivate GbWorkspaceLayoutEditPrivate;

struct _GbWorkspaceLayoutEdit
{
   GbWorkspaceLayout parent;

   /*< private >*/
   GbWorkspaceLayoutEditPrivate *priv;
};

struct _GbWorkspaceLayoutEditClass
{
   GbWorkspaceLayoutClass parent_class;
};

GType gb_workspace_layout_edit_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_WORKSPACE_LAYOUT_EDIT_H */

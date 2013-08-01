/* gb-workspace-pane-group.h
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

#ifndef GB_WORKSPACE_PANE_GROUP_H
#define GB_WORKSPACE_PANE_GROUP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_PANE_GROUP            (gb_workspace_pane_group_get_type())
#define GB_WORKSPACE_PANE_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_PANE_GROUP, GbWorkspacePaneGroup))
#define GB_WORKSPACE_PANE_GROUP_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_PANE_GROUP, GbWorkspacePaneGroup const))
#define GB_WORKSPACE_PANE_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_PANE_GROUP, GbWorkspacePaneGroupClass))
#define GB_IS_WORKSPACE_PANE_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_PANE_GROUP))
#define GB_IS_WORKSPACE_PANE_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_PANE_GROUP))
#define GB_WORKSPACE_PANE_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_PANE_GROUP, GbWorkspacePaneGroupClass))

typedef struct _GbWorkspacePaneGroup        GbWorkspacePaneGroup;
typedef struct _GbWorkspacePaneGroupClass   GbWorkspacePaneGroupClass;
typedef struct _GbWorkspacePaneGroupPrivate GbWorkspacePaneGroupPrivate;

struct _GbWorkspacePaneGroup
{
   GtkGrid parent;

   /*< private >*/
   GbWorkspacePaneGroupPrivate *priv;
};

struct _GbWorkspacePaneGroupClass
{
   GtkGridClass parent_class;
};

GType gb_workspace_pane_group_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_WORKSPACE_PANE_GROUP_H */

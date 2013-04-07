/* gb-workspace-pane.h
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GB_WORKSPACE_PANE_H
#define GB_WORKSPACE_PANE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_PANE            (gb_workspace_pane_get_type())
#define GB_WORKSPACE_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_PANE, GbWorkspacePane))
#define GB_WORKSPACE_PANE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_PANE, GbWorkspacePane const))
#define GB_WORKSPACE_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_PANE, GbWorkspacePaneClass))
#define GB_IS_WORKSPACE_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_PANE))
#define GB_IS_WORKSPACE_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_PANE))
#define GB_WORKSPACE_PANE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_PANE, GbWorkspacePaneClass))

typedef struct _GbWorkspacePane        GbWorkspacePane;
typedef struct _GbWorkspacePaneClass   GbWorkspacePaneClass;
typedef struct _GbWorkspacePanePrivate GbWorkspacePanePrivate;

struct _GbWorkspacePane
{
   GtkGrid parent;

   /*< private >*/
   GbWorkspacePanePrivate *priv;
};

struct _GbWorkspacePaneClass
{
   GtkGridClass parent_class;
};

GType        gb_workspace_pane_get_type      (void) G_GNUC_CONST;
const gchar *gb_workspace_pane_get_title     (GbWorkspacePane *pane);
const gchar *gb_workspace_pane_get_icon_name (GbWorkspacePane *pane);
void         gb_workspace_pane_set_title     (GbWorkspacePane *pane,
                                              const gchar     *title);
gboolean     gb_workspace_pane_get_modified  (GbWorkspacePane *pane);
void         gb_workspace_pane_set_modified  (GbWorkspacePane *pane,
                                              gboolean         modified);

G_END_DECLS

#endif /* GB_WORKSPACE_PANE_H */

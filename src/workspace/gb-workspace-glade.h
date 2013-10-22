/* gb-workspace-glade.h
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

#ifndef GB_WORKSPACE_GLADE_H
#define GB_WORKSPACE_GLADE_H

#include "gb-workspace-section.h"

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_GLADE            (gb_workspace_glade_get_type())
#define GB_WORKSPACE_GLADE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_GLADE, GbWorkspaceGlade))
#define GB_WORKSPACE_GLADE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_GLADE, GbWorkspaceGlade const))
#define GB_WORKSPACE_GLADE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_GLADE, GbWorkspaceGladeClass))
#define GB_IS_WORKSPACE_GLADE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_GLADE))
#define GB_IS_WORKSPACE_GLADE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_GLADE))
#define GB_WORKSPACE_GLADE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_GLADE, GbWorkspaceGladeClass))

typedef struct _GbWorkspaceGlade        GbWorkspaceGlade;
typedef struct _GbWorkspaceGladeClass   GbWorkspaceGladeClass;
typedef struct _GbWorkspaceGladePrivate GbWorkspaceGladePrivate;

struct _GbWorkspaceGlade
{
   GbWorkspaceSection parent;

   /*< private >*/
   GbWorkspaceGladePrivate *priv;
};

struct _GbWorkspaceGladeClass
{
   GbWorkspaceSectionClass parent_class;
};

GType gb_workspace_glade_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_WORKSPACE_GLADE_H */

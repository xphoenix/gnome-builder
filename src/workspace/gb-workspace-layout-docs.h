/* gb-workspace-layout-docs.h
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

#ifndef GB_WORKSPACE_LAYOUT_DOCS_H
#define GB_WORKSPACE_LAYOUT_DOCS_H

#include "gb-workspace-layout.h"

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_LAYOUT_DOCS            (gb_workspace_layout_docs_get_type())
#define GB_WORKSPACE_LAYOUT_DOCS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_LAYOUT_DOCS, GbWorkspaceLayoutDocs))
#define GB_WORKSPACE_LAYOUT_DOCS_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_LAYOUT_DOCS, GbWorkspaceLayoutDocs const))
#define GB_WORKSPACE_LAYOUT_DOCS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_LAYOUT_DOCS, GbWorkspaceLayoutDocsClass))
#define GB_IS_WORKSPACE_LAYOUT_DOCS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_LAYOUT_DOCS))
#define GB_IS_WORKSPACE_LAYOUT_DOCS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_LAYOUT_DOCS))
#define GB_WORKSPACE_LAYOUT_DOCS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_LAYOUT_DOCS, GbWorkspaceLayoutDocsClass))

typedef struct _GbWorkspaceLayoutDocs        GbWorkspaceLayoutDocs;
typedef struct _GbWorkspaceLayoutDocsClass   GbWorkspaceLayoutDocsClass;
typedef struct _GbWorkspaceLayoutDocsPrivate GbWorkspaceLayoutDocsPrivate;

struct _GbWorkspaceLayoutDocs
{
   GbWorkspaceLayout parent;

   /*< private >*/
   GbWorkspaceLayoutDocsPrivate *priv;
};

struct _GbWorkspaceLayoutDocsClass
{
   GbWorkspaceLayoutClass parent_class;
};

GType gb_workspace_layout_docs_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_WORKSPACE_LAYOUT_DOCS_H */

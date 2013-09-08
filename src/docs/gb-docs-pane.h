/* gb-docs-pane.h
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

#ifndef GB_DOCS_PANE_H
#define GB_DOCS_PANE_H

#include "gb-workspace-pane.h"

G_BEGIN_DECLS

#define GB_TYPE_DOCS_PANE            (gb_docs_pane_get_type())
#define GB_DOCS_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOCS_PANE, GbDocsPane))
#define GB_DOCS_PANE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOCS_PANE, GbDocsPane const))
#define GB_DOCS_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_DOCS_PANE, GbDocsPaneClass))
#define GB_IS_DOCS_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_DOCS_PANE))
#define GB_IS_DOCS_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_DOCS_PANE))
#define GB_DOCS_PANE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_DOCS_PANE, GbDocsPaneClass))

typedef struct _GbDocsPane        GbDocsPane;
typedef struct _GbDocsPaneClass   GbDocsPaneClass;
typedef struct _GbDocsPanePrivate GbDocsPanePrivate;

struct _GbDocsPane
{
   GbWorkspacePane parent;

   /*< private >*/
   GbDocsPanePrivate *priv;
};

struct _GbDocsPaneClass
{
   GbWorkspacePaneClass parent_class;
};

GType gb_docs_pane_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_DOCS_PANE_H */

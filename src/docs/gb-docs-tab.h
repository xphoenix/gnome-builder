/* gb-docs-tab.h
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

#ifndef GB_DOCS_TAB_H
#define GB_DOCS_TAB_H

#include <devhelp/devhelp.h>

#include "gb-tab.h"

G_BEGIN_DECLS

#define GB_TYPE_DOCS_TAB            (gb_docs_tab_get_type())
#define GB_DOCS_TAB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOCS_TAB, GbDocsTab))
#define GB_DOCS_TAB_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOCS_TAB, GbDocsTab const))
#define GB_DOCS_TAB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_DOCS_TAB, GbDocsTabClass))
#define GB_IS_DOCS_TAB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_DOCS_TAB))
#define GB_IS_DOCS_TAB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_DOCS_TAB))
#define GB_DOCS_TAB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_DOCS_TAB, GbDocsTabClass))

typedef struct _GbDocsTab        GbDocsTab;
typedef struct _GbDocsTabClass   GbDocsTabClass;
typedef struct _GbDocsTabPrivate GbDocsTabPrivate;

struct _GbDocsTab
{
   GbTab parent;

   /*< private >*/
   GbDocsTabPrivate *priv;
};

struct _GbDocsTabClass
{
   GbTabClass parent_class;
};

GType      gb_docs_tab_get_type (void) G_GNUC_CONST;
GtkWidget *gb_docs_tab_new      (void);
void       gb_docs_tab_set_link (GbDocsTab *tab,
                                 DhLink    *link_);

G_END_DECLS

#endif /* GB_DOCS_TAB_H */

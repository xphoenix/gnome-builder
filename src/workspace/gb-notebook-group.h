/* gb-notebook-group.h
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

#ifndef GB_NOTEBOOK_GROUP_H
#define GB_NOTEBOOK_GROUP_H

#include <gtk/gtk.h>

#include "gb-notebook.h"
#include "gb-tab.h"
#include "gb-tab-label.h"

G_BEGIN_DECLS

#define GB_TYPE_NOTEBOOK_GROUP            (gb_notebook_group_get_type())
#define GB_NOTEBOOK_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_NOTEBOOK_GROUP, GbNotebookGroup))
#define GB_NOTEBOOK_GROUP_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_NOTEBOOK_GROUP, GbNotebookGroup const))
#define GB_NOTEBOOK_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_NOTEBOOK_GROUP, GbNotebookGroupClass))
#define GB_IS_NOTEBOOK_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_NOTEBOOK_GROUP))
#define GB_IS_NOTEBOOK_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_NOTEBOOK_GROUP))
#define GB_NOTEBOOK_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_NOTEBOOK_GROUP, GbNotebookGroupClass))

typedef struct _GbNotebookGroup        GbNotebookGroup;
typedef struct _GbNotebookGroupClass   GbNotebookGroupClass;
typedef struct _GbNotebookGroupPrivate GbNotebookGroupPrivate;

struct _GbNotebookGroup
{
   GtkBin parent;

   /*< private >*/
   GbNotebookGroupPrivate *priv;
};

struct _GbNotebookGroupClass
{
   GtkBinClass parent_class;
};

GType      gb_notebook_group_get_type (void) G_GNUC_CONST;
GtkWidget *gb_notebook_group_new      (void);
void       gb_notebook_group_add_tab  (GbNotebookGroup *group,
                                       GbTab           *tab,
                                       GbTabLabel      *tab_label,
                                       GbNotebook      *notebook,
                                       gboolean         new_group);

G_END_DECLS

#endif /* GB_NOTEBOOK_GROUP_H */

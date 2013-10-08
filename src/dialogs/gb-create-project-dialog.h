/* gb-create-project-dialog.h
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

#ifndef GB_CREATE_PROJECT_DIALOG_H
#define GB_CREATE_PROJECT_DIALOG_H

#include <gtk/gtk.h>

#include "gb-project.h"

G_BEGIN_DECLS

#define GB_TYPE_CREATE_PROJECT_DIALOG            (gb_create_project_dialog_get_type())
#define GB_CREATE_PROJECT_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_CREATE_PROJECT_DIALOG, GbCreateProjectDialog))
#define GB_CREATE_PROJECT_DIALOG_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_CREATE_PROJECT_DIALOG, GbCreateProjectDialog const))
#define GB_CREATE_PROJECT_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_CREATE_PROJECT_DIALOG, GbCreateProjectDialogClass))
#define GB_IS_CREATE_PROJECT_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_CREATE_PROJECT_DIALOG))
#define GB_IS_CREATE_PROJECT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_CREATE_PROJECT_DIALOG))
#define GB_CREATE_PROJECT_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_CREATE_PROJECT_DIALOG, GbCreateProjectDialogClass))

typedef struct _GbCreateProjectDialog        GbCreateProjectDialog;
typedef struct _GbCreateProjectDialogClass   GbCreateProjectDialogClass;
typedef struct _GbCreateProjectDialogPrivate GbCreateProjectDialogPrivate;

struct _GbCreateProjectDialog
{
   GtkDialog parent;

   /*< private >*/
   GbCreateProjectDialogPrivate *priv;
};

struct _GbCreateProjectDialogClass
{
   GtkDialogClass parent_class;
};

GType      gb_create_project_dialog_get_type    (void) G_GNUC_CONST;
GtkWidget *gb_create_project_dialog_new         (void);
GbProject *gb_create_project_dialog_get_project (GbCreateProjectDialog *dialog);

G_END_DECLS

#endif /* GB_CREATE_PROJECT_DIALOG_H */

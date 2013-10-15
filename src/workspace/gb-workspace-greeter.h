/* gb-workspace-greeter.h
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

#ifndef GB_WORKSPACE_GREETER_H
#define GB_WORKSPACE_GREETER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_TYPE_WORKSPACE_GREETER            (gb_workspace_greeter_get_type())
#define GB_WORKSPACE_GREETER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_GREETER, GbWorkspaceGreeter))
#define GB_WORKSPACE_GREETER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_WORKSPACE_GREETER, GbWorkspaceGreeter const))
#define GB_WORKSPACE_GREETER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_WORKSPACE_GREETER, GbWorkspaceGreeterClass))
#define GB_IS_WORKSPACE_GREETER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_WORKSPACE_GREETER))
#define GB_IS_WORKSPACE_GREETER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_WORKSPACE_GREETER))
#define GB_WORKSPACE_GREETER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_WORKSPACE_GREETER, GbWorkspaceGreeterClass))

typedef struct _GbWorkspaceGreeter        GbWorkspaceGreeter;
typedef struct _GbWorkspaceGreeterClass   GbWorkspaceGreeterClass;
typedef struct _GbWorkspaceGreeterPrivate GbWorkspaceGreeterPrivate;

struct _GbWorkspaceGreeter
{
   GtkGrid parent;

   /*< private >*/
   GbWorkspaceGreeterPrivate *priv;
};

struct _GbWorkspaceGreeterClass
{
   GtkGridClass parent_class;
};

GType      gb_workspace_greeter_get_type (void) G_GNUC_CONST;
GtkWidget *gb_workspace_greeter_new      (void);

G_END_DECLS

#endif /* GB_WORKSPACE_GREETER_H */

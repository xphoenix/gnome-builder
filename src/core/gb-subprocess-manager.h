/* gb-subprocess-manager.h
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

#ifndef GB_SUBPROCESS_MANAGER_H
#define GB_SUBPROCESS_MANAGER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_SUBPROCESS_MANAGER            (gb_subprocess_manager_get_type())
#define GB_SUBPROCESS_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SUBPROCESS_MANAGER, GbSubprocessManager))
#define GB_SUBPROCESS_MANAGER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SUBPROCESS_MANAGER, GbSubprocessManager const))
#define GB_SUBPROCESS_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SUBPROCESS_MANAGER, GbSubprocessManagerClass))
#define GB_IS_SUBPROCESS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SUBPROCESS_MANAGER))
#define GB_IS_SUBPROCESS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SUBPROCESS_MANAGER))
#define GB_SUBPROCESS_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SUBPROCESS_MANAGER, GbSubprocessManagerClass))

typedef struct _GbSubprocessManager        GbSubprocessManager;
typedef struct _GbSubprocessManagerClass   GbSubprocessManagerClass;
typedef struct _GbSubprocessManagerPrivate GbSubprocessManagerPrivate;

struct _GbSubprocessManager
{
   GObject parent;

   /*< private >*/
   GbSubprocessManagerPrivate *priv;
};

struct _GbSubprocessManagerClass
{
   GObjectClass parent_class;
};

GType                gb_subprocess_manager_get_type (void) G_GNUC_CONST;
GbSubprocessManager *gb_subprocess_manager_new      (void);
void                 gb_subprocess_manager_start    (GbSubprocessManager  *manager);
void                 gb_subprocess_manager_stop     (GbSubprocessManager  *manager);
void                 gb_subprocess_manager_add      (GbSubprocessManager  *manager,
                                                     gchar               **argv,
                                                     gboolean              respawn);

G_END_DECLS

#endif /* GB_SUBPROCESS_MANAGER_H */

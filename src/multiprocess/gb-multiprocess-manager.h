/* gb-multiprocess-manager.h
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

#ifndef GB_MULTIPROCESS_MANAGER_H
#define GB_MULTIPROCESS_MANAGER_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define GB_TYPE_MULTIPROCESS_MANAGER            (gb_multiprocess_manager_get_type())
#define GB_MULTIPROCESS_MANAGER_DEFAULT         (gb_multiprocess_manager_get_default())
#define GB_MULTIPROCESS_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_MULTIPROCESS_MANAGER, GbMultiprocessManager))
#define GB_MULTIPROCESS_MANAGER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_MULTIPROCESS_MANAGER, GbMultiprocessManager const))
#define GB_MULTIPROCESS_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_MULTIPROCESS_MANAGER, GbMultiprocessManagerClass))
#define GB_IS_MULTIPROCESS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_MULTIPROCESS_MANAGER))
#define GB_IS_MULTIPROCESS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_MULTIPROCESS_MANAGER))
#define GB_MULTIPROCESS_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_MULTIPROCESS_MANAGER, GbMultiprocessManagerClass))

typedef struct _GbMultiprocessManager        GbMultiprocessManager;
typedef struct _GbMultiprocessManagerClass   GbMultiprocessManagerClass;
typedef struct _GbMultiprocessManagerPrivate GbMultiprocessManagerPrivate;

struct _GbMultiprocessManager
{
   GObject parent;

   /*< private >*/
   GbMultiprocessManagerPrivate *priv;
};

struct _GbMultiprocessManagerClass
{
   GObjectClass parent_class;
};

GType                  gb_multiprocess_manager_get_type       (void) G_GNUC_CONST;
GbMultiprocessManager *gb_multiprocess_manager_new            (void);
GbMultiprocessManager *gb_multiprocess_manager_get_default    (void);
void                   gb_multiprocess_manager_register       (GbMultiprocessManager  *manager,
                                                               const gchar            *name,
                                                               gchar                 **argv);
GDBusConnection       *gb_multiprocess_manager_get_connection (GbMultiprocessManager  *manager,
                                                               const gchar            *name,
                                                               GError                **error);
void                   gb_multiprocess_manager_shutdown       (GbMultiprocessManager  *manager);

G_END_DECLS

#endif /* GB_MULTIPROCESS_MANAGER_H */

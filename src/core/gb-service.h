/* gb-service.h
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

#ifndef GB_SERVICE_H
#define GB_SERVICE_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define GB_TYPE_SERVICE            (gb_service_get_type())
#define GB_TYPE_SERVICE_MODE       (gb_service_mode_get_type())
#define GB_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SERVICE, GbService))
#define GB_SERVICE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SERVICE, GbService const))
#define GB_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SERVICE, GbServiceClass))
#define GB_IS_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SERVICE))
#define GB_IS_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SERVICE))
#define GB_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SERVICE, GbServiceClass))

typedef struct _GbService        GbService;
typedef struct _GbServiceClass   GbServiceClass;
typedef struct _GbServicePrivate GbServicePrivate;
typedef enum   _GbServiceMode    GbServiceMode;

enum _GbServiceMode
{
   GB_SERVICE_LOCAL,
   GB_SERVICE_PROXY,
};

struct _GbService
{
   GInitiallyUnowned parent;

   /*< private >*/
   GbServicePrivate *priv;
};

struct _GbServiceClass
{
   GInitiallyUnownedClass parent_class;

   void (*start) (GbService *service);
   void (*stop)  (GbService *service);
};

GDBusConnection *gb_service_get_connection (GbService *service);
GbServiceMode    gb_service_get_mode       (GbService *service);
const gchar     *gb_service_get_name       (GbService *service);
GType            gb_service_get_type       (void) G_GNUC_CONST;
GType            gb_service_mode_get_type  (void) G_GNUC_CONST;
void             gb_service_start          (GbService *service);
void             gb_service_stop           (GbService *service);

G_END_DECLS

#endif /* GB_SERVICE_H */

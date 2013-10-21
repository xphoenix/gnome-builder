/* gb-project-service.h
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

#ifndef GB_PROJECT_SERVICE_H
#define GB_PROJECT_SERVICE_H

#include "gb-service.h"

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_SERVICE            (gb_project_service_get_type())
#define GB_PROJECT_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_SERVICE, GbProjectService))
#define GB_PROJECT_SERVICE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_SERVICE, GbProjectService const))
#define GB_PROJECT_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_SERVICE, GbProjectServiceClass))
#define GB_IS_PROJECT_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_SERVICE))
#define GB_IS_PROJECT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_SERVICE))
#define GB_PROJECT_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_SERVICE, GbProjectServiceClass))

typedef struct _GbProjectService        GbProjectService;
typedef struct _GbProjectServiceClass   GbProjectServiceClass;
typedef struct _GbProjectServicePrivate GbProjectServicePrivate;

struct _GbProjectService
{
   GbService parent;

   /*< private >*/
   GbProjectServicePrivate *priv;
};

struct _GbProjectServiceClass
{
   GbServiceClass parent_class;
};

GType  gb_project_service_get_type            (void) G_GNUC_CONST;
GList *gb_project_service_get_recent_projects (GbProjectService *service);

G_END_DECLS

#endif /* GB_PROJECT_SERVICE_H */

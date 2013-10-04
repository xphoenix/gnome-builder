/* gb-project.h
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
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

#ifndef GB_PROJECT_H
#define GB_PROJECT_H

#include <gio/gio.h>

#include "gb-project-group.h"
#include "gb-project-item.h"

G_BEGIN_DECLS

#define GB_TYPE_PROJECT            (gb_project_get_type())
#define GB_PROJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT, GbProject))
#define GB_PROJECT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT, GbProject const))
#define GB_PROJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT, GbProjectClass))
#define GB_IS_PROJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT))
#define GB_IS_PROJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT))
#define GB_PROJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT, GbProjectClass))
#define GB_PROJECT_ERROR           (gb_project_error_quark())

typedef struct _GbProject        GbProject;
typedef struct _GbProjectClass   GbProjectClass;
typedef struct _GbProjectPrivate GbProjectPrivate;
typedef enum   _GbProjectError   GbProjectError;

enum _GbProjectError
{
   GB_PROJECT_ERROR_INVALID_JSON = 1,
};

struct _GbProject
{
   GbProjectItem parent;

   /*< private >*/
   GbProjectPrivate *priv;
};

struct _GbProjectClass
{
   GbProjectItemClass parent_class;
};

GQuark            gb_project_error_quark   (void) G_GNUC_CONST;
const gchar      *gb_project_get_directory (GbProject            *project);
GbProjectGroup   *gb_project_get_files     (GbProject            *project);
const gchar      *gb_project_get_name      (GbProject            *project);
GbProjectGroup   *gb_project_get_targets   (GbProject            *project);
const gchar      *gb_project_get_version   (GbProject            *project);
GType             gb_project_get_type      (void) G_GNUC_CONST;
void              gb_project_set_name      (GbProject            *project,
                                            const gchar          *name);
void              gb_project_set_version   (GbProject            *project,
                                            const gchar          *version);

G_END_DECLS

#endif /* GB_PROJECT_H */

/* gb-project-target.h
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

#ifndef GB_PROJECT_TARGET_H
#define GB_PROJECT_TARGET_H

#include "gb-project-item.h"
#include "gb-project-group.h"

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_TARGET            (gb_project_target_get_type())
#define GB_PROJECT_TARGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_TARGET, GbProjectTarget))
#define GB_PROJECT_TARGET_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_TARGET, GbProjectTarget const))
#define GB_PROJECT_TARGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_TARGET, GbProjectTargetClass))
#define GB_IS_PROJECT_TARGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_TARGET))
#define GB_IS_PROJECT_TARGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_TARGET))
#define GB_PROJECT_TARGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_TARGET, GbProjectTargetClass))

typedef struct _GbProjectTarget        GbProjectTarget;
typedef struct _GbProjectTargetClass   GbProjectTargetClass;
typedef struct _GbProjectTargetPrivate GbProjectTargetPrivate;

struct _GbProjectTarget
{
   GbProjectItem parent;

   /*< private >*/
   GbProjectTargetPrivate *priv;
};

struct _GbProjectTargetClass
{
   GbProjectItemClass parent_class;
};

const gchar    *gb_project_target_get_directory (GbProjectTarget *target);
GbProjectGroup *gb_project_target_get_files     (GbProjectTarget *target);
const gchar    *gb_project_target_get_name      (GbProjectTarget *target);
GType           gb_project_target_get_type      (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_PROJECT_TARGET_H */

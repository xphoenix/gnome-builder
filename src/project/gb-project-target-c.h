/* gb-project-target-c.h
 *
 * Copyright (C) 2012 Christian Hergert <chris@dronelabs.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GB_PROJECT_TARGET_C_H
#define GB_PROJECT_TARGET_C_H

#include "gb-project-target.h"

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_TARGET_C            (gb_project_target_c_get_type())
#define GB_PROJECT_TARGET_C(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_TARGET_C, GbProjectTargetC))
#define GB_PROJECT_TARGET_C_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_TARGET_C, GbProjectTargetC const))
#define GB_PROJECT_TARGET_C_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_TARGET_C, GbProjectTargetCClass))
#define GB_IS_PROJECT_TARGET_C(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_TARGET_C))
#define GB_IS_PROJECT_TARGET_C_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_TARGET_C))
#define GB_PROJECT_TARGET_C_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_TARGET_C, GbProjectTargetCClass))

typedef struct _GbProjectTargetC        GbProjectTargetC;
typedef struct _GbProjectTargetCClass   GbProjectTargetCClass;
typedef struct _GbProjectTargetCPrivate GbProjectTargetCPrivate;

struct _GbProjectTargetC
{
   GbProjectTarget parent;

   /*< private >*/
   GbProjectTargetCPrivate *priv;
};

struct _GbProjectTargetCClass
{
   GbProjectTargetClass parent_class;
};

GType gb_project_target_c_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_PROJECT_TARGET_C_H */

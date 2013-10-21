/* gb-project-info.h
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

#ifndef GB_PROJECT_INFO_H
#define GB_PROJECT_INFO_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_INFO            (gb_project_info_get_type())
#define GB_PROJECT_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_INFO, GbProjectInfo))
#define GB_PROJECT_INFO_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_INFO, GbProjectInfo const))
#define GB_PROJECT_INFO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_INFO, GbProjectInfoClass))
#define GB_IS_PROJECT_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_INFO))
#define GB_IS_PROJECT_INFO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_INFO))
#define GB_PROJECT_INFO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_INFO, GbProjectInfoClass))

typedef struct _GbProjectInfo        GbProjectInfo;
typedef struct _GbProjectInfoClass   GbProjectInfoClass;
typedef struct _GbProjectInfoPrivate GbProjectInfoPrivate;

struct _GbProjectInfo
{
   GObject parent;

   /*< private >*/
   GbProjectInfoPrivate *priv;
};

struct _GbProjectInfoClass
{
   GObjectClass parent_class;
};

GType        gb_project_info_get_type             (void) G_GNUC_CONST;
GFile       *gb_project_info_get_file             (GbProjectInfo *info);
GDateTime   *gb_project_info_get_last_modified_at (GbProjectInfo *info);
const gchar *gb_project_info_get_name             (GbProjectInfo *info);

G_END_DECLS

#endif /* GB_PROJECT_INFO_H */

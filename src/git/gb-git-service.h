/* gb-git-service.h
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

#ifndef GB_GIT_SERVICE_H
#define GB_GIT_SERVICE_H

#include "gb-service.h"

G_BEGIN_DECLS

#define GB_TYPE_GIT_SERVICE            (gb_git_service_get_type())
#define GB_GIT_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_GIT_SERVICE, GbGitService))
#define GB_GIT_SERVICE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_GIT_SERVICE, GbGitService const))
#define GB_GIT_SERVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_GIT_SERVICE, GbGitServiceClass))
#define GB_IS_GIT_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_GIT_SERVICE))
#define GB_IS_GIT_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_GIT_SERVICE))
#define GB_GIT_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_GIT_SERVICE, GbGitServiceClass))

typedef struct _GbGitService        GbGitService;
typedef struct _GbGitServiceClass   GbGitServiceClass;
typedef struct _GbGitServicePrivate GbGitServicePrivate;

struct _GbGitService
{
   GbService parent;

   /*< private >*/
   GbGitServicePrivate *priv;
};

struct _GbGitServiceClass
{
   GbServiceClass parent_class;
};

GType gb_git_service_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_GIT_SERVICE_H */

/* gb-runner.h
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
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

#ifndef GB_RUNNER_H
#define GB_RUNNER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_RUNNER            (gb_runner_get_type())
#define GB_RUNNER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_RUNNER, GbRunner))
#define GB_RUNNER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_RUNNER, GbRunner const))
#define GB_RUNNER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_RUNNER, GbRunnerClass))
#define GB_IS_RUNNER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_RUNNER))
#define GB_IS_RUNNER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_RUNNER))
#define GB_RUNNER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_RUNNER, GbRunnerClass))

typedef struct _GbRunner        GbRunner;
typedef struct _GbRunnerClass   GbRunnerClass;
typedef struct _GbRunnerPrivate GbRunnerPrivate;

struct _GbRunner
{
   GObject parent;

   /*< private >*/
   GbRunnerPrivate *priv;
};

struct _GbRunnerClass
{
   GObjectClass parent_class;
};

GType gb_runner_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_RUNNER_H */

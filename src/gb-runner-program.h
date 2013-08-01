/* gb-runner-program.h:
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

#ifndef GB_RUNNER_PROGRAM_H
#define GB_RUNNER_PROGRAM_H

#include "gb-runner.h"

G_BEGIN_DECLS

#define GB_TYPE_RUNNER_PROGRAM            (gb_runner_program_get_type())
#define GB_RUNNER_PROGRAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_RUNNER_PROGRAM, GbRunnerProgram))
#define GB_RUNNER_PROGRAM_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_RUNNER_PROGRAM, GbRunnerProgram const))
#define GB_RUNNER_PROGRAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_RUNNER_PROGRAM, GbRunnerProgramClass))
#define GB_IS_RUNNER_PROGRAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_RUNNER_PROGRAM))
#define GB_IS_RUNNER_PROGRAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_RUNNER_PROGRAM))
#define GB_RUNNER_PROGRAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_RUNNER_PROGRAM, GbRunnerProgramClass))

typedef struct _GbRunnerProgram        GbRunnerProgram;
typedef struct _GbRunnerProgramClass   GbRunnerProgramClass;
typedef struct _GbRunnerProgramPrivate GbRunnerProgramPrivate;

struct _GbRunnerProgram
{
   GbRunner parent;

   /*< private >*/
   GbRunnerProgramPrivate *priv;
};

struct _GbRunnerProgramClass
{
   GbRunnerClass parent_class;
};

GType gb_runner_program_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_RUNNER_PROGRAM_H */

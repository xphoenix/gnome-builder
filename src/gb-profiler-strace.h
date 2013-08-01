/* gb-profiler-strace.h:
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

#ifndef GB_PROFILER_STRACE_H
#define GB_PROFILER_STRACE_H

#include "gb-profiler.h"

G_BEGIN_DECLS

#define GB_TYPE_PROFILER_STRACE            (gb_profiler_strace_get_type())
#define GB_PROFILER_STRACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROFILER_STRACE, GbProfilerStrace))
#define GB_PROFILER_STRACE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROFILER_STRACE, GbProfilerStrace const))
#define GB_PROFILER_STRACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROFILER_STRACE, GbProfilerStraceClass))
#define GB_IS_PROFILER_STRACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROFILER_STRACE))
#define GB_IS_PROFILER_STRACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROFILER_STRACE))
#define GB_PROFILER_STRACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROFILER_STRACE, GbProfilerStraceClass))

typedef struct _GbProfilerStrace        GbProfilerStrace;
typedef struct _GbProfilerStraceClass   GbProfilerStraceClass;
typedef struct _GbProfilerStracePrivate GbProfilerStracePrivate;

struct _GbProfilerStrace
{
   GbProfiler parent;

   /*< private >*/
   GbProfilerStracePrivate *priv;
};

struct _GbProfilerStraceClass
{
   GbProfilerClass parent_class;
};

GType gb_profiler_strace_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_PROFILER_STRACE_H */

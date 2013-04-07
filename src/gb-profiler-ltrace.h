/* gb-profiler-ltrace.h
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

#ifndef GB_PROFILER_LTRACE_H
#define GB_PROFILER_LTRACE_H

#include "gb-profiler.h"

G_BEGIN_DECLS

#define GB_TYPE_PROFILER_LTRACE            (gb_profiler_ltrace_get_type())
#define GB_PROFILER_LTRACE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROFILER_LTRACE, GbProfilerLtrace))
#define GB_PROFILER_LTRACE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROFILER_LTRACE, GbProfilerLtrace const))
#define GB_PROFILER_LTRACE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROFILER_LTRACE, GbProfilerLtraceClass))
#define GB_IS_PROFILER_LTRACE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROFILER_LTRACE))
#define GB_IS_PROFILER_LTRACE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROFILER_LTRACE))
#define GB_PROFILER_LTRACE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROFILER_LTRACE, GbProfilerLtraceClass))

typedef struct _GbProfilerLtrace        GbProfilerLtrace;
typedef struct _GbProfilerLtraceClass   GbProfilerLtraceClass;
typedef struct _GbProfilerLtracePrivate GbProfilerLtracePrivate;

struct _GbProfilerLtrace
{
   GbProfiler parent;

   /*< private >*/
   GbProfilerLtracePrivate *priv;
};

struct _GbProfilerLtraceClass
{
   GbProfilerClass parent_class;
};

GType gb_profiler_ltrace_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_PROFILER_LTRACE_H */

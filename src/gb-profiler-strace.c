/* gb-profiler-strace.c:
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

#include <glib/gi18n.h>

#include "gb-profiler-strace.h"

G_DEFINE_TYPE(GbProfilerStrace, gb_profiler_strace, GB_TYPE_PROFILER)

struct _GbProfilerStracePrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_profiler_strace_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_profiler_strace_parent_class)->finalize(object);
}

static void
gb_profiler_strace_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
   //GbProfilerStrace *strace = GB_PROFILER_STRACE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_profiler_strace_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
   //GbProfilerStrace *strace = GB_PROFILER_STRACE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_profiler_strace_class_init (GbProfilerStraceClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_profiler_strace_finalize;
   object_class->get_property = gb_profiler_strace_get_property;
   object_class->set_property = gb_profiler_strace_set_property;
   g_type_class_add_private(object_class, sizeof(GbProfilerStracePrivate));
}

static void
gb_profiler_strace_init (GbProfilerStrace *strace)
{
   strace->priv = G_TYPE_INSTANCE_GET_PRIVATE(strace,
                                              GB_TYPE_PROFILER_STRACE,
                                              GbProfilerStracePrivate);

   g_object_set(strace, "name", "Strace", NULL);
}

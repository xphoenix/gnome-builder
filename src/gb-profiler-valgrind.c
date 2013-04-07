/* gb-profiler-valgrind.c
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

#include <glib/gi18n.h>

#include "gb-profiler-valgrind.h"

G_DEFINE_TYPE(GbProfilerValgrind, gb_profiler_valgrind, GB_TYPE_PROFILER)

struct _GbProfilerValgrindPrivate
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
gb_profiler_valgrind_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_profiler_valgrind_parent_class)->finalize(object);
}

static void
gb_profiler_valgrind_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
   //GbProfilerValgrind *valgrind = GB_PROFILER_VALGRIND(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_profiler_valgrind_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
   //GbProfilerValgrind *valgrind = GB_PROFILER_VALGRIND(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_profiler_valgrind_class_init (GbProfilerValgrindClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_profiler_valgrind_finalize;
   object_class->get_property = gb_profiler_valgrind_get_property;
   object_class->set_property = gb_profiler_valgrind_set_property;
   g_type_class_add_private(object_class, sizeof(GbProfilerValgrindPrivate));
}

static void
gb_profiler_valgrind_init (GbProfilerValgrind *valgrind)
{
   valgrind->priv = G_TYPE_INSTANCE_GET_PRIVATE(valgrind,
                                                GB_TYPE_PROFILER_VALGRIND,
                                                GbProfilerValgrindPrivate);

   g_object_set(valgrind, "name", "Valgrind", NULL);
}

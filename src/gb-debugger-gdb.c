/* gb-debugger-gdb.c
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

#include "gb-debugger-gdb.h"

G_DEFINE_TYPE(GbDebuggerGdb, gb_debugger_gdb, GB_TYPE_DEBUGGER)

struct _GbDebuggerGdbPrivate
{
   gpointer dummy;
};

static void
gb_debugger_gdb_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_debugger_gdb_parent_class)->finalize(object);
}

static void
gb_debugger_gdb_class_init (GbDebuggerGdbClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_debugger_gdb_finalize;
   g_type_class_add_private(object_class, sizeof(GbDebuggerGdbPrivate));
}

static void
gb_debugger_gdb_init (GbDebuggerGdb *gdb)
{
   gdb->priv = G_TYPE_INSTANCE_GET_PRIVATE(gdb,
                                           GB_TYPE_DEBUGGER_GDB,
                                           GbDebuggerGdbPrivate);

   g_object_set(gdb, "name", "gdb", NULL);
}

/* gb-debugger-gdb.h
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

#ifndef GB_DEBUGGER_GDB_H
#define GB_DEBUGGER_GDB_H

#include "gb-debugger.h"

G_BEGIN_DECLS

#define GB_TYPE_DEBUGGER_GDB            (gb_debugger_gdb_get_type())
#define GB_DEBUGGER_GDB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DEBUGGER_GDB, GbDebuggerGdb))
#define GB_DEBUGGER_GDB_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DEBUGGER_GDB, GbDebuggerGdb const))
#define GB_DEBUGGER_GDB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_DEBUGGER_GDB, GbDebuggerGdbClass))
#define GB_IS_DEBUGGER_GDB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_DEBUGGER_GDB))
#define GB_IS_DEBUGGER_GDB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_DEBUGGER_GDB))
#define GB_DEBUGGER_GDB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_DEBUGGER_GDB, GbDebuggerGdbClass))

typedef struct _GbDebuggerGdb        GbDebuggerGdb;
typedef struct _GbDebuggerGdbClass   GbDebuggerGdbClass;
typedef struct _GbDebuggerGdbPrivate GbDebuggerGdbPrivate;

struct _GbDebuggerGdb
{
   GbDebugger parent;

   /*< private >*/
   GbDebuggerGdbPrivate *priv;
};

struct _GbDebuggerGdbClass
{
   GbDebuggerClass parent_class;
};

GType gb_debugger_gdb_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_DEBUGGER_GDB_H */

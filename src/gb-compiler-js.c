/* gb-compiler-js.c
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

#include "gb-compiler-js.h"

G_DEFINE_TYPE(GbCompilerJs, gb_compiler_js, GB_TYPE_COMPILER)

struct _GbCompilerJsPrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

static void
gb_compiler_js_class_init (GbCompilerJsClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   g_type_class_add_private(object_class, sizeof(GbCompilerJsPrivate));
}

static void
gb_compiler_js_init (GbCompilerJs *js)
{
   js->priv = G_TYPE_INSTANCE_GET_PRIVATE(js,
                                          GB_TYPE_COMPILER_JS,
                                          GbCompilerJsPrivate);
}

/* gb-compiler-clang.c
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

#include <clang-c/Index.h>
#include <glib/gi18n.h>

#include "gb-compiler-clang.h"

G_DEFINE_TYPE(GbCompilerClang, gb_compiler_clang, GB_TYPE_COMPILER)

struct _GbCompilerClangPrivate
{
   CXIndex index;
};

enum
{
   PROP_0,
   LAST_PROP
};

static void
gb_compiler_clang_finalize (GObject *object)
{
   GbCompilerClangPrivate *priv;

   priv = GB_COMPILER_CLANG(object)->priv;

   g_clear_pointer(&priv->index, clang_disposeIndex);

   G_OBJECT_CLASS(gb_compiler_clang_parent_class)->finalize(object);
}

static void
gb_compiler_clang_class_init (GbCompilerClangClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_compiler_clang_finalize;
   g_type_class_add_private(object_class, sizeof(GbCompilerClangPrivate));
}

static void
gb_compiler_clang_init (GbCompilerClang *clang)
{
   clang->priv = G_TYPE_INSTANCE_GET_PRIVATE(clang,
                                             GB_TYPE_COMPILER_CLANG,
                                             GbCompilerClangPrivate);

   g_object_set(clang, "name", "clang", NULL);

   clang->priv->index = clang_createIndex(0, 0);
}

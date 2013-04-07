/* gb-compiler-clang.h
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

#ifndef GB_COMPILER_CLANG_H
#define GB_COMPILER_CLANG_H

#include "gb-compiler.h"

G_BEGIN_DECLS

#define GB_TYPE_COMPILER_CLANG            (gb_compiler_clang_get_type())
#define GB_COMPILER_CLANG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMPILER_CLANG, GbCompilerClang))
#define GB_COMPILER_CLANG_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMPILER_CLANG, GbCompilerClang const))
#define GB_COMPILER_CLANG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_COMPILER_CLANG, GbCompilerClangClass))
#define GB_IS_COMPILER_CLANG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_COMPILER_CLANG))
#define GB_IS_COMPILER_CLANG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_COMPILER_CLANG))
#define GB_COMPILER_CLANG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_COMPILER_CLANG, GbCompilerClangClass))

typedef struct _GbCompilerClang        GbCompilerClang;
typedef struct _GbCompilerClangClass   GbCompilerClangClass;
typedef struct _GbCompilerClangPrivate GbCompilerClangPrivate;

struct _GbCompilerClang
{
   GbCompiler parent;

   /*< private >*/
   GbCompilerClangPrivate *priv;
};

struct _GbCompilerClangClass
{
   GbCompilerClass parent_class;
};

GType gb_compiler_clang_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_COMPILER_CLANG_H */

/* gb-compiler.h:
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

#ifndef GB_COMPILER_H
#define GB_COMPILER_H

#include <glib-object.h>

#include "gb-compiler-result.h"

G_BEGIN_DECLS

#define GB_TYPE_COMPILER            (gb_compiler_get_type())
#define GB_COMPILER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMPILER, GbCompiler))
#define GB_COMPILER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMPILER, GbCompiler const))
#define GB_COMPILER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_COMPILER, GbCompilerClass))
#define GB_IS_COMPILER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_COMPILER))
#define GB_IS_COMPILER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_COMPILER))
#define GB_COMPILER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_COMPILER, GbCompilerClass))

typedef struct _GbCompiler        GbCompiler;
typedef struct _GbCompilerClass   GbCompilerClass;
typedef struct _GbCompilerPrivate GbCompilerPrivate;

struct _GbCompiler
{
   GObject parent;

   /*< private >*/
   GbCompilerPrivate *priv;
};

struct _GbCompilerClass
{
   GObjectClass parent_class;
};

GType gb_compiler_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_COMPILER_H */

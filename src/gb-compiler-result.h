/* gb-compiler-result.h:
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

#ifndef GB_COMPILER_RESULT_H
#define GB_COMPILER_RESULT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_COMPILER_RESULT            (gb_compiler_result_get_type())
#define GB_COMPILER_RESULT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMPILER_RESULT, GbCompilerResult))
#define GB_COMPILER_RESULT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMPILER_RESULT, GbCompilerResult const))
#define GB_COMPILER_RESULT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_COMPILER_RESULT, GbCompilerResultClass))
#define GB_IS_COMPILER_RESULT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_COMPILER_RESULT))
#define GB_IS_COMPILER_RESULT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_COMPILER_RESULT))
#define GB_COMPILER_RESULT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_COMPILER_RESULT, GbCompilerResultClass))

typedef struct _GbCompilerResult        GbCompilerResult;
typedef struct _GbCompilerResultClass   GbCompilerResultClass;
typedef struct _GbCompilerResultPrivate GbCompilerResultPrivate;

struct _GbCompilerResult
{
   GObject parent;

   /*< private >*/
   GbCompilerResultPrivate *priv;
};

struct _GbCompilerResultClass
{
   GObjectClass parent_class;
};

GType gb_compiler_result_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_COMPILER_RESULT_H */

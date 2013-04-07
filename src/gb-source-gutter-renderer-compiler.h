/* gb-source-gutter-renderer-compiler.h
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

#ifndef GB_SOURCE_GUTTER_RENDERER_COMPILER_H
#define GB_SOURCE_GUTTER_RENDERER_COMPILER_H

#include <gtksourceview/gtksourcegutterrenderer.h>

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER            (gb_source_gutter_renderer_compiler_get_type())
#define GB_SOURCE_GUTTER_RENDERER_COMPILER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER, GbSourceGutterRendererCompiler))
#define GB_SOURCE_GUTTER_RENDERER_COMPILER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER, GbSourceGutterRendererCompiler const))
#define GB_SOURCE_GUTTER_RENDERER_COMPILER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER, GbSourceGutterRendererCompilerClass))
#define GB_IS_SOURCE_GUTTER_RENDERER_COMPILER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER))
#define GB_IS_SOURCE_GUTTER_RENDERER_COMPILER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER))
#define GB_SOURCE_GUTTER_RENDERER_COMPILER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER, GbSourceGutterRendererCompilerClass))

typedef struct _GbSourceGutterRendererCompiler        GbSourceGutterRendererCompiler;
typedef struct _GbSourceGutterRendererCompilerClass   GbSourceGutterRendererCompilerClass;
typedef struct _GbSourceGutterRendererCompilerPrivate GbSourceGutterRendererCompilerPrivate;

struct _GbSourceGutterRendererCompiler
{
   GtkSourceGutterRenderer parent;

   /*< private >*/
   GbSourceGutterRendererCompilerPrivate *priv;
};

struct _GbSourceGutterRendererCompilerClass
{
   GtkSourceGutterRendererClass parent_class;
};

GType gb_source_gutter_renderer_compiler_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SOURCE_GUTTER_RENDERER_COMPILER_H */

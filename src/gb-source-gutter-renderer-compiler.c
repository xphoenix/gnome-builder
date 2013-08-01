/* gb-source-gutter-renderer-compiler.c:
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

#include "gb-compiler.h"
#include "gb-source-gutter-renderer-compiler.h"

G_DEFINE_TYPE(GbSourceGutterRendererCompiler,
              gb_source_gutter_renderer_compiler,
              GTK_SOURCE_TYPE_GUTTER_RENDERER)

struct _GbSourceGutterRendererCompilerPrivate
{
   GbCompiler *compiler;
};

enum
{
   PROP_0,
   PROP_COMPILER,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbCompiler *
gb_source_gutter_renderer_compiler_get_compiler (GbSourceGutterRendererCompiler *compiler)
{
   g_return_val_if_fail(GB_IS_SOURCE_GUTTER_RENDERER_COMPILER(compiler), NULL);
   return compiler->priv->compiler;
}

void
gb_source_gutter_renderer_compiler_set_compiler (GbSourceGutterRendererCompiler *renderer,
                                                 GbCompiler                     *compiler)
{
   GbSourceGutterRendererCompilerPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_GUTTER_RENDERER_COMPILER(compiler));

   priv = renderer->priv;

   g_clear_object(&priv->compiler);
   priv->compiler = g_object_ref(compiler);
   g_object_notify_by_pspec(G_OBJECT(renderer), gParamSpecs[PROP_COMPILER]);
}

static void
gb_source_gutter_renderer_compiler_finalize (GObject *object)
{
   GbSourceGutterRendererCompilerPrivate *priv;

   priv = GB_SOURCE_GUTTER_RENDERER_COMPILER(object)->priv;

   g_clear_object(&priv->compiler);

   G_OBJECT_CLASS(gb_source_gutter_renderer_compiler_parent_class)->finalize(object);
}

static void
gb_source_gutter_renderer_compiler_get_property (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
   GbSourceGutterRendererCompiler *compiler = GB_SOURCE_GUTTER_RENDERER_COMPILER(object);

   switch (prop_id) {
   case PROP_COMPILER:
      g_value_set_object(
          value,
          gb_source_gutter_renderer_compiler_get_compiler(compiler));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_gutter_renderer_compiler_set_property (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
   GbSourceGutterRendererCompiler *compiler = GB_SOURCE_GUTTER_RENDERER_COMPILER(object);

   switch (prop_id) {
   case PROP_COMPILER:
      gb_source_gutter_renderer_compiler_set_compiler(
         compiler,
         g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_gutter_renderer_compiler_class_init (GbSourceGutterRendererCompilerClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_gutter_renderer_compiler_finalize;
   object_class->get_property = gb_source_gutter_renderer_compiler_get_property;
   object_class->set_property = gb_source_gutter_renderer_compiler_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceGutterRendererCompilerPrivate));

   gParamSpecs[PROP_COMPILER] =
      g_param_spec_object("compiler",
                          _("Compiler"),
                          _("The compiler for the renderer to visualize."),
                          GB_TYPE_COMPILER,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_COMPILER,
                                   gParamSpecs[PROP_COMPILER]);
}

static void
gb_source_gutter_renderer_compiler_init (GbSourceGutterRendererCompiler *compiler)
{
   compiler->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(compiler,
                                  GB_TYPE_SOURCE_GUTTER_RENDERER_COMPILER,
                                  GbSourceGutterRendererCompilerPrivate);
}

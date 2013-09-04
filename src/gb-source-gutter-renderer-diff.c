/* gb-source-gutter-renderer-diff.c
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

#include "gb-source-diff.h"
#include "gb-source-gutter-renderer-diff.h"

G_DEFINE_TYPE(GbSourceGutterRendererDiff,
              gb_source_gutter_renderer_diff,
              GTK_SOURCE_TYPE_GUTTER_RENDERER)

struct _GbSourceGutterRendererDiffPrivate
{
   GbSourceDiff *diff;
   guint         changed_handler;
   GdkRGBA       rgba_added;
   GdkRGBA       rgba_changed;
};

enum
{
   PROP_0,
   PROP_DIFF,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GtkSourceGutterRenderer *
gb_source_gutter_renderer_diff_new (GbSourceDiff *diff)
{
   g_return_val_if_fail(GB_IS_SOURCE_DIFF(diff), NULL);

   return g_object_new(GB_TYPE_SOURCE_GUTTER_RENDERER_DIFF,
                       "diff", diff,
                       NULL);
}

static void
gb_source_gutter_renderer_diff_changed (GbSourceDiff               *diff,
                                        GbSourceGutterRendererDiff *renderer)
{
   g_assert(GB_IS_SOURCE_DIFF(diff));
   g_assert(GB_IS_SOURCE_GUTTER_RENDERER_DIFF(renderer));
   gtk_source_gutter_renderer_queue_draw(GTK_SOURCE_GUTTER_RENDERER(renderer));
}

GbSourceDiff *
gb_source_gutter_renderer_diff_get_diff (GbSourceGutterRendererDiff *diff)
{
   g_return_val_if_fail(GB_IS_SOURCE_GUTTER_RENDERER_DIFF(diff), NULL);
   return diff->priv->diff;
}

void
gb_source_gutter_renderer_diff_set_diff (GbSourceGutterRendererDiff *renderer,
                                         GbSourceDiff               *diff)
{
   GbSourceGutterRendererDiffPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_GUTTER_RENDERER_DIFF(renderer));
   g_return_if_fail(!diff || GB_IS_SOURCE_DIFF(diff));

   priv = renderer->priv;

   if (priv->diff != diff) {
      if (priv->changed_handler) {
         g_signal_handler_disconnect(priv->diff, priv->changed_handler);
         priv->changed_handler = 0;
      }
      g_clear_object(&priv->diff);
      if (diff) {
         priv->diff = g_object_ref(diff);
         priv->changed_handler =
            g_signal_connect(priv->diff, "changed",
                             G_CALLBACK(gb_source_gutter_renderer_diff_changed),
                             renderer);
      }
   }

   g_object_notify_by_pspec(G_OBJECT(renderer), gParamSpecs[PROP_DIFF]);
}

static void
gb_source_gutter_renderer_diff_draw (GtkSourceGutterRenderer      *renderer,
                                     cairo_t                      *cr,
                                     GdkRectangle                 *bg_area,
                                     GdkRectangle                 *cell_area,
                                     GtkTextIter                  *begin,
                                     GtkTextIter                  *end,
                                     GtkSourceGutterRendererState  state)
{
   GbSourceGutterRendererDiffPrivate *priv;
   GbSourceGutterRendererDiff *diff = (GbSourceGutterRendererDiff *)renderer;
   GbSourceDiffState line_state;
   gint line;

   g_return_if_fail(GB_IS_SOURCE_GUTTER_RENDERER_DIFF(diff));

   priv = diff->priv;

   line = gtk_text_iter_get_line(begin);
   line_state = gb_source_diff_get_line_state(priv->diff, line + 1);

   switch (line_state) {
   case GB_SOURCE_DIFF_ADDED:
      gdk_cairo_rectangle(cr, cell_area);
      gdk_cairo_set_source_rgba(cr, &priv->rgba_added);
      cairo_fill(cr);
      break;
   case GB_SOURCE_DIFF_CHANGED:
      gdk_cairo_rectangle(cr, cell_area);
      gdk_cairo_set_source_rgba(cr, &priv->rgba_changed);
      cairo_fill(cr);
      break;
   case GB_SOURCE_DIFF_SAME:
   default:
      break;
   }
}

static void
gb_source_gutter_renderer_diff_dispose (GObject *object)
{
   GbSourceGutterRendererDiffPrivate *priv;

   priv = GB_SOURCE_GUTTER_RENDERER_DIFF(object)->priv;

   if (priv->changed_handler) {
      g_signal_handler_disconnect(priv->diff, priv->changed_handler);
      priv->changed_handler = 0;
   }

   g_clear_object(&priv->diff);

   G_OBJECT_CLASS(gb_source_gutter_renderer_diff_parent_class)->dispose(object);
}

static void
gb_source_gutter_renderer_diff_get_property (GObject    *object,
                                             guint       prop_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
   GbSourceGutterRendererDiff *diff = GB_SOURCE_GUTTER_RENDERER_DIFF(object);

   switch (prop_id) {
   case PROP_DIFF:
      g_value_set_object(value, gb_source_gutter_renderer_diff_get_diff(diff));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_gutter_renderer_diff_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
   GbSourceGutterRendererDiff *diff = GB_SOURCE_GUTTER_RENDERER_DIFF(object);

   switch (prop_id) {
   case PROP_DIFF:
      gb_source_gutter_renderer_diff_set_diff(diff, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_gutter_renderer_diff_class_init (GbSourceGutterRendererDiffClass *klass)
{
   GObjectClass *object_class;
   GtkSourceGutterRendererClass *renderer_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_source_gutter_renderer_diff_dispose;
   object_class->get_property = gb_source_gutter_renderer_diff_get_property;
   object_class->set_property = gb_source_gutter_renderer_diff_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceGutterRendererDiffPrivate));

   renderer_class = GTK_SOURCE_GUTTER_RENDERER_CLASS(klass);
   renderer_class->draw = gb_source_gutter_renderer_diff_draw;

   gParamSpecs[PROP_DIFF] =
      g_param_spec_object("diff",
                          _("Diff"),
                          _("The diff to visualize in the gutter."),
                          GB_TYPE_SOURCE_DIFF,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_DIFF,
                                   gParamSpecs[PROP_DIFF]);
}

static void
gb_source_gutter_renderer_diff_init (GbSourceGutterRendererDiff *diff)
{
   diff->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(diff,
                                  GB_TYPE_SOURCE_GUTTER_RENDERER_DIFF,
                                  GbSourceGutterRendererDiffPrivate);

   /*
    * TODO: Make these colors configurable?
    */
   gdk_rgba_parse(&diff->priv->rgba_added, "#8ae234");
   gdk_rgba_parse(&diff->priv->rgba_changed, "#fcaf3e");
}

/* gb-source-gutter-renderer-diff.c
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

#include "gb-source-diff.h"
#include "gb-source-gutter-renderer-diff.h"

G_DEFINE_TYPE(GbSourceGutterRendererDiff,
              gb_source_gutter_renderer_diff,
              GTK_SOURCE_TYPE_GUTTER_RENDERER)

struct _GbSourceGutterRendererDiffPrivate
{
   GbSourceDiff *diff;
};

enum
{
   PROP_0,
   PROP_DIFF,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

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
   g_return_if_fail(GB_IS_SOURCE_GUTTER_RENDERER_DIFF(diff));

   g_clear_object(&renderer->priv->diff);
   renderer->priv->diff = diff ? g_object_ref(diff) : NULL;
   g_object_notify_by_pspec(G_OBJECT(renderer), gParamSpecs[PROP_DIFF]);
}

static void
gb_source_gutter_renderer_diff_finalize (GObject *object)
{
   GbSourceGutterRendererDiffPrivate *priv;

   priv = GB_SOURCE_GUTTER_RENDERER_DIFF(object)->priv;

   g_clear_object(&priv->diff);

   G_OBJECT_CLASS(gb_source_gutter_renderer_diff_parent_class)->finalize(object);
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

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_gutter_renderer_diff_finalize;
   object_class->get_property = gb_source_gutter_renderer_diff_get_property;
   object_class->set_property = gb_source_gutter_renderer_diff_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceGutterRendererDiffPrivate));

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
}

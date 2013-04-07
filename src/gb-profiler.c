/* gb-profiler.c
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

#include "gb-profiler.h"

G_DEFINE_TYPE(GbProfiler, gb_profiler, G_TYPE_OBJECT)

struct _GbProfilerPrivate
{
   gchar *name;
};

enum
{
   PROP_0,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_profiler_get_name (GbProfiler *profiler)
{
   g_return_val_if_fail(GB_IS_PROFILER(profiler), NULL);
   return profiler->priv->name;
}

void
gb_profiler_set_name (GbProfiler  *profiler,
                      const gchar *name)
{
   g_return_if_fail(GB_IS_PROFILER(profiler));

   g_free(profiler->priv->name);
   profiler->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(profiler), gParamSpecs[PROP_NAME]);
}

static void
gb_profiler_finalize (GObject *object)
{
   GbProfilerPrivate *priv;

   priv = GB_PROFILER(object)->priv;

   g_clear_pointer(&priv->name, g_free);

   G_OBJECT_CLASS(gb_profiler_parent_class)->finalize(object);
}

static void
gb_profiler_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
   GbProfiler *profiler = GB_PROFILER(object);

   switch (prop_id) {
   case PROP_NAME:
      g_value_set_string(value, gb_profiler_get_name(profiler));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_profiler_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
   GbProfiler *profiler = GB_PROFILER(object);

   switch (prop_id) {
   case PROP_NAME:
      gb_profiler_set_name(profiler, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_profiler_class_init (GbProfilerClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_profiler_finalize;
   object_class->get_property = gb_profiler_get_property;
   object_class->set_property = gb_profiler_set_property;
   g_type_class_add_private(object_class, sizeof(GbProfilerPrivate));

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the profiler."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_profiler_init (GbProfiler *profiler)
{
   profiler->priv = G_TYPE_INSTANCE_GET_PRIVATE(profiler,
                                                GB_TYPE_PROFILER,
                                                GbProfilerPrivate);
}

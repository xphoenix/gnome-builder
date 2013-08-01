/* gb-debugger.c:
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

#include "gb-debugger.h"

G_DEFINE_TYPE(GbDebugger, gb_debugger, G_TYPE_OBJECT)

struct _GbDebuggerPrivate
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
gb_debugger_get_name (GbDebugger *debugger)
{
   g_return_val_if_fail(GB_IS_DEBUGGER(debugger), NULL);
   return debugger->priv->name;
}

void
gb_debugger_set_name (GbDebugger  *debugger,
                      const gchar *name)
{
   g_return_if_fail(GB_IS_DEBUGGER(debugger));
   g_free(debugger->priv->name);
   debugger->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(debugger), gParamSpecs[PROP_NAME]);
}

static void
gb_debugger_finalize (GObject *object)
{
   GbDebuggerPrivate *priv;

   priv = GB_DEBUGGER(object)->priv;

   g_clear_pointer(&priv->name, g_free);

   G_OBJECT_CLASS(gb_debugger_parent_class)->finalize(object);
}

static void
gb_debugger_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
   GbDebugger *debugger = GB_DEBUGGER(object);

   switch (prop_id) {
   case PROP_NAME:
      g_value_set_string(value, gb_debugger_get_name(debugger));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_debugger_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
   GbDebugger *debugger = GB_DEBUGGER(object);

   switch (prop_id) {
   case PROP_NAME:
      gb_debugger_set_name(debugger, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_debugger_class_init (GbDebuggerClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_debugger_finalize;
   object_class->get_property = gb_debugger_get_property;
   object_class->set_property = gb_debugger_set_property;
   g_type_class_add_private(object_class, sizeof(GbDebuggerPrivate));

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the debugger."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_debugger_init (GbDebugger *debugger)
{
   debugger->priv = G_TYPE_INSTANCE_GET_PRIVATE(debugger,
                                                GB_TYPE_DEBUGGER,
                                                GbDebuggerPrivate);
}

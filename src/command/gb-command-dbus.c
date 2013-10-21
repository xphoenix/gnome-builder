/* gb-command-dbus.c
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

#include "gb-command-dbus.h"

G_DEFINE_TYPE(GbCommandDbus, gb_command_dbus, GB_TYPE_COMMAND)

struct _GbCommandDbusPrivate
{
   GDBusConnection *connection;
   gchar           *name;
   gchar           *path;
   gchar           *interface;
   gchar           *method;
   GVariant        *params;
   GVariant        *result;
};

enum
{
   PROP_0,
   PROP_CONNECTION,
   PROP_NAME,
   PROP_PATH,
   PROP_INTERFACE,
   PROP_METHOD,
   PROP_PARAMS,
   PROP_RESULT,
   LAST_PROP
};

enum
{
   FINISHED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

GDBusConnection *
gb_command_dbus_get_connection (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);
   return dbus->priv->connection;
}

static void
gb_command_dbus_set_connection (GbCommandDbus   *dbus,
                                GDBusConnection *connection)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));
   g_return_if_fail(!dbus->priv->connection);

   dbus->priv->connection = connection ?
                            g_object_ref(connection) :
                            g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
}

/**
 * gb_command_dbus_get_name:
 * @dbus: (in): A #GbCommandDbus.
 *
 * Gets the "name" property.
 *
 * Returns: A string which should not be modified or freed.
 */
const gchar *
gb_command_dbus_get_name (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);

   return dbus->priv->name;
}

/**
 * gb_command_dbus_set_name:
 * @dbus: (in): A #GbCommandDbus.
 * @name: (in): A string.
 *
 * Sets the "name" property.
 */
void
gb_command_dbus_set_name (GbCommandDbus *dbus,
                                       const gchar *name)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   g_free(dbus->priv->name);
   dbus->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(dbus), gParamSpecs[PROP_NAME]);
}

/**
 * gb_command_dbus_get_path:
 * @dbus: (in): A #GbCommandDbus.
 *
 * Gets the "path" property.
 *
 * Returns: A string which should not be modified or freed.
 */
const gchar *
gb_command_dbus_get_path (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);

   return dbus->priv->path;
}

/**
 * gb_command_dbus_set_path:
 * @dbus: (in): A #GbCommandDbus.
 * @path: (in): A string.
 *
 * Sets the "path" property.
 */
void
gb_command_dbus_set_path (GbCommandDbus *dbus,
                          const gchar   *path)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   g_free(dbus->priv->path);
   dbus->priv->path = g_strdup(path);
   g_object_notify_by_pspec(G_OBJECT(dbus), gParamSpecs[PROP_PATH]);
}

/**
 * gb_command_dbus_get_interface:
 * @dbus: (in): A #GbCommandDbus.
 *
 * Gets the "interface" property.
 *
 * Returns: A string which should not be modified or freed.
 */
const gchar *
gb_command_dbus_get_interface (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);

   return dbus->priv->interface;
}

/**
 * gb_command_dbus_set_interface:
 * @dbus: (in): A #GbCommandDbus.
 * @interface: (in): A string.
 *
 * Sets the "interface" property.
 */
void
gb_command_dbus_set_interface (GbCommandDbus *dbus,
                                       const gchar *interface)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   g_free(dbus->priv->interface);
   dbus->priv->interface = g_strdup(interface);
   g_object_notify_by_pspec(G_OBJECT(dbus), gParamSpecs[PROP_INTERFACE]);
}

/**
 * gb_command_dbus_get_method:
 * @dbus: (in): A #GbCommandDbus.
 *
 * Gets the "method" property.
 *
 * Returns: A string which should not be modified or freed.
 */
const gchar *
gb_command_dbus_get_method (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);

   return dbus->priv->method;
}

/**
 * gb_command_dbus_set_method:
 * @dbus: (in): A #GbCommandDbus.
 * @method: (in): A string.
 *
 * Sets the "method" property.
 */
void
gb_command_dbus_set_method (GbCommandDbus *dbus,
                                       const gchar *method)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   g_free(dbus->priv->method);
   dbus->priv->method = g_strdup(method);
   g_object_notify_by_pspec(G_OBJECT(dbus), gParamSpecs[PROP_METHOD]);
}

GVariant *
gb_command_dbus_get_params (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);

   return dbus->priv->params;
}

static void
gb_command_dbus_set_params (GbCommandDbus *dbus,
                            GVariant      *params)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   g_clear_pointer(&dbus->priv->params, g_variant_unref);
   dbus->priv->params = params ? g_variant_ref(params) : NULL;
   g_object_notify_by_pspec(G_OBJECT(dbus), gParamSpecs[PROP_PARAMS]);
}

GVariant *
gb_command_dbus_get_result (GbCommandDbus *dbus)
{
   g_return_val_if_fail(GB_IS_COMMAND_DBUS(dbus), NULL);

   return dbus->priv->result;
}

static void
gb_command_dbus_set_result (GbCommandDbus *dbus,
                            GVariant      *result)
{
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   g_clear_pointer(&dbus->priv->result, g_variant_unref);
   dbus->priv->result = result ? g_variant_ref(result) : NULL;
   g_object_notify_by_pspec(G_OBJECT(dbus), gParamSpecs[PROP_RESULT]);
}

static void
gb_command_dbus_call_cb (GObject      *object,
                         GAsyncResult *result,
                         gpointer      user_data)
{
   GDBusConnection *connection = (GDBusConnection *)object;
   GbCommandDbus *dbus = user_data;
   GVariant *variant;
   GError *error = NULL;

   g_return_if_fail(G_IS_DBUS_CONNECTION(connection));
   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   variant = g_dbus_connection_call_finish(connection, result, &error);

   if (!variant) {
      gb_command_take_error(GB_COMMAND(dbus), error);
   } else {
      gb_command_dbus_set_result(dbus, variant);
      g_variant_unref(variant);
   }

   g_object_unref(dbus);
}

static void
gb_command_dbus_run (GbCommand *command)
{
   GbCommandDbusPrivate *priv;
   GbCommandDbus *dbus = (GbCommandDbus *)command;

   g_return_if_fail(GB_IS_COMMAND_DBUS(dbus));

   priv = dbus->priv;

   g_dbus_connection_call(priv->connection,
                          priv->name,
                          priv->path,
                          priv->interface,
                          priv->method,
                          priv->params,
                          NULL,
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          gb_command_dbus_call_cb,
                          g_object_ref(command));
}

static void
gb_command_dbus_finalize (GObject *object)
{
   GbCommandDbusPrivate *priv;

   priv = GB_COMMAND_DBUS(object)->priv;

   g_clear_object(&priv->connection);
   g_clear_pointer(&priv->name, g_free);
   g_clear_pointer(&priv->path, g_free);
   g_clear_pointer(&priv->interface, g_free);
   g_clear_pointer(&priv->method, g_free);
   g_clear_pointer(&priv->params, g_variant_unref);
   g_clear_pointer(&priv->result, g_variant_unref);

   G_OBJECT_CLASS(gb_command_dbus_parent_class)->finalize(object);
}

static void
gb_command_dbus_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
   GbCommandDbus *dbus = GB_COMMAND_DBUS(object);

   switch (prop_id) {
   case PROP_CONNECTION:
      g_value_set_object(value, gb_command_dbus_get_connection(dbus));
      break;
   case PROP_NAME:
      g_value_set_string(value, gb_command_dbus_get_name(dbus));
      break;
   case PROP_PATH:
      g_value_set_string(value, gb_command_dbus_get_path(dbus));
      break;
   case PROP_INTERFACE:
      g_value_set_string(value, gb_command_dbus_get_interface(dbus));
      break;
   case PROP_METHOD:
      g_value_set_string(value, gb_command_dbus_get_method(dbus));
      break;
   case PROP_PARAMS:
      g_value_set_variant(value, gb_command_dbus_get_params(dbus));
      break;
   case PROP_RESULT:
      g_value_set_variant(value, gb_command_dbus_get_result(dbus));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_command_dbus_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
   GbCommandDbus *dbus = GB_COMMAND_DBUS(object);

   switch (prop_id) {
   case PROP_CONNECTION:
      gb_command_dbus_set_connection(dbus, g_value_get_object(value));
      break;
   case PROP_NAME:
      gb_command_dbus_set_name(dbus, g_value_get_string(value));
      break;
   case PROP_PATH:
      gb_command_dbus_set_path(dbus, g_value_get_string(value));
      break;
   case PROP_INTERFACE:
      gb_command_dbus_set_interface(dbus, g_value_get_string(value));
      break;
   case PROP_METHOD:
      gb_command_dbus_set_method(dbus, g_value_get_string(value));
      break;
   case PROP_PARAMS:
      gb_command_dbus_set_params(dbus, g_value_get_variant(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_command_dbus_class_init (GbCommandDbusClass *klass)
{
   GObjectClass *object_class;
   GbCommandClass *command_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_command_dbus_finalize;
   object_class->get_property = gb_command_dbus_get_property;
   object_class->set_property = gb_command_dbus_set_property;
   g_type_class_add_private(object_class, sizeof(GbCommandDbusPrivate));

   command_class = GB_COMMAND_CLASS(klass);
   command_class->run = gb_command_dbus_run;

   gParamSpecs[PROP_CONNECTION] =
      g_param_spec_object("connection",
                          _("Connection"),
                          _("The DBus connection to use."),
                          G_TYPE_DBUS_CONNECTION,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CONNECTION,
                                   gParamSpecs[PROP_CONNECTION]);

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The well-known name on DBus."),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);

   gParamSpecs[PROP_PATH] =
      g_param_spec_string("path",
                          _("Path"),
                          _("The DBus path."),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PATH,
                                   gParamSpecs[PROP_PATH]);

   gParamSpecs[PROP_INTERFACE] =
      g_param_spec_string("interface",
                          _("Interface"),
                          _("The DBus interface name."),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_INTERFACE,
                                   gParamSpecs[PROP_INTERFACE]);

   gParamSpecs[PROP_METHOD] =
      g_param_spec_string("method",
                          _("Method"),
                          _("The method name to execute."),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_METHOD,
                                   gParamSpecs[PROP_METHOD]);

   gParamSpecs[PROP_PARAMS] =
      g_param_spec_variant("params",
                           _("Params"),
                           _("The parameters for the method call."),
                           G_VARIANT_TYPE_ANY,
                           NULL,
                           (G_PARAM_READWRITE |
                            G_PARAM_CONSTRUCT_ONLY |
                            G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PARAMS,
                                   gParamSpecs[PROP_PARAMS]);

   gParamSpecs[PROP_RESULT] =
      g_param_spec_variant("result",
                           _("Result"),
                           _("The result of the command, set after calling."),
                           G_VARIANT_TYPE_ANY,
                           NULL,
                           (G_PARAM_READABLE |
                            G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_RESULT,
                                   gParamSpecs[PROP_RESULT]);

   gSignals[FINISHED] = g_signal_new("finished",
                                     GB_TYPE_COMMAND_DBUS,
                                     G_SIGNAL_RUN_LAST,
                                     0,
                                     NULL,
                                     NULL,
                                     g_cclosure_marshal_VOID__VARIANT,
                                     G_TYPE_NONE,
                                     1,
                                     G_TYPE_VARIANT);
}

static void
gb_command_dbus_init (GbCommandDbus *dbus)
{
   dbus->priv = G_TYPE_INSTANCE_GET_PRIVATE(dbus,
                                            GB_TYPE_COMMAND_DBUS,
                                            GbCommandDbusPrivate);
}

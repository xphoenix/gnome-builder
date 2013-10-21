/* gb-service.c
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

#include "gb-service.h"

/**
 * SERVICE:service
 * @title: GbService
 * @short_description: A base class for service implementations.
 *
 * The #GbService class serves as a base class used by service implementations
 * for GNOME Builder sub-services.
 */

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "service"

G_DEFINE_ABSTRACT_TYPE(GbService, gb_service, G_TYPE_INITIALLY_UNOWNED)

struct _GbServicePrivate
{
   GDBusConnection *connection;
   GbServiceMode    mode;
   gchar           *name;
};

enum
{
   PROP_0,
   PROP_CONNECTION,
   PROP_MODE,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GDBusConnection *
gb_service_get_connection (GbService *service)
{
   g_return_val_if_fail(GB_IS_SERVICE(service), NULL);
   return service->priv->connection;
}

static void
gb_service_set_connection (GbService       *service,
                           GDBusConnection *connection)
{
   g_return_if_fail(GB_IS_SERVICE(service));

   g_clear_object(&service->priv->connection);
   service->priv->connection = connection ? g_object_ref(connection) : NULL;
   g_object_notify_by_pspec(G_OBJECT(service), gParamSpecs[PROP_CONNECTION]);
}

GbServiceMode
gb_service_get_mode (GbService *service)
{
   g_return_val_if_fail(GB_IS_SERVICE(service), 0);
   return service->priv->mode;
}

static void
gb_service_set_mode (GbService     *service,
                     GbServiceMode  mode)
{
   g_return_if_fail(GB_IS_SERVICE(service));

   service->priv->mode = mode;
   g_object_notify_by_pspec(G_OBJECT(service), gParamSpecs[PROP_MODE]);
}

const gchar *
gb_service_get_name (GbService *service)
{
   g_return_val_if_fail(GB_IS_SERVICE(service), NULL);
   return service->priv->name;
}

void
gb_service_set_name (GbService   *service,
                     const gchar *name)
{
   g_return_if_fail(GB_IS_SERVICE(service));

   g_free(service->priv->name);
   service->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(service), gParamSpecs[PROP_NAME]);
}

void
gb_service_start (GbService *service)
{
   g_return_if_fail(GB_IS_SERVICE(service));

   if (GB_SERVICE_GET_CLASS(service)->start) {
      GB_SERVICE_GET_CLASS(service)->start(service);
   }
}

void
gb_service_stop (GbService *service)
{
   g_return_if_fail(GB_IS_SERVICE(service));

   if (GB_SERVICE_GET_CLASS(service)->stop) {
      GB_SERVICE_GET_CLASS(service)->stop(service);
   }
}

static void
gb_service_finalize (GObject *object)
{
   GbServicePrivate *priv;

   priv = GB_SERVICE(object)->priv;

   g_clear_object(&priv->connection);
   g_clear_pointer(&priv->name, g_free);

   G_OBJECT_CLASS(gb_service_parent_class)->finalize(object);
}

static void
gb_service_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
   GbService *service = GB_SERVICE(object);

   switch (prop_id) {
   case PROP_CONNECTION:
      g_value_set_object(value, gb_service_get_connection(service));
      break;
   case PROP_MODE:
      g_value_set_enum(value, gb_service_get_mode(service));
      break;
   case PROP_NAME:
      g_value_set_string(value, gb_service_get_name(service));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_service_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
   GbService *service = GB_SERVICE(object);

   switch (prop_id) {
   case PROP_CONNECTION:
      gb_service_set_connection(service, g_value_get_object(value));
      break;
   case PROP_MODE:
      gb_service_set_mode(service, g_value_get_enum(value));
      break;
   case PROP_NAME:
      gb_service_set_name(service, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_service_class_init (GbServiceClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_service_finalize;
   object_class->get_property = gb_service_get_property;
   object_class->set_property = gb_service_set_property;
   g_type_class_add_private(object_class, sizeof(GbServicePrivate));

   gParamSpecs[PROP_CONNECTION] =
      g_param_spec_object("connection",
                          _("Connection"),
                          _("The intra-process DBus connection."),
                          G_TYPE_DBUS_CONNECTION,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CONNECTION,
                                   gParamSpecs[PROP_CONNECTION]);

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("Name"),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);

   gParamSpecs[PROP_MODE] =
      g_param_spec_enum("mode",
                        _("Mode"),
                        _("The service mode (local or remote)."),
                        GB_TYPE_SERVICE_MODE,
                        GB_SERVICE_LOCAL,
                        (G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MODE,
                                   gParamSpecs[PROP_MODE]);
}

static void
gb_service_init (GbService *service)
{
   service->priv = G_TYPE_INSTANCE_GET_PRIVATE(service,
                                               GB_TYPE_SERVICE,
                                               GbServicePrivate);
}

GType
gb_service_mode_get_type (void)
{
   static const GEnumValue values[] = {
      { GB_SERVICE_LOCAL, "GB_SERVICE_LOCAL", "LOCAL" },
      { GB_SERVICE_PROXY, "GB_SERVICE_PROXY", "PROXY" },
      { 0 }
   };
   static GType type_id;
   static gsize initialized;

   if (g_once_init_enter(&initialized)) {
      type_id = g_enum_register_static("GbServiceMode", values);
      g_once_init_leave(&initialized, TRUE);
   }

   return type_id;
}

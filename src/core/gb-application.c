/* gb-application.c
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
#include <stdlib.h>

#include "gb-application.h"
#include "gb-c-language-service.h"
#include "gb-git-service.h"
#include "gb-log.h"
#include "gb-project-service.h"
#include "gb-subprocess-manager.h"
#include "gb-workspace.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "app"

G_DEFINE_TYPE(GbApplication, gb_application, GTK_TYPE_APPLICATION)

struct _GbApplicationPrivate
{
   GbSubprocessManager *sub_manager;
   gchar               *type;
   GHashTable          *service_type_to_name;
   GHashTable          *service_type_to_always_in_parent;
   GHashTable          *service_type_to_service;
   GTestDBus           *service_dbus;
   gchar               *service_dbus_addr;
   GDBusConnection     *service_connection;
};

static gchar *gExecName;

GbApplication *
gb_application_get_default (void)
{
   static GbApplication *instance;

   if (g_once_init_enter(&instance)) {
      GbApplication *app;

      app = g_object_new(GB_TYPE_APPLICATION,
                         "application-id", "org.gnome.Builder",
                         "flags", (G_APPLICATION_NON_UNIQUE |
                                   G_APPLICATION_HANDLES_OPEN |
                                   G_APPLICATION_SEND_ENVIRONMENT),
                         NULL);
      g_object_add_weak_pointer(G_OBJECT(app), (gpointer *)&instance);
      g_application_set_default(G_APPLICATION(app));
      g_once_init_leave(&instance, app);
   }

   return instance;
}

static gboolean
gb_application_is_parent (GbApplication *application)
{
   g_return_val_if_fail(GB_IS_APPLICATION(application), FALSE);

   return (application->priv->type == NULL);
}

static GDBusConnection *
gb_application_get_service_connection (GbApplication *application)
{
   GbApplicationPrivate *priv;
   GError *error = NULL;

   ENTRY;

   g_return_val_if_fail(GB_IS_APPLICATION(application), NULL);

   priv = application->priv;

   if (!priv->service_connection) {
      priv->service_connection =
         g_dbus_connection_new_for_address_sync(
            priv->service_dbus_addr,
            0,
            NULL,
            NULL,
            &error);
      if (!priv->service_connection) {
         g_error("%s", error->message);
         g_error_free(error);
         exit(1);
      }
   }

   RETURN(priv->service_connection);
}

GbServiceMode
gb_application_get_service_mode (GbApplication *application,
                                 GType          service_type)
{
   GbApplicationPrivate *priv;

   g_return_val_if_fail(GB_IS_APPLICATION(application), 0);

   priv = application->priv;

   if (gb_application_is_parent(application)) {
      if (g_hash_table_lookup(priv->service_type_to_always_in_parent,
                              GINT_TO_POINTER(service_type))) {
         return GB_SERVICE_LOCAL;
      }
   } else {
      GHashTableIter iter;
      const gchar *value_name;
      gpointer key;
      gpointer value;
      GType key_type;

      g_hash_table_iter_init(&iter, priv->service_type_to_name);
      while (g_hash_table_iter_next(&iter, &key, &value)) {
         key_type = GPOINTER_TO_INT(key);
         value_name = value;

         if ((service_type == key_type) &&
             !g_strcmp0(value_name, priv->type)) {
            return GB_SERVICE_LOCAL;
         }
      }
   }

   return GB_SERVICE_PROXY;
}

static void
gb_application_add_service (GbApplication *application,
                            GType          service_type,
                            const gchar   *name,
                            GbServiceMode  mode)
{
   GbApplicationPrivate *priv;
   GDBusConnection *connection;
   const gchar *bus_addr;
   GbService *service;
   gchar **argv;

   ENTRY;

   g_return_if_fail(GB_IS_APPLICATION(application));
   g_return_if_fail(g_type_is_a(service_type, GB_TYPE_SERVICE));
   g_return_if_fail(name);
   g_return_if_fail((mode == GB_SERVICE_LOCAL) ||
                    (mode == GB_SERVICE_PROXY));

   priv = application->priv;

   /*
    * If this is the parent process and we are using a proxy to another
    * process then we need to create the subprocess as well.
    */
   if (gb_application_is_parent(application) && (mode == GB_SERVICE_PROXY)) {
      bus_addr = g_test_dbus_get_bus_address(priv->service_dbus);

      argv = g_new0(gchar *, 4);
      argv[0] = g_strdup(gExecName);
      argv[1] = g_strdup_printf("--type=%s", name);
      argv[2] = g_strdup_printf("--service-bus=%s", bus_addr);
      argv[3] = NULL;

      gb_subprocess_manager_add(priv->sub_manager, argv, TRUE);

      g_strfreev(argv);
   }

   /*
    * Create the local process copy of the service.
    */
   connection = gb_application_get_service_connection(application);
   service = g_object_new(service_type,
                          "connection", connection,
                          "name", name,
                          "mode", mode,
                          NULL);

   g_hash_table_insert(priv->service_type_to_service,
                       GINT_TO_POINTER(service_type),
                       g_object_ref_sink(service));

   EXIT;
}

GbService *
gb_application_get_service (GbApplication *application,
                            GType          service_type)
{
   GbApplicationPrivate *priv;
   GbService *service;

   ENTRY;

   g_return_val_if_fail(GB_IS_APPLICATION(application), NULL);

   priv = application->priv;

   service = g_hash_table_lookup(priv->service_type_to_service,
                                 GINT_TO_POINTER(service_type));

   if (!service) {
      GbServiceMode mode;
      const gchar *name;

      mode = gb_application_get_service_mode(application, service_type);
      name = g_hash_table_lookup(priv->service_type_to_name,
                                 GINT_TO_POINTER(service_type));

      gb_application_add_service(application, service_type, name, mode);

      service = g_hash_table_lookup(priv->service_type_to_service,
                                    GINT_TO_POINTER(service_type));
   }

   RETURN(service);
}

static void
on_quit_activated (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
   GApplication *appliction = user_data;

   g_application_quit(appliction);
}

static void
gb_application_activate (GApplication *application)
{
   static const GActionEntry app_entries[] = {
      { "quit", on_quit_activated },
   };
   GbApplicationPrivate *priv;
   GbWorkspace *workspace;
   GtkBuilder *builder;
   GMenuModel *menu_model;

   g_return_if_fail(GB_IS_APPLICATION(application));

   priv = GB_APPLICATION(application)->priv;

   if (gb_application_is_parent(GB_APPLICATION(application))) {
      g_action_map_add_action_entries(G_ACTION_MAP(application),
                                      app_entries,
                                      G_N_ELEMENTS(app_entries),
                                      application);

      builder = gtk_builder_new();
      gtk_builder_add_from_resource(builder,
                                    "/org/gnome/Builder/ui/gb-application-menu.ui",
                                    NULL);
      menu_model = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
      gtk_application_set_app_menu(GTK_APPLICATION(application), menu_model);
      g_clear_object(&builder);

      gb_subprocess_manager_start(priv->sub_manager);

      workspace = gb_workspace_new();
      gtk_application_add_window(GTK_APPLICATION(application),
                                 GTK_WINDOW(workspace));
      gtk_window_present(GTK_WINDOW(workspace));
   } else {
      g_application_hold(application);
   }
}

static void
gb_application_startup (GApplication *application)
{
   GbApplicationPrivate *priv;

   ENTRY;

   g_return_if_fail(GB_IS_APPLICATION(application));

   priv = GB_APPLICATION(application)->priv;

   /*
    * If this is the parent process, we need to start our private DBus
    * daemon for use by all of the clients.
    *
    * NOTE: If anyone wants to add auto-launching services and make the
    *       GDBusDaemon class public, that would be a pretty neat hack to
    *       remove the need on a dbus daemon. It would, however, mean that
    *       the UI process is also doing message passing, which is probably
    *       not awesome on our allocator.
    */
   if (gb_application_is_parent(GB_APPLICATION(application))) {
      priv->service_dbus = g_test_dbus_new(G_TEST_DBUS_NONE);
      g_test_dbus_up(priv->service_dbus);
      priv->service_dbus_addr =
         g_strdup(g_test_dbus_get_bus_address(priv->service_dbus));
      g_message("%s", priv->service_dbus_addr);
   }

   G_APPLICATION_CLASS(gb_application_parent_class)->startup(application);

   EXIT;
}

static void
gb_application_shutdown (GApplication *application)
{
   GbApplicationPrivate *priv;

   ENTRY;

   g_return_if_fail(GB_IS_APPLICATION(application));

   priv = GB_APPLICATION(application)->priv;

   gb_subprocess_manager_stop(priv->sub_manager);

   G_APPLICATION_CLASS(gb_application_parent_class)->shutdown(application);

   EXIT;
}

static gboolean
gb_application_local_command_line (GApplication   *application,
                                   gchar        ***arguments,
                                   int            *exit_status)
{
   GbApplicationPrivate *priv = GB_APPLICATION(application)->priv;
   GOptionContext *context;
   GOptionEntry entries[] = {
      { "type", 0, 0, G_OPTION_ARG_STRING, &priv->type },
      { "service-bus", 0, 0, G_OPTION_ARG_STRING, &priv->service_dbus_addr },
      { NULL }
   };
   gboolean ret;
   GError *error = NULL;
   gint argc;

   ENTRY;

   gExecName = g_strdup(*arguments[0]);

   context = g_option_context_new(_("- An IDE for GNOME."));
   g_option_context_add_main_entries(context, entries, NULL);
   g_option_context_add_group(context, gtk_get_option_group(FALSE));

   argc = g_strv_length(*arguments);
   if (!g_option_context_parse(context, &argc, arguments, &error)) {
      g_printerr("%s\n", error->message);
      g_error_free(error);
      exit(EXIT_FAILURE);
   }

   g_option_context_free(context);

   ret = G_APPLICATION_CLASS(gb_application_parent_class)->
      local_command_line(application, arguments, exit_status);

   RETURN(ret);
}

static void
gb_application_finalize (GObject *object)
{
   GbApplicationPrivate *priv;

   ENTRY;

   priv = GB_APPLICATION(object)->priv;

   g_clear_object(&priv->sub_manager);
   g_clear_object(&priv->service_connection);

   g_clear_pointer(&priv->service_type_to_always_in_parent, g_hash_table_unref);
   g_clear_pointer(&priv->service_type_to_name, g_hash_table_unref);
   g_clear_pointer(&priv->service_type_to_service, g_hash_table_unref);

   g_clear_pointer(&priv->type, g_free);
   g_clear_pointer(&priv->service_dbus_addr, g_free);

   if (priv->service_dbus) {
      /* Do nothing, unref will hang. */
   }

   G_OBJECT_CLASS(gb_application_parent_class)->finalize(object);

   EXIT;
}

static void
gb_application_class_init (GbApplicationClass *klass)
{
   GObjectClass *object_class;
   GApplicationClass *app_class;

   ENTRY;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_application_finalize;
   g_type_class_add_private(object_class, sizeof(GbApplicationPrivate));

   app_class = G_APPLICATION_CLASS(klass);
   app_class->activate = gb_application_activate;
   app_class->local_command_line = gb_application_local_command_line;
   app_class->startup = gb_application_startup;
   app_class->shutdown = gb_application_shutdown;

   EXIT;
}

static void
gb_application_init (GbApplication *application)
{
   ENTRY;

   application->priv = G_TYPE_INSTANCE_GET_PRIVATE(application,
                                                   GB_TYPE_APPLICATION,
                                                   GbApplicationPrivate);

   application->priv->sub_manager = gb_subprocess_manager_new();

   application->priv->service_type_to_service =
      g_hash_table_new_full(g_direct_hash,
                            g_direct_equal,
                            NULL,
                            g_object_unref);

   application->priv->service_type_to_name =
      g_hash_table_new_full(g_direct_hash,
                            g_direct_equal,
                            NULL,
                            g_free);

   application->priv->service_type_to_always_in_parent =
      g_hash_table_new(g_direct_hash, g_direct_equal);

   EXIT;
}

void
gb_application_register_service_type (GbApplication *application,
                                      const gchar   *name,
                                      GType          service_type,
                                      gboolean       always_in_parent)
{
   GbApplicationPrivate *priv;

   ENTRY;

   g_return_if_fail(GB_IS_APPLICATION(application));
   g_return_if_fail(name);
   g_return_if_fail(g_type_is_a(service_type, GB_TYPE_SERVICE));

   priv = application->priv;

   g_hash_table_insert(priv->service_type_to_name,
                       GINT_TO_POINTER(service_type),
                       g_strdup(name));

   g_hash_table_insert(priv->service_type_to_always_in_parent,
                       GINT_TO_POINTER(service_type),
                       GINT_TO_POINTER(always_in_parent));

   EXIT;
}

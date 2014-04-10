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
#include "gb-dbus-daemon.h"
#include "gb-log.h"
#include "gb-workspace.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "app"

#define MENU_UI_PATH "/org/gnome/Builder/ui/gb-application-menu.ui"

G_DEFINE_TYPE(GbApplication, gb_application, GTK_TYPE_APPLICATION)

struct _GbApplicationPrivate
{
   gchar           *type;
   GHashTable      *service_type_to_name;
   GHashTable      *service_type_to_always_in_parent;
   GHashTable      *service_type_to_service;
   GbDbusDaemon    *service_dbus;
   gchar           *service_dbus_addr;
   GDBusConnection *service_connection;
};

typedef struct
{
   const gchar *accel;
   const gchar *action;
} Accelerator;

static gchar *gExecName;
static const Accelerator gAccelerators[] = {
   { "<Primary>f", "section.find" },
   { "<Primary>k", "section.focus-search" },
   { "<Primary>t", "section.new-tab" },
   { "<Primary><Alt>n", "section.new-tab-group" },
   { "<Primary>n", "section.new-document" },
   { "<Primary>o", "section.open-document" },
   { "<Primary>s", "section.save-document" },
   { "<Primary>w", "section.close" },
};

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
   const gchar *addr;
   GError *error = NULL;

   ENTRY;

   g_return_val_if_fail(GB_IS_APPLICATION(application), NULL);

   priv = application->priv;

   addr = priv->service_dbus_addr;

   if (gb_application_is_parent(application)) {
      addr = gb_dbus_daemon_get_address(priv->service_dbus);
   }

   if (!priv->service_connection) {
      priv->service_connection =
         g_dbus_connection_new_for_address_sync(addr, 0, NULL, NULL, &error);
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
child_setup_func (gpointer user_data)
{
#ifdef __linux
#include <sys/prctl.h>
  prctl (PR_SET_PDEATHSIG, 15);
#endif
}

static void
gb_application_launch_service (GbApplication *application,
                               GType          service_type,
                               const gchar   *name)
{
   GbApplicationPrivate *priv;
   GSubprocessLauncher *launcher;
   GSubprocess *subprocess;
   GError *error = NULL;

   g_return_if_fail(GB_IS_APPLICATION(application));
   g_return_if_fail(g_type_is_a(service_type, GB_TYPE_SERVICE));
   g_return_if_fail(name);

   priv = application->priv;

   launcher = g_subprocess_launcher_new(G_SUBPROCESS_FLAGS_NONE);
   g_subprocess_launcher_set_child_setup(launcher,
                                         child_setup_func,
                                         NULL,
                                         NULL);

   subprocess = g_subprocess_launcher_spawn(launcher, &error,
                                            gExecName,
                                            "--type",
                                            name,
                                            "--service-bus",
                                            priv->service_dbus_addr,
                                            NULL);

   if (!subprocess) {
      g_error("%s", error->message);
      g_error_free(error);
      exit(1);
   }

   g_clear_object(&launcher);
   g_clear_object(&subprocess);
}

static void
gb_application_add_service (GbApplication *application,
                            GType          service_type,
                            const gchar   *name,
                            GbServiceMode  mode)
{
   GbApplicationPrivate *priv;
   GDBusConnection *connection;
   GbService *service;

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
      gb_application_launch_service(application, service_type, name);
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
   GtkBuilder *builder;
   GMenuModel *menu_model;
   GtkWidget *workspace;
   gint i;

   g_return_if_fail(GB_IS_APPLICATION(application));

   if (gb_application_is_parent(GB_APPLICATION(application))) {
      builder = gtk_builder_new();
      gtk_builder_add_from_resource(builder, MENU_UI_PATH, NULL);
      menu_model = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
      gtk_application_set_app_menu(GTK_APPLICATION(application), menu_model);
      g_clear_object(&builder);

      workspace = g_object_new(GB_TYPE_WORKSPACE,
                               "application", application,
                               NULL);
      gtk_application_add_window(GTK_APPLICATION(application),
                                 GTK_WINDOW(workspace));

      g_action_map_add_action_entries(G_ACTION_MAP(application),
                                      app_entries,
                                      G_N_ELEMENTS(app_entries),
                                      application);

      for (i = 0; i < G_N_ELEMENTS(gAccelerators); i++) {
         gtk_application_add_accelerator(GTK_APPLICATION(application),
                                         gAccelerators[i].accel,
                                         gAccelerators[i].action,
                                         NULL);
      }

      gtk_window_maximize(GTK_WINDOW(workspace));
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
    */
   if (gb_application_is_parent(GB_APPLICATION(application))) {
      priv->service_dbus = gb_dbus_daemon_new();
      gb_dbus_daemon_start(priv->service_dbus);
   }

   G_APPLICATION_CLASS(gb_application_parent_class)->startup(application);

   EXIT;
}

static void
gb_application_shutdown (GApplication *application)
{
   GbApplicationPrivate *priv;

   ENTRY;

   g_return_if_fail (GB_IS_APPLICATION(application));

   priv = GB_APPLICATION (application)->priv;

   if (priv->service_dbus) {
      gb_dbus_daemon_stop (priv->service_dbus);
   }

   G_APPLICATION_CLASS (gb_application_parent_class)->shutdown (application);

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

   g_clear_object(&priv->service_connection);

   g_clear_pointer(&priv->service_type_to_always_in_parent, g_hash_table_unref);
   g_clear_pointer(&priv->service_type_to_name, g_hash_table_unref);
   g_clear_pointer(&priv->service_type_to_service, g_hash_table_unref);

   g_clear_pointer(&priv->type, g_free);
   g_clear_pointer(&priv->service_dbus_addr, g_free);

   if (priv->service_dbus) {
      gb_dbus_daemon_stop(priv->service_dbus);
   }

   g_clear_object(&priv->service_dbus);

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

   /*
    * XXX: Workaround for webkit having some sort of failure when glade is also
    *      loaded into the same process. It would be nice to figure out why
    *      this is the case, but this works for now.
    */
   g_type_ensure(GTK_TYPE_OFFSCREEN_WINDOW);

   g_hash_table_insert(priv->service_type_to_name,
                       GINT_TO_POINTER(service_type),
                       g_strdup(name));

   g_hash_table_insert(priv->service_type_to_always_in_parent,
                       GINT_TO_POINTER(service_type),
                       GINT_TO_POINTER(always_in_parent));

   /*
    * HACK: Try to detect if we are running in tree and adjust our
    *       GSETTINGS_SCHEMA_DIR.
    */
   if (g_file_test ("gschemas.compiled", G_FILE_TEST_IS_REGULAR)) {
      g_setenv ("GSETTINGS_SCHEMA_DIR", ".", FALSE);
   }

   EXIT;
}

/* gnome-builder-worker.c
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

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixconnection.h>
#include <stdlib.h>

#include "gb-worker-typelib.h"

static GOptionContext *gContext;
static gboolean        gClang;
static gboolean        gTypelib;
static int             gDbusFd = -1;
static GOptionEntry    gEntries[] = {
   { "clang", 0, 0, G_OPTION_ARG_NONE, &gClang, "Run the clang worker process." },
   { "typelib", 0, 0, G_OPTION_ARG_NONE, &gTypelib, "Run the typelib worker process." },
   { "dbus-fd", 0, 0, G_OPTION_ARG_INT, &gDbusFd, "FD for private D-Bus communication." },
   { NULL }
};

static void
activate (GApplication *application)
{
   GDBusConnection *dbus_conn;
   GIOStream *io_stream;
   GSocket *socket;
   GError *error = NULL;

   if (!(socket = g_socket_new_from_fd(gDbusFd, &error))) {
      g_printerr("%s\n", error->message);
      return;
   }

   io_stream = g_object_new(G_TYPE_UNIX_CONNECTION,
                            "socket", socket,
                            NULL);
   g_assert(G_IS_IO_STREAM(io_stream));

   dbus_conn = g_dbus_connection_new_sync(io_stream,
                                          NULL,
                                          G_DBUS_CONNECTION_FLAGS_NONE,
                                          NULL,
                                          NULL,
                                          &error);

   if (!dbus_conn) {
      g_printerr("%s\n", error->message);
      return;
   }

   if (gTypelib) {
      gb_worker_typelib_init(dbus_conn);
   }

   if (gClang) {
      /*
       * TODO:
       */
   }
}

static gint
on_command_line (GApplication            *application,
                 GApplicationCommandLine *command_line,
                 gpointer                 user_data)
{
   GError *error = NULL;
   gchar **argv;
   gint argc;

   argv = g_application_command_line_get_arguments(command_line, &argc);

   gContext = g_option_context_new(NULL);
   g_option_context_add_main_entries(gContext, gEntries, NULL);
   if (!g_option_context_parse(gContext, &argc, &argv, &error)) {
      g_printerr("%s\n", error->message);
      g_error_free(error);
      g_strfreev(argv);
      return EXIT_FAILURE;
   }

   if (gDbusFd == -1) {
      g_printerr("--dbus-fd Must be specified.\n");
      g_printerr("\n");
      g_printerr("%s\n", g_option_context_get_help(gContext, TRUE, NULL));
      return EXIT_FAILURE;
   }

   g_application_hold(application);
   activate(application);

   g_strfreev(argv);

   return EXIT_SUCCESS;
}

gint
main (gint   argc,
      gchar *argv[])
{
   GApplication *app;

   g_set_prgname("gnome-builder-worker");

   app = g_object_new(G_TYPE_APPLICATION,
                      "application-id", "org.gnome.Builder.Worker",
                      "flags", (G_APPLICATION_NON_UNIQUE |
                                G_APPLICATION_HANDLES_COMMAND_LINE),
                      NULL);

   g_application_set_default(app);

   g_signal_connect(app,
                    "command-line",
                    G_CALLBACK(on_command_line),
                    NULL);

   g_application_run(app, argc, argv);

   g_object_unref(app);

   return EXIT_SUCCESS;
}

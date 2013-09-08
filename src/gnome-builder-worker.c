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

#include <gio/gio.h>
#include <gio/gunixconnection.h>
#include <stdlib.h>

static gboolean gClang;
static gboolean gTypelib;
static int gDbusFd = -1;
static GOptionEntry gEntries[] = {
   { "clang", 0, 0, G_OPTION_ARG_NONE, &gClang, "Run the clang worker process." },
   { "typelib", 0, 0, G_OPTION_ARG_NONE, &gTypelib, "Run the typelib worker process." },
   { "dbus-fd", 0, 0, G_OPTION_ARG_INT, &gDbusFd, "FD for private D-Bus communication." },
   { NULL }
};

gint
main (gint   argc,
      gchar *argv[])
{
   GDBusConnection *dbus_conn;
   GOptionContext *context;
   GIOStream *io_stream;
   GSocket *socket;
   GError *error = NULL;

   g_set_prgname("gnome-builder-worker");

   context = g_option_context_new(NULL);
   g_option_context_add_main_entries(context, gEntries, NULL);
   if (!g_option_context_parse(context, &argc, &argv, &error)) {
      g_printerr("%s\n", error->message);
      g_error_free(error);
      return EXIT_FAILURE;
   }

   if (gDbusFd == -1) {
      g_printerr("--dbus-fd Must be specified.\n");
      g_printerr("\n");
      g_printerr("%s\n", g_option_context_get_help(context, TRUE, NULL));
      return EXIT_FAILURE;
   }

   if (!(socket = g_socket_new_from_fd(gDbusFd, &error))) {
      g_printerr("%s\n", error->message);
      return EXIT_FAILURE;
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
      return EXIT_FAILURE;
   }

   if (gTypelib) {
   }

   if (gClang) {
   }

   return EXIT_SUCCESS;
}

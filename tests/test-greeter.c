/* test-greeter.c
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

#include <gtk/gtk.h>

#include "gb-workspace-greeter.h"

gint
main (gint argc,
      gchar *argv[])
{
   GtkWidget *window;
   GtkWidget *greeter;

   gtk_init(&argc, &argv);

   window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
   greeter = g_object_new(GB_TYPE_WORKSPACE_GREETER,
                          "expand", TRUE,
                          "visible", TRUE,
                          NULL);
   gtk_container_add(GTK_CONTAINER(window), greeter);
   gtk_widget_show(greeter);
   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(GTK_WINDOW(window));

   gtk_main();

   return 0;
}

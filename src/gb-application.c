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

#include <girepository.h>
#include <glib/gi18n.h>
#include <stdlib.h>

#include "gb-application.h"
#include "gb-application-resource.h"
#include "gb-file-filters.h"
#include "gb-source-pane.h"
#include "gb-workspace.h"

G_DEFINE_TYPE(GbApplication, gb_application, GTK_TYPE_APPLICATION)

struct _GbApplicationPrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

/**
 * gb_application_get_default:
 *
 * Fetch the default application instance.
 *
 * Returns: (transfer none): A #GbApplication.
 */
GbApplication *
gb_application_get_default (void)
{
   static GbApplication *instance;
   GbApplication *app;

   if (g_once_init_enter(&instance)) {
      app = g_object_new(GB_TYPE_APPLICATION,
                         "application-id", "org.gnome.Builder",
                         "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                         "register-session", TRUE,
                         NULL);
      g_application_set_default(G_APPLICATION(app));
      g_once_init_leave(&instance, app);
   }

   return instance;
}

static void
on_quit_activated (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       application)
{
   g_application_quit(application);
}

static void
on_open_activated (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       application)
{
   GtkWidget *parent = NULL;
   GtkWidget *pane;
   GtkDialog *dialog;
   GList *windows;
   GFile *file;
   char *projects_path;

   if ((windows = gtk_application_get_windows(application))) {
      parent = windows->data;
   }

   dialog = g_object_new(GTK_TYPE_FILE_CHOOSER_DIALOG,
                         "action", GTK_FILE_CHOOSER_ACTION_OPEN,
                         "title", _("Open File"),
                         "transient-for", parent,
                         NULL);

   projects_path = g_build_filename(g_get_home_dir(), "Projects", NULL);
   if (g_file_test(projects_path, G_FILE_TEST_IS_DIR)) {
      gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog),
                                           projects_path, NULL);
   }
   g_free(projects_path);

   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                               gb_file_filter_text_new());
   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                               gb_file_filter_c_new());
   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                               gb_file_filter_header_new());
   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                               gb_file_filter_js_new());

   gtk_dialog_add_buttons(dialog,
                          _("Cancel"), GTK_RESPONSE_CANCEL,
                          _("Open"), GTK_RESPONSE_OK,
                          NULL);

   if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
      if ((file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog)))) {
         pane = g_object_new(GB_TYPE_SOURCE_PANE,
                             "visible", TRUE,
                             NULL);
         gtk_container_add(GTK_CONTAINER(parent), pane);
         gb_source_pane_load_async(GB_SOURCE_PANE(pane), file, NULL, NULL, NULL);
         g_object_unref(file);
         gtk_widget_grab_focus(pane);
      }
   }

   gtk_widget_destroy(GTK_WIDGET(dialog));
}

static GActionEntry app_entries[] = {
   { "open", on_open_activated, NULL, NULL, NULL },
   { "quit", on_quit_activated, NULL, NULL, NULL },
};

static void
gb_application_activate (GApplication *application)
{
   GdkRectangle geometry;
   GtkWindow *window;
   GdkScreen *screen;

   {
      GtkCssProvider *provider;
      GError *error = NULL;
      GBytes *bytes;

      bytes = g_resource_lookup_data(gb_application_get_resource(), "/org/gnome/Builder/data/css/overrides.css", 0, NULL);
      if (bytes) {
         provider = gtk_css_provider_new();
         if (!gtk_css_provider_load_from_data(provider, g_bytes_get_data(bytes, NULL), g_bytes_get_size(bytes), &error)) {
            g_printerr("%s\n", error->message);
            g_clear_error(&error);
         } else {
            gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
         }
         g_object_unref(provider);
         g_bytes_unref(bytes);
      }
   }

   g_action_map_add_action_entries(G_ACTION_MAP(application), app_entries, G_N_ELEMENTS(app_entries), application);

   {
      GtkBuilder *builder;
      GMenuModel *model;
      GError *error = NULL;

      builder = gtk_builder_new();
      if (!gtk_builder_add_from_resource(builder, "/org/gnome/Builder/data/ui/gb-application-menu.ui", &error)) {
         g_error("%s", error->message);
      }

      model = G_MENU_MODEL(gtk_builder_get_object(builder, "appmenu"));
      gtk_application_set_app_menu(GTK_APPLICATION(application), model);
   }

   window = g_object_new(GB_TYPE_WORKSPACE,
                         "title", _("Builder"),
                         "window-position", GTK_WIN_POS_CENTER,
                         NULL);
   screen = gdk_screen_get_default();
   gdk_screen_get_monitor_geometry(screen,
                                   gdk_screen_get_primary_monitor(screen),
                                   &geometry);
   gtk_window_set_default_size(window,
                               geometry.width * 0.8,
                               geometry.height * 0.8);
   gtk_window_maximize(window);
   gtk_application_add_window(GTK_APPLICATION(application), window);
   gtk_window_present(window);
}

static int
gb_application_command_line (GApplication *application,
                             GApplicationCommandLine *command_line)
{
   GOptionContext *context;
   GInputStream *stream;
   GError *error = NULL;
   gchar **args;
   gchar **argv;
   gchar *path;
   GFile *file;
   gint argc;
   gint i;
   int ret = EXIT_SUCCESS;

   g_return_val_if_fail(GB_IS_APPLICATION(application), 0);
   g_return_val_if_fail(G_IS_APPLICATION_COMMAND_LINE(command_line), 0);

   args = g_application_command_line_get_arguments(command_line, &argc);
   argv = g_new0(gchar *, argc + 1);
   for (i = 0; i < argc; i++) {
      argv[i] = args[i];
   }

   context = g_option_context_new(_("- Build applications for Gnome."));
   g_option_context_add_group(context, gtk_get_option_group(TRUE));
   g_option_context_add_group(context, g_irepository_get_option_group());
   if (!g_option_context_parse(context, &argc, &argv, &error)) {
      g_application_command_line_printerr(command_line, "%s\n",
                                          error->message);
      g_error_free(error);
      ret = EXIT_FAILURE;
      goto failure;
   }
   g_option_context_free(context);

   /*
    * If there is an argument left and it is a path to a directory,
    * we will attempt to open it as a project.
    */
   if ((argc == 2) && g_file_test(argv[1], G_FILE_TEST_IS_DIR)) {
      /*
       * TODO: Load project file.
       */
   } else {
      g_application_activate(application);
   }

failure:
   g_strfreev(args);
   g_free(argv);

   return ret;
}

static void
gb_application_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_application_parent_class)->finalize(object);
}

static void
gb_application_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   //GbApplication *application = GB_APPLICATION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_application_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   //GbApplication *application = GB_APPLICATION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_application_class_init (GbApplicationClass *klass)
{
   GObjectClass *object_class;
   GApplicationClass *application_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_application_finalize;
   object_class->get_property = gb_application_get_property;
   object_class->set_property = gb_application_set_property;
   g_type_class_add_private(object_class, sizeof(GbApplicationPrivate));

   application_class = G_APPLICATION_CLASS(klass);
   application_class->activate = gb_application_activate;
   application_class->command_line = gb_application_command_line;
}

static void
gb_application_init (GbApplication *application)
{
   application->priv = G_TYPE_INSTANCE_GET_PRIVATE(application,
                                                   GB_TYPE_APPLICATION,
                                                   GbApplicationPrivate);
}

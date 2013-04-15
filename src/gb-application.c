/* gb-application.c
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

#include "gb-application.h"
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

   if ((windows = gtk_application_get_windows(application))) {
      parent = windows->data;
   }

   dialog = g_object_new(GTK_TYPE_FILE_CHOOSER_DIALOG,
                         "action", GTK_FILE_CHOOSER_ACTION_OPEN,
                         "title", _("Open File"),
                         "transient-for", parent,
                         NULL);
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

      provider = gtk_css_provider_new();
      if (!gtk_css_provider_load_from_path(provider, "css/overrides.css", &error)) {
         g_printerr("%s\n", error->message);
         g_error_free(error);
      }

      gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), 1000);// GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

      g_object_unref(provider);
   }

   g_action_map_add_action_entries(G_ACTION_MAP(application), app_entries, G_N_ELEMENTS(app_entries), application);

   {
      GtkBuilder *builder;
      GMenuModel *model;
      GError *error = NULL;

      builder = gtk_builder_new();
      if (!gtk_builder_add_from_file(builder, "builder-menu.ui", &error)) {
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
}

static void
gb_application_init (GbApplication *application)
{
   application->priv = G_TYPE_INSTANCE_GET_PRIVATE(application,
                                                   GB_TYPE_APPLICATION,
                                                   GbApplicationPrivate);
}

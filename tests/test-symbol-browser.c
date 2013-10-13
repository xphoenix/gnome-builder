#include <girepository.h>

#include "gb-application-resource.h"
#include "gb-icon-theme.h"
#include "gb-symbol-tree-gir.h"

gint
main (gint   argc,
      gchar *argv[])
{
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *tree;

   gtk_init(&argc, &argv);

   g_resources_register(gb_application_get_resource());

   gb_icon_theme_init();

   g_irepository_require(g_irepository_get_default(),
                         "Gtk", "3.0", G_IREPOSITORY_LOAD_FLAG_LAZY,
                         NULL);

   window = g_object_new(GTK_TYPE_WINDOW,
                         "title", "Browser",
                         "default-width", 300,
                         "default-height", 600,
                         NULL);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(window), scroller);

   tree = g_object_new(GB_TYPE_SYMBOL_TREE_GIR,
                       "repository", g_irepository_get_default(),
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(scroller), tree);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(window);

   gtk_main();

   return 0;
}

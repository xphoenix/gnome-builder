#include "gb-application-resource.h"
#include "gb-tree.h"
#include "gb-tree-node.h"
#include "gb-tree-builder.h"

gint
main (gint   argc,
      gchar *argv[])
{
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *tree;

   gtk_init(&argc, &argv);

   g_resources_register(gb_application_get_resource());

   window = g_object_new(GTK_TYPE_WINDOW,
                         "title", "Browser",
                         "default-width", 300,
                         "default-height", 600,
                         NULL);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(window), scroller);

   tree = g_object_new(GB_TYPE_TREE,
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(scroller), tree);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(window);

   gtk_main();

   return 0;
}

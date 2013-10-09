#include "gb-application-resource.h"
#include "gb-icon-theme.h"
#include "gb-tree.h"
#include "gb-tree-node.h"
#include "gb-tree-node-gir.h"
#include "gb-tree-builder.h"
#include "gb-tree-builder-gir.h"

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

   gb_tree_add_builder(GB_TREE(tree),
                       gb_tree_builder_gir_new());

   g_object_set(tree, "root", gb_tree_node_gir_new(NULL), NULL);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(window);

   gtk_main();

   return 0;
}

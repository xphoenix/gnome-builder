#include "gb-application-resource.h"
#include "gb-tree.h"
#include "gb-tree-node.h"
#include "gb-tree-builder.h"
#include "gb-project.h"

gint
main (gint   argc,
      gchar *argv[])
{
   GbTreeNode *root;
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *tree;
   GbProject *project;

   gtk_init(&argc, &argv);

   g_resources_register(gb_application_get_resource());

   project = g_object_new(GB_TYPE_PROJECT,
                          "name", "Builder",
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

   root = g_object_new(GB_TYPE_TREE_NODE,
                       "item", project,
                       "text", "Test",
                       NULL);

   tree = g_object_new(GB_TYPE_TREE,
                       "root", root,
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(scroller), tree);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(window);

   gtk_main();

   return 0;
}

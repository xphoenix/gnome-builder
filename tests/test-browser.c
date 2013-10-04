#include "gb-application-resource.h"
#include "gb-tree.h"
#include "gb-tree-node.h"
#include "gb-tree-builder.h"
#include "gb-project.h"
#include "gb-project-format.h"

static void
open_cb (GObject      *object,
         GAsyncResult *result,
         gpointer      user_data)
{
   GbProject *project;
   GError *error = NULL;

   project = gb_project_format_open_finish(GB_PROJECT_FORMAT(object),
                                           result,
                                           &error);
   if (!project) {
      g_warning("%s\n", error->message);
      g_error_free(error);
   }
}

gint
main (gint   argc,
      gchar *argv[])
{
   GbProjectFormat *format;
   GInputStream *stream;
   GbTreeNode *root;
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *tree;
   GbProject *project;
   GFile *file;

   gtk_init(&argc, &argv);

   g_resources_register(gb_application_get_resource());

   file = g_file_new_for_path(".testproject/.gbproject");
   stream = G_INPUT_STREAM(g_file_read(file, NULL, NULL));
   format = gb_project_format_new();
   gb_project_format_open_async(format,
                                ".testproject",
                                stream,
                                NULL,
                                open_cb,
                                NULL);
   g_object_unref(format);
   g_object_unref(file);
   g_object_unref(stream);

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

#include "gb-application-resource.h"
#include "gb-icon-theme.h"
#include "gb-tree.h"
#include "gb-tree-node.h"
#include "gb-tree-node-project.h"
#include "gb-tree-builder.h"
#include "gb-tree-builder-project.h"
#include "gb-project.h"
#include "gb-project-file.h"
#include "gb-project-group.h"
#include "gb-project-item.h"
#include "gb-project-format.h"
#include "gb-project-target.h"
#include "gb-project-target-c.h"

static void
open_cb (GObject      *object,
         GAsyncResult *result,
         gpointer      user_data)
{
   GbTreeNode *root;
   GbProject *project;
   GbTree *tree = user_data;
   GError *error = NULL;

   project = gb_project_format_open_finish(GB_PROJECT_FORMAT(object),
                                           result,
                                           &error);
   if (!project) {
      g_warning("%s\n", error->message);
      g_error_free(error);
      return;
   }

   root = g_object_new(GB_TYPE_TREE_NODE_PROJECT,
                       "project", project,
                       NULL);

   g_object_set(tree, "root", root, NULL);

   g_object_unref(project);
   g_object_unref(root);
}

gint
main (gint   argc,
      gchar *argv[])
{
   GbProjectFormat *format;
   GInputStream *stream;
   GtkWindow *window;
   GtkWidget *scroller;
   GtkWidget *tree;
   GFile *file;

   gtk_init(&argc, &argv);

   g_type_ensure(GB_TYPE_PROJECT);
   g_type_ensure(GB_TYPE_PROJECT_ITEM);
   g_type_ensure(GB_TYPE_PROJECT_FILE);
   g_type_ensure(GB_TYPE_PROJECT_GROUP);
   g_type_ensure(GB_TYPE_PROJECT_TARGET);
   g_type_ensure(GB_TYPE_PROJECT_TARGET_C);

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
                       gb_tree_builder_project_new());

   file = g_file_new_for_path(".testproject/.gbproject");
   stream = G_INPUT_STREAM(g_file_read(file, NULL, NULL));
   format = gb_project_format_new();
   gb_project_format_open_async(format,
                                ".testproject",
                                stream,
                                NULL,
                                open_cb,
                                tree);
   g_object_unref(format);
   g_object_unref(file);
   g_object_unref(stream);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(window);

   gtk_main();

   return 0;
}

#include "../src/source/gb-source-snippet-parser.h"

static void
test1 (void)
{
   GbSourceSnippetParser *parser;
   gboolean r;
   GError *error = NULL;
   GFile *file;

   file = g_file_new_for_path("test.snippet");

   parser = gb_source_snippet_parser_new();
   r = gb_source_snippet_parser_load_from_file(parser, file, &error);
   g_assert_no_error(error);
   g_assert(r);

   g_object_add_weak_pointer(G_OBJECT(parser), (gpointer *)&parser);
   g_object_unref(parser);
   g_assert(!parser);

   g_object_unref(file);
}

gint
main (gint   argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);
   g_test_add_func("/Snippets/Parser/basic", test1);
   return g_test_run();
}

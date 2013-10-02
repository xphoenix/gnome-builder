#include "gb-source-snippet.h"
#include "gb-source-snippet-chunk.h"
#include "gb-source-snippet-parser.h"

static gchar *test1_strings[] = {
   "",
   " ",
   "test",
   " ",
   "this",
   " ",
   "$1 $2|functify|upper",
   NULL
};

static void
test1 (void)
{
   GbSourceSnippetParser *parser;
   GbSourceSnippetChunk *chunk;
   GbSourceSnippet *snippet;
   gboolean r;
   GError *error = NULL;
   GFile *file;
   GList *snippets;
   guint n_chunks;
   guint i;

   file = g_file_new_for_path("test.snippet");

   parser = gb_source_snippet_parser_new();
   r = gb_source_snippet_parser_load_from_file(parser, file, &error);
   g_assert_no_error(error);
   g_assert(r);

   snippets = gb_source_snippet_parser_get_snippets(parser);
   snippet = snippets->data;

   n_chunks = gb_source_snippet_get_n_chunks(snippet);
   for (i = 0; i < n_chunks; i++) {
      chunk = gb_source_snippet_get_nth_chunk(snippet, i);
      g_assert_cmpstr(test1_strings[i], ==,
                      gb_source_snippet_chunk_get_spec(chunk));
   }

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

#include "../src/source/snippet-parser.h"

static const gchar *test1_data[] = {
   "snippet a\n",
   "\t$0 ${1} ${2:data} three ${4}.\n",
   NULL
};

static void
test1_cb (GbSourceSnippet *snippet,
          gpointer         user_data)
{
}

static void
test1 (void)
{
   SnippetParser *parser;
   gint i;

   parser = snippet_parser_new(test1_cb, NULL);
   for (i = 0; test1_data[i]; i++) {
      snippet_parser_feed_line(parser, test1_data[i]);
   }
   snippet_parser_finish(parser);
   snippet_parser_free(parser);
}

static void
test2 (void)
{
   GbSourceSnippetChunk *chunk;
   const gchar *text;

   chunk = g_object_new(GB_TYPE_SOURCE_SNIPPET_CHUNK,
                        "text", "some func",
                        NULL);
   g_object_add_weak_pointer(G_OBJECT(chunk), (gpointer *)&chunk);

   gb_source_snippet_chunk_add_filter(chunk, "functify");
   gb_source_snippet_chunk_add_filter(chunk, "upper");

   text = gb_source_snippet_chunk_get_text(chunk);
   g_assert_cmpstr(text, ==, "SOME_FUNC");

   gb_source_snippet_chunk_add_filter(chunk, "camelize");
   text = gb_source_snippet_chunk_get_text(chunk);
   g_assert_cmpstr(text, ==, "SomeFunc");

   g_object_unref(chunk);
   g_assert(!chunk);
}

gint
main (gint   argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);
   g_test_add_func("/Snippets/Parser/basic", test1);
   g_test_add_func("/Snippets/Parser/filters", test2);
   return g_test_run();
}

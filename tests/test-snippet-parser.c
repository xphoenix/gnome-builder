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

gint
main (gint   argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);
   g_test_add_func("/Snippets/Parser/basic", test1);
   return g_test_run();
}

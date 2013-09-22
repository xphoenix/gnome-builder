#include "gb-source-snippet-context.h"

static void
test1 (void)
{
   GbSourceSnippetContext *context;
   gchar *expanded;

   context = gb_source_snippet_context_new();
   gb_source_snippet_context_add_variable(context, "$1", "abcd");
   gb_source_snippet_context_add_variable(context, "$2", "defg");
   gb_source_snippet_context_add_variable(context, "$123123", "asdf");

   expanded = gb_source_snippet_context_expand(context, "$123123 $1\\|$1_$2");
   g_assert_cmpstr(expanded, ==, "asdf abcd|abcd_defg");
   g_free(expanded);

   expanded = gb_source_snippet_context_expand(context, "\\$123");
   g_assert_cmpstr(expanded, ==, "$123");
   g_free(expanded);

   expanded = gb_source_snippet_context_expand(context, "\\|upper");
   g_assert_cmpstr(expanded, ==, "|upper");
   g_free(expanded);

   g_object_add_weak_pointer(G_OBJECT(context), (gpointer *)&context);
   g_object_unref(context);
   g_assert(!context);
}

static void
test2 (void)
{
   GbSourceSnippetContext *context;
   gchar *expanded;

   context = gb_source_snippet_context_new();
   gb_source_snippet_context_add_variable(context, "$1", "abcd");
   gb_source_snippet_context_add_variable(context, "$2", "defg");
   gb_source_snippet_context_add_variable(context, "$123123", "asdf");

   expanded = gb_source_snippet_context_expand(context, "$123123 $1\\|$1_$2|upper");
   g_assert_cmpstr(expanded, ==, "ASDF ABCD|ABCD_DEFG");
   g_free(expanded);

   expanded = gb_source_snippet_context_expand(context, "$123123 $1\\|$1_$2|lower");
   g_assert_cmpstr(expanded, ==, "asdf abcd|abcd_defg");
   g_free(expanded);

   expanded = gb_source_snippet_context_expand(context, "some_func|camelize");
   g_assert_cmpstr(expanded, ==, "SomeFunc");
   g_free(expanded);

   expanded = gb_source_snippet_context_expand(context, "SomeFunc|functify");
   g_assert_cmpstr(expanded, ==, "some_func");
   g_free(expanded);

   expanded = gb_source_snippet_context_expand(context, "SomeFunc|functify|upper");
   g_assert_cmpstr(expanded, ==, "SOME_FUNC");
   g_free(expanded);

   g_object_add_weak_pointer(G_OBJECT(context), (gpointer *)&context);
   g_object_unref(context);
   g_assert(!context);
}

gint
main (gint argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);

   g_test_add_func("/Gb/Source/Snippet/Context/expand", test1);
   g_test_add_func("/Gb/Source/Snippet/Context/filter", test2);

   return g_test_run();
}

#include "fuzzy.h"

static void
test_search_fuzzy (void)
{
   FuzzyMatch match;
   GArray *matches;
   Fuzzy *fuzzy;

   fuzzy = fuzzy_new();

   fuzzy_insert(fuzzy, "gtk_widget_show", NULL);
   fuzzy_insert(fuzzy, "gtk_widget_hide", NULL);

   matches = fuzzy_match(fuzzy, "gtkwtshw", 0);
   g_assert_cmpint(matches->len, ==, 1);
   match = g_array_index(matches, FuzzyMatch, 0);
   g_assert_cmpstr(match.key, ==, "gtk_widget_show");
   g_array_unref(matches);

   matches = fuzzy_match(fuzzy, "kwthde", 0);
   g_assert_cmpint(matches->len, ==, 1);
   match = g_array_index(matches, FuzzyMatch, 0);
   g_assert_cmpstr(match.key, ==, "gtk_widget_hide");
   g_array_unref(matches);

   fuzzy_free(fuzzy);
}

gint
main (gint argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);
   g_test_add_func("/Search/Fuzzy", test_search_fuzzy);
   return g_test_run();
}

#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "gb-source-view.h"
#include "gb-source-snippet.h"
#include "gb-source-snippet-parser.h"

static GtkWidget *window;
static GtkWidget *scroller;
static GtkWidget *view;
static GbSourceSnippet *snippet;

static gboolean
push_snippet (gpointer data)
{
   gb_source_view_push_snippet(GB_SOURCE_VIEW(view), snippet);

   return FALSE;
}

static void
monospace (GtkWidget *widget)
{
   PangoFontDescription *font_desc;

   font_desc = pango_font_description_from_string("Monospace");
   gtk_widget_override_font(widget, font_desc);
   pango_font_description_free(font_desc);
}

gint
main (gint   argc,
      gchar *argv[])
{
   GbSourceSnippetParser *parser;
   GFile *file;
   GList *list;

   gtk_init(&argc, &argv);

   file = g_file_new_for_path("test.snippet");
   parser = g_object_new(GB_TYPE_SOURCE_SNIPPET_PARSER, NULL);
   gb_source_snippet_parser_load_from_file(parser, file, NULL);
   list = gb_source_snippet_parser_get_snippets(parser);
   g_assert(list);
   snippet = list->data;

   window = g_object_new(GTK_TYPE_WINDOW,
                         "title", "Editor",
                         "default-width", 800,
                         "default-height", 500,
                         "window-position", GTK_WIN_POS_CENTER,
                         NULL);

   scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
   gtk_container_add(GTK_CONTAINER(window), scroller);

   view = g_object_new(GB_TYPE_SOURCE_VIEW,
                       "visible", TRUE,
                       NULL);
   gtk_container_add(GTK_CONTAINER(scroller), view);
   monospace(view);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(GTK_WINDOW(window));

   g_timeout_add(1000, push_snippet, NULL);

   gtk_main();

   return 0;
}


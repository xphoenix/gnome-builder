#include <glib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#include "gb-source-snippet.h"
#include "gb-source-snippet-completion-provider.h"
#include "gb-source-snippet-parser.h"
#include "gb-source-snippets.h"
#include "gb-source-view.h"

static GtkWidget *window;
static GtkWidget *scroller;
static GtkWidget *view;

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
   GtkSourceCompletionProvider *provider;
   GbSourceSnippetParser *parser;
   GtkSourceStyleScheme *s;
   GtkSourceCompletion *completion;
   GtkSourceLanguage *l;
   GbSourceSnippets *snippets;
   GtkSourceBuffer *buffer;
   GFile *file;
   GList *list;

   gtk_init(&argc, &argv);

   snippets = gb_source_snippets_new();

   file = g_file_new_for_path("test.snippet");
   parser = g_object_new(GB_TYPE_SOURCE_SNIPPET_PARSER, NULL);
   gb_source_snippet_parser_load_from_file(parser, file, NULL);
   list = gb_source_snippet_parser_get_snippets(parser);
   for (; list; list = list->next) {
      gb_source_snippets_add(snippets, list->data);
   }
   g_object_unref(parser);
   g_object_unref(file);

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

   l = gtk_source_language_manager_get_language(
      gtk_source_language_manager_get_default(),
      "c");

   s = gtk_source_style_scheme_manager_get_scheme(
      gtk_source_style_scheme_manager_get_default(),
      "tango");

   buffer = g_object_new(GTK_SOURCE_TYPE_BUFFER,
                         "language", l,
                         "style-scheme", s,
                         NULL);
   view = g_object_new(GB_TYPE_SOURCE_VIEW,
                       "buffer", buffer,
                       "visible", TRUE,
                       "show-line-numbers", TRUE,
                       "show-right-margin", TRUE,
                       "right-margin-position", 80,
                       NULL);
   monospace(view);
   gtk_container_add(GTK_CONTAINER(scroller), view);

   completion = gtk_source_view_get_completion((GtkSourceView *)view);
   provider = gb_source_snippet_completion_provider_new(GB_SOURCE_VIEW(view), snippets);
   gtk_source_completion_add_provider(completion, provider, NULL);

   g_signal_connect(window, "delete-event", gtk_main_quit, NULL);
   gtk_window_present(GTK_WINDOW(window));

   gtk_main();

   return 0;
}


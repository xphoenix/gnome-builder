/* gb-source-view.c:
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestylescheme.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "gb-source-state.h"
#include "gb-source-view.h"

#ifndef g_str_empty0
#define g_str_empty0(s) (!(s) || !(s)[0])
#endif

G_DEFINE_TYPE(GbSourceView, gb_source_view, GTK_SOURCE_TYPE_VIEW)

struct _GbSourceViewPrivate
{
   gboolean       search_has_matches;
   char          *search_text;
   GbSourceState *state;
};

enum
{
   PROP_0,
   PROP_SEARCH_HAS_MATCHES,
   PROP_SEARCH_TEXT,
   PROP_STATE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

gboolean
gb_source_view_get_search_has_matches (GbSourceView *view)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), FALSE);
   return view->priv->search_has_matches;
}

const gchar *
gb_source_view_get_search_text (GbSourceView *view)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), NULL);
   return view->priv->search_text;
}

static GtkTextTag *
gb_source_view_ref_search_tag (GbSourceView *view)
{
   GtkTextTagTable *tag_table;
   GtkTextBuffer *buffer;
   GtkTextTag *search_tag;

   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), NULL);

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
   tag_table = gtk_text_buffer_get_tag_table(buffer);
   if (!(search_tag = gtk_text_tag_table_lookup(tag_table, "Tag::Search"))) {
      search_tag = g_object_new(GTK_TYPE_TEXT_TAG,
                                "name", "Tag::Search",
                                "background", "#fce94f",
                                "foreground", "#2e3436",
                                NULL);
      gtk_text_tag_table_add(tag_table, search_tag);
      return search_tag;
   }

   return g_object_ref(search_tag);
}

static void
gb_source_view_update_search (GbSourceView *view,
                              gboolean      force_notify)
{
   GbSourceViewPrivate *priv;
   GRegexCompileFlags flags = 0;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;
   GMatchInfo *match_info = NULL;
   GtkTextTag *search_tag;
   gboolean has_matches = FALSE;
   GRegex *regex = NULL;
   gchar *text;
   gchar *escaped;

   g_assert(GB_IS_SOURCE_VIEW(view));

   priv = view->priv;

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
   gtk_text_buffer_get_bounds(buffer, &begin, &end);

   search_tag = gb_source_view_ref_search_tag(view);
   gtk_text_buffer_remove_tag(buffer, search_tag, &begin, &end);

   if (g_str_empty0(priv->search_text)) {
      goto cleanup;
   }

   if (priv->search_text) {
#if 0
      if (!priv->search_case_sensitive) {
         flags = G_REGEX_CASELESS;
      }
#else
      flags = G_REGEX_CASELESS;
#endif
      escaped = g_regex_escape_string(priv->search_text, -1);
      regex = g_regex_new(escaped, flags, 0, NULL);
      g_free(escaped);
   }

   if (regex) {
      text = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
      if (g_regex_match(regex, text, 0, &match_info)) {
         guint count;
         guint i;
         gint begin_pos;
         gint end_pos;

         do {
            count = g_match_info_get_match_count(match_info);
            for (i = 0; i < count; i++) {
               if (g_match_info_fetch_pos(match_info, i, &begin_pos, &end_pos)) {
                  gtk_text_buffer_get_iter_at_offset(buffer, &begin, begin_pos);
                  gtk_text_buffer_get_iter_at_offset(buffer, &end, end_pos);
                  gtk_text_buffer_apply_tag(buffer, search_tag, &begin, &end);
                  has_matches = TRUE;
               }
            }
         } while (g_match_info_next(match_info, NULL));
      }
      g_match_info_free(match_info);
      g_regex_unref(regex);
      g_free(text);
   }

cleanup:
   if (force_notify || (priv->search_has_matches != has_matches)) {
      priv->search_has_matches = has_matches;
      g_object_notify_by_pspec(G_OBJECT(view), gParamSpecs[PROP_SEARCH_HAS_MATCHES]);
   }

   gtk_widget_queue_draw(GTK_WIDGET(view));
   g_object_unref(search_tag);
}

void
gb_source_view_set_search_text (GbSourceView *view,
                                const gchar  *search_text)
{
   gboolean force_notify;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   force_notify = !!g_strcmp0(search_text, view->priv->search_text);

   g_free(view->priv->search_text);
   view->priv->search_text = g_strdup(search_text);
   g_object_notify_by_pspec(G_OBJECT(view), gParamSpecs[PROP_SEARCH_TEXT]);

   gb_source_view_update_search(view, force_notify);
}

GbSourceState *
gb_source_view_get_state (GbSourceView *view)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), NULL);
   return view->priv->state;
}

void
gb_source_view_set_state (GbSourceView  *view,
                          GbSourceState *state)
{
   g_return_if_fail(GB_IS_SOURCE_VIEW(view));
   g_return_if_fail(!state || GB_IS_SOURCE_STATE(state));

   g_clear_object(&view->priv->state);
   view->priv->state = state ? g_object_ref(state) : NULL;
   g_object_notify_by_pspec(G_OBJECT(view), gParamSpecs[PROP_STATE]);
}

static void
gb_source_view_finalize (GObject *object)
{
   GbSourceViewPrivate *priv;

   priv = GB_SOURCE_VIEW(object)->priv;

   g_clear_object(&priv->state);

   G_OBJECT_CLASS(gb_source_view_parent_class)->finalize(object);
}

static void
gb_source_view_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   GbSourceView *view = GB_SOURCE_VIEW(object);

   switch (prop_id) {
   case PROP_SEARCH_HAS_MATCHES:
      g_value_set_boolean(value, gb_source_view_get_search_has_matches(view));
      break;
   case PROP_SEARCH_TEXT:
      g_value_set_string(value, gb_source_view_get_search_text(view));
      break;
   case PROP_STATE:
      g_value_set_object(value, gb_source_view_get_state(view));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   GbSourceView *view = GB_SOURCE_VIEW(object);

   switch (prop_id) {
   case PROP_SEARCH_TEXT:
      gb_source_view_set_search_text(view, g_value_get_string(value));
      break;
   case PROP_STATE:
      gb_source_view_set_state(view, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_class_init (GbSourceViewClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_view_finalize;
   object_class->get_property = gb_source_view_get_property;
   object_class->set_property = gb_source_view_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceViewPrivate));

   gParamSpecs[PROP_SEARCH_TEXT] =
      g_param_spec_string("search-text",
                          _("Search Text"),
                          _("The current search text."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_SEARCH_TEXT,
                                   gParamSpecs[PROP_SEARCH_TEXT]);

   gParamSpecs[PROP_SEARCH_HAS_MATCHES] =
      g_param_spec_boolean("search-has-matches",
                           _("Search has matches"),
                           _("If the search-text has any matches."),
                           FALSE,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_SEARCH_HAS_MATCHES,
                                   gParamSpecs[PROP_SEARCH_HAS_MATCHES]);

   gParamSpecs[PROP_STATE] =
      g_param_spec_object("state",
                          _("State"),
                          _("The state of the source view."),
                          GB_TYPE_SOURCE_STATE,
                          (G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS |
                           G_PARAM_CONSTRUCT));
   g_object_class_install_property(object_class, PROP_STATE,
                                   gParamSpecs[PROP_STATE]);

   gtk_source_style_scheme_manager_append_search_path(
         gtk_source_style_scheme_manager_get_default(),
         "data/style-schemes");
}

static void
gb_source_view_init (GbSourceView *view)
{
   GtkSourceLanguageManager *lm;
   PangoFontDescription *font;
   GtkSourceStyleScheme *scheme;
   GtkSourceLanguage *lang;
   GtkTextBuffer *buffer;

   view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,
                                            GB_TYPE_SOURCE_VIEW,
                                            GbSourceViewPrivate);

   font = pango_font_description_from_string("Monospace 10");
   gtk_widget_override_font(GTK_WIDGET(view), font);
   pango_font_description_free(font);

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
   if (GTK_SOURCE_IS_BUFFER(buffer)) {
      scheme = gtk_source_style_scheme_manager_get_scheme(
            gtk_source_style_scheme_manager_get_default(),
            "solarized-light");

      lm = gtk_source_language_manager_get_default();
      lang = gtk_source_language_manager_get_language(lm, "c");

      g_object_set(buffer,
                   "language", lang,
                   "style-scheme", scheme,
                   NULL);
   }

   g_object_set(view,
                "show-line-numbers", TRUE,
                "show-right-margin", TRUE,
                NULL);
}

/* gb-source-view-state-snippet.c
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

#include "gb-source-view-state-insert.h"
#include "gb-source-view-state-snippet.h"

G_DEFINE_TYPE(GbSourceViewStateSnippet,
              gb_source_view_state_snippet,
              GB_TYPE_SOURCE_VIEW_STATE)

struct _GbSourceViewStateSnippetPrivate
{
   GbSourceSnippet *snippet;
   guint            key_press_handler;
   guint            mark_set_handler;
   guint            insert_text_handler;
   guint            delete_range_handler;
   guint            draw_handler;
};

enum
{
   PROP_0,
   PROP_SNIPPET,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippet *
gb_source_view_state_snippet_get_snippet (GbSourceViewStateSnippet *snippet)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW_STATE_SNIPPET(snippet), NULL);
   return snippet->priv->snippet;
}

static void
gb_source_view_state_snippet_set_snippet (GbSourceViewStateSnippet *state,
                                          GbSourceSnippet          *snippet)
{
   GbSourceViewStateSnippetPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_VIEW_STATE_SNIPPET(state));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = state->priv;

   g_clear_object(&priv->snippet);
   priv->snippet = g_object_ref(snippet);
}

static void
gb_source_view_state_snippet_on_insert_text (GtkTextBuffer            *buffer,
                                             GtkTextIter              *location,
                                             gchar                    *text,
                                             gint                      length,
                                             GbSourceViewStateSnippet *state)
{
   gb_source_snippet_insert_text(state->priv->snippet,
                                 buffer,
                                 location,
                                 text,
                                 length);
}

static void
gb_source_view_state_snippet_on_delete_range (GtkTextBuffer            *buffer,
                                              GtkTextIter              *begin,
                                              GtkTextIter              *end,
                                              GbSourceViewStateSnippet *state)
{
   GbSourceViewStateSnippetPrivate *priv;
   GbSourceViewState *insert;
   GbSourceView *view;
   GtkTextMark *mark_begin;
   GtkTextMark *mark_end;
   GtkTextIter a;
   GtkTextIter b;

   g_assert(GB_IS_SOURCE_VIEW_STATE_SNIPPET(state));
   g_assert(GTK_IS_TEXT_BUFFER(buffer));
   g_assert(begin);
   g_assert(end);

   priv = state->priv;

   /*
    * If the entire snippet has been deleted, then lets abort and go back to
    * insert state.
    */
   g_object_get(priv->snippet,
                "mark-begin", &mark_begin,
                "mark-end", &mark_end,
                NULL);
   if (mark_begin && mark_end) {
      gtk_text_buffer_get_iter_at_mark(buffer, &a, mark_begin);
      gtk_text_buffer_get_iter_at_mark(buffer, &b, mark_end);
      if (!gtk_text_iter_compare(&a, &b)) {
         view = gb_source_view_state_get_view(GB_SOURCE_VIEW_STATE(state));
         insert = gb_source_view_state_insert_new();
         g_object_set(view, "state", insert, NULL);
         g_object_unref(insert);
         return;
      }
   }
   g_clear_object(&mark_begin);
   g_clear_object(&mark_end);

   gb_source_snippet_delete_range(state->priv->snippet,
                                       buffer,
                                       begin,
                                       end);
}

static gboolean
gb_source_view_state_snippet_on_key_press (GbSourceView             *view,
                                           GdkEventKey              *key,
                                           GbSourceViewStateSnippet *snippet)
{
   GbSourceViewState *state;

   switch (key->keyval) {
   case GDK_KEY_ISO_Left_Tab:
      if (!gb_source_snippet_move_previous(snippet->priv->snippet)) {
         goto insert_state;
      }
      return TRUE;
   case GDK_KEY_Tab:
      if (!gb_source_snippet_move_next(snippet->priv->snippet)) {
         goto insert_state;
      }
      return TRUE;
   case GDK_KEY_Escape:
      goto insert_state;
   default:
      return FALSE;
   }

insert_state:
   state = g_object_new(GB_TYPE_SOURCE_VIEW_STATE_INSERT,  NULL);
   g_object_set(view, "state", state, NULL);
   gtk_widget_queue_draw(GTK_WIDGET(view));
   g_object_unref(state);

   return TRUE;
}

static void
gb_source_view_state_snippet_on_mark_set (GtkTextBuffer            *buffer,
                                          GtkTextIter              *location,
                                          GtkTextMark              *mark,
                                          GbSourceViewStateSnippet *snippet)
{
   GbSourceViewStateSnippetPrivate *priv = snippet->priv;
   GbSourceViewState *state;
   GbSourceView *view;
   GtkTextMark *insert;
   GtkTextMark *mark_begin;
   GtkTextMark *mark_end;
   GtkTextIter begin;
   GtkTextIter end;

   insert = gtk_text_buffer_get_insert(buffer);
   if (mark == insert) {
      mark_begin = gb_source_snippet_get_mark_begin(priv->snippet);
      mark_end = gb_source_snippet_get_mark_end(priv->snippet);
      gtk_text_buffer_get_iter_at_mark(buffer, &begin, mark_begin);
      gtk_text_buffer_get_iter_at_mark(buffer, &end, mark_end);

      if ((gtk_text_iter_compare(location, &begin) < 0) ||
          (gtk_text_iter_compare(location, &end) > 0)) {
         state = gb_source_view_state_insert_new();
         view = gb_source_view_state_get_view(GB_SOURCE_VIEW_STATE(snippet));
         g_object_set(view, "state", state, NULL);
         g_object_unref(state);
      }
   }
}

static gboolean
gb_source_view_state_snippet_on_draw (GtkWidget                *widget,
                                      cairo_t                  *cr,
                                      GbSourceViewStateSnippet *snippet)
{
   gb_source_snippet_draw(snippet->priv->snippet, widget, cr);
   return FALSE;
}

static void
gb_source_view_state_snippet_load (GbSourceViewState *state,
                                   GbSourceView      *view)
{
   GbSourceViewStateSnippetPrivate *priv;
   GbSourceViewStateSnippet *snippet = (GbSourceViewStateSnippet *)state;
   GtkTextBuffer *buffer;
   GtkTextMark *mark;
   GtkTextIter iter;

   g_return_if_fail(GB_IS_SOURCE_VIEW_STATE_SNIPPET(snippet));
   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   priv = snippet->priv;

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

   if (priv->snippet) {
      mark = gtk_text_buffer_get_insert(buffer);
      gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
      gb_source_snippet_insert(priv->snippet, buffer, &iter);
      gb_source_snippet_move_next(priv->snippet);
   }

   priv->draw_handler =
      g_signal_connect(view,
                       "draw",
                       G_CALLBACK(gb_source_view_state_snippet_on_draw),
                       state);

   priv->key_press_handler =
      g_signal_connect(view,
                       "key-press-event",
                       G_CALLBACK(gb_source_view_state_snippet_on_key_press),
                       state);

   priv->mark_set_handler =
      g_signal_connect(buffer,
                       "mark-set",
                       G_CALLBACK(gb_source_view_state_snippet_on_mark_set),
                       state);

   priv->insert_text_handler =
      g_signal_connect_after(
         buffer,
         "insert-text",
         G_CALLBACK(gb_source_view_state_snippet_on_insert_text),
         state);

   priv->delete_range_handler =
      g_signal_connect_after(buffer,
                             "delete-range",
                             G_CALLBACK(gb_source_view_state_snippet_on_delete_range),
                             state);
}

static void
gb_source_view_state_snippet_unload (GbSourceViewState *state,
                                     GbSourceView      *view)
{
   GbSourceViewStateSnippetPrivate *priv;
   GtkTextBuffer *buffer;

   g_return_if_fail(GB_IS_SOURCE_VIEW_STATE_SNIPPET(state));

   priv = GB_SOURCE_VIEW_STATE_SNIPPET(state)->priv;

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

   g_signal_handler_disconnect(view, priv->key_press_handler);
   priv->key_press_handler = 0;

   g_signal_handler_disconnect(view, priv->draw_handler);
   priv->draw_handler = 0;

   g_signal_handler_disconnect(buffer, priv->mark_set_handler);
   priv->mark_set_handler = 0;

   g_signal_handler_disconnect(buffer, priv->insert_text_handler);
   priv->insert_text_handler = 0;

   g_signal_handler_disconnect(buffer, priv->delete_range_handler);
   priv->delete_range_handler = 0;
}

static void
gb_source_view_state_snippet_finalize (GObject *object)
{
   GbSourceViewStateSnippetPrivate *priv;

   priv = GB_SOURCE_VIEW_STATE_SNIPPET(object)->priv;

   g_clear_object(&priv->snippet);

   G_OBJECT_CLASS(gb_source_view_state_snippet_parent_class)->finalize(object);
}

static void
gb_source_view_state_snippet_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
   GbSourceViewStateSnippet *snippet = GB_SOURCE_VIEW_STATE_SNIPPET(object);

   switch (prop_id) {
   case PROP_SNIPPET:
      g_value_set_object(value,
                         gb_source_view_state_snippet_get_snippet(snippet));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_state_snippet_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
   GbSourceViewStateSnippet *snippet = GB_SOURCE_VIEW_STATE_SNIPPET(object);

   switch (prop_id) {
   case PROP_SNIPPET:
      gb_source_view_state_snippet_set_snippet(snippet,
                                               g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_state_snippet_class_init (GbSourceViewStateSnippetClass *klass)
{
   GObjectClass *object_class;
   GbSourceViewStateClass *state_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_view_state_snippet_finalize;
   object_class->get_property = gb_source_view_state_snippet_get_property;
   object_class->set_property = gb_source_view_state_snippet_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceViewStateSnippetPrivate));

   state_class = GB_SOURCE_VIEW_STATE_CLASS(klass);
   state_class->load = gb_source_view_state_snippet_load;
   state_class->unload = gb_source_view_state_snippet_unload;

   gParamSpecs[PROP_SNIPPET] =
      g_param_spec_object("snippet",
                          _("Snippet"),
                          _("The snippet to be inserted into the view."),
                          GB_TYPE_SOURCE_SNIPPET,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
   g_object_class_install_property(object_class, PROP_SNIPPET,
                                   gParamSpecs[PROP_SNIPPET]);
}

static void
gb_source_view_state_snippet_init (GbSourceViewStateSnippet *snippet)
{
   snippet->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(snippet,
                                  GB_TYPE_SOURCE_VIEW_STATE_SNIPPET,
                                  GbSourceViewStateSnippetPrivate);
}

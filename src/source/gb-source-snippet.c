/* gb-source-snippet.c
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

#include "gb-source-snippet.h"
#include "gb-source-snippet-private.h"
#include "gb-source-snippet-chunk.h"
#include "gb-source-snippet-context.h"

#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "snippet"

#if 0
#define ENTRY     g_message("ENTRY: %s():%d", G_STRFUNC, __LINE__)
#define RETURN(r) do { g_message(" EXIT: %s():%d", G_STRFUNC, __LINE__); return r; } while (0)
#define EXIT      do { g_message(" EXIT: %s():%d", G_STRFUNC, __LINE__); return; } while (0)
#else
#define ENTRY
#define RETURN(r) return r
#define EXIT
#endif

G_DEFINE_TYPE(GbSourceSnippet, gb_source_snippet, G_TYPE_OBJECT)

struct _GbSourceSnippetPrivate
{
   GbSourceSnippetContext *context;
   GtkTextBuffer          *buffer;
   GPtrArray              *chunks;
   GArray                 *runs;
   GtkTextMark            *mark_begin;
   GtkTextMark            *mark_end;
   gchar                  *trigger;
   gint                    tab_stop;
   gint                    max_tab_stop;
   gint                    current_chunk;
   gboolean                inserted;
};

enum
{
   PROP_0,
   PROP_BUFFER,
   PROP_MARK_BEGIN,
   PROP_MARK_END,
   PROP_TRIGGER,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippet *
gb_source_snippet_new (const gchar *trigger)
{
   GbSourceSnippet *ret;

   ENTRY;
   ret = g_object_new(GB_TYPE_SOURCE_SNIPPET,
                      "trigger", trigger,
                      NULL);
   RETURN(ret);
}

GbSourceSnippet *
gb_source_snippet_copy (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GbSourceSnippet *ret;
   gint i;

   ENTRY;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);

   priv = snippet->priv;

   ret = g_object_new(GB_TYPE_SOURCE_SNIPPET,
                      "trigger", snippet->priv->trigger,
                      NULL);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      chunk = gb_source_snippet_chunk_copy(chunk);
      gb_source_snippet_add_chunk(ret, chunk);
      g_object_unref(chunk);
   }

   RETURN(ret);
}

const gchar *
gb_source_snippet_get_trigger (GbSourceSnippet *snippet)
{
   ENTRY;
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);
   RETURN(snippet->priv->trigger);
}

void
gb_source_snippet_set_trigger (GbSourceSnippet *snippet,
                               const gchar     *trigger)
{
   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   g_free(snippet->priv->trigger);
   snippet->priv->trigger = g_strdup(trigger);

   EXIT;
}

static gint
gb_source_snippet_get_offset (GbSourceSnippet *snippet,
                              GtkTextIter     *iter)
{
   GbSourceSnippetPrivate *priv;
   GtkTextIter begin;
   gint ret;

   ENTRY;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), 0);
   g_return_val_if_fail(iter, 0);

   priv = snippet->priv;

   gtk_text_buffer_get_iter_at_mark(priv->buffer, &begin, priv->mark_begin);
   ret = gtk_text_iter_get_offset(iter) - gtk_text_iter_get_offset(&begin);
   ret = MAX(0, ret);

   RETURN(ret);
}

static gint
gb_source_snippet_get_index (GbSourceSnippet *snippet,
                             GtkTextIter     *iter)
{
   GbSourceSnippetPrivate *priv;
   gint offset;
   gint run;
   gint i;

   ENTRY;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), 0);
   g_return_val_if_fail(iter, 0);

   priv = snippet->priv;

   offset = gb_source_snippet_get_offset(snippet, iter);

   for (i = 0; i < priv->runs->len; i++) {
      run = g_array_index(priv->runs, gint, i);
      offset -= run;
      if (offset <= 0) {
         /*
          * HACK: This is the central part of the hack by using offsets
          *       instead of textmarks (which gives us lots of gravity grief).
          *       We guess which snippet it is based on the current chunk.
          */
         if ((i + 1) == priv->current_chunk) {
            RETURN(i + 1);
         }
         RETURN(i);
      }
   }

   RETURN(priv->runs->len - 1);
}

static gboolean
gb_source_snippet_within_bounds (GbSourceSnippet *snippet,
                                 GtkTextIter     *iter)
{
   GbSourceSnippetPrivate *priv;
   GtkTextIter begin;
   GtkTextIter end;
   gboolean ret;

   ENTRY;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), FALSE);
   g_return_val_if_fail(iter, FALSE);

   priv = snippet->priv;

   gtk_text_buffer_get_iter_at_mark(priv->buffer, &begin, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(priv->buffer, &end, priv->mark_end);

   ret = ((gtk_text_iter_compare(&begin, iter) <= 0) &&
           (gtk_text_iter_compare(&end, iter) >= 0));

   RETURN(ret);
}

gboolean
gb_source_snippet_insert_set (GbSourceSnippet *snippet,
                              GtkTextMark      *mark)
{
   GbSourceSnippetPrivate *priv;
   GtkTextIter iter;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(GTK_IS_TEXT_MARK(mark));

   priv = snippet->priv;

   gtk_text_buffer_get_iter_at_mark(priv->buffer, &iter, mark);

   if (!gb_source_snippet_within_bounds(snippet, &iter)) {
      RETURN(FALSE);
   }

   priv->current_chunk = gb_source_snippet_get_index(snippet, &iter);

   RETURN(TRUE);
}

static void
gb_source_snippet_get_chunk_range (GbSourceSnippet *snippet,
                                   gint             n,
                                   GtkTextIter     *begin,
                                   GtkTextIter     *end)
{
   GbSourceSnippetPrivate *priv;
   gint run;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(n >= 0);
   g_return_if_fail(begin);
   g_return_if_fail(end);

   priv = snippet->priv;

   gtk_text_buffer_get_iter_at_mark(priv->buffer, begin, priv->mark_begin);

   for (i = 0; i < n; i++) {
      run = g_array_index(priv->runs, gint, i);
      gtk_text_iter_forward_chars(begin, run);
   }

   gtk_text_iter_assign(end, begin);
   run = g_array_index(priv->runs, gint, n);
   gtk_text_iter_forward_chars(end, run);

   EXIT;
}

static void
gb_source_snippet_select_chunk (GbSourceSnippet *snippet,
                                gint             n)
{
   GbSourceSnippetPrivate *priv;
   GtkTextIter begin;
   GtkTextIter end;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(n >= 0);
   g_return_if_fail(n < snippet->priv->runs->len);

   priv = snippet->priv;

   gb_source_snippet_get_chunk_range(snippet, n, &begin, &end);
   gtk_text_buffer_select_range(priv->buffer, &begin, &end);
   priv->current_chunk = n;

   EXIT;
}

gboolean
gb_source_snippet_move_next (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk = NULL;
   GtkTextIter iter;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   if (priv->tab_stop > priv->max_tab_stop) {
      RETURN(FALSE);
   }

   priv->tab_stop++;

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      if (gb_source_snippet_chunk_get_tab_stop(chunk) == priv->tab_stop) {
         gb_source_snippet_select_chunk(snippet, i);
         RETURN(TRUE);
      }
   }

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      if (!gb_source_snippet_chunk_get_tab_stop(chunk)) {
         gb_source_snippet_select_chunk(snippet, i);
         RETURN(FALSE);
      }
   }

   gtk_text_buffer_get_iter_at_mark(priv->buffer, &iter, priv->mark_end);
   gtk_text_buffer_select_range(priv->buffer, &iter, &iter);
   priv->current_chunk = priv->chunks->len - 1;

   RETURN(FALSE);
}

gboolean
gb_source_snippet_move_previous (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk = NULL;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   priv->tab_stop = MAX(1, priv->tab_stop - 1);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      if (gb_source_snippet_chunk_get_tab_stop(chunk) == priv->tab_stop) {
         gb_source_snippet_select_chunk(snippet, i);
         RETURN(TRUE);
      }
   }

   RETURN(FALSE);
}

static void
gb_source_snippet_update_context (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   const gchar *text;
   gchar key[12];
   guint i;
   gint tab_stop;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   gb_source_snippet_context_emit_changed(priv->context);

   gb_source_snippet_context_clear_variables(priv->context);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      tab_stop = gb_source_snippet_chunk_get_tab_stop(chunk);
      if (tab_stop > 0) {
         if ((text = gb_source_snippet_chunk_get_text(chunk))) {
            g_snprintf(key, sizeof key, "$%d", tab_stop);
            key[sizeof key - 1] = '\0';
            gb_source_snippet_context_add_variable(priv->context, key, text);
         }
      }
   }

   gb_source_snippet_context_emit_changed(priv->context);

   EXIT;
}

void
gb_source_snippet_begin (GbSourceSnippet *snippet,
                         GtkTextBuffer   *buffer,
                         GtkTextIter     *iter)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   const gchar *text;
   gint len;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(!snippet->priv->buffer);
   g_return_if_fail(!snippet->priv->mark_begin);
   g_return_if_fail(!snippet->priv->mark_end);
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(iter);

   priv = snippet->priv;

   priv->inserted = TRUE;

   gb_source_snippet_update_context(snippet);

   priv->buffer = g_object_ref(buffer);
   priv->mark_begin = gtk_text_buffer_create_mark(buffer, NULL, iter, TRUE);

   gtk_text_buffer_begin_user_action(buffer);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      if ((text = gb_source_snippet_chunk_get_text(chunk))) {
         len = g_utf8_strlen(text, -1);
         g_array_append_val(priv->runs, len);
         gtk_text_buffer_insert(buffer, iter, text, -1);
      }
   }

   priv->mark_end = gtk_text_buffer_create_mark(buffer, NULL, iter, FALSE);

   g_object_ref(priv->mark_begin);
   g_object_ref(priv->mark_end);

   gtk_text_buffer_end_user_action(buffer);

   gb_source_snippet_move_next(snippet);

   EXIT;
}

void
gb_source_snippet_finish (GbSourceSnippet *snippet)
{
   ENTRY;
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   EXIT;
}

void
gb_source_snippet_pause (GbSourceSnippet *snippet)
{
   ENTRY;
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   EXIT;
}

void
gb_source_snippet_unpause (GbSourceSnippet *snippet)
{
   ENTRY;
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   EXIT;
}

void
gb_source_snippet_add_chunk (GbSourceSnippet      *snippet,
                             GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetPrivate *priv;
   gint tab_stop;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   g_return_if_fail(!snippet->priv->inserted);

   priv = snippet->priv;

   g_ptr_array_add(priv->chunks, g_object_ref(chunk));

   gb_source_snippet_chunk_set_context(chunk, priv->context);

   tab_stop = gb_source_snippet_chunk_get_tab_stop(chunk);
   priv->max_tab_stop = MAX(priv->max_tab_stop, tab_stop);

   EXIT;
}

gchar *
gb_source_snippet_get_nth_text (GbSourceSnippet *snippet,
                                gint             n)
{
   GbSourceSnippetPrivate *priv;
   GtkTextIter iter;
   GtkTextIter end;
   gchar *ret;
   gint i;

   ENTRY;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);
   g_return_val_if_fail(n >= 0, NULL);

   priv = snippet->priv;

   gtk_text_buffer_get_iter_at_mark(priv->buffer, &iter, priv->mark_begin);

   for (i = 0; i < n; i++) {
      gtk_text_iter_forward_chars(&iter, g_array_index(priv->runs, gint, i));
   }

   gtk_text_iter_assign(&end, &iter);
   gtk_text_iter_forward_chars(&end, g_array_index(priv->runs, gint, n));

   ret = gtk_text_buffer_get_text(priv->buffer, &iter, &end, TRUE);

   RETURN(ret);
}

static void
gb_source_snippet_replace_chunk_text (GbSourceSnippet *snippet,
                                      gint             n,
                                      const gchar     *text)
{
   GbSourceSnippetPrivate *priv;
   GtkTextIter begin;
   GtkTextIter end;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(n >= 0);
   g_return_if_fail(text);

   priv = snippet->priv;

   gb_source_snippet_get_chunk_range(snippet, n, &begin, &end);
   gtk_text_buffer_delete(priv->buffer, &begin, &end);
   gtk_text_buffer_insert(priv->buffer, &end, text, -1);
   g_array_index(priv->runs, gint, n) = g_utf8_strlen(text, -1);

   EXIT;
}

static void
gb_source_snippet_rewrite_updated_chunks (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   const gchar *text;
   gchar *real_text;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      text = gb_source_snippet_chunk_get_text(chunk);
      real_text = gb_source_snippet_get_nth_text(snippet, i);
      if (!!g_strcmp0(text, real_text)) {
         gb_source_snippet_replace_chunk_text(snippet, i, text);
      }
      g_free(real_text);
   }

   EXIT;
}

void
gb_source_snippet_before_insert_text (GbSourceSnippet *snippet,
                                      GtkTextBuffer   *buffer,
                                      GtkTextIter     *iter,
                                      gchar           *text,
                                      gint             len)
{
   GbSourceSnippetPrivate *priv;
   gint utf8_len;
   gint n;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(snippet->priv->current_chunk >= 0);
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(iter);

   priv = snippet->priv;

   n = gb_source_snippet_get_index(snippet, iter);
   utf8_len = g_utf8_strlen(text, len);
   g_array_index(priv->runs, gint, n) += utf8_len;

#if 0
   g_print("I: ");
   for (n = 0; n < priv->runs->len; n++) {
      g_print("%d ", g_array_index(priv->runs, gint, n));
   }
   g_print("\n");
#endif

   EXIT;
}

void
gb_source_snippet_after_insert_text (GbSourceSnippet *snippet,
                                     GtkTextBuffer   *buffer,
                                     GtkTextIter     *iter,
                                     gchar           *text,
                                     gint             len)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GtkTextMark *here;
   gchar *new_text;
   gint n;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(snippet->priv->current_chunk >= 0);
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(iter);

   priv = snippet->priv;

   n = gb_source_snippet_get_index(snippet, iter);
   chunk = g_ptr_array_index(priv->chunks, n);
   new_text = gb_source_snippet_get_nth_text(snippet, n);
   gb_source_snippet_chunk_set_text(chunk, new_text);
   gb_source_snippet_chunk_set_text_set(chunk, TRUE);
   g_free(new_text);

   here = gtk_text_buffer_create_mark(buffer, NULL, iter, TRUE);

   gb_source_snippet_update_context(snippet);
   gb_source_snippet_update_context(snippet);
   gb_source_snippet_rewrite_updated_chunks(snippet);

   gtk_text_buffer_get_iter_at_mark(buffer, iter, here);
   gtk_text_buffer_delete_mark(buffer, here);

#if 0
   gb_source_snippet_context_dump(priv->context);
#endif

   EXIT;
}

void
gb_source_snippet_before_delete_range (GbSourceSnippet *snippet,
                                       GtkTextBuffer   *buffer,
                                       GtkTextIter     *begin,
                                       GtkTextIter     *end)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   gchar *new_text;
   gint *run;
   gint len;
   gint n;
   gint i;
   gint lower_bound = -1;
   gint upper_bound = -1;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(begin);
   g_return_if_fail(end);

   priv = snippet->priv;

   len = gtk_text_iter_get_offset(end) - gtk_text_iter_get_offset(begin);
   n = gb_source_snippet_get_index(snippet, begin);
   priv->current_chunk = n;

   while (len && n < priv->runs->len) {
      if (lower_bound == -1 || n < lower_bound) {
         lower_bound = n;
      }
      if (n > upper_bound) {
         upper_bound = n;
      }
      run = &g_array_index(priv->runs, gint, n);
      if (len > *run) {
         len -= *run;
         *run = 0;
         n++;
         continue;
      }
      *run -= len;
      len = 0;
      break;
   }

   for (i = lower_bound; i <= upper_bound; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      new_text = gb_source_snippet_get_nth_text(snippet, i);
      gb_source_snippet_chunk_set_text(chunk, new_text);
      gb_source_snippet_chunk_set_text_set(chunk, TRUE);
      g_free(new_text);
   }

#if 0
   g_print("D: ");
   for (n = 0; n < priv->runs->len; n++) {
      g_print("%d ", g_array_index(priv->runs, gint, n));
   }
   g_print("\n");
#endif

   EXIT;
}

void
gb_source_snippet_after_delete_range (GbSourceSnippet *snippet,
                                      GtkTextBuffer   *buffer,
                                      GtkTextIter     *begin,
                                      GtkTextIter     *end)
{
   GbSourceSnippetPrivate *priv;
   GtkTextMark *here;

   ENTRY;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(begin);
   g_return_if_fail(end);

   priv = snippet->priv;

   here = gtk_text_buffer_create_mark(buffer, NULL, begin, TRUE);

   gb_source_snippet_update_context(snippet);
   gb_source_snippet_update_context(snippet);
   gb_source_snippet_rewrite_updated_chunks(snippet);

   gtk_text_buffer_get_iter_at_mark(buffer, begin, here);
   gtk_text_buffer_get_iter_at_mark(buffer, end, here);
   gtk_text_buffer_delete_mark(buffer, here);

#if 0
   gb_source_snippet_context_dump(priv->context);
#endif

   EXIT;
}

static void
gb_source_snippet_dispose (GObject *object)
{
   GbSourceSnippetPrivate *priv;

   ENTRY;

   priv = GB_SOURCE_SNIPPET(object)->priv;

   if (priv->mark_begin) {
      gtk_text_buffer_delete_mark(priv->buffer, priv->mark_begin);
      g_clear_object(&priv->mark_begin);
   }

   if (priv->mark_end) {
      gtk_text_buffer_delete_mark(priv->buffer, priv->mark_end);
      g_clear_object(&priv->mark_end);
   }

   g_clear_pointer(&priv->runs, (GDestroyNotify)g_array_unref);
   g_clear_pointer(&priv->chunks, (GDestroyNotify)g_ptr_array_unref);

   g_clear_object(&priv->buffer);

   g_clear_object(&priv->context);

   EXIT;
}

static void
gb_source_snippet_finalize (GObject *object)
{
   ENTRY;
   G_OBJECT_CLASS(gb_source_snippet_parent_class)->finalize(object);
   EXIT;
}

static void
gb_source_snippet_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
   GbSourceSnippet *snippet = GB_SOURCE_SNIPPET(object);

   switch (prop_id) {
   case PROP_BUFFER:
      g_value_set_object(value, snippet->priv->buffer);
      break;
   case PROP_MARK_BEGIN:
      g_value_set_object(value, snippet->priv->mark_begin);
      break;
   case PROP_MARK_END:
      g_value_set_object(value, snippet->priv->mark_end);
      break;
   case PROP_TRIGGER:
      g_value_set_string(value, snippet->priv->trigger);
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_snippet_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
   GbSourceSnippet *snippet = GB_SOURCE_SNIPPET(object);

   switch (prop_id) {
   case PROP_TRIGGER:
      gb_source_snippet_set_trigger(snippet, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_snippet_class_init (GbSourceSnippetClass *klass)
{
   GObjectClass *object_class;

   ENTRY;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_source_snippet_dispose;
   object_class->finalize = gb_source_snippet_finalize;
   object_class->get_property = gb_source_snippet_get_property;
   object_class->set_property = gb_source_snippet_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetPrivate));

   gParamSpecs[PROP_BUFFER] =
      g_param_spec_object("buffer",
                          _("Buffer"),
                          _("The GtkTextBuffer for the snippet."),
                          GTK_TYPE_TEXT_BUFFER,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_BUFFER,
                                   gParamSpecs[PROP_BUFFER]);

   gParamSpecs[PROP_MARK_BEGIN] =
      g_param_spec_object("mark-begin",
                          _("Mark Begin"),
                          _("The beginning text mark."),
                          GTK_TYPE_TEXT_MARK,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MARK_BEGIN,
                                   gParamSpecs[PROP_MARK_BEGIN]);

   gParamSpecs[PROP_MARK_END] =
      g_param_spec_object("mark-end",
                          _("Mark End"),
                          _("The ending text mark."),
                          GTK_TYPE_TEXT_MARK,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MARK_END,
                                   gParamSpecs[PROP_MARK_END]);

   gParamSpecs[PROP_TRIGGER] =
      g_param_spec_string("trigger",
                          _("Trigger"),
                          _("The trigger for the snippet."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TRIGGER,
                                   gParamSpecs[PROP_TRIGGER]);

   EXIT;
}

static void
gb_source_snippet_init (GbSourceSnippet *snippet)
{
   ENTRY;

   snippet->priv = G_TYPE_INSTANCE_GET_PRIVATE(snippet,
                                               GB_TYPE_SOURCE_SNIPPET,
                                               GbSourceSnippetPrivate);

   snippet->priv->tab_stop = 0;
   snippet->priv->max_tab_stop = -1;
   snippet->priv->chunks = g_ptr_array_new_with_free_func(g_object_unref);
   snippet->priv->runs = g_array_new(FALSE, FALSE, sizeof(gint));
   snippet->priv->context = gb_source_snippet_context_new();

   EXIT;
}

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
#include "gb-source-snippet-chunk.h"

G_DEFINE_TYPE(GbSourceSnippet, gb_source_snippet, G_TYPE_OBJECT)

struct _GbSourceSnippetPrivate
{
   GPtrArray   *chunks;
   gchar       *trigger;
   GtkTextMark *mark_begin;
   GtkTextMark *mark_end;
   gint         tab_stop;
   gboolean     updating_chunks;
};

enum
{
   PROP_0,
   PROP_TRIGGER,
   PROP_MARK_BEGIN,
   PROP_MARK_END,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippet *
gb_source_snippet_new (const gchar *trigger)
{
   return g_object_new(GB_TYPE_SOURCE_SNIPPET,
                       "trigger", trigger,
                       NULL);
}

GbSourceSnippet *
gb_source_snippet_copy (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GbSourceSnippet *ret;
   guint i;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);

   priv = snippet->priv;

   ret = g_object_new(GB_TYPE_SOURCE_SNIPPET,
                      "trigger", priv->trigger,
                      NULL);
   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      chunk = gb_source_snippet_chunk_copy(chunk);
      g_ptr_array_add(ret->priv->chunks, chunk);
   }

   return ret;
}

static GbSourceSnippetChunk *
gb_source_snippet_get_chunk_at_tab_stop (GbSourceSnippet *snippet,
                                         guint            tab_stop)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   guint i;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);

   priv = snippet->priv;

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      if (gb_source_snippet_chunk_get_tab_stop(chunk) == tab_stop) {
         return chunk;
      }
   }

   return NULL;
}

static void
gb_source_snippet_update_defaults (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GbSourceSnippetChunk *linked;
   const gchar *str;
   gint linked_to;
   gint i;

   g_assert(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      linked_to = gb_source_snippet_chunk_get_linked_chunk(chunk);
      if (linked_to > 0) {
         linked = gb_source_snippet_get_chunk_at_tab_stop(snippet, linked_to);
         if (linked) {
            str = gb_source_snippet_chunk_get_text(linked);
            gb_source_snippet_chunk_set_text(chunk, str);
         }
      }
   }
}

void
gb_source_snippet_add_chunk (GbSourceSnippet      *snippet,
                             GbSourceSnippetChunk *chunk)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   g_ptr_array_add(snippet->priv->chunks, g_object_ref(chunk));
   gb_source_snippet_update_defaults(snippet);
}

static gint
get_max_tab_stop (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   gint max_tab_stop = -1;
   gint tab_stop;
   gint i;

   priv = snippet->priv;

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      tab_stop = gb_source_snippet_chunk_get_tab_stop(chunk);
      if (tab_stop > max_tab_stop) {
         max_tab_stop = tab_stop;
      }
   }

   return max_tab_stop;
}

gboolean
gb_source_snippet_move_next (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GtkTextBuffer *buffer;
   GtkTextIter iter;
   gint max_tab_stop;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), FALSE);

   priv = snippet->priv;

   max_tab_stop = get_max_tab_stop(snippet);

   if (priv->tab_stop > max_tab_stop) {
move_to_end:
      buffer = gtk_text_mark_get_buffer(priv->mark_begin);
      gtk_text_buffer_get_iter_at_mark(buffer, &iter, priv->mark_end);
      gtk_text_buffer_select_range(buffer, &iter, &iter);
      return FALSE;
   }

   priv->tab_stop++;

   /*
    * If there are no more chunks to tab through, then move to $0 if it was
    * created or place cursor at end of snippet.
    */
   if (!(chunk = gb_source_snippet_get_chunk_at_tab_stop(snippet, priv->tab_stop))) {
      if ((chunk = gb_source_snippet_get_chunk_at_tab_stop(snippet, 0))) {
         gb_source_snippet_chunk_select(chunk);
         return FALSE;
      }
      goto move_to_end;
   }

   /*
    * Select the next chunk.
    */
   gb_source_snippet_chunk_select(chunk);

   /*
    * Allow ourselves to be called once more after our last tabstop so that
    * we can try to place the cursor on $0.
    */
   return (priv->tab_stop <= max_tab_stop);
}

gboolean
gb_source_snippet_move_previous (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GtkTextBuffer *buffer;
   GtkTextIter iter;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), FALSE);

   priv = snippet->priv;

   if (--priv->tab_stop < 0) {
      buffer = gtk_text_mark_get_buffer(priv->mark_begin);
      gtk_text_buffer_get_iter_at_mark(buffer, &iter, priv->mark_begin);
      gtk_text_buffer_select_range(buffer, &iter, &iter);
      return FALSE;
   }

   if (!(chunk = gb_source_snippet_get_chunk_at_tab_stop(snippet, priv->tab_stop))) {
      return FALSE;
   }

   gb_source_snippet_chunk_select(chunk);

   return TRUE;
}

static gchar *
get_line_prefix (GtkTextBuffer *buffer,
                 GtkTextIter   *iter)
{
   GtkTextIter begin;
   GString *str;
   gunichar c;

   g_assert(GTK_IS_TEXT_BUFFER(buffer));
   g_assert(iter);

   gtk_text_iter_assign(&begin, iter);
   gtk_text_iter_set_line_offset(&begin, 0);

   str = g_string_new(NULL);

   while (gtk_text_iter_compare(&begin, iter) < 0) {
      c = gtk_text_iter_get_char(&begin);
      if (c == '\t' || c == ' ') {
         g_string_append_unichar(str, c);
      } else {
         g_string_append_c(str, ' ');
      }
      if (!gtk_text_iter_forward_char(&begin)) {
         break;
      }
   }

   return g_string_free(str, FALSE);
}

void
gb_source_snippet_insert (GbSourceSnippet *snippet,
                          GtkTextBuffer   *buffer,
                          GtkTextIter     *location,
                          guint            tab_size,
                          gboolean         use_spaces)

{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   gchar *line_prefix;
   guint i;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(location);

   priv = snippet->priv;

   if (priv->mark_begin || priv->mark_end) {
      g_warning("Cannot overwrite previous snippet position.");
      return;
   }

   line_prefix = get_line_prefix(buffer, location);

   priv->mark_begin = gtk_text_buffer_create_mark(buffer, NULL, location, TRUE);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      gb_source_snippet_chunk_insert(chunk, buffer, location,
                                     line_prefix, tab_size, use_spaces);
   }

   priv->mark_end = gtk_text_buffer_create_mark(buffer, NULL, location, FALSE);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      gb_source_snippet_chunk_build_marks(chunk, buffer);
   }

   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_MARK_BEGIN]);
   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_MARK_END]);
}

void
gb_source_snippet_finish (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GtkTextBuffer *buffer;
   guint i;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   if (!priv->mark_begin || !priv->mark_end) {
      g_warning("Cannot finish snippet as it has not been inserted.");
      return;
   }

   buffer = gtk_text_mark_get_buffer(priv->mark_begin);

   /*
    * TODO: Remove all our GtkTextTag in the range.
    */

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      gb_source_snippet_chunk_finish(chunk);
   }

   gtk_text_buffer_delete_mark(buffer, priv->mark_begin);
   gtk_text_buffer_delete_mark(buffer, priv->mark_end);
   g_clear_object(&priv->mark_begin);
   g_clear_object(&priv->mark_end);

   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_MARK_BEGIN]);
   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_MARK_END]);
}

void
gb_source_snippet_remove (GbSourceSnippet *snippet)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;
   guint i;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   if (!priv->mark_begin || !priv->mark_end) {
      g_warning("Cannot remove snippet as it has not been inserted.");
      return;
   }

   buffer = gtk_text_mark_get_buffer(priv->mark_begin);

   /*
    * TODO: Remove all our GtkTextTag in the range.
    */

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      gb_source_snippet_chunk_remove(chunk);
   }

   gtk_text_buffer_get_iter_at_mark(buffer, &begin, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, priv->mark_end);
   gtk_text_buffer_delete(buffer, &begin, &end);
   gtk_text_buffer_delete_mark(buffer, priv->mark_begin);
   gtk_text_buffer_delete_mark(buffer, priv->mark_end);
   g_clear_object(&priv->mark_begin);
   g_clear_object(&priv->mark_end);

   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_MARK_BEGIN]);
   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_MARK_END]);
}

void
gb_source_snippet_insert_text (GbSourceSnippet *snippet,
                               GtkTextBuffer   *buffer,
                               GtkTextIter     *location,
                               const gchar     *text,
                               guint            length)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GbSourceSnippetChunk *linked;
   GtkTextMark *here;
   guint i;
   gint linked_to;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(location);
   g_return_if_fail(text);

   priv = snippet->priv;

   if (priv->updating_chunks) {
      return;
   }

   priv->updating_chunks = TRUE;

   here = gtk_text_buffer_create_mark(buffer, NULL, location, TRUE);

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      if (gb_source_snippet_chunk_contains(chunk, buffer, location)) {
         gb_source_snippet_chunk_set_modified(chunk, TRUE);
      } else {
         linked_to = gb_source_snippet_chunk_get_linked_chunk(chunk);
         if (linked_to != -1) {
            linked = gb_source_snippet_get_chunk_at_tab_stop(snippet, linked_to);
            if (linked) {
               gb_source_snippet_chunk_update(chunk, linked, buffer);
               gtk_text_buffer_get_iter_at_mark(buffer, location, here);
            }
         }
      }
   }

   gtk_text_buffer_get_iter_at_mark(buffer, location, here);
   gtk_text_buffer_delete_mark(buffer, here);

   priv->updating_chunks = FALSE;
}

void
gb_source_snippet_delete_range (GbSourceSnippet *snippet,
                                GtkTextBuffer   *buffer,
                                GtkTextIter     *begin,
                                GtkTextIter     *end)
{
#if 0
   g_print("Text deleted from snippet.\n");
#endif
}

void
gb_source_snippet_draw (GbSourceSnippet *snippet,
                        GtkWidget       *widget,
                        cairo_t         *cr)
{
   GbSourceSnippetPrivate *priv;
   GbSourceSnippetChunk *chunk;
   guint i;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = snippet->priv;

   for (i = 0; i < priv->chunks->len; i++) {
      chunk = g_ptr_array_index(priv->chunks, i);
      gb_source_snippet_chunk_draw(chunk, widget, cr);
   }
}

const gchar *
gb_source_snippet_get_trigger (GbSourceSnippet *snippet)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);
   return snippet->priv->trigger;
}

void
gb_source_snippet_set_trigger(GbSourceSnippet *snippet,
                              const gchar     *trigger)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   g_free(snippet->priv->trigger);
   snippet->priv->trigger = g_strdup(trigger);
   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_TRIGGER]);
}

GtkTextMark *
gb_source_snippet_get_mark_begin (GbSourceSnippet *snippet)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);
   return snippet->priv->mark_begin;
}

GtkTextMark *
gb_source_snippet_get_mark_end (GbSourceSnippet *snippet)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);
   return snippet->priv->mark_end;
}

static void
gb_source_snippet_dispose (GObject *object)
{
   GbSourceSnippetPrivate *priv = GB_SOURCE_SNIPPET(object)->priv;

   g_clear_pointer(&priv->chunks, (GDestroyNotify)g_ptr_array_unref);

   G_OBJECT_CLASS(gb_source_snippet_parent_class)->dispose(object);
}

static void
gb_source_snippet_finalize (GObject *object)
{
   GbSourceSnippetPrivate *priv;

   priv = GB_SOURCE_SNIPPET(object)->priv;

   g_clear_pointer(&priv->trigger, g_free);
   g_clear_pointer(&priv->chunks, (GDestroyNotify)g_ptr_array_unref);

   G_OBJECT_CLASS(gb_source_snippet_parent_class)->finalize(object);
}

static void
gb_source_snippet_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
   GbSourceSnippet *snippet = GB_SOURCE_SNIPPET(object);

   switch (prop_id) {
   case PROP_TRIGGER:
      g_value_set_string(value, gb_source_snippet_get_trigger(snippet));
      break;
   case PROP_MARK_BEGIN:
      g_value_set_object(value, gb_source_snippet_get_mark_begin(snippet));
      break;
   case PROP_MARK_END:
      g_value_set_object(value, gb_source_snippet_get_mark_end(snippet));
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

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_source_snippet_dispose;
   object_class->finalize = gb_source_snippet_finalize;
   object_class->get_property = gb_source_snippet_get_property;
   object_class->set_property = gb_source_snippet_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetPrivate));

   gParamSpecs[PROP_TRIGGER] =
      g_param_spec_string("trigger",
                          _("Trigger"),
                          _("The trigger of the snippet."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TRIGGER,
                                   gParamSpecs[PROP_TRIGGER]);

   gParamSpecs[PROP_MARK_BEGIN] =
      g_param_spec_object("mark-begin",
                          _("Mark Begin"),
                          _("The beginning of the snippet region."),
                          GTK_TYPE_TEXT_MARK,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MARK_BEGIN,
                                   gParamSpecs[PROP_MARK_BEGIN]);

   gParamSpecs[PROP_MARK_END] =
      g_param_spec_object("mark-end",
                          _("Mark End"),
                          _("The endning of the snippet region."),
                          GTK_TYPE_TEXT_MARK,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MARK_END,
                                   gParamSpecs[PROP_MARK_END]);
}

static void
gb_source_snippet_init (GbSourceSnippet *snippet)
{
   snippet->priv = G_TYPE_INSTANCE_GET_PRIVATE(snippet,
                                               GB_TYPE_SOURCE_SNIPPET,
                                               GbSourceSnippetPrivate);
   snippet->priv->tab_stop = 0;
   snippet->priv->chunks = g_ptr_array_new();
   g_ptr_array_set_free_func(snippet->priv->chunks, g_object_unref);
}

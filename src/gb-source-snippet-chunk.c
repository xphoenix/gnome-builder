/* gb-source-snippet-chunk.c
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

#include "gb-source-snippet-chunk.h"

G_DEFINE_TYPE(GbSourceSnippetChunk, gb_source_snippet_chunk, G_TYPE_OBJECT)

struct _GbSourceSnippetChunkPrivate
{
   guint        linked_chunk;
   guint        offset_begin;
   guint        offset_end;
   GtkTextMark *mark_begin;
   GtkTextMark *mark_end;
   gint         tab_stop;
   gchar       *text;
   gboolean     modified;
};

enum
{
   PROP_0,
   PROP_LINKED_CHUNK,
   PROP_MARK_BEGIN,
   PROP_MARK_END,
   PROP_MODIFIED,
   PROP_TAB_STOP,
   PROP_TEXT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippetChunk *
gb_source_snippet_chunk_new (void)
{
   return g_object_new(GB_TYPE_SOURCE_SNIPPET_CHUNK,
                       NULL);
}

void
gb_source_snippet_chunk_draw (GbSourceSnippetChunk *chunk,
                              GtkWidget            *widget,
                              cairo_t              *cr)
{
   GbSourceSnippetChunkPrivate *priv = chunk->priv;
   GtkStyleContext *style;
   GtkTextBuffer *buffer;
   GdkRectangle rect;
   GdkRectangle rect2;
   GtkTextView *view;
   GtkTextIter iter;
   GtkTextIter end;
   GdkRGBA color;

   if (priv->tab_stop < 0) {
      return;
   }

   cairo_save(cr);

   view = GTK_TEXT_VIEW(widget);
   buffer = gtk_text_view_get_buffer(view);

   gtk_text_buffer_get_iter_at_mark(buffer, &iter, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, priv->mark_end);

   /*
    * TODO: We should probably use cairo_region_t here so we handle wrapping.
    */

   gtk_text_view_get_iter_location(view, &iter, &rect);
   gtk_text_view_buffer_to_window_coords(view,
                                         GTK_TEXT_WINDOW_WIDGET,
                                         rect.x,
                                         rect.y,
                                         &rect.x,
                                         &rect.y);

   gtk_text_view_get_iter_location(view, &end, &rect2);
   gtk_text_view_buffer_to_window_coords(view,
                                         GTK_TEXT_WINDOW_WIDGET,
                                         rect2.x,
                                         rect2.y,
                                         &rect2.x,
                                         &rect2.y);

   rect.width = rect2.x - rect.x;
   rect.height -= 1;

   cairo_translate(cr, 0.5, 0.5);
   style = gtk_widget_get_style_context(widget);
   gtk_style_context_get_color(style, GTK_STATE_FLAG_INSENSITIVE, &color);
   color.alpha = 0.7;
   gdk_cairo_set_source_rgba(cr, &color);
   cairo_set_line_width(cr, 1);

   if (priv->linked_chunk != -1) {
      const gdouble dashes[] = { 2.0 };
      cairo_set_dash(cr, dashes, 1, 0);
   }

   gdk_cairo_rectangle(cr, &rect);

   cairo_stroke(cr);

   cairo_restore(cr);
}

gboolean
gb_source_snippet_chunk_contains (GbSourceSnippetChunk *chunk,
                                  GtkTextBuffer        *buffer,
                                  GtkTextIter          *location)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextIter begin;
   GtkTextIter end;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), FALSE);
   g_return_val_if_fail(location, FALSE);

   priv = chunk->priv;

   if (!priv->mark_begin || !priv->mark_end) {
      return FALSE;
   }

   gtk_text_buffer_get_iter_at_mark(buffer, &begin, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, priv->mark_end);

   if ((gtk_text_iter_compare(location, &begin) >= 0) &&
       (gtk_text_iter_compare(location, &end) <= 0)) {
      return TRUE;
   }

   return FALSE;
}

void
gb_source_snippet_chunk_set_modified (GbSourceSnippetChunk *chunk,
                                      gboolean              modified)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   chunk->priv->modified = modified;
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_MODIFIED]);
}

void
gb_source_snippet_chunk_update (GbSourceSnippetChunk *chunk,
                                GbSourceSnippetChunk *linked,
                                GtkTextBuffer        *buffer)
{
   GtkTextIter begin;
   GtkTextIter end;
   gchar *our_text;
   gchar *text;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(linked));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));

   if (chunk->priv->modified) {
      return;
   }

   gtk_text_buffer_get_iter_at_mark(buffer, &begin, linked->priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, linked->priv->mark_end);
   text = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);

   gtk_text_buffer_get_iter_at_mark(buffer, &begin, chunk->priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, chunk->priv->mark_end);
   our_text = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);

   if (0 != g_strcmp0(text, our_text)) {
      gtk_text_buffer_delete(buffer, &begin, &end);
      gtk_text_buffer_insert(buffer, &begin, text, -1);
   }

   g_free(text);
   g_free(our_text);
}

GbSourceSnippetChunk *
gb_source_snippet_chunk_copy (GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetChunk *ret;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), NULL);

   ret = g_object_new(GB_TYPE_SOURCE_SNIPPET_CHUNK,
                      "linked-chunk", chunk->priv->linked_chunk,
                      "tab-stop", chunk->priv->tab_stop,
                      "text", chunk->priv->text,
                      NULL);

   return ret;
}

gboolean
gb_source_snippet_chunk_get_modified (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), FALSE);
   return chunk->priv->modified;
}

gint
gb_source_snippet_chunk_get_linked_chunk (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), 0);
   return chunk->priv->linked_chunk;
}

void
gb_source_snippet_chunk_set_linked_chunk (GbSourceSnippetChunk *chunk,
                                          guint                 linked_chunk)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   chunk->priv->linked_chunk = linked_chunk;
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_LINKED_CHUNK]);
}

const gchar *
gb_source_snippet_chunk_get_text (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), NULL);
   return chunk->priv->text;
}

void
gb_source_snippet_chunk_set_text (GbSourceSnippetChunk *chunk,
                                  const gchar          *text)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   g_free(chunk->priv->text);
   chunk->priv->text = g_strdup(text);
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_TEXT]);
}

GtkTextMark *
gb_source_snippet_chunk_get_mark_begin (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), NULL);
   return chunk->priv->mark_begin;
}

GtkTextMark *
gb_source_snippet_chunk_get_mark_end (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), NULL);
   return chunk->priv->mark_end;
}

void
gb_source_snippet_chunk_finish (GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextBuffer *buffer;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   priv = chunk->priv;

   buffer = gtk_text_mark_get_buffer(priv->mark_begin);
   gtk_text_buffer_delete_mark(buffer, priv->mark_begin);
   gtk_text_buffer_delete_mark(buffer, priv->mark_end);
   g_clear_object(&priv->mark_begin);
   g_clear_object(&priv->mark_end);
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_MARK_BEGIN]);
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_MARK_END]);
}

void
gb_source_snippet_chunk_remove (GbSourceSnippetChunk *chunk)
{
   gb_source_snippet_chunk_finish(chunk);
}

void
gb_source_snippet_chunk_select (GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   priv = chunk->priv;

   buffer = gtk_text_mark_get_buffer(priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &begin, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, priv->mark_end);
   gtk_text_buffer_select_range(buffer, &begin, &end);
}

void
gb_source_snippet_chunk_insert (GbSourceSnippetChunk *chunk,
                                GtkTextBuffer        *buffer,
                                GtkTextIter          *location,
                                const gchar          *line_prefix,
                                guint                 tab_size,
                                gboolean              use_spaces)
{
   GbSourceSnippetChunkPrivate *priv;
   const gchar *text;
   gunichar c;
   GString *str;
   guint i;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
   g_return_if_fail(location);

   priv = chunk->priv;

   if (priv->mark_begin || priv->mark_end) {
      g_warning("Chunk has already been inserted, ignoring.");
      return;
   }

   priv->offset_begin = gtk_text_iter_get_offset(location);

   if ((text = priv->text)) {
      str = g_string_new(NULL);

      for (text = priv->text;
           (c = g_utf8_get_char(text));
           text = g_utf8_next_char(text)) {
         switch (c) {
         case '\t':
            if (use_spaces) {
               for (i = 0; i < tab_size; i++) {
                  g_string_append_c(str, ' ');
               }
            } else {
               g_string_append_c(str, '\t');
            }
            break;
         case '\n':
            g_string_append_c(str, '\n');
            g_string_append(str, line_prefix);
            break;
         default:
            g_string_append_unichar(str, c);
            break;
         }
      }

      gtk_text_buffer_insert(buffer, location, str->str, str->len);

      g_string_free(str, TRUE);
   }

   priv->offset_end = gtk_text_iter_get_offset(location);
}

void
gb_source_snippet_chunk_build_marks (GbSourceSnippetChunk *chunk,
                                     GtkTextBuffer        *buffer)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextIter iter;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   priv = chunk->priv;

   if (priv->offset_begin) {
      gtk_text_buffer_get_iter_at_offset(buffer, &iter, priv->offset_begin);
      priv->mark_begin = gtk_text_buffer_create_mark(buffer, NULL, &iter, TRUE);
      g_object_ref(priv->mark_begin);
   }

   if (priv->offset_end) {
      gtk_text_buffer_get_iter_at_offset(buffer, &iter, priv->offset_end);
      priv->mark_end = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);
      g_object_ref(priv->mark_end);
   }

   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_MARK_BEGIN]);
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_MARK_END]);
}

gint
gb_source_snippet_chunk_get_tab_stop (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), -1);
   return chunk->priv->tab_stop;
}

void
gb_source_snippet_chunk_set_tab_stop (GbSourceSnippetChunk *chunk,
                                      gint                  tab_stop)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   chunk->priv->tab_stop = tab_stop;
   g_object_notify_by_pspec(G_OBJECT(chunk), gParamSpecs[PROP_TAB_STOP]);
}

static void
gb_source_snippet_chunk_finalize (GObject *object)
{
   GbSourceSnippetChunkPrivate *priv;

   priv = GB_SOURCE_SNIPPET_CHUNK(object)->priv;

   g_clear_object(&priv->mark_begin);
   g_clear_object(&priv->mark_end);
   g_clear_pointer(&priv->text, g_free);

   G_OBJECT_CLASS(gb_source_snippet_chunk_parent_class)->finalize(object);
}

static void
gb_source_snippet_chunk_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
   GbSourceSnippetChunk *chunk = GB_SOURCE_SNIPPET_CHUNK(object);

   switch (prop_id) {
   case PROP_LINKED_CHUNK:
      g_value_set_int(value,
                      gb_source_snippet_chunk_get_linked_chunk(chunk));
      break;
   case PROP_MARK_BEGIN:
      g_value_set_object(value,
                         gb_source_snippet_chunk_get_mark_begin(chunk));
      break;
   case PROP_MARK_END:
      g_value_set_object(value,
                         gb_source_snippet_chunk_get_mark_end(chunk));
      break;
   case PROP_MODIFIED:
      g_value_set_boolean(value,
                          gb_source_snippet_chunk_get_modified(chunk));
      break;
   case PROP_TAB_STOP:
      g_value_set_int(value, gb_source_snippet_chunk_get_tab_stop(chunk));
      break;
   case PROP_TEXT:
      g_value_set_string(value, gb_source_snippet_chunk_get_text(chunk));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_snippet_chunk_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
   GbSourceSnippetChunk *chunk = GB_SOURCE_SNIPPET_CHUNK(object);

   switch (prop_id) {
   case PROP_LINKED_CHUNK:
      gb_source_snippet_chunk_set_linked_chunk(chunk,
                                                    g_value_get_int(value));
      break;
   case PROP_MODIFIED:
      gb_source_snippet_chunk_set_modified(chunk,
                                                g_value_get_boolean(value));
      break;
   case PROP_TAB_STOP:
      gb_source_snippet_chunk_set_tab_stop(chunk, g_value_get_int(value));
      break;
   case PROP_TEXT:
      gb_source_snippet_chunk_set_text(chunk, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_snippet_chunk_class_init (GbSourceSnippetChunkClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_snippet_chunk_finalize;
   object_class->get_property = gb_source_snippet_chunk_get_property;
   object_class->set_property = gb_source_snippet_chunk_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetChunkPrivate));

   gParamSpecs[PROP_LINKED_CHUNK] =
      g_param_spec_int("linked-chunk",
                       _("Linked Chunk"),
                       _("A chunk index this is linked to."),
                       -1,
                       G_MAXINT,
                       -1,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_LINKED_CHUNK,
                                   gParamSpecs[PROP_LINKED_CHUNK]);

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

   gParamSpecs[PROP_MODIFIED] =
      g_param_spec_boolean("modified",
                           _("Modified"),
                           _("If the chunk has been modified directly."),
                           FALSE,
                           (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MODIFIED,
                                   gParamSpecs[PROP_MODIFIED]);

   gParamSpecs[PROP_TAB_STOP] =
      g_param_spec_int("tab-stop",
                       _("Tab Stop"),
                       _("The tab stop in the snippet params."),
                       -1,
                       G_MAXINT,
                       -1,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TAB_STOP,
                                   gParamSpecs[PROP_TAB_STOP]);

   gParamSpecs[PROP_TEXT] =
      g_param_spec_string("text",
                          _("Text"),
                          _("The text to render."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TEXT,
                                   gParamSpecs[PROP_TEXT]);
}

static void
gb_source_snippet_chunk_init (GbSourceSnippetChunk *chunk)
{
   chunk->priv = G_TYPE_INSTANCE_GET_PRIVATE(chunk,
                                             GB_TYPE_SOURCE_SNIPPET_CHUNK,
                                             GbSourceSnippetChunkPrivate);
   chunk->priv->linked_chunk = -1;
   chunk->priv->tab_stop = -1;
}

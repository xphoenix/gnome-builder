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
   GtkTextMark *mark_begin;
   GtkTextMark *mark_end;

   gint         tab_stop;

   gchar       *spec;
   gchar       *text;
   gboolean     text_set;

   guint        offset_begin;
   guint        offset_end;
};

enum
{
   PROP_0,
   PROP_SPEC,
   PROP_TAB_STOP,
   PROP_TEXT,
   PROP_TEXT_SET,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippetChunk *
gb_source_snippet_chunk_new (void)
{
   g_object_new(GB_TYPE_SOURCE_SNIPPET_CHUNK, NULL);
}

GbSourceSnippetChunk *
gb_source_snippet_chunk_copy (GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetChunkPrivate *priv;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), NULL);

   priv = chunk->priv;

   return g_object_new(GB_TYPE_SOURCE_SNIPPET_CHUNK,
                       "tab-stop", priv->tab_stop,
                       "spec", priv->spec,
                       NULL);
}

const gchar *
gb_source_snippet_chunk_get_spec (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), NULL);
   return chunk->priv->spec;
}

void
gb_source_snippet_chunk_set_spec (GbSourceSnippetChunk *chunk,
                                  const gchar          *spec)
{
   GbSourceSnippetChunkPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   priv = chunk->priv;

   g_free(priv->spec);
   priv->spec = g_strdup(spec);

   if (!priv->text) {
      priv->text = g_strdup(spec);
   }
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
   g_return_if_fail(text);

   g_free(chunk->priv->text);
   chunk->priv->text = g_strdup(text);
   chunk->priv->text_set = TRUE;
}

gboolean
gb_source_snippet_chunk_get_text_set (GbSourceSnippetChunk *chunk)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), FALSE);
   return chunk->priv->text_set;
}

void
gb_source_snippet_chunk_edited (GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;
   gchar *text;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   priv = chunk->priv;

   g_assert(priv->mark_begin);
   g_assert(priv->mark_end);

   buffer = gtk_text_mark_get_buffer(priv->mark_begin);

   gtk_text_buffer_get_iter_at_mark(buffer, &begin, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, priv->mark_end);

   g_free(priv->text);
   priv->text = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);
   priv->text_set = TRUE;
}

void
gb_source_snippet_chunk_expand (GbSourceSnippetChunk   *chunk,
                                GbSourceSnippetContext *context)
{
   GbSourceSnippetChunkPrivate *priv;
   gchar *text;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CONTEXT(context));

   priv = chunk->priv;

   if (priv->text_set) {
      return;
   }

   if (priv->spec) {
      text = gb_source_snippet_context_expand(context, priv->spec);
      g_free(priv->text);
      priv->text = text;
   }
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
                                GtkTextIter          *location)
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

   gtk_text_buffer_insert(buffer, location, priv->text, -1);

   priv->offset_end = gtk_text_iter_get_offset(location);
}

gboolean
gb_source_snippet_chunk_contains (GbSourceSnippetChunk *chunk,
                                  const GtkTextIter    *location)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk), FALSE);
   g_return_val_if_fail(location, FALSE);

   priv = chunk->priv;

   buffer = gtk_text_iter_get_buffer(location);

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
}

void
gb_source_snippet_chunk_rebuild (GbSourceSnippetChunk *chunk)
{
   GbSourceSnippetChunkPrivate *priv;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;
   gint offset;
   gint len;

   g_return_if_fail(GB_IS_SOURCE_SNIPPET_CHUNK(chunk));

   priv = chunk->priv;

   if (!priv->text) {
      return;
   }

   buffer = gtk_text_mark_get_buffer(priv->mark_begin);

   gtk_text_buffer_get_iter_at_mark(buffer, &begin, priv->mark_begin);
   gtk_text_buffer_get_iter_at_mark(buffer, &end, priv->mark_end);

   offset = gtk_text_iter_get_offset(&begin);
   len = g_utf8_strlen(priv->text, -1);

   priv->offset_begin = offset;
   priv->offset_end = offset + len;

   gtk_text_buffer_delete_mark(buffer, priv->mark_begin);
   g_clear_object(&priv->mark_begin);

   gtk_text_buffer_delete_mark(buffer, priv->mark_end);
   g_clear_object(&priv->mark_end);

   gtk_text_buffer_delete(buffer, &begin, &end);

   gtk_text_buffer_get_iter_at_offset(buffer, &begin, offset);
   gtk_text_buffer_insert(buffer, &begin, priv->text, -1);

   g_print("Inserted %s\n", priv->text);

   gtk_text_buffer_get_iter_at_offset(buffer, &begin, offset);
   gtk_text_buffer_get_iter_at_offset(buffer, &end, offset + len);

   priv->mark_begin = gtk_text_buffer_create_mark(buffer, NULL, &begin, TRUE);
   g_object_ref(priv->mark_begin);

   priv->mark_end = gtk_text_buffer_create_mark(buffer, NULL, &end, FALSE);
   g_object_ref(priv->mark_end);
}

static void
gb_source_snippet_chunk_finalize (GObject *object)
{
   GbSourceSnippetChunkPrivate *priv;

   priv = GB_SOURCE_SNIPPET_CHUNK(object)->priv;

   g_free(priv->spec);
   g_free(priv->text);

   g_clear_object(&priv->mark_begin);
   g_clear_object(&priv->mark_end);

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
   case PROP_SPEC:
      g_value_set_string(value, gb_source_snippet_chunk_get_spec(chunk));
      break;
   case PROP_TAB_STOP:
      g_value_set_int(value, gb_source_snippet_chunk_get_tab_stop(chunk));
      break;
   case PROP_TEXT:
      g_value_set_string(value, gb_source_snippet_chunk_get_text(chunk));
      break;
   case PROP_TEXT_SET:
      g_value_set_boolean(value, gb_source_snippet_chunk_get_text_set(chunk));
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
   case PROP_SPEC:
      gb_source_snippet_chunk_set_spec(chunk, g_value_get_string(value));
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

   gParamSpecs[PROP_SPEC] =
      g_param_spec_string("spec",
                          _("Spec"),
                          _("The input spec for the chunk such as \"$1|upper\"."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SPEC,
                                   gParamSpecs[PROP_SPEC]);

   gParamSpecs[PROP_TAB_STOP] =
      g_param_spec_int("tab-stop",
                       _("Tab Stop"),
                       _("The tab stop for the snippet."),
                       -1,
                       G_MAXINT,
                       -1,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TAB_STOP,
                                   gParamSpecs[PROP_TAB_STOP]);

   gParamSpecs[PROP_TEXT] =
      g_param_spec_string("text",
                          _("Text"),
                          _("The text for the chunk."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TEXT,
                                   gParamSpecs[PROP_TEXT]);

   gParamSpecs[PROP_TEXT_SET] =
      g_param_spec_boolean("text-set",
                          _("Text Set"),
                          _("If the \"text\" property has been set."),
                          FALSE,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TEXT_SET,
                                   gParamSpecs[PROP_TEXT_SET]);
}

static void
gb_source_snippet_chunk_init (GbSourceSnippetChunk *chunk)
{
   chunk->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(chunk,
                                  GB_TYPE_SOURCE_SNIPPET_CHUNK,
                                  GbSourceSnippetChunkPrivate);

   chunk->priv->tab_stop = -1;
}

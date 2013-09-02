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
   gint   tab_stop;
   gchar *text;
};

enum
{
   PROP_0,
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

static void
gb_source_snippet_chunk_finalize (GObject *object)
{
   GbSourceSnippetChunkPrivate *priv = GB_SOURCE_SNIPPET_CHUNK(object)->priv;

   g_free(priv->text);

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

   gParamSpecs[PROP_TAB_STOP] =
      g_param_spec_int("tab-stop",
                       _("Tab Stop"),
                       _("The tab stop when moving through parameters."),
                       -1,
                       G_MAXINT,
                       -1,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
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
}

static void
gb_source_snippet_chunk_init (GbSourceSnippetChunk *chunk)
{
   chunk->priv = G_TYPE_INSTANCE_GET_PRIVATE(chunk,
                                             GB_TYPE_SOURCE_SNIPPET_CHUNK,
                                             GbSourceSnippetChunkPrivate);

   chunk->priv->tab_stop = -1;
}

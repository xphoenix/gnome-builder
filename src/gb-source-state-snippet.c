/* gb-source-state-snippet.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gb-source-snippet.h"
#include "gb-source-state-snippet.h"

G_DEFINE_TYPE(GbSourceStateSnippet,
              gb_source_state_snippet,
              GB_TYPE_SOURCE_STATE)

struct _GbSourceStateSnippetPrivate
{
   GbSourceSnippet *snippet;
};

enum
{
   PROP_0,
   PROP_SNIPPET,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippet *
gb_source_state_snippet_get_snippet (GbSourceStateSnippet *snippet)
{
   g_return_val_if_fail(GB_IS_SOURCE_STATE_SNIPPET(snippet), NULL);
   return snippet->priv->snippet;
}

void
gb_source_state_snippet_set_snippet (GbSourceStateSnippet *state,
                                     GbSourceSnippet      *snippet)
{
   g_return_if_fail(GB_IS_SOURCE_STATE_SNIPPET(snippet));
   g_return_if_fail(!snippet || GB_IS_SOURCE_SNIPPET(snippet));

   g_clear_object(&state->priv->snippet);
   state->priv->snippet = snippet ? g_object_ref(snippet) : NULL;
   g_object_notify_by_pspec(G_OBJECT(state), gParamSpecs[PROP_SNIPPET]);
}

static void
gb_source_state_snippet_finalize (GObject *object)
{
   GbSourceStateSnippetPrivate *priv;

   priv = GB_SOURCE_STATE_SNIPPET(object)->priv;

   g_clear_object(&priv->snippet);

   G_OBJECT_CLASS(gb_source_state_snippet_parent_class)->finalize(object);
}

static void
gb_source_state_snippet_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
   GbSourceStateSnippet *snippet = GB_SOURCE_STATE_SNIPPET(object);

   switch (prop_id) {
   case PROP_SNIPPET:
      g_value_set_object(value, gb_source_state_snippet_get_snippet(snippet));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_state_snippet_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
   GbSourceStateSnippet *snippet = GB_SOURCE_STATE_SNIPPET(object);

   switch (prop_id) {
   case PROP_SNIPPET:
      gb_source_state_snippet_set_snippet(snippet, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_state_snippet_class_init (GbSourceStateSnippetClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_state_snippet_finalize;
   object_class->get_property = gb_source_state_snippet_get_property;
   object_class->set_property = gb_source_state_snippet_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceStateSnippetPrivate));

   gParamSpecs[PROP_SNIPPET] =
      g_param_spec_object("snippet",
                          _("Snippet"),
                          _("The snippet being inserted into the view."),
                          GB_TYPE_SOURCE_SNIPPET,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SNIPPET,
                                   gParamSpecs[PROP_SNIPPET]);
}

static void
gb_source_state_snippet_init (GbSourceStateSnippet *snippet)
{
   snippet->priv = G_TYPE_INSTANCE_GET_PRIVATE(snippet,
                                               GB_TYPE_SOURCE_STATE_SNIPPET,
                                               GbSourceStateSnippetPrivate);
}

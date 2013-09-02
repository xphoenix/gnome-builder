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

G_DEFINE_TYPE(GbSourceSnippet, gb_source_snippet, G_TYPE_OBJECT)

struct _GbSourceSnippetPrivate
{
   gchar *trigger;
};

enum
{
   PROP_0,
   PROP_TRIGGER,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_source_snippet_get_trigger (GbSourceSnippet *snippet)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET(snippet), NULL);
   return snippet->priv->trigger;
}

void
gb_source_snippet_set_trigger (GbSourceSnippet *snippet,
                               const gchar     *trigger)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   g_free(snippet->priv->trigger);
   snippet->priv->trigger = g_strdup(trigger);
   g_object_notify_by_pspec(G_OBJECT(snippet), gParamSpecs[PROP_TRIGGER]);
}

static void
gb_source_snippet_finalize (GObject *object)
{
   GbSourceSnippetPrivate *priv = GB_SOURCE_SNIPPET(object)->priv;

   g_clear_pointer(&priv->trigger, g_free);

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
   object_class->finalize = gb_source_snippet_finalize;
   object_class->get_property = gb_source_snippet_get_property;
   object_class->set_property = gb_source_snippet_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetPrivate));

   gParamSpecs[PROP_TRIGGER] =
      g_param_spec_string("trigger",
                          _("Trigger"),
                          _("The trigger for the snippet."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TRIGGER,
                                   gParamSpecs[PROP_TRIGGER]);
}

static void
gb_source_snippet_init (GbSourceSnippet *snippet)
{
   snippet->priv = G_TYPE_INSTANCE_GET_PRIVATE(snippet,
                                               GB_TYPE_SOURCE_SNIPPET,
                                               GbSourceSnippetPrivate);
}

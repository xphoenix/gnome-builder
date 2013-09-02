/* gb-source-snippet-completion-provider.c
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
#include <gtksourceview/gtksourcecompletionitem.h>

#include "gb-source-snippet-completion-provider.h"

static void init_provider (GtkSourceCompletionProviderIface *iface);

G_DEFINE_TYPE_EXTENDED(GbSourceSnippetCompletionProvider,
                       gb_source_snippet_completion_provider,
                       G_TYPE_OBJECT,
                       0,
                       G_IMPLEMENT_INTERFACE(GTK_SOURCE_TYPE_COMPLETION_PROVIDER,
                                             init_provider))

struct _GbSourceSnippetCompletionProviderPrivate
{
   GbSourceSnippets *snippets;
};

enum
{
   PROP_0,
   PROP_SNIPPETS,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceSnippets *
gb_source_snippet_completion_provider_get_snippets (GbSourceSnippetCompletionProvider *provider)
{
   g_return_val_if_fail(GB_IS_SOURCE_SNIPPET_COMPLETION_PROVIDER(provider), NULL);

   return provider->priv->snippets;
}

void
gb_source_snippet_completion_provider_set_snippets (GbSourceSnippetCompletionProvider *provider,
                                                    GbSourceSnippets                  *snippets)
{
   g_return_if_fail(GB_IS_SOURCE_SNIPPET_COMPLETION_PROVIDER(provider));

   g_clear_object(&provider->priv->snippets);
   provider->priv->snippets = snippets ? g_object_ref(snippets) : NULL;
   g_object_notify_by_pspec(G_OBJECT(provider), gParamSpecs[PROP_SNIPPETS]);
}

static void
gb_source_snippet_completion_provider_finalize (GObject *object)
{
   GbSourceSnippetCompletionProviderPrivate *priv;

   priv = GB_SOURCE_SNIPPET_COMPLETION_PROVIDER(object)->priv;

   g_clear_object(&priv->snippets);

   G_OBJECT_CLASS(gb_source_snippet_completion_provider_parent_class)->finalize(object);
}

static void
gb_source_snippet_completion_provider_get_property (GObject    *object,
                                                    guint       prop_id,
                                                    GValue     *value,
                                                    GParamSpec *pspec)
{
   GbSourceSnippetCompletionProvider *provider = GB_SOURCE_SNIPPET_COMPLETION_PROVIDER(object);

   switch (prop_id) {
   case PROP_SNIPPETS:
      g_value_set_object(value, gb_source_snippet_completion_provider_get_snippets(provider));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_snippet_completion_provider_set_property (GObject      *object,
                                                    guint         prop_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec)
{
   GbSourceSnippetCompletionProvider *provider = GB_SOURCE_SNIPPET_COMPLETION_PROVIDER(object);

   switch (prop_id) {
   case PROP_SNIPPETS:
      gb_source_snippet_completion_provider_set_snippets(provider, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_snippet_completion_provider_class_init (GbSourceSnippetCompletionProviderClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_snippet_completion_provider_finalize;
   object_class->get_property = gb_source_snippet_completion_provider_get_property;
   object_class->set_property = gb_source_snippet_completion_provider_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetCompletionProviderPrivate));

   gParamSpecs[PROP_SNIPPETS] =
      g_param_spec_object("snippets",
                          _("Snippets"),
                          _("The snippets to complete with this provider."),
                          GB_TYPE_SOURCE_SNIPPETS,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SNIPPETS,
                                   gParamSpecs[PROP_SNIPPETS]);
}

static void
gb_source_snippet_completion_provider_init (GbSourceSnippetCompletionProvider *provider)
{
   provider->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(provider,
                                  GB_TYPE_SOURCE_SNIPPET_COMPLETION_PROVIDER,
                                  GbSourceSnippetCompletionProviderPrivate);
}

static gboolean
is_stop_char (gunichar c)
{
   switch (c) {
   case '_':
      return FALSE;
   case ')':
   case '(':
   case '&':
   case '*':
   case '{':
   case '}':
   case ' ':
   case '\t':
   case '[':
   case ']':
   case '=':
   case '"':
   case '\'':
      return TRUE;
   default:
      return !g_unichar_isalnum(c);
   }
}

static gchar *
get_word (GtkSourceCompletionProvider *provider,
          GtkTextIter                 *iter)
{
   GtkTextIter *end;
   gboolean moved = FALSE;
   gunichar c;
   gchar *word;

   end = gtk_text_iter_copy(iter);

   do {
      if (!gtk_text_iter_backward_char(iter)) {
         break;
      }
      c = gtk_text_iter_get_char(iter);
      moved = TRUE;
   } while (!is_stop_char(c));

   if (moved && !gtk_text_iter_is_start(iter)) {
      gtk_text_iter_forward_char(iter);
   }

   word = g_strstrip(gtk_text_iter_get_text(iter, end));

   gtk_text_iter_free(end);

   return word;
}

static GdkPixbuf *
provider_get_icon (GtkSourceCompletionProvider *provider)
{
   return NULL;
}

static gint
provider_get_interactive_delay (GtkSourceCompletionProvider *provider)
{
   return 0;
}

static gint
provider_get_priority (GtkSourceCompletionProvider *provider)
{
   return 100;
}

static gchar *
provider_get_name (GtkSourceCompletionProvider *provider)
{
   return g_strdup(_("Snippets"));
}

static void
provider_populate (GtkSourceCompletionProvider *provider,
                   GtkSourceCompletionContext  *context)
{
   GtkSourceCompletionItem *item;
   GtkTextIter iter;
   GList *list = NULL;
   gchar *word;

   gtk_source_completion_context_get_iter(context, &iter);
   word = get_word(provider, &iter);

   /*
    * TODO: Fetch by word.
    */

   gtk_source_completion_context_add_proposals(context, provider, list, TRUE);

   g_list_foreach(list, (GFunc)g_object_unref, NULL);
   g_list_free(list);
   g_free(word);
}

static void
init_provider (GtkSourceCompletionProviderIface *iface)
{
   iface->get_icon = provider_get_icon;
   iface->get_interactive_delay = provider_get_interactive_delay;
   iface->get_name = provider_get_name;
   iface->populate = provider_populate;
   iface->get_priority = provider_get_priority;
}

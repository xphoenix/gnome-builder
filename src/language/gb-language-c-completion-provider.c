/* gb-language-c-completion-provider.c
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

#include <gtksourceview/gtksourcecompletionitem.h>

#include "gb-language-c-completion-provider.h"
#include "trie.h"

static void provider_init (GtkSourceCompletionProviderIface *iface);

G_DEFINE_TYPE_EXTENDED(GbLanguageCCompletionProvider,
                       gb_language_c_completion_provider,
                       G_TYPE_OBJECT,
                       0,
                       G_IMPLEMENT_INTERFACE(GTK_SOURCE_TYPE_COMPLETION_PROVIDER,
                                             provider_init))

struct _GbLanguageCCompletionProviderPrivate
{
   Trie *trie;
};

static GdkPixbuf *gPixbuf;

static void
gb_language_c_completion_provider_finalize (GObject *object)
{
   GbLanguageCCompletionProviderPrivate *priv;

   priv = GB_LANGUAGE_C_COMPLETION_PROVIDER(object)->priv;

   trie_destroy(priv->trie);
   priv->trie = NULL;

   G_OBJECT_CLASS(gb_language_c_completion_provider_parent_class)->finalize(object);
}

static void
gb_language_c_completion_provider_class_init (GbLanguageCCompletionProviderClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_language_c_completion_provider_finalize;
   g_type_class_add_private(object_class, sizeof(GbLanguageCCompletionProviderPrivate));

   gPixbuf = gdk_pixbuf_new_from_resource("/org/gnome/Builder/data/icons/keyword-16x.png", NULL);
}

static void
gb_language_c_completion_provider_init (GbLanguageCCompletionProvider *provider)
{
   static const gchar *keywords[] = {
      "auto",
      "break",
      "case",
      "char",
      "const",
      "continue",
      "default",
      "do",
      "double",
      "else",
      "enum",
      "extern",
      "float",
      "for",
      "goto",
      "if",
      "int",
      "long",
      "register",
      "return",
      "short",
      "signed",
      "sizeof",
      "static",
      "struct",
      "switch",
      "typedef",
      "union",
      "unsigned",
      "void",
      "volatile",
      "while",
      NULL
   };
   gint i;

   provider->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(provider,
                                  GB_TYPE_LANGUAGE_C_COMPLETION_PROVIDER,
                                  GbLanguageCCompletionProviderPrivate);

   provider->priv->trie = trie_new(NULL);

   for (i = 0; keywords[i]; i++) {
      trie_insert(provider->priv->trie, keywords[i], GINT_TO_POINTER(1));
   }
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

static gchar *
get_name (GtkSourceCompletionProvider *provider)
{
   return g_strdup("C Keywords");
}

static gint
get_interactive_delay (GtkSourceCompletionProvider *provider)
{
   return 0;
}

static gint
get_priority (GtkSourceCompletionProvider *provider)
{
   return 150;
}

static gboolean
traverse_cb (Trie        *trie,
             const gchar *key,
             gpointer     value,
             gpointer     user_data)
{
   GtkSourceCompletionItem *item;
   GList **list = user_data;

   item = gtk_source_completion_item_new(key, key, gPixbuf, NULL);
   *list = g_list_prepend(*list, item);

   return FALSE;
}

static void
provider_populate (GtkSourceCompletionProvider *provider,
                   GtkSourceCompletionContext  *context)
{
   GbLanguageCCompletionProviderPrivate *priv;
   GtkTextIter iter;
   GList *list = NULL;
   gchar *word;

   priv = GB_LANGUAGE_C_COMPLETION_PROVIDER(provider)->priv;

   gtk_source_completion_context_get_iter(context, &iter);
   word = get_word(provider, &iter);

   if (word && *word) {
      trie_traverse(priv->trie,
                    word,
                    G_PRE_ORDER,
                    G_TRAVERSE_LEAVES,
                    -1,
                    traverse_cb,
                    &list);
   }

   gtk_source_completion_context_add_proposals(context, provider, list, TRUE);

   g_list_foreach(list, (GFunc)g_object_unref, NULL);
   g_list_free(list);
}

static void
provider_init (GtkSourceCompletionProviderIface *iface)
{
   iface->get_name = get_name;
   iface->get_interactive_delay = get_interactive_delay;
   iface->get_priority = get_priority;
   iface->populate = provider_populate;
}

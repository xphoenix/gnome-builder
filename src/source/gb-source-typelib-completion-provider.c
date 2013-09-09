/* gb-source-typelib-completion-provider.c
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

#include <gio/gio.h>
#include <glib/gi18n.h>

#include "gb-multiprocess-manager.h"
#include "gb-source-typelib-completion-provider.h"

#define TYPELIB_WORKER_NAME "typelib-completion-provider"

static void provider_init (GtkSourceCompletionProviderIface *iface);

G_DEFINE_TYPE_EXTENDED(GbSourceTypelibCompletionProvider,
                       gb_source_typelib_completion_provider,
                       G_TYPE_OBJECT,
                       0,
                       G_IMPLEMENT_INTERFACE(GTK_SOURCE_TYPE_COMPLETION_PROVIDER,
                                             provider_init))

struct _GbSourceTypelibCompletionProviderPrivate
{
   GDBusConnection *peer;
};

GtkSourceCompletionProvider *
gb_source_typelib_completion_provider_new (void)
{
   return g_object_new(GB_TYPE_SOURCE_TYPELIB_COMPLETION_PROVIDER, NULL);
}

static GDBusConnection *
get_peer (GbSourceTypelibCompletionProvider *provider)
{
   GbSourceTypelibCompletionProviderPrivate *priv = provider->priv;
   GError *error = NULL;

   if (!priv->peer) {
      priv->peer = gb_multiprocess_manager_get_connection(GB_MULTIPROCESS_MANAGER_DEFAULT, TYPELIB_WORKER_NAME, &error);
      if (priv->peer) {
         g_object_add_weak_pointer(G_OBJECT(priv->peer), (gpointer *)&priv->peer);
      } else {
         g_warning("%s", error->message);
         g_error_free(error);
         return NULL;
      }
   }

   return priv->peer;
}

static void
gb_source_typelib_completion_provider_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_source_typelib_completion_provider_parent_class)->finalize(object);
}

static void
gb_source_typelib_completion_provider_class_init (GbSourceTypelibCompletionProviderClass *klass)
{
   static const gchar *argv[] = { "--typelib", NULL };
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_typelib_completion_provider_finalize;
   g_type_class_add_private(object_class, sizeof(GbSourceTypelibCompletionProviderPrivate));

   gb_multiprocess_manager_register(GB_MULTIPROCESS_MANAGER_DEFAULT,
                                    TYPELIB_WORKER_NAME,
                                    (gchar **)argv);
}

static void
gb_source_typelib_completion_provider_init (GbSourceTypelibCompletionProvider *provider)
{
   provider->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(provider,
                                  GB_TYPE_SOURCE_TYPELIB_COMPLETION_PROVIDER,
                                  GbSourceTypelibCompletionProviderPrivate);
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
   return g_strdup(_("Typelib"));
}

static void
provider_populate (GtkSourceCompletionProvider *provider,
                   GtkSourceCompletionContext  *context)
{
   GDBusConnection *peer;
   GtkTextIter iter;
   gchar *word;

   peer = get_peer(GB_SOURCE_TYPELIB_COMPLETION_PROVIDER(provider));
   if (!peer) {
      g_warning("No dbus provider to communicate with.");
      return;
   }

   g_print("DBUS: %p\n", peer);

   gtk_source_completion_context_get_iter(context, &iter);
   word = get_word(provider, &iter);

   g_print("Searching for: %s\n", word);

   g_free(word);
}

static void
provider_init (GtkSourceCompletionProviderIface *iface)
{
   iface->get_icon = provider_get_icon;
   iface->get_interactive_delay = provider_get_interactive_delay;
   iface->get_priority = provider_get_priority;
   iface->get_name = provider_get_name;
   iface->populate = provider_populate;
}

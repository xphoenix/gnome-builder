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
#include <gtksourceview/gtksourcecompletionitem.h>

#include "gb-application-resource.h"
#include "gb-dbus-typelib.h"
#include "gb-icon-theme.h"
#include "gb-multiprocess-manager.h"
#include "gb-source-snippet.h"
#include "gb-source-snippet-chunk.h"
#include "gb-source-typelib-completion-item.h"
#include "gb-source-typelib-completion-provider.h"
#include "gb-source-view.h"

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
   GbDBusTypelib *proxy;
   GbSourceView *view;
};

GtkSourceCompletionProvider *
gb_source_typelib_completion_provider_new (GbSourceView *view)
{
   GbSourceTypelibCompletionProvider *provider;

   provider = g_object_new(GB_TYPE_SOURCE_TYPELIB_COMPLETION_PROVIDER,
                           NULL);

   /*
    * TODO: Use property.
    */
   provider->priv->view = view;
   g_object_add_weak_pointer(G_OBJECT(view),
                             (gpointer *)&provider->priv->view);

   return GTK_SOURCE_COMPLETION_PROVIDER(provider);
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

static GbDBusTypelib *
get_proxy (GbSourceTypelibCompletionProvider *provider)
{
   GbSourceTypelibCompletionProviderPrivate *priv = provider->priv;
   GDBusConnection *peer;

   if (!priv->proxy) {
      peer = get_peer(provider);
      if (!peer) {
         return NULL;
      }

      priv->proxy = gb_dbus_typelib_proxy_new_sync(peer,
                                                   G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                                   NULL,
                                                   "/org/gnome/Builder/Typelib",
                                                   NULL,
                                                   NULL);
      g_object_add_weak_pointer(G_OBJECT(priv->proxy),
                                (gpointer *)&priv->proxy);

      /*
       * XXX: temporary for testing.
       */
      gb_dbus_typelib_call_require_sync(priv->proxy, "cairo", "1.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "Gtk", "3.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "Gdk", "3.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "GdkPixbuf", "2.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "GdkX11", "3.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "GLib", "2.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "Gio", "2.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "GObject", "2.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "Pango", "1.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "PangoCairo", "1.0", NULL, NULL);
      gb_dbus_typelib_call_require_sync(priv->proxy, "Vte", "2.90", NULL, NULL);
   }

   return priv->proxy;
}

static void
gb_source_typelib_completion_provider_finalize (GObject *object)
{
   GbSourceTypelibCompletionProviderPrivate *priv = GB_SOURCE_TYPELIB_COMPLETION_PROVIDER(object)->priv;

   if (priv->view) {
      g_object_remove_weak_pointer(G_OBJECT(priv->view),
                                   (gpointer *)&priv->view);
      priv->view = NULL;
   }

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

static void
get_word_bounds (GtkSourceCompletionProvider *provider,
                 const GtkTextIter           *iter,
                 GtkTextIter                 *begin,
                 GtkTextIter                 *end)
{
   gboolean moved = FALSE;
   gunichar c;

   gtk_text_iter_assign(end, iter);
   gtk_text_iter_assign(begin, iter);

   do {
      if (!gtk_text_iter_backward_char(begin)) {
         break;
      }
      c = gtk_text_iter_get_char(begin);
      moved = TRUE;
   } while (!is_stop_char(c));

   if (moved && !gtk_text_iter_is_start(begin)) {
      gtk_text_iter_forward_char(begin);
   }
}

static gchar *
get_word (GtkSourceCompletionProvider *provider,
          GtkTextIter                 *iter)
{
   GtkTextIter begin;
   GtkTextIter end;

   get_word_bounds(provider, iter, &begin, &end);
   return gtk_text_iter_get_text(&begin, iter);
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
closure_free (gpointer *closure)
{
   g_object_unref(closure[0]);
   g_object_unref(closure[1]);
   g_free(closure[2]);
   g_free(closure);
}

static void
complete_cb (GObject      *object,
             GAsyncResult *result,
             gpointer      user_data)
{
   GVariant *matches;
   gpointer *closure;
   GError *error = NULL;
   GList *list = NULL;
   guint flags;
   gint scale = 1; /* TODO: Get scale from widget */

   closure = (gpointer *)user_data;

   if (!gb_dbus_typelib_call_complete_finish(GB_DBUS_TYPELIB(object),
                                             &matches,
                                             result,
                                             &error)) {
      if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
         g_warning("%s", error->message);
      }
      goto cleanup;
   }

   if (matches) {
      GVariantIter *viter;
      const gchar *text;
      gdouble score;

      g_variant_get(matches, "a(sid)", &viter);
      while (g_variant_iter_loop(viter, "(sid)", &text, &flags, &score)) {
         GtkSourceCompletionItem *item;
         GdkPixbuf *pixbuf;
         gboolean is_function = FALSE;
         const gchar *icon_name = NULL;
         const gchar *overlay1 = NULL;
         const gchar *overlay2 = NULL;

         switch ((flags & 0xFF)) {
         case 1:
            icon_name = "class";
            break;
         case 2:
            icon_name = "method";
            is_function = TRUE;
            break;
         case 3:
            is_function = TRUE;
            icon_name = "method";
            overlay1 = "static";
            break;
         case 4:
            icon_name = "enum";
            break;
         case 5:
            icon_name = "enum-value";
            break;
         default:
            pixbuf = NULL;
            break;
         }

         if ((flags & 0x8000)) {
            overlay2 = "deprecated";
         }

         pixbuf = gb_icon_theme_load_with_overlay(icon_name,
                                                  scale,
                                                  overlay1,
                                                  overlay2);

         item = g_object_new(GB_TYPE_SOURCE_TYPELIB_COMPLETION_ITEM,
                             "icon", pixbuf,
                             "is-function", is_function,
                             "search-term", (gchar *)closure[2],
                             "text", text,
                             NULL);
         if (!item) {
            g_error("Failed to create item.");
         }
         list = g_list_prepend(list, item);
      }

      list = g_list_reverse(list);

      g_variant_unref(matches);
   }

   gtk_source_completion_context_add_proposals(closure[1], closure[0], list, TRUE);
   g_list_foreach(list, (GFunc)g_object_unref, NULL);
   g_list_free(list);

cleanup:
   closure_free(closure);
}

static void
provider_populate (GtkSourceCompletionProvider *provider,
                   GtkSourceCompletionContext  *context)
{
   GbDBusTypelib *proxy;
   GCancellable *cancellable;
   GtkTextIter iter;
   gpointer *closure;
   gchar *word;

   proxy = get_proxy(GB_SOURCE_TYPELIB_COMPLETION_PROVIDER(provider));
   if (!proxy) {
      g_warning("No dbus proxy.");
      return;
   }

   gtk_source_completion_context_get_iter(context, &iter);
   word = get_word(provider, &iter);

   closure = g_new0(gpointer, 3);
   closure[0] = g_object_ref(provider);
   closure[1] = g_object_ref(context);
   closure[2] = g_strdup(word);

   cancellable = g_cancellable_new();
   g_signal_connect_object(context,
                           "cancelled",
                           G_CALLBACK(g_cancellable_cancel),
                           cancellable,
                           G_CONNECT_SWAPPED);

   gb_dbus_typelib_call_complete(proxy,
                                 word,
                                 cancellable,
                                 complete_cb,
                                 closure);

   g_object_unref(cancellable);
}

static gboolean
provider_activate_proposal (GtkSourceCompletionProvider *provider,
                            GtkSourceCompletionProposal *proposal,
                            GtkTextIter                 *iter)
{
   GbSourceTypelibCompletionProviderPrivate *priv;
   GbSourceTypelibCompletionProvider *self;
   GbSourceTypelibCompletionItem *item;
   GbSourceSnippet *snippet;
   GbDBusTypelib *proxy;
   GtkTextBuffer *buffer;
   GtkTextIter begin;
   GtkTextIter end;

   self = GB_SOURCE_TYPELIB_COMPLETION_PROVIDER(provider);
   priv = self->priv;
   item = GB_SOURCE_TYPELIB_COMPLETION_ITEM(proposal);

   buffer = gtk_text_iter_get_buffer(iter);

   /*
    * Remove existing word.
    */
   get_word_bounds(provider, iter, &begin, &end);
   gtk_text_buffer_delete(buffer, &begin, &end);

   if (gb_source_typelib_completion_item_get_is_function(item)) {
      gchar **params;
      gchar *symbol;

      /*
       * Get the parameter list via DBus.
       */
      g_object_get(proposal, "text", &symbol, NULL);
      proxy = get_proxy(self);
      if (gb_dbus_typelib_call_get_params_sync(proxy,
                                               symbol,
                                               &params,
                                               NULL,
                                               NULL)) {
         gb_source_typelib_completion_item_set_params(
            item,
            (const gchar * const *)params);
         g_strfreev(params);
      }
      g_free(symbol);
   }

   /*
    * Insert a snippet for this completion item.
    */
   snippet = gb_source_typelib_completion_item_get_snippet(item);
   gb_source_view_push_snippet(GB_SOURCE_VIEW(priv->view), snippet);
   g_object_unref(snippet);

   /*
    * TODO: Store mapping for match to elevate result later.
    */

   return TRUE;
}

static void
provider_init (GtkSourceCompletionProviderIface *iface)
{
   iface->get_icon = provider_get_icon;
   iface->get_interactive_delay = provider_get_interactive_delay;
   iface->get_priority = provider_get_priority;
   iface->get_name = provider_get_name;
   iface->populate = provider_populate;
   iface->activate_proposal = provider_activate_proposal;
}

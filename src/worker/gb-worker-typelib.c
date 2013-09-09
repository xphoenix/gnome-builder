/* gb-worker-typelib.c
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

#include <girepository.h>

#include "gb-worker-typelib.h"
#include "gb-dbus-typelib.h"
#include "trie.h"

static GbDBusTypelib *gSkeleton;
static Trie *gTrie;
static guint gCount;

static void
handle_require (GbDBusTypelib         *typelib,
                GDBusMethodInvocation *method,
                const gchar           *name,
                const gchar           *version)
{
   GError *error = NULL;

   if (!g_irepository_require(g_irepository_get_default(),
                              name,
                              version,
                              0,
                              &error)) {
      g_dbus_method_invocation_take_error(method, error);
      return;
   }

   g_dbus_method_invocation_return_value(method, NULL);
}

static gboolean
traverse_cb (Trie        *trie,
             const gchar *key,
             gpointer     value,
             gpointer     user_data)
{
   GVariantBuilder *builder = user_data;

   gCount++;

   g_variant_builder_add(builder, "s", key);

   return (gCount == 1000);
}

static void
handle_get_methods (GbDBusTypelib         *typelib,
                    GDBusMethodInvocation *method,
                    const gchar           *word)
{
   GVariantBuilder *builder;
   GVariant *value;

   builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
   gCount = 0;

   trie_traverse(gTrie,
                 word,
                 G_POST_ORDER,
                 G_TRAVERSE_LEAVES,
                 -1,
                 traverse_cb,
                 method);

   g_dbus_method_invocation_return_value(method,
                                         g_variant_builder_end(builder));
}

void
gb_worker_typelib_init (GDBusConnection *connection)
{
   GError *error = NULL;

   gTrie = trie_new(NULL);

   gSkeleton = gb_dbus_typelib_skeleton_new();

   g_signal_connect(gSkeleton, "handle-require", G_CALLBACK(handle_require), NULL);
   g_signal_connect(gSkeleton, "handle-get-methods", G_CALLBACK(handle_get_methods), NULL);

   if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(gSkeleton),
                                         connection,
                                         "/org/gnome/Builder/Typelib",
                                         &error)) {
      g_error("%s\n", error->message);
      g_error_free(error);
   }
}

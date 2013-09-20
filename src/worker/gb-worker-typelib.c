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
#include <string.h>

#include "fuzzy.h"
#include "gb-dbus-typelib.h"
#include "gb-worker-typelib.h"

static GbDBusTypelib *gSkeleton;
static Fuzzy         *gFuzzy;
static GHashTable    *gParams;
static GStringChunk  *gChunks;

#define TYPE_CLASS    GINT_TO_POINTER(1)
#define TYPE_METHOD   GINT_TO_POINTER(2)
#define TYPE_FUNCTION GINT_TO_POINTER(3)

static void
load_function_info (GIRepository   *repository,
                    const gchar    *namespace,
                    GIFunctionInfo *info,
                    const gchar    *self_name,
                    gpointer        user_data)
{
   GIFunctionInfoFlags flags;
   const gchar *symbol;
   const gchar *name;
   GIArgInfo *arg;
   gchar **args;
   gint n_args;
   gint i;
   gint j = 0;

   symbol = g_function_info_get_symbol(info);
   fuzzy_insert(gFuzzy, symbol, user_data);

   n_args = g_callable_info_get_n_args((GICallableInfo *)info);
   args = g_new0(gchar*, n_args + 2);

   flags = g_function_info_get_flags(info);
   if ((flags & GI_FUNCTION_IS_METHOD)) {
      if (self_name) {
         args[j++] = (gchar *)self_name;
      }
   }

   for (i = 0; i < n_args; i++) {
      arg = g_callable_info_get_arg((GICallableInfo *)info, i);
      if (!g_arg_info_is_return_value(arg)) {
         name = g_base_info_get_name(arg);
         args[j++] = g_string_chunk_insert(gChunks, name);
      }
   }

   g_hash_table_insert(gParams, g_strdup(symbol), args);
}

static void
load_object_info (GIRepository *repository,
                  const gchar  *namespace,
                  GIObjectInfo *info)
{
   GIFunctionInfo *method;
   const gchar *symbol;
   const gchar *name;
   gchar *name_lower;
   gchar *tmp;
   guint n_methods;
   guint i;

   symbol = g_object_info_get_type_name(info);
   name = g_base_info_get_name((GIBaseInfo *)info);

   name_lower = g_ascii_strdown(name, -1);
   tmp = name_lower;
   name_lower = g_string_chunk_insert(gChunks, name_lower);
   g_free(tmp);

   fuzzy_insert(gFuzzy, symbol, TYPE_CLASS);

   n_methods = g_object_info_get_n_methods(info);
   for (i = 0; i < n_methods; i++) {
      method = g_object_info_get_method(info, i);
      load_function_info(repository, namespace, method, name_lower, TYPE_METHOD);
   }
}

static void
load_info (GIRepository *repository,
           const gchar  *namespace,
           GIBaseInfo   *info)
{
   if (GI_IS_FUNCTION_INFO(info)) {
      load_function_info(repository, namespace, (GIFunctionInfo *)info, NULL, TYPE_FUNCTION);
   } else if (GI_IS_OBJECT_INFO(info)) {
      load_object_info(repository, namespace, (GIObjectInfo *)info);
   }
}

static void
load_namespace (GIRepository *repository,
                const gchar  *namespace)
{
   GIBaseInfo *info;
   guint n_info;
   guint i;

   n_info = g_irepository_get_n_infos(repository, namespace);
   for (i = 0; i < n_info; i++) {
      info = g_irepository_get_info(repository, namespace, i);
      load_info(repository, namespace, info);
   }
}

static void
handle_require (GbDBusTypelib         *typelib,
                GDBusMethodInvocation *method,
                const gchar           *name,
                const gchar           *version)
{
   GIRepository *repository;
   GError *error = NULL;

   repository = g_irepository_get_default();

   if (!g_irepository_require(repository,
                              name,
                              version,
                              0,
                              &error)) {
      g_dbus_method_invocation_take_error(method, error);
      return;
   }

   fuzzy_begin_bulk_insert(gFuzzy);
   load_namespace(repository, name);
   fuzzy_end_bulk_insert(gFuzzy);

   g_dbus_method_invocation_return_value(method, NULL);
}

static void
handle_complete (GbDBusTypelib         *typelib,
                 GDBusMethodInvocation *method,
                 const gchar           *word)
{
   GVariantBuilder builder;
   FuzzyMatch *match;
   GVariant *value;
   gdouble score;
   GArray *matches;
   gint i;

   if (!word || !*word) {
      g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sid)"));
      value = g_variant_new("(a(sid))", &builder);
      g_dbus_method_invocation_return_value(method, value);
      return;
   }

   matches = fuzzy_match(gFuzzy, word, 1000);

   g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sid)"));

   for (i = 0; i < matches->len; i++) {
      match = &g_array_index(matches, FuzzyMatch, i);
      g_variant_builder_add(&builder, "(sid)",
                            match->key,
                            GPOINTER_TO_UINT(match->value),
                            match->score);
   }

   value = g_variant_new("(a(sid))", &builder);
   g_dbus_method_invocation_return_value(g_object_ref(method), value);

   g_array_unref(matches);
}

static void
handle_get_params (GbDBusTypelib         *typelib,
                   GDBusMethodInvocation *method,
                   const gchar           *symbol)
{
   GVariantBuilder builder;
   GVariant *value;
   gchar **params;
   gint i;

   g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

   if ((params = g_hash_table_lookup(gParams, symbol))) {
      for (i = 0; params[i]; i++) {
         g_variant_builder_add(&builder, "s", params[i]);
      }
   }

   value = g_variant_new("(as)", &builder);
   g_dbus_method_invocation_return_value(g_object_ref(method), value);
}

void
gb_worker_typelib_init (GDBusConnection *connection)
{
   GError *error = NULL;

   gFuzzy = fuzzy_new(FALSE);
   gChunks = g_string_chunk_new(4096);
   gSkeleton = gb_dbus_typelib_skeleton_new();
   gParams = g_hash_table_new_full(g_str_hash,
                                   g_str_equal,
                                   g_free,
                                   g_free);

   g_signal_connect(gSkeleton, "handle-require", G_CALLBACK(handle_require), NULL);
   g_signal_connect(gSkeleton, "handle-complete", G_CALLBACK(handle_complete), NULL);
   g_signal_connect(gSkeleton, "handle-get-params", G_CALLBACK(handle_get_params), NULL);

   if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(gSkeleton),
                                         connection,
                                         "/org/gnome/Builder/Typelib",
                                         &error)) {
      g_error("%s\n", error->message);
      g_error_free(error);
   }
}

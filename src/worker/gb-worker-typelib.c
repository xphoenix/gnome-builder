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

#define TYPE_CLASS    GINT_TO_POINTER(1)
#define TYPE_METHOD   GINT_TO_POINTER(2)
#define TYPE_FUNCTION GINT_TO_POINTER(3)

static void
load_function_info (GIRepository   *repository,
                    const gchar    *namespace,
                    GIFunctionInfo *info,
                    gpointer        flags)
{
   const gchar *symbol;

   symbol = g_function_info_get_symbol(info);
   fuzzy_insert(gFuzzy, symbol, flags);
}

static void
load_object_info (GIRepository *repository,
                  const gchar  *namespace,
                  GIObjectInfo *info)
{
   GIFunctionInfo *method;
   const gchar *symbol;
   guint n_methods;
   guint i;

   symbol = g_object_info_get_type_name(info);

   fuzzy_insert(gFuzzy, symbol, TYPE_CLASS);

   n_methods = g_object_info_get_n_methods(info);
   for (i = 0; i < n_methods; i++) {
      method = g_object_info_get_method(info, i);
      load_function_info(repository, namespace, method, TYPE_METHOD);
   }
}

static void
load_info (GIRepository *repository,
           const gchar  *namespace,
           GIBaseInfo   *info)
{
   if (GI_IS_FUNCTION_INFO(info)) {
      load_function_info(repository, namespace, (GIFunctionInfo *)info, TYPE_FUNCTION);
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

void
gb_worker_typelib_init (GDBusConnection *connection)
{
   GError *error = NULL;

   gFuzzy = fuzzy_new(FALSE);
   gSkeleton = gb_dbus_typelib_skeleton_new();

   g_signal_connect(gSkeleton, "handle-require", G_CALLBACK(handle_require), NULL);
   g_signal_connect(gSkeleton, "handle-complete", G_CALLBACK(handle_complete), NULL);

   if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(gSkeleton),
                                         connection,
                                         "/org/gnome/Builder/Typelib",
                                         &error)) {
      g_error("%s\n", error->message);
      g_error_free(error);
   }
}

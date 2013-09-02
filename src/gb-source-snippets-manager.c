/* gb-source-snippets-manager.c
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

#include "gb-source-snippets-manager.h"

G_DEFINE_TYPE(GbSourceSnippetsManager,
              gb_source_snippets_manager,
              G_TYPE_OBJECT)

struct _GbSourceSnippetsManagerPrivate
{
   GHashTable *by_language_id;
};

GbSourceSnippetsManager *
gb_source_snippets_manager_get_default (void)
{
   static GbSourceSnippetsManager *instance;

   if (!instance) {
      instance = g_object_new(GB_TYPE_SOURCE_SNIPPETS_MANAGER, NULL);
      g_object_add_weak_pointer(G_OBJECT(instance),
                                (gpointer *)&instance);
   }

   return instance;
}

GbSourceSnippets *
gb_source_snippets_manager_get_for_language (GbSourceSnippetsManager *manager,
                                             GtkSourceLanguage       *language)
{
   GbSourceSnippetsManagerPrivate *priv;
   const char *language_id;

   g_return_val_if_fail(GB_IS_SOURCE_SNIPPETS_MANAGER(manager), NULL);
   g_return_val_if_fail(GTK_SOURCE_IS_LANGUAGE(language), NULL);

   priv = manager->priv;

   language_id = gtk_source_language_get_id(language);
   return g_hash_table_lookup(priv->by_language_id, language_id);
}

static void
gb_source_snippets_manager_finalize (GObject *object)
{
   GbSourceSnippetsManagerPrivate *priv;
   
   priv = GB_SOURCE_SNIPPETS_MANAGER(object)->priv;

   g_clear_pointer(&priv->by_language_id, g_hash_table_unref);

   G_OBJECT_CLASS(gb_source_snippets_manager_parent_class)->finalize(object);
}

static void
gb_source_snippets_manager_class_init (GbSourceSnippetsManagerClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_snippets_manager_finalize;
   g_type_class_add_private(object_class, sizeof(GbSourceSnippetsManagerPrivate));
}

static void
gb_source_snippets_manager_init (GbSourceSnippetsManager *manager)
{
   manager->priv = G_TYPE_INSTANCE_GET_PRIVATE(manager,
                                               GB_TYPE_SOURCE_SNIPPETS_MANAGER,
                                               GbSourceSnippetsManagerPrivate);

   manager->priv->by_language_id = g_hash_table_new_full(g_str_hash,
                                                         g_str_equal,
                                                         g_free,
                                                         g_object_unref);
}

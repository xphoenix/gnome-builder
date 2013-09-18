/* gb-search-completion.c
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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gb-search-completion.h"

G_DEFINE_TYPE(GbSearchCompletion,
              gb_search_completion,
              GTK_TYPE_ENTRY_COMPLETION)

struct _GbSearchCompletionPrivate
{
   GPtrArray    *providers;
   GtkListStore *model;
};

enum
{
   COLUMN_PIXBUF,
   COLUMN_MARKUP,
   COLUMN_LAST
};

GtkEntryCompletion *
gb_search_completion_new (void)
{
   return g_object_new(GB_TYPE_SEARCH_COMPLETION, NULL);
}

void
gb_search_completion_add_provider (GbSearchCompletion *completion,
                                   GbSearchProvider   *provider)
{
   g_return_if_fail(GB_IS_SEARCH_COMPLETION(completion));
   g_return_if_fail(GB_IS_SEARCH_PROVIDER(provider));

   g_ptr_array_add(completion->priv->providers, g_object_ref(provider));
   /* TODO: sort */
   /* TODO: invalidate completions */
}

void
gb_search_completion_reload (GbSearchCompletion *completion,
                             const gchar        *search_term)
{
   GbSearchCompletionPrivate *priv;
   GbSearchProvider *provider;
   guint i;

   g_return_if_fail(GB_IS_SEARCH_COMPLETION(completion));
   g_return_if_fail(search_term && *search_term);

   priv = completion->priv;

   gtk_list_store_clear(priv->model);

   for (i = 0; i < priv->providers->len; i++) {
      provider = g_ptr_array_index(priv->providers, i);
      gb_search_provider_populate(provider, search_term, priv->model);
   }
}

void
gb_search_completion_remove_provider (GbSearchCompletion *completion,
                                      GbSearchProvider   *provider)
{
   g_return_if_fail(GB_IS_SEARCH_COMPLETION(completion));
   g_return_if_fail(GB_IS_SEARCH_PROVIDER(provider));

   g_ptr_array_remove(completion->priv->providers, provider);
}

gboolean
gb_search_completion_match_selected (GtkEntryCompletion *completion,
                                     GtkTreeModel       *model,
                                     GtkTreeIter        *iter)
{
   GbSearchCompletionPrivate *priv;
   GbSearchProvider *provider;

   g_return_val_if_fail(GB_IS_SEARCH_COMPLETION(completion), FALSE);
   g_return_val_if_fail(GTK_IS_LIST_STORE(model), FALSE);
   g_return_val_if_fail(iter, FALSE);

   priv = GB_SEARCH_COMPLETION(completion)->priv;

   gtk_tree_model_get(model, iter,
                      GB_SEARCH_COMPLETION_COLUMN_PROVIDER, &provider,
                      -1);

   if (provider) {
      gb_search_provider_activate(provider, model, iter);
      g_object_unref(provider);
   }

   return TRUE;
}

static gboolean
gb_search_completion_match_func (GtkEntryCompletion *completion,
                                 const gchar        *key,
                                 GtkTreeIter        *iter,
                                 gpointer            user_data)
{
   return TRUE;
}

static void
gb_search_completion_constructed (GObject *object)
{
   GbSearchCompletionPrivate *priv = GB_SEARCH_COMPLETION(object)->priv;
   GtkCellRenderer *renderer;

   g_object_set(object,
                "model", priv->model,
                "text-column", GB_SEARCH_COMPLETION_COLUMN_TEXT,
                NULL);

   renderer = g_object_new(GTK_TYPE_CELL_RENDERER_PIXBUF,
                           "height", 16,
                           "width", 16,
                           "xpad", 3,
                           NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(object), renderer, FALSE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(object), renderer,
                                 "pixbuf", COLUMN_PIXBUF);

   renderer = g_object_new(GTK_TYPE_CELL_RENDERER_TEXT,
                           "xalign", 0.0f,
                           NULL);
   gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(object), renderer, TRUE);
   gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(object), renderer,
                                 "markup", COLUMN_MARKUP);

   if (G_OBJECT_CLASS(gb_search_completion_parent_class)->constructed) {
      G_OBJECT_CLASS(gb_search_completion_parent_class)->constructed(object);
   }
}

static void
gb_search_completion_finalize (GObject *object)
{
   GbSearchCompletionPrivate *priv = GB_SEARCH_COMPLETION(object)->priv;

   g_ptr_array_unref(priv->providers);
   priv->providers = NULL;

   g_clear_object(&priv->model);

   G_OBJECT_CLASS(gb_search_completion_parent_class)->finalize(object);
}

static void
gb_search_completion_class_init (GbSearchCompletionClass *klass)
{
   GObjectClass *object_class;
   GtkEntryCompletionClass *completion_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->constructed = gb_search_completion_constructed;
   object_class->finalize = gb_search_completion_finalize;
   g_type_class_add_private(object_class, sizeof(GbSearchCompletionPrivate));

   completion_class = GTK_ENTRY_COMPLETION_CLASS(klass);
   completion_class->match_selected = gb_search_completion_match_selected;
}

static void
gb_search_completion_init (GbSearchCompletion *completion)
{
   GtkCellRenderer *renderer;

   completion->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(completion,
                                  GB_TYPE_SEARCH_COMPLETION,
                                  GbSearchCompletionPrivate);

   completion->priv->providers = g_ptr_array_new();
   g_ptr_array_set_free_func(completion->priv->providers, g_object_unref);

   completion->priv->model = gtk_list_store_new(5,
                                                GDK_TYPE_PIXBUF,
                                                G_TYPE_STRING,
                                                G_TYPE_STRING,
                                                GB_TYPE_SEARCH_PROVIDER,
                                                G_TYPE_OBJECT);

   gtk_entry_completion_set_match_func(GTK_ENTRY_COMPLETION(completion),
                                       gb_search_completion_match_func,
                                       NULL,
                                       NULL);
}

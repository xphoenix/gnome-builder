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

#include "gb-search-completion.h"

G_DEFINE_TYPE(GbSearchCompletion,
              gb_search_completion,
              GTK_TYPE_ENTRY_COMPLETION)

struct _GbSearchCompletionPrivate
{
   gpointer dummy;
};

GtkEntryCompletion *
gb_search_completion_new (void)
{
   return g_object_new(GB_TYPE_SEARCH_COMPLETION, NULL);
}

static void
gb_search_completion_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_search_completion_parent_class)->finalize(object);
}

static void
gb_search_completion_class_init (GbSearchCompletionClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_search_completion_finalize;
   g_type_class_add_private(object_class, sizeof(GbSearchCompletionPrivate));
}

static void
gb_search_completion_init (GbSearchCompletion *completion)
{
   completion->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(completion,
                                  GB_TYPE_SEARCH_COMPLETION,
                                  GbSearchCompletionPrivate);
}

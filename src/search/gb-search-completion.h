/* gb-search-completion.h
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

#ifndef GB_SEARCH_COMPLETION_H
#define GB_SEARCH_COMPLETION_H

#include <gtk/gtk.h>

#include "gb-search-provider.h"

G_BEGIN_DECLS

#define GB_TYPE_SEARCH_COMPLETION            (gb_search_completion_get_type())
#define GB_SEARCH_COMPLETION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SEARCH_COMPLETION, GbSearchCompletion))
#define GB_SEARCH_COMPLETION_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SEARCH_COMPLETION, GbSearchCompletion const))
#define GB_SEARCH_COMPLETION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SEARCH_COMPLETION, GbSearchCompletionClass))
#define GB_IS_SEARCH_COMPLETION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SEARCH_COMPLETION))
#define GB_IS_SEARCH_COMPLETION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SEARCH_COMPLETION))
#define GB_SEARCH_COMPLETION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SEARCH_COMPLETION, GbSearchCompletionClass))

typedef struct _GbSearchCompletion        GbSearchCompletion;
typedef struct _GbSearchCompletionClass   GbSearchCompletionClass;
typedef enum   _GbSearchCompletionColumn  GbSearchCompletionColumn;
typedef struct _GbSearchCompletionPrivate GbSearchCompletionPrivate;

struct _GbSearchCompletion
{
   GtkEntryCompletion parent;

   /*< private >*/
   GbSearchCompletionPrivate *priv;
};

struct _GbSearchCompletionClass
{
   GtkEntryCompletionClass parent_class;
};

enum _GbSearchCompletionColumn
{
   GB_SEARCH_COMPLETION_COLUMN_PIXBUF = 0,
   GB_SEARCH_COMPLETION_COLUMN_MARKUP = 1,
   GB_SEARCH_COMPLETION_COLUMN_TEXT   = 2,
};

GType               gb_search_completion_get_type        (void) G_GNUC_CONST;
GtkEntryCompletion *gb_search_completion_new             (void);
void                gb_search_completion_add_provider    (GbSearchCompletion *completion,
                                                          GbSearchProvider   *provider);
void                gb_search_completion_remove_provider (GbSearchCompletion *completion,
                                                          GbSearchProvider   *provider);
void                gb_search_completion_reload          (GbSearchCompletion *completion,
                                                          const gchar        *search_term);

G_END_DECLS

#endif /* GB_SEARCH_COMPLETION_H */

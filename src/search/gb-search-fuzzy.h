/* gb-search-fuzzy.h
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

#ifndef GB_SEARCH_FUZZY_H
#define GB_SEARCH_FUZZY_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_SEARCH_FUZZY            (gb_search_fuzzy_get_type())
#define GB_SEARCH_FUZZY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SEARCH_FUZZY, GbSearchFuzzy))
#define GB_SEARCH_FUZZY_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SEARCH_FUZZY, GbSearchFuzzy const))
#define GB_SEARCH_FUZZY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SEARCH_FUZZY, GbSearchFuzzyClass))
#define GB_IS_SEARCH_FUZZY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SEARCH_FUZZY))
#define GB_IS_SEARCH_FUZZY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SEARCH_FUZZY))
#define GB_SEARCH_FUZZY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SEARCH_FUZZY, GbSearchFuzzyClass))

typedef struct _GbSearchFuzzy        GbSearchFuzzy;
typedef struct _GbSearchFuzzyClass   GbSearchFuzzyClass;
typedef struct _GbSearchFuzzyPrivate GbSearchFuzzyPrivate;

struct _GbSearchFuzzy
{
   GObject parent;

   /*< private >*/
   GbSearchFuzzyPrivate *priv;
};

struct _GbSearchFuzzyClass
{
   GObjectClass parent_class;
};

GType          gb_search_fuzzy_get_type              (void) G_GNUC_CONST;
GbSearchFuzzy *gb_search_fuzzy_new                   (void);
gchar         *gb_search_fuzzy_match_score_highlight (GbSearchFuzzy *fuzzy,
                                                      const gchar   *haystack,
                                                      const gchar   *needle,
                                                      gfloat        *score);

G_END_DECLS

#endif /* GB_SEARCH_FUZZY_H */

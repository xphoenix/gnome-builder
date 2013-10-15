/* gb-back-forward-list.h
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

#ifndef GB_BACK_FORWARD_LIST_H
#define GB_BACK_FORWARD_LIST_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_BACK_FORWARD_LIST            (gb_back_forward_list_get_type())
#define GB_BACK_FORWARD_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_BACK_FORWARD_LIST, GbBackForwardList))
#define GB_BACK_FORWARD_LIST_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_BACK_FORWARD_LIST, GbBackForwardList const))
#define GB_BACK_FORWARD_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_BACK_FORWARD_LIST, GbBackForwardListClass))
#define GB_IS_BACK_FORWARD_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_BACK_FORWARD_LIST))
#define GB_IS_BACK_FORWARD_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_BACK_FORWARD_LIST))
#define GB_BACK_FORWARD_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_BACK_FORWARD_LIST, GbBackForwardListClass))

typedef struct _GbBackForwardList        GbBackForwardList;
typedef struct _GbBackForwardListClass   GbBackForwardListClass;
typedef struct _GbBackForwardListPrivate GbBackForwardListPrivate;

struct _GbBackForwardList
{
   GObject parent;

   /*< private >*/
   GbBackForwardListPrivate *priv;
};

struct _GbBackForwardListClass
{
   GObjectClass parent_class;
};

GType              gb_back_forward_list_get_type            (void) G_GNUC_CONST;
GbBackForwardList *gb_back_forward_list_new                 (void);
void               gb_back_forward_list_push                (GbBackForwardList *list,
                                                             const gchar       *uri);
void               gb_back_forward_list_go_backward         (GbBackForwardList *list);
void               gb_back_forward_list_go_forward          (GbBackForwardList *list);
gboolean           gb_back_forward_list_get_can_go_backward (GbBackForwardList *list);
gboolean           gb_back_forward_list_get_can_go_forward  (GbBackForwardList *list);

G_END_DECLS

#endif /* GB_BACK_FORWARD_LIST_H */

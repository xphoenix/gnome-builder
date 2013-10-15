/* gb-back-forward-list.c
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

#include "gb-back-forward-list.h"

#define MAX_BACK_LENGTH 25

G_DEFINE_TYPE(GbBackForwardList, gb_back_forward_list, G_TYPE_OBJECT)

struct _GbBackForwardListPrivate
{
   GList *back;
   gchar *current;
   GList *forward;
};

enum
{
   URI_CHANGED,
   LAST_SIGNAL
};

static guint gSignals[LAST_SIGNAL];

GbBackForwardList *
gb_back_forward_list_new (void)
{
   return g_object_new(GB_TYPE_BACK_FORWARD_LIST, NULL);
}

void
gb_back_forward_list_push (GbBackForwardList *list,
                           const gchar       *uri)
{
   GbBackForwardListPrivate *priv;
   GList *last;
   guint length;

   g_return_if_fail(GB_IS_BACK_FORWARD_LIST(list));
   g_return_if_fail(uri);

   priv = list->priv;

   if (priv->current) {
      priv->back = g_list_prepend(priv->back, priv->current);
      priv->current = NULL;

      length = g_list_length(priv->back);
      last = g_list_last(priv->back);
      for (; length > MAX_BACK_LENGTH; length--) {
         g_free(last->data);
         last = last->prev;
         priv->back = g_list_delete_link(priv->back, last->next);
      }
   }

   if (priv->forward) {
      g_list_foreach(priv->forward, (GFunc)g_free, NULL);
      g_list_free(priv->forward);
      priv->forward = NULL;
   }

   priv->current = g_strdup(uri);
}

gboolean
gb_back_forward_list_get_can_go_backward (GbBackForwardList *list)
{
   g_return_val_if_fail(GB_IS_BACK_FORWARD_LIST(list), FALSE);
   return !!list->priv->back;
}

gboolean
gb_back_forward_list_get_can_go_forward (GbBackForwardList *list)
{
   g_return_val_if_fail(GB_IS_BACK_FORWARD_LIST(list), FALSE);
   return !!list->priv->forward;
}

void
gb_back_forward_list_go_backward (GbBackForwardList *list)
{
   GbBackForwardListPrivate *priv;
   GList *link_;

   g_return_if_fail(GB_IS_BACK_FORWARD_LIST(list));

   priv = list->priv;

   if ((link_ = priv->back)) {
      if (priv->current) {
         priv->forward = g_list_prepend(priv->forward, priv->current);
      }
      priv->back = g_list_remove_link(priv->back, link_);
      priv->current = link_->data;
      g_list_free(link_);
   }
}

void
gb_back_forward_list_go_forward (GbBackForwardList *list)
{
   GbBackForwardListPrivate *priv;
   GList *link_;

   g_return_if_fail(GB_IS_BACK_FORWARD_LIST(list));

   priv = list->priv;

   if ((link_ = priv->forward)) {
      if (priv->current) {
         priv->back = g_list_prepend(priv->back, priv->current);
         priv->current = NULL;
      }
      priv->current = link_->data;
      priv->forward = g_list_remove_link(priv->forward, link_);
      g_list_free(link_);
   }
}

static void
gb_back_forward_list_finalize (GObject *object)
{
   GbBackForwardListPrivate *priv;

   priv = GB_BACK_FORWARD_LIST(object)->priv;

   g_list_foreach(priv->back, (GFunc)g_free, NULL);
   g_list_free(priv->back);
   priv->back = NULL;

   g_list_foreach(priv->forward, (GFunc)g_free, NULL);
   g_list_free(priv->forward);
   priv->forward = NULL;

   G_OBJECT_CLASS(gb_back_forward_list_parent_class)->finalize(object);
}

static void
gb_back_forward_list_class_init (GbBackForwardListClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_back_forward_list_finalize;
   g_type_class_add_private(object_class, sizeof(GbBackForwardListPrivate));

   gSignals[URI_CHANGED] = g_signal_new("uri-changed",
                                        GB_TYPE_BACK_FORWARD_LIST,
                                        G_SIGNAL_RUN_LAST,
                                        0,
                                        NULL,
                                        NULL,
                                        g_cclosure_marshal_VOID__STRING,
                                        G_TYPE_NONE,
                                        1,
                                        G_TYPE_STRING);
}

static void
gb_back_forward_list_init (GbBackForwardList *list)
{
   list->priv = G_TYPE_INSTANCE_GET_PRIVATE(list,
                                            GB_TYPE_BACK_FORWARD_LIST,
                                            GbBackForwardListPrivate);
}

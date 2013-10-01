/* gb-source-view.c
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

#include "gb-source-snippet-private.h"
#include "gb-source-view.h"

G_DEFINE_TYPE(GbSourceView, gb_source_view, GTK_SOURCE_TYPE_VIEW)

struct _GbSourceViewPrivate
{
   GQueue *snippets;

   GtkTextBuffer *buffer;
   guint          buffer_insert_text_handler;
   guint          buffer_insert_text_after_handler;
   guint          buffer_delete_range_handler;
   guint          buffer_delete_range_after_handler;
   guint          buffer_mark_set_handler;
};

static void
gb_source_view_block_handlers (GbSourceView *view)
{
   GbSourceViewPrivate *priv = view->priv;

   if (priv->buffer) {
      g_signal_handler_block(priv->buffer,
                             priv->buffer_insert_text_handler);
      g_signal_handler_block(priv->buffer,
                             priv->buffer_insert_text_after_handler);
      g_signal_handler_block(priv->buffer,
                             priv->buffer_delete_range_handler);
      g_signal_handler_block(priv->buffer,
                             priv->buffer_delete_range_after_handler);
      g_signal_handler_block(priv->buffer,
                             priv->buffer_mark_set_handler);
   }
}

static void
gb_source_view_unblock_handlers (GbSourceView *view)
{
   GbSourceViewPrivate *priv = view->priv;

   if (priv->buffer) {
      g_signal_handler_unblock(priv->buffer,
                               priv->buffer_insert_text_handler);
      g_signal_handler_unblock(priv->buffer,
                               priv->buffer_insert_text_after_handler);
      g_signal_handler_unblock(priv->buffer,
                               priv->buffer_delete_range_handler);
      g_signal_handler_unblock(priv->buffer,
                               priv->buffer_delete_range_after_handler);
      g_signal_handler_unblock(priv->buffer,
                               priv->buffer_mark_set_handler);
   }
}

void
gb_source_view_pop_snippet (GbSourceView *view)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   priv = view->priv;

   if ((snippet = g_queue_pop_head(priv->snippets))) {
      gb_source_snippet_finish(snippet);
      g_object_unref(snippet);
   }

   if ((snippet = g_queue_peek_head(priv->snippets))) {
      gb_source_snippet_unpause(snippet);
   }
}

void
gb_source_view_push_snippet (GbSourceView    *view,
                             GbSourceSnippet *snippet)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *previous;
   GtkTextBuffer *buffer;
   GtkTextMark *mark;
   GtkTextIter iter;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));
   g_return_if_fail(GB_IS_SOURCE_SNIPPET(snippet));

   priv = view->priv;

   if ((previous = g_queue_peek_head(priv->snippets))) {
      gb_source_snippet_pause(previous);
   }

   g_queue_push_head(priv->snippets, g_object_ref(snippet));

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
   mark = gtk_text_buffer_get_insert(buffer);
   gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);

   gb_source_view_block_handlers(view);
   gb_source_snippet_begin(snippet, buffer, &iter);
   gb_source_view_unblock_handlers(view);
}

static void
on_insert_text (GtkTextBuffer *buffer,
                GtkTextIter   *iter,
                gchar         *text,
                gint           len,
                gpointer       user_data)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;
   GbSourceView *view = user_data;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   priv = view->priv;

   gb_source_view_block_handlers(view);

   if ((snippet = g_queue_peek_head(priv->snippets))) {
      gb_source_snippet_before_insert_text(snippet, buffer, iter, text, len);
   }

   gb_source_view_unblock_handlers(view);
}

static void
on_insert_text_after (GtkTextBuffer *buffer,
                      GtkTextIter   *iter,
                      gchar         *text,
                      gint           len,
                      gpointer       user_data)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;
   GbSourceView *view = user_data;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   priv = view->priv;

   gb_source_view_block_handlers(view);

   if ((snippet = g_queue_peek_head(priv->snippets))) {
      gb_source_snippet_after_insert_text(snippet, buffer, iter, text, len);
   }

   gb_source_view_unblock_handlers(view);
}

static void
on_delete_range (GtkTextBuffer *buffer,
                 GtkTextIter   *begin,
                 GtkTextIter   *end,
                 gpointer       user_data)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;
   GbSourceView *view = user_data;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   priv = view->priv;

   gb_source_view_block_handlers(view);

   if ((snippet = g_queue_peek_head(priv->snippets))) {
      gb_source_snippet_before_delete_range(snippet, buffer, begin, end);
   }

   gb_source_view_unblock_handlers(view);
}

static void
on_delete_range_after (GtkTextBuffer *buffer,
                       GtkTextIter   *begin,
                       GtkTextIter   *end,
                       gpointer       user_data)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;
   GbSourceView *view = user_data;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   priv = view->priv;

   gb_source_view_block_handlers(view);

   if ((snippet = g_queue_peek_head(priv->snippets))) {
      gb_source_snippet_after_delete_range(snippet, buffer, begin, end);
   }

   gb_source_view_unblock_handlers(view);
}

static void
on_mark_set (GtkTextBuffer *buffer,
             GtkTextIter   *iter,
             GtkTextMark   *mark,
             gpointer       user_data)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;
   GbSourceView *view = user_data;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));
   g_return_if_fail(iter);
   g_return_if_fail(GTK_IS_TEXT_MARK(mark));

   priv = view->priv;

   gb_source_view_block_handlers(view);

   if ((mark == gtk_text_buffer_get_insert(buffer))) {
again:
      if ((snippet = g_queue_peek_head(priv->snippets))) {
         if (!gb_source_snippet_insert_set(snippet, mark)) {
            gb_source_view_pop_snippet(view);
            goto again;
         }
      }
   }

   gb_source_view_unblock_handlers(view);
}

static void
gb_source_view_notify_buffer (GObject    *object,
                              GParamSpec *pspec,
                              gpointer    user_data)
{
   GbSourceViewPrivate *priv;
   GtkTextBuffer *buffer;

   g_return_if_fail(GB_IS_SOURCE_VIEW(object));
   g_return_if_fail(pspec);
   g_return_if_fail(!g_strcmp0(pspec->name, "buffer"));

   priv = GB_SOURCE_VIEW(object)->priv;

   if (priv->buffer) {
      g_signal_handler_disconnect(priv->buffer,
                                  priv->buffer_insert_text_handler);
      g_signal_handler_disconnect(priv->buffer,
                                  priv->buffer_insert_text_after_handler);
      g_signal_handler_disconnect(priv->buffer,
                                  priv->buffer_delete_range_handler);
      g_signal_handler_disconnect(priv->buffer,
                                  priv->buffer_delete_range_after_handler);
      g_signal_handler_disconnect(priv->buffer,
                                  priv->buffer_mark_set_handler);
      priv->buffer_insert_text_handler = 0;
      priv->buffer_insert_text_after_handler = 0;
      priv->buffer_delete_range_handler = 0;
      priv->buffer_delete_range_after_handler = 0;
      priv->buffer_mark_set_handler = 0;
      g_object_remove_weak_pointer(G_OBJECT(priv->buffer),
                                   (gpointer *)&priv->buffer);
      priv->buffer = NULL;
   }

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));

   if (buffer) {
      priv->buffer = buffer;
      g_object_add_weak_pointer(G_OBJECT(buffer),
                                (gpointer *)&priv->buffer);
      priv->buffer_insert_text_handler =
         g_signal_connect_object(buffer,
                                 "insert-text",
                                 G_CALLBACK(on_insert_text),
                                 object,
                                 0);
      priv->buffer_insert_text_after_handler =
         g_signal_connect_object(buffer,
                                 "insert-text",
                                 G_CALLBACK(on_insert_text_after),
                                 object,
                                 G_CONNECT_AFTER);
      priv->buffer_delete_range_handler =
         g_signal_connect_object(buffer,
                                 "delete-range",
                                 G_CALLBACK(on_delete_range),
                                 object,
                                 0);
      priv->buffer_delete_range_after_handler =
         g_signal_connect_object(buffer,
                                 "delete-range",
                                 G_CALLBACK(on_delete_range_after),
                                 object,
                                 G_CONNECT_AFTER);
      priv->buffer_mark_set_handler =
         g_signal_connect_object(buffer,
                                 "mark-set",
                                 G_CALLBACK(on_mark_set),
                                 object,
                                 0);
   }
}

static gboolean
gb_source_view_key_press_event (GtkWidget   *widget,
                                GdkEventKey *event)
{
   GbSourceViewPrivate *priv;
   GbSourceSnippet *snippet;
   GbSourceView *view = (GbSourceView *)widget;

   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), FALSE);
   g_return_val_if_fail(event, FALSE);

   priv = view->priv;

   if ((snippet = g_queue_peek_head(priv->snippets))) {
      switch ((gint)event->keyval) {
      case GDK_KEY_Tab:
         gb_source_view_block_handlers(view);
         if (!gb_source_snippet_move_next(snippet)) {
            gb_source_view_pop_snippet(view);
         }
         gb_source_view_unblock_handlers(view);
         return TRUE;
      case GDK_KEY_ISO_Left_Tab:
         gb_source_view_block_handlers(view);
         gb_source_snippet_move_previous(snippet);
         gb_source_view_unblock_handlers(view);
         return TRUE;
      default:
         break;
      }
   }

   return GTK_WIDGET_CLASS(gb_source_view_parent_class)->key_press_event(widget, event);
}

static void
gb_source_view_finalize (GObject *object)
{
   GbSourceViewPrivate *priv;

   priv = GB_SOURCE_VIEW(object)->priv;

   g_clear_pointer(&priv->snippets, g_queue_free);

   G_OBJECT_CLASS(gb_source_view_parent_class)->finalize(object);
}

static void
gb_source_view_class_init (GbSourceViewClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_view_finalize;
   g_type_class_add_private(object_class, sizeof(GbSourceViewPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->key_press_event = gb_source_view_key_press_event;
}

static void
gb_source_view_init (GbSourceView *view)
{
   view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,
                                            GB_TYPE_SOURCE_VIEW,
                                            GbSourceViewPrivate);

   view->priv->snippets = g_queue_new();

   g_signal_connect(view,
                    "notify::buffer",
                    G_CALLBACK(gb_source_view_notify_buffer),
                    NULL);
}

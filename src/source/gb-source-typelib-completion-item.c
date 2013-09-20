/* gb-source-typelib-completion-item.c
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

#include "highlight.h"
#include "gb-source-typelib-completion-item.h"

static void
proposal_init (GtkSourceCompletionProposalIface *iface);

G_DEFINE_TYPE_EXTENDED(GbSourceTypelibCompletionItem,
                       gb_source_typelib_completion_item,
                       G_TYPE_OBJECT,
                       0,
                       G_IMPLEMENT_INTERFACE(GTK_SOURCE_TYPE_COMPLETION_PROPOSAL,
                                             proposal_init))

struct _GbSourceTypelibCompletionItemPrivate
{
   GdkPixbuf  *icon;
   gchar      *search_term;
   gchar      *text;
   gchar     **params;
   gboolean   is_function;
};

enum
{
   PROP_0,
   PROP_ICON,
   PROP_IS_FUNCTION,
   PROP_MARKUP,
   PROP_SEARCH_TERM,
   PROP_TEXT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

void
gb_source_typelib_completion_item_set_params (GbSourceTypelibCompletionItem *item,
                                              const gchar * const           *params)
{
   g_return_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item));

   g_strfreev(item->priv->params);
   item->priv->params = g_strdupv((gchar **)params);
}

GbSourceSnippet *
gb_source_typelib_completion_item_get_snippet (GbSourceTypelibCompletionItem *item)
{
   GbSourceTypelibCompletionItemPrivate *priv;
   GbSourceSnippetChunk *chunk;
   GbSourceSnippet *snippet;

   g_return_val_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item), NULL);

   priv = item->priv;

   /*
    * TODO: This will get more complicated as we start tracking parameters
    *       and such. We will need to insert a chunk for each parameter.
    */

   snippet = gb_source_snippet_new(NULL);

   chunk = gb_source_snippet_chunk_new();
   gb_source_snippet_chunk_set_text(chunk, priv->text);
   gb_source_snippet_add_chunk(snippet, chunk);
   g_object_unref(chunk);

   if (priv->is_function) {
      guint i = 1;

      chunk = gb_source_snippet_chunk_new();
      gb_source_snippet_chunk_set_text(chunk, " (");
      gb_source_snippet_add_chunk(snippet, chunk);
      g_object_unref(chunk);

      if (priv->params && priv->params[0]) {
         for (i = 0; priv->params[i]; i++) {
            if (i != 0) {
               chunk = gb_source_snippet_chunk_new();
               gb_source_snippet_chunk_set_text(chunk, ", ");
               gb_source_snippet_add_chunk(snippet, chunk);
               g_object_unref(chunk);
            }
            chunk = gb_source_snippet_chunk_new();
            gb_source_snippet_chunk_set_text(chunk, priv->params[i]);
            gb_source_snippet_chunk_set_tab_stop(chunk, i + 1);
            gb_source_snippet_add_chunk(snippet, chunk);
            g_object_unref(chunk);
         }
      } else {
         chunk = gb_source_snippet_chunk_new();
         gb_source_snippet_chunk_set_tab_stop(chunk, 1);
         gb_source_snippet_add_chunk(snippet, chunk);
         g_object_unref(chunk);
      }

      chunk = gb_source_snippet_chunk_new();
      gb_source_snippet_chunk_set_text(chunk, ")");
      gb_source_snippet_add_chunk(snippet, chunk);
      g_object_unref(chunk);

      chunk = gb_source_snippet_chunk_new();
      gb_source_snippet_chunk_set_tab_stop(chunk, 0);
      gb_source_snippet_add_chunk(snippet, chunk);
      g_object_unref(chunk);
   }

   return snippet;
}

gboolean
gb_source_typelib_completion_item_get_is_function (GbSourceTypelibCompletionItem *item)
{
   g_return_val_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item), FALSE);

   return item->priv->is_function;
}

void
gb_source_typelib_completion_item_set_is_function (GbSourceTypelibCompletionItem *item,
                                                   gboolean                       is_function)
{
   g_return_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item));

   item->priv->is_function = is_function;
}

static const gchar *
gb_source_typelib_completion_item_get_search_term (GbSourceTypelibCompletionItem *item)
{
   g_return_val_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item), NULL);
   return item->priv->search_term;
}

static void
gb_source_typelib_completion_item_set_search_term (GbSourceTypelibCompletionItem *item,
                                                   const gchar                   *search_term)
{
   g_return_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item));

   g_free(item->priv->search_term);
   item->priv->search_term = g_strdup(search_term);
}

static gchar *
gb_source_typelib_completion_item_get_text (GtkSourceCompletionProposal *proposal)
{
   GbSourceTypelibCompletionItem *item = GB_SOURCE_TYPELIB_COMPLETION_ITEM(proposal);

   g_return_val_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item), NULL);

   return g_strdup(item->priv->text);
}

static void
gb_source_typelib_completion_item_set_text (GbSourceTypelibCompletionItem *item,
                                            const gchar                   *text)
{
   g_return_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item));

   g_free(item->priv->text);
   item->priv->text = g_strdup(text);
}

static gchar *
gb_source_typelib_completion_item_get_markup (GtkSourceCompletionProposal *proposal)
{
   GbSourceTypelibCompletionItem *item = GB_SOURCE_TYPELIB_COMPLETION_ITEM(proposal);
   gchar *ret;

   g_return_val_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item), NULL);

   ret = highlight_substrings(item->priv->text,
                              item->priv->search_term,
                              "<span color='#333333'>",
                              "</span>");

   {
      gchar *tmp = ret;

      ret = g_strdup_printf("<span color='#777777'>%s</span>", tmp);
      g_free(tmp);
   }

   return ret;
}

static GdkPixbuf *
gb_source_typelib_completion_item_get_icon (GtkSourceCompletionProposal *proposal)
{
   GbSourceTypelibCompletionItem *item = GB_SOURCE_TYPELIB_COMPLETION_ITEM(proposal);

   g_return_val_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item), NULL);

   return item->priv->icon;
}

static void
gb_source_typelib_completion_item_set_icon (GbSourceTypelibCompletionItem *item,
                                            GdkPixbuf                     *pixbuf)
{
   g_return_if_fail(GB_IS_SOURCE_TYPELIB_COMPLETION_ITEM(item));
   g_return_if_fail(!pixbuf || GDK_IS_PIXBUF(pixbuf));

   g_clear_object(&item->priv->icon);
   item->priv->icon = pixbuf ? g_object_ref(pixbuf) : NULL;
}

static void
gb_source_typelib_completion_item_finalize (GObject *object)
{
   GbSourceTypelibCompletionItemPrivate *priv;

   priv = GB_SOURCE_TYPELIB_COMPLETION_ITEM(object)->priv;

   g_clear_object(&priv->icon);
   g_free(priv->search_term);
   g_free(priv->text);
   g_strfreev(priv->params);

   G_OBJECT_CLASS(gb_source_typelib_completion_item_parent_class)->finalize(object);
}

static void
gb_source_typelib_completion_item_get_property (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *pspec)
{
   GbSourceTypelibCompletionItem *item = GB_SOURCE_TYPELIB_COMPLETION_ITEM(object);

   switch (prop_id) {
   case PROP_MARKUP:
      g_value_take_string(value, gb_source_typelib_completion_item_get_markup(GTK_SOURCE_COMPLETION_PROPOSAL(item)));
      break;
   case PROP_ICON:
      g_value_set_object(value, gb_source_typelib_completion_item_get_icon(GTK_SOURCE_COMPLETION_PROPOSAL(item)));
      break;
   case PROP_IS_FUNCTION:
      g_value_set_boolean(value, gb_source_typelib_completion_item_get_is_function(item));
      break;
   case PROP_SEARCH_TERM:
      g_value_set_string(value, gb_source_typelib_completion_item_get_search_term(item));
      break;
   case PROP_TEXT:
      g_value_take_string(value, gb_source_typelib_completion_item_get_text(GTK_SOURCE_COMPLETION_PROPOSAL(item)));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_typelib_completion_item_set_property (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *pspec)
{
   GbSourceTypelibCompletionItem *item = GB_SOURCE_TYPELIB_COMPLETION_ITEM(object);

   switch (prop_id) {
   case PROP_ICON:
      gb_source_typelib_completion_item_set_icon(item, g_value_get_object(value));
      break;
   case PROP_IS_FUNCTION:
      gb_source_typelib_completion_item_set_is_function(item, g_value_get_boolean(value));
      break;
   case PROP_SEARCH_TERM:
      gb_source_typelib_completion_item_set_search_term(item, g_value_get_string(value));
      break;
   case PROP_TEXT:
      gb_source_typelib_completion_item_set_text(item, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_typelib_completion_item_class_init (GbSourceTypelibCompletionItemClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_typelib_completion_item_finalize;
   object_class->get_property = gb_source_typelib_completion_item_get_property;
   object_class->set_property = gb_source_typelib_completion_item_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceTypelibCompletionItemPrivate));

   gParamSpecs[PROP_TEXT] =
      g_param_spec_string("text",
                          _("Text"),
                          _("The text for the proposal."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TEXT,
                                   gParamSpecs[PROP_TEXT]);

   gParamSpecs[PROP_ICON] =
      g_param_spec_object("icon",
                          _("Icon"),
                          _("Icon"),
                          GDK_TYPE_PIXBUF,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_ICON,
                                   gParamSpecs[PROP_ICON]);

   gParamSpecs[PROP_IS_FUNCTION] =
      g_param_spec_boolean("is-function",
                          _("Is Function"),
                          _("Is this symbol a function."),
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_IS_FUNCTION,
                                   gParamSpecs[PROP_IS_FUNCTION]);

   gParamSpecs[PROP_MARKUP] =
      g_param_spec_string("markup",
                          _("Markup"),
                          _("The markup text."),
                          NULL,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MARKUP,
                                   gParamSpecs[PROP_MARKUP]);

   gParamSpecs[PROP_SEARCH_TERM] =
      g_param_spec_string("search-term",
                          _("Search Term"),
                          _("The current search term."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SEARCH_TERM,
                                   gParamSpecs[PROP_SEARCH_TERM]);
}

static void
gb_source_typelib_completion_item_init (GbSourceTypelibCompletionItem *item)
{
   item->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(item,
                                  GB_TYPE_SOURCE_TYPELIB_COMPLETION_ITEM,
                                  GbSourceTypelibCompletionItemPrivate);
}

static void
proposal_init (GtkSourceCompletionProposalIface *iface)
{
   iface->get_icon = gb_source_typelib_completion_item_get_icon;
   iface->get_text= gb_source_typelib_completion_item_get_text;
   iface->get_markup = gb_source_typelib_completion_item_get_markup;
}

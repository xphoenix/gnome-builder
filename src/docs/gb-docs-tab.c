/* gb-docs-tab.c
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

#include "gb-docs-tab.h"

G_DEFINE_TYPE(GbDocsTab, gb_docs_tab, GB_TYPE_TAB)

struct _GbDocsTabPrivate
{
   GtkWidget *revealer;
   GtkWidget *search_bar;
   GtkWidget *vbox;
   GtkWidget *webview;
};

enum
{
   PROP_0,
   PROP_LINK,
   PROP_TITLE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GtkWidget *
gb_docs_tab_new (void)
{
   return g_object_new(GB_TYPE_DOCS_TAB, NULL);
}

void
gb_docs_tab_set_link (GbDocsTab *tab,
                      DhLink    *link_)
{
   gchar *uri;

   g_return_if_fail(GB_IS_DOCS_TAB(tab));

   if (link_) {
      uri = dh_link_get_uri(link_);
      webkit_web_view_load_uri(WEBKIT_WEB_VIEW(tab->priv->webview), uri);
      g_free(uri);

      g_object_notify_by_pspec(G_OBJECT(tab), gParamSpecs[PROP_TITLE]);
   }
}

const gchar *
gb_docs_tab_get_title (GbDocsTab *tab)
{
   const gchar *ret;
   g_return_val_if_fail(GB_IS_DOCS_TAB(tab), NULL);

   ret = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->priv->webview));
   if (!ret) {
      ret = _("Empty Page");
   }

   return ret;
}

static void
on_title_changed (GbDocsTab  *tab,
                  GParamSpec *pspec,
                  GtkWidget  *webview)
{
   g_return_if_fail(GB_IS_DOCS_TAB(tab));

   g_object_notify_by_pspec(G_OBJECT(tab), gParamSpecs[PROP_TITLE]);
}

static void
gb_docs_tab_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
   GbDocsTab *tab = GB_DOCS_TAB(object);

   switch (prop_id) {
   case PROP_TITLE:
      g_value_set_string(value, gb_docs_tab_get_title(tab));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_docs_tab_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
   GbDocsTab *tab = GB_DOCS_TAB(object);

   switch (prop_id) {
   case PROP_LINK:
      gb_docs_tab_set_link(tab, g_value_get_boxed(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_docs_tab_class_init (GbDocsTabClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->get_property = gb_docs_tab_get_property;
   object_class->set_property = gb_docs_tab_set_property;
   g_type_class_add_private(object_class, sizeof(GbDocsTabPrivate));

   gParamSpecs[PROP_LINK] =
      g_param_spec_boxed("link",
                          _("Link"),
                          _("A link to view."),
                          DH_TYPE_LINK,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_LINK,
                                   gParamSpecs[PROP_LINK]);

   gParamSpecs[PROP_TITLE] =
      g_param_spec_string("title",
                          _("Title"),
                          _("The title of the tab."),
                          NULL,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TITLE,
                                   gParamSpecs[PROP_TITLE]);
}

static void
gb_docs_tab_init (GbDocsTab *tab)
{
   GbDocsTabPrivate *priv;

   tab->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE(tab,
                                  GB_TYPE_DOCS_TAB,
                                  GbDocsTabPrivate);

   priv->vbox = g_object_new(GTK_TYPE_BOX,
                             "orientation", GTK_ORIENTATION_VERTICAL,
                             "visible", TRUE,
                             NULL);
   gtk_container_add(GTK_CONTAINER(tab), priv->vbox);

   priv->revealer = g_object_new(GTK_TYPE_REVEALER,
                                 "vexpand", FALSE,
                                 "visible", FALSE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->vbox), priv->revealer);

   priv->search_bar = g_object_new(GTK_TYPE_SEARCH_BAR,
                                   "show-close-button", TRUE,
                                   "visible", TRUE,
                                   NULL);
   gtk_container_add(GTK_CONTAINER(priv->revealer), priv->search_bar);

   priv->webview = g_object_new(WEBKIT_TYPE_WEB_VIEW,
                                "hexpand", TRUE,
                                "vexpand", TRUE,
                                "visible", TRUE,
                                NULL);
   g_signal_connect_object(priv->webview,
                           "notify::title",
                           G_CALLBACK(on_title_changed),
                           tab,
                           G_CONNECT_SWAPPED);
   gtk_container_add(GTK_CONTAINER(priv->vbox), priv->webview);
}

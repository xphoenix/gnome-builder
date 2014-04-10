/* gb-tab.c
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

#include "gb-tab.h"

struct _GbTabPrivate
{
   gchar *title;
   gboolean can_save;

   GtkWidget *vbox;
   GtkWidget *info_revealer;
   GtkWidget *info_bar;
};

enum
{
   PROP_0,
   PROP_CAN_SAVE,
   PROP_TITLE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE(GbTab,
                        gb_tab,
                        GTK_TYPE_BIN,
                        G_ADD_PRIVATE(GbTab))

gboolean
gb_tab_get_can_save (GbTab *tab)
{
   g_return_val_if_fail(GB_IS_TAB(tab), FALSE);

   return tab->priv->can_save;
}

void
gb_tab_set_can_save (GbTab    *tab,
                     gboolean  can_save)
{
   g_return_if_fail(GB_IS_TAB(tab));

   tab->priv->can_save = can_save;
   g_object_notify_by_pspec(G_OBJECT(tab), gParamSpecs[PROP_CAN_SAVE]);
}

const gchar *
gb_tab_get_title (GbTab *tab)
{
   g_return_val_if_fail(GB_IS_TAB(tab), NULL);

   return tab->priv->title;
}

void
gb_tab_set_title (GbTab       *tab,
                  const gchar *title)
{
   g_return_if_fail(GB_IS_TAB(tab));

   g_free(tab->priv->title);
   tab->priv->title = g_strdup(title);
   g_object_notify_by_pspec(G_OBJECT(tab), gParamSpecs[PROP_TITLE]);
}

static void
gb_tab_add (GtkContainer *container,
            GtkWidget    *child)
{
   GbTab *tab = (GbTab *)container;

   g_return_if_fail(GB_IS_TAB(tab));
   g_return_if_fail(GTK_IS_WIDGET(child));

   if (gtk_bin_get_child(GTK_BIN(tab))) {
      gtk_container_add_with_properties(GTK_CONTAINER(tab->priv->vbox), child,
                                        "expand", TRUE,
                                        NULL);
   } else {
      GTK_CONTAINER_CLASS(gb_tab_parent_class)->add(container, child);
   }
}

static void
gb_tab_finalize (GObject *object)
{
   GbTabPrivate *priv = GB_TAB(object)->priv;

   g_clear_pointer(&priv->title, g_free);

   G_OBJECT_CLASS(gb_tab_parent_class)->finalize(object);
}

static void
gb_tab_get_property (GObject    *object,
                     guint       prop_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
   GbTab *tab = GB_TAB(object);

   switch (prop_id) {
   case PROP_TITLE:
      g_value_set_string(value, gb_tab_get_title(tab));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_tab_set_property (GObject      *object,
                     guint         prop_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
   GbTab *tab = GB_TAB(object);

   switch (prop_id) {
   case PROP_TITLE:
      gb_tab_set_title(tab, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_tab_class_init (GbTabClass *klass)
{
   GObjectClass *object_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_tab_finalize;
   object_class->get_property = gb_tab_get_property;
   object_class->set_property = gb_tab_set_property;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_tab_add;

   gParamSpecs[PROP_CAN_SAVE] =
      g_param_spec_boolean("can-save",
                           _("Can Save"),
                           _("If the tab can currently be saved."),
                           FALSE,
                           (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_SAVE,
                                   gParamSpecs[PROP_CAN_SAVE]);

   gParamSpecs[PROP_TITLE] =
      g_param_spec_string("title",
                          _("Title"),
                          _("The title of the tab."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TITLE,
                                   gParamSpecs[PROP_TITLE]);
}

static void
gb_tab_init (GbTab *tab)
{
   tab->priv = gb_tab_get_instance_private(tab);

   tab->priv->vbox = g_object_new(GTK_TYPE_BOX,
                                  "orientation", GTK_ORIENTATION_VERTICAL,
                                  "visible", TRUE,
                                  NULL);
   gtk_container_add(GTK_CONTAINER(tab), tab->priv->vbox);

   tab->priv->info_revealer = g_object_new(GTK_TYPE_REVEALER,
                                           "visible", FALSE,
                                           NULL);
   gtk_container_add(GTK_CONTAINER(tab->priv->vbox), tab->priv->info_revealer);

   tab->priv->info_bar = g_object_new(GTK_TYPE_INFO_BAR,
                                      "visible", TRUE,
                                      NULL);
   gtk_container_add(GTK_CONTAINER(tab->priv->info_revealer),
                     tab->priv->info_bar);
}

/* gb-tab-label.c
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

#include <gedit/gedit-close-button.h>
#include <glib/gi18n.h>

#include "gb-tab-label.h"

struct _GbTabLabelPrivate
{
   GtkWidget *box;
   GtkWidget *close_button;
   GtkWidget *label;
   GtkWidget *tab;
};

enum
{
   PROP_0,
   PROP_TAB,
   LAST_PROP
};

enum
{
   CLOSE_CLICKED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

G_DEFINE_TYPE_WITH_CODE(GbTabLabel,
                        gb_tab_label,
                        GTK_TYPE_BIN,
                        G_ADD_PRIVATE(GbTabLabel))

GtkWidget *
gb_tab_label_new (GbTab *tab)
{
   return g_object_new(GB_TYPE_TAB_LABEL,
                       "tab", tab,
                       NULL);
}

GtkWidget *
gb_tab_label_get_tab (GbTabLabel *label)
{
   g_return_val_if_fail(GB_IS_TAB_LABEL(label), NULL);

   return label->priv->tab;
}

static void
gb_tab_label_set_tab (GbTabLabel *label,
                      GtkWidget  *tab)
{
   GbTabLabelPrivate *priv;

   g_return_if_fail(GB_IS_TAB_LABEL(label));
   g_return_if_fail(!tab || GB_IS_TAB(tab));

   priv = label->priv;

   priv->tab = tab;

   g_object_add_weak_pointer(G_OBJECT(priv->tab), (gpointer *)&priv->tab);

   g_object_bind_property(tab, "title", priv->label, "label",
                          G_BINDING_SYNC_CREATE);
}

static void
gb_tab_label_close_clicked (GbTabLabel *label,
                            GtkWidget  *button)
{
   g_return_if_fail(GB_IS_TAB_LABEL(label));
   g_return_if_fail(GTK_IS_BUTTON(button));

   g_signal_emit(label, gSignals[CLOSE_CLICKED], 0);
}

static void
gb_tab_label_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_tab_label_parent_class)->finalize(object);
}

static void
gb_tab_label_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
   GbTabLabel *label = GB_TAB_LABEL(object);

   switch (prop_id) {
   case PROP_TAB:
      g_value_set_object(value, gb_tab_label_get_tab(label));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_tab_label_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
   GbTabLabel *label = GB_TAB_LABEL(object);

   switch (prop_id) {
   case PROP_TAB:
      gb_tab_label_set_tab(label, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_tab_label_class_init (GbTabLabelClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_tab_label_finalize;
   object_class->get_property = gb_tab_label_get_property;
   object_class->set_property = gb_tab_label_set_property;

   gParamSpecs[PROP_TAB] =
      g_param_spec_object("tab",
                          _("Tab"),
                          _("The tab for the label."),
                          GB_TYPE_TAB,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TAB,
                                   gParamSpecs[PROP_TAB]);

   gSignals[CLOSE_CLICKED] = g_signal_new("close-clicked",
                                          GB_TYPE_TAB_LABEL,
                                          G_SIGNAL_RUN_FIRST,
                                          0,
                                          NULL,
                                          NULL,
                                          g_cclosure_marshal_VOID__VOID,
                                          G_TYPE_NONE,
                                          0);
}

static void
gb_tab_label_init (GbTabLabel *label)
{
   label->priv = gb_tab_label_get_instance_private(label);

   label->priv->box = g_object_new(GTK_TYPE_BOX,
                                   "orientation", GTK_ORIENTATION_HORIZONTAL,
                                   "visible", TRUE,
                                   NULL);
   gtk_container_add(GTK_CONTAINER(label), label->priv->box);

   label->priv->label = g_object_new(GTK_TYPE_LABEL,
                                     "ellipsize", PANGO_ELLIPSIZE_END,
                                     "halign", GTK_ALIGN_CENTER,
                                     "hexpand", TRUE,
                                     "label", NULL,
                                     "single-line-mode", TRUE,
                                     "valign", GTK_ALIGN_CENTER,
                                     "visible", TRUE,
                                     NULL);
   gtk_container_add(GTK_CONTAINER(label->priv->box), label->priv->label);

   label->priv->close_button = g_object_new(GEDIT_TYPE_CLOSE_BUTTON,
                                            "visible", TRUE,
                                            NULL);
   g_signal_connect_object(label->priv->close_button,
                           "clicked",
                           G_CALLBACK(gb_tab_label_close_clicked),
                           label,
                           G_CONNECT_SWAPPED);
   gtk_container_add(GTK_CONTAINER(label->priv->box),
                     label->priv->close_button);
}

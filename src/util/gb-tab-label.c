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

#include <glib/gi18n.h>

#include "gb-tab-label.h"

G_DEFINE_TYPE(GbTabLabel, gb_tab_label, GTK_TYPE_BIN)

struct _GbTabLabelPrivate
{
   gboolean   modified;
   gchar     *title_string;

   GtkWidget *custom_title;
   GtkWidget *title;
   GtkWidget *title_container;
   GtkWidget *button;
};

enum
{
   PROP_0,
   PROP_CUSTOM_TITLE,
   PROP_TITLE,
   PROP_MODIFIED,
   LAST_PROP
};

enum
{
   CLOSE_CLICKED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

gboolean
gb_tab_label_get_modified (GbTabLabel *label)
{
   g_return_val_if_fail(GB_IS_TAB_LABEL(label), FALSE);

   return label->priv->modified;
}

void
gb_tab_label_set_modified (GbTabLabel *label,
                           gboolean    modified)
{
   GbTabLabelPrivate *priv;

   g_return_if_fail(GB_IS_TAB_LABEL(label));

   priv = label->priv;

   if (priv->title) {
#if 0
      GtkStyleContext *style_context;

      style_context = gtk_widget_get_style_context(priv->title);
      if (modified) {
         gtk_style_context_add_class(style_context, "dim-label");
      } else {
         gtk_style_context_remove_class(style_context, "dim-label");
      }
#else
      PangoAttribute *attr;
      PangoAttrList *attrs = NULL;

      /*
       * I don't really like this, but we need the designers to give us an
       * answer on how to do modified state.
       */

      if (modified) {
         attr = pango_attr_style_new(PANGO_STYLE_ITALIC);
         attrs = pango_attr_list_new();
         pango_attr_list_insert(attrs, attr);
      }

      gtk_label_set_attributes(GTK_LABEL(priv->title), attrs);

      g_clear_pointer(&attrs, (GDestroyNotify)pango_attr_list_unref);
#endif
   }
}

static void
gb_tab_label_remove_title (GbTabLabel *label)
{
   GbTabLabelPrivate *priv;

   g_return_if_fail(GB_IS_TAB_LABEL(label));

   priv = label->priv;

   if (priv->title) {
      g_object_remove_weak_pointer(G_OBJECT(priv->title),
                                   (gpointer *)&priv->title);
      gtk_container_remove(GTK_CONTAINER(priv->title_container),
                           priv->title);
      priv->title = NULL;
   }

   if (priv->custom_title) {
      g_object_remove_weak_pointer(G_OBJECT(priv->custom_title),
                                   (gpointer *)&priv->custom_title);
      gtk_container_remove(GTK_CONTAINER(priv->title_container),
                           priv->custom_title);
      priv->custom_title = NULL;
   }
}

GtkWidget *
gb_tab_label_get_custom_label (GbTabLabel *label)
{
   g_return_val_if_fail(GB_IS_TAB_LABEL(label), NULL);

   return label->priv->custom_title;
}

void
gb_tab_label_set_custom_title (GbTabLabel *label,
                               GtkWidget  *custom_title)
{
   GbTabLabelPrivate *priv;

   g_return_if_fail(GB_IS_TAB_LABEL(label));
   g_return_if_fail(GTK_IS_WIDGET(custom_title));

   priv = label->priv;

   gb_tab_label_remove_title(label);

   priv->custom_title = custom_title;
   g_object_add_weak_pointer(G_OBJECT(priv->custom_title),
                             (gpointer *)&priv->custom_title);
   gtk_container_add(GTK_CONTAINER(priv->title_container),
                     priv->custom_title);

   g_object_notify_by_pspec(G_OBJECT(label),
                            gParamSpecs[PROP_CUSTOM_TITLE]);
}

const gchar *
gb_tab_label_get_title (GbTabLabel *label)
{
   g_return_val_if_fail(GB_IS_TAB_LABEL(label), NULL);

   return label->priv->title_string;
}

void
gb_tab_label_set_title (GbTabLabel  *label,
                        const gchar *title)
{
   GbTabLabelPrivate *priv;
   GtkStyleContext *style_context;

   g_return_if_fail(GB_IS_TAB_LABEL(label));

   priv = label->priv;

   gb_tab_label_remove_title(label);

   priv->title = g_object_new(GTK_TYPE_LABEL,
                              "ellipsize", PANGO_ELLIPSIZE_NONE,
                              "halign", GTK_ALIGN_CENTER,
                              "hexpand", TRUE,
                              "label", title,
                              "single-line-mode", TRUE,
                              "use-markup", FALSE,
                              "valign", GTK_ALIGN_CENTER,
                              "visible", TRUE,
                              NULL);
   style_context = gtk_widget_get_style_context(priv->title);
   if (priv->modified) {
      gtk_style_context_add_class(style_context, "modified");
   }
   g_object_add_weak_pointer(G_OBJECT(priv->title),
                             (gpointer *)&priv->title);
   gtk_container_add(GTK_CONTAINER(priv->title_container), priv->title);

   g_clear_pointer(&priv->title_string, g_free);
   priv->title_string = g_strdup(title);

   g_object_notify_by_pspec(G_OBJECT(label), gParamSpecs[PROP_TITLE]);
}

static void
gb_tab_label_clicked (GbTabLabel *label,
                      GtkWidget  *button)
{
   g_return_if_fail(GB_IS_TAB_LABEL(label));
   g_return_if_fail(GTK_IS_BUTTON(button));

   g_signal_emit(label, gSignals[CLOSE_CLICKED], 0);
}

static void
gb_tab_label_finalize (GObject *object)
{
   GbTabLabelPrivate *priv = GB_TAB_LABEL(object)->priv;

   g_clear_pointer(&priv->title, g_free);

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
   case PROP_CUSTOM_TITLE:
      g_value_set_object(value, gb_tab_label_get_custom_label(label));
      break;
   case PROP_TITLE:
      g_value_set_string(value, gb_tab_label_get_title(label));
      break;
   case PROP_MODIFIED:
      g_value_set_boolean(value, gb_tab_label_get_modified(label));
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
   case PROP_CUSTOM_TITLE:
      gb_tab_label_set_custom_title(label, g_value_get_object(value));
      break;
   case PROP_TITLE:
      gb_tab_label_set_title(label, g_value_get_string(value));
      break;
   case PROP_MODIFIED:
      gb_tab_label_set_modified(label, g_value_get_boolean(value));
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
   g_type_class_add_private(object_class, sizeof(GbTabLabelPrivate));

   gParamSpecs[PROP_CUSTOM_TITLE] =
      g_param_spec_object("custom-title",
                          _("Custom Title"),
                          _("A custom GtkWidget label."),
                          GTK_TYPE_WIDGET,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CUSTOM_TITLE,
                                   gParamSpecs[PROP_CUSTOM_TITLE]);

   gParamSpecs[PROP_MODIFIED] =
      g_param_spec_boolean("modified",
                          _("Modified"),
                          _("If the tab contents are modified."),
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MODIFIED,
                                   gParamSpecs[PROP_MODIFIED]);

   gParamSpecs[PROP_TITLE] =
      g_param_spec_string("title",
                          _("Title"),
                          _("The title for the tab label."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TITLE,
                                   gParamSpecs[PROP_TITLE]);

   gSignals[CLOSE_CLICKED] = g_signal_new("close-clicked",
                                          GB_TYPE_TAB_LABEL,
                                          G_SIGNAL_RUN_LAST,
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
   GbTabLabelPrivate *priv;
   GtkWidget *box;
   GtkWidget *button;
   GtkWidget *icon;

   label->priv = G_TYPE_INSTANCE_GET_PRIVATE(label,
                                             GB_TYPE_TAB_LABEL,
                                             GbTabLabelPrivate);

   priv = label->priv;

   box = g_object_new(GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "visible", TRUE,
                      NULL);
   gtk_container_add(GTK_CONTAINER(label), box);

   /*
    * TODO: This just adds padding to the side of the label. this is a
    * horrible horrible hack. because we are just guessing the size of the
    * button, and because it simply wastes space. it would be nice to reuse
    * that space when you are getting shrunk down.
    */

   priv->title_container = g_object_new(GTK_TYPE_ALIGNMENT,
                                        "margin-left", 24,
                                        "hexpand", TRUE,
                                        "visible", TRUE,
                                        NULL);
   gtk_container_add(GTK_CONTAINER(box), priv->title_container);

   icon = g_object_new(GTK_TYPE_IMAGE,
                       "halign", GTK_ALIGN_CENTER,
                       "icon-name", "window-close-symbolic",
                       "icon-size", GTK_ICON_SIZE_MENU,
                       "valign", GTK_ALIGN_CENTER,
                       "visible", TRUE,
                       NULL);
   button = g_object_new(GTK_TYPE_BUTTON,
                         "child", icon,
                         "relief", GTK_RELIEF_NONE,
                         "valign", GTK_ALIGN_CENTER,
                         "visible", TRUE,
                         NULL);
   g_signal_connect_object(button,
                           "clicked",
                           G_CALLBACK(gb_tab_label_clicked),
                           label,
                           G_CONNECT_SWAPPED);
   gtk_container_add(GTK_CONTAINER(box), button);

   gb_tab_label_set_title(label, "");
}

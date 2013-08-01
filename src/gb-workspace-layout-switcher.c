/* gb-workspace-layout-switcher.c
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

#include "gb-workspace.h"
#include "gb-workspace-layout-edit.h"
#include "gb-workspace-layout-switcher.h"

G_DEFINE_TYPE(GbWorkspaceLayoutSwitcher,
              gb_workspace_layout_switcher,
              GTK_TYPE_BUTTON_BOX)

struct _GbWorkspaceLayoutSwitcherPrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
item_toggled (GtkWidget *button,
              gpointer   pmode)
{
   GbWorkspaceMode mode = GPOINTER_TO_INT(pmode);
   GtkWidget *toplevel;
   gboolean active;

   g_assert(GTK_IS_WIDGET(button));

   g_object_get(button, "active", &active, NULL);

   if (active) {
      toplevel = gtk_widget_get_toplevel(button);
      if (GB_IS_WORKSPACE(toplevel)) {
         gb_workspace_set_mode(GB_WORKSPACE(toplevel), mode);
      }
   }
}

static void
gb_workspace_layout_switcher_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_layout_switcher_parent_class)->finalize(object);
}

static void
gb_workspace_layout_switcher_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
   //GbWorkspaceLayoutSwitcher *switcher = GB_WORKSPACE_LAYOUT_SWITCHER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_layout_switcher_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
   //GbWorkspaceLayoutSwitcher *switcher = GB_WORKSPACE_LAYOUT_SWITCHER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_layout_switcher_class_init (GbWorkspaceLayoutSwitcherClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_layout_switcher_finalize;
   object_class->get_property = gb_workspace_layout_switcher_get_property;
   object_class->set_property = gb_workspace_layout_switcher_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceLayoutSwitcherPrivate));
}

static void
gb_workspace_layout_switcher_init (GbWorkspaceLayoutSwitcher *switcher)
{
   switcher->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(switcher,
                                  GB_TYPE_WORKSPACE_LAYOUT_SWITCHER,
                                  GbWorkspaceLayoutSwitcherPrivate);

   gtk_style_context_add_class(
      gtk_widget_get_style_context(GTK_WIDGET(switcher)),
      GTK_STYLE_CLASS_LINKED);

   {
      GtkWidget *b;

      b = g_object_new(GTK_TYPE_RADIO_BUTTON,
                       "draw-indicator", FALSE,
                       "label", _("UI"),
                       "visible", TRUE,
                       NULL);
      g_signal_connect(b, "toggled", G_CALLBACK(item_toggled),
                       GINT_TO_POINTER(GB_WORKSPACE_UI));
      gtk_container_add(GTK_CONTAINER(switcher), b);

      b = g_object_new(GTK_TYPE_RADIO_BUTTON,
                       "draw-indicator", FALSE,
                       "group", b,
                       "label", _("Edit"),
                       "visible", TRUE,
                       NULL);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b), TRUE);
      g_signal_connect(b, "toggled", G_CALLBACK(item_toggled),
                       GINT_TO_POINTER(GB_WORKSPACE_EDIT));
      gtk_container_add(GTK_CONTAINER(switcher), b);

      b = g_object_new(GTK_TYPE_RADIO_BUTTON,
                       "draw-indicator", FALSE,
                       "group", b,
                       "label", _("Debug"),
                       "visible", TRUE,
                       NULL);
      g_signal_connect(b, "toggled", G_CALLBACK(item_toggled),
                       GINT_TO_POINTER(GB_WORKSPACE_DEBUG));
      gtk_container_add(GTK_CONTAINER(switcher), b);

      b = g_object_new(GTK_TYPE_RADIO_BUTTON,
                       "draw-indicator", FALSE,
                       "group", b,
                       "label", _("VCS"),
                       "visible", TRUE,
                       NULL);
      g_signal_connect(b, "toggled", G_CALLBACK(item_toggled),
                       GINT_TO_POINTER(GB_WORKSPACE_VCS));
      gtk_container_add(GTK_CONTAINER(switcher), b);

      b = g_object_new(GTK_TYPE_RADIO_BUTTON,
                       "draw-indicator", FALSE,
                       "group", b,
                       "label", _("Docs"),
                       "visible", TRUE,
                       NULL);
      g_signal_connect(b, "toggled", G_CALLBACK(item_toggled),
                       GINT_TO_POINTER(GB_WORKSPACE_DOCS));
      gtk_container_add(GTK_CONTAINER(switcher), b);
   }
}

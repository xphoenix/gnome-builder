/* gb-glade-panel.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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
#include <gladeui/glade.h>
#include <ide.h>

#include "gb-glade-panel.h"

struct _GbGladePanel
{
  GtkBox          parent_instance;

  GladeEditor    *editor;
  GladeInspector *inspector;
};

G_DEFINE_TYPE (GbGladePanel, gb_glade_panel, GTK_TYPE_BOX)

enum {
  PROP_0,
  LAST_PROP
};

static GParamSpec *gParamSpecs [LAST_PROP];

static void
fixup_inspector_styling (GtkWidget *widget,
                         gpointer   user_data)
{
  if (GTK_IS_ENTRY (widget))
    {
      GtkWidget *parent;

      parent = gtk_widget_get_parent (widget);
      gtk_container_child_set (GTK_CONTAINER (parent), widget, "padding", 0, NULL);

      g_object_set (widget,
                    "primary-icon-name", "edit-find-symbolic",
                    "placeholder-text", NULL,
                    "secondary-icon-name", NULL,
                    NULL);
    }
  else if (GTK_IS_SCROLLED_WINDOW (widget))
    {
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_NONE);
    }
}

static void
fixup_editor_styling (GtkWidget *widget,
                      gpointer   user_data)
{
  if (GTK_IS_NOTEBOOK (widget))
    {
      gtk_notebook_set_show_border (GTK_NOTEBOOK (widget), FALSE);
    }
}

static void
gb_glade_panel_finalize (GObject *object)
{
  G_OBJECT_CLASS (gb_glade_panel_parent_class)->finalize (object);
}

static void
gb_glade_panel_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GbGladePanel *self = GB_GLADE_PANEL (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gb_glade_panel_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GbGladePanel *self = GB_GLADE_PANEL (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gb_glade_panel_class_init (GbGladePanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gb_glade_panel_finalize;
  object_class->get_property = gb_glade_panel_get_property;
  object_class->set_property = gb_glade_panel_set_property;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/builder/plugins/glade/gb-glade-panel.ui");
  gtk_widget_class_bind_template_child (widget_class, GbGladePanel, editor);
  gtk_widget_class_bind_template_child (widget_class, GbGladePanel, inspector);
}

static void
gb_glade_panel_init (GbGladePanel *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_container_foreach (GTK_CONTAINER (self->editor),
                         fixup_editor_styling,
                         NULL);
  gtk_container_foreach (GTK_CONTAINER (self->inspector),
                         fixup_inspector_styling,
                         NULL);
}

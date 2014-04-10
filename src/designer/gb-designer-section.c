/* gb-designer-section.c
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

#include <gladeui/glade.h>
#include <glib/gi18n.h>

#include "gb-designer-section.h"

struct _GbDesignerSectionPrivate
{
   GladeApp *app;

   GladeProject *project;

   GtkWidget *editor;
   GtkWidget *hpaned1;
   GtkWidget *hpaned2;
   GtkWidget *inspector;
   GtkWidget *notebook;
   GtkWidget *palette;
   GtkWidget *vpaned1;
};

G_DEFINE_TYPE_WITH_CODE(GbDesignerSection,
                        gb_designer_section,
                        GB_TYPE_WORKSPACE_SECTION,
                        G_ADD_PRIVATE(GbDesignerSection))

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_designer_section_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_designer_section_parent_class)->finalize(object);
}

static void
gb_designer_section_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
   //GbDesignerSection *section = GB_DESIGNER_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_designer_section_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
   //GbDesignerSection *section = GB_DESIGNER_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_designer_section_class_init (GbDesignerSectionClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_designer_section_finalize;
   object_class->get_property = gb_designer_section_get_property;
   object_class->set_property = gb_designer_section_set_property;
}

static void
gb_designer_section_init (GbDesignerSection *section)
{
   GbDesignerSectionPrivate *priv;

   priv = section->priv = gb_designer_section_get_instance_private(section);

   priv->app = glade_app_new();
   priv->project = glade_project_new();

   priv->hpaned1 = g_object_new(GTK_TYPE_PANED,
                                "orientation", GTK_ORIENTATION_HORIZONTAL,
                                "position", 200,
                                "visible", TRUE,
                                NULL);
   gtk_container_add(GTK_CONTAINER(section), priv->hpaned1);

   priv->palette = g_object_new(GLADE_TYPE_PALETTE,
                                "visible", TRUE,
                                NULL);
   gtk_paned_add1(GTK_PANED(priv->hpaned1), priv->palette);

   priv->hpaned2 = g_object_new(GTK_TYPE_PANED,
                                "orientation", GTK_ORIENTATION_HORIZONTAL,
                                "position", 325,
                                "visible", TRUE,
                                NULL);
   gtk_paned_add2(GTK_PANED(priv->hpaned1), priv->hpaned2);

   priv->notebook = g_object_new(GTK_TYPE_NOTEBOOK,
                                 "hexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->hpaned2),
                                     priv->notebook,
                                     "resize", TRUE,
                                     NULL);

   {
      /*
       * XXX: Temporary.
       */

      GtkWidget *designer;

      designer = g_object_new (GLADE_TYPE_DESIGN_VIEW,
                               "project", priv->project,
                               "visible", TRUE,
                               NULL);
      gtk_container_add (GTK_CONTAINER (priv->notebook), designer);
   }

   priv->vpaned1 = g_object_new(GTK_TYPE_PANED,
                                "orientation", GTK_ORIENTATION_VERTICAL,
                                "position", 325,
                                "visible", TRUE,
                                "width-request", 350,
                                NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->hpaned2),
                                     priv->vpaned1,
                                     "resize", FALSE,
                                     NULL);

   priv->inspector = g_object_new(GLADE_TYPE_INSPECTOR,
                                  "project", priv->project,
                                  "visible", TRUE,
                                  NULL);
   gtk_paned_add1(GTK_PANED(priv->vpaned1), priv->inspector);

   priv->editor = g_object_new(GLADE_TYPE_EDITOR,
                               "project", priv->project,
                               "visible", TRUE,
                               NULL);
   gtk_paned_add2(GTK_PANED(priv->vpaned1), priv->editor);
}

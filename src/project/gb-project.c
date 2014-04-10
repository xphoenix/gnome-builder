/* gb-project.c
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

#include "gb-project.h"

struct _GbProjectPrivate
{
   gchar *name;
};

G_DEFINE_TYPE_WITH_CODE(GbProject,
                        gb_project,
                        G_TYPE_OBJECT,
                        G_ADD_PRIVATE(GbProject))

enum
{
   PROP_0,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbProject *
gb_project_new (void)
{
   return g_object_new(GB_TYPE_PROJECT, NULL);
}

const gchar *
gb_project_get_name (GbProject *project)
{
   g_return_val_if_fail(GB_IS_PROJECT(project), NULL);

   return project->priv->name;
}

void
gb_project_set_name (GbProject   *project,
                     const gchar *name)
{
   g_return_if_fail(GB_IS_PROJECT(project));

   g_free(project->priv->name);
   project->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(project), gParamSpecs[PROP_NAME]);
}

static void
gb_project_finalize (GObject *object)
{
   GbProjectPrivate *priv;

   priv = GB_PROJECT(object)->priv;

   g_clear_pointer(&priv->name, g_free);

   G_OBJECT_CLASS(gb_project_parent_class)->finalize(object);
}

static void
gb_project_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
   GbProject *project = GB_PROJECT(object);

   switch (prop_id) {
   case PROP_NAME:
      g_value_set_string(value, gb_project_get_name(project));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_project_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
   GbProject *project = GB_PROJECT(object);

   switch (prop_id) {
   case PROP_NAME:
      gb_project_set_name(project, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_project_class_init (GbProjectClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_project_finalize;
   object_class->get_property = gb_project_get_property;
   object_class->set_property = gb_project_set_property;

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the project."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_project_init (GbProject *project)
{
   project->priv = gb_project_get_instance_private(project);
   project->priv->name = g_strdup(_("New Project"));
}

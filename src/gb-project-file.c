/* gb-project-file.c
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

#include "gb-project-file.h"

G_DEFINE_TYPE(GbProjectFile, gb_project_file, GB_TYPE_PROJECT_ITEM)

struct _GbProjectFilePrivate
{
   gchar *path;
};

enum
{
   PROP_0,
   PROP_PATH,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_project_file_get_path (GbProjectFile *file)
{
   g_return_val_if_fail(GB_IS_PROJECT_FILE(file), NULL);
   return file->priv->path;
}

void
gb_project_file_set_path (GbProjectFile *file,
                          const gchar   *path)
{
   g_return_if_fail(GB_IS_PROJECT_FILE(file));

   g_free(file->priv->path);
   file->priv->path = g_strdup(path);
   g_object_notify_by_pspec(G_OBJECT(file), gParamSpecs[PROP_PATH]);
}

static void
gb_project_file_finalize (GObject *object)
{
   GbProjectFilePrivate *priv;

   priv = GB_PROJECT_FILE(object)->priv;

   g_clear_pointer(&priv->path, g_free);

   G_OBJECT_CLASS(gb_project_file_parent_class)->finalize(object);
}

static void
gb_project_file_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
   GbProjectFile *file = GB_PROJECT_FILE(object);

   switch (prop_id) {
   case PROP_PATH:
      g_value_set_string(value, gb_project_file_get_path(file));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_project_file_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
   GbProjectFile *file = GB_PROJECT_FILE(object);

   switch (prop_id) {
   case PROP_PATH:
      gb_project_file_set_path(file, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_project_file_class_init (GbProjectFileClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_project_file_finalize;
   object_class->get_property = gb_project_file_get_property;
   object_class->set_property = gb_project_file_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectFilePrivate));

   gParamSpecs[PROP_PATH] =
      g_param_spec_string("path",
                          _("Path"),
                          _("The path to the file."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_PATH,
                                   gParamSpecs[PROP_PATH]);
}

static void
gb_project_file_init (GbProjectFile *file)
{
   file->priv = G_TYPE_INSTANCE_GET_PRIVATE(file,
                                            GB_TYPE_PROJECT_FILE,
                                            GbProjectFilePrivate);
}

/* gb-project-file.c
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
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

#include "gb-log.h"
#include "gb-project.h"
#include "gb-project-file.h"

G_DEFINE_TYPE(GbProjectFile, gb_project_file, GB_TYPE_PROJECT_ITEM)

struct _GbProjectFilePrivate
{
   gchar *path;
   GbProjectFileMode mode;
};

enum
{
   PROP_0,
   PROP_FILE,
   PROP_ICON_NAME,
   PROP_MODE,
   PROP_PATH,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

/**
 * gb_project_file_get_file:
 * @file: (in): A #GbProjectFile.
 *
 * Fetches a #GFile that represents the particular file.
 *
 * Returns: (transfer full): A #GFile.
 */
GFile *
gb_project_file_get_file (GbProjectFile *file)
{
   GbProjectItem *project;
   GFile *ret;
   gchar *path;

   ENTRY;

   g_return_val_if_fail(GB_IS_PROJECT_FILE(file), NULL);

   project = gb_project_item_get_toplevel(GB_PROJECT_ITEM(file));
   g_assert(GB_IS_PROJECT(project));

   path = g_build_filename(gb_project_get_directory(GB_PROJECT(project)),
                           file->priv->path,
                           NULL);
   ret = g_file_new_for_path(path);
   g_free(path);

   RETURN(ret);
}

const gchar *
gb_project_file_get_icon_name (GbProjectFile *file)
{
   const gchar *icon_name;

   g_return_val_if_fail(GB_IS_PROJECT_FILE(file), NULL);

#if 0
   if (!(icon_name = gb_path_get_icon_name(file->priv->path))) {
      icon_name = GTK_STOCK_FILE;
   }
#else
   icon_name = "file";
#endif

   return icon_name;
}

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
   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_FILE(file));

   g_free(file->priv->path);
   file->priv->path = g_strdup(path);
   g_object_notify_by_pspec(G_OBJECT(file), gParamSpecs[PROP_PATH]);

   EXIT;
}

GbProjectFileMode
gb_project_file_get_mode (GbProjectFile *file)
{
   g_return_val_if_fail(GB_IS_PROJECT_FILE(file), 0);
   return file->priv->mode;
}

void
gb_project_file_set_mode (GbProjectFile     *file,
                          GbProjectFileMode  mode)
{
   g_return_if_fail(GB_IS_PROJECT_FILE(file));
   g_return_if_fail(mode >= GB_PROJECT_FILE_REGULAR);
   g_return_if_fail(mode <= GB_PROJECT_FILE_SOURCE);

   file->priv->mode = mode;
   g_object_notify_by_pspec(G_OBJECT(file), gParamSpecs[PROP_MODE]);
}

/**
 * gb_project_file_finalize:
 * @object: (in): A #GbProjectFile.
 *
 * Finalizer for a #GbProjectFile instance.  Frees any resources held by
 * the instance.
 */
static void
gb_project_file_finalize (GObject *object)
{
   GbProjectFilePrivate *priv = GB_PROJECT_FILE(object)->priv;

   ENTRY;
   g_free(priv->path);
   G_OBJECT_CLASS(gb_project_file_parent_class)->finalize(object);
   EXIT;
}

/**
 * gb_project_file_get_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
gb_project_file_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
   GbProjectFile *file = GB_PROJECT_FILE(object);

   switch (prop_id) {
   case PROP_FILE:
      g_value_take_object(value, gb_project_file_get_file(file));
      break;
   case PROP_ICON_NAME:
      g_value_set_string(value, gb_project_file_get_icon_name(file));
      break;
   case PROP_MODE:
      g_value_set_enum(value, gb_project_file_get_mode(file));
      break;
   case PROP_PATH:
      g_value_set_string(value, gb_project_file_get_path(file));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_file_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_project_file_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
   GbProjectFile *file = GB_PROJECT_FILE(object);

   switch (prop_id) {
   case PROP_MODE:
      gb_project_file_set_mode(file, g_value_get_enum(value));
      break;
   case PROP_PATH:
      gb_project_file_set_path(file, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_file_class_init:
 * @klass: (in): A #GbProjectFileClass.
 *
 * Initializes the #GbProjectFileClass and prepares the vtable.
 */
static void
gb_project_file_class_init (GbProjectFileClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_project_file_finalize;
   object_class->get_property = gb_project_file_get_property;
   object_class->set_property = gb_project_file_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectFilePrivate));

   /**
    * GbProjectFile:file:
    *
    * The "file" property contains a #GFile.
    */
   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("A GFile for the project file."),
                          G_TYPE_FILE,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);

   /**
    * GbProjectFile:icon-name:
    *
    * The "icon-name" property describes what icon to display next to the
    * file in project browsers or views.
    */
   gParamSpecs[PROP_ICON_NAME] =
      g_param_spec_string("icon-name",
                          _("Icon Name"),
                          _("The name of the icon to display with the file."),
                          NULL,
                          (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_ICON_NAME,
                                   gParamSpecs[PROP_ICON_NAME]);

   /**
    * GbProjectFile:mode:
    *
    * The "mode" of the file within the project.
    */
   gParamSpecs[PROP_MODE] =
      g_param_spec_enum("mode",
                        _("Mode"),
                        _("The mode of the file in the project."),
                        GB_TYPE_PROJECT_FILE_MODE,
                        GB_PROJECT_FILE_REGULAR,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_MODE,
                                   gParamSpecs[PROP_MODE]);

   /**
    * GbProjectFile:path:
    *
    * The path to the file within the project.
    */
   gParamSpecs[PROP_PATH] =
      g_param_spec_string("path",
                          _("Path"),
                          _("The path to the file."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PATH,
                                   gParamSpecs[PROP_PATH]);
}

/**
 * gb_project_file_init:
 * @file: (in): A #GbProjectFile.
 *
 * Initializes the newly created #GbProjectFile instance.
 */
static void
gb_project_file_init (GbProjectFile *file)
{
   ENTRY;
   file->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(file,
                                  GB_TYPE_PROJECT_FILE,
                                  GbProjectFilePrivate);
   EXIT;
}

GType
gb_project_file_mode_get_type (void)
{
   static gsize initialized = FALSE;
   static GType type_id = 0;
   static const GEnumValue values[] = {
      { GB_PROJECT_FILE_REGULAR, "GB_PROJECT_FILE_REGULAR", "REGULAR" },
      { GB_PROJECT_FILE_SOURCE, "GB_PROJECT_FILE_SOURCE", "SOURCE" },
      { 0 }
   };

   if (g_once_init_enter(&initialized)) {
      type_id = g_enum_register_static("GbProjectFileMode", values);
      g_once_init_leave(&initialized, TRUE);
   }

   return type_id;
}

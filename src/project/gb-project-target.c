/* gb-project-target.c
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
#include "gb-project-file.h"
#include "gb-project-group.h"
#include "gb-project-target.h"

G_DEFINE_TYPE(GbProjectTarget, gb_project_target, GB_TYPE_PROJECT_ITEM)

struct _GbProjectTargetPrivate
{
   GbProjectGroup *files;
   gchar *directory;
   gchar *name;
};

enum
{
   PROP_0,
   PROP_DIRECTORY,
   PROP_FILES,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_project_target_get_directory (GbProjectTarget *target)
{
   g_return_val_if_fail(GB_IS_PROJECT_TARGET(target), NULL);
   return target->priv->directory;
}

void
gb_project_target_set_directory (GbProjectTarget *target,
                            const gchar     *directory)
{
   g_return_if_fail(GB_IS_PROJECT_TARGET(target));

   g_free(target->priv->directory);
   target->priv->directory = g_strdup(directory);
   g_object_notify_by_pspec(G_OBJECT(target), gParamSpecs[PROP_DIRECTORY]);
}

const gchar *
gb_project_target_get_name (GbProjectTarget *target)
{
   g_return_val_if_fail(GB_IS_PROJECT_TARGET(target), NULL);
   return target->priv->name;
}

void
gb_project_target_set_name (GbProjectTarget *target,
                            const gchar     *name)
{
   g_return_if_fail(GB_IS_PROJECT_TARGET(target));

   g_free(target->priv->name);
   target->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(target), gParamSpecs[PROP_NAME]);
}

/**
 * gb_project_target_get_files:
 * @target: (in): A #GbProjectTarget.
 *
 * Gets a #GbProjectGroup containing the files for the target.
 *
 * Returns: (transfer none): A #GbProjectGroup.
 */
GbProjectGroup *
gb_project_target_get_files (GbProjectTarget *target)
{
   g_return_val_if_fail(GB_IS_PROJECT_TARGET(target), NULL);
   return target->priv->files;
}

static void
gb_project_target_set_files (GbProjectTarget *target,
                             GbProjectGroup  *group)
{
   GbProjectTargetPrivate *priv;

   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_TARGET(target));

   priv = target->priv;

   g_clear_object(&priv->files);
   g_object_set(group, "item-type", GB_TYPE_PROJECT_FILE, NULL);
   priv->files = g_object_ref_sink(group);
   g_object_notify_by_pspec(G_OBJECT(target), gParamSpecs[PROP_FILES]);

   EXIT;
}

/**
 * gb_project_target_dispose:
 * @object: (in): A #GbProjectTarget.
 *
 * Dispose for #GbProjectTarget. Releases referenced objects.
 */
static void
gb_project_target_dispose (GObject *object)
{
   GbProjectTargetPrivate *priv = GB_PROJECT_TARGET(object)->priv;

   ENTRY;
   g_clear_object(&priv->files);
   G_OBJECT_CLASS(gb_project_target_parent_class)->dispose(object);
   EXIT;
}

/**
 * gb_project_target_finalize:
 * @object: (in): A #GbProjectTarget.
 *
 * Finalizer for a #GbProjectTarget instance. Frees any resources held by
 * the instance.
 */
static void
gb_project_target_finalize (GObject *object)
{
   GbProjectTargetPrivate *priv = GB_PROJECT_TARGET(object)->priv;

   ENTRY;
   g_free(priv->name);
   g_free(priv->directory);
   G_OBJECT_CLASS(gb_project_target_parent_class)->finalize(object);
   EXIT;
}

/**
 * gb_project_target_get_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
gb_project_target_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
   GbProjectTarget *target = GB_PROJECT_TARGET(object);

   switch (prop_id) {
   case PROP_DIRECTORY:
      g_value_set_string(value, gb_project_target_get_directory(target));
      break;
   case PROP_FILES:
      g_value_set_object(value, gb_project_target_get_files(target));
      break;
   case PROP_NAME:
      g_value_set_string(value, gb_project_target_get_name(target));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_target_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_project_target_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
   GbProjectTarget *target = GB_PROJECT_TARGET(object);

   switch (prop_id) {
   case PROP_DIRECTORY:
      gb_project_target_set_directory(target, g_value_get_string(value));
      break;
   case PROP_FILES:
      gb_project_target_set_files(target, g_value_get_object(value));
      break;
   case PROP_NAME:
      gb_project_target_set_name(target, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_target_class_init:
 * @klass: (in): A #GbProjectTargetClass.
 *
 * Initializes the #GbProjectTargetClass and prepares the vtable.
 */
static void
gb_project_target_class_init (GbProjectTargetClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_project_target_dispose;
   object_class->finalize = gb_project_target_finalize;
   object_class->get_property = gb_project_target_get_property;
   object_class->set_property = gb_project_target_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectTargetPrivate));

   gParamSpecs[PROP_DIRECTORY] =
      g_param_spec_string("directory",
                          _("Directory"),
                          _("Directory"),
                          NULL,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_DIRECTORY,
                                   gParamSpecs[PROP_DIRECTORY]);

   /**
    * GbProjectTarget:files:
    *
    * The files for the project.
    */
   gParamSpecs[PROP_FILES] =
      g_param_spec_object("files",
                          _("Files"),
                          _("The files for the target."),
                          GB_TYPE_PROJECT_GROUP,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_FILES,
                                   gParamSpecs[PROP_FILES]);

   /**
    * GbProjectTarget:name:
    *
    * The name of the target.
    */
   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("Name"),
                          NULL,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

/**
 * gb_project_target_init:
 * @target: (in): A #GbProjectTarget.
 *
 * Initializes the newly created #GbProjectTarget instance.
 */
static void
gb_project_target_init (GbProjectTarget *target)
{
   ENTRY;
   target->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(target,
                                  GB_TYPE_PROJECT_TARGET,
                                  GbProjectTargetPrivate);
   EXIT;
}

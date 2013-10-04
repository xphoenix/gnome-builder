/* gb-project.c
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

#include <gdk/gdk.h>
#include <glib/gi18n.h>

#include "gb-log.h"
#include "gb-project.h"
#include "gb-project-file.h"
#include "gb-project-target.h"

G_DEFINE_TYPE(GbProject, gb_project, GB_TYPE_PROJECT_ITEM)

struct _GbProjectPrivate
{
   GbProjectGroup *files;
   GbProjectGroup *targets;

   gchar *directory;
   gchar *name;
   gchar *version;
};

enum
{
   PROP_0,
   PROP_DIRECTORY,
   PROP_FILES,
   PROP_NAME,
   PROP_TARGETS,
   PROP_VERSION,
   LAST_PROP
};

enum
{
   LOADED,
   SAVED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

/**
 * gb_project_get_files:
 * @project: (in): A #GbProject.
 *
 * Gets a group of #GbProjectTarget instances.
 *
 * Returns: (transfer none): A #GbProjectGroup.
 */
GbProjectGroup *
gb_project_get_files (GbProject *project)
{
   g_return_val_if_fail(GB_IS_PROJECT(project), NULL);
   return project->priv->files;
}

static void
gb_project_set_files (GbProject      *project,
                      GbProjectGroup *files)
{
   GbProjectPrivate *priv;

   g_return_if_fail(GB_IS_PROJECT(project));
   g_return_if_fail(GB_IS_PROJECT_GROUP(files));

   priv = project->priv;

   g_object_set(files, "item-type", GB_TYPE_PROJECT_FILE, NULL);
   g_clear_object(&priv->files);
   priv->files = g_object_ref_sink(files);
   g_object_notify_by_pspec(G_OBJECT(project), gParamSpecs[PROP_FILES]);
}

/**
 * gb_project_get_targets:
 * @project: (in): A #GbProject.
 *
 * Gets a group of #GbProjectTarget instances.
 *
 * Returns: (transfer none): A #GbProjectGroup.
 */
GbProjectGroup *
gb_project_get_targets (GbProject *project)
{
   g_return_val_if_fail(GB_IS_PROJECT(project), NULL);
   return project->priv->targets;
}

static void
gb_project_set_targets (GbProject      *project,
                        GbProjectGroup *targets)
{
   GbProjectPrivate *priv;

   g_return_if_fail(GB_IS_PROJECT(project));
   g_return_if_fail(GB_IS_PROJECT_GROUP(targets));

   priv = project->priv;

   g_object_set(targets, "item-type", GB_TYPE_PROJECT_TARGET, NULL);
   g_clear_object(&priv->targets);
   priv->targets = g_object_ref_sink(targets);
   g_object_notify_by_pspec(G_OBJECT(project), gParamSpecs[PROP_TARGETS]);
}

/**
 * gb_project_get_directory:
 * @project: (in): A #GbProject.
 *
 * Get the directory that contains the project. The project information file
 * is contained in a regular file named .gbproject.
 *
 * Returns: A string containing the directory path.
 */
const gchar *
gb_project_get_directory (GbProject *project)
{
   g_return_val_if_fail(GB_IS_PROJECT(project), NULL);
   return project->priv->directory;
}

/**
 * gb_project_set_directory:
 * @project: (in): A #GbProject.
 * @directory: (in): A string containing the directory.
 *
 * Sets the directory to use for the project.
 */
static void
gb_project_set_directory (GbProject   *project,
                          const gchar *directory)
{
   g_return_if_fail(GB_IS_PROJECT(project));

   g_free(project->priv->directory);
   project->priv->directory = g_strdup(directory);
   g_object_notify_by_pspec(G_OBJECT(project), gParamSpecs[PROP_DIRECTORY]);
}

/**
 * gb_project_get_name:
 * @project: (in): A #GbProject.
 *
 * Get the name of the project.
 *
 * Returns: A string containing the project name.
 */
const gchar *
gb_project_get_name (GbProject *project)
{
   g_return_val_if_fail(GB_IS_PROJECT(project), NULL);
   return project->priv->name;
}

/**
 * gb_project_set_name:
 * @project: (in): A #GbProject.
 * @name: (in): A string.
 *
 * Set the name of the project.
 */
void
gb_project_set_name (GbProject   *project,
                     const gchar *name)
{
   g_return_if_fail(GB_IS_PROJECT(project));

   g_free(project->priv->name);
   project->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(project), gParamSpecs[PROP_NAME]);
}

/**
 * gb_project_get_version:
 * @project: (in): A #GbProject.
 *
 * Get the version of the project. This should be in the format of
 * "major.minor.micro" such as "0.1.2".
 *
 * Returns: A string containing the version.
 */
const gchar *
gb_project_get_version (GbProject *project)
{
   g_return_val_if_fail(GB_IS_PROJECT(project), NULL);
   return project->priv->version;
}

/**
 * gb_project_set_version:
 * @project: (in): A #GbProject.
 * @version: (in): A string containing a major.minor.micro version.
 *
 * Sets the version of the program,
 */
void
gb_project_set_version (GbProject   *project,
                        const gchar *version)
{
   g_return_if_fail(GB_IS_PROJECT(project));

   g_free(project->priv->version);
   project->priv->version = g_strdup(version);
   g_object_notify_by_pspec(G_OBJECT(project), gParamSpecs[PROP_VERSION]);
}

/**
 * gb_project_dispose:
 * @object: (in): A #GbProject.
 *
 * Dispose for a #GbProject instance. Frees objects held by the instance.
 */
static void
gb_project_dispose (GObject *object)
{
   GbProjectPrivate *priv;

   ENTRY;

   priv = GB_PROJECT(object)->priv;
   g_clear_object(&priv->targets);
   G_OBJECT_CLASS(gb_project_parent_class)->dispose(object);

   EXIT;
}

/**
 * gb_project_finalize:
 * @object: (in): A #GbProject.
 *
 * Finalizer for a #GbProject instance.  Frees any resources held by
 * the instance.
 */
static void
gb_project_finalize (GObject *object)
{
   GbProjectPrivate *priv;

   ENTRY;

   priv = GB_PROJECT(object)->priv;

   g_free(priv->directory);
   g_free(priv->name);
   g_free(priv->version);

   G_OBJECT_CLASS(gb_project_parent_class)->finalize(object);

   EXIT;
}

/**
 * gb_project_get_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
gb_project_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
   GbProject *project = GB_PROJECT(object);

   switch (prop_id) {
   case PROP_DIRECTORY:
      g_value_set_string(value, gb_project_get_directory(project));
      break;
   case PROP_FILES:
      g_value_set_object(value, gb_project_get_files(project));
      break;
   case PROP_NAME:
      g_value_set_string(value, gb_project_get_name(project));
      break;
   case PROP_TARGETS:
      g_value_set_object(value, gb_project_get_targets(project));
      break;
   case PROP_VERSION:
      g_value_set_string(value, gb_project_get_version(project));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_project_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
   GbProject *project = GB_PROJECT(object);

   switch (prop_id) {
   case PROP_DIRECTORY:
      gb_project_set_directory(project, g_value_get_string(value));
      break;
   case PROP_FILES:
      gb_project_set_files(project, g_value_get_object(value));
      break;
   case PROP_NAME:
      gb_project_set_name(project, g_value_get_string(value));
      break;
   case PROP_TARGETS:
      gb_project_set_targets(project, g_value_get_object(value));
      break;
   case PROP_VERSION:
      gb_project_set_version(project, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_class_init:
 * @klass: (in): A #GbProjectClass.
 *
 * Initializes the #GbProjectClass and prepares the vtable.
 */
static void
gb_project_class_init (GbProjectClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_project_dispose;
   object_class->finalize = gb_project_finalize;
   object_class->get_property = gb_project_get_property;
   object_class->set_property = gb_project_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectPrivate));

   /**
    * GbProject:directory:
    *
    * The directory containing the project.
    */
   gParamSpecs[PROP_DIRECTORY] =
      g_param_spec_string("directory",
                          _("Directory"),
                          _("The directory for the project."),
                          NULL,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_DIRECTORY,
                                   gParamSpecs[PROP_DIRECTORY]);

   /**
    * GbProject:files:
    *
    * The files in the project that are not part of a target.
    */
   gParamSpecs[PROP_FILES] =
      g_param_spec_object("files",
                          _("Files"),
                          _("Non-target files for the project."),
                          GB_TYPE_PROJECT_GROUP,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_FILES,
                                   gParamSpecs[PROP_FILES]);

   /**
    * GbProject:name:
    *
    * The name of the project.
    */
   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the project."),
                          NULL,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);

   /**
    * GbProject:targets:
    *
    * The targets of the project.
    */
   gParamSpecs[PROP_TARGETS] =
      g_param_spec_object("targets",
                          _("Targets"),
                          _("The group of targets for the project."),
                          GB_TYPE_PROJECT_GROUP,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_TARGETS,
                                   gParamSpecs[PROP_TARGETS]);

   /**
    * GbProject:version:
    *
    * The version of the project.
    */
   gParamSpecs[PROP_VERSION] =
      g_param_spec_string("version",
                          _("Version"),
                          _("The version of the project."),
                          NULL,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_VERSION,
                                   gParamSpecs[PROP_VERSION]);

   /**
    * GbProject:loaded:
    *
    * The "loaded" signal is emitted when the project has been successfully
    * loaded from disk.
    */
   gSignals[LOADED] = g_signal_new("loaded",
                                   GB_TYPE_PROJECT,
                                   G_SIGNAL_RUN_FIRST,
                                   0,
                                   NULL,
                                   NULL,
                                   g_cclosure_marshal_VOID__VOID,
                                   G_TYPE_NONE,
                                   0);

   /**
    * GbProject::saved:
    *
    * The "saved" signal is emitted when the project has been successfully
    * saved to disk.
    */
   gSignals[SAVED] = g_signal_new("saved",
                                  GB_TYPE_PROJECT,
                                  G_SIGNAL_RUN_FIRST,
                                  0,
                                  NULL,
                                  NULL,
                                  g_cclosure_marshal_VOID__VOID,
                                  G_TYPE_NONE,
                                  0);
}

/**
 * gb_project_init:
 * @project: (in): A #GbProject.
 *
 * Initializes the newly created #GbProject instance.
 */
static void
gb_project_init (GbProject *project)
{
   GbProjectPrivate *priv;

   ENTRY;

   project->priv = priv =
      G_TYPE_INSTANCE_GET_PRIVATE(project,
                                  GB_TYPE_PROJECT,
                                  GbProjectPrivate);

   priv->targets =
      g_object_new(GB_TYPE_PROJECT_GROUP,
                   "parent", project,
                   "item-type", GB_TYPE_PROJECT_ITEM,
                   NULL);

   priv->files =
      g_object_new(GB_TYPE_PROJECT_GROUP,
                   "parent", project,
                   "item-type", GB_TYPE_PROJECT_ITEM,
                   NULL);

   EXIT;
}

GQuark
gb_project_error_quark (void)
{
   return g_quark_from_static_string("gb-project-error-quark");
}

/* ide-cmake-builder.c
 *
 * Copyright (C) 2016 Philipp Andronov <filipp.andronov@gmail.com>
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
#include <ide.h>

#include "ide-cmake-builder.h"
#include "ide-cmake-build-task.h"

struct _IdeCmakeBuilder
{
  IdeBuilder parent_instance;
};

G_DEFINE_TYPE (IdeCmakeBuilder, ide_cmake_builder, IDE_TYPE_BUILDER)

/**
 * ide_cmake_builder_get_build_directory:
 *
 * Gets the directory that will contain the generated makefiles and build root.
 *
 * Returns: (transfer full): A #GFile containing the build directory.
 */
GFile *
ide_cmake_builder_get_build_directory (IdeCmakeBuilder *self)
{
  g_autofree gchar *path = NULL;
  IdeConfiguration *configuration;
  IdeContext *context;
  IdeProject *project;
  IdeDevice *device;
  const gchar *root_build_dir;
  const gchar *project_id;
  const gchar *device_id;
  const gchar *system_type;

  g_return_val_if_fail (IDE_IS_CMAKE_BUILDER (self), NULL);

  context = ide_object_get_context (IDE_OBJECT (self));

  configuration = ide_builder_get_configuration (IDE_BUILDER (self));

  device = ide_configuration_get_device (configuration);
  device_id = ide_device_get_id (device);
  system_type = ide_device_get_system_type (device);

  project = ide_context_get_project (context);
  project_id = ide_project_get_id (project);

  root_build_dir = ide_context_get_root_build_dir (context);

  path = g_build_filename (root_build_dir, project_id, device_id, system_type, NULL);
  return g_file_new_for_path (path);
}

/**
 * ide_cmake_builder_get_needs_bootstrap:
 *
 * Gets flag showing that project build has been changed, so cached build information, such
 * as compile flags, number of files and makefiles should be updated
 *
 * Returns: (transfer full): A #gboolean shpwing if project makefiles needs to be
 * regenerated
 */
gboolean
ide_cmake_builder_get_needs_bootstrap (IdeCmakeBuilder *self)
{
  return FALSE;
}

//
// *** IdeBuilder overrides
//
static void
ide_cmake_builder_after_build(GObject      *object,
                              GAsyncResult *result,
                              gpointer      user_data)
{
  GError *error = NULL;
  g_autoptr(GTask) task = user_data;
  IdeCmakeBuildTask *build_result = (IdeCmakeBuildTask *)object;

  g_return_if_fail (IDE_IS_CMAKE_BUILD_TASK (build_result));
  g_return_if_fail (G_IS_TASK (task));

  if (!ide_cmake_build_task_execute_finish(build_result, result, &error)) {
    if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
      ide_build_result_set_mode (IDE_BUILD_RESULT (build_result), _("Build cancelled"));
    } else {
      ide_build_result_set_mode (IDE_BUILD_RESULT (build_result), _("Build failed"));
    }
    g_task_return_error (task, error);
  } else {
    ide_build_result_set_mode (IDE_BUILD_RESULT (build_result), _("Build successful"));
    g_task_return_pointer (task, g_object_ref (build_result), g_object_unref);
  }
}

static void
ide_cmake_builder_build_async (IdeBuilder           *builder,
                               IdeBuilderBuildFlags  flags,
                               IdeBuildResult       **result,
                               GCancellable         *cancellable,
                               GAsyncReadyCallback   callback,
                               gpointer              user_data)
{
  IdeCmakeBuilder *self = (IdeCmakeBuilder *)builder;
  g_autoptr(IdeCmakeBuildTask) build_result = NULL;
  g_autoptr(GTask) task = NULL;
  g_autoptr(GFile) directory = NULL;
  IdeConfiguration *configuration;
  IdeContext *context;
  GPtrArray *build_steps;

  g_return_if_fail (IDE_IS_CMAKE_BUILDER (builder));
  g_return_if_fail (IDE_IS_CMAKE_BUILDER (self));

  build_steps = g_ptr_array_new ();
  g_ptr_array_add(build_steps, ide_cmake_build_task_setenv);
  g_ptr_array_add(build_steps, ide_cmake_build_task_mkdirs);
  g_ptr_array_add(build_steps, ide_cmake_build_task_cmake);
  g_ptr_array_add(build_steps, ide_cmake_build_task_make);

  task = g_task_new (self, cancellable, callback, user_data);
  context = ide_object_get_context (IDE_OBJECT (builder));
  configuration = ide_builder_get_configuration (IDE_BUILDER (self));
  directory = ide_cmake_builder_get_build_directory (self);
  build_result = g_object_new (IDE_TYPE_CMAKE_BUILD_TASK,
                               "context", context,
                               "configuration", configuration,
                               "directory", directory,
                               "steps", build_steps,
                               NULL);

  if (result != NULL) {
    *result = g_object_ref (build_result);
  }

  // Launch build
  ide_cmake_build_task_execute_async(
    build_result,
    cancellable,
    ide_cmake_builder_after_build,
    g_object_ref (task)
  );
}

static IdeBuildResult *
ide_cmake_builder_build_finish (IdeBuilder    *builder,
                                GAsyncResult  *result,
                                GError        **error)
{
  GTask *task = (GTask *)result;

  g_return_val_if_fail (IDE_IS_CMAKE_BUILDER (builder), NULL);
  g_return_val_if_fail (G_IS_TASK (task), NULL);

  return g_task_propagate_pointer (task, error);
}

static void
ide_cmake_builder_install_async (IdeBuilder           *builder,
                                 IdeBuildResult       **result,
                                 GCancellable         *cancellable,
                                 GAsyncReadyCallback   callback,
                                 gpointer              user_data)
{
}

static IdeBuildResult *
ide_cmake_builder_install_finish (IdeBuilder    *builder,
                                  GAsyncResult  *result,
                                  GError        **error)
{
  return 0;
}

//
// IdeCmakeBuilder type definitions
//

static void
ide_cmake_builder_class_init (IdeCmakeBuilderClass *klass)
{
  IdeBuilderClass *builder_class = IDE_BUILDER_CLASS (klass);

  builder_class->build_async = ide_cmake_builder_build_async;
  builder_class->build_finish = ide_cmake_builder_build_finish;
  builder_class->install_async = ide_cmake_builder_install_async;
  builder_class->install_finish = ide_cmake_builder_install_finish;
}

static void
ide_cmake_builder_init (IdeCmakeBuilder *self)
{
}

#ifndef IDE_CMAKE_BUILDER_STEP_H
#define IDE_CMAKE_BUILDER_STEP_H

#include <gio/gio.h>
#include <ide.h>


#define IDE_TYPE_CMAKE_BUILD_TASK (ide_cmake_build_task_get_type())
G_DECLARE_FINAL_TYPE (IdeCmakeBuildTask, ide_cmake_build_task, IDE, CMAKE_BUILD_TASK, IdeBuildResult)


typedef gboolean (*BuildStep) (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error);


void ide_cmake_build_task_execute_async(IdeCmakeBuildTask   *self,
                                        GCancellable        *cancellable,
                                        GAsyncReadyCallback  callback,
                                        gpointer             user_data);

gboolean ide_cmake_build_task_execute_finish(IdeCmakeBuildTask  *self,
                                             GAsyncResult       *result,
                                             GError             **error);

/**
 * setenv:
 * Setup environment variables in the build subprocess
 *
 */
gboolean ide_cmake_build_task_setenv (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error);

/**
 * mkdirs:
 * Creates necessary directory structure
 */
gboolean ide_cmake_build_task_mkdirs (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error);

/**
 * cmake:
 * Calls cmake to recreate Makeflies for the project
 */
gboolean ide_cmake_build_task_cmake  (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error);

/**
 * compile_db:
 * Parse JSON compilation database generated by cmake
 */
gboolean ide_cmake_build_task_compile_db  (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error);

/**
 * make:
 * Calls GNU make to perform actual build
 */
gboolean ide_cmake_build_task_make   (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error);

#endif /* IDE_CMAKE_BUILDER_STEP_H */
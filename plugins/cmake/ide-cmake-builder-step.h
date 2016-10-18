#ifndef IDE_CMAKE_BUILD_TASK_H
#define IDE_CMAKE_BUILD_TASK_H

#include <gio/gio.h>
#include <ide.h>


#define IDE_TYPE_AUTOTOOLS_BUILD_TASK (ide_cmake_build_task_get_type())
G_DECLARE_FINAL_TYPE (IdeCmakeBuildTask, ide_cmake_build_task, IDE, CMAKE_BUILD_TASK, IdeBuildResult)


typedef gboolean (*BuildStep) (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable);

/**
 * setenv:
 * Setup environment variables in the build subprocess
 *
 */
gboolean setenv (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable);

/**
 * mkdirs:
 * Creates necessary directory structure
 */
gboolean mkdirs (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable);

/**
 * cmake:
 * Calls cmake to recreate Makeflies for the project
 */
gboolean cmake  (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable);

/**
 * make:
 * Calls GNU make to perform actual build
 */
gboolean make   (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable);

#endif /* IDE_CMAKE_BUILD_TASK_H */

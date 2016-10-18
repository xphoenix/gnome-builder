#include <glib/gi18n.h>
#include "ide-cmake-build-task.h"

struct _IdeCmakeBuildTask
{
  IdeBuildResult    parent;
  IdeConfiguration *configuration;

  GFile            *directory;
  GPtrArray        *extra_targets;
  GPtrArray        *steps;

  guint             executed : 1;
};

G_DEFINE_TYPE (IdeCmakeBuildTask, ide_cmake_build_task, IDE_TYPE_BUILD_RESULT)

enum {
  PROP_0,
  PROP_CONFIGURATION,
  PROP_DIRECTORY,
  PROP_STEPS,
  LAST_PROP
};
static GParamSpec *properties [LAST_PROP];

//
// *** IdeCmakeBuildTask type implementation
//
static void
ide_cmake_build_task_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  IdeCmakeBuildTask *self = IDE_CMAKE_BUILD_TASK (object);
  switch (prop_id) {
    case PROP_CONFIGURATION:
      g_value_set_object(value, self->configuration);
      break;

    case PROP_DIRECTORY:
      g_value_set_object(value, self->directory);
      break;

    case PROP_STEPS:
      g_value_set_object(value, self->steps);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
ide_cmake_build_task_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  IdeCmakeBuildTask *self = IDE_CMAKE_BUILD_TASK (object);
  IdeConfiguration *conf;
  GFile *dir;
  GPtrArray *steps;
  switch (prop_id) {
    case PROP_CONFIGURATION:
      conf = g_value_get_object(value);
      g_assert (IDE_IS_CONFIGURATION (conf));

      if (g_set_object (&self->configuration, conf)) {
        g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CONFIGURATION]);
      }
      break;

    case PROP_DIRECTORY:
      dir = g_value_get_object(value);
      g_assert (dir && G_IS_FILE(dir));

      if (g_set_object (&self->directory, dir)) {
        g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_DIRECTORY]);
      }
      break;

    case PROP_STEPS:
      steps = g_value_get_object(value);
      g_assert(steps != 0);

      g_ptr_array_free(self->steps, FALSE);
      if (g_set_object (&self->steps, steps)) {
        g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_STEPS]);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
ide_cmake_build_task_class_init (IdeCmakeBuildTaskClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = 0;//ide_autotools_build_task_finalize;
  object_class->get_property = ide_cmake_build_task_get_property;
  object_class->set_property = ide_cmake_build_task_set_property;

  properties [PROP_CONFIGURATION] =
    g_param_spec_object ("configuration",
                        "Configuration",
                        "The configuration for this build.",
                        IDE_TYPE_CONFIGURATION,
                        (G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS));

  properties [PROP_DIRECTORY] =
    g_param_spec_object ("directory",
                         "Directory",
                         "The directory to perform the build within.",
                         G_TYPE_FILE,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));

  properties [PROP_STEPS] =
    g_param_spec_boolean ("steps",
                          "Build steps",
                          "Steps needs to be performed during the build",
                          FALSE,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
ide_cmake_build_task_init (IdeCmakeBuildTask *self)
{
}

//
// *** Async execution: call all prebuild hooks, execute build steps and call all post
// build hooks
//

/**
 * ide_cmake_build_task_steps_worker:
 * Executes given set of build steps asynchoniously.
 *
 */
static void
ide_cmake_build_task_steps_worker(GTask        *task,
                                  gpointer      source_object,
                                  gpointer      task_data,
                                  GCancellable *cancellable)
{

}

/**
 * ide_cmake_build_task_after_configuration_prebuild:
 * Called after configuration prebuild hooks are complete and starts build steps execution
 *
 *
 */
void
ide_cmake_build_task_after_configuration_prebuild (GObject      *object,
                                                   GAsyncResult *result,
                                                   gpointer      user_data)
{
  IdeBuildCommandQueue *cmdq = (IdeBuildCommandQueue *)object;
  g_autoptr(GTask) task = user_data;
  IdeCmakeBuildTask *self;
  GError *error = NULL;

  IDE_ENTRY;

  g_assert (IDE_IS_BUILD_COMMAND_QUEUE (cmdq));
  g_assert (G_IS_ASYNC_RESULT (result));

  self = g_task_get_source_object (task);

  if (!ide_build_command_queue_execute_finish (cmdq, result, &error)) {
    ide_build_result_log_stderr (IDE_BUILD_RESULT (self), "%s %s", _("Build Failed: "), error->message);
    g_task_return_error (task, error);
    IDE_EXIT;
  }

  g_task_run_in_thread (task, ide_cmake_build_task_steps_worker);
  IDE_EXIT;
}

/**
 * ide_cmake_build_task_after_runtime_prebuild:
 * Called when runtime prebuild hooks are complete and starts configuration prebuild hooks
 */
void
ide_cmake_build_task_after_runtime_prebuild (GObject      *object,
                                             GAsyncResult *result,
                                             gpointer      user_data)
{
  IdeRuntime *runtime = (IdeRuntime *)object;
  IdeCmakeBuildTask *self;
  g_autoptr(GTask) task = user_data;
  g_autoptr(IdeBuildCommandQueue) prebuild = NULL;
  GCancellable *cancellable;
  GError *error = NULL;

  IDE_ENTRY;

  g_assert (IDE_IS_RUNTIME (runtime));
  g_assert (G_IS_TASK (task));
  g_assert (G_IS_ASYNC_RESULT (result));

  if (!ide_runtime_prebuild_finish (runtime, result, &error)) {
    g_task_return_error (task, error);
    IDE_EXIT;
  }

  /*
   * Now that the runtime has prepared itself, we need to allow the
   * configuration's prebuild commands to be executed.
   */
  self = g_task_get_source_object (task);
  g_assert (IDE_IS_CMAKE_BUILD_TASK (self));

  prebuild = ide_configuration_get_prebuild (self->configuration);
  g_assert (IDE_IS_BUILD_COMMAND_QUEUE (prebuild));

  cancellable = g_task_get_cancellable (task);
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  ide_build_command_queue_execute_async (prebuild,
                                         runtime,
                                         ide_configuration_get_environment (self->configuration),
                                         IDE_BUILD_RESULT (self),
                                         cancellable,
                                         ide_cmake_build_task_after_configuration_prebuild,
                                         g_steal_pointer (&task));

  IDE_EXIT;
}

/**
 * ide_cmake_build_task_execute_async:
 * Executes given IdeCmakeBuildTask asynchroniously, step one - call runtime prebuild
 */
void
ide_cmake_build_task_execute_async (IdeCmakeBuildTask     *self,
                                    GCancellable          *cancellable,
                                    GAsyncReadyCallback    callback,
                                    gpointer               user_data)
{
  IdeRuntime *runtime;
  g_autoptr(GTask) task = NULL;
  IDE_ENTRY;

  g_return_if_fail (IDE_IS_CMAKE_BUILD_TASK (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (callback != NULL);

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, ide_cmake_build_task_execute_async);
  g_task_set_task_data (task, self, NULL);

  if (self->executed) {
    g_task_return_new_error (
      task,
      G_IO_ERROR,
      G_IO_ERROR_FAILED,
      "%s",
      _("Cannot execute build task more than once")
    );
  } else {
    self->executed = TRUE;

    runtime = ide_configuration_get_runtime (self->configuration);
    if (runtime == NULL) {
      g_task_return_new_error (
        task,
        IDE_RUNTIME_ERROR,
        IDE_RUNTIME_ERROR_NO_SUCH_RUNTIME,
        "%s “%s”",
        _("Failed to locate runtime"),
        ide_configuration_get_runtime_id (self->configuration)
      );
    } else {
      // Execute the pre-hook for the runtime before we start building
      ide_runtime_prebuild_async (
        runtime,
        cancellable,
        ide_cmake_build_task_after_runtime_prebuild,
        g_steal_pointer (&task)
      );
    }
  }
  IDE_EXIT;
}

//
// *** Build steps implementation
//

gboolean ide_cmake_build_task_setenv (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

gboolean ide_cmake_build_task_mkdirs (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

gboolean ide_cmake_build_task_cmake (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

gboolean ide_cmake_build_task_make (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

#include <glib/gi18n.h>
#include <glib-object.h>
#include "ide-cmake-build-task.h"

struct _IdeCmakeBuildTask
{
  IdeBuildResult    parent;
  IdeConfiguration *configuration;

  GFile            *directory;
  //GPtrArray        *extra_targets;
  GPtrArray        *steps;

  guint             executed : 1;

  // private fields
  IdeRuntime *runtime;
  GFile *projectDir;
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
      g_value_set_boxed(value, self->steps);
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
      steps = g_value_get_boxed(value);
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
ide_cmake_build_task_finalize (GObject *object)
{
  IdeCmakeBuildTask *self = (IdeCmakeBuildTask *)object;

  g_clear_object (&self->directory);
  g_clear_object (&self->configuration);
  //g_clear_pointer (&self->extra_targets, g_ptr_array_unref);
  g_clear_pointer (&self->steps, g_ptr_array_unref);

  G_OBJECT_CLASS (ide_cmake_build_task_parent_class)->finalize (object);
}


static void
ide_cmake_build_task_class_init (IdeCmakeBuildTaskClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_cmake_build_task_finalize;
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
    g_param_spec_boxed ("steps",
                          "Build steps",
                          "Steps needs to be performed during the build",
                          G_TYPE_PTR_ARRAY,
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
gboolean
ide_cmake_build_task_execute_finish (IdeCmakeBuildTask  *self,
                                     GAsyncResult       *result,
                                     GError             **error)
{
  GTask *task = (GTask *)result;
  guint sequence;
  gboolean ret;

  IDE_ENTRY;

  g_return_val_if_fail (IDE_IS_CMAKE_BUILD_TASK (self), FALSE);
  g_return_val_if_fail (G_IS_TASK (task), FALSE);

// TODO: Check that build runs necessary configuration
//
//  sequence = ide_configuration_get_sequence (self->configuration);
//  if (self->sequence == sequence) {
//    ide_configuration_set_dirty (self->configuration, FALSE);
//  }

  /* Mark the task as failed */
  ret = g_task_propagate_boolean (task, error);
  if (ret == FALSE) {
    ide_build_result_set_failed (IDE_BUILD_RESULT (self), TRUE);
  }

  ide_build_result_set_running (IDE_BUILD_RESULT (self), FALSE);
  IDE_RETURN (ret);
}

/**
 * ide_cmake_build_task_steps_worker:
 * Executes given set of build steps asynchoniously.
 */
static void
ide_cmake_build_task_steps_worker(GTask        *task,
                                  gpointer      source_object,
                                  gpointer      task_data,
                                  GCancellable *cancellable)
{
  GError *error = NULL;
  gboolean result;
  IdeCmakeBuildTask *self;

  self = g_task_get_source_object (task);
  g_assert(IDE_IS_CMAKE_BUILD_TASK(self));

  // Run all tasks
  result = TRUE;
  for (guint i = 0; i < self->steps->len && result && !g_cancellable_is_cancelled (cancellable) ; i++) {
    BuildStep f = (BuildStep)g_ptr_array_index(self->steps, i);
    result = f(self, cancellable, &error);
    result &= (error == NULL);
  }

  // Running post build steps
  if (result) {
    IdeEnvironment *environment;
    IdeBuildCommandQueue *postbuild;

    postbuild = ide_configuration_get_postbuild (self->configuration);
    environment = ide_environment_copy (ide_configuration_get_environment (self->configuration));

    result &= ide_build_command_queue_execute (
        postbuild,
        self->runtime,
        environment,
        IDE_BUILD_RESULT (self),
        cancellable,
        &error
    );
  }

  // Done!
  if (!result) {
    ide_build_result_log_stderr (IDE_BUILD_RESULT (self), "%s %s", _("Build Failed: "), error->message);
    g_task_return_error (task, error);
  } else {
    g_task_return_boolean (task, TRUE);
  }
}

/**
 * ide_cmake_build_task_after_configuration_prebuild:
 * Called after configuration prebuild hooks are complete and starts build steps execution
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
  g_assert(IDE_IS_CMAKE_BUILD_TASK(self));

  if (!ide_build_command_queue_execute_finish (cmdq, result, &error)) {
    ide_build_result_log_stderr (IDE_BUILD_RESULT (self), "%s %s", _("Build Failed: "), error->message);
    g_task_return_error (task, error);
  } else {
    g_task_run_in_thread (task, ide_cmake_build_task_steps_worker);
  }
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
  } else {
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
  }
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
  g_autoptr(GTask) task = NULL;
  IdeContext *context;

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_CMAKE_BUILD_TASK (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_return_if_fail (callback != NULL);

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, ide_cmake_build_task_execute_async);

  if (self->executed) {
    g_task_return_new_error (
      task,
      G_IO_ERROR,
      G_IO_ERROR_FAILED,
      "%s",
      _("Cannot execute build task more than once")
    );
  } else {
    context = ide_object_get_context (IDE_OBJECT (self));

    self->executed = TRUE;
    self->runtime = ide_configuration_get_runtime (self->configuration);
    self->projectDir = ide_context_get_project_file (context);

    // TODO: better handling here
    if (g_file_query_file_type(self->projectDir, G_FILE_QUERY_INFO_NONE, cancellable) != G_FILE_TYPE_DIRECTORY) {
      self->projectDir = g_file_get_parent(self->projectDir);
    }

    if (self->runtime == NULL) {
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
        self->runtime,
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

static gboolean
log_in_main (gpointer data)
{
  struct {
    IdeBuildResult *result;
    gchar *message;
  } *pair = data;

  ide_build_result_log_stdout (pair->result, "%s", pair->message);

  g_free (pair->message);
  g_object_unref (pair->result);
  g_slice_free1 (sizeof *pair, pair);

  return G_SOURCE_REMOVE;
}

static IdeSubprocess *
log_and_spawn (IdeCmakeBuildTask  *self,
               IdeSubprocessLauncher  *launcher,
               GCancellable           *cancellable,
               GError                **error,
               const gchar           *argv0,
               ...)
{
  g_autoptr(GError) local_error = NULL;
  IdeSubprocess *ret;
  struct {
    IdeBuildResult *result;
    gchar *message;
  } *pair;
  GString *log;
  gchar *item;
  va_list args;
  gint popcnt = 0;

  g_assert (IDE_IS_CMAKE_BUILD_TASK (self));
  g_assert (IDE_IS_SUBPROCESS_LAUNCHER (launcher));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  log = g_string_new (argv0);
  ide_subprocess_launcher_push_argv (launcher, argv0);

  va_start (args, argv0);
  while (NULL != (item = va_arg (args, gchar *)))
    {
      ide_subprocess_launcher_push_argv (launcher, item);
      g_string_append_printf (log, " '%s'", item);
      popcnt++;
    }
  va_end (args);

  pair = g_slice_alloc (sizeof *pair);
  pair->result = g_object_ref (self);
  pair->message = g_string_free (log, FALSE);
  g_timeout_add (0, log_in_main, pair);

  ret = ide_subprocess_launcher_spawn_sync (launcher, cancellable, &local_error);

  if (ret == NULL)
    {
      ide_build_result_log_stderr (IDE_BUILD_RESULT (self), "%s %s",
                                   _("Build Failed: "),
                                   local_error->message);
      g_propagate_error (error, g_steal_pointer (&local_error));
    }

  /* pop make args */
  for (; popcnt; popcnt--)
    g_free (ide_subprocess_launcher_pop_argv (launcher));

  /* pop "make" */
  g_free (ide_subprocess_launcher_pop_argv (launcher));

  return ret;
}

gboolean ide_cmake_build_task_setenv (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error) {
  return TRUE;
}

gboolean ide_cmake_build_task_mkdirs (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error) {
  g_autofree char *path;

  g_assert (IDE_IS_CMAKE_BUILD_TASK (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  path = g_file_get_path(self->directory);
  if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
    if (g_mkdir_with_parents (path, 0750) != 0) {
      *error = g_error_new (
        G_IO_ERROR,
        G_IO_ERROR_FAILED,
        _("Failed to create build directory.")
      );
      return FALSE;
    }
  } else if (!g_file_test (path, G_FILE_TEST_IS_DIR)){
    *error = g_error_new (
      G_IO_ERROR,
      G_IO_ERROR_NOT_DIRECTORY,
      _("'%s' is not a directory."),
      path
    );
    return FALSE;
  }

  return TRUE;
}

gboolean ide_cmake_build_task_cmake (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error) {
  g_autofree char *build_path = NULL;
  g_autofree char *project_path = NULL;
  g_autoptr(IdeSubprocess) process = NULL;
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;

  g_assert (IDE_IS_CMAKE_BUILD_TASK (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  ide_build_result_set_mode (IDE_BUILD_RESULT (self), _("Running cmake…"));
  if (NULL == (launcher = ide_runtime_create_launcher (self->runtime, error))) {
    return FALSE;
  }

  // Configure runtime
  build_path = g_file_get_path(self->directory);
  project_path = g_file_get_path(self->projectDir);
  ide_subprocess_launcher_set_cwd (launcher, build_path);
  ide_subprocess_launcher_setenv (launcher, "LANG", "C", TRUE);

  // check runtime
  if (!ide_runtime_contains_program_in_path (self->runtime, "cmake", cancellable)) {
    *error = g_error_new (
      G_IO_ERROR,
      G_IO_ERROR_NOT_FOUND,
      "Failed to locate make."
    );
    return FALSE;
  }

  // Launch cmake
  process = log_and_spawn(self, launcher, cancellable, error, "cmake", "-DCMAKE_EXPORT_COMPILE_COMMANDS=On", project_path, NULL);
  if (!process) {
    return FALSE;
  }

  ide_build_result_log_subprocess (IDE_BUILD_RESULT (self), process);
  if (!ide_subprocess_wait_check (process, cancellable, error)) {
    return FALSE;
  }

  return TRUE;
}

gboolean ide_cmake_build_task_make (IdeCmakeBuildTask *self, GCancellable *cancellable, GError **error) {
  char *make_bin = NULL;
  g_autofree char *build_path = NULL;
  g_autofree char *project_path = NULL;
  g_autoptr(IdeSubprocess) process = NULL;
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;

  g_assert (IDE_IS_CMAKE_BUILD_TASK (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  ide_build_result_set_mode (IDE_BUILD_RESULT (self), _("Running make…"));
  if (NULL == (launcher = ide_runtime_create_launcher (self->runtime, error))) {
    return FALSE;
  }

  // Configure runtime
  build_path = g_file_get_path(self->directory);
  project_path = g_file_get_path(self->projectDir);
  ide_subprocess_launcher_set_flags (launcher, (G_SUBPROCESS_FLAGS_STDERR_PIPE | G_SUBPROCESS_FLAGS_STDOUT_PIPE));
  ide_subprocess_launcher_set_cwd (launcher, build_path);
  ide_subprocess_launcher_setenv (launcher, "LANG", "C", TRUE);

  // check runtime
  if (ide_runtime_contains_program_in_path (self->runtime, "gmake", cancellable)) {
    make_bin = "gmake";
  } else if (ide_runtime_contains_program_in_path (self->runtime, "make", cancellable)) {
    make_bin = "make";
  } else {
    *error = g_error_new (
      G_IO_ERROR,
      G_IO_ERROR_NOT_FOUND,
      "Failed to locate make."
    );
    return FALSE;
  }

  // Launch cmake
  process = log_and_spawn(self, launcher, cancellable, error, make_bin, NULL);
  if (!process) {
    return FALSE;
  }

  ide_build_result_log_subprocess (IDE_BUILD_RESULT (self), process);
  if (!ide_subprocess_wait_check (process, cancellable, error)) {
    return FALSE;
  }

  return TRUE;
}

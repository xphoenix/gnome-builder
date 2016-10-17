#define G_LOG_DOMAIN "ide-cmake-build-system"

#include "config.h"

#include <egg-counter.h>
#include <egg-task-cache.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtksourceview/gtksource.h>
#include <ide.h>
#include <ide-internal.h>

#include "ide-cmake-build-system.h"

struct _IdeCmakeBuildSystem
{
  IdeObject     parent_instance;

  GFile        *project_file;
  EggTaskCache *task_cache;
  gchar        *tarball_name;
};

static void async_initable_iface_init (GAsyncInitableIface *iface);
static void build_system_iface_init (IdeBuildSystemInterface *iface);

G_DEFINE_TYPE_WITH_CODE (IdeCmakeBuildSystem,
                         ide_cmake_build_system,
                         IDE_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE, async_initable_iface_init)
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_BUILD_SYSTEM, build_system_iface_init))

enum {
  PROP_0,
  PROP_PROJECT_FILE,
  LAST_PROP
};
static GParamSpec *properties [LAST_PROP];

//
// IdeCmakeBuildSystem type definitions
//

static void
ide_cmake_build_system_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  IdeCmakeBuildSystem *self = IDE_CMAKE_BUILD_SYSTEM (object);
  switch(prop_id) {
    case PROP_PROJECT_FILE:
        g_value_set_object (value, self->project_file);
        break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
ide_cmake_build_system_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  IdeCmakeBuildSystem *self = IDE_CMAKE_BUILD_SYSTEM (object);
  switch (prop_id) {
    case PROP_PROJECT_FILE:
      g_clear_object (&self->project_file);
      self->project_file = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
ide_cmake_build_system_class_init(IdeCmakeBuildSystemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = ide_cmake_build_system_get_property;
  object_class->set_property = ide_cmake_build_system_set_property;

  properties [PROP_PROJECT_FILE] =
    g_param_spec_object ("project-file",
                         "Project File",
                         "The path of the project file.",
                         G_TYPE_FILE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
ide_cmake_build_system_init(IdeCmakeBuildSystem *self)
{

}

//
// ** Various helper functions
//

/*
 * Verifies that given file is a cmake build script
 */
static gboolean
is_cmake_build_script (GFile *file)
{
  gchar *name;
  gboolean ret;

  g_assert (G_IS_FILE (file));

  name = g_file_get_basename (file);
  ret = (0 == g_strcmp0 (name, "CMakeLists.txt"));
  g_free (name);

  return ret;
}

//
// *** GAsyncInit interface implementation
//

/*
 * Actual worker that implements CMakeLists.txt file search. Recursively go up from the
 * given project file until CMakeLists.txt file found
 */
static void
ide_cmake_build_system_discover_file_worker (GTask        *task,
                                             gpointer      source_object,
                                             gpointer      task_data,
                                             GCancellable *cancellable)
{
  GFile *file = task_data;
  GFile *parent;

  g_assert (G_IS_TASK (task));
  g_assert (G_IS_FILE (file));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  // If project file is a bulid script - nothing to do
  if (is_cmake_build_script (file) && g_file_query_exists (file, cancellable))
  {
    g_task_return_pointer (task, g_object_ref (file), g_object_unref);
    return;
  }

  // Recursively go up by the file tree
  parent = g_object_ref (file);
  while (parent != NULL)
  {
    GFile *tmp;
    GFile *child;

    child = g_file_get_child (parent, "CMakeLists.txt");
    if (g_file_query_exists (child, cancellable))
    {
      g_task_return_pointer (task, g_object_ref (child), g_object_unref);
      g_clear_object (&child);
      g_clear_object (&parent);
      return;
    }

    tmp = parent;
    parent = g_file_get_parent (parent);

    g_clear_object (&child);
    g_clear_object (&tmp);
  }

  g_clear_object (&parent);
  g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, _("Failed to locate CMakeLists.txt"));
}

/*
 * Initialize build system for the current project. In short it tries to find CMakeLists.txt file
 * to setup cmake build process
 */
static void
ide_cmake_build_system_init_async (GAsyncInitable      *initable,
                                       gint                 io_priority,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             user_data)
{
  IdeCmakeBuildSystem *system = (IdeCmakeBuildSystem *)initable;
  g_autoptr(GTask) task;
  IdeContext *context;
  GFile *project_file;

  g_return_if_fail (IDE_IS_CMAKE_BUILD_SYSTEM (system));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));


  context = ide_object_get_context (IDE_OBJECT (system));
  project_file = ide_context_get_project_file (context);

  task = g_task_new (system, cancellable, callback, user_data);
  g_task_set_task_data (task, g_object_ref (project_file), g_object_unref);
  g_task_run_in_thread (task, ide_cmake_build_system_discover_file_worker);
}


/*
 * Process async file search operation results and finish build system initialization. Returns true in case
 * if there is a CMAkeLists.txt file in the project
 */
static gboolean
ide_cmake_build_system_init_finish (GAsyncInitable  *initable,
                                    GAsyncResult    *result,
                                    GError         **error)
{
  GTask *task = (GTask *)result;

  g_return_val_if_fail (IDE_IS_CMAKE_BUILD_SYSTEM (initable), FALSE);
  g_return_val_if_fail (G_IS_TASK (task), FALSE);

  return g_task_propagate_boolean (task, error);
}

static void
async_initable_iface_init (GAsyncInitableIface *iface)
{
  iface->init_async = ide_cmake_build_system_init_async;
  iface->init_finish = ide_cmake_build_system_init_finish;
}
//
// *** IdeBuildSystem interface implementation
//

// ide_cmake_build_system_get_priority implements IdeBuildSystem interface
static gint
ide_cmake_build_system_get_priority (IdeBuildSystem *system)
{
  return -100;
};

// ide_cmake_build_system_get_builder implements IdeBuildSystem interface
static IdeBuilder*
ide_cmake_build_system_get_builder (IdeBuildSystem *system, IdeConfiguration *configuration, GError **error)
{

  return 0;
};

// ide_cmake_build_system_get_build_flags_async implements IdeBuildSystem interface
static void
ide_cmake_build_system_get_build_flags_async (IdeBuildSystem *self,
                                                IdeFile              *file,
                                                GCancellable         *cancellable,
                                                GAsyncReadyCallback   callback,
                                                gpointer              user_data)
{

};


// ide_cmake_build_system_get_build_flags_finish implements IdeBuildSystem interface                                                }
static gchar **
ide_cmake_build_system_get_build_flags_finish (IdeBuildSystem       *self,
                                                GAsyncResult         *result,
                                                GError              **error)
{
  return 0;
};

// ide_cmake_build_system_get_build_targets_async implements IdeBuildSystem interface
static void
ide_cmake_build_system_get_build_targets_async (IdeBuildSystem       *self,
                                                GCancellable         *cancellable,
                                                GAsyncReadyCallback   callback,
                                                gpointer              user_data)
{

};

// ide_cmake_build_system_get_build_targets_finish implements IdeBuildSystem interface
static GPtrArray*
ide_cmake_build_system_get_build_targets_finish (IdeBuildSystem       *self,
                                                GAsyncResult         *result,
                                                GError              **error)
{
  return 0;
};

/*
 * Setup IdeBuildSystem interface
 */
static void
build_system_iface_init (IdeBuildSystemInterface *iface)
{
  iface->get_priority = ide_cmake_build_system_get_priority;
  iface->get_builder = ide_cmake_build_system_get_builder;
  iface->get_build_flags_async = ide_cmake_build_system_get_build_flags_async;
  iface->get_build_flags_finish = ide_cmake_build_system_get_build_flags_finish;
  iface->get_build_targets_async = ide_cmake_build_system_get_build_targets_async;
  iface->get_build_targets_finish = ide_cmake_build_system_get_build_targets_finish;
}

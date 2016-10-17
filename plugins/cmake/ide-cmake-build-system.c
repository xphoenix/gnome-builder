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

//
// IdeCmakeBuildSystem type definitions
//

static void
ide_cmake_build_system_class_init(IdeCmakeBuildSystemClass *klass) {

}

static void
ide_cmake_build_system_init(IdeCmakeBuildSystem *self) {

}

//
// *** GAsyncInit interface implementation
//

static void
ide_cmake_build_system_init_async (GAsyncInitable      *initable,
                                       gint                 io_priority,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             user_data)
{
  IdeCmakeBuildSystem *system = (IdeCmakeBuildSystem *)initable;
  g_autoptr(GTask) task = NULL;
  IdeContext *context;
  GFile *project_file;

  g_return_if_fail (IDE_IS_CMAKE_BUILD_SYSTEM (system));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (initable, cancellable, callback, user_data);
  context = ide_object_get_context (IDE_OBJECT (system));
  project_file = ide_context_get_project_file (context);

  /*
  ide_autotools_build_system_discover_file_async (system,
                                                  project_file,
                                                  cancellable,
                                                  discover_file_cb,
                                                  g_object_ref (task));
   */
}

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
static void build_system_iface_init (IdeBuildSystemInterface *iface)
{
  iface->get_priority = ide_cmake_build_system_get_priority;
  iface->get_builder = ide_cmake_build_system_get_builder;
  iface->get_build_flags_async = ide_cmake_build_system_get_build_flags_async;
  iface->get_build_flags_finish = ide_cmake_build_system_get_build_flags_finish;
  iface->get_build_targets_async = ide_cmake_build_system_get_build_targets_async;
  iface->get_build_targets_finish = ide_cmake_build_system_get_build_targets_finish;
}


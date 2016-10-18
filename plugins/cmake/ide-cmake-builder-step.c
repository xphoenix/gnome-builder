#include "ide-cmake-builder-step.h"

struct _IdeCmakeBuildTask
{
  IdeBuildResult    parent;
  IdeConfiguration *configuration;

  GFile            *directory;
  GPtrArray        *extra_targets;
  GPtrArray        *steps;
};

G_DEFINE_TYPE (IdeCmakeBuildTask, ide_cmake_build_task, IDE_TYPE_BUILD_RESULT)

enum {
  PROP_0,
  PROP_CONFIGURATION,
  PROP_DIRECTORY,
  PROP_STEPS
  LAST_PROP
};

//
// *** IdeCmakeBuildTask type implementation
//
static void
ide_cmake_build_task_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{

}

static void
ide_cmake_build_task_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{

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

  properties [PROP_REQUIRE_AUTOGEN] =
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
// *** Build steps implementation
//

gboolean setenv (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

gboolean mkdirs (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

gboolean cmake (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

gboolean make (GTask *task, IdeCmakeBuildTask *self, GCancellable *cancellable) {
  return FALSE;
}

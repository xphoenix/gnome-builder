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
  return 0;
}

gboolean
ide_cmake_builder_get_needs_bootstrap (IdeCmakeBuilder *self)
{
  return FALSE;
}

//
// *** IdeBuilder overrides
//

static void
ide_cmake_builder_build_async (IdeBuilder           *builder,
                               IdeBuilderBuildFlags  flags,
                               IdeBuildResult       **result,
                               GCancellable         *cancellable,
                               GAsyncReadyCallback   callback,
                               gpointer              user_data)
{
}

static IdeBuildResult *
ide_cmake_builder_build_finish (IdeBuilder    *builder,
                                GAsyncResult  *result,
                                GError        **error)
{
  return 0;
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

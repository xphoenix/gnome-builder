#ifndef IDE_CMAKE_BUILDER_STEP_H
#define IDE_CMAKE_BUILDER_STEP_H

#include <gio/gio.h>
#include <ide.h>


#define IDE_TYPE_CMAKE_COMPILE_DB (ide_cmake_compile_db_get_type())
G_DECLARE_FINAL_TYPE (IdeCmakeCompileDB, ide_cmake_compile_db, IDE, CMAKE_COMPILE_DB, GObject)


#ifndef IDE_CMAKE_COMPILE_DB_H
#define IDE_CMAKE_COMPILE_DB_H

#include <gio/gio.h>
#include <ide.h>


#define IDE_TYPE_CMAKE_COMPILE_DB (ide_cmake_compile_db_get_type())
G_DECLARE_FINAL_TYPE (IdeCmakeCompileDb, ide_cmake_compile_db, IDE, CMAKE_COMPILE_DB, GObject)

/*
 * ide_cmake_compile_db_load:
 * Loads compilation commands database from JSON file
 */
IdeCmakeCompileDb *
ide_cmake_compile_db_load(GFile *file, GError **error);

/**
 * ide_cmake_compile_db_get_flags:
 * Returns compilation flags for the given file. If a such file doesn't take a part in
 * the build then NULL returns
 */
const char *
ide_cmake_compile_db_get_flags(IdeCmakeCompileDb *db, const char *file);


#endif /* IDE_CMAKE_COMPILE_DB_H */

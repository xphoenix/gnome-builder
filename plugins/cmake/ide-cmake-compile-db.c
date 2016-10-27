#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include "ide-cmake-compile-db.h"

struct _IdeCmakeCompileDb {
  GPtrArray *rows;
};
G_DEFINE_TYPE (IdeCmakeCompileDb, ide_cmake_compile_db, G_TYPE_OBJECT)

//
// *** Compilation commands database row
//
typedef struct {
  const char *file;
  const char *directory;
  const char *flags;
} CompileCommand;

static void
free_compile_command(gpointer data) {
  CompileCommand *i = (CompileCommand *)data;
  g_free(i->file);
  g_free(i->directory);
  g_free(i->flags);
  g_free(i);
}

static int
compare_compile_command(gconstpointer left,
                        gconstpointer right)
{
  CompileCommand *l = (CompileCommand *)left;
  CompileCommand *r = (CompileCommand *)right;

  return g_strcmp0(l->file, r->file);
}

static int
search_compile_command(const void*left, const void*right)
{

}
//
// *** IdeCmakeCompileDb type implementation
//
static void
ide_cmake_compile_db_finalize (GObject *object)
{
  IdeCmakeCompileDb *self = (IdeCmakeCompileDb *)object;
  g_clear_pointer (&self->rows, g_ptr_array_unref);
  G_OBJECT_CLASS (ide_cmake_compile_db_parent_class)->finalize (object);
}

static void
ide_cmake_compile_db_init (IdeCmakeCompileDb *self)
{
  self->rows = g_ptr_array_new_with_free_func (&free_compile_command);
}

static void
ide_cmake_compile_db_class_init (IdeCmakeCompileDbClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_cmake_compile_db_finalize;
  object_class->get_property = NULL;
  object_class->set_property = NULL;
}

static void
ide_cmake_compile_db_load_process_raw_node (JsonArray *array,
                                            guint      index_,
                                            JsonNode  *element_node,
                                            gpointer   user_data)
{
  IdeCmakeCompileDb *self = IDE_CMAKE_COMPILE_DB(user_data);

  JsonObject *object = json_node_get_object(element_node);
  JsonNode *file = json_object_get_member(object, "file");
  JsonNode *directory = json_object_get_member(object, "directory");
  JsonNode *flags = json_object_get_member(object, "command");

  CompileCommand *i = g_new(CompileCommand, 1);
  i->file = json_node_dup_string(file);
  i->directory = json_node_dup_string(directory);
  i->flags = json_node_dup_string(flags);

  g_ptr_array_add(self->rows, i);
}

//
// *** Methods
//
IdeCmakeCompileDb *
ide_cmake_compile_db_load(GFile *file, GError **error) {
  g_autoptr(JsonParser) parser = json_parser_new_immutable();
  g_autofree char *filepath = g_file_get_path(file);

  JsonNode *root;
  JsonArray *array;


  gboolean loaded = json_parser_load_from_file (parser, filepath, error);
  if (!loaded) {
    g_print ("Unable to parse `%s': %s\n", filepath, (*error)->message);
    return NULL;
  }

  root = json_parser_get_root (parser);
  JsonNodeType type  = json_node_get_node_type (root);

  const char *type_str;
  switch (type) {
  case JSON_NODE_OBJECT:
       type_str = "Object";
       break;
  case JSON_NODE_VALUE:
       type_str = "Value";
       break;
  case JSON_NODE_NULL:
       type_str = "null";
       break;
  case JSON_NODE_ARRAY:
      type_str = "array";
      break;
  default:
      type_str = "Unknown";
  }

  if (type != JSON_NODE_ARRAY) {
    *error = g_error_new (
      G_IO_ERROR,
      G_IO_ERROR_FAILED,
      _("Root object expected to be array but found to be '%s' instead"),
      type_str
    );
    return NULL;
  }

  IdeCmakeCompileDb *db = g_object_new(IDE_TYPE_CMAKE_COMPILE_DB, NULL);

  // Fill database with values
  array = json_node_get_array (root);
  json_array_foreach_element(array, &ide_cmake_compile_db_load_process_raw_node, db);

  // Sort database for access
  g_ptr_array_sort(db->rows, &compare_compile_command);
  return db;
}

const char *
ide_cmake_compile_db_get_flags(IdeCmakeCompileDb *db, const char *file) {
  gconstpointer *ptr = bsearch(file, db->rows->pdata, db->rows->len, sizeof(gpointer), &search_compile_command);

  return prt == NULL ? NULL : ((Com pileCommand*)(*ptr))->flags;
}

/* gb-project-format.c
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
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
#include <json-glib/json-glib.h>

#include "gb-log.h"
#include "gb-project-format.h"

G_DEFINE_TYPE(GbProjectFormat, gb_project_format, G_TYPE_OBJECT)

GbProjectFormat *
gb_project_format_new (void)
{
   return g_object_new(GB_TYPE_PROJECT_FORMAT, NULL);
}

/**
 * gb_project_format_open_async:
 * @format: (in): A #GbProjectFormat.
 * @directory: (in): The directory containing the project.
 * @stream: (in): A #GInputStream.
 * @cancellable: (in) (allow-none): A #GCancellable or %NULL.
 * @callback: (in): A callback to notify of completion.
 * @user_data: (in): User data for @callback.
 *
 * Asynchronously opens a project from @stream using the format implemented
 * by the #GbProjectFormat instance. The default format is a JSON encoded
 * document.
 */
void
gb_project_format_open_async (GbProjectFormat     *format,
                              const gchar         *directory,
                              GInputStream        *stream,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_FORMAT(format));
   g_return_if_fail(directory != NULL);
   g_return_if_fail(G_IS_INPUT_STREAM(stream));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback != NULL);

   GB_PROJECT_FORMAT_GET_CLASS(format)->open_async(format,
                                                   directory,
                                                   stream,
                                                   cancellable,
                                                   callback,
                                                   user_data);

   EXIT;
}

/**
 * gb_project_format_open_finish:
 * @format: (in): A #GbProjectFormat.
 * @result: (in): A #GAsyncResult.
 * @error: (out): A location for a #GError, or %NULL.
 *
 * Completes a request to gb_project_format_open_async(). If successful; a
 * #GbProject instance is returned, otherwise %NULL is returned and @error
 * is set.
 *
 * Returns: (transfer full): A #GbProject if successful; otherwise %NULL.
 */
GbProject *
gb_project_format_open_finish (GbProjectFormat  *format,
                               GAsyncResult     *result,
                               GError          **error)
{
   GbProject *ret;

   ENTRY;

   g_return_val_if_fail(GB_IS_PROJECT_FORMAT(format), NULL);
   g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);

   ret = GB_PROJECT_FORMAT_GET_CLASS(format)->open_finish(format,
                                                          result,
                                                          error);

   RETURN(ret);
}

/**
 * gb_project_format_save_async:
 * @format: (in): A #GbProjectFormat.
 * @stream: (in): A #GOutputStream.
 * @cancellable: (in) (allow-none): A #GCancellable or %NULL.
 * @callback: (in): A callback to notify of completion.
 * @user_data: (in): User data for @callback.
 *
 * Asynchronously saves @project as to @stream using the format implemented
 * by @format. The default format is a JSON encoded document.
 */
void
gb_project_format_save_async (GbProjectFormat     *format,
                              GbProject           *project,
                              GOutputStream       *stream,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_FORMAT(format));
   g_return_if_fail(G_IS_OUTPUT_STREAM(stream));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback != NULL);

   GB_PROJECT_FORMAT_GET_CLASS(format)->save_async(format,
                                                   project,
                                                   stream,
                                                   cancellable,
                                                   callback,
                                                   user_data);

   EXIT;
}

/**
 * gb_project_format_save_finish:
 * @format: (in): A #GbProjectFormat.
 * @result: (in): A #GAsyncResult.
 * @error: (out): A location for a #GError, or %NULL.
 *
 * Completes an asynchronous request to save a project using
 * gb_project_format_save_async().
 *
 * Returns: %TRUE if successful; otherwise %FALSE and @error is set.
 */
gboolean
gb_project_format_save_finish (GbProjectFormat  *format,
                               GAsyncResult     *result,
                               GError          **error)
{
   gboolean ret;

   ENTRY;

   g_return_val_if_fail(GB_IS_PROJECT_FORMAT(format), FALSE);
   g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

   ret = GB_PROJECT_FORMAT_GET_CLASS(format)->save_finish(format,
                                                          result,
                                                          error);

   RETURN(ret);
}

static void
gb_project_format_real_open_cb (GObject      *object,
                                GAsyncResult *result,
                                gpointer      user_data)
{
   GSimpleAsyncResult *simple = user_data;
   const gchar *directory;
   JsonParser *parser = (JsonParser *)object;
   JsonNode *root;
   GObject *project;
   GError *error = NULL;

   ENTRY;

   g_assert(JSON_IS_PARSER(parser));
   g_assert(G_IS_SIMPLE_ASYNC_RESULT(simple));

   if (!json_parser_load_from_stream_finish(parser, result, &error)) {
      g_simple_async_result_take_error(simple, error);
      GOTO(failure);
   }

   root = json_parser_get_root(parser);
   g_assert(root);

   if (!(project = json_gobject_deserialize(GB_TYPE_PROJECT, root))) {
      g_simple_async_result_set_error(simple, GB_PROJECT_FORMAT_ERROR,
                                      GB_PROJECT_FORMAT_ERROR_INVALID_JSON,
                                      _("Failed to deserialize from JSON."));
      GOTO(failure);
   }

   directory = g_object_get_data(G_OBJECT(simple), "directory");
   g_object_set(project, "directory", directory, NULL);
   g_simple_async_result_set_op_res_gpointer(simple, project, g_object_unref);

failure:
   g_simple_async_result_complete_in_idle(simple);
   g_object_unref(simple);

   EXIT;
}

static void
gb_project_format_real_open_async (GbProjectFormat     *format,
                                   const gchar         *directory,
                                   GInputStream        *stream,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
   GSimpleAsyncResult *simple;
   JsonParser *parser;

   ENTRY;

   simple = g_simple_async_result_new(G_OBJECT(format), callback, user_data,
                                      gb_project_format_real_open_async);
   g_object_set_data_full(G_OBJECT(simple), "directory",
                          g_strdup(directory), g_free);

   parser = json_parser_new();
   json_parser_load_from_stream_async(parser, stream, cancellable,
                                      gb_project_format_real_open_cb,
                                      simple);
   g_object_unref(parser);

   EXIT;
}

static GbProject *
gb_project_format_real_open_finish (GbProjectFormat  *format,
                                    GAsyncResult     *result,
                                    GError          **error)
{
   GSimpleAsyncResult *simple = (GSimpleAsyncResult *)result;
   GbProject *ret = NULL;

   ENTRY;

   g_return_val_if_fail(G_IS_SIMPLE_ASYNC_RESULT(simple), NULL);

   if (!(ret = g_simple_async_result_get_op_res_gpointer(simple))) {
      g_simple_async_result_propagate_error(simple, error);
   }

   RETURN(ret ? g_object_ref(ret) : NULL);
}

static void
gb_project_format_real_save_async (GbProjectFormat     *format,
                                   GbProject           *project,
                                   GOutputStream       *stream,
                                   GCancellable        *cancellable,
                                   GAsyncReadyCallback  callback,
                                   gpointer             user_data)
{
   ENTRY;

   EXIT;
}

static gboolean
gb_project_format_real_save_finish (GbProjectFormat  *format,
                                    GAsyncResult     *result,
                                    GError          **error)
{
   gboolean ret = FALSE;

   ENTRY;

   RETURN(ret);
}

static void
gb_project_format_class_init (GbProjectFormatClass *klass)
{
   klass->open_async = gb_project_format_real_open_async;
   klass->open_finish = gb_project_format_real_open_finish;
   klass->save_async = gb_project_format_real_save_async;
   klass->save_finish = gb_project_format_real_save_finish;
}

static void
gb_project_format_init (GbProjectFormat *format)
{
}

GQuark
gb_project_format_error_quark (void)
{
   return g_quark_from_static_string("gb-project-format-error");
}

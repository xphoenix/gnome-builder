/* gb-buffer-tasks.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
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
#include <string.h>

#include "gb-buffer-tasks.h"

#define CHUNK_SIZE 8192
#define UPDATE_PROGRESS(current, total) \
   if (progress_callback) { \
      gsize pos = current; \
      if (G_IS_SEEKABLE(progress_stream)) { \
         pos = g_seekable_tell(progress_stream); \
      } \
      progress_callback(pos, total, progress_data); \
   }

typedef struct
{
   GtkTextBuffer         *buffer;
   GFile                 *file;
   GBytes                *bytes;
   GConverter            *coder;
   GConverter            *compressor;
   GFileProgressCallback  progress_callback;
   gpointer               progress_data;
} GbBufferTaskData;

static void
gb_buffer_task_data_free (gpointer data)
{
   GbBufferTaskData *task_data = data;

   if (task_data) {
      g_clear_object(&task_data->buffer);
      g_clear_object(&task_data->file);
      g_clear_object(&task_data->coder);
      g_clear_object(&task_data->compressor);
      g_clear_pointer(&task_data->bytes, g_bytes_unref);
      g_free(task_data);
   }
}

static GBytes *
_g_input_stream_read_eof_bytes (GInputStream           *input_stream,
                                gsize                   expected_size,
                                GSeekable              *progress_stream,
                                GFileProgressCallback   progress_callback,
                                gpointer                progress_data,
                                GCancellable           *cancellable,
                                GError                **error)
{
   GByteArray *array;
   gssize r;
   guint8 buf[CHUNK_SIZE];

   g_return_val_if_fail(G_IS_INPUT_STREAM(input_stream), NULL);
   g_return_val_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable), NULL);

   array = g_byte_array_sized_new(expected_size);

   while (TRUE) {
      UPDATE_PROGRESS(array->len, expected_size);
      r = g_input_stream_read(input_stream,
                              buf,
                              sizeof buf,
                              cancellable,
                              error);
      if (r == -1) {
         g_byte_array_free(array, TRUE);
         return NULL;
      } else if (r == 0) {
         UPDATE_PROGRESS(array->len, expected_size);
         return g_byte_array_free_to_bytes(array);
      } else {
         g_byte_array_append(array, buf, r);
      }
   }
}

static void
gb_buffer_load_task_run (GTask        *task,
                         gpointer      source_object,
                         gpointer      task_data,
                         GCancellable *cancellable)
{
   GbBufferTaskData *data = task_data;
   GFileInputStream *file_stream = NULL;
   GInputStream *tmp_stream = NULL;
   GInputStream *stream = NULL;
   GFileInfo *file_info;
   GBytes *bytes;
   GError *error = NULL;
   gchar *str;
   gsize len;
   gsize expected_size = 0;

   g_return_if_fail(G_IS_TASK(task));
   g_return_if_fail(task_data);
   g_return_if_fail(data->file);

   /*
    * Build our input stream chain that may be doing inline decoding
    * and decompression.
    */
   file_stream = g_file_read(data->file, cancellable, &error);
   if (!file_stream) {
      g_task_return_error(task, error);
      goto cleanup;
   }
   stream = g_object_ref(file_stream);
   if (data->compressor) {
      tmp_stream = stream;
      stream = g_converter_input_stream_new(stream, data->compressor);
      g_clear_object(&tmp_stream);
   }
   if (data->coder) {
      tmp_stream = stream;
      stream = g_converter_input_stream_new(stream, data->coder);
      g_clear_object(&tmp_stream);
   }

   /*
    * Query the filesystem to determine file size to use for progress.
    * It is okay if this fails, we just can't provide meaningful progress
    * if that is the case.
    */
   file_info = g_file_query_info(data->file,
                                 G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                 G_FILE_QUERY_INFO_NONE,
                                 cancellable,
                                 NULL);
   if (file_info) {
      expected_size =
         g_file_info_get_attribute_uint64(file_info,
                                          G_FILE_ATTRIBUTE_STANDARD_SIZE);
   }

   /*
    * Read in loop until all of the file contents have been read.
    * Use the position in the underlying file stream for progress
    * callbacks.
    */
   bytes = _g_input_stream_read_eof_bytes(stream,
                                          expected_size,
                                          G_SEEKABLE(file_stream),
                                          data->progress_callback,
                                          data->progress_data,
                                          cancellable,
                                          &error);

   /*
    * Check to verify that our data is in UTF-8 format.
    */
   str = (gchar *)g_bytes_get_data(bytes, &len);
   if (!g_utf8_validate(str, len, NULL)) {
      g_bytes_unref(bytes);
      g_task_return_new_error(task,
                              G_IO_ERROR,
                              G_IO_ERROR_INVALID_DATA,
                              _("Invalid encoding within file."));
      goto cleanup;
   }

   g_task_return_pointer(task, bytes, (GDestroyNotify)g_bytes_unref);

cleanup:
   g_clear_object(&file_stream);
   g_clear_object(&stream);
}

GTask *
gb_buffer_load_task_new (gpointer               source_object,
                         GFile                 *file,
                         GConverter            *decoder,
                         GConverter            *decompressor,
                         GCancellable          *cancellable,
                         GFileProgressCallback  progress_callback,
                         gpointer               progress_data,
                         GAsyncReadyCallback    callback,
                         gpointer               user_data)
{
   GbBufferTaskData *data;
   GTask *task;

   g_return_val_if_fail(G_IS_FILE(file), NULL);
   g_return_val_if_fail(!decoder || G_IS_CONVERTER(decoder), NULL);
   g_return_val_if_fail(!decompressor || G_IS_CONVERTER(decompressor), NULL);
   g_return_val_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable), NULL);
   g_return_val_if_fail(callback, NULL);

   data = g_new0(GbBufferTaskData, 1);
   data->file = g_object_ref(file);
   data->coder = decoder ? g_object_ref(decoder) : NULL;
   data->compressor = decompressor ? g_object_ref(decompressor) : NULL;
   data->progress_callback = progress_callback;
   data->progress_data = progress_data;

   task = g_task_new(source_object, cancellable, callback, user_data);
   g_task_set_task_data(task, data, gb_buffer_task_data_free);
   g_task_run_in_thread(task, gb_buffer_load_task_run);

   return task;
}

static gboolean
_g_output_stream_write_all_bytes (GOutputStream          *output_stream,
                                  GBytes                 *bytes,
                                  GFileProgressCallback   progress_callback,
                                  gpointer                progress_data,
                                  GCancellable           *cancellable,
                                  GError                **error)
{
   const gchar *buf;
   gssize r;
   gsize buflen;
   gsize off = 0;

   g_return_val_if_fail (G_IS_OUTPUT_STREAM (output_stream), FALSE);
   g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

   buf = g_bytes_get_data (bytes, &buflen);

   while (TRUE) {
      if (progress_callback) {
         progress_callback(off, buflen, progress_data);
      }

      r = g_output_stream_write (output_stream,
                                 buf + off,
                                 MIN (buflen - off, CHUNK_SIZE),
                                 cancellable,
                                 error);

      if (r <= 0) {
         return FALSE;
      } else {
         off += r;
      }

      if (off == buflen) {
         if (progress_callback) {
            progress_callback(off, buflen, progress_data);
         }
         return TRUE;
      }
   }
}

static void
gb_buffer_save_task_run (GTask        *task,
                         gpointer      source_object,
                         gpointer      task_data,
                         GCancellable *cancellable)
{
   GFileOutputStream *file_stream = NULL;
   GbBufferTaskData *data = task_data;
   GOutputStream *tmp_stream = NULL;
   GOutputStream *stream = NULL;
   GError *error = NULL;

   g_return_if_fail (G_IS_TASK (task));
   g_return_if_fail (task_data);
   g_return_if_fail (data->file);

   /*
    * Build our output stream chain that may be doing inline encoding
    * and compression.
    */
   file_stream = g_file_replace(data->file,
                                NULL,
                                TRUE, /* TODO: Wrap Make Backup */
                                G_FILE_CREATE_REPLACE_DESTINATION,
                                cancellable,
                                &error);
   if (!file_stream) {
      g_task_return_error(task, error);
      goto cleanup;
   }
   stream = g_object_ref(file_stream);
   if (data->compressor) {
      tmp_stream = stream;
      stream = g_converter_output_stream_new(stream, data->compressor);
      g_clear_object(&tmp_stream);
   }
   if (data->coder) {
      tmp_stream = stream;
      stream = g_converter_output_stream_new(stream, data->coder);
      g_clear_object(&tmp_stream);
   }

   if (!_g_output_stream_write_all_bytes (stream,
                                          data->bytes,
                                          data->progress_callback,
                                          data->progress_data,
                                          cancellable,
                                          &error)) {
      g_task_return_error(task, error);
   } else {
      g_task_return_boolean(task, TRUE);
   }

cleanup:
   g_clear_object(&file_stream);
   g_clear_object(&stream);
}

GTask *
gb_buffer_save_task_new (gpointer               source_object,
                         GFile                 *file,
                         GConverter            *encoder,
                         GConverter            *compressor,
                         GCancellable          *cancellable,
                         GFileProgressCallback  progress_callback,
                         gpointer               progress_data,
                         GAsyncReadyCallback    callback,
                         gpointer               user_data)
{
   GbBufferTaskData *data;
   GtkTextBuffer *buffer = source_object;
   GtkTextIter begin;
   GtkTextIter end;
   GTask *task;
   gchar *text;

   g_return_val_if_fail(GTK_IS_TEXT_BUFFER(buffer), NULL);
   g_return_val_if_fail(G_IS_FILE(file), NULL);
   g_return_val_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable), NULL);
   g_return_val_if_fail(callback, NULL);

   gtk_text_buffer_get_bounds(buffer, &begin, &end);
   text = gtk_text_buffer_get_text(source_object, &begin, &end, TRUE);

   data = g_new0 (GbBufferTaskData, 1);
   data->file = g_object_ref (file);
   data->bytes = g_bytes_new_take (text, strlen(text));
   data->coder = encoder ? g_object_ref (encoder) : NULL;
   data->compressor = compressor ? g_object_ref (compressor) : NULL;
   data->progress_callback = progress_callback;
   data->progress_data = progress_data;

   task = g_task_new (source_object, cancellable, callback, user_data);
   g_task_set_task_data (task, data, gb_buffer_task_data_free);
   g_task_run_in_thread (task, gb_buffer_save_task_run);

   return task;
}

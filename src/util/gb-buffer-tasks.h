/* gb-buffer-tasks.h
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

#ifndef GB_BUFFER_TASKS_H
#define GB_BUFFER_TASKS_H

#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

GTask *gb_buffer_load_task_new (gpointer               source_object,
                                GFile                 *file,
                                GConverter            *decoder,
                                GConverter            *decompressor,
                                GCancellable          *cancellable,
                                GFileProgressCallback  progress_callback,
                                gpointer               progress_data,
                                GAsyncReadyCallback    callback,
                                gpointer               user_data);

GTask *gb_buffer_save_task_new (GtkTextBuffer         *buffer,
                                GFile                 *file,
                                GConverter            *endecoder,
                                GConverter            *compressor,
                                GCancellable          *cancellable,
                                GFileProgressCallback  progress_callback,
                                gpointer               progress_data,
                                GAsyncReadyCallback    callback,
                                gpointer               user_data);

G_END_DECLS

#endif /* GB_BUFFER_TASKS_H */

/* gb-gtk.c
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

#include "gb-animation.h"
#include "gb-gtk.h"

gpointer
gb_gtk_builder_load_and_get_object (const gchar *resource_path,
                                    const gchar *name)
{
   GtkBuilder *builder;
   gpointer ret = NULL;
   GError *error = NULL;

   g_return_val_if_fail(resource_path, NULL);
   g_return_val_if_fail(name, NULL);

   builder = gtk_builder_new();

   if (!gtk_builder_add_from_resource(builder, resource_path, &error)) {
      g_error("%s", error->message); /* Fatal */
      g_error_free(error);
      goto failure;
   }

   if ((ret = gtk_builder_get_object(builder, name))) {
      ret = g_object_ref(ret);
   }

failure:
   g_object_unref(builder);

   return ret;
}

typedef struct
{
   GtkProgressBar *progress_bar;
   gdouble fraction;
} ProgressUpdate;

static gboolean
update_progress (gpointer data)
{
   ProgressUpdate *update = data;
   GbAnimation *animation;

   g_return_val_if_fail(update, G_SOURCE_REMOVE);

   animation = g_object_get_data(G_OBJECT(update->progress_bar), "animation");
   if (animation) {
      gb_animation_stop(animation);
   }

   animation = gb_object_animate(update->progress_bar,
                                 GB_ANIMATION_EASE_IN_OUT_QUAD,
                                 250,
                                 NULL,
                                 "fraction", update->fraction,
                                 NULL);
   g_object_set_data_full(G_OBJECT(update->progress_bar),
                          "animation",
                          g_object_ref(animation),
                          g_object_unref);

   g_object_unref(update->progress_bar);
   g_free(update);

   return G_SOURCE_REMOVE;
}

void
gb_gtk_progress_bar_file_progress_callback (goffset  current_num_bytes,
                                            goffset  total_num_bytes,
                                            gpointer user_data)
{
   GtkProgressBar *progress_bar = user_data;
   ProgressUpdate *update;

   g_return_if_fail(GTK_IS_PROGRESS_BAR(progress_bar));

   update = g_new0(ProgressUpdate, 1);
   update->progress_bar = g_object_ref(progress_bar);

   if (total_num_bytes) {
      update->fraction = (gdouble)current_num_bytes / (gdouble)total_num_bytes;
   }

   g_idle_add(update_progress, update);
}

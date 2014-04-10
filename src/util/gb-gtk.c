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
                                 350,
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

static gboolean
hide_in_idle (gpointer data)
{
   gtk_widget_hide (data);
   g_object_unref (data);

   return FALSE;
}

void
gb_gtk_widget_hide_in_idle (guint      timeout_msec,
                            GtkWidget *widget)
{
   g_timeout_add (timeout_msec, hide_in_idle, g_object_ref (widget));
}

/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*************************************************************
 * The following code has been modified to work for GdkRGBA. *
 *************************************************************/

/**
 * _gb_rgba_to_hls:
 * @color: a #GdkRGBA
 * @hue: (out): return location for the hue value or %NULL
 * @luminance: (out): return location for the luminance value or %NULL
 * @saturation: (out): return location for the saturation value or %NULL
 *
 * Converts @color to the HLS format.
 *
 * The @hue value is in the 0 .. 360 range. The @luminance and
 * @saturation values are in the 0 .. 1 range.
 */
void
_gb_rgba_to_hls (const GdkRGBA *color,
		           float         *hue,
		           float         *luminance,
		           float         *saturation)
{
  float red, green, blue;
  float min, max, delta;
  float h, l, s;

  g_return_if_fail (color != NULL);

  red   = color->red;
  green = color->green;
  blue  = color->blue;

  if (red > green)
    {
      if (red > blue)
	max = red;
      else
	max = blue;

      if (green < blue)
	min = green;
      else
	min = blue;
    }
  else
    {
      if (green > blue)
	max = green;
      else
	max = blue;

      if (red < blue)
	min = red;
      else
	min = blue;
    }

  l = (max + min) / 2;
  s = 0;
  h = 0;

  if (max != min)
    {
      if (l <= 0.5)
	s = (max - min) / (max + min);
      else
	s = (max - min) / (2.0 - max - min);

      delta = max - min;

      if (red == max)
	h = (green - blue) / delta;
      else if (green == max)
	h = 2.0 + (blue - red) / delta;
      else if (blue == max)
	h = 4.0 + (red - green) / delta;

      h *= 60;

      if (h < 0)
	h += 360.0;
    }

  if (hue)
    *hue = h;

  if (luminance)
    *luminance = l;

  if (saturation)
    *saturation = s;
}

/**
 * _gb_rgba_from_hls:
 * @color: (out): return location for a #GdkRGBA
 * @hue: hue value, in the 0 .. 360 range
 * @luminance: luminance value, in the 0 .. 1 range
 * @saturation: saturation value, in the 0 .. 1 range
 *
 * Converts a color expressed in HLS (hue, luminance and saturation)
 * values into a #GdkRGBA.
 */
void
_gb_rgba_from_hls (GdkRGBA *color,
			          float    hue,
			          float    luminance,
			          float    saturation)
{
  float tmp1, tmp2;
  float tmp3[3];
  float clr[3];
  int   i;

  hue /= 360.0;

  if (saturation == 0)
    {
      color->red = color->green = color->blue = luminance;

      return;
    }

  if (luminance <= 0.5)
    tmp2 = luminance * (1.0 + saturation);
  else
    tmp2 = luminance + saturation - (luminance * saturation);

  tmp1 = 2.0 * luminance - tmp2;

  tmp3[0] = hue + 1.0 / 3.0;
  tmp3[1] = hue;
  tmp3[2] = hue - 1.0 / 3.0;

  for (i = 0; i < 3; i++)
    {
      if (tmp3[i] < 0)
        tmp3[i] += 1.0;

      if (tmp3[i] > 1)
        tmp3[i] -= 1.0;

      if (6.0 * tmp3[i] < 1.0)
        clr[i] = tmp1 + (tmp2 - tmp1) * tmp3[i] * 6.0;
      else if (2.0 * tmp3[i] < 1.0)
        clr[i] = tmp2;
      else if (3.0 * tmp3[i] < 2.0)
        clr[i] = (tmp1 + (tmp2 - tmp1) * ((2.0 / 3.0) - tmp3[i]) * 6.0);
      else
        clr[i] = tmp1;
    }

  color->red   = clr[0];
  color->green = clr[1];
  color->blue  = clr[2];
}

/**
 * _gb_rgba_shade:
 * @color: a #GdkRGBA
 * @factor: the shade factor to apply
 * @result: (out caller-allocates): return location for the shaded color
 *
 * Shades @color by @factor and saves the modified color into @result.
 */
void
_gb_rgba_shade (const GdkRGBA *color,
                gdouble        factor,
                GdkRGBA       *result)
{
  float h, l, s;

  g_return_if_fail (color != NULL);
  g_return_if_fail (result != NULL);

  _gb_rgba_to_hls (color, &h, &l, &s);

  l = CLAMP (l * factor, 0.0, 1.0);
  s = CLAMP (s * factor, 0.0, 1.0);

  _gb_rgba_from_hls (result, h, l, s);

  result->alpha = color->alpha;
}

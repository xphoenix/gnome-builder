/* gb-icon-theme.c
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

#include <gtk/gtk.h>
#include <stdlib.h>

#include "gb-application-resource.h"
#include "gb-icon-theme.h"

static GHashTable *gPixbufs;

static gchar *
get_path (const gchar *icon_name,
          gint         scale)
{
   gchar *path;

   if (scale != 1) {
      path = g_strdup_printf("/org/gnome/Builder/data/icons/%s-16x@%d.png",
                             icon_name, scale);
   } else {
      path = g_strdup_printf("/org/gnome/Builder/data/icons/%s-16x.png",
                             icon_name);
   }

   return path;
}

static void
apply_overlay (GdkPixbuf   *base,
               const gchar *overlay,
               gint         scale)
{
   GdkPixbuf *tmp;
   gchar *path;

   path = get_path(overlay, scale);
   tmp = gdk_pixbuf_new_from_resource(path, NULL);
   if (!tmp) {
      g_warning("Failed to locate resource \"%s\"", path);
      g_free(path);
      return;
   }
   g_free(path);

   gdk_pixbuf_composite(tmp, base, 0, 0,
                        gdk_pixbuf_get_width(tmp),
                        gdk_pixbuf_get_height(tmp),
                        0, 0,
                        1, 1,
                        GDK_INTERP_BILINEAR,
                        255);

   g_object_unref(tmp);
}

static void
cleanup_pixbufs (void)
{
   g_hash_table_unref(gPixbufs);
   gPixbufs = NULL;
}

static GdkPixbuf *
get_pixbuf (const gchar *name,
            gint         scale,
            const gchar *overlay1,
            const gchar *overlay2)
{
   GdkPixbuf *pixbuf;
   gchar *path;
   gchar *key;

   if (!gPixbufs) {
      gPixbufs = g_hash_table_new(g_str_hash, g_str_equal);
      atexit(cleanup_pixbufs);
   }

   key = g_strdup_printf("%s-%s-%s", name, overlay1, overlay2);

   if (!(pixbuf = g_hash_table_lookup(gPixbufs, key))) {
      path = get_path(name, scale);
      pixbuf = gdk_pixbuf_new_from_resource(path, NULL);
      g_free(path);

      if (overlay1) {
         apply_overlay(pixbuf, overlay1, scale);
      }

      if (overlay2) {
         apply_overlay(pixbuf, overlay2, scale);
      }

      g_hash_table_insert(gPixbufs, g_strdup(key), pixbuf);
   }

   g_free(key);

   return pixbuf;
}

GdkPixbuf *
gb_icon_theme_load_with_overlay (const gchar *icon_name,
                                 gint         scale,
                                 const gchar *overlay1,
                                 const gchar *overlay2)
{
   return get_pixbuf(icon_name, scale, overlay1, overlay2);
}

void
gb_icon_theme_init (void)
{
   static gsize initialized;

   if (g_once_init_enter(&initialized)) {
      GdkPixbuf *pixbuf;

#define REGISTER_ICON(name, size, path) \
      G_STMT_START { \
         pixbuf = gdk_pixbuf_new_from_resource("/org/gnome/Builder/data/icons/"path, NULL); \
         gtk_icon_theme_add_builtin_icon(name, size, pixbuf); \
         g_object_unref(pixbuf); \
      } G_STMT_END

      REGISTER_ICON("gb-project", 16, "project-16x.png");
      REGISTER_ICON("text-x-chdr", 16, "text-x-chdr-16x.png");
      REGISTER_ICON("text-x-csrc", 16, "text-x-csrc-16x.png");

      g_once_init_leave(&initialized, TRUE);
   }
}

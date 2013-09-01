/* gb-source-fullscreen-container.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcestyle.h>
#include <gtksourceview/gtksourcestylescheme.h>
#include <gtksourceview/gtksourceview.h>
#include <string.h>

#include "gb-source-fullscreen-container.h"

G_DEFINE_TYPE(GbSourceFullscreenContainer,
              gb_source_fullscreen_container,
              GTK_TYPE_BIN)

struct _GbSourceFullscreenContainerPrivate
{
   GtkWidget *box;
   GtkWidget *scroller;
   GtkWidget *source_view;
   GtkWidget *spacer;

   PangoLayout *layout;
};

static gint
get_text_width (GbSourceFullscreenContainer *container)
{
   GbSourceFullscreenContainerPrivate *priv = container->priv;
   PangoFontDescription *font;
   GtkStyleContext *context;
   gboolean show_right_margin;
   GString *str;
   gint right_margin_pos;
   gint i;
   gint margin;
   gint width = 0;

   if (!priv->layout || !priv->source_view) {
      return 0;
   }

   g_object_get(priv->source_view,
                "right-margin-position", &right_margin_pos,
                "show-right-margin", &show_right_margin,
                NULL);

   if (!show_right_margin) {
      return 0;
   }

   context = gtk_widget_get_style_context(priv->source_view);
   gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL,
                         "font", &font,
                         NULL);
   pango_layout_set_font_description(priv->layout, font);
   pango_font_description_free(font);

   str = g_string_new(NULL);
   for (i = 0; i < right_margin_pos; i++) {
      g_string_append_c(str, ' ');
   }
   pango_layout_set_text(priv->layout, str->str, str->len);
   g_string_free(str, TRUE);

   pango_layout_get_pixel_size(priv->layout, &width, NULL);
   margin = gtk_text_view_get_left_margin(GTK_TEXT_VIEW(priv->source_view));

   return MAX(0, width + margin);
}

static void
set_source_view (GbSourceFullscreenContainer *container,
                 GtkWidget                   *child)
{
   GbSourceFullscreenContainerPrivate *priv = container->priv;
   PangoContext *context;

   if (priv->source_view) {
      gtk_container_remove(GTK_CONTAINER(priv->scroller), priv->source_view);
      g_object_remove_weak_pointer(G_OBJECT(priv->source_view),
                                   (gpointer *)&priv->source_view);
      priv->source_view = NULL;
      g_clear_object(&priv->layout);
   }

   if (child) {
      priv->source_view = child;
      g_object_add_weak_pointer(G_OBJECT(child),
                                (gpointer *)&priv->source_view);
      gtk_container_add(GTK_CONTAINER(priv->scroller), child);
      context = gtk_widget_get_pango_context(child);
      priv->layout = pango_layout_new(context);
   }

   gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void
get_colors (GbSourceFullscreenContainer *container,
            GdkRGBA                     *bg_rgba,
            GdkRGBA                     *fg_rgba,
            GdkRGBA                     *text_rgba)
{
   GbSourceFullscreenContainerPrivate *priv = container->priv;
   GtkSourceStyleScheme *scheme;
   GtkSourceStyle *style;
   GtkTextBuffer *buffer;
   char *str;

   memset(bg_rgba, 0, sizeof *bg_rgba);
   memset(fg_rgba, 0, sizeof *fg_rgba);
   memset(text_rgba, 0, sizeof *text_rgba);

   if (!priv->source_view) {
      return;
   }

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->source_view));
   scheme = gtk_source_buffer_get_style_scheme(GTK_SOURCE_BUFFER(buffer));
   if (!scheme) {
      return;
   }

   style = gtk_source_style_scheme_get_style(scheme, "text");
   if (style) {
      g_object_get(style, "background", &str, NULL);
      if (str) {
         gdk_rgba_parse(bg_rgba, str);
         gdk_rgba_parse(text_rgba, str);
         g_free(str);
      }
   }

   style = gtk_source_style_scheme_get_style(scheme, "right-margin");
   if (style) {
      g_object_get(style, "background", &str, NULL);
      if (str) {
         gdk_rgba_parse(bg_rgba, str);
         bg_rgba->alpha = 15. / 255.;
         gdk_rgba_parse(fg_rgba, str);
         fg_rgba->alpha = 40. / 255.;
         g_free(str);
      } else {
         bg_rgba->alpha = 0.0;
         fg_rgba->alpha = 0.0;
      }
   }

   return;
}

static gboolean
draw_spacer (GbSourceFullscreenContainer *container,
             cairo_t                     *cr,
             GtkDrawingArea              *spacer)
{
   GtkStyleContext *context;
   GtkAllocation alloc;
   GdkWindow *window;
   GdkRGBA bg_rgba;
   GdkRGBA fg_rgba;
   GdkRGBA text_rgba;

   window = gtk_widget_get_window(GTK_WIDGET(spacer));
   if (!gtk_cairo_should_draw_window(cr, window)) {
      return FALSE;
   }

   gtk_widget_get_allocation(GTK_WIDGET(spacer), &alloc);
   alloc.x = 0;
   alloc.y = 0;

   context = gtk_widget_get_style_context(container->priv->spacer);
   gtk_style_context_save(context);
   gtk_style_context_add_class(context, GTK_STYLE_CLASS_VIEW);
   gtk_render_background(context, cr, alloc.x, alloc.y,
                         alloc.width, alloc.height);
   gtk_style_context_restore(context);

   get_colors(container, &bg_rgba, &fg_rgba, &text_rgba);

   cairo_save(cr);

   gdk_cairo_rectangle(cr, &alloc);
   gdk_cairo_set_source_rgba(cr, &text_rgba);
   cairo_fill_preserve(cr);
   gdk_cairo_set_source_rgba(cr, &bg_rgba);
   cairo_fill(cr);

   cairo_set_line_width(cr, 1.0);
   cairo_move_to(cr, alloc.x + alloc.width - 0.5, alloc.y);
   cairo_line_to(cr, alloc.x + alloc.width - 0.5, alloc.y + alloc.height);
   gdk_cairo_set_source_rgba(cr, &fg_rgba);
   cairo_stroke(cr);

   cairo_restore(cr);

   return FALSE;
}

static void
gb_source_fullscreen_container_add (GtkContainer *container,
                                    GtkWidget    *child)
{
   g_return_if_fail(GB_IS_SOURCE_FULLSCREEN_CONTAINER(container));

   if (GTK_SOURCE_IS_VIEW(child)) {
      set_source_view(GB_SOURCE_FULLSCREEN_CONTAINER(container), child);
   } else {
      GTK_CONTAINER_CLASS(gb_source_fullscreen_container_parent_class)->
            add(container, child);
   }
}

static void
gb_source_fullscreen_container_size_allocate (GtkWidget     *widget,
                                              GtkAllocation *alloc)
{
   GbSourceFullscreenContainerPrivate *priv;
   gint width_request;
   gint text_width;
   gint cur_width;

   g_assert(GB_IS_SOURCE_FULLSCREEN_CONTAINER(widget));

   priv = GB_SOURCE_FULLSCREEN_CONTAINER(widget)->priv;

   text_width = get_text_width(GB_SOURCE_FULLSCREEN_CONTAINER(widget));
   if (!text_width) {
      GTK_WIDGET_CLASS(gb_source_fullscreen_container_parent_class)->
            size_allocate(widget, alloc);
      return;
   }

   width_request = MAX ((alloc->width - text_width) / 2, 0);
   g_object_get(priv->spacer,
                "width-request", &cur_width,
                NULL);

   if (cur_width != width_request) {
      g_object_set(priv->spacer,
                   "width-request", MAX(0, width_request),
                   NULL);
   }

   GTK_WIDGET_CLASS(gb_source_fullscreen_container_parent_class)->
         size_allocate(widget, alloc);
}

static void
gb_source_fullscreen_container_finalize (GObject *object)
{
   GbSourceFullscreenContainerPrivate *priv;

   priv = GB_SOURCE_FULLSCREEN_CONTAINER(object)->priv;

   set_source_view(GB_SOURCE_FULLSCREEN_CONTAINER(object), NULL);

   g_clear_object(&priv->layout);

   G_OBJECT_CLASS(gb_source_fullscreen_container_parent_class)->finalize(object);
}

static void
gb_source_fullscreen_container_class_init (GbSourceFullscreenContainerClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;
   GtkContainerClass *container_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_fullscreen_container_finalize;
   g_type_class_add_private(object_class,
                            sizeof(GbSourceFullscreenContainerPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->size_allocate = gb_source_fullscreen_container_size_allocate;

   container_class = GTK_CONTAINER_CLASS(klass);
   container_class->add = gb_source_fullscreen_container_add;
}

static void
gb_source_fullscreen_container_init (GbSourceFullscreenContainer *container)
{
   GbSourceFullscreenContainerPrivate *priv;

   container->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(container,
                                  GB_TYPE_SOURCE_FULLSCREEN_CONTAINER,
                                  GbSourceFullscreenContainerPrivate);
   priv = container->priv;

   priv->box = g_object_new(GTK_TYPE_BOX,
                            "orientation", GTK_ORIENTATION_HORIZONTAL,
                            "visible", TRUE,
                            NULL);
   gtk_container_add(GTK_CONTAINER(container), priv->box);

   priv->spacer = g_object_new(GTK_TYPE_DRAWING_AREA,
                               "hexpand", FALSE,
                               "visible", TRUE,
                               NULL);
   g_signal_connect_swapped(priv->spacer, "draw",
                            G_CALLBACK(draw_spacer),
                            container);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->box), priv->spacer,
                                     "expand", FALSE,
                                     "position", 0,
                                     NULL);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "hexpand", TRUE,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add_with_properties(GTK_CONTAINER(priv->box), priv->scroller,
                                     "expand", TRUE,
                                     "fill", TRUE,
                                     "position", 1,
                                     NULL);
}

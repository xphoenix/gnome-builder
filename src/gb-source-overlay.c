/* gb-source-overlay.c
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

#include "gb-source-overlay.h"
#include "gb-util.h"

G_DEFINE_TYPE(GbSourceOverlay, gb_source_overlay, GTK_TYPE_DRAWING_AREA)

#define DEFAULT_BG_HEX "rgba(0,0,0,0.6)"

struct _GbSourceOverlayPrivate
{
   GdkRGBA                 bg;
   GtkSourceView          *source_view;
   GtkSourceSearchContext *search_context;
};

enum
{
   PROP_0,
   PROP_BACKGROUND_RGBA,
   PROP_SEARCH_CONTEXT,
   PROP_SOURCE_VIEW,
   LAST_PROP
};

typedef void (*GbSourceOverlayMatchFunc) (GbSourceOverlay   *overlay,
                                          const GtkTextIter *match_begin,
                                          const GtkTextIter *match_end,
                                          gpointer           user_data);

static GParamSpec *gParamSpecs[LAST_PROP];

static void
rounded_rectangle (cairo_t *cr,
                   gint     x,
                   gint     y,
                   gint     width,
                   gint     height,
                   gint     x_radius,
                   gint     y_radius)
{
   gint x1, x2;
   gint y1, y2;
   gint xr1, xr2;
   gint yr1, yr2;

   x1 = x;
   x2 = x1 + width;
   y1 = y;
   y2 = y1 + height;

   x_radius = MIN (x_radius, width / 2.0);
   y_radius = MIN (y_radius, width / 2.0);

   xr1 = x_radius;
   xr2 = x_radius / 2.0;
   yr1 = y_radius;
   yr2 = y_radius / 2.0;

   cairo_move_to (cr, x1 + xr1, y1);
   cairo_line_to (cr, x2 - xr1, y1);
   cairo_curve_to (cr, x2 - xr2, y1, x2, y1 + yr2, x2, y1 + yr1);
   cairo_line_to (cr, x2, y2 - yr1);
   cairo_curve_to (cr, x2, y2 - yr2, x2 - xr2, y2, x2 - xr1, y2);
   cairo_line_to (cr, x1 + xr1, y2);
   cairo_curve_to (cr, x1 + xr2, y2, x1, y2 - yr2, x1, y2 - yr1);
   cairo_line_to (cr, x1, y1 + yr1);
   cairo_curve_to (cr, x1, y1 + yr2, x1 + xr2, y1, x1 + xr1, y1);
   cairo_close_path (cr);
}

GtkWidget *
gb_source_overlay_new (GtkSourceView *source_view,
                       GtkSourceSearchContext *search_context)
{
   g_return_val_if_fail(GTK_SOURCE_IS_VIEW(source_view), NULL);
   g_return_val_if_fail(GTK_SOURCE_IS_SEARCH_CONTEXT(search_context), NULL);

   return g_object_new(GB_TYPE_SOURCE_OVERLAY,
                       "source-view", source_view,
                       "search-context", search_context,
                       NULL);
}

const GdkRGBA *
gb_source_overlay_get_background_rgba (GbSourceOverlay *overlay)
{
   g_return_val_if_fail(GB_IS_SOURCE_OVERLAY(overlay), NULL);

   return &overlay->priv->bg;
}

void
gb_source_overlay_set_background_rgba (GbSourceOverlay *overlay,
                                       const GdkRGBA   *background_rgba)
{
   GbSourceOverlayPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_OVERLAY(overlay));

   priv = overlay->priv;

   if (background_rgba) {
      memcpy(&priv->bg, background_rgba, sizeof priv->bg);
   } else {
      gdk_rgba_parse(&priv->bg, DEFAULT_BG_HEX);
   }

   g_object_notify_by_pspec(G_OBJECT(overlay),
                            gParamSpecs[PROP_BACKGROUND_RGBA]);
}

GtkSourceView *
gb_source_overlay_get_source_view (GbSourceOverlay *overlay)
{
   g_return_val_if_fail(GB_IS_SOURCE_OVERLAY(overlay), NULL);

   return overlay->priv->source_view;
}

void
gb_source_overlay_set_source_view (GbSourceOverlay *overlay,
                                   GtkSourceView   *source_view)
{
   GbSourceOverlayPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_OVERLAY(overlay));
   g_return_if_fail(!source_view || GTK_SOURCE_IS_VIEW(source_view));

   priv = overlay->priv;

   if (priv->source_view != source_view) {
      g_clear_object(&priv->source_view);
      priv->source_view = source_view ? g_object_ref(source_view) : NULL;
      g_object_notify_by_pspec(G_OBJECT(overlay),
                               gParamSpecs[PROP_SOURCE_VIEW]);
   }
}

GtkSourceSearchContext *
gb_source_overlay_get_search_context (GbSourceOverlay *overlay)
{
   g_return_val_if_fail(GB_IS_SOURCE_OVERLAY(overlay), NULL);

   return overlay->priv->search_context;
}

void
gb_source_overlay_set_search_context (GbSourceOverlay        *overlay,
                                      GtkSourceSearchContext *search_context)
{
   GbSourceOverlayPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_OVERLAY(overlay));
   g_return_if_fail(!search_context ||
                    GTK_SOURCE_IS_SEARCH_CONTEXT(search_context));

   priv = overlay->priv;

   if (priv->search_context != search_context) {
      g_clear_object(&priv->search_context);
      priv->search_context = search_context ?
            g_object_ref(search_context) : NULL;
      g_object_notify_by_pspec(G_OBJECT(overlay),
                               gParamSpecs[PROP_SEARCH_CONTEXT]);
   }
}

static void
gb_source_overlay_get_background (GbSourceOverlay *overlay,
                                  GdkRGBA         *background)
{
   GbSourceOverlayPrivate *priv = overlay->priv;
   GtkSourceStyleScheme *scheme;
   GtkSourceStyle *style;
   GtkTextBuffer *buffer;
   char *bg_color;

   /*
    * TODO: Cache this information.
    */

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->source_view));
   if (GTK_SOURCE_IS_BUFFER(buffer)) {
      scheme = gtk_source_buffer_get_style_scheme(GTK_SOURCE_BUFFER(buffer));
      style = gtk_source_style_scheme_get_style(scheme, "search-match");
      if (GTK_SOURCE_IS_STYLE(style)) {
         g_object_get(style,
                      "background", &bg_color,
                      NULL);
         if (bg_color) {
            gdk_rgba_parse(background, bg_color);
         }
         g_free(bg_color);
         return;
      }
   }

   g_assert_not_reached();
   gdk_rgba_parse(background, "#000000");
}

static cairo_region_t *
_gdk_cairo_region_create_from_clip_extents (cairo_t *cr)
{
   cairo_rectangle_int_t crect;
   GdkRectangle rect;

   gdk_cairo_get_clip_rectangle(cr, &rect);
   crect.x = rect.x;
   crect.y = rect.y;
   crect.width = rect.width;
   crect.height = rect.height;

   return cairo_region_create_rectangle(&crect);
}

static void
gb_source_overlay_foreach_visible_match (GbSourceOverlay          *overlay,
                                         GbSourceOverlayMatchFunc  match_func,
                                         gpointer                  match_data)
{
   GbSourceOverlayPrivate *priv;
   GdkRectangle rect;
   GtkTextIter iter;
   GtkTextIter match_begin;
   GtkTextIter match_end;
   GtkTextIter vis_begin;
   GtkTextIter vis_end;
   gint64 first = -1;

   g_assert(GB_IS_SOURCE_OVERLAY(overlay));

   priv = overlay->priv;

   if (!priv->search_context || !priv->source_view) {
      return;
   }

   gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(priv->source_view), &rect);
   gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(priv->source_view),
                                      &vis_begin,
                                      rect.x,
                                      rect.y);
   gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(priv->source_view),
                                      &vis_end,
                                      rect.x + rect.width,
                                      rect.y + rect.height);

   gtk_text_iter_assign(&iter, &vis_begin);

   do {
      if (gtk_source_search_context_forward(priv->search_context,
                                            &iter,
                                            &match_begin,
                                            &match_end)) {
         if ((first != -1) &&
             (gtk_text_iter_get_offset(&match_begin) == first)) {
            break;
         }
         match_func(overlay, &match_begin, &match_end, match_data);
         gtk_text_iter_assign(&iter, &match_end);
         if (first == -1) {
            first = gtk_text_iter_get_offset(&match_begin);
         }
         continue;
      }
      break;
   } while ((gtk_text_iter_compare(&iter, &vis_begin) > 0) &&
            (gtk_text_iter_compare(&iter, &vis_end) < 0));
}

static void
gb_source_overlay_subtract_match (GbSourceOverlay   *overlay,
                                  const GtkTextIter *begin,
                                  const GtkTextIter *end,
                                  gpointer           user_data)
{
   GbSourceOverlayPrivate *priv = overlay->priv;
   cairo_rectangle_int_t r;
   cairo_region_t *region = user_data;
   GdkRectangle rect;
   GdkRectangle rect2;

   gtk_text_view_get_iter_location(GTK_TEXT_VIEW(priv->source_view),
                                   begin,
                                   &rect);
   gtk_text_view_buffer_to_window_coords(GTK_TEXT_VIEW(priv->source_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         rect.x,
                                         rect.y,
                                         &rect.x,
                                         &rect.y);

   gtk_text_view_get_iter_location(GTK_TEXT_VIEW(priv->source_view),
                                   end,
                                   &rect2);
   gtk_text_view_buffer_to_window_coords(GTK_TEXT_VIEW(priv->source_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         rect2.x,
                                         rect2.y,
                                         &rect2.x,
                                         &rect2.y);

   r.x = MIN(rect.x, rect2.x);
   r.y = MIN(rect.y, rect2.y);
   r.width = MAX(rect.x + rect.width, rect2.x) - r.x;
   r.height = MAX(rect.y + rect.height, rect2.y + rect2.height) - r.y;

   cairo_region_subtract_rectangle(region, &r);
}

static void
gb_source_overlay_subtract_matches (GbSourceOverlay *overlay,
                                    cairo_region_t  *region)
{
   gb_source_overlay_foreach_visible_match(overlay,
                                           gb_source_overlay_subtract_match,
                                           region);
}

static void
gb_source_overlay_draw_bubble_with_radius (GbSourceOverlay   *overlay,
                                           const GtkTextIter *begin,
                                           const GtkTextIter *end,
                                           cairo_t           *cr,
                                           gint               radius)
{
   cairo_rectangle_int_t r;
   GtkTextView *view;
   GdkRectangle r1;
   GdkRectangle r2;

   view = GTK_TEXT_VIEW(overlay->priv->source_view);

   gtk_text_view_get_iter_location(view, begin, &r1);
   gtk_text_view_get_iter_location(view, end, &r2);

   gtk_text_view_buffer_to_window_coords(view,
                                         GTK_TEXT_WINDOW_WIDGET,
                                         r1.x, r1.y, &r1.x, &r1.y);
   gtk_text_view_buffer_to_window_coords(view,
                                         GTK_TEXT_WINDOW_WIDGET,
                                         r2.x, r2.y, &r2.x, &r2.y);

   r.x = MIN(r1.x, r2.x);
   r.y = MIN(r1.y, r2.y);
   r.width = MAX(r1.x + r1.width, r2.x) - r.x;
   r.height = MAX(r1.y + r1.height, r2.y + r2.height) - r.y;

   r.x -= radius;
   r.y -= radius;
   r.width += radius * 2;
   r.height += radius * 2;

   rounded_rectangle(cr, r.x, r.y, r.width, r.height, radius, radius);
}

static void
gb_source_overlay_draw_bubble_outer (GbSourceOverlay   *overlay,
                                     const GtkTextIter *begin,
                                     const GtkTextIter *end,
                                     gpointer           user_data)
{
   cairo_t *cr = user_data;

   gb_source_overlay_draw_bubble_with_radius(overlay, begin, end, cr, 3);
}

static void
gb_source_overlay_draw_bubble_inner (GbSourceOverlay   *overlay,
                                     const GtkTextIter *begin,
                                     const GtkTextIter *end,
                                     gpointer           user_data)
{
   cairo_t *cr = user_data;

   gb_source_overlay_draw_bubble_with_radius(overlay, begin, end, cr, 2);
}

static void
gb_source_overlay_do_draw (GbSourceOverlay *overlay,
                           cairo_t         *cr)
{
   GbSourceOverlayPrivate *priv;
   cairo_region_t *region;
   GdkRGBA fg1;
   GdkRGBA shade;

   g_assert(GB_IS_SOURCE_OVERLAY(overlay));

   priv = overlay->priv;

   /*
    * Draw the translucent background.
    */
   gdk_cairo_set_source_rgba(cr, &priv->bg);
   region = _gdk_cairo_region_create_from_clip_extents(cr);
   gb_source_overlay_subtract_matches(overlay, region);
   gdk_cairo_region(cr, region);
   cairo_region_destroy(region);
   cairo_fill_preserve(cr);
   cairo_clip(cr);

   /*
    * Draw darker outer bubble line.
    */
   gb_source_overlay_foreach_visible_match(
         overlay,
         gb_source_overlay_draw_bubble_outer,
         cr);
   gb_source_overlay_get_background(overlay, &fg1);
   _gb_rgba_shade(&fg1, 0.7, &shade);
   gdk_cairo_set_source_rgba(cr, &shade);
   cairo_fill(cr);

   /*
    * Draw lighter inner bubble line.
    */
   gb_source_overlay_foreach_visible_match(
         overlay,
         gb_source_overlay_draw_bubble_inner,
         cr);
   gb_source_overlay_get_background(overlay, &fg1);
   _gb_rgba_shade(&fg1, 1.05, &shade);
   gdk_cairo_set_source_rgba(cr, &shade);
   cairo_fill(cr);
}

static gboolean
gb_source_overlay_draw (GtkWidget *widget,
                        cairo_t   *cr)
{
   GbSourceOverlay *overlay = (GbSourceOverlay *)widget;
   GdkWindow *window;

   g_assert(GB_IS_SOURCE_OVERLAY(overlay));
   g_assert(cr);

   if ((window = gtk_widget_get_window(widget)) &&
       gtk_cairo_should_draw_window(cr, window)) {
      cairo_save(cr);
      gb_source_overlay_do_draw(overlay, cr);
      cairo_restore(cr);
   }

   return FALSE;
}

static gboolean
gb_source_overlay_event (GtkWidget *widget,
                         GdkEvent  *event)
{
   GbSourceOverlay *overlay = (GbSourceOverlay *)widget;
   gboolean ret = FALSE;

   switch (event->type) {
   case GDK_BUTTON_PRESS:
   case GDK_2BUTTON_PRESS:
   case GDK_3BUTTON_PRESS:
   case GDK_BUTTON_RELEASE:
   case GDK_TOUCH_BEGIN:
   case GDK_TOUCH_UPDATE:
   case GDK_TOUCH_END:
   case GDK_TOUCH_CANCEL:
      gtk_widget_hide(widget);
      ret = TRUE;
      break;
   case GDK_SCROLL:
      gtk_propagate_event(GTK_WIDGET(overlay->priv->source_view), event);
      ret = TRUE;
      break;
   case GDK_NOTHING:
   case GDK_DELETE:
   case GDK_DESTROY:
   case GDK_EXPOSE:
   case GDK_MOTION_NOTIFY:
   case GDK_KEY_PRESS:
   case GDK_KEY_RELEASE:
   case GDK_ENTER_NOTIFY:
   case GDK_LEAVE_NOTIFY:
   case GDK_FOCUS_CHANGE:
   case GDK_CONFIGURE:
   case GDK_MAP:
   case GDK_UNMAP:
   case GDK_PROPERTY_NOTIFY:
   case GDK_SELECTION_CLEAR:
   case GDK_SELECTION_REQUEST:
   case GDK_SELECTION_NOTIFY:
   case GDK_PROXIMITY_IN:
   case GDK_PROXIMITY_OUT:
   case GDK_DRAG_ENTER:
   case GDK_DRAG_LEAVE:
   case GDK_DRAG_MOTION:
   case GDK_DRAG_STATUS:
   case GDK_DROP_START:
   case GDK_DROP_FINISHED:
   case GDK_CLIENT_EVENT:
   case GDK_VISIBILITY_NOTIFY:
   case GDK_WINDOW_STATE:
   case GDK_SETTING:
   case GDK_OWNER_CHANGE:
   case GDK_GRAB_BROKEN:
   case GDK_DAMAGE:
   case GDK_EVENT_LAST:
   default:
      ret = FALSE;
   }

   return ret;
}

static void
gb_source_overlay_finalize (GObject *object)
{
   GbSourceOverlayPrivate *priv;

   priv = GB_SOURCE_OVERLAY(object)->priv;

   g_clear_object(&priv->search_context);
   g_clear_object(&priv->source_view);

   G_OBJECT_CLASS(gb_source_overlay_parent_class)->finalize(object);
}

static void
gb_source_overlay_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
   GbSourceOverlay *overlay = GB_SOURCE_OVERLAY(object);

   switch (prop_id) {
   case PROP_BACKGROUND_RGBA:
      g_value_set_boxed(value, gb_source_overlay_get_background_rgba(overlay));
      break;
   case PROP_SEARCH_CONTEXT:
      g_value_set_object(value, gb_source_overlay_get_search_context(overlay));
      break;
   case PROP_SOURCE_VIEW:
      g_value_set_object(value, gb_source_overlay_get_source_view(overlay));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_overlay_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
   GbSourceOverlay *overlay = GB_SOURCE_OVERLAY(object);

   switch (prop_id) {
   case PROP_BACKGROUND_RGBA:
      gb_source_overlay_set_background_rgba(overlay, g_value_get_boxed(value));
      break;
   case PROP_SEARCH_CONTEXT:
      gb_source_overlay_set_search_context(overlay, g_value_get_object(value));
      break;
   case PROP_SOURCE_VIEW:
      gb_source_overlay_set_source_view(overlay, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_overlay_class_init (GbSourceOverlayClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_overlay_finalize;
   object_class->get_property = gb_source_overlay_get_property;
   object_class->set_property = gb_source_overlay_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceOverlayPrivate));

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->event = gb_source_overlay_event;
   widget_class->draw = gb_source_overlay_draw;

   gParamSpecs[PROP_BACKGROUND_RGBA] =
      g_param_spec_boxed("background-rgba",
                          _("Background RGBA"),
                          _("The background to render."),
                          GDK_TYPE_RGBA,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_BACKGROUND_RGBA,
                                   gParamSpecs[PROP_BACKGROUND_RGBA]);

   gParamSpecs[PROP_SEARCH_CONTEXT] =
      g_param_spec_object("search-context",
                          _("Search Context"),
                          _("The search context to use."),
                          GTK_SOURCE_TYPE_SEARCH_CONTEXT,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SEARCH_CONTEXT,
                                   gParamSpecs[PROP_SEARCH_CONTEXT]);

   gParamSpecs[PROP_SOURCE_VIEW] =
      g_param_spec_object("source-view",
                          _("Source View"),
                          _("The source view to overlay."),
                          GTK_SOURCE_TYPE_VIEW,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_SOURCE_VIEW,
                                   gParamSpecs[PROP_SOURCE_VIEW]);
}

static void
gb_source_overlay_init (GbSourceOverlay *overlay)
{
   overlay->priv = G_TYPE_INSTANCE_GET_PRIVATE(overlay,
                                               GB_TYPE_SOURCE_OVERLAY,
                                               GbSourceOverlayPrivate);

   gdk_rgba_parse(&overlay->priv->bg, DEFAULT_BG_HEX);

   gtk_widget_add_events(GTK_WIDGET(overlay),
                         (GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK |
                          GDK_TOUCH_MASK |
                          GDK_SCROLL_MASK |
                          GDK_SMOOTH_SCROLL_MASK));
}

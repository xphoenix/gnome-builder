/* gb-source-view.c
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

#include <glib/gi18n.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <gtksourceview/gtksourcelanguage.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include <gtksourceview/gtksourcestylescheme.h>
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "gb-source-state.h"
#include "gb-source-view.h"

G_DEFINE_TYPE(GbSourceView, gb_source_view, GTK_SOURCE_TYPE_VIEW)

struct _GbSourceViewPrivate
{
   GbSourceState *state;
};

enum
{
   PROP_0,
   PROP_STATE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceState *
gb_source_view_get_state (GbSourceView *view)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), NULL);
   return view->priv->state;
}

void
gb_source_view_set_state (GbSourceView  *view,
                          GbSourceState *state)
{
   g_return_if_fail(GB_IS_SOURCE_VIEW(view));
   g_return_if_fail(!state || GB_IS_SOURCE_STATE(state));

   g_clear_object(&view->priv->state);
   view->priv->state = state ? g_object_ref(state) : NULL;
   g_object_notify_by_pspec(G_OBJECT(view), gParamSpecs[PROP_STATE]);
}

static void
gb_source_view_finalize (GObject *object)
{
   GbSourceViewPrivate *priv;

   priv = GB_SOURCE_VIEW(object)->priv;

   g_clear_object(&priv->state);

   G_OBJECT_CLASS(gb_source_view_parent_class)->finalize(object);
}

static void
gb_source_view_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   GbSourceView *view = GB_SOURCE_VIEW(object);

   switch (prop_id) {
   case PROP_STATE:
      g_value_set_object(value, gb_source_view_get_state(view));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   GbSourceView *view = GB_SOURCE_VIEW(object);

   switch (prop_id) {
   case PROP_STATE:
      gb_source_view_set_state(view, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_class_init (GbSourceViewClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_view_finalize;
   object_class->get_property = gb_source_view_get_property;
   object_class->set_property = gb_source_view_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceViewPrivate));

   gParamSpecs[PROP_STATE] =
      g_param_spec_object("state",
                          _("State"),
                          _("The state of the source view."),
                          GB_TYPE_SOURCE_STATE,
                          (G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS |
                           G_PARAM_CONSTRUCT));
   g_object_class_install_property(object_class, PROP_STATE,
                                   gParamSpecs[PROP_STATE]);
}

static void
gb_source_view_init (GbSourceView *view)
{
   GtkSourceStyleSchemeManager *sm;
   GtkSourceLanguageManager *lm;
   PangoFontDescription *font;
   GtkSourceStyleScheme *scheme;
   GtkSourceLanguage *lang;
   GtkTextBuffer *buffer;

   view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,
                                            GB_TYPE_SOURCE_VIEW,
                                            GbSourceViewPrivate);

   font = pango_font_description_from_string("Monospace 10");
   gtk_widget_override_font(GTK_WIDGET(view), font);
   pango_font_description_free(font);

   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
   if (GTK_SOURCE_IS_BUFFER(buffer)) {
      sm = gtk_source_style_scheme_manager_get_default();
      gtk_source_style_scheme_manager_append_search_path(sm, ".");
      scheme = gtk_source_style_scheme_manager_get_scheme(sm, "tango");

      lm = gtk_source_language_manager_get_default();
      lang = gtk_source_language_manager_get_language(lm, "c");

      g_object_set(buffer,
                   "language", lang,
                   "style-scheme", scheme,
                   NULL);
   }

   g_object_set(view,
                "show-line-numbers", TRUE,
                "show-right-margin", TRUE,
                NULL);
}

/* gb-source-view.c
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
#include <gtksourceview/gtksourcestyleschememanager.h>

#include "gb-source-view.h"
#include "gb-source-view-state.h"
#include "gb-source-view-state-insert.h"

G_DEFINE_TYPE(GbSourceView, gb_source_view, GTK_SOURCE_TYPE_VIEW)

struct _GbSourceViewPrivate
{
   GbSourceViewState *state;
};

enum
{
   PROP_0,
   PROP_STATE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceViewState *
gb_source_view_get_state (GbSourceView *view)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW(view), NULL);
   return view->priv->state;
}

void
gb_source_view_set_state (GbSourceView      *view,
                          GbSourceViewState *state)
{
   GbSourceViewPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_VIEW(view));
   g_return_if_fail(!state || GB_IS_SOURCE_VIEW_STATE(state));

   priv = view->priv;

#if 0
   g_print("Changing state from \"%s\" to \"%s\".\n",
           priv->state ? g_type_name(G_TYPE_FROM_INSTANCE(priv->state)) : "",
           state ? g_type_name(G_TYPE_FROM_INSTANCE(state)) : "");
#endif

   if (priv->state) {
      gb_source_view_state_unload(priv->state, view);
      g_clear_object(&priv->state);
   }

   if (state) {
      priv->state = g_object_ref(state);
      gb_source_view_state_load(priv->state, view);
   }

   g_object_notify_by_pspec(G_OBJECT(view), gParamSpecs[PROP_STATE]);
}

static void
gb_source_view_dispose (GObject *object)
{
   GbSourceView *view = GB_SOURCE_VIEW(object);

   gb_source_view_set_state(view, NULL);

   G_OBJECT_CLASS(gb_source_view_parent_class)->dispose(object);
}

static void
gb_source_view_finalize (GObject *object)
{
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
   object_class->dispose = gb_source_view_dispose;
   object_class->finalize = gb_source_view_finalize;
   object_class->get_property = gb_source_view_get_property;
   object_class->set_property = gb_source_view_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceViewPrivate));

   gParamSpecs[PROP_STATE] =
      g_param_spec_object("state",
                          _("State"),
                          _("The current state machine state."),
                          GB_TYPE_SOURCE_VIEW_STATE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STATE,
                                   gParamSpecs[PROP_STATE]);

   gtk_source_style_scheme_manager_append_search_path(
         gtk_source_style_scheme_manager_get_default(),
         "data/style-schemes");
}

static void
gb_source_view_init (GbSourceView *view)
{
   GbSourceViewState *state;

   view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,
                                            GB_TYPE_SOURCE_VIEW,
                                            GbSourceViewPrivate);

   state = gb_source_view_state_insert_new();
   gb_source_view_set_state(view, state);
   g_object_unref(state);
}

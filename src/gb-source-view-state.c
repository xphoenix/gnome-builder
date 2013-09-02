/* gb-source-view-state.c
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

#include "gb-source-view-state.h"

G_DEFINE_ABSTRACT_TYPE(GbSourceViewState, gb_source_view_state, G_TYPE_OBJECT)

struct _GbSourceViewStatePrivate
{
   GbSourceView *view;
};

enum
{
   PROP_0,
   PROP_VIEW,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbSourceView *
gb_source_view_state_get_view (GbSourceViewState *state)
{
   g_return_val_if_fail(GB_IS_SOURCE_VIEW_STATE(state), NULL);
   return state->priv->view;
}

static void
gb_source_view_state_set_view (GbSourceViewState *state,
                               GbSourceView      *view)
{
   GbSourceViewStatePrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_VIEW_STATE(state));

   priv = state->priv;

   if (priv->view) {
      g_object_remove_weak_pointer(G_OBJECT(priv->view),
                                   (gpointer *)&priv->view);
      priv->view = NULL;
   }

   if (view) {
      priv->view = view;
      g_object_add_weak_pointer(G_OBJECT(view), (gpointer *)&priv->view);
   }
}

void
gb_source_view_state_load (GbSourceViewState *state,
                           GbSourceView      *view)
{
   GbSourceViewStateClass *klass;

   g_return_if_fail(GB_IS_SOURCE_VIEW_STATE(state));
   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   gb_source_view_state_set_view(state, view);

   klass = GB_SOURCE_VIEW_STATE_GET_CLASS(state);
   if (klass->load) {
      klass->load(state, view);
   }
}

void
gb_source_view_state_unload (GbSourceViewState *state,
                             GbSourceView      *view)
{
   GbSourceViewStateClass *klass;

   g_return_if_fail(GB_IS_SOURCE_VIEW_STATE(state));
   g_return_if_fail(GB_IS_SOURCE_VIEW(view));

   klass = GB_SOURCE_VIEW_STATE_GET_CLASS(state);
   if (klass->unload) {
      klass->unload(state, view);
   }

   gb_source_view_state_set_view(state, NULL);
}

static void
gb_source_view_state_finalize (GObject *object)
{
   GbSourceViewStatePrivate *priv;

   priv = GB_SOURCE_VIEW_STATE(object)->priv;

   if (priv->view) {
      g_object_remove_weak_pointer(G_OBJECT(priv->view),
                                   (gpointer *)&priv->view);
      priv->view = NULL;
   }

   G_OBJECT_CLASS(gb_source_view_state_parent_class)->finalize(object);
}

static void
gb_source_view_state_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
   GbSourceViewState *state = GB_SOURCE_VIEW_STATE(object);

   switch (prop_id) {
   case PROP_VIEW:
      g_value_set_object(value, gb_source_view_state_get_view(state));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_state_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
   //GbSourceViewState *state = GB_SOURCE_VIEW_STATE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_view_state_class_init (GbSourceViewStateClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_view_state_finalize;
   object_class->get_property = gb_source_view_state_get_property;
   object_class->set_property = gb_source_view_state_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceViewStatePrivate));

   gParamSpecs[PROP_VIEW] =
      g_param_spec_object("view",
                          _("View"),
                          _("The underlying view widget."),
                          GB_TYPE_SOURCE_VIEW,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_VIEW,
                                   gParamSpecs[PROP_VIEW]);
}

static void
gb_source_view_state_init (GbSourceViewState *state)
{
   state->priv = G_TYPE_INSTANCE_GET_PRIVATE(state,
                                             GB_TYPE_SOURCE_VIEW_STATE,
                                             GbSourceViewStatePrivate);
}

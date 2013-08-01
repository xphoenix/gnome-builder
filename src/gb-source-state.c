/* gb-source-state.c:
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

#include "gb-source-state.h"
#include "gb-source-view.h"

G_DEFINE_TYPE(GbSourceState, gb_source_state, G_TYPE_INITIALLY_UNOWNED)

struct _GbSourceStatePrivate
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
gb_source_state_get_view (GbSourceState *state)
{
   g_return_val_if_fail(GB_IS_SOURCE_STATE(state), NULL);
   return state->priv->view;
}

void
gb_source_state_set_view (GbSourceState *state,
                          GbSourceView  *view)
{
   GbSourceStatePrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_STATE(state));

   priv = state->priv;

   if (view == priv->view) {
      return;
   }

   if (priv->view) {
      g_object_remove_weak_pointer(G_OBJECT(state), (gpointer *)&priv->view);
      priv->view = NULL;
   }

   if (view) {
      priv->view = g_object_ref(view);
      g_object_add_weak_pointer(G_OBJECT(state), (gpointer *)&priv->view);
   }

   g_object_notify_by_pspec(G_OBJECT(state), gParamSpecs[PROP_VIEW]);
}

static void
gb_source_state_finalize (GObject *object)
{
   GbSourceStatePrivate *priv;

   priv = GB_SOURCE_STATE(object)->priv;

   if (priv->view) {
      g_object_remove_weak_pointer(object, (gpointer *)&priv->view);
      priv->view = NULL;
   }

   G_OBJECT_CLASS(gb_source_state_parent_class)->finalize(object);
}

static void
gb_source_state_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
   GbSourceState *state = GB_SOURCE_STATE(object);

   switch (prop_id) {
   case PROP_VIEW:
      g_value_set_object(value, gb_source_state_get_view(state));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_state_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
   GbSourceState *state = GB_SOURCE_STATE(object);

   switch (prop_id) {
   case PROP_VIEW:
      gb_source_state_set_view(state, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_state_class_init (GbSourceStateClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_state_finalize;
   object_class->get_property = gb_source_state_get_property;
   object_class->set_property = gb_source_state_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceStatePrivate));

   gParamSpecs[PROP_VIEW] =
      g_param_spec_object("view",
                          _("View"),
                          _("The view the state is operating upon."),
                          GB_TYPE_SOURCE_VIEW,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_VIEW,
                                   gParamSpecs[PROP_VIEW]);
}

static void
gb_source_state_init (GbSourceState *state)
{
   state->priv = G_TYPE_INSTANCE_GET_PRIVATE(state,
                                             GB_TYPE_SOURCE_STATE,
                                             GbSourceStatePrivate);
}

/* gb-source-view-state.h
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

#ifndef GB_SOURCE_VIEW_STATE_H
#define GB_SOURCE_VIEW_STATE_H

#include <glib-object.h>

#include "gb-source-view.h"

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_VIEW_STATE            (gb_source_view_state_get_type())
#define GB_SOURCE_VIEW_STATE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_VIEW_STATE, GbSourceViewState))
#define GB_SOURCE_VIEW_STATE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_VIEW_STATE, GbSourceViewState const))
#define GB_SOURCE_VIEW_STATE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_VIEW_STATE, GbSourceViewStateClass))
#define GB_IS_SOURCE_VIEW_STATE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_VIEW_STATE))
#define GB_IS_SOURCE_VIEW_STATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_VIEW_STATE))
#define GB_SOURCE_VIEW_STATE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_VIEW_STATE, GbSourceViewStateClass))

typedef struct _GbSourceViewState        GbSourceViewState;
typedef struct _GbSourceViewStateClass   GbSourceViewStateClass;
typedef struct _GbSourceViewStatePrivate GbSourceViewStatePrivate;

struct _GbSourceViewState
{
   GObject parent;

   /*< private >*/
   GbSourceViewStatePrivate *priv;
};

struct _GbSourceViewStateClass
{
   GObjectClass parent_class;

   void (*load)   (GbSourceViewState *state,
                   GbSourceView      *view);
   void (*unload) (GbSourceViewState *state,
                   GbSourceView      *view);

   gpointer reserved[8];
};

GType         gb_source_view_state_get_type (void) G_GNUC_CONST;
GbSourceView *gb_source_view_state_get_view (GbSourceViewState *state);
void          gb_source_view_state_load     (GbSourceViewState *state,
                                             GbSourceView      *view);
void          gb_source_view_state_unload   (GbSourceViewState *state,
                                             GbSourceView      *view);

G_END_DECLS

#endif /* GB_SOURCE_VIEW_STATE_H */

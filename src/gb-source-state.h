/* gb-source-state.h
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

#ifndef GB_SOURCE_STATE_H
#define GB_SOURCE_STATE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_STATE            (gb_source_state_get_type())
#define GB_SOURCE_STATE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_STATE, GbSourceState))
#define GB_SOURCE_STATE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_STATE, GbSourceState const))
#define GB_SOURCE_STATE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_STATE, GbSourceStateClass))
#define GB_IS_SOURCE_STATE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_STATE))
#define GB_IS_SOURCE_STATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_STATE))
#define GB_SOURCE_STATE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_STATE, GbSourceStateClass))

typedef struct _GbSourceState        GbSourceState;
typedef struct _GbSourceStateClass   GbSourceStateClass;
typedef struct _GbSourceStatePrivate GbSourceStatePrivate;

struct _GbSourceState
{
   GInitiallyUnowned parent;

   /*< private >*/
   GbSourceStatePrivate *priv;
};

struct _GbSourceStateClass
{
   GInitiallyUnownedClass parent_class;
};

GType gb_source_state_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SOURCE_STATE_H */

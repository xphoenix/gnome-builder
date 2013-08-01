/* gb-source-state-insert.h
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

#ifndef GB_SOURCE_STATE_INSERT_H
#define GB_SOURCE_STATE_INSERT_H

#include "gb-source-state.h"

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_STATE_INSERT            (gb_source_state_insert_get_type())
#define GB_SOURCE_STATE_INSERT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_STATE_INSERT, GbSourceStateInsert))
#define GB_SOURCE_STATE_INSERT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_STATE_INSERT, GbSourceStateInsert const))
#define GB_SOURCE_STATE_INSERT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_STATE_INSERT, GbSourceStateInsertClass))
#define GB_IS_SOURCE_STATE_INSERT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_STATE_INSERT))
#define GB_IS_SOURCE_STATE_INSERT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_STATE_INSERT))
#define GB_SOURCE_STATE_INSERT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_STATE_INSERT, GbSourceStateInsertClass))

typedef struct _GbSourceStateInsert        GbSourceStateInsert;
typedef struct _GbSourceStateInsertClass   GbSourceStateInsertClass;
typedef struct _GbSourceStateInsertPrivate GbSourceStateInsertPrivate;

struct _GbSourceStateInsert
{
   GbSourceState parent;

   /*< private >*/
   GbSourceStateInsertPrivate *priv;
};

struct _GbSourceStateInsertClass
{
   GbSourceStateClass parent_class;
};

GType gb_source_state_insert_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SOURCE_STATE_INSERT_H */

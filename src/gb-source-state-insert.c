/* gb-source-state-insert.c
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

#include "gb-source-state-insert.h"

G_DEFINE_TYPE(GbSourceStateInsert,
              gb_source_state_insert,
              GB_TYPE_SOURCE_STATE)

struct _GbSourceStateInsertPrivate
{
   gpointer dummy;
};

static void
gb_source_state_insert_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_source_state_insert_parent_class)->finalize(object);
}

static void
gb_source_state_insert_class_init (GbSourceStateInsertClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_source_state_insert_finalize;
   g_type_class_add_private(object_class, sizeof(GbSourceStateInsertPrivate));
}

static void
gb_source_state_insert_init (GbSourceStateInsert *insert)
{
   insert->priv = G_TYPE_INSTANCE_GET_PRIVATE(insert,
                                              GB_TYPE_SOURCE_STATE_INSERT,
                                              GbSourceStateInsertPrivate);
}

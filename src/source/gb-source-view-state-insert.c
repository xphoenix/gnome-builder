/* gb-source-view-state-insert.c
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

#include "gb-source-view-state-insert.h"
#include "gb-source-view-state-snippet.h"

G_DEFINE_TYPE(GbSourceViewStateInsert,
              gb_source_view_state_insert,
              GB_TYPE_SOURCE_VIEW_STATE)

GbSourceViewState *
gb_source_view_state_insert_new (void)
{
   return g_object_new(GB_TYPE_SOURCE_VIEW_STATE_INSERT, NULL);
}

static void
gb_source_view_state_insert_load (GbSourceViewState *state,
                                  GbSourceView      *view)
{
}

static void
gb_source_view_state_insert_unload (GbSourceViewState *state,
                                    GbSourceView      *view)
{
}

static void
gb_source_view_state_insert_class_init (GbSourceViewStateInsertClass *klass)
{
   GbSourceViewStateClass *state_class;

   state_class = GB_SOURCE_VIEW_STATE_CLASS(klass);
   state_class->load = gb_source_view_state_insert_load;
   state_class->unload = gb_source_view_state_insert_unload;
}

static void
gb_source_view_state_insert_init (GbSourceViewStateInsert *insert)
{
}

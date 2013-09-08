/* gb-workspace-layout.c
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

#include "gb-workspace-layout.h"

G_DEFINE_ABSTRACT_TYPE(GbWorkspaceLayout, gb_workspace_layout, GTK_TYPE_GRID)

struct _GbWorkspaceLayoutPrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

void
gb_workspace_layout_fullscreen (GbWorkspaceLayout *layout)
{
   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT(layout));

   if (GB_WORKSPACE_LAYOUT_GET_CLASS(layout)->fullscreen) {
      GB_WORKSPACE_LAYOUT_GET_CLASS(layout)->fullscreen(layout);
   }
}

void
gb_workspace_layout_unfullscreen (GbWorkspaceLayout *layout)
{
   g_return_if_fail(GB_IS_WORKSPACE_LAYOUT(layout));

   if (GB_WORKSPACE_LAYOUT_GET_CLASS(layout)->unfullscreen) {
      GB_WORKSPACE_LAYOUT_GET_CLASS(layout)->unfullscreen(layout);
   }
}

static void
gb_workspace_layout_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_workspace_layout_parent_class)->finalize(object);
}

static void
gb_workspace_layout_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
   //GbWorkspaceLayout *layout = GB_WORKSPACE_LAYOUT(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_layout_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
   //GbWorkspaceLayout *layout = GB_WORKSPACE_LAYOUT(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_layout_class_init (GbWorkspaceLayoutClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_layout_finalize;
   object_class->get_property = gb_workspace_layout_get_property;
   object_class->set_property = gb_workspace_layout_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceLayoutPrivate));
}

static void
gb_workspace_layout_init (GbWorkspaceLayout *layout)
{
   layout->priv = G_TYPE_INSTANCE_GET_PRIVATE(layout,
                                              GB_TYPE_WORKSPACE_LAYOUT,
                                              GbWorkspaceLayoutPrivate);
}

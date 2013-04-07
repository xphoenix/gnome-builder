/* gb-terminal-pane.c
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
#include <vte/vte.h>

#include "gb-terminal-pane.h"

G_DEFINE_TYPE(GbTerminalPane, gb_terminal_pane, GB_TYPE_WORKSPACE_PANE)

struct _GbTerminalPanePrivate
{
   GtkWidget *scroller;
   GtkWidget *vte;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_terminal_pane_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_terminal_pane_parent_class)->finalize(object);
}

static void
gb_terminal_pane_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
   //GbTerminalPane *pane = GB_TERMINAL_PANE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_terminal_pane_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
   //GbTerminalPane *pane = GB_TERMINAL_PANE(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_terminal_pane_class_init (GbTerminalPaneClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_terminal_pane_finalize;
   object_class->get_property = gb_terminal_pane_get_property;
   object_class->set_property = gb_terminal_pane_set_property;
   g_type_class_add_private(object_class, sizeof(GbTerminalPanePrivate));
}

static void
size_allocate (GtkWidget     *widget,
               GtkAllocation *alloc)
{
   if (alloc->y < 0) {
      alloc->y = ABS(0);
   }

#if 0
   g_print("%u %u %u %u\n",
           alloc->x, alloc->y, alloc->width, alloc->height);
#endif
}

static void
gb_terminal_pane_init (GbTerminalPane *pane)
{
   static const gchar *argv[] = { "/bin/bash", NULL };

   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_TERMINAL_PANE,
                                            GbTerminalPanePrivate);

   pane->priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                       "hexpand", TRUE,
                                       "vexpand", TRUE,
                                       "visible", TRUE,
                                       NULL);
   gtk_container_add(GTK_CONTAINER(pane), pane->priv->scroller);

   pane->priv->vte = g_object_new(VTE_TYPE_TERMINAL,
                                  "visible", TRUE,
                                  NULL);
   g_signal_connect(pane->priv->vte, "size-allocate", G_CALLBACK(size_allocate), NULL);
   gtk_container_add(GTK_CONTAINER(pane->priv->scroller), pane->priv->vte);

   vte_terminal_fork_command_full(VTE_TERMINAL(pane->priv->vte),
                                  VTE_PTY_DEFAULT,
                                  ".",
                                  (gchar **)argv,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);
}

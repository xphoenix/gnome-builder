/* gb-terminal-pane.h
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

#ifndef GB_TERMINAL_PANE_H
#define GB_TERMINAL_PANE_H

#include <vte/vte.h>

#include "gb-workspace-pane.h"

G_BEGIN_DECLS

#define GB_TYPE_TERMINAL_PANE            (gb_terminal_pane_get_type())
#define GB_TERMINAL_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TERMINAL_PANE, GbTerminalPane))
#define GB_TERMINAL_PANE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TERMINAL_PANE, GbTerminalPane const))
#define GB_TERMINAL_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_TERMINAL_PANE, GbTerminalPaneClass))
#define GB_IS_TERMINAL_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_TERMINAL_PANE))
#define GB_IS_TERMINAL_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_TERMINAL_PANE))
#define GB_TERMINAL_PANE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_TERMINAL_PANE, GbTerminalPaneClass))

typedef struct _GbTerminalPane        GbTerminalPane;
typedef struct _GbTerminalPaneClass   GbTerminalPaneClass;
typedef struct _GbTerminalPanePrivate GbTerminalPanePrivate;

struct _GbTerminalPane
{
   GbWorkspacePane parent;

   /*< private >*/
   GbTerminalPanePrivate *priv;
};

struct _GbTerminalPaneClass
{
   GbWorkspacePaneClass parent_class;
};

GType gb_terminal_pane_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_TERMINAL_PANE_H */

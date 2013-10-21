/* gb-command.h
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

#ifndef GB_COMMAND_H
#define GB_COMMAND_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_COMMAND            (gb_command_get_type())
#define GB_COMMAND(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMMAND, GbCommand))
#define GB_COMMAND_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMMAND, GbCommand const))
#define GB_COMMAND_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_COMMAND, GbCommandClass))
#define GB_IS_COMMAND(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_COMMAND))
#define GB_IS_COMMAND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_COMMAND))
#define GB_COMMAND_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_COMMAND, GbCommandClass))

typedef struct _GbCommand        GbCommand;
typedef struct _GbCommandClass   GbCommandClass;
typedef struct _GbCommandPrivate GbCommandPrivate;

struct _GbCommand
{
   GObject parent;

   /*< private >*/
   GbCommandPrivate *priv;
};

struct _GbCommandClass
{
   GObjectClass parent_class;

   void (*run)  (GbCommand *command);
   void (*undo) (GbCommand *command);
};

gboolean     gb_command_run              (GbCommand    *command,
                                          GError      **error);
gboolean     gb_command_get_can_redo     (GbCommand    *command);
gboolean     gb_command_get_can_undo     (GbCommand    *command);
const gchar *gb_command_get_text         (GbCommand    *command);
const gchar *gb_command_get_tooltip_text (GbCommand    *command);
GType        gb_command_get_type         (void) G_GNUC_CONST;
GbCommand   *gb_command_new              (void);
void         gb_command_set_can_redo     (GbCommand    *command,
                                          gboolean      can_redo);
void         gb_command_set_can_undo     (GbCommand    *command,
                                          gboolean      can_undo);
void         gb_command_set_error        (GbCommand    *command,
                                          const GError *error);
void         gb_command_set_text         (GbCommand    *command,
                                          const gchar  *text);
void         gb_command_set_tooltip_text (GbCommand    *command,
                                          const gchar  *tooltip_text);
void         gb_command_take_error       (GbCommand    *command,
                                          GError       *error);
gboolean     gb_command_undo             (GbCommand    *command,
                                          GError      **error);

G_END_DECLS

#endif /* GB_COMMAND_H */

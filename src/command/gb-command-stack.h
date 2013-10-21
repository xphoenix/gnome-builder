/* gb-command-stack.h
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

#ifndef GB_COMMAND_STACK_H
#define GB_COMMAND_STACK_H

#include "gb-command.h"

G_BEGIN_DECLS

#define GB_TYPE_COMMAND_STACK            (gb_command_stack_get_type())
#define GB_COMMAND_STACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMMAND_STACK, GbCommandStack))
#define GB_COMMAND_STACK_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_COMMAND_STACK, GbCommandStack const))
#define GB_COMMAND_STACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_COMMAND_STACK, GbCommandStackClass))
#define GB_IS_COMMAND_STACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_COMMAND_STACK))
#define GB_IS_COMMAND_STACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_COMMAND_STACK))
#define GB_COMMAND_STACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_COMMAND_STACK, GbCommandStackClass))

typedef struct _GbCommandStack        GbCommandStack;
typedef struct _GbCommandStackClass   GbCommandStackClass;
typedef struct _GbCommandStackPrivate GbCommandStackPrivate;

struct _GbCommandStack
{
   GObject parent;

   /*< private >*/
   GbCommandStackPrivate *priv;
};

struct _GbCommandStackClass
{
   GObjectClass parent_class;
};

gboolean        gb_command_stack_get_can_redo (GbCommandStack *stack);
gboolean        gb_command_stack_get_can_undo (GbCommandStack *stack);
GType           gb_command_stack_get_type     (void) G_GNUC_CONST;
GbCommandStack *gb_command_stack_new          (void);
gboolean        gb_command_stack_push         (GbCommandStack  *stack,
                                               GbCommand       *command,
                                               GError         **error);
gboolean        gb_command_stack_redo         (GbCommandStack  *stack,
                                               GError         **error);
gboolean        gb_command_stack_undo         (GbCommandStack  *stack,
                                               GError         **error);

G_END_DECLS

#endif /* GB_COMMAND_STACK_H */

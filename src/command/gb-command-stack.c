/* gb-command-stack.c
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

#include "gb-command-stack.h"

G_DEFINE_TYPE(GbCommandStack, gb_command_stack, G_TYPE_OBJECT)

struct _GbCommandStackPrivate
{
   GQueue *undo;
   GQueue *redo;
};

enum
{
   PROP_0,
   PROP_CAN_REDO,
   PROP_CAN_UNDO,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbCommandStack *
gb_command_stack_new (void)
{
   return g_object_new(GB_TYPE_COMMAND_STACK, NULL);
}

gboolean
gb_command_stack_get_can_redo (GbCommandStack *stack)
{
   g_return_val_if_fail(GB_IS_COMMAND_STACK(stack), FALSE);
   return !g_queue_is_empty(stack->priv->redo);
}

gboolean
gb_command_stack_get_can_undo (GbCommandStack *stack)
{
   g_return_val_if_fail(GB_IS_COMMAND_STACK(stack), FALSE);
   return !g_queue_is_empty(stack->priv->undo);
}

gboolean
gb_command_stack_push (GbCommandStack  *stack,
                       GbCommand       *command,
                       GError         **error)
{
   GbCommandStackPrivate *priv;
   GbCommand *old;
   gboolean ret;

   g_return_if_fail(GB_IS_COMMAND_STACK(stack));

   priv = stack->priv;

   while ((old = g_queue_pop_head(priv->redo))) {
      g_object_unref(old);
   }

   if ((ret = gb_command_run(command, error)) &&
       gb_command_get_can_undo(command)) {
      g_queue_push_head(priv->undo, g_object_ref(command));
   }

   g_object_notify_by_pspec(G_OBJECT(stack), gParamSpecs[PROP_CAN_UNDO]);
   g_object_notify_by_pspec(G_OBJECT(stack), gParamSpecs[PROP_CAN_REDO]);

   return ret;
}

gboolean
gb_command_stack_redo (GbCommandStack  *stack,
                       GError         **error)
{
   GbCommandStackPrivate *priv;
   GbCommand *command;
   gboolean ret = TRUE;

   g_return_val_if_fail(GB_IS_COMMAND_STACK(stack), FALSE);

   priv = stack->priv;

   if ((command = g_queue_pop_head(priv->redo))) {
      if ((ret = gb_command_run(command, error))) {
         if (gb_command_get_can_undo(command)) {
            g_queue_push_head(priv->undo, command);
         } else {
            g_object_unref(command);
         }
      }

      g_object_notify_by_pspec(G_OBJECT(stack), gParamSpecs[PROP_CAN_UNDO]);
      g_object_notify_by_pspec(G_OBJECT(stack), gParamSpecs[PROP_CAN_REDO]);

      return ret;
   }

   g_warning("No command to redo was found.");

   return TRUE;
}

gboolean
gb_command_stack_undo (GbCommandStack  *stack,
                       GError         **error)
{
   GbCommandStackPrivate *priv;
   GbCommand *command;
   gboolean ret = TRUE;

   g_return_val_if_fail(GB_IS_COMMAND_STACK(stack), FALSE);

   priv = stack->priv;

   if ((command = g_queue_pop_head(priv->undo))) {
      if ((ret = gb_command_undo(command, error))) {
         if (gb_command_get_can_redo(command)) {
            g_queue_push_head(priv->redo, command);
         } else {
            g_object_unref(command);
         }
      }

      g_object_notify_by_pspec(G_OBJECT(stack), gParamSpecs[PROP_CAN_UNDO]);
      g_object_notify_by_pspec(G_OBJECT(stack), gParamSpecs[PROP_CAN_REDO]);

      return ret;
   }

   g_warning("No command to undo was found.");

   return TRUE;
}

static void
gb_command_stack_finalize (GObject *object)
{
   GbCommandStackPrivate *priv;
   GbCommand *command;

   priv = GB_COMMAND_STACK(object)->priv;

   while ((command = g_queue_pop_head(priv->undo)))
      g_object_unref(command);
   g_queue_free(priv->undo);

   while ((command = g_queue_pop_head(priv->redo)))
      g_object_unref(command);
   g_queue_free(priv->redo);

   G_OBJECT_CLASS(gb_command_stack_parent_class)->finalize(object);
}

static void
gb_command_stack_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
   GbCommandStack *stack = GB_COMMAND_STACK(object);

   switch (prop_id) {
   case PROP_CAN_UNDO:
      g_value_set_boolean(value, gb_command_stack_get_can_undo(stack));
      break;
   case PROP_CAN_REDO:
      g_value_set_boolean(value, gb_command_stack_get_can_redo(stack));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_command_stack_class_init (GbCommandStackClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_command_stack_finalize;
   object_class->get_property = gb_command_stack_get_property;
   g_type_class_add_private(object_class, sizeof(GbCommandStackPrivate));

   gParamSpecs[PROP_CAN_REDO] =
      g_param_spec_boolean("can-redo",
                           _("Can Redo"),
                           _("If the command stack can be redone."),
                           FALSE,
                           (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_REDO,
                                   gParamSpecs[PROP_CAN_REDO]);

   gParamSpecs[PROP_CAN_UNDO] =
      g_param_spec_boolean("can-undo",
                           _("Can Undo"),
                           _("If the command stack can be undone."),
                           FALSE,
                           (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_UNDO,
                                   gParamSpecs[PROP_CAN_UNDO]);
}

static void
gb_command_stack_init (GbCommandStack *stack)
{
   stack->priv = G_TYPE_INSTANCE_GET_PRIVATE(stack,
                                             GB_TYPE_COMMAND_STACK,
                                             GbCommandStackPrivate);

   stack->priv->undo = g_queue_new();
   stack->priv->redo = g_queue_new();
}

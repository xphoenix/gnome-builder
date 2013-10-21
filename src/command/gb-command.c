/* gb-command.c
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
#include <gio/gio.h>

#include "gb-command.h"

/**
 * SECTION:GbCommand
 * @title: GbCommand
 * @short_description: A general command abstraction.
 *
 * #GbCommand provides a general abstraction on commands. Most things should
 * be implemented with commands so that they can go onto the undo/redo stack.
 *
 * You can connect to the ::run or ::undo signals to use the general GbCommand
 * class without subclassing it. Remember to set :text and :tooltip-text
 * properties so that the Redo/Undo labels look appropriate.
 *
 * If you would like to mark a command as failed during execution of the
 * command, use gb_command_set_error().
 */

G_DEFINE_TYPE(GbCommand, gb_command, G_TYPE_OBJECT)

struct _GbCommandPrivate
{
   gboolean  can_redo;
   gboolean  can_undo;
   gchar    *text;
   gchar    *tooltip_text;
   GError   *error;
};

enum
{
   PROP_0,
   PROP_CAN_REDO,
   PROP_CAN_UNDO,
   PROP_TEXT,
   PROP_TOOLTIP_TEXT,
   LAST_PROP
};

enum
{
   UNDO,
   RUN,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

gboolean
gb_command_get_can_redo (GbCommand *command)
{
   g_return_val_if_fail(GB_IS_COMMAND(command), FALSE);
   return command->priv->can_redo;
}

void
gb_command_set_can_redo (GbCommand *command,
                         gboolean   can_redo)
{
   g_return_if_fail(GB_IS_COMMAND(command));

   command->priv->can_redo = can_redo;
   g_object_notify_by_pspec(G_OBJECT(command), gParamSpecs[PROP_CAN_REDO]);
}

gboolean
gb_command_get_can_undo (GbCommand *command)
{
   g_return_val_if_fail(GB_IS_COMMAND(command), FALSE);
   return command->priv->can_undo;
}

void
gb_command_set_can_undo (GbCommand *command,
                         gboolean   can_undo)
{
   g_return_if_fail(GB_IS_COMMAND(command));

   command->priv->can_undo = can_undo;
   g_object_notify_by_pspec(G_OBJECT(command), gParamSpecs[PROP_CAN_UNDO]);
}

const gchar *
gb_command_get_text (GbCommand *command)
{
   g_return_val_if_fail(GB_IS_COMMAND(command), NULL);
   return command->priv->text;
}

void
gb_command_set_text (GbCommand   *command,
                     const gchar *text)
{
   g_return_if_fail(GB_IS_COMMAND(command));

   g_free(command->priv->text);
   command->priv->text = g_strdup(text);
   g_object_notify_by_pspec(G_OBJECT(command), gParamSpecs[PROP_TEXT]);
}

const gchar *
gb_command_get_tooltip_text (GbCommand *command)
{
   g_return_val_if_fail(GB_IS_COMMAND(command), NULL);
   return command->priv->tooltip_text;
}

void
gb_command_set_tooltip_text (GbCommand   *command,
                             const gchar *tooltip_text)
{
   g_return_if_fail(GB_IS_COMMAND(command));

   g_free(command->priv->tooltip_text);
   command->priv->tooltip_text = g_strdup(tooltip_text);
   g_object_notify_by_pspec(G_OBJECT(command), gParamSpecs[PROP_TOOLTIP_TEXT]);
}

gboolean
gb_command_run (GbCommand  *command,
                GError    **error)
{
   GbCommandPrivate *priv;

   g_return_val_if_fail(GB_IS_COMMAND(command), FALSE);

   priv = command->priv;

   if (priv->error) {
      g_set_error_literal(error,
                          G_IO_ERROR,
                          G_IO_ERROR_FAILED,
                          _("The command has already failed."));
      return FALSE;
   }

   g_signal_emit(command, gSignals[RUN], 0);

   if (priv->error) {
      if (error) {
         *error = g_error_copy(priv->error);
      }
      return FALSE;
   }

   return TRUE;
}

gboolean
gb_command_undo (GbCommand  *command,
                 GError    **error)
{
   GbCommandPrivate *priv;

   g_return_val_if_fail(GB_IS_COMMAND(command), FALSE);

   priv = command->priv;

   if (priv->error) {
      g_set_error_literal(error,
                          G_IO_ERROR,
                          G_IO_ERROR_FAILED,
                          _("The command has already failed."));
      return FALSE;
   }

   g_signal_emit(command, gSignals[UNDO], 0);

   if (priv->error) {
      if (error) {
         *error = g_error_copy(priv->error);
      }
      return FALSE;
   }

   return TRUE;
}

void
gb_command_take_error (GbCommand *command,
                       GError    *error)
{
   g_return_if_fail(GB_IS_COMMAND(command));

   g_clear_error(&command->priv->error);
   command->priv->error = error;
}

void
gb_command_set_error (GbCommand    *command,
                      const GError *error)
{
   g_return_if_fail(GB_IS_COMMAND(command));

   gb_command_take_error(command, g_error_copy(error));
}

static void
gb_command_finalize (GObject *object)
{
   GbCommandPrivate *priv;

   priv = GB_COMMAND(object)->priv;

   g_clear_pointer(&priv->text, g_free);
   g_clear_pointer(&priv->tooltip_text, g_free);
   g_clear_error(&priv->error);

   G_OBJECT_CLASS(gb_command_parent_class)->finalize(object);
}

static void
gb_command_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
   GbCommand *command = GB_COMMAND(object);

   switch (prop_id) {
   case PROP_CAN_REDO:
      g_value_set_boolean(value, gb_command_get_can_redo(command));
      break;
   case PROP_CAN_UNDO:
      g_value_set_boolean(value, gb_command_get_can_undo(command));
      break;
   case PROP_TEXT:
      g_value_set_string(value, gb_command_get_text(command));
      break;
   case PROP_TOOLTIP_TEXT:
      g_value_set_string(value, gb_command_get_tooltip_text(command));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_command_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
   GbCommand *command = GB_COMMAND(object);

   switch (prop_id) {
   case PROP_CAN_REDO:
      gb_command_set_can_redo(command, g_value_get_boolean(value));
      break;
   case PROP_CAN_UNDO:
      gb_command_set_can_undo(command, g_value_get_boolean(value));
      break;
   case PROP_TEXT:
      gb_command_set_text(command, g_value_get_string(value));
      break;
   case PROP_TOOLTIP_TEXT:
      gb_command_set_tooltip_text(command, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_command_class_init (GbCommandClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_command_finalize;
   object_class->get_property = gb_command_get_property;
   object_class->set_property = gb_command_set_property;
   g_type_class_add_private(object_class, sizeof(GbCommandPrivate));

   gParamSpecs[PROP_CAN_REDO] =
      g_param_spec_boolean("can-redo",
                          _("Can Redo"),
                          _("If the command can be redone."),
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_REDO,
                                   gParamSpecs[PROP_CAN_REDO]);

   gParamSpecs[PROP_CAN_UNDO] =
      g_param_spec_boolean("can-undo",
                           _("Can Undo"),
                           _("If the command can be undone."),
                           FALSE,
                           (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_UNDO,
                                   gParamSpecs[PROP_CAN_UNDO]);

   gParamSpecs[PROP_TEXT] =
      g_param_spec_string("text",
                          _("Text"),
                          _("The text for the command label."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TEXT,
                                   gParamSpecs[PROP_TEXT]);

   gParamSpecs[PROP_TOOLTIP_TEXT] =
      g_param_spec_string("tooltip-text",
                          _("Tooltip Text"),
                          _("The tooltip text for the command label."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TOOLTIP_TEXT,
                                   gParamSpecs[PROP_TOOLTIP_TEXT]);

   gSignals[RUN] = g_signal_new("run",
                                GB_TYPE_COMMAND,
                                G_SIGNAL_RUN_LAST,
                                G_STRUCT_OFFSET(GbCommandClass, run),
                                NULL,
                                NULL,
                                g_cclosure_marshal_VOID__VOID,
                                G_TYPE_NONE,
                                0);

   gSignals[UNDO] = g_signal_new("undo",
                                 GB_TYPE_COMMAND,
                                 G_SIGNAL_RUN_LAST,
                                 G_STRUCT_OFFSET(GbCommandClass, undo),
                                 NULL,
                                 NULL,
                                 g_cclosure_marshal_VOID__VOID,
                                 G_TYPE_NONE,
                                 0);
}

static void
gb_command_init (GbCommand *command)
{
   command->priv = G_TYPE_INSTANCE_GET_PRIVATE(command,
                                               GB_TYPE_COMMAND,
                                               GbCommandPrivate);
}

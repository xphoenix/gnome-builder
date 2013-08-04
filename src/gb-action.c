/* gb-action.c
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

#include "gb-action.h"

struct _GbActionPrivate
{
   GbApplication *application;
   gboolean       enabled;
   gchar         *name;
   GbProject     *project;
   GbWorkspace   *workspace;
};

enum
{
   PROP_0,
   PROP_APPLICATION,
   PROP_ENABLED,
   PROP_NAME,
   PROP_PARAMETER_TYPE,
   PROP_PROJECT,
   PROP_STATE,
   PROP_STATE_TYPE,
   PROP_WORKSPACE,
   LAST_PROP
};

enum
{
   ACTIVATE,
   LAST_SIGNAL
};

static void g_action_init (GActionInterface *iface);

G_DEFINE_TYPE_EXTENDED(GbAction,
                       gb_action,
                       G_TYPE_OBJECT,
                       0,
                       G_IMPLEMENT_INTERFACE(G_TYPE_ACTION, g_action_init));

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

GbAction *
gb_action_new (const gchar *name)
{
   return g_object_new(GB_TYPE_ACTION, "name", name, NULL);
}

/**
 * gb_action_get_application:
 * @action: (in): A #GbAction.
 *
 * Returns: (transfer none): A #GbApplication.
 */
GbApplication *
gb_action_get_application (GbAction *action)
{
   g_return_val_if_fail(GB_IS_ACTION(action), NULL);
   return action->priv->application;
}

void
gb_action_set_application (GbAction      *action,
                           GbApplication *application)
{
   g_return_if_fail(GB_IS_ACTION(action));
   g_return_if_fail(!application || GB_IS_APPLICATION(application));

   if (application != action->priv->application) {
      g_clear_object(&action->priv->application);
      action->priv->application =
         application ? g_object_ref(application) : NULL;
      g_object_notify_by_pspec(G_OBJECT(action),
                               gParamSpecs[PROP_APPLICATION]);
   }
}

/**
 * gb_action_get_project:
 * @action: (in): A #GbAction.
 *
 * Returns: (transfer none): A #GbProject.
 */
GbProject *
gb_action_get_project (GbAction *action)
{
   g_return_val_if_fail(GB_IS_ACTION(action), NULL);
   return action->priv->project;
}

void
gb_action_set_project (GbAction  *action,
                       GbProject *project)
{
   g_return_if_fail(GB_IS_ACTION(action));
   g_return_if_fail(!project || GB_IS_PROJECT(project));

   if (project != action->priv->project) {
      g_clear_object(&action->priv->project);
      action->priv->project =
         project ? g_object_ref(project) : NULL;
      g_object_notify_by_pspec(G_OBJECT(action),
                               gParamSpecs[PROP_PROJECT]);
   }
}

/**
 * gb_action_get_workspace:
 * @action: (in): A #GbAction.
 *
 * Gets the affected workspace.
 *
 * Returns: (transfer none): A #GbWorkspace.
 */
GbWorkspace *
gb_action_get_workspace (GbAction *action)
{
   g_return_val_if_fail(GB_IS_ACTION(action), NULL);
   return action->priv->workspace;
}

void
gb_action_set_workspace (GbAction    *action,
                         GbWorkspace *workspace)
{
   g_return_if_fail(GB_IS_ACTION(action));
   g_return_if_fail(!workspace || GB_IS_WORKSPACE(workspace));

   if (workspace != action->priv->workspace) {
      g_clear_object(&action->priv->workspace);
      action->priv->workspace =
         workspace ? g_object_ref(workspace) : NULL;
      g_object_notify_by_pspec(G_OBJECT(action),
                               gParamSpecs[PROP_WORKSPACE]);
   }
}

static const gchar *
gb_action_get_name (GAction *action)
{
   g_return_val_if_fail(GB_IS_ACTION(action), NULL);
   return GB_ACTION(action)->priv->name;
}

static void
gb_action_set_name (GbAction    *action,
                    const gchar *name)
{
   g_return_if_fail(GB_IS_ACTION(action));
   g_free(action->priv->name);
   action->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(action), gParamSpecs[PROP_NAME]);
}

static const GVariantType *
gb_action_get_parameter_type (GAction *action)
{
   return NULL;
}

static const GVariantType *
gb_action_get_state_type (GAction *action)
{
   return NULL;
}

static GVariant *
gb_action_get_state_hint (GAction *action)
{
   return NULL;
}

static gboolean
gb_action_get_enabled (GAction *action)
{
   g_return_val_if_fail(GB_IS_ACTION(action), FALSE);
   return GB_ACTION(action)->priv->enabled;
}

void
gb_action_set_enabled (GbAction *action,
                       gboolean  enabled)
{
   g_return_if_fail(GB_IS_ACTION(action));
   action->priv->enabled = enabled;
   g_object_notify_by_pspec(G_OBJECT(action), gParamSpecs[PROP_ENABLED]);
}

static GVariant *
gb_action_get_state (GAction *action)
{
   return NULL;
}

static void
gb_action_change_state (GAction  *action,
                        GVariant *value)
{
}

static void
gb_action_activate (GAction  *action,
                    GVariant *parameter)
{
   g_signal_emit(action, gSignals[ACTIVATE], 0, parameter);
}

static void
g_action_init (GActionInterface *iface)
{
   iface->get_name = gb_action_get_name;
   iface->get_parameter_type = gb_action_get_parameter_type;
   iface->get_state_type = gb_action_get_state_type;
   iface->get_state_hint = gb_action_get_state_hint;
   iface->get_enabled = gb_action_get_enabled;
   iface->get_state = gb_action_get_state;
   iface->change_state = gb_action_change_state;
   iface->activate = gb_action_activate;
}

static void
gb_action_finalize (GObject *object)
{
   GbActionPrivate *priv;

   priv = GB_ACTION(object)->priv;

   g_clear_object(&priv->application);
   g_clear_object(&priv->project);
   g_clear_object(&priv->workspace);

   G_OBJECT_CLASS(gb_action_parent_class)->finalize(object);
}

static void
gb_action_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
   GbAction *action = GB_ACTION(object);

   switch (prop_id) {
   case PROP_APPLICATION:
      g_value_set_object(value, gb_action_get_application(action));
      break;
   case PROP_ENABLED:
      g_value_set_boolean(value, gb_action_get_enabled(G_ACTION(action)));
      break;
   case PROP_NAME:
      g_value_set_string(value, gb_action_get_name(G_ACTION(action)));
      break;
   case PROP_PARAMETER_TYPE:
      g_value_set_boxed(value, gb_action_get_parameter_type(G_ACTION(action)));
      break;
   case PROP_PROJECT:
      g_value_set_object(value, gb_action_get_project(action));
      break;
   case PROP_STATE:
      g_value_set_variant(value, NULL);
      break;
   case PROP_STATE_TYPE:
      g_value_set_boxed(value, gb_action_get_state_type(G_ACTION(action)));
      break;
   case PROP_WORKSPACE:
      g_value_set_object(value, gb_action_get_workspace(action));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_action_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
   GbAction *action = GB_ACTION(object);

   switch (prop_id) {
   case PROP_APPLICATION:
      gb_action_set_application(action, g_value_get_object(value));
      break;
   case PROP_ENABLED:
      gb_action_set_enabled(action, g_value_get_boolean(value));
      break;
   case PROP_NAME:
      gb_action_set_name(action, g_value_get_string(value));
      break;
   case PROP_PROJECT:
      gb_action_set_project(action, g_value_get_object(value));
      break;
   case PROP_WORKSPACE:
      gb_action_set_workspace(action, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_action_class_init (GbActionClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_action_finalize;
   object_class->get_property = gb_action_get_property;
   object_class->set_property = gb_action_set_property;
   g_type_class_add_private(object_class, sizeof(GbActionPrivate));

   gParamSpecs[PROP_APPLICATION] =
      g_param_spec_object("application",
                          _("Application"),
                          _("The application in question."),
                          GB_TYPE_APPLICATION,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_APPLICATION,
                                   gParamSpecs[PROP_APPLICATION]);

   gParamSpecs[PROP_ENABLED] =
      g_param_spec_boolean("enabled",
                          _("Enabled"),
                          _("If the action is enabled."),
                          TRUE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_ENABLED,
                                   gParamSpecs[PROP_ENABLED]);

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the action."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);

   gParamSpecs[PROP_PARAMETER_TYPE] =
      g_param_spec_boxed("parameter-type",
                          _("Parameter Type"),
                          _("The parameter type of the GAction."),
                          G_TYPE_VARIANT_TYPE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PARAMETER_TYPE,
                                   gParamSpecs[PROP_PARAMETER_TYPE]);

   gParamSpecs[PROP_PROJECT] =
      g_param_spec_object("project",
                          _("Project"),
                          _("The project in question."),
                          GB_TYPE_PROJECT,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_PROJECT,
                                   gParamSpecs[PROP_PROJECT]);

   gParamSpecs[PROP_STATE] =
      g_param_spec_variant("state",
                           _("State"),
                           _("The state of the GAction."),
                           G_VARIANT_TYPE_ANY,
                           NULL,
                           (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STATE,
                                   gParamSpecs[PROP_STATE]);

   gParamSpecs[PROP_STATE_TYPE] =
      g_param_spec_boxed("state-type",
                          _("State Type"),
                          _("The state type of the GAction."),
                          G_TYPE_VARIANT_TYPE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STATE_TYPE,
                                   gParamSpecs[PROP_STATE_TYPE]);

   gParamSpecs[PROP_WORKSPACE] =
      g_param_spec_object("workspace",
                          _("Workspace"),
                          _("The workspace in question."),
                          GB_TYPE_WORKSPACE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_WORKSPACE,
                                   gParamSpecs[PROP_WORKSPACE]);

   gSignals[ACTIVATE] = g_signal_new("activate",
                                     GB_TYPE_ACTION,
                                     G_SIGNAL_RUN_FIRST,
                                     0,
                                     NULL,
                                     NULL,
                                     g_cclosure_marshal_VOID__VARIANT,
                                     G_TYPE_NONE,
                                     1,
                                     G_TYPE_VARIANT);
}

static void
gb_action_init (GbAction *action)
{
   action->priv = G_TYPE_INSTANCE_GET_PRIVATE(action,
                                              GB_TYPE_ACTION,
                                              GbActionPrivate);

   action->priv->enabled = TRUE;
}

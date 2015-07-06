/* ide-plugin-adapter.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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
#include <libpeas/peas.h>
#include <stdlib.h>

#include "ide-context.h"
#include "ide-macros.h"
#include "ide-plugin-adapter.h"

struct _IdePluginAdapter
{
  IdeObject      parent_instnace;

  PeasExtension *extension;
  PeasEngine    *engine;
  gchar         *match_key;
  gchar         *match_value;

  GType          interface_type;
};

G_DEFINE_TYPE (IdePluginAdapter, ide_plugin_adapter, IDE_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_INTERFACE_TYPE,
  PROP_ENGINE,
  PROP_EXTENSION,
  PROP_MATCH_KEY,
  PROP_MATCH_VALUE,
  LAST_PROP
};

enum {
  CREATE,
  LAST_SIGNAL
};

static GParamSpec *gParamSpecs [LAST_PROP];

IdePluginAdapter *
ide_plugin_adapter_new (IdeContext  *context,
                        PeasEngine  *engine,
                        GType        interface_type,
                        const gchar *match_key)
{
  g_return_val_if_fail (IDE_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (PEAS_IS_ENGINE (engine), NULL);
  g_return_val_if_fail (G_TYPE_IS_INTERFACE (interface_type), NULL);

  return g_object_new (IDE_TYPE_PLUGIN_ADAPTER,
                       "context", context,
                       "engine", engine,
                       "interface-type", interface_type,
                       "match-key", match_key,
                       NULL);
}

/**
 * ide_plugin_adapter_get_engine:
 *
 * Gets the #IdePluginAdapter:engine property.
 *
 * Returns: (transfer none): A #PeasEngine.
 */
PeasEngine *
ide_plugin_adapter_get_engine (IdePluginAdapter *self)
{
  g_return_val_if_fail (IDE_IS_PLUGIN_ADAPTER (self), NULL);

  return self->engine;
}

/**
 * ide_plugin_adapter_get_interface_type:
 *
 * Gets the #IdePluginAdapter:interface-type property.
 */
GType
ide_plugin_adapter_get_interface_type (IdePluginAdapter *self)
{
  g_return_val_if_fail (IDE_IS_PLUGIN_ADAPTER (self), G_TYPE_INVALID);

  return self->interface_type;
}

static void
ide_plugin_adapter_set_interface_type (IdePluginAdapter *self,
                                       GType             interface_type)
{
  g_return_if_fail (IDE_IS_PLUGIN_ADAPTER (self));
  g_return_if_fail (G_TYPE_IS_INTERFACE (interface_type));

  if (interface_type != self->interface_type)
    {
      self->interface_type = interface_type;
      g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_INTERFACE_TYPE]);
    }
}

static gboolean
ide_plugin_adapter_plugin_info_matches (IdePluginAdapter *self,
                                        PeasPluginInfo   *plugin_info)
{
  const gchar *value;
  gchar **parts;
  gboolean ret;

  g_assert (IDE_IS_PLUGIN_ADAPTER (self));
  g_assert (plugin_info != NULL);

  if (self->match_key == NULL)
    return FALSE;

  value = peas_plugin_info_get_external_data (plugin_info, self->match_key);
  if (value == NULL)
    return FALSE;

  parts = g_strsplit (value, ",", 0);
  ret = g_strv_contains ((const gchar * const *)parts, self->match_value);
  g_strfreev (parts);

  return ret;
}

static void
ide_plugin_adapter_reload (IdePluginAdapter *self)
{
  PeasPluginInfo *best_match = NULL;
  g_autofree gchar *priority_name = NULL;
  const GList *list;
  gint best_match_priority = G_MAXINT;

  g_assert (IDE_IS_PLUGIN_ADAPTER (self));

  list = peas_engine_get_plugin_list (self->engine);
  priority_name = g_strdup_printf ("%s-Priority", self->match_key);

  for (; list; list = list->next)
    {
      PeasPluginInfo *plugin_info = list->data;

      if (peas_engine_provides_extension (self->engine, plugin_info, self->interface_type))
        {
          const gchar *priority_str;
          gint priority;

          if (!ide_plugin_adapter_plugin_info_matches (self, plugin_info))
            continue;

          priority_str = peas_plugin_info_get_external_data (plugin_info, priority_name);
          priority = priority_str ? atoi (priority_str) : 0;

          /*
           * Lower integer value indicates higher priority.
           */
          if (priority < best_match_priority)
            {
              best_match = plugin_info;
              best_match_priority = priority;
            }
        }
    }

  /*
   * If the best_match type matches our current extension type, then there
   * is nothing more to do. We can short circuit.
   */
  if (self->extension != NULL)
    {
      PeasExtensionBase *base = PEAS_EXTENSION_BASE (self->extension);

      if (peas_extension_base_get_plugin_info (base) == best_match)
        return;
    }

  /*
   * Release our previously loaded extension.
   */
  g_clear_object (&self->extension);

  /*
   * If we found a matching plugin, load it now.
   */
  if (best_match != NULL)
    {
      IdeContext *context;

      context = ide_object_get_context (IDE_OBJECT (self));
      self->extension = peas_engine_create_extension (self->engine,
                                                      best_match,
                                                      self->interface_type,
                                                      "context", context,
                                                      NULL);

      if (G_IS_INITIALLY_UNOWNED (self->extension))
        g_object_ref_sink (self->extension);
    }

  g_assert ((self->extension == NULL) ||
            (g_type_is_a (G_TYPE_FROM_INSTANCE (self->extension), self->interface_type)));

  g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_EXTENSION]);
}

static void
ide_plugin_adapter__engine_load_plugin (IdePluginAdapter *self,
                                        PeasPluginInfo   *plugin_info,
                                        PeasEngine       *engine)
{
  g_return_if_fail (IDE_IS_PLUGIN_ADAPTER (self));
  g_return_if_fail (plugin_info != NULL);
  g_return_if_fail (PEAS_IS_ENGINE (engine));

  if (peas_engine_provides_extension (engine, plugin_info, self->interface_type))
    ide_plugin_adapter_reload (self);
}

static void
ide_plugin_adapter__engine_unload_plugin (IdePluginAdapter *self,
                                          PeasPluginInfo   *plugin_info,
                                          PeasEngine       *engine)
{
  g_return_if_fail (IDE_IS_PLUGIN_ADAPTER (self));
  g_return_if_fail (plugin_info != NULL);
  g_return_if_fail (PEAS_IS_ENGINE (engine));

  if (self->extension != NULL)
    {
      PeasExtensionBase *base = PEAS_EXTENSION_BASE (self->extension);

      if (peas_extension_base_get_plugin_info (base) == plugin_info)
        ide_plugin_adapter_reload (self);
    }
}

static void
ide_plugin_adapter_set_engine (IdePluginAdapter *self,
                               PeasEngine       *engine)
{
  g_return_if_fail (IDE_IS_PLUGIN_ADAPTER (self));
  g_return_if_fail (!engine || PEAS_IS_ENGINE (engine));

  if (engine == NULL)
    engine = peas_engine_get_default ();

  self->engine = g_object_ref (engine);

  g_signal_connect_object (self->engine,
                           "load-plugin",
                           G_CALLBACK (ide_plugin_adapter__engine_load_plugin),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->engine,
                           "unload-plugin",
                           G_CALLBACK (ide_plugin_adapter__engine_unload_plugin),
                           self,
                           G_CONNECT_SWAPPED);
}

static void
ide_plugin_adapter_set_match_key (IdePluginAdapter *self,
                                  const gchar      *match_key)
{
  g_return_if_fail (IDE_IS_PLUGIN_ADAPTER (self));

  if (!ide_str_equal0 (match_key, self->match_key))
    {
      g_free (self->match_key);
      self->match_key = g_strdup (match_key);
      g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_MATCH_KEY]);
    }
}

const gchar *
ide_plugin_adapter_get_match_key (IdePluginAdapter *self)
{
  g_return_val_if_fail (IDE_IS_PLUGIN_ADAPTER (self), NULL);

  return self->match_key;
}

const gchar *
ide_plugin_adapter_get_match_value (IdePluginAdapter *self)
{
  g_return_val_if_fail (IDE_IS_PLUGIN_ADAPTER (self), NULL);

  return self->match_value;
}

void
ide_plugin_adapter_set_match_value (IdePluginAdapter *self,
                                    const gchar      *match_value)
{
  g_return_if_fail (IDE_IS_PLUGIN_ADAPTER (self));

  if (!ide_str_equal0 (match_value, self->match_value))
    {
      g_free (self->match_value);
      self->match_value = g_strdup (match_value);
      ide_plugin_adapter_reload (self);
      g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_MATCH_VALUE]);
    }
}

/**
 * ide_plugin_adapter_get_extension:
 *
 * Gets the #IdePluginAdapter:extension property.
 *
 * Returns: (transfer none) (type GObject.Object) (nullable): A #GObject or %NULL.
 */
gpointer
ide_plugin_adapter_get_extension (IdePluginAdapter *self)
{
  g_return_val_if_fail (IDE_IS_PLUGIN_ADAPTER (self), NULL);

  return self->extension;
}

static void
ide_plugin_adapter_finalize (GObject *object)
{
  IdePluginAdapter *self = (IdePluginAdapter *)object;

  g_clear_object (&self->engine);
  g_clear_object (&self->extension);
  g_clear_pointer (&self->match_key, g_free);
  g_clear_pointer (&self->match_value, g_free);

  self->interface_type = G_TYPE_INVALID;

  G_OBJECT_CLASS (ide_plugin_adapter_parent_class)->finalize (object);
}

static void
ide_plugin_adapter_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  IdePluginAdapter *self = IDE_PLUGIN_ADAPTER (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      g_value_set_object (value, self->engine);
      break;

    case PROP_EXTENSION:
      g_value_set_object (value, self->extension);
      break;

    case PROP_INTERFACE_TYPE:
      g_value_set_gtype (value, self->interface_type);
      break;

    case PROP_MATCH_KEY:
      g_value_set_string (value, self->match_key);
      break;

    case PROP_MATCH_VALUE:
      g_value_set_string (value, self->match_value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_plugin_adapter_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  IdePluginAdapter *self = IDE_PLUGIN_ADAPTER (object);

  switch (prop_id)
    {
    case PROP_ENGINE:
      ide_plugin_adapter_set_engine (self, g_value_get_object (value));
      break;

    case PROP_INTERFACE_TYPE:
      ide_plugin_adapter_set_interface_type (self, g_value_get_gtype (value));
      break;

    case PROP_MATCH_KEY:
      ide_plugin_adapter_set_match_key (self, g_value_get_string (value));
      break;

    case PROP_MATCH_VALUE:
      ide_plugin_adapter_set_match_value (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_plugin_adapter_class_init (IdePluginAdapterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_plugin_adapter_finalize;
  object_class->get_property = ide_plugin_adapter_get_property;
  object_class->set_property = ide_plugin_adapter_set_property;

  gParamSpecs [PROP_ENGINE] =
    g_param_spec_object ("engine",
                         _("Engine"),
                         _("Engine"),
                         PEAS_TYPE_ENGINE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  gParamSpecs [PROP_EXTENSION] =
    g_param_spec_object ("extension",
                         _("Extension"),
                         _("Extension"),
                         G_TYPE_OBJECT,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  gParamSpecs [PROP_INTERFACE_TYPE] =
    g_param_spec_gtype ("interface-type",
                        _("Interface Type"),
                        _("Interface Type"),
                        G_TYPE_INTERFACE,
                        (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  gParamSpecs [PROP_MATCH_KEY] =
    g_param_spec_string ("match-key",
                         _("Match Key"),
                         _("The match key to use to locate a proper extension."),
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  gParamSpecs [PROP_MATCH_VALUE] =
    g_param_spec_string ("match-value",
                         _("Match Value"),
                         _("Match Value"),
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);
}

static void
ide_plugin_adapter_init (IdePluginAdapter *self)
{
}

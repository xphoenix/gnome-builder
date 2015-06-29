/* ide-extension-point.c
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

#define G_LOG_DOMAIN "ide-extension-point"

#include <glib/gi18n.h>
#include <libpeas/peas.h>
#include <stdlib.h>

#include "ide-extension-point.h"

struct _IdeExtensionPoint
{
  GObject      parent_instance;

  GType        interface_type;
  const gchar *match_key;
  const gchar *priority_key;
};

enum {
  PROP_0,
  PROP_REQUIRED_TYPE,
  LAST_PROP
};

enum {
  CHANGED,
  LAST_SIGNAL
};

G_DEFINE_TYPE (IdeExtensionPoint, ide_extension_point, G_TYPE_OBJECT)

static GParamSpec *gParamSpecs [LAST_PROP];
static GRecMutex   gExtensionsMutex;
static GHashTable *gExtensions;
static guint       gSignals [LAST_SIGNAL];

static void
load_plugin_cb (PeasEngine     *engine,
                PeasPluginInfo *plugin_info,
                gpointer        unused)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_assert (PEAS_IS_ENGINE (engine));
  g_assert (plugin_info != NULL);

  g_rec_mutex_lock (&gExtensionsMutex);

  g_hash_table_iter_init (&iter, gExtensions);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      IdeExtensionPoint *point = value;

      if (peas_engine_provides_extension (engine, plugin_info, point->interface_type))
        g_signal_emit (point, gSignals [CHANGED], 0);
    }

  g_rec_mutex_unlock (&gExtensionsMutex);
}

static void
unload_plugin_cb (PeasEngine     *engine,
                  PeasPluginInfo *plugin_info,
                  gpointer        unused)
{
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_assert (PEAS_IS_ENGINE (engine));
  g_assert (plugin_info != NULL);

  g_rec_mutex_lock (&gExtensionsMutex);

  g_hash_table_iter_init (&iter, gExtensions);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      IdeExtensionPoint *point = value;

      if (peas_engine_provides_extension (engine, plugin_info, point->interface_type))
        g_signal_emit (point, gSignals [CHANGED], 0);
    }

  g_rec_mutex_unlock (&gExtensionsMutex);
}

static void
ensure_plugin_signals_locked (void)
{
  static gsize initialized;

  if (g_once_init_enter (&initialized))
    {
      PeasEngine *engine;

      engine = peas_engine_get_default ();
      g_signal_connect (engine,
                        "load-plugin",
                        G_CALLBACK (load_plugin_cb),
                        NULL);
      g_signal_connect (engine,
                        "unload-plugin",
                        G_CALLBACK (unload_plugin_cb),
                        NULL);
      g_once_init_leave (&initialized, TRUE);
    }
}

static void
ide_extension_point_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  IdeExtensionPoint *self = (IdeExtensionPoint *)object;

  switch (prop_id)
    {
    case PROP_REQUIRED_TYPE:
      g_value_set_gtype (value, self->interface_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_extension_point_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  IdeExtensionPoint *self = (IdeExtensionPoint *)object;

  switch (prop_id)
    {
    case PROP_REQUIRED_TYPE:
      self->interface_type = g_value_get_gtype (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_extension_point_class_init (IdeExtensionPointClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = ide_extension_point_get_property;
  object_class->set_property = ide_extension_point_set_property;

  gParamSpecs [PROP_REQUIRED_TYPE] =
    g_param_spec_gtype ("required-type",
                        _("Required Type"),
                        _("Required Type"),
                        G_TYPE_INTERFACE,
                        (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);

  gSignals [CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
ide_extension_point_init (IdeExtensionPoint *self)
{
}

void
ide_extension_point_register (GType        interface_type,
                              const gchar *match_key,
                              const gchar *priority_key)
{
  IdeExtensionPoint *point;

  g_return_if_fail (G_TYPE_IS_INTERFACE (interface_type));

  point = g_object_new (IDE_TYPE_EXTENSION_POINT,
                        "required-type", interface_type,
                        NULL);
  point->match_key = g_intern_string (match_key);
  point->priority_key = g_intern_string (priority_key);

  g_rec_mutex_lock (&gExtensionsMutex);

  if (gExtensions == NULL)
    gExtensions = g_hash_table_new (NULL, NULL);

  if (g_hash_table_contains (gExtensions, (gpointer)interface_type))
    {
      g_warning ("IdeExtensionPoint for interface \"%s\" is already registered.",
                 g_type_name (interface_type));
      goto unlock;
    }

  g_hash_table_insert (gExtensions, (gpointer)interface_type, point);

unlock:
  g_rec_mutex_unlock (&gExtensionsMutex);
}

/**
 * ide_extension_point_lookup:
 *
 * Gets an extension point by name.
 *
 * Returns: (transfer none): An #IdeExtensionPoint.
 */
IdeExtensionPoint *
ide_extension_point_lookup (GType type)
{
  IdeExtensionPoint *point = NULL;

  g_return_val_if_fail (G_TYPE_IS_INTERFACE (type), NULL);

  g_rec_mutex_lock (&gExtensionsMutex);

  ensure_plugin_signals_locked ();

  if (gExtensions != NULL)
    point = g_hash_table_lookup (gExtensions, (gpointer)type);

  g_rec_mutex_unlock (&gExtensionsMutex);

  return point;
}

static gboolean
matches_value (IdeExtensionPoint *point,
               PeasPluginInfo    *plugin_info,
               const gchar       *match_value)
{
  const gchar *line;

  if ((point->match_key == NULL) || (match_value == NULL))
    return TRUE;

  line = peas_plugin_info_get_external_data (plugin_info, point->match_key);

  if (line != NULL)
    {
      g_auto(GStrv) parts = NULL;
      gint i;

      if (strstr (line, ",") == NULL)
        return (g_strcmp0 (match_value, line) == 0);

      parts = g_strsplit (line, ",", 0);

      if (g_strv_contains ((const gchar * const *)parts, match_value))
        return TRUE;
    }

  return FALSE;
}

/**
 * ide_extension_point_create:
 *
 * Creates a new instance of an extension point based on priority.
 *
 * Returns: (transfer full) (nullable) (type GObject.Object): A new #GObject or %NULL.
 */
gpointer
ide_extension_point_create (GType        type,
                            const gchar *match_value,
                            const gchar *first_property,
                            ...)
{
  IdeExtensionPoint *point;
  const GList *list;
  PeasEngine *engine;
  PeasPluginInfo *best_match = NULL;
  gint best_match_priority = G_MAXINT32;
  gpointer ret = NULL;
  va_list args;

  g_return_val_if_fail (G_TYPE_IS_INTERFACE (type), NULL);

  point = ide_extension_point_lookup (type);

  if (point == NULL)
    {
      g_warning ("No IdeExtensionPoint has been registered for type \"%s\".",
                 g_type_name (type));
      return NULL;
    }

  engine = peas_engine_get_default ();
  list = peas_engine_get_plugin_list (engine);

  for (; list; list = list->next)
    {
      PeasPluginInfo *plugin_info = list->data;
      const gchar *priority_str;
      gint priority = 0;

      if (!peas_plugin_info_is_loaded (plugin_info))
        continue;

      if (!peas_engine_provides_extension (engine, plugin_info, type))
        continue;

      if (!matches_value (point, plugin_info, match_value))
        continue;

      priority_str = peas_plugin_info_get_external_data (plugin_info, point->priority_key);

      if (priority_str != NULL)
        priority = atoi (priority_str);

      if (priority < best_match_priority)
        best_match = plugin_info;
    }

  if (best_match)
    {
      va_start (args, first_property);
      ret = peas_engine_create_extension_valist (engine, best_match, type, first_property, args);
      va_end (args);
    }

  return ret;
}

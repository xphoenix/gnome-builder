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

#include "ide-extension-point.h"

struct _IdeExtensionPoint
{
  GObject      parent_instance;
  const gchar *name;
  GPtrArray   *extensions;
};

typedef struct
{
  const gchar *module_name;
  GType        type;
  gint         priority;
  guint        loaded : 1;
} IdeExtension;

enum {
  PROP_0,
  PROP_NAME,
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

static IdeExtension *
ide_extension_new (GType type,
                   gint  priority)
{
  IdeExtension *exten;
  GTypePlugin *plugin;

  exten = g_slice_new0 (IdeExtension);
  exten->type = type;
  exten->priority = priority;
  exten->loaded = TRUE;

  plugin = g_type_get_plugin (type);

  if (PEAS_IS_OBJECT_MODULE (plugin))
    {
      gchar *module_name;

      g_object_get (plugin, "module-name", &module_name, NULL);
      exten->module_name = g_intern_string (module_name);
      g_free (module_name);
    }

  return exten;
}

static void
load_plugin_cb (PeasEngine     *engine,
                PeasPluginInfo *plugin_info,
                gpointer        unused)
{
  const gchar *module_name;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_assert (PEAS_IS_ENGINE (engine));
  g_assert (plugin_info != NULL);

  /*
   * FIXME: Since we don't have loader information, we could possibly have
   *        two plugins from different loaders with the same module name.
   */

  module_name = peas_plugin_info_get_module_name (plugin_info);

  g_rec_mutex_lock (&gExtensionsMutex);

  g_hash_table_iter_init (&iter, gExtensions);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      IdeExtensionPoint *point = value;
      gboolean emit_changed = FALSE;
      gsize i;

      for (i = 0; i < point->extensions->len; i++)
        {
          IdeExtension *exten = g_ptr_array_index (point->extensions, i);

          if (g_strcmp0 (exten->module_name, module_name) == 0)
            {
              if (exten->loaded != TRUE)
                {
                  exten->loaded = TRUE;
                  emit_changed = TRUE;
                }
            }
        }

      if (emit_changed)
        g_signal_emit (point, gSignals [CHANGED], 0);
    }

  g_rec_mutex_unlock (&gExtensionsMutex);
}

static void
unload_plugin_cb (PeasEngine     *engine,
                  PeasPluginInfo *plugin_info,
                  gpointer        unused)
{
  const gchar *module_name;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_assert (PEAS_IS_ENGINE (engine));
  g_assert (plugin_info != NULL);

  /*
   * FIXME: Since we don't have loader information, we could possibly have
   *        two plugins from different loaders with the same module name.
   */

  module_name = peas_plugin_info_get_module_name (plugin_info);

  g_rec_mutex_lock (&gExtensionsMutex);

  g_hash_table_iter_init (&iter, gExtensions);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      IdeExtensionPoint *point = value;
      gboolean emit_changed = FALSE;
      gsize i;

      for (i = 0; i < point->extensions->len; i++)
        {
          IdeExtension *exten = g_ptr_array_index (point->extensions, i);

          if (g_strcmp0 (exten->module_name, module_name) == 0)
            {
              if (exten->loaded != FALSE)
                {
                  exten->loaded = FALSE;
                  emit_changed = TRUE;
                }
            }
        }

      if (emit_changed)
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
ide_extension_point_finalize (GObject *object)
{
  G_OBJECT_CLASS (ide_extension_point_parent_class)->finalize (object);
}

static void
ide_extension_point_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  IdeExtensionPoint *self = IDE_EXTENSION_POINT (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_static_string (value, self->name);
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
  IdeExtensionPoint *self = IDE_EXTENSION_POINT (object);

  switch (prop_id)
    {
    case PROP_NAME:
      self->name = g_intern_string (g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_extension_point_class_init (IdeExtensionPointClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_extension_point_finalize;
  object_class->get_property = ide_extension_point_get_property;
  object_class->set_property = ide_extension_point_set_property;

  gParamSpecs [PROP_NAME] =
    g_param_spec_string("name",
                        _("Name"),
                        _("Name"),
                        NULL,
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
  self->extensions = g_ptr_array_new ();
}

/**
 * ide_extension_point_lookup:
 *
 * Gets an extension point by name.
 *
 * Returns: (transfer none): An #IdeExtensionPoint.
 */
IdeExtensionPoint *
ide_extension_point_lookup (const gchar *name)
{
  IdeExtensionPoint *self;

  g_rec_mutex_lock (&gExtensionsMutex);

  ensure_plugin_signals_locked ();

  name = g_intern_string (name);

  if (gExtensions == NULL)
    gExtensions = g_hash_table_new (NULL, NULL);

  self = g_hash_table_lookup (gExtensions, name);

  if (self == NULL)
    {
      self = g_object_new (IDE_TYPE_EXTENSION_POINT,
                           "name", name,
                           NULL);
      g_hash_table_insert (gExtensions, (gchar *)name, self);
    }

  g_rec_mutex_unlock (&gExtensionsMutex);

  return self;
}

static gint
compare_extension (gconstpointer a,
                   gconstpointer b)
{
  const IdeExtension *exta = a;
  const IdeExtension *extb = b;

  if (exta->priority == extb->priority)
    return g_strcmp0 (g_type_name (exta->type), g_type_name (extb->type));

  return extb->priority - exta->priority;
}

void
ide_extension_point_implement (const gchar *name,
                               GType        implementation_type,
                               gint         priority)
{
  IdeExtensionPoint *self;
  IdeExtension *exten;

  g_return_if_fail (name);
  g_return_if_fail (*name != '\0');
  g_return_if_fail (implementation_type != G_TYPE_INVALID);
  g_return_if_fail (G_TYPE_IS_OBJECT (implementation_type));

  self = ide_extension_point_lookup (name);
  exten = ide_extension_new (implementation_type, priority);

  g_ptr_array_add (self->extensions, exten);
  g_ptr_array_sort (self->extensions, compare_extension);

  g_signal_emit (self, gSignals [CHANGED], 0);
}

/**
 * ide_extension_point_create:
 *
 * Creates a new instance of an extension point based on priority.
 *
 * Returns: (transfer full) (nullable) (type GObject.Object): A new #GObject or %NULL.
 */
gpointer
ide_extension_point_create (const gchar *name,
                            const gchar *first_property,
                            ...)
{
  IdeExtensionPoint *self;
  gpointer ret = NULL;
  va_list args;
  gint i;

  self = ide_extension_point_lookup (name);

  for (i = 0; (ret == NULL) && (i < self->extensions->len); i++)
    {
      IdeExtension *exten;

      exten = g_ptr_array_index (self->extensions, i);

      if (!exten->loaded)
        continue;

      va_start (args, first_property);
      ret = g_object_new_valist (exten->type, first_property, args);
      va_end (args);
    }

  return ret;
}

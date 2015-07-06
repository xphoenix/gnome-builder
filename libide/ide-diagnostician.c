/* ide-diagnostician.c
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

#define G_LOG_DOMAIN "ide-diagnostician"

#include <glib/gi18n.h>
#include <gtksourceview/gtksource.h>
#include <libpeas/peas.h>

#include "ide-debug.h"
#include "ide-diagnostic-provider.h"
#include "ide-diagnostician.h"
#include "ide-diagnostics.h"
#include "ide-file.h"
#include "ide-internal.h"

struct _IdeDiagnostician
{
  IdeObject          parent_instance;

  GPtrArray         *providers;
  GtkSourceLanguage *language;
};

enum {
  PROP_0,
  PROP_LANGUAGE,
  LAST_PROP
};

typedef struct
{
  IdeDiagnostics *diagnostics;
  guint           total;
  guint           active;
} DiagnoseState;

G_DEFINE_TYPE (IdeDiagnostician, ide_diagnostician, IDE_TYPE_OBJECT)

static GParamSpec *gParamSpecs [LAST_PROP];

static void
diagnose_state_free (gpointer data)
{
  DiagnoseState *state = data;

  if (state)
    {
      g_clear_pointer (&state->diagnostics, ide_diagnostics_unref);
      g_slice_free (DiagnoseState, state);
    }
}

static void
diagnose_cb (GObject      *object,
             GAsyncResult *result,
             gpointer      user_data)
{
  IdeDiagnosticProvider *provider = (IdeDiagnosticProvider *)object;
  IdeDiagnostics *ret;
  g_autoptr(GTask) task = user_data;
  g_autoptr(GError) error = NULL;
  DiagnoseState *state;

  g_return_if_fail (IDE_IS_DIAGNOSTIC_PROVIDER (provider));
  g_return_if_fail (G_IS_TASK (task));

  state = g_task_get_task_data (task);

  state->active--;

  ret = ide_diagnostic_provider_diagnose_finish (provider, result, &error);

  if (!ret)
    goto maybe_complete;

  ide_diagnostics_merge (state->diagnostics, ret);
  ide_diagnostics_unref (ret);

maybe_complete:
  if (state->total == 1 && error)
    g_task_return_error (task, g_error_copy (error));
  else if (!state->active)
    g_task_return_pointer (task,
                           ide_diagnostics_ref (state->diagnostics),
                           (GDestroyNotify)ide_diagnostics_unref);
}

void
ide_diagnostician_diagnose_async (IdeDiagnostician    *self,
                                  IdeFile             *file,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  DiagnoseState *state;
  g_autoptr(GTask) task = NULL;
  gsize i;

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_DIAGNOSTICIAN (self));
  g_return_if_fail (IDE_IS_FILE (file));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (self, cancellable, callback, user_data);

  if (self->providers->len == 0)
    {
      g_task_return_pointer (task,
                             _ide_diagnostics_new (NULL),
                             (GDestroyNotify)ide_diagnostics_unref);
      IDE_EXIT;
    }

  state = g_slice_new0 (DiagnoseState);
  state->active = self->providers->len;
  state->total = self->providers->len;
  state->diagnostics = _ide_diagnostics_new (NULL);

  g_task_set_task_data (task, state, diagnose_state_free);

  for (i = 0; i < self->providers->len; i++)
    {
      IdeDiagnosticProvider *provider;

      provider = g_ptr_array_index (self->providers, i);
      ide_diagnostic_provider_diagnose_async (provider,
                                              file,
                                              cancellable,
                                              diagnose_cb,
                                              g_object_ref (task));
    }

  IDE_EXIT;
}

IdeDiagnostics *
ide_diagnostician_diagnose_finish (IdeDiagnostician  *self,
                                   GAsyncResult      *result,
                                   GError           **error)
{
  GTask *task = (GTask *)result;
  IdeDiagnostics *ret;

  IDE_ENTRY;

  g_return_val_if_fail (G_IS_TASK (result), NULL);

  ret = g_task_propagate_pointer (task, error);

  IDE_RETURN (ret);
}

static gboolean
contains_language (const gchar *languages,
                   const gchar *lang_id)
{
  g_auto(GStrv) parts = g_strsplit (languages, ",", 0);

  return (g_strv_contains ((const gchar * const *)parts, lang_id) ||
          g_strv_contains ((const gchar * const *)parts, "*"));
}

static void
ide_diagnostician__engine_load_plugin (IdeDiagnostician *self,
                                       PeasPluginInfo   *plugin_info,
                                       PeasEngine       *engine)
{
  PeasExtension *exten;
  IdeContext *context;
  const gchar *languages;
  const gchar *lang_id;

  g_assert (IDE_IS_DIAGNOSTICIAN (self));
  g_assert (plugin_info != NULL);
  g_assert (PEAS_IS_ENGINE (engine));

  if ((self->language == NULL) || !(lang_id = gtk_source_language_get_id (self->language)))
    return;

  if (!peas_engine_provides_extension (engine, plugin_info, IDE_TYPE_DIAGNOSTIC_PROVIDER))
    return;

  languages = peas_plugin_info_get_external_data (plugin_info, "Diagnostic-Languages");
  if (!contains_language (languages, lang_id))
    return;

  context = ide_object_get_context (IDE_OBJECT (self));
  exten = peas_engine_create_extension (engine, plugin_info, IDE_TYPE_DIAGNOSTIC_PROVIDER,
                                        "context", context,
                                        NULL);
  g_ptr_array_add (self->providers, exten);
}

static void
ide_diagnostician__engine_unload_plugin (IdeDiagnostician *self,
                                         PeasPluginInfo   *plugin_info,
                                         PeasEngine       *engine)
{
  gsize i;

  g_assert (IDE_IS_DIAGNOSTICIAN (self));
  g_assert (plugin_info != NULL);
  g_assert (PEAS_IS_ENGINE (engine));

  for (i = 0; i < self->providers->len; i++)
    {
      PeasExtension *exten = g_ptr_array_index (self->providers, i);

      if (peas_extension_base_get_plugin_info (PEAS_EXTENSION_BASE (exten)) == plugin_info)
        {
          g_ptr_array_remove_index_fast (self->providers, i);
          break;
        }
    }
}

static void
ide_diagnostician_reload (IdeDiagnostician *self,
                          const gchar      *lang_id)
{
  g_return_if_fail (IDE_IS_DIAGNOSTICIAN (self));

  if (self->providers->len > 0)
    g_ptr_array_remove_range (self->providers, 0, self->providers->len);

  if (lang_id != NULL)
    {
      PeasEngine *engine;
      const GList *list;

      engine = peas_engine_get_default ();
      list = peas_engine_get_plugin_list (engine);

      for (; list; list = list->next)
        ide_diagnostician__engine_load_plugin (self, list->data, engine);
    }
}

/**
 * ide_diagnostician_get_language:
 *
 * Gets the IdeDiagnostician::language property.
 *
 * Returns: (nullable) (transfer none): A #GtkSourceLanguage or %NULL.
 */
GtkSourceLanguage *
ide_diagnostician_get_language (IdeDiagnostician *self)
{
  g_return_val_if_fail (IDE_IS_DIAGNOSTICIAN (self), NULL);

  return self->language;
}

void
ide_diagnostician_set_language (IdeDiagnostician  *self,
                                GtkSourceLanguage *language)
{
  g_return_if_fail (IDE_IS_DIAGNOSTICIAN (self));
  g_return_if_fail (!language || GTK_SOURCE_IS_LANGUAGE (language));

  if (g_set_object (&self->language, language))
    {
      const gchar *lang_id = NULL;

      if (language != NULL)
        lang_id = gtk_source_language_get_id (language);
      ide_diagnostician_reload (self, lang_id);
      g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_LANGUAGE]);
    }
}

static void
ide_diagnostician_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  IdeDiagnostician *self = (IdeDiagnostician *)object;

  switch (prop_id)
    {
    case PROP_LANGUAGE:
      g_value_set_object (value, ide_diagnostician_get_language (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_diagnostician_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  IdeDiagnostician *self = (IdeDiagnostician *)object;

  switch (prop_id)
    {
    case PROP_LANGUAGE:
      ide_diagnostician_set_language (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_diagnostician_class_init (IdeDiagnosticianClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = ide_diagnostician_get_property;
  object_class->set_property = ide_diagnostician_set_property;

  gParamSpecs [PROP_LANGUAGE] =
    g_param_spec_object ("language",
                         _("Language"),
                         _("Language"),
                         GTK_SOURCE_TYPE_LANGUAGE,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);
}

static void
ide_diagnostician_init (IdeDiagnostician *self)
{
  PeasEngine *engine;

  self->providers = g_ptr_array_new_with_free_func (g_object_unref);

  engine = peas_engine_get_default ();

  g_signal_connect_object (engine,
                           "load-plugin",
                           G_CALLBACK (ide_diagnostician__engine_load_plugin),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (engine,
                           "unload-plugin",
                           G_CALLBACK (ide_diagnostician__engine_unload_plugin),
                           self,
                           G_CONNECT_SWAPPED);
}

IdeDiagnostician *
ide_diagnostician_new (IdeContext        *context,
                       GtkSourceLanguage *language)
{
  return g_object_new (IDE_TYPE_DIAGNOSTICIAN,
                       "context", context,
                       "language", language,
                       NULL);
}

gboolean
ide_diagnostician_is_ready (IdeDiagnostician *diagnostician)
{
  g_return_val_if_fail (IDE_IS_DIAGNOSTICIAN (diagnostician), FALSE);

  return (diagnostician->providers->len > 0);
}

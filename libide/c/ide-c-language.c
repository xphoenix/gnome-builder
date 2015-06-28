/* ide-c-language.c
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

#include "egg-signal-group.h"

#include "ide-c-format-provider.h"
#include "ide-c-indenter.h"
#include "ide-c-language.h"
#include "ide-extension-point.h"
#include "ide-internal.h"

typedef struct
{
  IdeDiagnostician  *diagnostician;
  IdeIndenter       *indenter;
} IdeCLanguagePrivate;

static void _g_initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_EXTENDED (IdeCLanguage, ide_c_language, IDE_TYPE_LANGUAGE, 0,
                        G_ADD_PRIVATE (IdeCLanguage)
                        G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                               _g_initable_iface_init))

static GList *
ide_c_language_get_completion_providers (IdeLanguage *language)
{
  GList *providers = NULL;

  g_return_val_if_fail (IDE_IS_C_LANGUAGE (language), NULL);

  providers = IDE_LANGUAGE_CLASS (ide_c_language_parent_class)->get_completion_providers (language);
#if 0
  providers = g_list_append (providers, g_object_new (IDE_TYPE_C_FORMAT_PROVIDER, NULL));
  providers = g_list_append (providers, g_object_new (IDE_TYPE_CLANG_COMPLETION_PROVIDER, NULL));
#endif

  return providers;
}

static IdeIndenter *
ide_c_language_get_indenter (IdeLanguage *language)
{
  IdeCLanguage *self = (IdeCLanguage *)language;
  IdeCLanguagePrivate *priv = ide_c_language_get_instance_private (self);

  g_return_val_if_fail (IDE_IS_C_LANGUAGE (self), NULL);

  return priv->indenter;
}

static const gchar *
ide_c_language_get_name (IdeLanguage *self)
{
  return _("C");
}

static void
ide_c_language_dispose (GObject *object)
{
  IdeCLanguage *self = (IdeCLanguage *)object;
  IdeCLanguagePrivate *priv = ide_c_language_get_instance_private (self);

  g_clear_object (&priv->diagnostician);
  g_clear_object (&priv->indenter);

  G_OBJECT_CLASS (ide_c_language_parent_class)->dispose (object);
}

static void
ide_c_language_class_init (IdeCLanguageClass *klass)
{
  IdeLanguageClass *language_class = IDE_LANGUAGE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  language_class->get_completion_providers = ide_c_language_get_completion_providers;
  language_class->get_indenter = ide_c_language_get_indenter;
  language_class->get_name = ide_c_language_get_name;

  object_class->dispose = ide_c_language_dispose;
}

static void
ide_c_language_init (IdeCLanguage *self)
{
}

static gboolean
ide_c_language_initiable_init (GInitable     *initable,
                               GCancellable  *cancellable,
                               GError       **error)
{
  IdeCLanguage *self = (IdeCLanguage *)initable;
  IdeCLanguagePrivate *priv = ide_c_language_get_instance_private (self);
  const gchar *id;

  g_return_val_if_fail (IDE_IS_C_LANGUAGE (self), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  id = ide_language_get_id (IDE_LANGUAGE (self));

  if ((g_strcmp0 (id, "c") == 0) ||
      (g_strcmp0 (id, "chdr") == 0) ||
      (g_strcmp0 (id, "cpp") == 0))
    {
      IdeContext *context;
      g_autofree gchar *path = NULL;

      context = ide_object_get_context (IDE_OBJECT (initable));

      /*
       * Create our indenter to provide as-you-type indentation.
       *
       * TODO: How can we disambiguate c/c++/obj-c headers?
       */
      if (!ide_str_equal0 (id, "cpp"))
        priv->indenter = g_object_new (IDE_TYPE_C_INDENTER,
                                       "context", context,
                                       NULL);

      /*
       * TODO: Refactory design (rename local, extract method, etc).
       */

      return TRUE;
    }

  g_set_error (error,
               G_IO_ERROR,
               G_IO_ERROR_NOT_SUPPORTED,
               _("Language id does not match a C language."));

  return FALSE;
}

static void
_g_initable_iface_init (GInitableIface *iface)
{
  iface->init = ide_c_language_initiable_init;
}

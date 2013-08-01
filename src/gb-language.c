/* gb-language.c
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

#include "gb-language.h"

G_DEFINE_ABSTRACT_TYPE(GbLanguage, gb_language, G_TYPE_OBJECT)

struct _GbLanguagePrivate
{
   gchar *name;
};

enum
{
   PROP_0,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_language_get_name (GbLanguage *language)
{
   return language->priv->name;
}

void
gb_language_set_name (GbLanguage *language,
                      const gchar *name)
{
   g_return_if_fail(GB_IS_LANGUAGE(language));

   g_free(language->priv->name);
   language->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(language), gParamSpecs[PROP_NAME]);
}

static void
gb_language_finalize (GObject *object)
{
   GbLanguagePrivate *priv;

   priv = GB_LANGUAGE(object)->priv;

   g_clear_pointer(&priv->name, g_free);

   G_OBJECT_CLASS(gb_language_parent_class)->finalize(object);
}

static void
gb_language_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
   GbLanguage *language = GB_LANGUAGE(object);

   switch (prop_id) {
   case PROP_NAME:
      g_value_set_string(value, gb_language_get_name(language));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_language_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
   GbLanguage *language = GB_LANGUAGE(object);

   switch (prop_id) {
   case PROP_NAME:
      gb_language_set_name(language, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_language_class_init (GbLanguageClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_language_finalize;
   object_class->get_property = gb_language_get_property;
   object_class->set_property = gb_language_set_property;
   g_type_class_add_private(object_class, sizeof(GbLanguagePrivate));

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("The name of the language."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_language_init (GbLanguage *language)
{
   language->priv = G_TYPE_INSTANCE_GET_PRIVATE(language,
                                                GB_TYPE_LANGUAGE,
                                                GbLanguagePrivate);
}

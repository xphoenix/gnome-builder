/* gb-c-language-service.c
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

#include "gb-c-language-service.h"

G_DEFINE_TYPE(GbCLanguageService, gb_c_language_service, GB_TYPE_SERVICE)

struct _GbCLanguageServicePrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

#if 0
static GParamSpec *gParamSpecs[LAST_PROP];
#endif

static void
gb_c_language_service_finalize (GObject *object)
{
#if 0
   GbCLanguageServicePrivate *priv;

   priv = GB_C_LANGUAGE_SERVICE(object)->priv;
#endif

   G_OBJECT_CLASS(gb_c_language_service_parent_class)->finalize(object);
}

static void
gb_c_language_service_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
#if 0
   GbCLanguageService *service = GB_C_LANGUAGE_SERVICE(object);
#endif

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_c_language_service_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
#if 0
   GbCLanguageService *service = GB_C_LANGUAGE_SERVICE(object);
#endif

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_c_language_service_class_init (GbCLanguageServiceClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_c_language_service_finalize;
   object_class->get_property = gb_c_language_service_get_property;
   object_class->set_property = gb_c_language_service_set_property;
   g_type_class_add_private(object_class, sizeof(GbCLanguageServicePrivate));
}

static void
gb_c_language_service_init (GbCLanguageService *service)
{
   service->priv = G_TYPE_INSTANCE_GET_PRIVATE(service,
                                               GB_TYPE_C_LANGUAGE_SERVICE,
                                               GbCLanguageServicePrivate);
}

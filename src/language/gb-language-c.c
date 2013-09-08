/* gb-language-c.c
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

#include "gb-language-c.h"

G_DEFINE_TYPE(GbLanguageC, gb_language_c, GB_TYPE_LANGUAGE)

struct _GbLanguageCPrivate
{
   gpointer dummy;
};

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_language_c_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_language_c_parent_class)->finalize(object);
}

static void
gb_language_c_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
   //GbLanguageC *c = GB_LANGUAGE_C(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_language_c_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
   //GbLanguageC *c = GB_LANGUAGE_C(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_language_c_class_init (GbLanguageCClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_language_c_finalize;
   object_class->get_property = gb_language_c_get_property;
   object_class->set_property = gb_language_c_set_property;
   g_type_class_add_private(object_class, sizeof(GbLanguageCPrivate));
}

static void
gb_language_c_init (GbLanguageC *c)
{
   c->priv = G_TYPE_INSTANCE_GET_PRIVATE(c,
                                         GB_TYPE_LANGUAGE_C,
                                         GbLanguageCPrivate);

   g_object_set(c, "name", "C", NULL);
}

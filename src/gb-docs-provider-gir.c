/* gb-docs-provider-gir.c
 *
 * Copyright (C) 2013 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gb-docs-provider-gir.h"

G_DEFINE_TYPE(GbDocsProviderGir, gb_docs_provider_gir, GB_TYPE_DOCS_PROVIDER)

struct _GbDocsProviderGirPrivate
{
   gpointer dummy;
};

static void
gb_docs_provider_gir_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_docs_provider_gir_parent_class)->finalize(object);
}

static void
gb_docs_provider_gir_class_init (GbDocsProviderGirClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_docs_provider_gir_finalize;
   g_type_class_add_private(object_class, sizeof(GbDocsProviderGirPrivate));
}

static void
gb_docs_provider_gir_init (GbDocsProviderGir *gir)
{
   gir->priv = G_TYPE_INSTANCE_GET_PRIVATE(gir,
                                           GB_TYPE_DOCS_PROVIDER_GIR,
                                           GbDocsProviderGirPrivate);

   g_object_set(gir, "name", "Gir", NULL);
}

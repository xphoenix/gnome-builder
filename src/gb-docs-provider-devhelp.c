/* gb-docs-provider-devhelp.c
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

#include "gb-docs-provider-devhelp.h"

G_DEFINE_TYPE(GbDocsProviderDevhelp,
              gb_docs_provider_devhelp,
              GB_TYPE_DOCS_PROVIDER)

struct _GbDocsProviderDevhelpPrivate
{
   gpointer dummy;
};

static void
gb_docs_provider_devhelp_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_docs_provider_devhelp_parent_class)->finalize(object);
}

static void
gb_docs_provider_devhelp_class_init (GbDocsProviderDevhelpClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_docs_provider_devhelp_finalize;
   g_type_class_add_private(object_class, sizeof(GbDocsProviderDevhelpPrivate));
}

static void
gb_docs_provider_devhelp_init (GbDocsProviderDevhelp *devhelp)
{
   devhelp->priv = G_TYPE_INSTANCE_GET_PRIVATE(devhelp,
                                               GB_TYPE_DOCS_PROVIDER_DEVHELP,
                                               GbDocsProviderDevhelpPrivate);

   g_object_set(devhelp, "name", "Devhelp", NULL);
}

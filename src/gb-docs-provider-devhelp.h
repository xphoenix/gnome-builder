/* gb-docs-provider-devhelp.h
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

#ifndef GB_DOCS_PROVIDER_DEVHELP_H
#define GB_DOCS_PROVIDER_DEVHELP_H

#include "gb-docs-provider.h"

G_BEGIN_DECLS

#define GB_TYPE_DOCS_PROVIDER_DEVHELP            (gb_docs_provider_devhelp_get_type())
#define GB_DOCS_PROVIDER_DEVHELP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOCS_PROVIDER_DEVHELP, GbDocsProviderDevhelp))
#define GB_DOCS_PROVIDER_DEVHELP_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_DOCS_PROVIDER_DEVHELP, GbDocsProviderDevhelp const))
#define GB_DOCS_PROVIDER_DEVHELP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_DOCS_PROVIDER_DEVHELP, GbDocsProviderDevhelpClass))
#define GB_IS_DOCS_PROVIDER_DEVHELP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_DOCS_PROVIDER_DEVHELP))
#define GB_IS_DOCS_PROVIDER_DEVHELP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_DOCS_PROVIDER_DEVHELP))
#define GB_DOCS_PROVIDER_DEVHELP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_DOCS_PROVIDER_DEVHELP, GbDocsProviderDevhelpClass))

typedef struct _GbDocsProviderDevhelp        GbDocsProviderDevhelp;
typedef struct _GbDocsProviderDevhelpClass   GbDocsProviderDevhelpClass;
typedef struct _GbDocsProviderDevhelpPrivate GbDocsProviderDevhelpPrivate;

struct _GbDocsProviderDevhelp
{
   GbDocsProvider parent;

   /*< private >*/
   GbDocsProviderDevhelpPrivate *priv;
};

struct _GbDocsProviderDevhelpClass
{
   GbDocsProviderClass parent_class;
};

GType gb_docs_provider_devhelp_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_DOCS_PROVIDER_DEVHELP_H */

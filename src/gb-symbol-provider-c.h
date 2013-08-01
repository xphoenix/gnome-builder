/* gb-symbol-provider-c.h:
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

#ifndef GB_SYMBOL_PROVIDER_C_H
#define GB_SYMBOL_PROVIDER_C_H

#include "gb-symbol-provider.h"

G_BEGIN_DECLS

#define GB_TYPE_SYMBOL_PROVIDER_C            (gb_symbol_provider_c_get_type())
#define GB_SYMBOL_PROVIDER_C(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SYMBOL_PROVIDER_C, GbSymbolProviderC))
#define GB_SYMBOL_PROVIDER_C_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SYMBOL_PROVIDER_C, GbSymbolProviderC const))
#define GB_SYMBOL_PROVIDER_C_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SYMBOL_PROVIDER_C, GbSymbolProviderCClass))
#define GB_IS_SYMBOL_PROVIDER_C(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SYMBOL_PROVIDER_C))
#define GB_IS_SYMBOL_PROVIDER_C_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SYMBOL_PROVIDER_C))
#define GB_SYMBOL_PROVIDER_C_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SYMBOL_PROVIDER_C, GbSymbolProviderCClass))

typedef struct _GbSymbolProviderC        GbSymbolProviderC;
typedef struct _GbSymbolProviderCClass   GbSymbolProviderCClass;
typedef struct _GbSymbolProviderCPrivate GbSymbolProviderCPrivate;

struct _GbSymbolProviderC
{
   GbSymbolProvider parent;

   /*< private >*/
   GbSymbolProviderCPrivate *priv;
};

struct _GbSymbolProviderCClass
{
   GbSymbolProviderClass parent_class;
};

GType gb_symbol_provider_c_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SYMBOL_PROVIDER_C_H */

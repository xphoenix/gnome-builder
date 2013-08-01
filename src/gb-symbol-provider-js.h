/* gb-symbol-provider-js.h:
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

#ifndef GB_SYMBOL_PROVIDER_JS_H
#define GB_SYMBOL_PROVIDER_JS_H

#include "gb-symbol-provider.h"

G_BEGIN_DECLS

#define GB_TYPE_SYMBOL_PROVIDER_JS            (gb_symbol_provider_js_get_type())
#define GB_SYMBOL_PROVIDER_JS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SYMBOL_PROVIDER_JS, GbSymbolProviderJs))
#define GB_SYMBOL_PROVIDER_JS_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SYMBOL_PROVIDER_JS, GbSymbolProviderJs const))
#define GB_SYMBOL_PROVIDER_JS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SYMBOL_PROVIDER_JS, GbSymbolProviderJsClass))
#define GB_IS_SYMBOL_PROVIDER_JS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SYMBOL_PROVIDER_JS))
#define GB_IS_SYMBOL_PROVIDER_JS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SYMBOL_PROVIDER_JS))
#define GB_SYMBOL_PROVIDER_JS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SYMBOL_PROVIDER_JS, GbSymbolProviderJsClass))

typedef struct _GbSymbolProviderJs        GbSymbolProviderJs;
typedef struct _GbSymbolProviderJsClass   GbSymbolProviderJsClass;
typedef struct _GbSymbolProviderJsPrivate GbSymbolProviderJsPrivate;

struct _GbSymbolProviderJs
{
   GbSymbolProvider parent;

   /*< private >*/
   GbSymbolProviderJsPrivate *priv;
};

struct _GbSymbolProviderJsClass
{
   GbSymbolProviderClass parent_class;
};

GType gb_symbol_provider_js_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SYMBOL_PROVIDER_JS_H */

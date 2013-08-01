/* gb-language-js.h:
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

#ifndef GB_LANGUAGE_JS_H
#define GB_LANGUAGE_JS_H

#include "gb-language.h"

G_BEGIN_DECLS

#define GB_TYPE_LANGUAGE_JS            (gb_language_js_get_type())
#define GB_LANGUAGE_JS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE_JS, GbLanguageJs))
#define GB_LANGUAGE_JS_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE_JS, GbLanguageJs const))
#define GB_LANGUAGE_JS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_LANGUAGE_JS, GbLanguageJsClass))
#define GB_IS_LANGUAGE_JS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_LANGUAGE_JS))
#define GB_IS_LANGUAGE_JS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_LANGUAGE_JS))
#define GB_LANGUAGE_JS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_LANGUAGE_JS, GbLanguageJsClass))

typedef struct _GbLanguageJs        GbLanguageJs;
typedef struct _GbLanguageJsClass   GbLanguageJsClass;
typedef struct _GbLanguageJsPrivate GbLanguageJsPrivate;

struct _GbLanguageJs
{
   GbLanguage parent;

   /*< private >*/
   GbLanguageJsPrivate *priv;
};

struct _GbLanguageJsClass
{
   GbLanguageClass parent_class;
};

GType         gb_language_js_get_type (void) G_GNUC_CONST;
GbLanguageJs *gb_language_js_new      (void);

G_END_DECLS

#endif /* GB_LANGUAGE_JS_H */

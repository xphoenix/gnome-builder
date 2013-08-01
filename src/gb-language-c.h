/* gb-language-c.h:
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

#ifndef GB_LANGUAGE_C_H
#define GB_LANGUAGE_C_H

#include "gb-language.h"

G_BEGIN_DECLS

#define GB_TYPE_LANGUAGE_C            (gb_language_c_get_type())
#define GB_LANGUAGE_C(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE_C, GbLanguageC))
#define GB_LANGUAGE_C_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE_C, GbLanguageC const))
#define GB_LANGUAGE_C_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_LANGUAGE_C, GbLanguageCClass))
#define GB_IS_LANGUAGE_C(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_LANGUAGE_C))
#define GB_IS_LANGUAGE_C_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_LANGUAGE_C))
#define GB_LANGUAGE_C_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_LANGUAGE_C, GbLanguageCClass))

typedef struct _GbLanguageC        GbLanguageC;
typedef struct _GbLanguageCClass   GbLanguageCClass;
typedef struct _GbLanguageCPrivate GbLanguageCPrivate;

struct _GbLanguageC
{
   GbLanguage parent;

   /*< private >*/
   GbLanguageCPrivate *priv;
};

struct _GbLanguageCClass
{
   GbLanguageClass parent_class;
};

GType        gb_language_c_get_type (void) G_GNUC_CONST;
GbLanguageC *gb_language_c_new      (void);

G_END_DECLS

#endif /* GB_LANGUAGE_C_H */

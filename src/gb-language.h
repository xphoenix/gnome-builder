/* gb-language.h
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

#ifndef GB_LANGUAGE_H
#define GB_LANGUAGE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_LANGUAGE            (gb_language_get_type())
#define GB_LANGUAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE, GbLanguage))
#define GB_LANGUAGE_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE, GbLanguage const))
#define GB_LANGUAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_LANGUAGE, GbLanguageClass))
#define GB_IS_LANGUAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_LANGUAGE))
#define GB_IS_LANGUAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_LANGUAGE))
#define GB_LANGUAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_LANGUAGE, GbLanguageClass))

typedef struct _GbLanguage        GbLanguage;
typedef struct _GbLanguageClass   GbLanguageClass;
typedef struct _GbLanguagePrivate GbLanguagePrivate;

struct _GbLanguage
{
   GObject parent;

   /*< private >*/
   GbLanguagePrivate *priv;
};

struct _GbLanguageClass
{
   GObjectClass parent_class;
};

GType gb_language_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_LANGUAGE_H */

/* gb-language-formatter.h:
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

#ifndef GB_LANGUAGE_FORMATTER_H
#define GB_LANGUAGE_FORMATTER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_LANGUAGE_FORMATTER            (gb_language_formatter_get_type())
#define GB_LANGUAGE_FORMATTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE_FORMATTER, GbLanguageFormatter))
#define GB_LANGUAGE_FORMATTER_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_LANGUAGE_FORMATTER, GbLanguageFormatter const))
#define GB_LANGUAGE_FORMATTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_LANGUAGE_FORMATTER, GbLanguageFormatterClass))
#define GB_IS_LANGUAGE_FORMATTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_LANGUAGE_FORMATTER))
#define GB_IS_LANGUAGE_FORMATTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_LANGUAGE_FORMATTER))
#define GB_LANGUAGE_FORMATTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_LANGUAGE_FORMATTER, GbLanguageFormatterClass))

typedef struct _GbLanguageFormatter        GbLanguageFormatter;
typedef struct _GbLanguageFormatterClass   GbLanguageFormatterClass;
typedef struct _GbLanguageFormatterPrivate GbLanguageFormatterPrivate;

struct _GbLanguageFormatter
{
   GObject parent;

   /*< private >*/
   GbLanguageFormatterPrivate *priv;
};

struct _GbLanguageFormatterClass
{
   GObjectClass parent_class;
};

GType gb_language_formatter_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_LANGUAGE_FORMATTER_H */

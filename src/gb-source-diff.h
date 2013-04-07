/* gb-source-diff.h
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

#ifndef GB_SOURCE_DIFF_H
#define GB_SOURCE_DIFF_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GB_TYPE_SOURCE_DIFF            (gb_source_diff_get_type())
#define GB_SOURCE_DIFF(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_DIFF, GbSourceDiff))
#define GB_SOURCE_DIFF_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SOURCE_DIFF, GbSourceDiff const))
#define GB_SOURCE_DIFF_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SOURCE_DIFF, GbSourceDiffClass))
#define GB_IS_SOURCE_DIFF(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SOURCE_DIFF))
#define GB_IS_SOURCE_DIFF_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SOURCE_DIFF))
#define GB_SOURCE_DIFF_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SOURCE_DIFF, GbSourceDiffClass))

typedef struct _GbSourceDiff        GbSourceDiff;
typedef struct _GbSourceDiffClass   GbSourceDiffClass;
typedef struct _GbSourceDiffPrivate GbSourceDiffPrivate;

struct _GbSourceDiff
{
   GObject parent;

   /*< private >*/
   GbSourceDiffPrivate *priv;
};

struct _GbSourceDiffClass
{
   GObjectClass parent_class;
};

GType gb_source_diff_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SOURCE_DIFF_H */

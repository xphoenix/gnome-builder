/* gb-project-format.h
 *
 * Copyright (C) 2011 Christian Hergert <chris@dronelabs.com>
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

#ifndef GB_PROJECT_FORMAT_H
#define GB_PROJECT_FORMAT_H

#include <gio/gio.h>
#include <glib-object.h>

#include "gb-project.h"

G_BEGIN_DECLS

#define GB_TYPE_PROJECT_FORMAT            (gb_project_format_get_type())
#define GB_PROJECT_FORMAT_ERROR           (gb_project_format_error_quark())
#define GB_PROJECT_FORMAT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_FORMAT, GbProjectFormat))
#define GB_PROJECT_FORMAT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_PROJECT_FORMAT, GbProjectFormat const))
#define GB_PROJECT_FORMAT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_PROJECT_FORMAT, GbProjectFormatClass))
#define GB_IS_PROJECT_FORMAT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_PROJECT_FORMAT))
#define GB_IS_PROJECT_FORMAT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_PROJECT_FORMAT))
#define GB_PROJECT_FORMAT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_PROJECT_FORMAT, GbProjectFormatClass))

typedef struct _GbProjectFormat        GbProjectFormat;
typedef struct _GbProjectFormatClass   GbProjectFormatClass;
typedef struct _GbProjectFormatPrivate GbProjectFormatPrivate;
typedef enum   _GbProjectFormatError   GbProjectFormatError;

enum _GbProjectFormatError
{
   GB_PROJECT_FORMAT_ERROR_INVALID_JSON = 1,
};

struct _GbProjectFormat
{
   GObject parent;

   /*< private >*/
   GbProjectFormatPrivate *priv;
};

struct _GbProjectFormatClass
{
   GObjectClass parent_class;

   void       (*open_async)  (GbProjectFormat      *format,
                              const gchar          *directory,
                              GInputStream         *stream,
                              GCancellable         *cancellable,
                              GAsyncReadyCallback   callback,
                              gpointer              user_data);
   GbProject *(*open_finish) (GbProjectFormat      *format,
                              GAsyncResult         *result,
                              GError              **error);
   void       (*save_async)  (GbProjectFormat      *format,
                              GbProject            *project,
                              GOutputStream        *stream,
                              GCancellable         *cancellable,
                              GAsyncReadyCallback   callback,
                              gpointer              user_data);
   gboolean   (*save_finish) (GbProjectFormat      *format,
                              GAsyncResult         *result,
                              GError              **error);
};

GbProjectFormat *gb_project_format_new         (void);
GQuark           gb_project_format_error_quark (void) G_GNUC_CONST;
GType            gb_project_format_get_type    (void) G_GNUC_CONST;
void             gb_project_format_open_async  (GbProjectFormat      *format,
                                                const gchar          *directory,
                                                GInputStream         *stream,
                                                GCancellable         *cancellable,
                                                GAsyncReadyCallback   callback,
                                                gpointer              user_data);
GbProject       *gb_project_format_open_finish (GbProjectFormat      *format,
                                                GAsyncResult         *result,
                                                GError              **error);
void             gb_project_format_save_async  (GbProjectFormat      *format,
                                                GbProject            *project,
                                                GOutputStream        *stream,
                                                GCancellable         *cancellable,
                                                GAsyncReadyCallback   callback,
                                                gpointer              user_data);
gboolean         gb_project_format_save_finish (GbProjectFormat      *format,
                                                GAsyncResult         *result,
                                                GError              **error);

G_END_DECLS

#endif /* GB_PROJECT_FORMAT_H */

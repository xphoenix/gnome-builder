/* ide-extension-point.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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

#ifndef IDE_EXTENSION_POINT_H
#define IDE_EXTENSION_POINT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define IDE_TYPE_EXTENSION_POINT (ide_extension_point_get_type())

G_DECLARE_FINAL_TYPE (IdeExtensionPoint, ide_extension_point, IDE, EXTENSION_POINT, GObject)

void               ide_extension_point_register (GType        interface_type,
                                                 const gchar *match_key,
                                                 const gchar *priority_key);
IdeExtensionPoint *ide_extension_point_lookup   (GType        interface_type);
gpointer           ide_extension_point_create   (GType        interface_type,
                                                 const gchar *match_key,
                                                 const gchar *first_property,
                                                 ...);

G_END_DECLS

#endif /* IDE_EXTENSION_POINT_H */

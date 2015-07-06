/* ide-plugin-adapter.h
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

#ifndef IDE_PLUGIN_ADAPTER_H
#define IDE_PLUGIN_ADAPTER_H

#include <libpeas/peas.h>

#include "ide-object.h"

G_BEGIN_DECLS

#define IDE_TYPE_PLUGIN_ADAPTER (ide_plugin_adapter_get_type())

G_DECLARE_FINAL_TYPE (IdePluginAdapter, ide_plugin_adapter, IDE, PLUGIN_ADAPTER, IdeObject)

IdePluginAdapter *ide_plugin_adapter_new                (IdeContext       *context,
                                                         PeasEngine       *engine,
                                                         GType             interface_type,
                                                         const gchar      *match_key);
PeasEngine       *ide_plugin_adapter_get_engine         (IdePluginAdapter *self);
GType             ide_plugin_adapter_get_interface_type (IdePluginAdapter *self);
const gchar      *ide_plugin_adapter_get_match_key      (IdePluginAdapter *self);
const gchar      *ide_plugin_adapter_get_match_value    (IdePluginAdapter *self);
void              ide_plugin_adapter_set_match_value    (IdePluginAdapter *self,
                                                         const gchar      *match_key);
gpointer          ide_plugin_adapter_get_extension      (IdePluginAdapter *self);

G_END_DECLS

#endif /* IDE_PLUGIN_ADAPTER_H */

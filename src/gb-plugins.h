/* gb-plugin.h
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

#ifndef GB_PLUGIN_H
#define GB_PLUGIN_H

#include <libpeas/peas.h>

#include "gconstructor.h"

G_BEGIN_DECLS

#define GB_DEFINE_EMBEDDED_PLUGIN(name, resource, plugin_path, ...)    \
  static PeasPluginInfo *                                              \
  name##_register (PeasObjectModule *module)                           \
  {                                                                    \
    g_resources_register (resource);                                   \
    __VA_ARGS__                                                        \
    return peas_plugin_info_new_embedded (module, plugin_path);        \
  }                                                                    \
                                                                       \
  G_DEFINE_CONSTRUCTOR(name##_plugin);                                 \
  static void                                                          \
  name##_plugin (void)                                                 \
  {                                                                    \
    gb_plugins_register (name##_register);                             \
  }

#define GB_DEFINE_PLUGIN_TYPE(PLUGIN, IMPL)                            \
  G_STMT_START {                                                       \
    peas_object_module_register_extension_type (module, PLUGIN, IMPL); \
  } G_STMT_END;

typedef PeasPluginInfo *(*GbPluginRegisterFunc) (PeasObjectModule *module);

void gb_plugins_register (GbPluginRegisterFunc callback);
void gb_plugins_load     (void);

G_END_DECLS

#endif /* GB_PLUGIN_H */
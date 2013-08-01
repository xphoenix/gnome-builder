/* gb-search-provider.c:
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

#include "gb-search-provider.h"

void
gb_search_provider_focus_search (GbSearchProvider *provider)
{
   GbSearchProviderIface *iface;

   g_return_if_fail(GB_IS_SEARCH_PROVIDER(provider));

   iface = GB_SEARCH_PROVIDER_GET_INTERFACE(provider);
   if (iface->focus_search) {
      iface->focus_search(provider);
   }
}

GType
gb_search_provider_get_type (void)
{
   static GType type_id = 0;

   if (g_once_init_enter((gsize *)&type_id)) {
      GType _type_id;
      const GTypeInfo g_type_info = {
         sizeof(GbSearchProviderIface),
         NULL, /* base_init */
         NULL, /* base_finalize */
         NULL, /* class_init */
         NULL, /* class_finalize */
         NULL, /* class_data */
         0,    /* instance_size */
         0,    /* n_preallocs */
         NULL, /* instance_init */
         NULL  /* value_vtable */
      };

      _type_id = g_type_register_static(G_TYPE_INTERFACE,
                                        "GbSearchProvider",
                                        &g_type_info,
                                        0);
      g_type_interface_add_prerequisite(_type_id, G_TYPE_OBJECT);
      g_once_init_leave((gsize *)&type_id, _type_id);
   }

   return type_id;
}

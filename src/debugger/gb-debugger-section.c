/* gb-debugger-section.c
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

#include <glib/gi18n.h>

#include "gb-debugger-section.h"

struct _GbDebuggerSectionPrivate
{
   gpointer dummy;
};

G_DEFINE_TYPE_WITH_CODE(GbDebuggerSection,
                        gb_debugger_section,
                        GB_TYPE_WORKSPACE_SECTION,
                        G_ADD_PRIVATE(GbDebuggerSection))

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_debugger_section_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_debugger_section_parent_class)->finalize(object);
}

static void
gb_debugger_section_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
   //GbDebuggerSection *section = GB_DEBUGGER_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_debugger_section_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
   //GbDebuggerSection *section = GB_DEBUGGER_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_debugger_section_class_init (GbDebuggerSectionClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_debugger_section_finalize;
   object_class->get_property = gb_debugger_section_get_property;
   object_class->set_property = gb_debugger_section_set_property;
}

static void
gb_debugger_section_init (GbDebuggerSection *section)
{
   section->priv = gb_debugger_section_get_instance_private(section);
}

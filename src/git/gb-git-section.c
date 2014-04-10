/* gb-git-section.c
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

#include "gb-git-section.h"

struct _GbGitSectionPrivate
{
   gpointer dummy;
};

G_DEFINE_TYPE_WITH_CODE(GbGitSection,
                        gb_git_section,
                        GB_TYPE_WORKSPACE_SECTION,
                        G_ADD_PRIVATE(GbGitSection))

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

static void
gb_git_section_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_git_section_parent_class)->finalize(object);
}

static void
gb_git_section_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   //GbGitSection *section = GB_GIT_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_git_section_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   //GbGitSection *section = GB_GIT_SECTION(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_git_section_class_init (GbGitSectionClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_git_section_finalize;
   object_class->get_property = gb_git_section_get_property;
   object_class->set_property = gb_git_section_set_property;
}

static void
gb_git_section_init (GbGitSection *section)
{
   section->priv = gb_git_section_get_instance_private(section);
}

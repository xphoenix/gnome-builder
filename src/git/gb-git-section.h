/* gb-git-section.h
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

#ifndef GB_GIT_SECTION_H
#define GB_GIT_SECTION_H

#include "gb-workspace-section.h"

G_BEGIN_DECLS

#define GB_TYPE_GIT_SECTION            (gb_git_section_get_type())
#define GB_GIT_SECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_GIT_SECTION, GbGitSection))
#define GB_GIT_SECTION_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_GIT_SECTION, GbGitSection const))
#define GB_GIT_SECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_GIT_SECTION, GbGitSectionClass))
#define GB_IS_GIT_SECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_GIT_SECTION))
#define GB_IS_GIT_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_GIT_SECTION))
#define GB_GIT_SECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_GIT_SECTION, GbGitSectionClass))

typedef struct _GbGitSection        GbGitSection;
typedef struct _GbGitSectionClass   GbGitSectionClass;
typedef struct _GbGitSectionPrivate GbGitSectionPrivate;

struct _GbGitSection
{
   GbWorkspaceSection parent;

   /*< private >*/
   GbGitSectionPrivate *priv;
};

struct _GbGitSectionClass
{
   GbWorkspaceSectionClass parent_class;
};

GType gb_git_section_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_GIT_SECTION_H */

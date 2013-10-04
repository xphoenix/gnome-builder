/* gb-tree-builder-project.h
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

#ifndef GB_TREE_BUILDER_PROJECT_H
#define GB_TREE_BUILDER_PROJECT_H

#include "gb-tree-builder.h"

G_BEGIN_DECLS

#define GB_TYPE_TREE_BUILDER_PROJECT            (gb_tree_builder_project_get_type())
#define GB_TREE_BUILDER_PROJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TREE_BUILDER_PROJECT, GbTreeBuilderProject))
#define GB_TREE_BUILDER_PROJECT_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_TREE_BUILDER_PROJECT, GbTreeBuilderProject const))
#define GB_TREE_BUILDER_PROJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_TREE_BUILDER_PROJECT, GbTreeBuilderProjectClass))
#define GB_IS_TREE_BUILDER_PROJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_TREE_BUILDER_PROJECT))
#define GB_IS_TREE_BUILDER_PROJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_TREE_BUILDER_PROJECT))
#define GB_TREE_BUILDER_PROJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_TREE_BUILDER_PROJECT, GbTreeBuilderProjectClass))

typedef struct _GbTreeBuilderProject        GbTreeBuilderProject;
typedef struct _GbTreeBuilderProjectClass   GbTreeBuilderProjectClass;
typedef struct _GbTreeBuilderProjectPrivate GbTreeBuilderProjectPrivate;

struct _GbTreeBuilderProject
{
	GbTreeBuilder parent;

	/*< private >*/
	GbTreeBuilderProjectPrivate *priv;
};

struct _GbTreeBuilderProjectClass
{
	GbTreeBuilderClass parent_class;
};

GType          gb_tree_builder_project_get_type (void) G_GNUC_CONST;
GbTreeBuilder *gb_tree_builder_project_new      (void);

G_END_DECLS

#endif /* GB_TREE_BUILDER_PROJECT_H */

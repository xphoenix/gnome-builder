/* gb-tree-node-project.c
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

#include <glib/gi18n.h>

#include "gb-project.h"
#include "gb-tree-node-project.h"

G_DEFINE_TYPE(GbTreeNodeProject, gb_tree_node_project, GB_TYPE_TREE_NODE)

struct _GbTreeNodeProjectPrivate
{
	GbProject *project;
};

enum
{
	PROP_0,
	PROP_PROJECT,
	LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

/**
 * gb_tree_node_project_set_project:
 * @project: (in): A #GbTreeNodeProject.
 *
 * Sets the project property for the #GbTreeNodeProject.
 */
static void
gb_tree_node_project_set_project (GbTreeNodeProject *node,
                                  GbProject         *project)
{
	GbTreeNodeProjectPrivate *priv;

	g_return_if_fail(GB_IS_TREE_NODE_PROJECT(node));
	g_return_if_fail(GB_IS_PROJECT(project));

	priv = node->priv;

	g_clear_object(&priv->project);
	priv->project = g_object_ref(project);
	g_object_notify_by_pspec(G_OBJECT(node), gParamSpecs[PROP_PROJECT]);
}

/**
 * gb_tree_node_project_finalize:
 * @object: (in): A #GbTreeNodeProject.
 *
 * Finalizer for a #GbTreeNodeProject instance.  Frees any resources held by
 * the instance.
 */
static void
gb_tree_node_project_finalize (GObject *object)
{
	GbTreeNodeProjectPrivate *priv = GB_TREE_NODE_PROJECT(object)->priv;

	g_clear_object(&priv->project);

	G_OBJECT_CLASS(gb_tree_node_project_parent_class)->finalize(object);
}

/**
 * gb_tree_node_project_get_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
gb_tree_node_project_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
	GbTreeNodeProject *project = GB_TREE_NODE_PROJECT(object);

	switch (prop_id) {
	case PROP_PROJECT:
		g_value_set_object(value, project->priv->project);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

/**
 * gb_tree_node_project_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_tree_node_project_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	GbTreeNodeProject *project = GB_TREE_NODE_PROJECT(object);

	switch (prop_id) {
	case PROP_PROJECT:
		gb_tree_node_project_set_project(project, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

/**
 * gb_tree_node_project_class_init:
 * @klass: (in): A #GbTreeNodeProjectClass.
 *
 * Initializes the #GbTreeNodeProjectClass and prepares the vtable.
 */
static void
gb_tree_node_project_class_init (GbTreeNodeProjectClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = gb_tree_node_project_finalize;
	object_class->get_property = gb_tree_node_project_get_property;
	object_class->set_property = gb_tree_node_project_set_property;
	g_type_class_add_private(object_class, sizeof(GbTreeNodeProjectPrivate));

	gParamSpecs[PROP_PROJECT] =
		g_param_spec_object("project",
		                    _("Project"),
		                    _("The nodes project."),
		                    GB_TYPE_PROJECT,
		                    G_PARAM_READWRITE);
	g_object_class_install_property(object_class, PROP_PROJECT,
	                                gParamSpecs[PROP_PROJECT]);
}

/**
 * gb_tree_node_project_init:
 * @project: (in): A #GbTreeNodeProject.
 *
 * Initializes the newly created #GbTreeNodeProject instance.
 */
static void
gb_tree_node_project_init (GbTreeNodeProject *project)
{
	project->priv =
		G_TYPE_INSTANCE_GET_PRIVATE(project,
		                            GB_TYPE_TREE_NODE_PROJECT,
		                            GbTreeNodeProjectPrivate);
}

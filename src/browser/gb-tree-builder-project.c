/* gb-tree-builder-project.c
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

#include <gio/gio.h>
#include <glib/gi18n.h>

#include "gb-log.h"
#include "gb-path.h"
#include "gb-project.h"
#include "gb-project-file.h"
#include "gb-project-group.h"
#include "gb-project-item.h"
#include "gb-project-target.h"
#include "gb-source-pane.h"
#include "gb-tree.h"
#include "gb-tree-builder-project.h"
#include "gb-tree-node-project.h"
#include "gb-workspace.h"
#include "gb-workspace-pane.h"

G_DEFINE_TYPE(GbTreeBuilderProject,
              gb_tree_builder_project,
              GB_TYPE_TREE_BUILDER)

struct _GbTreeBuilderProjectPrivate
{
   GtkActionGroup *actions;
};

static const gchar gActionUi[] = "<ui>"
                                 " <popup name=\"popup\">"
                                 "  <menuitem action=\"open-project-file\"/>"
                                 " </popup>"
                                 "</ui>";

GbTreeBuilder *
gb_tree_builder_project_new (void)
{
   GbTreeBuilder *ret;

   ENTRY;
   ret = g_object_new(GB_TYPE_TREE_BUILDER_PROJECT, NULL);
   RETURN(ret);
}

static void
gb_tree_builder_project_added (GbTreeBuilder *builder,
                               GtkWidget     *tree)
{
   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));
   g_return_if_fail(GB_IS_TREE(tree));

#if 0
   GbTreeBuilderProjectPrivate *priv = builder->priv;
   ui_manager = gb_tree_get_menu_ui(GB_TREE(tree));
   g_assert(GTK_IS_UI_MANAGER(ui_manager));
   gtk_ui_manager_insert_action_group(ui_manager, priv->actions, 100);
   gtk_ui_manager_add_ui_from_string(ui_manager, gActionUi, -1, NULL);
#endif
}

/**
 * binding_transform_basename:
 * @binding: (in): A #GBinding.
 * @value: (in): A #GValue.
 * @target_value: (out): A #GValue for the result.
 *
 * GBindingTransformFunc that ransforms a path into its basename.
 *
 * Returns: %TRUE.
 */
static gboolean
binding_transform_basename (GBinding     *binding,
                            const GValue *value,
                            GValue       *target_value,
                            gpointer      user_data)
{
   //const gchar *src = g_value_get_string(value);
   //g_value_take_string(target_value, g_path_get_basename(src));
   /*
    * TODO: Possibly some elipsize?
    */
   g_value_set_string(target_value, g_value_get_string(value));
   return TRUE;
}


/**
 * gb_tree_builder_project_build_project:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle building #GbTreeNode that contains a #GbProject.
 */
static void
gb_tree_builder_project_build_project (GbTreeBuilder *builder,
                                       GbTreeNode    *node)
{
   GbProjectGroup *files;
   GbProjectGroup *targets;
   GbTreeNode *child;
   GbProject *project = NULL;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));
   g_return_if_fail(GB_IS_TREE_NODE(node));

   g_object_get(node, "project", &project, NULL);
   g_assert(!project || GB_IS_PROJECT(project));

   if (project) {
      /*
       * Targets node.
       */
      targets = gb_project_get_targets(project);
      child = g_object_new(GB_TYPE_TREE_NODE,
                           "icon-name", "folder",
                           "item", targets,
                           "text", _("Targets"),
                           NULL);
      gb_tree_node_append(node, child);

      files = gb_project_get_files(project);
      child = g_object_new(GB_TYPE_TREE_NODE,
                           "icon-name", "folder",
                           "item", files,
                           "text", _("Files"),
                           NULL);
      gb_tree_node_append(node, child);

      /*
       * Resources node.
       */
#if 0
      child = g_object_new(GB_TYPE_TREE_NODE,
                           "icon-name", "folder",
                           "text", _("Resources"),
                           NULL);
      g_object_bind_property(project, "resources", child, "item",
                             G_BINDING_SYNC_CREATE);
      gb_tree_node_append(node, child);
#endif
   }

   g_clear_object(&project);
}

/**
 * gb_tree_builder_project_build_targets:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle building #GbTreeNode that contains a #GbProjectGroup of
 * #GbTarget.
 */
static void
gb_tree_builder_project_build_targets (GbTreeBuilder *builder,
                                       GbTreeNode    *node)
{
   GbProjectGroup *targets;
   GbProjectItem *target;
   GbTreeNode *child;
   gint length;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));
   g_return_if_fail(GB_IS_TREE_NODE(node));

   targets = GB_PROJECT_GROUP(gb_tree_node_get_item(node));
   length = gb_project_group_get_count(targets);
   for (i = 0; i < length; i++) {
      target = gb_project_group_get_item(targets, i);
      child = g_object_new(GB_TYPE_TREE_NODE,
                           "icon-name", "gb-project",
                           "item", target,
                           NULL);
      g_object_bind_property(target, "name", child, "text",
                             G_BINDING_SYNC_CREATE);
      gb_tree_node_append(node, child);
   }

   EXIT;
}

/**
 * gb_tree_builder_project_build_target:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle building a #GbTarget node.
 */
static void
gb_tree_builder_project_build_target (GbTreeBuilder *builder,
                                      GbTreeNode    *node)
{
   GbProjectGroup *files;
   GbProjectItem *target;
   GbTreeNode *child;

   ENTRY;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));
   g_return_if_fail(GB_IS_TREE_NODE(node));

   target = gb_tree_node_get_item(node);
   g_assert(GB_IS_PROJECT_TARGET(target));

   files = gb_project_target_get_files(GB_PROJECT_TARGET(target));
   g_assert(GB_IS_PROJECT_GROUP(files));

   child = g_object_new(GB_TYPE_TREE_NODE,
                        "icon-name", "folder",
                        "text", _("Dependencies"),
                        NULL);
   gb_tree_node_append(node, child);

   child = g_object_new(GB_TYPE_TREE_NODE,
                        "icon-name", "folder",
                        "item", files,
                        "text", _("Files"),
                        NULL);
   g_object_set_data(G_OBJECT(child), "files", GINT_TO_POINTER(TRUE));
   gb_tree_node_append(node, child);

   EXIT;
}

#if 0
static gboolean
is_header (GbProjectItem *item)
{
   GbProjectFileMode mode;
   gboolean ret = FALSE;
   GFile *f;
   gchar *uri;

   mode = gb_project_file_get_mode(GB_PROJECT_FILE(item));
   if (mode != GB_PROJECT_FILE_SOURCE) {
      return FALSE;
   }

   g_object_get(item, "file", &f, NULL);
   uri = g_file_get_uri(f);
   ret = g_str_has_suffix(uri, ".h") ||
         g_str_has_suffix(uri, ".hh") ||
         g_str_has_suffix(uri, ".hpp");
   g_object_unref(f);
   g_free(uri);

   return ret;
}

static gboolean
is_source (GbProjectItem *item)
{
   GbProjectFileMode mode;
   gboolean ret = FALSE;
   GFile *f;
   gchar *uri;

   mode = gb_project_file_get_mode(GB_PROJECT_FILE(item));
   if (mode != GB_PROJECT_FILE_SOURCE) {
      return FALSE;
   }

   g_object_get(item, "file", &f, NULL);
   uri = g_file_get_uri(f);
   ret = g_str_has_suffix(uri, ".c") ||
         g_str_has_suffix(uri, ".cc") ||
         g_str_has_suffix(uri, ".cxx") ||
         g_str_has_suffix(uri, ".cpp") ||
         g_str_has_suffix(uri, ".py");
   g_object_unref(f);
   g_free(uri);

   return ret;
}
#endif

/**
 * gb_tree_builder_project_build_files:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle building #GbTreeNode that contains a #GbProjectGroup of
 * #GbProjectFile.
 */
static void
gb_tree_builder_project_build_files (GbTreeBuilder *builder,
                                     GbTreeNode    *node)
{
   GbProjectGroup *files;
   GbProjectItem *file;
   const gchar *icon_name;
   GbTreeNode *child;
   gchar *path;
   gint length;
   gint i;

   ENTRY;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));
   g_return_if_fail(GB_IS_TREE_NODE(node));

   files = GB_PROJECT_GROUP(gb_tree_node_get_item(node));
   length = gb_project_group_get_count(files);
   for (i = 0; i < length; i++) {
      file = gb_project_group_get_item(files, i);
      path = g_file_get_path(gb_project_file_get_file(GB_PROJECT_FILE(file)));
      if (!(icon_name = gb_path_get_icon_name(path))) {
         icon_name = "text-x-generic";
      }
      g_free(path);
      child = g_object_new(GB_TYPE_TREE_NODE,
                           "icon-name", icon_name,
                           "item", file,
                           NULL);
      g_object_bind_property_full(file, "path",
                                  child, "text",
                                  G_BINDING_SYNC_CREATE,
                                  binding_transform_basename, NULL,
                                  NULL, NULL);
      gb_tree_node_append(node, child);
   }

   EXIT;
}

/**
 * gb_tree_builder_project_build_node:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle building a given node if we know how.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_tree_builder_project_build_node (GbTreeBuilder *builder,
                                    GbTreeNode    *node)
{
   GbProjectItem *item = NULL;
   GType item_type;

   ENTRY;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));
   g_return_if_fail(GB_IS_TREE_NODE(node));

   item = gb_tree_node_get_item(node);

   if (GB_IS_TREE_NODE_PROJECT(node)) {
      gb_tree_builder_project_build_project(builder, node);
   } else if (item) {
      if (GB_IS_PROJECT_GROUP(item)) {
         item_type = gb_project_group_get_item_type(GB_PROJECT_GROUP(item));
         if (g_type_is_a(item_type, GB_TYPE_PROJECT_TARGET)) {
            gb_tree_builder_project_build_targets(builder, node);
         } else if (g_type_is_a(item_type, GB_TYPE_PROJECT_FILE)) {
            gb_tree_builder_project_build_files(builder, node);
         }
      } else if (GB_IS_PROJECT_TARGET(item)) {
         gb_tree_builder_project_build_target(builder, node);
      }
   }

   EXIT;
}

/**
 * gb_tree_builder_project_node_activated:
 * @builder: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle the activation of a tree node. If it is a node format we
 * know about, try and do the right thing.
 *
 * Returns: %TRUE if the activation was handled, otherwise %FALSE.
 */
static gboolean
gb_tree_builder_project_node_activated (GbTreeBuilder *builder,
                                        GbTreeNode    *node)
{
   return TRUE;
}

/**
 * gb_tree_builder_project_node_selected:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle the node being selected. Update actions if necessary.
 */
static void
gb_tree_builder_project_node_selected (GbTreeBuilder *builder,
                                       GbTreeNode    *node)
{
#if 0
   GbTreeBuilderProjectPrivate *priv;
   GbProjectItem *item;
   gboolean can_open;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));

   priv = GB_TREE_BUILDER_PROJECT(builder)->priv;

   item = gb_tree_node_get_item(node);
   can_open = (item && GB_IS_PROJECT_FILE(item));

#define SET(name, ...) \
   g_object_set(gtk_action_group_get_action(priv->actions, name), \
                __VA_ARGS__)
   SET("open-project-file", "visible", can_open, NULL);
#undef SET
#endif
}

/**
 * gb_tree_builder_project_node_unselected:
 * @project: (in): A #GbTreeBuilderProject.
 * @node: (in): A #GbTreeNode.
 *
 * Handle the node being unselected. Update actions if necessary.
 */
static void
gb_tree_builder_project_node_unselected (GbTreeBuilder *builder,
                                         GbTreeNode    *node)
{
}

#if 0
static void
gb_tree_builder_project_open (GtkAction     *action,
                              GbTreeBuilder *builder)
{
   GbWorkspacePane *pane;
   GbProjectItem *item;
   GbTreeNode *node;
   GtkWidget *toplevel;
   GbTree *tree;
   GFile *file;

   g_return_if_fail(GB_IS_TREE_BUILDER_PROJECT(builder));

   tree = GB_TREE(gb_tree_builder_get_tree(builder));
   node = gb_tree_get_selected(tree);
   item = gb_tree_node_get_item(node);

   /*
    * TODO: We should really use a plugin to find what views
    *       can open a given file. That way we don't duplicate
    *       this file open crap everywhere.
    */

   file = gb_project_file_get_file(GB_PROJECT_FILE(item));
   pane = g_object_new(GB_TYPE_SOURCE_PANE,
                       "file", file,
                       "visible", TRUE,
                       NULL);
   g_object_unref(file);

   /*
    * Get the workspace and add it to it.
    */
   toplevel = gtk_widget_get_toplevel(GTK_WIDGET(tree));
   if (GB_IS_WORKSPACE(toplevel)) {
      gtk_container_add(GTK_CONTAINER(toplevel),
                        GTK_WIDGET(pane));
   } else {
      g_object_unref(pane);
   }
}
#endif

static void
gb_tree_builder_project_finalize (GObject *object)
{
   GbTreeBuilderProjectPrivate *priv;

   ENTRY;

   priv = GB_TREE_BUILDER_PROJECT(object)->priv;

   g_clear_object(&priv->actions);

   G_OBJECT_CLASS(gb_tree_builder_project_parent_class)->finalize(object);

   EXIT;
}

/**
 * gb_tree_builder_project_class_init:
 * @klass: (in): A #GbTreeBuilderProjectClass.
 *
 * Initializes the #GbTreeBuilderProjectClass and prepares the vtable.
 */
static void
gb_tree_builder_project_class_init (GbTreeBuilderProjectClass *klass)
{
   GObjectClass *object_class;
   GbTreeBuilderClass *builder_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_tree_builder_project_finalize;
   g_type_class_add_private(object_class, sizeof(GbTreeBuilderProjectPrivate));

   builder_class = GB_TREE_BUILDER_CLASS(klass);
   builder_class->added = gb_tree_builder_project_added;
   builder_class->build_node = gb_tree_builder_project_build_node;
   builder_class->node_activated = gb_tree_builder_project_node_activated;
   builder_class->node_selected = gb_tree_builder_project_node_selected;
   builder_class->node_unselected = gb_tree_builder_project_node_unselected;
}

/**
 * gb_tree_builder_project_init:
 * @project: (in): A #GbTreeBuilderProject.
 *
 * Initializes the newly created #GbTreeBuilderProject instance.
 */
static void
gb_tree_builder_project_init (GbTreeBuilderProject *project)
{
#if 0
   static const GtkActionEntry actions[] = {
      { "open-project-file", GTK_STOCK_OPEN, N_("Open File"), NULL, NULL,
        G_CALLBACK(gb_tree_builder_project_open) },
   };
#endif

   project->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(project,
                                  GB_TYPE_TREE_BUILDER_PROJECT,
                                  GbTreeBuilderProjectPrivate);

#if 0
   priv->actions = g_object_new(GTK_TYPE_ACTION_GROUP,
                                "name", "GbTreeBuilderProject::actions",
                                NULL);

   gtk_action_group_add_actions(priv->actions, actions,
                                G_N_ELEMENTS(actions),
                                project);
#endif
}

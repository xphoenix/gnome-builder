/* gb-project-item.c
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
#include <json-glib/json-glib.h>

#include "gb-log.h"
#include "gb-project.h"
#include "gb-project-group.h"
#include "gb-project-item.h"
#include "gb-project-target.h"

static void json_serializable_init (gpointer);

G_DEFINE_TYPE_EXTENDED(GbProjectItem,
                       gb_project_item,
                       G_TYPE_INITIALLY_UNOWNED,
                       0,
                       G_IMPLEMENT_INTERFACE(JSON_TYPE_SERIALIZABLE,
                                             json_serializable_init))

struct _GbProjectItemPrivate
{
   GbProjectItem *parent;
};

enum
{
   PROP_0,
   PROP_PARENT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

/**
 * gb_project_item_get_toplevel:
 * @item: (in): A #GbProjectItem.
 *
 * Gets the top-most #GbProjectItem in the tree. This should typically be
 * a #GbProject instance.
 *
 * Returns: (transfer none): A #GbProjectItem.
 */
GbProjectItem *
gb_project_item_get_toplevel (GbProjectItem *item)
{
   g_return_val_if_fail(GB_IS_PROJECT_ITEM(item), NULL);

   while (item->priv->parent) { item = item->priv->parent; }
   return item;
}

/**
 * gb_project_item_get_parent:
 * @item: (in): A #GbProjectItem.
 *
 * Gets the parent project item.
 *
 * Returns: (transfer none): A #GbProjectItem.
 */
GbProjectItem *
gb_project_item_get_parent (GbProjectItem *item)
{
   g_return_val_if_fail(GB_IS_PROJECT_ITEM(item), NULL);
   return item->priv->parent;
}

/**
 * gb_project_item_set_parent:
 * @item: (in): A #GbProjectItem.
 * @parent: (in): A #GbProjectItem.
 *
 * Sets the parent project item. The top-most project item should be
 * the project.
 */
static void
gb_project_item_set_parent (GbProjectItem *item,
                            GbProjectItem *parent)
{
   GbProjectItemPrivate *priv;
   gpointer instance;

   ENTRY;

   priv = item->priv;

   if ((instance = priv->parent)) {
      priv->parent = NULL;
      g_object_remove_weak_pointer(instance, (gpointer *)&priv->parent);
   }

   if (parent) {
      priv->parent = parent;
      g_object_add_weak_pointer(G_OBJECT(parent), (gpointer *)&priv->parent);
      g_object_notify_by_pspec(G_OBJECT(item), gParamSpecs[PROP_PARENT]);
   }

   EXIT;
}

/**
 * gb_project_item_finalize:
 * @object: (in): A #GbProjectItem.
 *
 * Disposer for a #GbProjectItem instance. Frees any related objects.
 */
static void
gb_project_item_dispose (GObject *object)
{
   ENTRY;
   gb_project_item_set_parent((GbProjectItem *)object, NULL);
   G_OBJECT_CLASS(gb_project_item_parent_class)->dispose(object);
   EXIT;
}

/**
 * gb_project_item_finalize:
 * @object: (in): A #GbProjectItem.
 *
 * Finalizer for a #GbProjectItem instance.  Frees any resources held by
 * the instance.
 */
static void
gb_project_item_finalize (GObject *object)
{
   ENTRY;
   gb_project_item_set_parent(GB_PROJECT_ITEM(object), NULL);
   G_OBJECT_CLASS(gb_project_item_parent_class)->finalize(object);
   EXIT;
}

/**
 * gb_project_item_get_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
gb_project_item_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
   GbProjectItem *item = GB_PROJECT_ITEM(object);

   switch (prop_id) {
   case PROP_PARENT:
      g_value_set_object(value, gb_project_item_get_parent(item));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_item_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_project_item_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
   GbProjectItem *item = GB_PROJECT_ITEM(object);

   switch (prop_id) {
   case PROP_PARENT:
      gb_project_item_set_parent(item, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_item_class_init:
 * @klass: (in): A #GbProjectItemClass.
 *
 * Initializes the #GbProjectItemClass and prepares the vtable.
 *
 * Returns: None.
 * Side effects: None.
 */
static void
gb_project_item_class_init (GbProjectItemClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_project_item_dispose;
   object_class->finalize = gb_project_item_finalize;
   object_class->get_property = gb_project_item_get_property;
   object_class->set_property = gb_project_item_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectItemPrivate));

   /**
    * GbProjectItem:parent:
    *
    * The parent #GbProjectItem.
    */
   gParamSpecs[PROP_PARENT] =
      g_param_spec_object("parent",
                          _("Parent"),
                          _("The parent project item."),
                          GB_TYPE_PROJECT_ITEM,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_PARENT,
                                   gParamSpecs[PROP_PARENT]);
}

/**
 * gb_project_item_init:
 * @item: (in): A #GbProjectItem.
 *
 * Initializes the newly created #GbProjectItem instance.
 */
static void
gb_project_item_init (GbProjectItem *item)
{
   ENTRY;
   item->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(item,
                                  GB_TYPE_PROJECT_ITEM,
                                  GbProjectItemPrivate);
   EXIT;
}

GQuark
gb_project_item_error_quark (void)
{
   return g_quark_from_static_string("gb-project-item-error-quark");
}

/**
 * gb_project_item_resolve_type:
 * @name: (in): A GType name.
 *
 * Tries to resolve a type from a "GObject" type name.
 *
 * This function is taken from Gtk+. See
 * http://git.gnome.org/browse/gtk+/tree/gtk/gtkbuilder.c.
 *
 * Returns: A #GType.
 */
GType
gb_project_item_resolve_type (const gchar *name)
{
   static GModule *module = NULL;
   GType (*func) () = NULL;
   GString *symbol_name;
   char c, *symbol;
   int i;
   GType gtype = G_TYPE_INVALID;

   ENTRY;

   g_return_val_if_fail(name != NULL, G_TYPE_NONE);

   if ((gtype = g_type_from_name(name))) {
      RETURN(gtype);
   }

   symbol_name = g_string_new ("");

   if (!module) {
      module = g_module_open (NULL, 0);
   }

   for (i = 0; name[i] != '\0'; i++) {
      c = name[i];
      /* skip if uppercase, first or previous is uppercase */
      if ((c == g_ascii_toupper (c) &&
           i > 0 && name[i-1] != g_ascii_toupper (name[i-1])) ||
          (i > 2 && name[i]   == g_ascii_toupper (name[i]) &&
           name[i-1] == g_ascii_toupper (name[i-1]) &&
           name[i-2] == g_ascii_toupper (name[i-2]))) {
         g_string_append_c (symbol_name, '_');
      }
      g_string_append_c (symbol_name, g_ascii_tolower (c));
   }
   g_string_append (symbol_name, "_get_type");

   symbol = g_string_free (symbol_name, FALSE);

   if (g_module_symbol (module, symbol, (gpointer)&func)) {
      gtype = func ();
   }

   g_free (symbol);

   RETURN(gtype);
}

static gboolean
gb_project_item_deserialize_property (JsonSerializable *serializable,
                                      const gchar      *prop_name,
                                      GValue           *value,
                                      GParamSpec       *pspec,
                                      JsonNode         *node)
{
   GbProjectGroup *group;
   GbProjectItem *item = (GbProjectItem *)serializable;
   GbProjectItem *child;
   const gchar *type_name;
   JsonObject *element;
   JsonArray *array;
   JsonNode *element_node;
   gboolean ret = FALSE;
   GType type;
   guint length;
   guint i;

   ENTRY;

   g_return_val_if_fail(GB_IS_PROJECT_ITEM(item), FALSE);
   g_return_val_if_fail(prop_name != NULL, FALSE);
   g_return_val_if_fail(value != NULL, FALSE);
   g_return_val_if_fail(pspec != NULL, FALSE);

   if (G_VALUE_HOLDS(value, GB_TYPE_PROJECT_GROUP)) {
      group = g_object_new(GB_TYPE_PROJECT_GROUP,
                           "item-type", GB_TYPE_PROJECT_ITEM,
                           "parent", item,
                           NULL);
      array = json_node_get_array(node);
      length = json_array_get_length(array);
      for (i = 0; i < length; i++) {
         element_node = json_array_get_element(array, i);
         element = json_node_get_object(element_node);
         type_name = json_object_get_string_member(element, "type");
         type = gb_project_item_resolve_type(type_name);
         child = GB_PROJECT_ITEM(json_gobject_deserialize(type, element_node));
         gb_project_group_append(group, child);
      }
      g_value_set_object(value, group);
      ret = TRUE;
   } else {
      ret = json_serializable_default_deserialize_property(serializable,
                                                           prop_name,
                                                           value,
                                                           pspec,
                                                           node);
   }

   RETURN(ret);
}

static void
json_serializable_init (gpointer p)
{
   JsonSerializableIface *iface = p;

   iface->deserialize_property = gb_project_item_deserialize_property;
}

/* gb-project-group.c
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
#include <string.h>

#include "gb-project-group.h"
#include "gb-log.h"

G_DEFINE_TYPE(GbProjectGroup, gb_project_group, GB_TYPE_PROJECT_ITEM)

struct _GbProjectGroupPrivate
{
   GPtrArray *items;
   GType item_type;
};

enum
{
   PROP_0,
   PROP_COUNT,
   PROP_ITEM_TYPE,
   LAST_PROP
};

enum
{
   INSERTED,
   REMOVED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

/**
 * gb_project_group_get_count:
 * @group: (in): A #GbProjectGroup.
 *
 * Gets the number of items contained in @group.
 *
 * Returns: An unsigned integer containing the count.
 */
guint
gb_project_group_get_count (GbProjectGroup *group)
{
   g_return_val_if_fail(GB_IS_PROJECT_GROUP(group), 0);
   return group->priv->items->len;
}

/**
 * gb_project_group_insert:
 * @group: (in): A #GbProjectGroup.
 * @index_: (in): The index for @item.
 * @item: (in) (transfer full): A #GbProjectItem to insert.
 *
 * Inserts @item into @group at @index_.
 */
void
gb_project_group_insert (GbProjectGroup *group,
                         guint           index_,
                         GbProjectItem  *item)
{
   GbProjectGroupPrivate *priv;
   gboolean do_memmove;
   GType gtype;

   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_GROUP(group));
   g_return_if_fail(index_ <= group->priv->items->len);
   g_return_if_fail(GB_IS_PROJECT_ITEM(item));

   priv = group->priv;

   /*
    * Make sure this object can be stored in the group.
    */
   gtype = G_TYPE_FROM_INSTANCE(item);
   if (!g_type_is_a(gtype, priv->item_type)) {
      g_warning(_("%s is not a %s instance."),
                g_type_name(gtype),
                g_type_name(priv->item_type));
      EXIT;
   }

   /*
    * HACK: Workaround for GPtrArray not having insert.
    *
    * GPtrArray as of 2.30 does not allow us to do insertions. So we
    * synthesize it ourself by first appending the item to the end so
    * our array is big enough, then perform a memmove() to move the
    * tail data, and then copy the new value in.
    */
   do_memmove = (index_ != priv->items->len);
   g_ptr_array_add(priv->items, g_object_ref_sink(item));
   if (do_memmove) {
      g_memmove(&priv->items->pdata[index_ + 1],
                &priv->items->pdata[index_],
                sizeof(gpointer) * (priv->items->len - 1 - index_));
      priv->items->pdata[index_] = item;
   }

   g_object_set(item, "parent", group, NULL);
   g_signal_emit(group, gSignals[INSERTED], 0, index_, item);
   g_object_notify_by_pspec(G_OBJECT(group), gParamSpecs[PROP_COUNT]);

   EXIT;
}

/**
 * gb_project_group_append:
 * @group: (in): A #GbProjectGroup.
 * @item: (in) (transfer full): A #GbProjectItem.
 *
 * Appends @item into @group.
 */
void
gb_project_group_append (GbProjectGroup *group,
                         GbProjectItem  *item)
{
   ENTRY;
   g_return_if_fail(GB_IS_PROJECT_GROUP(group));
   g_return_if_fail(GB_IS_PROJECT_ITEM(item));
   gb_project_group_insert(group, group->priv->items->len, item);
   EXIT;
}

/**
 * gb_project_group_remove_index:
 * @group: (in): A #GbProjectGroup.
 * @index_: (in): The index of the #GbProjectItem to remove.
 *
 * Removes the #GbProjectItem found at index @index_. The reference to
 * the child item will be released.
 */
void
gb_project_group_remove_index (GbProjectGroup *group,
                               guint           index_)
{
   GbProjectItem *item;

   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_GROUP(group));
   g_return_if_fail(index_ < group->priv->items->len);

   if ((item = g_ptr_array_remove_index(group->priv->items, index_))) {
      g_signal_emit(group, gSignals[REMOVED], 0, item);
      g_object_notify_by_pspec(G_OBJECT(group), gParamSpecs[PROP_COUNT]);
      g_object_unref(item);
   }

   EXIT;
}

/**
 * gb_project_group_remove:
 * @group: (in): A #GbProjectGroup.
 * @item: (in): A #GbProjectItem.
 *
 * Removes @item from @group and disowns @group<!-- -->'s reference.
 */
void
gb_project_group_remove (GbProjectGroup *group,
                         GbProjectItem  *item)
{
   GbProjectGroupPrivate *priv;
   guint i;

   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_GROUP(group));
   g_return_if_fail(GB_IS_PROJECT_ITEM(item));

   priv = group->priv;

   for (i = 0; i < priv->items->len; i++) {
      if (item == g_ptr_array_index(priv->items, i)) {
         gb_project_group_remove_index(group, i);
         break;
      }
   }

   EXIT;
}

/**
 * gb_project_group_get_items:
 * @group: (in): A #GbProjectGroup.
 *
 * Gets all of the items contained in @group and returns them in a newly
 * allocated #GList. The caller is responsible for freeing the #GList.
 * The #GbProjectItem<!-- -->'s within the #GList are not referenced.
 *
 * Returns: (transfer container) (element-type GbProjectItem*): A #GList.
 */
GList *
gb_project_group_get_items (GbProjectGroup *group)
{
   GbProjectGroupPrivate *priv;
   GList *list = NULL;
   guint i;

   ENTRY;

   g_return_val_if_fail(GB_IS_PROJECT_GROUP(group), NULL);

   priv = group->priv;

   for (i = 0; i < priv->items->len; i++) {
      list = g_list_prepend(list, g_ptr_array_index(priv->items, i));
   }

   list = g_list_reverse(list);

   RETURN(list);
}

/**
 * gb_project_group_get_item:
 * @group: (in): A #GbProjectGroup.
 * @index_: (in): The index of the item to get.
 *
 * Gets an item from the group.
 *
 * Returns: (transfer none): A #GbProjectItem.
 */
GbProjectItem *
gb_project_group_get_item (GbProjectGroup *group,
                           guint           index_)
{
   ENTRY;

   g_return_val_if_fail(GB_IS_PROJECT_GROUP(group), NULL);
   g_return_val_if_fail(index_ < group->priv->items->len, NULL);

   RETURN(g_ptr_array_index(group->priv->items, index_));
}

/**
 * gb_project_group_get_item_type:
 * @group: (in): A #GbProjectGroup.
 *
 * Gets the #GType of items that may be contained in @group.
 *
 * Returns: A #GType.
 */
GType
gb_project_group_get_item_type (GbProjectGroup *group)
{
   g_return_val_if_fail(GB_IS_PROJECT_GROUP(group), G_TYPE_NONE);
   return group->priv->item_type;
}

static void
gb_project_group_set_item_type (GbProjectGroup *group,
                                GType           item_type)
{
   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_GROUP(group));
   g_return_if_fail(g_type_is_a(item_type, GB_TYPE_PROJECT_ITEM));

   group->priv->item_type = item_type;
   g_object_notify_by_pspec(G_OBJECT(group), gParamSpecs[PROP_ITEM_TYPE]);

   EXIT;
}

/**
 * gb_project_group_dispose:
 * @object: (in): A #GbProjectGroup.
 *
 * Dispose for a #GbProjectGroup instance. Any related #GObject<!-- -->'s
 * are released to break circular dependencies.
 */
static void
gb_project_group_dispose (GObject *object)
{
   GbProjectGroupPrivate *priv;
   GbProjectGroup *group = (GbProjectGroup *)object;

   ENTRY;

   g_return_if_fail(GB_IS_PROJECT_GROUP(group));

   priv = group->priv;

   while (priv->items->len) {
      gb_project_group_remove_index(group, 0);
   }

   G_OBJECT_CLASS(gb_project_group_parent_class)->dispose(object);

   EXIT;
}

/**
 * gb_project_group_finalize:
 * @object: (in): A #GbProjectGroup.
 *
 * Finalizer for a #GbProjectGroup instance.  Frees any resources held by
 * the instance.
 */
static void
gb_project_group_finalize (GObject *object)
{
   GbProjectGroupPrivate *priv = GB_PROJECT_GROUP(object)->priv;

   ENTRY;

   g_assert_cmpint(priv->items->len, ==, 0);
   g_ptr_array_free(priv->items, TRUE);

   G_OBJECT_CLASS(gb_project_group_parent_class)->finalize(object);

   EXIT;
}

/**
 * gb_project_group_get_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (out): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Get a given #GObject property.
 */
static void
gb_project_group_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
   GbProjectGroup *group = GB_PROJECT_GROUP(object);

   switch (prop_id) {
   case PROP_COUNT:
      g_value_set_uint(value, gb_project_group_get_count(group));
      break;
   case PROP_ITEM_TYPE:
      g_value_set_gtype(value, gb_project_group_get_item_type(group));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_group_set_property:
 * @object: (in): A #GObject.
 * @prop_id: (in): The property identifier.
 * @value: (in): The given property.
 * @pspec: (in): A #ParamSpec.
 *
 * Set a given #GObject property.
 */
static void
gb_project_group_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
   GbProjectGroup *group = GB_PROJECT_GROUP(object);

   switch (prop_id) {
   case PROP_ITEM_TYPE:
      gb_project_group_set_item_type(group, g_value_get_gtype(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

/**
 * gb_project_group_class_init:
 * @klass: (in): A #GbProjectGroupClass.
 *
 * Initializes the #GbProjectGroupClass and prepares the vtable.
 */
static void
gb_project_group_class_init (GbProjectGroupClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_project_group_dispose;
   object_class->finalize = gb_project_group_finalize;
   object_class->get_property = gb_project_group_get_property;
   object_class->set_property = gb_project_group_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectGroupPrivate));

   /**
    * GbProjectGroup:count:
    *
    * The "count" property contains the number of items in the group.
    */
   gParamSpecs[PROP_COUNT] =
      g_param_spec_uint("count",
                        _("Count"),
                        _("The number of items in the group."),
                        0,
                        G_MAXUINT,
                        0,
                        G_PARAM_READABLE);
   g_object_class_install_property(object_class, PROP_COUNT,
                                   gParamSpecs[PROP_COUNT]);

   /**
    * GbProjectGroup:item-type:
    *
    * The "item-type" property contains the #GType of objects that may be
    * stored within the #GbProjectGroup instance.
    */
   gParamSpecs[PROP_ITEM_TYPE] =
      g_param_spec_gtype("item-type",
                          _("Item Type"),
                          _("The type of items that may be contained."),
                          GB_TYPE_PROJECT_ITEM,
                          G_PARAM_READWRITE);
   g_object_class_install_property(object_class, PROP_ITEM_TYPE,
                                   gParamSpecs[PROP_ITEM_TYPE]);

   /**
    * GbProjectGroup::inserted:
    * @group: (in): A #GbProjectGroup.
    * @index_: (in): The index position of the item.
    * @item: (in): A #GbProjectItem.
    *
    * The "inserted" signal is emitted when @item is inserted into @group.
    */
   gSignals[INSERTED] = g_signal_new("inserted",
                                     GB_TYPE_PROJECT_GROUP,
                                     G_SIGNAL_RUN_FIRST,
                                     0,
                                     NULL,
                                     NULL,
                                     g_cclosure_marshal_generic,
                                     G_TYPE_NONE,
                                     2,
                                     G_TYPE_INT,
                                     G_TYPE_OBJECT);

   /**
    * GbProjectGroup::removed:
    * @group: A #GbProjectGroup.
    * @item: The #GbProjectItem that was removed.
    *
    * The "removed" signal is emitted when @item is removed from @group.
    */
   gSignals[REMOVED] = g_signal_new("removed",
                                    GB_TYPE_PROJECT_GROUP,
                                    G_SIGNAL_RUN_FIRST,
                                    0,
                                    NULL,
                                    NULL,
                                    g_cclosure_marshal_generic,
                                    G_TYPE_NONE,
                                    1,
                                    G_TYPE_OBJECT);
}

/**
 * gb_project_group_init:
 * @group: (in): A #GbProjectGroup.
 *
 * Initializes the newly created #GbProjectGroup instance.
 */
static void
gb_project_group_init (GbProjectGroup *group)
{
   group->priv =
      G_TYPE_INSTANCE_GET_PRIVATE(group,
                                  GB_TYPE_PROJECT_GROUP,
                                  GbProjectGroupPrivate);
   group->priv->items = g_ptr_array_new();
   group->priv->item_type = GB_TYPE_PROJECT_ITEM;
}

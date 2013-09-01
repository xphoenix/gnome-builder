/* gb-workspace-pane.c
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

#include "gb-workspace-pane.h"
#include "gb-workspace-pane-group.h"

G_DEFINE_TYPE(GbWorkspacePane, gb_workspace_pane, GTK_TYPE_GRID)

struct _GbWorkspacePanePrivate
{
   gboolean  can_fullscreen;
   gboolean  can_save;
   gboolean  busy;
   gchar    *icon_name;
   gchar    *title;
   gchar    *uri;
};

enum
{
   PROP_0,
   PROP_BUSY,
   PROP_CAN_FULLSCREEN,
   PROP_CAN_SAVE,
   PROP_ICON_NAME,
   PROP_TITLE,
   PROP_URI,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_workspace_pane_get_icon_name (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), NULL);
   return pane->priv->icon_name;
}

void
gb_workspace_pane_close (GbWorkspacePane *pane)
{
   GtkWidget *parent;

   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   for (parent = gtk_widget_get_parent(GTK_WIDGET(pane));
        parent;
        parent = gtk_widget_get_parent(parent)) {
      if (GB_IS_WORKSPACE_PANE_GROUP(parent)) {
         gb_workspace_pane_group_close(GB_WORKSPACE_PANE_GROUP(parent), pane);
         break;
      }
   }
}

void
gb_workspace_pane_fullscreen (GbWorkspacePane *pane)
{
   GbWorkspacePaneClass *klass;

   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   if (pane->priv->can_fullscreen) {
      klass = GB_WORKSPACE_PANE_GET_CLASS(pane);
      if (klass->fullscreen) {
         klass->fullscreen(pane);
      }
   }
}

void
gb_workspace_pane_set_icon_name (GbWorkspacePane *pane,
                                 const gchar     *icon_name)
{
   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   g_free(pane->priv->icon_name);
   pane->priv->icon_name = g_strdup(icon_name);
   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_ICON_NAME]);
}

const gchar *
gb_workspace_pane_get_uri (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), NULL);
   return pane->priv->uri;
}

void
gb_workspace_pane_set_uri (GbWorkspacePane *pane,
                           const gchar     *uri)
{
   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   g_free(pane->priv->uri);
   pane->priv->uri = g_strdup(uri);
   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_URI]);
}

gboolean
gb_workspace_pane_get_can_fullscreen (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), FALSE);
   return pane->priv->can_fullscreen;
}

void
gb_workspace_pane_set_can_fullscreen (GbWorkspacePane *pane,
                                      gboolean         can_fullscreen)
{
   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   if (pane->priv->can_fullscreen != can_fullscreen) {
      pane->priv->can_fullscreen = can_fullscreen;
      g_object_notify_by_pspec(G_OBJECT(pane),
                               gParamSpecs[PROP_CAN_FULLSCREEN]);
   }
}

gboolean
gb_workspace_pane_get_can_save (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), FALSE);
   return pane->priv->can_save;
}

void
gb_workspace_pane_set_can_save (GbWorkspacePane *pane,
                                gboolean         can_save)
{
   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   if (pane->priv->can_save != can_save) {
      pane->priv->can_save = can_save;
      g_object_notify_by_pspec(G_OBJECT(pane),
                               gParamSpecs[PROP_CAN_SAVE]);
   }
}

gboolean
gb_workspace_pane_get_busy (GbWorkspacePane *pane)
{
   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));
   return pane->priv->busy;
}

void
gb_workspace_pane_set_busy (GbWorkspacePane *pane,
                            gboolean         busy)
{
   GbWorkspacePanePrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   priv = pane->priv;

   if (priv->busy != busy) {
      priv->busy = busy;
      g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_BUSY]);
   }
}

const gchar *
gb_workspace_pane_get_title (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), NULL);
   return pane->priv->title;
}

void
gb_workspace_pane_set_title (GbWorkspacePane *pane,
                             const gchar     *title)
{
   GbWorkspacePanePrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   priv = pane->priv;

   g_free(priv->title);
   priv->title = g_strdup(title);
   g_object_notify_by_pspec(G_OBJECT(pane), gParamSpecs[PROP_TITLE]);
}

void
gb_workspace_pane_save_async (GbWorkspacePane     *pane,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
   GbWorkspacePaneClass *klass;

   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));
   g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
   g_return_if_fail(callback);

   klass = GB_WORKSPACE_PANE_GET_CLASS(pane);
   if (klass->save_async) {
      klass->save_async(pane, cancellable, callback, user_data);
   }
}

gboolean
gb_workspace_pane_save_finish (GbWorkspacePane  *pane,
                               GAsyncResult     *result,
                               GError          **error)
{
   GbWorkspacePaneClass *klass;

   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), FALSE);
   g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

   klass = GB_WORKSPACE_PANE_GET_CLASS(pane);
   if (klass->save_finish) {
      return klass->save_finish(pane, result, error);
   }

   return TRUE;
}

static void
gb_workspace_pane_finalize (GObject *object)
{
   GbWorkspacePanePrivate *priv = GB_WORKSPACE_PANE(object)->priv;

   g_clear_pointer(&priv->icon_name, g_free);
   g_clear_pointer(&priv->title, g_free);
   g_clear_pointer(&priv->uri, g_free);

   G_OBJECT_CLASS(gb_workspace_pane_parent_class)->finalize(object);
}

static void
gb_workspace_pane_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
   GbWorkspacePane *pane = GB_WORKSPACE_PANE(object);

   switch (prop_id) {
   case PROP_BUSY:
      g_value_set_boolean(value, gb_workspace_pane_get_busy(pane));
      break;
   case PROP_CAN_FULLSCREEN:
      g_value_set_boolean(value, gb_workspace_pane_get_can_fullscreen(pane));
      break;
   case PROP_CAN_SAVE:
      g_value_set_boolean(value, gb_workspace_pane_get_can_save(pane));
      break;
   case PROP_ICON_NAME:
      g_value_set_string(value, gb_workspace_pane_get_icon_name(pane));
      break;
   case PROP_TITLE:
      g_value_set_string(value, gb_workspace_pane_get_title(pane));
      break;
   case PROP_URI:
      g_value_set_string(value, gb_workspace_pane_get_uri(pane));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_pane_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
   GbWorkspacePane *pane = GB_WORKSPACE_PANE(object);

   switch (prop_id) {
   case PROP_BUSY:
      gb_workspace_pane_set_busy(pane, g_value_get_boolean(value));
      break;
   case PROP_CAN_FULLSCREEN:
      gb_workspace_pane_set_can_fullscreen(pane, g_value_get_boolean(value));
      break;
   case PROP_CAN_SAVE:
      gb_workspace_pane_set_can_save(pane, g_value_get_boolean(value));
      break;
   case PROP_ICON_NAME:
      gb_workspace_pane_set_icon_name(pane, g_value_get_string(value));
      break;
   case PROP_TITLE:
      gb_workspace_pane_set_title(pane, g_value_get_string(value));
      break;
   case PROP_URI:
      gb_workspace_pane_set_uri(pane, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_pane_class_init (GbWorkspacePaneClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_pane_finalize;
   object_class->get_property = gb_workspace_pane_get_property;
   object_class->set_property = gb_workspace_pane_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspacePanePrivate));

   gParamSpecs[PROP_BUSY] =
      g_param_spec_boolean("busy",
                           _("Busy"),
                           _("If the workspace pane busy indicator should be set."),
                           FALSE,
                           (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_BUSY,
                                   gParamSpecs[PROP_BUSY]);

   gParamSpecs[PROP_CAN_FULLSCREEN] =
      g_param_spec_boolean("can-fullscreen",
                          _("Can Fullscreen"),
                          _("If the widget can fullscreen."),
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_FULLSCREEN,
                                   gParamSpecs[PROP_CAN_FULLSCREEN]);

   gParamSpecs[PROP_CAN_SAVE] =
      g_param_spec_boolean("can-save",
                          _("Modified"),
                          _("If the panes contents have been modified."),
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CAN_SAVE,
                                   gParamSpecs[PROP_CAN_SAVE]);

   gParamSpecs[PROP_ICON_NAME] =
      g_param_spec_string("icon-name",
                          _("Icon Name"),
                          _("The name of the icon to display."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_ICON_NAME,
                                   gParamSpecs[PROP_ICON_NAME]);

   gParamSpecs[PROP_TITLE] =
      g_param_spec_string("title",
                          _("Title"),
                          _("The title of the pane."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_TITLE,
                                   gParamSpecs[PROP_TITLE]);

   gParamSpecs[PROP_URI] =
      g_param_spec_string("uri",
                          _("Uri"),
                          _("The uri of the document."),
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_URI,
                                   gParamSpecs[PROP_URI]);
}

static void
gb_workspace_pane_init (GbWorkspacePane *pane)
{
   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_WORKSPACE_PANE,
                                            GbWorkspacePanePrivate);
}

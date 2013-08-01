/* gb-workspace-pane.c:
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

G_DEFINE_TYPE(GbWorkspacePane, gb_workspace_pane, GTK_TYPE_GRID)

struct _GbWorkspacePanePrivate
{
   gchar *icon_name;
   gboolean modified;
   gchar *title;
};

enum
{
   PROP_0,
   PROP_ICON_NAME,
   PROP_MODIFIED,
   PROP_TITLE,
   LAST_PROP
};

enum
{
   MODIFIED_CHANGED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

const gchar *
gb_workspace_pane_get_icon_name (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), NULL);
   return pane->priv->icon_name;
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

gboolean
gb_workspace_pane_get_modified (GbWorkspacePane *pane)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_PANE(pane), FALSE);
   return pane->priv->modified;
}

void
gb_workspace_pane_set_modified (GbWorkspacePane *pane,
                                gboolean         modified)
{
   g_return_if_fail(GB_IS_WORKSPACE_PANE(pane));

   if (pane->priv->modified != modified) {
      pane->priv->modified = modified;
      g_object_notify_by_pspec(G_OBJECT(pane),
                               gParamSpecs[PROP_MODIFIED]);
      g_signal_emit(pane, gSignals[MODIFIED_CHANGED], 0);
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

static void
gb_workspace_pane_finalize (GObject *object)
{
   GbWorkspacePanePrivate *priv = GB_WORKSPACE_PANE(object)->priv;

   g_clear_pointer(&priv->title, g_free);

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
   case PROP_ICON_NAME:
      g_value_set_string(value, gb_workspace_pane_get_icon_name(pane));
      break;
   case PROP_MODIFIED:
      g_value_set_boolean(value, gb_workspace_pane_get_modified(pane));
      break;
   case PROP_TITLE:
      g_value_set_string(value, gb_workspace_pane_get_title(pane));
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
   case PROP_ICON_NAME:
      gb_workspace_pane_set_icon_name(pane, g_value_get_string(value));
      break;
   case PROP_MODIFIED:
      gb_workspace_pane_set_modified(pane, g_value_get_boolean(value));
      break;
   case PROP_TITLE:
      gb_workspace_pane_set_title(pane, g_value_get_string(value));
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

   gParamSpecs[PROP_ICON_NAME] =
      g_param_spec_string("icon-name",
                          _("Icon Name"),
                          _("The name of the icon to display."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_ICON_NAME,
                                   gParamSpecs[PROP_ICON_NAME]);

   gParamSpecs[PROP_MODIFIED] =
      g_param_spec_boolean("modified",
                          _("Modified"),
                          _("If the panes contents have been modified."),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_MODIFIED,
                                   gParamSpecs[PROP_MODIFIED]);

   gParamSpecs[PROP_TITLE] =
      g_param_spec_string("title",
                          _("Title"),
                          _("The title of the pane."),
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
   g_object_class_install_property(object_class, PROP_TITLE,
                                   gParamSpecs[PROP_TITLE]);

   gSignals[MODIFIED_CHANGED] = g_signal_new("modified-changed",
                                             GB_TYPE_WORKSPACE_PANE,
                                             G_SIGNAL_RUN_FIRST,
                                             0,
                                             NULL,
                                             NULL,
                                             g_cclosure_marshal_VOID__VOID,
                                             G_TYPE_NONE,
                                             0);
}

static void
gb_workspace_pane_init (GbWorkspacePane *pane)
{
   pane->priv = G_TYPE_INSTANCE_GET_PRIVATE(pane,
                                            GB_TYPE_WORKSPACE_PANE,
                                            GbWorkspacePanePrivate);
}

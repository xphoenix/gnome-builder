/* gb-project-info.c
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

#include "gb-project-info.h"

G_DEFINE_TYPE(GbProjectInfo, gb_project_info, G_TYPE_OBJECT)

/**
 * SECTION:gb-project-info
 * @title: GbProjectInfo
 * @short_description: Information about a known project.
 *
 * This class contains information about a project. It is used for accessing
 * recent projects without having to load the entire project file, which could
 * both fail and be time consuming.
 *
 * This is meant to be stored as recent project information which can be
 * displayed by the greeter.
 */

struct _GbProjectInfoPrivate
{
   GFile     *file;
   GDateTime *last_modified_at;
   gchar     *name;
};

enum
{
   PROP_0,
   PROP_FILE,
   PROP_LAST_MODIFIED_AT,
   PROP_NAME,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

const gchar *
gb_project_info_get_name (GbProjectInfo *info)
{
   g_return_val_if_fail(GB_IS_PROJECT_INFO(info), NULL);

   return info->priv->name;
}

static void
gb_project_info_set_name (GbProjectInfo *info,
                          const gchar   *name)
{
   g_return_if_fail(GB_IS_PROJECT_INFO(info));

   g_free(info->priv->name);
   info->priv->name = g_strdup(name);
   g_object_notify_by_pspec(G_OBJECT(info), gParamSpecs[PROP_NAME]);
}

GDateTime *
gb_project_info_get_last_modified_at (GbProjectInfo *info)
{
   g_return_val_if_fail(GB_IS_PROJECT_INFO(info), NULL);

   return info->priv->last_modified_at;
}

static void
gb_project_info_set_last_modified_at (GbProjectInfo *info,
                                      GDateTime     *last_modified_at)
{
   g_return_if_fail(GB_IS_PROJECT_INFO(info));

   g_clear_pointer(&info->priv->last_modified_at, g_date_time_unref);
   info->priv->last_modified_at = last_modified_at ?
      g_date_time_ref(last_modified_at) : NULL;
   g_object_notify_by_pspec(G_OBJECT(info),
                            gParamSpecs[PROP_LAST_MODIFIED_AT]);
}

/**
 * gb_project_info_get_file:
 * @info: (in): A #GbProjectInfo.
 *
 * Fetches the "file" property.
 *
 * Returns: (transfer none): A #GFile.
 */
GFile *
gb_project_info_get_file (GbProjectInfo *info)
{
   g_return_if_fail(GB_IS_PROJECT_INFO(info));

   return info->priv->file;
}

static void
gb_project_info_set_file (GbProjectInfo *info,
                          GFile         *file)
{
   g_return_if_fail(GB_IS_PROJECT_INFO(info));

   g_clear_object(&info->priv->file);
   info->priv->file = file ? g_object_ref(file) : NULL;
   g_object_notify_by_pspec(G_OBJECT(info), gParamSpecs[PROP_FILE]);
}

static void
gb_project_info_finalize (GObject *object)
{
   GbProjectInfoPrivate *priv;

   priv = GB_PROJECT_INFO(object)->priv;

   g_clear_pointer(&priv->last_modified_at, g_date_time_unref);
   g_clear_pointer(&priv->name, g_free);
   g_clear_object(&priv->file);

   G_OBJECT_CLASS(gb_project_info_parent_class)->finalize(object);
}

static void
gb_project_info_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
   GbProjectInfo *info = GB_PROJECT_INFO(object);

   switch (prop_id) {
   case PROP_FILE:
      g_value_set_object(value, gb_project_info_get_file(info));
      break;
   case PROP_LAST_MODIFIED_AT:
      g_value_set_boxed(value, gb_project_info_get_last_modified_at(info));
      break;
   case PROP_NAME:
      g_value_set_string(value, gb_project_info_get_name(info));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_project_info_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
   GbProjectInfo *info = GB_PROJECT_INFO(object);

   switch (prop_id) {
   case PROP_FILE:
      gb_project_info_set_file(info, g_value_get_object(value));
      break;
   case PROP_LAST_MODIFIED_AT:
      gb_project_info_set_last_modified_at(info, g_value_get_boxed(value));
      break;
   case PROP_NAME:
      gb_project_info_set_name(info, g_value_get_string(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_project_info_class_init (GbProjectInfoClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_project_info_finalize;
   object_class->get_property = gb_project_info_get_property;
   object_class->set_property = gb_project_info_set_property;
   g_type_class_add_private(object_class, sizeof(GbProjectInfoPrivate));

   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("A GFile representing the project file."),
                          G_TYPE_FILE,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);

   gParamSpecs[PROP_LAST_MODIFIED_AT] =
      g_param_spec_boxed("last-modified-at",
                         _("Last Modified At"),
                         _("The date and time of last modification."),
                         G_TYPE_DATE_TIME,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_LAST_MODIFIED_AT,
                                   gParamSpecs[PROP_LAST_MODIFIED_AT]);

   gParamSpecs[PROP_NAME] =
      g_param_spec_string("name",
                          _("Name"),
                          _("Name"),
                          NULL,
                          (G_PARAM_READWRITE |
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_NAME,
                                   gParamSpecs[PROP_NAME]);
}

static void
gb_project_info_init (GbProjectInfo *info)
{
   info->priv = G_TYPE_INSTANCE_GET_PRIVATE(info,
                                            GB_TYPE_PROJECT_INFO,
                                            GbProjectInfoPrivate);
}

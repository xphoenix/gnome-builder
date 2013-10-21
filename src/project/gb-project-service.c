/* gb-project-service.c
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

#include "gb-application.h"
#include "gb-project.h"
#include "gb-project-info.h"
#include "gb-project-service.h"

G_DEFINE_TYPE(GbProjectService, gb_project_service, GB_TYPE_SERVICE)

struct _GbProjectServicePrivate
{
   GList *projects;
   GList *recent_projects;
};

enum
{
   ADDED,
   LAST_SIGNAL
};

static guint gSignals[LAST_SIGNAL];

gboolean
gb_project_service_add (GbProjectService *service,
                        GbProject        *project)
{
   GbProjectServicePrivate *priv;

   g_return_val_if_fail(GB_IS_PROJECT_SERVICE(service), FALSE);
   g_return_val_if_fail(GB_IS_PROJECT(project), FALSE);

   priv = service->priv;

   priv->projects = g_list_prepend(priv->projects, g_object_ref(project));
   g_signal_emit(service, gSignals[ADDED], 0, project);

   return TRUE;
}

/**
 * gb_project_service_get_recent_projects:
 * @service: (in): A #GbProjectService.
 *
 * Fetches the recent projects as known to GNOME Builder.
 *
 * Returns: (transfer none) (element-type GbProjectInfo*): A list of
 *   #GbProjectInfo that should not be modified or freed.
 */
GList *
gb_project_service_get_recent_projects (GbProjectService *service)
{
   g_return_val_if_fail(GB_IS_PROJECT_SERVICE(service), NULL);
   return service->priv->recent_projects;
}

static void
gb_project_service_finalize (GObject *object)
{
   GbProjectServicePrivate *priv;

   priv = GB_PROJECT_SERVICE(object)->priv;

   g_list_foreach(priv->projects, (GFunc)g_object_unref, NULL);
   g_list_free(priv->projects);
   priv->projects = NULL;

   G_OBJECT_CLASS(gb_project_service_parent_class)->finalize(object);
}

static void
gb_project_service_class_init (GbProjectServiceClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_project_service_finalize;
   g_type_class_add_private(object_class, sizeof(GbProjectServicePrivate));

   gSignals[ADDED] = g_signal_new("added",
                                  GB_TYPE_PROJECT_SERVICE,
                                  G_SIGNAL_RUN_LAST,
                                  0,
                                  NULL,
                                  NULL,
                                  g_cclosure_marshal_VOID__OBJECT,
                                  G_TYPE_NONE,
                                  1,
                                  GB_TYPE_PROJECT);
}

static void
gb_project_service_init (GbProjectService *service)
{
   GbProjectInfo *info;
   GDateTime *dt;
   GFile *file;

   service->priv = G_TYPE_INSTANCE_GET_PRIVATE(service,
                                               GB_TYPE_PROJECT_SERVICE,
                                               GbProjectServicePrivate);

   /*
    * TODO: Remove this dummy code when we get recent stuff taken care of.
    */
   dt = g_date_time_new_now_utc();
   file = g_file_new_for_path(".gbproject");
   info = g_object_new(GB_TYPE_PROJECT_INFO,
                       "name", "GNOME Builder",
                       "file", file,
                       "last-modified-at", dt,
                       NULL);
   service->priv->recent_projects = g_list_append(NULL, info);
   g_date_time_unref(dt);
   g_object_unref(file);
}

GB_SERVICE_REGISTER("project",
                    gb_project_service,
                    GB_TYPE_PROJECT_SERVICE,
                    TRUE)

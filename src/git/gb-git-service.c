/* gb-git-service.c
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
#include "gb-git-service.h"
#include "gb-git-service-dbus.h"

#define GB_GIT_SERVICE_PATH "/org/gnome/Builder/Services/Git"

G_DEFINE_TYPE(GbGitService, gb_git_service, GB_TYPE_SERVICE)

struct _GbGitServicePrivate
{
   GbGitServiceDBusObjectProxy    *proxy;
   GbGitServiceDBusObjectSkeleton *skeleton;
};

static void
gb_git_service_start (GbService *service)
{
   GbGitServicePrivate *priv;
   GDBusConnection *connection;
   GbServiceMode mode;
   GbGitService *git = (GbGitService *)service;

   g_return_val_if_fail(GB_IS_GIT_SERVICE(git), NULL);

   priv = git->priv;

   connection = gb_service_get_connection(service);
   mode = gb_service_get_mode(service);

   if (mode == GB_SERVICE_LOCAL) {
      priv->skeleton =
         gb_git_service_dbus_object_skeleton_new(GB_GIT_SERVICE_PATH);
   } else {
      priv->proxy =
         gb_git_service_dbus_object_proxy_new(connection,
                                              GB_GIT_SERVICE_PATH);
   }
}

static void
gb_git_service_finalize (GObject *object)
{
   G_OBJECT_CLASS(gb_git_service_parent_class)->finalize(object);
}

static void
gb_git_service_class_init (GbGitServiceClass *klass)
{
   GObjectClass *object_class;
   GbServiceClass *service_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_git_service_finalize;
   g_type_class_add_private(object_class, sizeof(GbGitServicePrivate));

   service_class = GB_SERVICE_CLASS(klass);
   service_class->start = gb_git_service_start;
}

static void
gb_git_service_init (GbGitService *service)
{
   service->priv = G_TYPE_INSTANCE_GET_PRIVATE(service,
                                               GB_TYPE_GIT_SERVICE,
                                               GbGitServicePrivate);
}

GB_SERVICE_REGISTER("git", gb_git_service, GB_TYPE_GIT_SERVICE, FALSE)

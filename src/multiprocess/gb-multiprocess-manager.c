/* gb-multiprocess-manager.c
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

#include <errno.h>
#include <gio/gunixconnection.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "gb-multiprocess-manager.h"

G_DEFINE_TYPE(GbMultiprocessManager, gb_multiprocess_manager, G_TYPE_OBJECT)

struct _GbMultiprocessManagerPrivate
{
   GHashTable *registered;
};

typedef struct
{
   gchar            *name;
   gchar           **argv;
   GPid              pid;
   gint              child_fd;
   gint              parent_fd;
   GDBusConnection  *dbus_conn;
} Worker;

GbMultiprocessManager *
gb_multiprocess_manager_new (void)
{
   return g_object_new(GB_TYPE_MULTIPROCESS_MANAGER, NULL);
}

GbMultiprocessManager *
gb_multiprocess_manager_get_default (void)
{
   static GbMultiprocessManager *instance;

   if (!instance) {
      instance = gb_multiprocess_manager_new();
      g_object_add_weak_pointer(G_OBJECT(instance), (gpointer *)&instance);
   }

   return instance;
}

static void
worker_close_fds (Worker *worker)
{
   struct rlimit rl;
   gint i;
   gint max_fd;

   if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
      g_error("Failed to call getrlimit.");
   }

   max_fd = rl.rlim_max;

   for (i = 3; i < max_fd; i++) {
      if (i != worker->child_fd) {
         close(i);
      }
   }
}

static void
worker_free (gpointer data)
{
   Worker *worker = data;

   if (worker->child_fd) {
      close(worker->child_fd);
   }

   if (worker->parent_fd) {
      close(worker->parent_fd);
   }

   g_free(worker->name);
   g_strfreev(worker->argv);
   g_clear_object(&worker->dbus_conn);
   g_free(worker);
}

static Worker *
worker_new (const gchar  *name,
            gchar       **argv)
{
   Worker *worker;

   worker = g_new0(Worker, 1);
   worker->name = g_strdup(name);
   worker->argv = g_strdupv(argv);
   worker->child_fd = -1;
   worker->parent_fd = -1;

   return worker;
}

static void
worker_child_setup (gpointer data)
{
   Worker *worker = data;

   worker_close_fds(worker);
}

static void
worker_start (Worker *worker)
{
   GIOStream *io_stream;
   GSocket *socket;
   GError *error = NULL;
   int sockets[2];

   errno = 0;
   if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
      g_warning("Failed to reate socket pair: %s", strerror(errno));
      return;
   }

   /*
    * TODO: 
    *
    *    Unify error handling.
    *    Store error in Worker for use by get_connection().
    */

   worker->child_fd = sockets[0];
   worker->parent_fd = sockets[1];

   if (!(socket = g_socket_new_from_fd(worker->parent_fd, &error))) {
      g_warning("%s", error->message);
      g_error_free(error);
      close(worker->child_fd);
      close(worker->parent_fd);
      worker->child_fd = -1;
      worker->parent_fd = -1;
      return;
   }

   {
      GPtrArray *argv;
      gchar **env = NULL;
      gchar *working_dir;
      gint i;

      working_dir = g_get_current_dir();

      argv = g_ptr_array_new_with_free_func(g_free);
      g_ptr_array_add(argv, g_strdup("gnome-builder-worker"));
      g_ptr_array_add(argv, g_strdup_printf("--dbus-fd=%d", worker->child_fd));
      for (i = 0; worker->argv && worker->argv[i]; i++) {
         g_ptr_array_add(argv, g_strdup(worker->argv[i]));
      }
      g_ptr_array_add(argv, NULL);

      if (!g_spawn_async(working_dir,
                         (gchar **)argv->pdata,
                         env,
                         G_SPAWN_LEAVE_DESCRIPTORS_OPEN,
                         worker_child_setup,
                         worker,
                         &worker->pid,
                         &error)) {
         g_ptr_array_unref(argv);
         g_free(working_dir);
         g_warning("Failed to execute worker: %s",
                   error->message);
         g_error_free(error);
         return;
      }

      g_ptr_array_unref(argv);
      g_free(working_dir);
   }

   io_stream = g_object_new(G_TYPE_UNIX_CONNECTION,
                            "socket", socket,
                            NULL);
   g_assert(G_IS_IO_STREAM(io_stream));

   g_object_unref(socket);

   worker->dbus_conn = g_dbus_connection_new_sync(io_stream,
                                                  NULL,
                                                  G_DBUS_CONNECTION_FLAGS_NONE,
                                                  NULL,
                                                  NULL,
                                                  &error);
   g_object_unref(io_stream);

   if (!worker->dbus_conn) {
      g_warning("Failed to create dbus connection: %s",
                error->message);
      g_error_free(error);
      close(worker->child_fd);
      worker->child_fd = -1;
      close(worker->parent_fd);
      worker->parent_fd = -1;
      return;
   }
}

GDBusConnection *
gb_multiprocess_manager_get_connection (GbMultiprocessManager  *manager,
                                        const gchar            *name,
                                        GError                **error)
{
   GbMultiprocessManagerPrivate *priv;
   Worker *worker;

   g_return_val_if_fail(GB_IS_MULTIPROCESS_MANAGER(manager), NULL);
   g_return_val_if_fail(name, NULL);

   priv = manager->priv;

   /*
    * TODO: Set error.
    */

   if (!(worker = g_hash_table_lookup(priv->registered, name))) {
      return NULL;
   }

   if (!worker->dbus_conn) {
      worker_start(worker);
   }

   return worker->dbus_conn;
}

static void
gb_multiprocess_manager_finalize (GObject *object)
{
   GbMultiprocessManagerPrivate *priv;

   priv = GB_MULTIPROCESS_MANAGER(object)->priv;

   g_clear_pointer(&priv->registered, g_hash_table_unref);

   G_OBJECT_CLASS(gb_multiprocess_manager_parent_class)->finalize(object);
}

static void
gb_multiprocess_manager_class_init (GbMultiprocessManagerClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_multiprocess_manager_finalize;
   g_type_class_add_private(object_class, sizeof(GbMultiprocessManagerPrivate));
}

static void
gb_multiprocess_manager_init (GbMultiprocessManager *manager)
{
   manager->priv = G_TYPE_INSTANCE_GET_PRIVATE(manager,
                                               GB_TYPE_MULTIPROCESS_MANAGER,
                                               GbMultiprocessManagerPrivate);

   manager->priv->registered = g_hash_table_new_full(g_str_hash,
                                                     g_str_equal,
                                                     NULL,
                                                     worker_free);
}

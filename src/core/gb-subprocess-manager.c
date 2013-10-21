/* gb-subprocess-manager.c
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
#include <signal.h>

#include "gb-subprocess-manager.h"

G_DEFINE_TYPE(GbSubprocessManager, gb_subprocess_manager, G_TYPE_OBJECT)

struct _GbSubprocessManagerPrivate
{
   GSList   *procs;
   gboolean  started;
};

typedef struct
{
   gchar    **argv;
   gboolean   respawn;
   GPid       pid;
   GError    *error;
} Process;

enum
{
   PROP_0,
   LAST_PROP
};

//static GParamSpec *gParamSpecs[LAST_PROP];

GbSubprocessManager *
gb_subprocess_manager_new (void)
{
   return g_object_new(GB_TYPE_SUBPROCESS_MANAGER, NULL);
}

static gboolean
process_start (Process *process)
{
   gchar **env = NULL;

   if (process->error) {
      return FALSE;
   }

   if (!g_spawn_async(g_get_current_dir(),
                      process->argv,
                      env,
                      G_SPAWN_DEFAULT,
                      NULL,
                      NULL,
                      &process->pid,
                      &process->error)) {
      g_warning("%s", process->error->message);
      return FALSE;
   }

   return TRUE;
}

void
gb_subprocess_manager_add (GbSubprocessManager  *manager,
                           gchar               **argv,
                           gboolean              respawn)
{
   GbSubprocessManagerPrivate *priv;
   Process *process;

   g_return_if_fail(GB_IS_SUBPROCESS_MANAGER(manager));

   priv = manager->priv;

   process = g_new0(Process, 1);
   process->argv = g_strdupv(argv);
   process->respawn = respawn;

   priv->procs = g_slist_prepend(priv->procs, process);

   if (priv->started) {
      process_start(process);
   }
}

void
gb_subprocess_manager_start (GbSubprocessManager *manager)
{
   GbSubprocessManagerPrivate *priv;
   GSList *iter;

   g_return_if_fail(GB_IS_SUBPROCESS_MANAGER(manager));

   priv = manager->priv;

   for (iter = priv->procs; iter; iter = iter->next) {
      process_start(iter->data);
   }

   priv->started = TRUE;
}

void
gb_subprocess_manager_stop (GbSubprocessManager *manager)
{
   GbSubprocessManagerPrivate *priv;
   Process *process;
   GSList *iter;

   g_return_if_fail(GB_IS_SUBPROCESS_MANAGER(manager));

   priv = manager->priv;

   priv->started = FALSE;

   for (iter = priv->procs; iter; iter = iter->next) {
      process = iter->data;

      if (process->pid) {
         kill(process->pid, SIGKILL);
      }
   }
}

static void
gb_subprocess_manager_finalize (GObject *object)
{
   //GbSubprocessManagerPrivate *priv;

   //priv = GB_SUBPROCESS_MANAGER(object)->priv;

   G_OBJECT_CLASS(gb_subprocess_manager_parent_class)->finalize(object);
}

static void
gb_subprocess_manager_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
   //GbSubprocessManager *manager = GB_SUBPROCESS_MANAGER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_subprocess_manager_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
   //GbSubprocessManager *manager = GB_SUBPROCESS_MANAGER(object);

   switch (prop_id) {
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_subprocess_manager_class_init (GbSubprocessManagerClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_subprocess_manager_finalize;
   object_class->get_property = gb_subprocess_manager_get_property;
   object_class->set_property = gb_subprocess_manager_set_property;
   g_type_class_add_private(object_class, sizeof(GbSubprocessManagerPrivate));
}

static void
gb_subprocess_manager_init (GbSubprocessManager *manager)
{
   manager->priv = G_TYPE_INSTANCE_GET_PRIVATE(manager,
                                               GB_TYPE_SUBPROCESS_MANAGER,
                                               GbSubprocessManagerPrivate);
}

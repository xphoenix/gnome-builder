/* gb-action.h
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

#ifndef GB_ACTION_H
#define GB_ACTION_H

#include <glib-object.h>

#include "gb-application.h"
#include "gb-project.h"
#include "gb-workspace.h"

G_BEGIN_DECLS

#define GB_TYPE_ACTION                        (gb_action_get_type())
#define GB_ACTION(obj)                        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_ACTION, GbAction))
#define GB_ACTION_CONST(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_ACTION, GbAction const))
#define GB_ACTION_CLASS(klass)                (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_ACTION, GbActionClass))
#define GB_IS_ACTION(obj)                     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_ACTION))
#define GB_IS_ACTION_CLASS(klass)             (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_ACTION))
#define GB_ACTION_GET_CLASS(obj)              (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_ACTION, GbActionClass))

#define GB_ACTION_REGISTER_STATIC_N(N, name, func) \
   static void __gb_action_##N##_init (void) __attribute__((constructor)); \
   static void __gb_action_##N##_init (void) { \
      GbAction *a = gb_action_new(name); \
      g_signal_connect(a, "activate", G_CALLBACK(func), NULL); \
      g_action_map_add_action(G_ACTION_MAP(GB_APPLICATION_DEFAULT), G_ACTION(a)); \
      g_object_unref(a); \
   }

#define GB_ACTION_REGISTER_STATIC(name, func) GB_ACTION_REGISTER_STATIC_N(__COUNTER__, name, func)

typedef struct _GbAction        GbAction;
typedef struct _GbActionClass   GbActionClass;
typedef struct _GbActionPrivate GbActionPrivate;

struct _GbAction
{
   GObject parent;

   /*< private >*/
   GbActionPrivate *priv;
};

struct _GbActionClass
{
   GObjectClass parent_class;
};

GbAction      *gb_action_new             (const gchar *name);
GType          gb_action_get_type        (void) G_GNUC_CONST;
GbApplication *gb_action_get_application (GbAction    *action);
GbProject     *gb_action_get_project     (GbAction    *action);
GbWorkspace   *gb_action_get_workspace   (GbAction    *action);
void           gb_action_set_enabled     (GbAction    *action,
                                          gboolean     enabled);

G_END_DECLS

#endif /* GB_ACTION_H */

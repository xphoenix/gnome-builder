/* gb-glade-addin.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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
#include <libpeas/peas.h>

#include "gb-glade-addin.h"
#include "gb-glade-panel.h"
#include "gb-workbench-addin.h"
#include "gb-workspace.h"

struct _GbGladeAddin
{
  GObject       parent_instnace;

  GbWorkbench  *workbench;
  GbGladePanel *panel;
};

static void workbench_addin_iface_init (GbWorkbenchAddinInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GbGladeAddin, gb_glade_addin, G_TYPE_OBJECT, 0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GB_TYPE_WORKBENCH_ADDIN,
                                                               workbench_addin_iface_init))

enum {
  PROP_0,
  PROP_WORKBENCH,
  LAST_PROP
};

static GParamSpec *gParamSpecs [LAST_PROP];

static void
gb_glade_addin_load (GbWorkbenchAddin *addin)
{
  GbGladeAddin *self = (GbGladeAddin *)addin;
  GtkWidget *workspace;
  GtkWidget *pane;

  g_assert (GB_IS_GLADE_ADDIN (self));

  workspace = gb_workbench_get_workspace (self->workbench);
  pane = gb_workspace_get_right_pane (GB_WORKSPACE (workspace));

  self->panel = g_object_new (GB_TYPE_GLADE_PANEL,
                              "visible", TRUE,
                              NULL);
  gb_workspace_pane_add_page (GB_WORKSPACE_PANE (pane), GTK_WIDGET (self->panel),
                              _("Glade"), "glade-symbolic");
}

static void
gb_glade_addin_finalize (GObject *object)
{
  G_OBJECT_CLASS (gb_glade_addin_parent_class)->finalize (object);
}

static void
gb_glade_addin_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GbGladeAddin *self = (GbGladeAddin *)object;

  switch (prop_id)
    {
    case PROP_WORKBENCH:
      ide_set_weak_pointer (&self->workbench, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gb_glade_addin_class_finalize (GbGladeAddinClass *klass)
{
}

static void
gb_glade_addin_class_init (GbGladeAddinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gb_glade_addin_finalize;
  object_class->set_property = gb_glade_addin_set_property;

  gParamSpecs [PROP_WORKBENCH] =
    g_param_spec_object ("workbench",
                         _("Workbench"),
                         _("Workbench"),
                         GB_TYPE_WORKBENCH,
                         (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);
}

static void
gb_glade_addin_init (GbGladeAddin *self)
{
}

static void
workbench_addin_iface_init (GbWorkbenchAddinInterface *iface)
{
  iface->load = gb_glade_addin_load;
}

void
peas_register_types (PeasObjectModule *module)
{
  gb_glade_addin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              GB_TYPE_WORKBENCH_ADDIN,
                                              GB_TYPE_GLADE_ADDIN);
}

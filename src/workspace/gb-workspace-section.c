/* gb-workspace-section.c
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

#include "gb-workspace-section.h"

G_DEFINE_TYPE(GbWorkspaceSection, gb_workspace_section, GTK_TYPE_BIN)

struct _GbWorkspaceSectionPrivate
{
   GbDocument *current_document;
};

enum
{
   PROP_0,
   PROP_CURRENT_DOCUMENT,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

GbDocument *
gb_workspace_section_get_current_document (GbWorkspaceSection *section)
{
   g_return_val_if_fail(GB_IS_WORKSPACE_SECTION(section), NULL);

   return section->priv->current_document;
}

void
gb_workspace_section_set_current_document (GbWorkspaceSection *section,
                                           GbDocument         *document)
{
   GbWorkspaceSectionPrivate *priv;

   g_return_if_fail(GB_IS_WORKSPACE_SECTION(section));
   g_return_if_fail(!document || GB_IS_DOCUMENT(document));

   priv = section->priv;

   if (priv->current_document) {
      g_object_remove_weak_pointer(G_OBJECT(priv->current_document),
                                   (gpointer *)&priv->current_document);
      priv->current_document = NULL;
   }

   if (document) {
      priv->current_document = document;
      g_object_add_weak_pointer(G_OBJECT(document),
                                (gpointer *)&priv->current_document);
   }

   g_object_notify_by_pspec(G_OBJECT(section),
                            gParamSpecs[PROP_CURRENT_DOCUMENT]);
}

static void
gb_workspace_section_finalize (GObject *object)
{
   GbWorkspaceSectionPrivate *priv;

   priv = GB_WORKSPACE_SECTION(object)->priv;

   if (priv->current_document) {
      g_object_remove_weak_pointer(G_OBJECT(priv->current_document),
                                   (gpointer *)&priv->current_document);
      priv->current_document = NULL;
   }

   G_OBJECT_CLASS(gb_workspace_section_parent_class)->finalize(object);
}

static void
gb_workspace_section_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
   GbWorkspaceSection *section = GB_WORKSPACE_SECTION(object);

   switch (prop_id) {
   case PROP_CURRENT_DOCUMENT:
      g_value_set_object(value,
                         gb_workspace_section_get_current_document(section));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_section_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
   GbWorkspaceSection *section = GB_WORKSPACE_SECTION(object);

   switch (prop_id) {
   case PROP_CURRENT_DOCUMENT:
      gb_workspace_section_set_current_document(section,
                                                g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_workspace_section_class_init (GbWorkspaceSectionClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_workspace_section_finalize;
   object_class->get_property = gb_workspace_section_get_property;
   object_class->set_property = gb_workspace_section_set_property;
   g_type_class_add_private(object_class, sizeof(GbWorkspaceSectionPrivate));

   gParamSpecs[PROP_CURRENT_DOCUMENT] =
      g_param_spec_object("current-document",
                          _("Current Document"),
                          _("The current document for the section."),
                          GB_TYPE_DOCUMENT,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_CURRENT_DOCUMENT,
                                   gParamSpecs[PROP_CURRENT_DOCUMENT]);
}

static void
gb_workspace_section_init (GbWorkspaceSection *section)
{
   section->priv = G_TYPE_INSTANCE_GET_PRIVATE(section,
                                               GB_TYPE_WORKSPACE_SECTION,
                                               GbWorkspaceSectionPrivate);
}

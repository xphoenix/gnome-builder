/* gb-symbol-combo-box.h:
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

#ifndef GB_SYMBOL_COMBO_BOX_H
#define GB_SYMBOL_COMBO_BOX_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GB_TYPE_SYMBOL_COMBO_BOX            (gb_symbol_combo_box_get_type())
#define GB_SYMBOL_COMBO_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SYMBOL_COMBO_BOX, GbSymbolComboBox))
#define GB_SYMBOL_COMBO_BOX_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GB_TYPE_SYMBOL_COMBO_BOX, GbSymbolComboBox const))
#define GB_SYMBOL_COMBO_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GB_TYPE_SYMBOL_COMBO_BOX, GbSymbolComboBoxClass))
#define GB_IS_SYMBOL_COMBO_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GB_TYPE_SYMBOL_COMBO_BOX))
#define GB_IS_SYMBOL_COMBO_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GB_TYPE_SYMBOL_COMBO_BOX))
#define GB_SYMBOL_COMBO_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GB_TYPE_SYMBOL_COMBO_BOX, GbSymbolComboBoxClass))

typedef struct _GbSymbolComboBox        GbSymbolComboBox;
typedef struct _GbSymbolComboBoxClass   GbSymbolComboBoxClass;
typedef struct _GbSymbolComboBoxPrivate GbSymbolComboBoxPrivate;

struct _GbSymbolComboBox
{
   GtkComboBox parent;

   /*< private >*/
   GbSymbolComboBoxPrivate *priv;
};

struct _GbSymbolComboBoxClass
{
   GtkComboBoxClass parent_class;
};

GType gb_symbol_combo_box_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* GB_SYMBOL_COMBO_BOX_H */

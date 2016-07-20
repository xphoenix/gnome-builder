/* ide-editor-layout-stack-controls.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "ide-editor-layout-stack-controls"

#include <egg-binding-group.h>
#include <egg-signal-group.h>
#include <egg-simple-popover.h>
#include <glib/gi18n.h>

#include "editor/ide-editor-frame.h"
#include "editor/ide-editor-frame-private.h"
#include "editor/ide-editor-layout-stack-controls.h"
#include "editor/ide-editor-tweak-widget.h"
#include "editor/ide-editor-view.h"
#include "editor/ide-editor-view-private.h"

struct _IdeEditorLayoutStackControls
{
  GtkBox                parent_instance;

  IdeEditorView        *view;
  EggBindingGroup      *document_bindings;
  EggSignalGroup       *document_signals;

  EggSimplePopover     *goto_line_popover;
  GtkButton            *warning_button;
  EggSimpleLabel       *line_label;
  EggSimpleLabel       *column_label;
  GtkLabel             *range_label;
  GtkMenuButton        *tweak_button;
  IdeEditorTweakWidget *tweak_widget;
};

G_DEFINE_TYPE (IdeEditorLayoutStackControls, ide_editor_layout_stack_controls, GTK_TYPE_BOX)

static gboolean
language_to_string (GBinding     *binding,
                    const GValue *from_value,
                    GValue       *to_value,
                    gpointer      user_data)
{
  GtkSourceLanguage *language;

  g_assert (G_VALUE_HOLDS (from_value, GTK_SOURCE_TYPE_LANGUAGE));
  g_assert (G_VALUE_HOLDS_STRING (to_value));
  g_assert (user_data == NULL);

  language = g_value_get_object (from_value);

  if (language != NULL)
    g_value_set_string (to_value, gtk_source_language_get_name (language));
  else
    g_value_set_string (to_value, _("Plain Text"));

  return TRUE;
}

static void
document_cursor_moved (IdeEditorLayoutStackControls *self,
                       const GtkTextIter            *iter,
                       GtkTextBuffer                *buffer)
{
  IdeSourceView *source_view;
  GtkTextIter bounds;
  GtkTextMark *mark;
  gchar str[32];
  guint line;
  gint column;
  gint column2;

  g_assert (IDE_IS_EDITOR_LAYOUT_STACK_CONTROLS (self));
  g_assert (iter != NULL);
  g_assert (IDE_IS_BUFFER (buffer));

  if (self->view == NULL)
    return;

  source_view = ide_editor_view_get_active_source_view (self->view);

  ide_source_view_get_visual_position (source_view, &line, (guint *)&column);

  mark = gtk_text_buffer_get_selection_bound (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &bounds, mark);

  g_snprintf (str, sizeof str, "%d", line + 1);
  egg_simple_label_set_label (self->line_label, str);

  g_snprintf (str, sizeof str, "%d", column + 1);
  egg_simple_label_set_label (self->column_label, str);

  if (!gtk_widget_has_focus (GTK_WIDGET (source_view)) ||
      gtk_text_iter_equal (&bounds, iter) ||
      (gtk_text_iter_get_line (iter) != gtk_text_iter_get_line (&bounds)))
    {
      gtk_widget_set_visible (GTK_WIDGET (self->range_label), FALSE);
      return;
    }

  /* We have a selection that is on the same line.
   * Lets give some detail as to how long the selection is.
   */
  column2 = gtk_source_view_get_visual_column (GTK_SOURCE_VIEW (source_view), &bounds);

  g_snprintf (str, sizeof str, "%u", ABS (column2 - column));
  gtk_label_set_label (self->range_label, str);
  gtk_widget_set_visible (GTK_WIDGET (self->range_label), TRUE);
}


static void
goto_line_activate (IdeEditorLayoutStackControls *self,
                    const gchar                  *text,
                    EggSimplePopover             *popover)
{
  gint64 value;

  g_assert (IDE_IS_EDITOR_LAYOUT_STACK_CONTROLS (self));
  g_assert (EGG_IS_SIMPLE_POPOVER (popover));

  if (self->view == NULL)
    return;

  if (!ide_str_empty0 (text))
    {
      value = g_ascii_strtoll (text, NULL, 10);

      if ((value > 0) && (value < G_MAXINT))
        {
          GtkTextIter iter;
          GtkTextBuffer *buffer = GTK_TEXT_BUFFER (self->view->document);

          gtk_widget_grab_focus (GTK_WIDGET (self->view->frame1->source_view));
          gtk_text_buffer_get_iter_at_line (buffer, &iter, value - 1);
          gtk_text_buffer_select_range (buffer, &iter, &iter);
          ide_source_view_scroll_to_iter (self->view->frame1->source_view,
                                          &iter, 0.25, TRUE, 1.0, 0.5, TRUE);
        }
    }
}

static gboolean
goto_line_insert_text (IdeEditorLayoutStackControls *self,
                       guint                         position,
                       const gchar                  *chars,
                       guint                         n_chars,
                       EggSimplePopover             *popover)
{
  g_assert (IDE_IS_EDITOR_LAYOUT_STACK_CONTROLS (self));
  g_assert (EGG_IS_SIMPLE_POPOVER (popover));
  g_assert (chars != NULL);

  for (; *chars; chars = g_utf8_next_char (chars))
    {
      if (!g_unichar_isdigit (g_utf8_get_char (chars)))
        return GDK_EVENT_STOP;
    }

  return GDK_EVENT_PROPAGATE;
}

static void
goto_line_changed (IdeEditorLayoutStackControls *self,
                   EggSimplePopover             *popover)
{
  gchar *message;
  const gchar *text;
  GtkTextIter begin;
  GtkTextIter end;

  g_assert (IDE_IS_EDITOR_LAYOUT_STACK_CONTROLS (self));
  g_assert (EGG_IS_SIMPLE_POPOVER (popover));

  if (self->view == NULL)
    return;

  text = egg_simple_popover_get_text (popover);

  gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (self->view->document), &begin, &end);

  if (!ide_str_empty0 (text))
    {
      gint64 value;

      value = g_ascii_strtoll (text, NULL, 10);

      if (value > 0)
        {
          if (value <= gtk_text_iter_get_line (&end) + 1)
            {
              egg_simple_popover_set_message (popover, NULL);
              egg_simple_popover_set_ready (popover, TRUE);
              return;
            }
        }
    }

  /* translators: the user selected a number outside the value range for the document. */
  message = g_strdup_printf (_("Provide a number between 1 and %u"),
                             gtk_text_iter_get_line (&end) + 1);
  egg_simple_popover_set_message (popover, message);
  egg_simple_popover_set_ready (popover, FALSE);

  g_free (message);
}

static void
warning_button_clicked (IdeEditorLayoutStackControls *self,
                        GtkButton                    *button)
{
  IdeSourceView *source_view;

  g_assert (IDE_IS_EDITOR_LAYOUT_STACK_CONTROLS (self));
  g_assert (GTK_IS_BUTTON (button));

  if (self->view == NULL)
    return;

  source_view = ide_editor_view_get_active_source_view (self->view);
  gtk_widget_grab_focus (GTK_WIDGET (source_view));
  g_signal_emit_by_name (source_view, "move-error", GTK_DIR_DOWN);
}

static void
ide_editor_layout_stack_controls_finalize (GObject *object)
{
  IdeEditorLayoutStackControls *self = (IdeEditorLayoutStackControls *)object;

  g_clear_object (&self->document_bindings);
  g_clear_object (&self->document_signals);

  self->view = NULL;

  G_OBJECT_CLASS (ide_editor_layout_stack_controls_parent_class)->finalize (object);
}

static void
ide_editor_layout_stack_controls_class_init (IdeEditorLayoutStackControlsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = ide_editor_layout_stack_controls_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/builder/ui/ide-editor-layout-stack-controls.ui");
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, column_label);
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, goto_line_popover);
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, line_label);
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, range_label);
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, warning_button);
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, tweak_button);
  gtk_widget_class_bind_template_child (widget_class, IdeEditorLayoutStackControls, tweak_widget);
}

static void
ide_editor_layout_stack_controls_init (IdeEditorLayoutStackControls *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (self->goto_line_popover,
                           "activate",
                           G_CALLBACK (goto_line_activate),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->goto_line_popover,
                           "insert-text",
                           G_CALLBACK (goto_line_insert_text),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->goto_line_popover,
                           "changed",
                           G_CALLBACK (goto_line_changed),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->warning_button,
                           "clicked",
                           G_CALLBACK (warning_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);

  self->document_bindings = egg_binding_group_new ();

  egg_binding_group_bind (self->document_bindings, "has-diagnostics",
                          self->warning_button, "visible",
                          G_BINDING_SYNC_CREATE);

  egg_binding_group_bind_full (self->document_bindings, "language",
                               self->tweak_button, "label",
                               G_BINDING_SYNC_CREATE,
                               language_to_string, NULL, NULL, NULL);

  self->document_signals = egg_signal_group_new (IDE_TYPE_BUFFER);

  egg_signal_group_connect_object (self->document_signals,
                                   "cursor-moved",
                                   G_CALLBACK (document_cursor_moved),
                                   self,
                                   G_CONNECT_SWAPPED);
}

void
ide_editor_layout_stack_controls_set_view (IdeEditorLayoutStackControls *self,
                                           IdeEditorView                *view)
{
  g_return_if_fail (IDE_IS_EDITOR_LAYOUT_STACK_CONTROLS (self));
  g_return_if_fail (!view || IDE_IS_EDITOR_VIEW (view));

  if (self->view == view)
    return;

  egg_binding_group_set_source (self->document_bindings, NULL);
  egg_signal_group_set_target (self->document_signals, NULL);

  if (self->view != NULL)
    {
      g_signal_handlers_disconnect_by_func (self->view,
                                            G_CALLBACK (gtk_widget_destroyed),
                                            &self->view);
      self->view = NULL;
    }

  if (view != NULL)
    {
      self->view = view;
      g_signal_connect (view,
                        "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &self->view);
      egg_binding_group_set_source (self->document_bindings, view->document);
      egg_signal_group_set_target (self->document_signals, view->document);
    }
}

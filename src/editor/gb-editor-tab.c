/* gb-editor-tab.c
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
#include <gtksourceview/gtksource.h>
#include <nautilus/nautilus-floating-bar.h>

#include "gb-editor-document.h"
#include "gb-editor-tab.h"
#include "gb-gtk.h"
#include "gb-source-overlay.h"
#include "gb-source-snippet.h"
#include "gb-source-snippet-completion-provider.h"
#include "gb-source-snippets.h"
#include "gb-source-snippets-manager.h"
#include "gb-source-view.h"

struct _GbEditorTabPrivate
{
   GbEditorDocument *document;

   GtkSourceSearchContext *search;
   GtkSourceSearchSettings *search_settings;

   GtkSourceCompletionProvider *snippets_provider;

   GFile *file;

   gboolean is_empty;

   GSettings *settings;
   GSettings *shared_settings;

   GtkWidget *floating_bar;
   GtkWidget *overlay;
   GtkWidget *progress_bar;
   GtkWidget *scroller;
   GtkWidget *search_bar;
   GtkWidget *search_entry;
   GtkWidget *search_overlay;
   GtkWidget *text_view;
   GtkWidget *vbox;

   guint first_change_handler;
};

enum
{
   PROP_0,
   PROP_FILE,
   PROP_FONT,
   PROP_STYLE_SCHEME_NAME,
   LAST_PROP
};

enum
{
   FOCUSED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

G_DEFINE_TYPE_WITH_CODE(GbEditorTab,
                        gb_editor_tab,
                        GB_TYPE_TAB,
                        G_ADD_PRIVATE(GbEditorTab))

gboolean
gb_editor_tab_get_is_empty (GbEditorTab *tab)
{
   g_return_val_if_fail (GB_IS_EDITOR_TAB (tab), FALSE);

   return tab->priv->is_empty;
}

static void
gb_editor_tab_reload_snippets (GbEditorTab       *tab,
                               GtkSourceLanguage *lang)
{
   GbSourceSnippetsManager *manager;
   GbEditorTabPrivate *priv;
   GbSourceSnippets *snippets = NULL;

   g_return_if_fail(GB_IS_EDITOR_TAB(tab));

   priv = tab->priv;

   if (lang) {
      manager = gb_source_snippets_manager_get_default ();
      snippets = gb_source_snippets_manager_get_for_language (manager, lang);
   }

   g_object_set (priv->snippets_provider,
                 "snippets", snippets,
                 NULL);
}

static void
gb_editor_tab_guess_language (GbEditorTab *tab)
{
   GtkSourceLanguageManager *manager;
   GbEditorTabPrivate *priv;
   GtkSourceLanguage *lang;
   gboolean result_uncertain;
   gchar *content_type;
   gchar *filename;

   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   priv = tab->priv;

   if (!priv->file) {
      return;
   }

   manager = gtk_source_language_manager_get_default ();

   filename = g_file_get_path (priv->file);

   content_type = g_content_type_guess (filename, NULL, 0, &result_uncertain);

   if (result_uncertain) {
      g_free (content_type);
      content_type = NULL;
   }

   lang = gtk_source_language_manager_guess_language (manager, filename, content_type);
   gtk_source_buffer_set_language (GTK_SOURCE_BUFFER (priv->document), lang);

   gb_editor_tab_reload_snippets (tab, lang);

   g_free (content_type);
   g_free (filename);
}

GFile *
gb_editor_tab_get_file (GbEditorTab *tab)
{
   g_return_val_if_fail(GB_IS_EDITOR_TAB(tab), NULL);

   return tab->priv->file;
}

void
gb_editor_tab_set_file (GbEditorTab *tab,
                        GFile       *file)
{
   GbEditorTabPrivate *priv;
   gchar *title;

   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   priv = tab->priv;

   if (file) {
      title = g_file_get_basename (file);
      priv->is_empty = FALSE;
   } else {
      title = _("Unsaved Document");
   }

   g_clear_object (&priv->file);
   priv->file = file ? g_object_ref (file) : NULL;

   gb_tab_set_title (GB_TAB (tab), title);
   gb_editor_tab_guess_language (tab);

   g_object_notify_by_pspec (G_OBJECT (tab), gParamSpecs[PROP_FILE]);
}

static void
gb_editor_tab_save_to_file_cb (GObject       *object,
                               GAsyncResult  *result,
                               gpointer       user_data)
{
   GbEditorDocument *document = (GbEditorDocument *)object;
   GbEditorTab *tab = user_data;
   GError *error = NULL;

   g_return_if_fail(GB_IS_EDITOR_TAB (tab));

   if (!gb_editor_document_save_to_file_finish (document, result, &error)) {
      g_warning ("%s", error->message);
      g_error_free (error);
      g_object_unref (tab);
      return;
   }

   gb_gtk_widget_hide_in_idle (350, tab->priv->progress_bar);

   g_object_unref (tab);
}

void
gb_editor_tab_save (GbEditorTab *tab)
{
   GbEditorTabPrivate *priv;
   GtkWidget *toplevel;
   GtkWidget *dialog;
   GFile *file = NULL;

   g_return_if_fail (GB_IS_EDITOR_TAB(tab));

   priv = tab->priv;

   if (!priv->file) {
      toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));

      dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG,
                             "action", GTK_FILE_CHOOSER_ACTION_SAVE,
                             "local-only", FALSE,
                             "title", _("Save"),
                             "transient-for", toplevel,
                             "visible", TRUE,
                             NULL);

      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              _("Cancel"), GTK_RESPONSE_CANCEL,
                              _("Save"), GTK_RESPONSE_OK,
                              NULL);

      if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog))) {
         file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
      }

      gtk_widget_destroy (dialog);

      if (!file) {
         return;
      }

      gb_editor_tab_set_file (tab, file);

      g_clear_object (&file);
   }

   gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->progress_bar), 0.0);
   gtk_widget_show (priv->progress_bar);

   gb_editor_document_save_to_file_async (priv->document,
                                          priv->file,
                                          NULL,
                                          gb_gtk_progress_bar_file_progress_callback,
                                          priv->progress_bar,
                                          gb_editor_tab_save_to_file_cb,
                                          g_object_ref (tab));

   priv->is_empty = FALSE;
}

static void
gb_editor_tab_load_from_file_cb (GObject       *object,
                                 GAsyncResult  *result,
                                 gpointer       user_data)
{
   GbEditorDocument *document = (GbEditorDocument *)object;
   GbEditorTab *tab = user_data;
   GError *error = NULL;

   g_return_if_fail(GB_IS_EDITOR_TAB (tab));

   if (!gb_editor_document_load_from_file_finish (document, result, &error)) {
      g_warning ("%s", error->message);
      g_error_free (error);
   }

   g_object_unref (tab);
}

void
gb_editor_tab_open (GbEditorTab *tab,
                    GFile       *file)
{
   GbEditorTabPrivate *priv;
   GtkWidget *dialog;
   GtkWidget *toplevel;
   GFile *file_copy = NULL;

   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   priv = tab->priv;

   if (!file) {
      toplevel = gtk_widget_get_toplevel (GTK_WIDGET (tab));

      dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG,
                             "action", GTK_FILE_CHOOSER_ACTION_OPEN,
                             "local-only", FALSE,
                             "title", _("Open"),
                             "transient-for", toplevel,
                             "visible", TRUE,
                             NULL);

      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              _("Cancel"), GTK_RESPONSE_CANCEL,
                              _("Open"), GTK_RESPONSE_OK,
                              NULL);

      if (GTK_RESPONSE_OK == gtk_dialog_run (GTK_DIALOG (dialog))) {
         file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (dialog));
         file_copy = file;
      }

      gtk_widget_destroy (dialog);

      if (!file) {
         return;
      }
   }

   gb_editor_tab_set_file (tab, file);

   gb_editor_document_load_from_file_async (priv->document,
                                            file,
                                            NULL,
#if 0
                                            gb_gtk_progress_bar_file_progress_callback,
                                            priv->progress_bar,
#else
                                            NULL,
                                            NULL,
#endif
                                            gb_editor_tab_load_from_file_cb,
                                            g_object_ref (tab));

   g_clear_object (&file_copy);

   priv->is_empty = FALSE;
}

static void
gb_editor_tab_apply_settings (GbEditorTab *tab)
{
   GbEditorTabPrivate *priv;
   GtkSourceLanguage *lang;
   const gchar *name;
   gchar *path;

   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   priv = tab->priv;

   g_clear_object (&priv->settings);

   lang = gtk_source_buffer_get_language (GTK_SOURCE_BUFFER (priv->document));

   if (lang) {
      name = gtk_source_language_get_id (lang);
   } else {
      name = "plaintext";
   }

   path = g_strdup_printf ("/org/gnome/builder/preferences/editor/language/%s/", name);
   priv->settings = g_settings_new_with_path ("org.gnome.builder.preferences.editor.language", path);
   g_free (path);

   g_settings_bind (priv->settings, "highlight-matching-brackets", priv->document, "highlight-matching-brackets", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "highlight-current-line", priv->text_view, "highlight-current-line", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "insert-spaces-instead-of-tabs", priv->text_view, "insert-spaces-instead-of-tabs", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "right-margin-position", priv->text_view, "right-margin-position", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "show-line-marks", priv->text_view, "show-line-marks", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "show-line-numbers", priv->text_view, "show-line-numbers", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "show-right-margin", priv->text_view, "show-right-margin", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->settings, "tab-width", priv->text_view, "tab-width", G_SETTINGS_BIND_GET);
}

static void
gb_editor_tab_update_location (GbEditorTab *tab)
{
   GbEditorTabPrivate *priv;
   GtkTextBuffer *buffer;
   GtkTextMark *mark;
   GtkTextIter iter;
   gboolean overwrite;
   gchar *text;
   guint line;
   guint column;

   g_return_if_fail(GB_IS_EDITOR_TAB(tab));

   priv = tab->priv;

   buffer = GTK_TEXT_BUFFER(priv->document);

   overwrite = gtk_text_view_get_overwrite(GTK_TEXT_VIEW(priv->text_view));
   mark = gtk_text_buffer_get_insert(buffer);
   gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
   line = gtk_text_iter_get_line(&iter) + 1;
   column = gtk_source_view_get_visual_column(GTK_SOURCE_VIEW(priv->text_view),
                                              &iter) + 1;
   if (!overwrite) {
      text = g_strdup_printf(_("Ln %u, Col %u  INS"), line, column);
   } else {
      text = g_strdup_printf(_("Ln %u, Col %u  REP"), line, column);
   }
   g_object_set(priv->floating_bar, "label", text, NULL);
   g_free(text);

   if (!gtk_widget_get_visible(priv->floating_bar)) {
      gtk_widget_show(priv->floating_bar);
   }
}

static void
gb_editor_tab_update_title (GbEditorTab *tab)
{
   GbEditorTabPrivate *priv;
   gchar *title;

   g_return_if_fail (GB_IS_EDITOR_TAB(tab));

   priv = tab->priv;

   if (priv->file) {
      title = g_file_get_basename (priv->file);
   } else {
      title = g_strdup (_("Unsaved Document"));
   }

   gb_tab_set_title (GB_TAB (tab), title);

   g_free (title);
}

static void
gb_editor_tab_grab_focus (GtkWidget *widget)
{
   g_return_if_fail(GB_IS_EDITOR_TAB(widget));

   gtk_widget_grab_focus(GB_EDITOR_TAB(widget)->priv->text_view);
}

void
gb_editor_tab_find (GbEditorTab *tab)
{
   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (tab->priv->search_bar), TRUE);
   gtk_widget_grab_focus (tab->priv->search_entry);
}

static void
gb_editor_tab_set_font (GbEditorTab *tab,
                        const gchar *font)
{
   PangoFontDescription *font_desc;

   g_return_if_fail(GB_IS_EDITOR_TAB(tab));

   if (!font)
      font = "Monospace";

   font_desc = pango_font_description_from_string (font);

   if (!font_desc)
      return;

   gtk_widget_override_font (GTK_WIDGET (tab->priv->text_view), font_desc);

   pango_font_description_free (font_desc);
}

static void
gb_editor_tab_set_style_scheme_name (GbEditorTab *tab,
                                     const gchar *style_scheme_name)
{
   GtkSourceStyleSchemeManager *manager;
   GtkSourceStyleScheme *style_scheme;
   GbEditorTabPrivate *priv;

   g_return_if_fail(GB_IS_EDITOR_TAB(tab));

   priv = tab->priv;

   if (!style_scheme_name)
      style_scheme_name = "tango";

   manager = gtk_source_style_scheme_manager_get_default ();
   style_scheme = gtk_source_style_scheme_manager_get_scheme (manager, style_scheme_name);
   gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (priv->document), style_scheme);
}

static void
gb_editor_tab_emit_focused (GbEditorTab *tab)
{
   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   if (gtk_widget_has_focus (tab->priv->text_view)) {
      g_signal_emit (tab, gSignals[FOCUSED], 0);
   }
}

static void
gb_editor_tab_search_entry_changed (GbEditorTab    *tab,
                                    GtkSearchEntry *entry)
{
   const gchar *text;

   g_return_if_fail(GB_IS_EDITOR_TAB(tab));
   g_return_if_fail(GTK_IS_ENTRY(entry));

   text = gtk_entry_get_text (GTK_ENTRY (entry));

   gtk_widget_set_visible (tab->priv->search_overlay, text && *text);
}

static void
gb_editor_tab_handle_first_change (GbEditorTab      *tab,
                                   GbEditorDocument *document)
{
   g_return_if_fail (GB_IS_EDITOR_TAB (tab));

   tab->priv->is_empty = FALSE;
}

static void
gb_editor_tab_finalize (GObject *object)
{
   GbEditorTabPrivate *priv = GB_EDITOR_TAB(object)->priv;

   g_clear_object (&priv->document);
   g_clear_object (&priv->file);
   g_clear_object (&priv->search);
   g_clear_object (&priv->search_settings);
   g_clear_object (&priv->snippets_provider);

   G_OBJECT_CLASS (gb_editor_tab_parent_class)->finalize (object);
}

static void
gb_editor_tab_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
   GbEditorTab *tab = GB_EDITOR_TAB(object);

   switch (prop_id) {
   case PROP_FILE:
      g_value_set_object (value, gb_editor_tab_get_file (tab));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_editor_tab_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
   GbEditorTab *tab = GB_EDITOR_TAB(object);

   switch (prop_id) {
   case PROP_FILE:
      gb_editor_tab_set_file (tab, g_value_get_object (value));
      break;
   case PROP_FONT:
      gb_editor_tab_set_font (tab, g_value_get_string (value));
      break;
   case PROP_STYLE_SCHEME_NAME:
      gb_editor_tab_set_style_scheme_name (tab, g_value_get_string (value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_editor_tab_class_init (GbEditorTabClass *klass)
{
   GObjectClass *object_class;
   GtkWidgetClass *widget_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = gb_editor_tab_finalize;
   object_class->get_property = gb_editor_tab_get_property;
   object_class->set_property = gb_editor_tab_set_property;

   widget_class = GTK_WIDGET_CLASS(klass);
   widget_class->grab_focus = gb_editor_tab_grab_focus;

   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("The file for the buffer."),
                          G_TYPE_FILE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);

   gParamSpecs[PROP_FONT] =
      g_param_spec_string("font",
                          _("Font"),
                          _("The font to use."),
                          "Monospace",
                          (G_PARAM_WRITABLE |
                           G_PARAM_CONSTRUCT |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FONT,
                                   gParamSpecs[PROP_FONT]);

   gParamSpecs[PROP_STYLE_SCHEME_NAME] =
      g_param_spec_string("style-scheme-name",
                          _("Style Scheme Name"),
                          _("The name of the style scheme."),
                          "tango",
                          (G_PARAM_WRITABLE |
                           G_PARAM_CONSTRUCT |
                           G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_STYLE_SCHEME_NAME,
                                   gParamSpecs[PROP_STYLE_SCHEME_NAME]);

   gSignals[FOCUSED] = g_signal_new ("focused",
                                     GB_TYPE_EDITOR_TAB,
                                     G_SIGNAL_RUN_LAST,
                                     0,
                                     NULL,
                                     NULL,
                                     g_cclosure_marshal_VOID__VOID,
                                     G_TYPE_NONE,
                                     0);
}

static void
gb_editor_tab_init (GbEditorTab *tab)
{
   GtkSourceCompletion *completion;
   GbEditorTabPrivate *priv;

   priv = tab->priv = gb_editor_tab_get_instance_private(tab);

   priv->is_empty = TRUE;

   priv->document = gb_editor_document_new();

   priv->first_change_handler =
      g_signal_connect_object (priv->document,
                               "changed",
                               G_CALLBACK (gb_editor_tab_handle_first_change),
                               tab,
                               G_CONNECT_SWAPPED);

   priv->search_settings = gtk_source_search_settings_new ();

   priv->search = gtk_source_search_context_new (GTK_SOURCE_BUFFER (priv->document),
                                                 priv->search_settings);

   g_signal_connect_object (priv->document,
                            "notify::file",
                            G_CALLBACK (gb_editor_tab_update_title),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   priv->vbox = g_object_new (GTK_TYPE_BOX,
                              "orientation", GTK_ORIENTATION_VERTICAL,
                              "visible", TRUE,
                              NULL);
   gtk_container_add (GTK_CONTAINER (tab), priv->vbox);

   priv->search_bar = g_object_new (GTK_TYPE_SEARCH_BAR,
                                    "search-mode-enabled", FALSE,
                                    "show-close-button", FALSE,
                                    "visible", TRUE,
                                    NULL);
   gtk_container_add (GTK_CONTAINER (priv->vbox), priv->search_bar);

   priv->search_entry = g_object_new (GTK_TYPE_SEARCH_ENTRY,
                                      "visible", TRUE,
                                      "width-chars", 32,
                                      NULL);
   g_object_bind_property (priv->search_entry, "text",
                           priv->search_settings, "search-text",
                           G_BINDING_SYNC_CREATE);
   g_signal_connect_object (priv->search_entry,
                            "changed",
                            G_CALLBACK (gb_editor_tab_search_entry_changed),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));
   gtk_container_add (GTK_CONTAINER (priv->search_bar), priv->search_entry);

   priv->overlay = g_object_new(GTK_TYPE_OVERLAY,
                                "vexpand", TRUE,
                                "visible", TRUE,
                                NULL);
   gtk_container_add(GTK_CONTAINER(priv->vbox), priv->overlay);

   priv->scroller = g_object_new(GTK_TYPE_SCROLLED_WINDOW,
                                 "visible", TRUE,
                                 NULL);
   gtk_container_add(GTK_CONTAINER(priv->overlay), priv->scroller);

   priv->text_view = g_object_new(GB_TYPE_SOURCE_VIEW,
                                  "buffer", priv->document,
                                  "visible", TRUE,
                                  NULL);
   gtk_container_add(GTK_CONTAINER(priv->scroller), priv->text_view);

   completion = gtk_source_view_get_completion (GTK_SOURCE_VIEW (priv->text_view));

   g_object_set (completion,
                 "show-headers", FALSE,
                 "select-on-show", TRUE,
                 NULL);

   priv->snippets_provider =
      g_object_new (GB_TYPE_SOURCE_SNIPPET_COMPLETION_PROVIDER,
                    "source-view", priv->text_view,
                    NULL);

   gtk_source_completion_add_provider (completion,
                                       priv->snippets_provider,
                                       NULL);

   priv->search_overlay = g_object_new (GB_TYPE_SOURCE_OVERLAY,
                                        "hexpand", TRUE,
                                        "search-context", priv->search,
                                        "source-view", priv->text_view,
                                        "vexpand", TRUE,
                                        "visible", FALSE,
                                        NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->search_overlay);

   priv->progress_bar = g_object_new (GTK_TYPE_PROGRESS_BAR,
                                      "halign", GTK_ALIGN_FILL,
                                      "valign", GTK_ALIGN_START,
                                      NULL);
   gtk_style_context_add_class (gtk_widget_get_style_context (priv->progress_bar),
                                GTK_STYLE_CLASS_OSD);
   gtk_overlay_add_overlay (GTK_OVERLAY (priv->overlay), priv->progress_bar);

   priv->floating_bar = g_object_new(NAUTILUS_TYPE_FLOATING_BAR,
                                     "halign", GTK_ALIGN_END,
                                     "valign", GTK_ALIGN_END,
                                     "visible", TRUE,
                                     NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(priv->overlay), priv->floating_bar);

   g_signal_connect_object (priv->text_view,
                            "toggle-overwrite",
                            G_CALLBACK (gb_editor_tab_update_location),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   g_signal_connect_object (priv->document,
                            "notify::language",
                            G_CALLBACK (gb_editor_tab_apply_settings),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   g_signal_connect_object (priv->document,
                            "delete-range",
                            G_CALLBACK (gb_editor_tab_update_location),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   g_signal_connect_object (priv->document,
                            "insert-text",
                            G_CALLBACK (gb_editor_tab_update_location),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   g_signal_connect_object (priv->document,
                            "mark-set",
                            G_CALLBACK (gb_editor_tab_update_location),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   g_signal_connect_object (priv->text_view,
                            "grab-focus",
                            G_CALLBACK (gb_editor_tab_emit_focused),
                            tab,
                            (G_CONNECT_AFTER | G_CONNECT_SWAPPED));

   priv->shared_settings = g_settings_new ("org.gnome.builder.preferences.editor");

   g_settings_bind (priv->shared_settings, "font", tab, "font", G_SETTINGS_BIND_GET);
   g_settings_bind (priv->shared_settings, "style-scheme", tab, "style-scheme-name", G_SETTINGS_BIND_GET);

   gb_editor_tab_update_location (tab);

   gb_editor_tab_apply_settings (tab);
}

/* gb-source-diff.c
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
#include <glib/gstdio.h>

#include "gb-source-diff.h"

G_DEFINE_TYPE(GbSourceDiff, gb_source_diff, G_TYPE_OBJECT)

#define DEFAULT_TIMEOUT_MSEC 300

struct _GbSourceDiffPrivate
{
   GtkTextBuffer *buffer;
   GFile         *file;
   GHashTable    *line_state;
   guint          line_state_seq;
   guint          seq;
   guint          timeout_handler;
   guint          changed_handler;
};

enum
{
   PROP_0,
   PROP_BUFFER,
   PROP_FILE,
   LAST_PROP
};

enum
{
   CHANGED,
   LAST_SIGNAL
};

static GParamSpec *gParamSpecs[LAST_PROP];
static guint       gSignals[LAST_SIGNAL];

static void
gb_source_diff_set_line_state (GbSourceDiff *diff,
                               GHashTable   *line_state,
                               guint         sequence)
{
   GbSourceDiffPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_DIFF(diff));

   priv = diff->priv;

   if (sequence > priv->line_state_seq) {
      g_clear_pointer(&priv->line_state, g_hash_table_unref);
      priv->line_state = g_hash_table_ref(line_state);
      priv->line_state_seq = sequence;
      g_signal_emit(diff, gSignals[CHANGED], 0);
   }
}

static gboolean
gb_source_diff_result_timeout (gpointer data)
{
   gpointer *closure = data;

   gb_source_diff_set_line_state(closure[0], closure[1],
                                 GPOINTER_TO_INT(closure[2]));

   g_object_unref(closure[0]);
   g_hash_table_unref(closure[1]);
   g_free(closure);

   return FALSE;
}

static void
gb_source_diff_queue_result (GbSourceDiff *diff,
                             GHashTable   *result,
                             guint         sequence)
{
   gpointer *closure;

   closure = g_new0(gpointer, 3);
   closure[0] = g_object_ref(diff);
   closure[1] = g_hash_table_ref(result);
   closure[2] = GINT_TO_POINTER(sequence);

   g_timeout_add(0, gb_source_diff_result_timeout, closure);
}

static inline void
set_line (GHashTable        *hashtable,
          gint               lineno,
          GbSourceDiffState  state)
{
   g_hash_table_replace(hashtable,
                        GINT_TO_POINTER(lineno),
                        GINT_TO_POINTER(state));
}

static void
parse_line (const gchar *c,
            gint        *src_begin,
            gint        *src_end,
            gchar       *mode,
            gint        *dst_begin,
            gint        *dst_end)
{
   gint vars[4] = { -1, -1, -1, -1 };
   gint i = 0;

   *mode = 0;

   for (; i < 4 && *c != '\n' && *c; c++) {
      switch (*c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         if (vars[i] == -1) {
            vars[i] = 0;
         }
         vars[i] = (vars[i] * 10) + (*c - '0');
         break;
      case 'a':
      case 'c':
      case 'd':
         *mode = *c;
         i = MAX(1, i);
      case ',':
         i++;
         break;
      default:
         break;
      }
   }

   if (vars[1] == -1) {
      vars[1] = vars[0];
   }

   if (vars[3] == -1) {
      vars[3] = vars[2];
   }

   *src_begin = vars[0];
   *src_end = vars[1];
   *dst_begin = vars[2];
   *dst_end = vars[3];
}

static void
gb_source_diff_parse_line (GbSourceDiff *diff,
                           GString      *str,
                           GHashTable   *hashtable)
{
   gchar mode = 0;
   guint i;
   gint src_begin = 0;
   gint src_end = 0;
   gint dst_begin = 0;
   gint dst_end = 0;

   if (g_ascii_isdigit(*str->str)) {
      parse_line(str->str, &src_begin, &src_end, &mode, &dst_begin, &dst_end);
      for (i = dst_begin; i <= dst_end; i++) {
         switch (mode) {
         case 'a':
            set_line(hashtable, i, GB_SOURCE_DIFF_ADDED);
            break;
         case 'c':
            set_line(hashtable, i, GB_SOURCE_DIFF_CHANGED);
         case 'd':
            /* TODO: Can we track delete? Perhaps just show a half
             *       red line to signify delete?
             */
            break;
         default:
            break;
         }
      }
   }
}

static gpointer
gb_source_diff_worker (gpointer data)
{
   gpointer *closure = data;
   GbSourceDiff *diff;
   GIOChannel *channel;
   GHashTable *hashtable = NULL;
   GIOStatus status;
   GString *str = NULL;
   GError *error = NULL;
   gssize r;
   gsize to_write;
   gchar *working_dir;
   gchar *path;
   gchar *text;
   gchar **argv = NULL;
   gchar **env = NULL;
   guint sequence;
   gint standard_input = -1;
   gint standard_output = -1;
   GPid child_pid = 0;

   g_assert(data);

   diff = closure[0];
   path = closure[1];
   text = closure[2];
   sequence = GPOINTER_TO_INT(closure[3]);

   g_assert(GB_IS_SOURCE_DIFF(diff));
   g_assert(path);
   g_assert(text);

   working_dir = g_path_get_dirname(path);
   str = g_string_new(NULL);

   /*
    * TODO: I think we want to always use `git diff', but we should check to
    *       ensure we are within a git tree. For the case of just opening
    *       files that are not part of a project, we still want to work.
    */
   argv = g_new0(gchar*, 5);
   argv[0] = (gchar *)"diff";
   argv[1] = (gchar *)"-d";
   argv[2] = path;
   argv[3] = (gchar *)"-";
   argv[4] = NULL;

   if (!g_spawn_async_with_pipes(working_dir,
                                 argv,
                                 env,
                                 (G_SPAWN_SEARCH_PATH |
                                  G_SPAWN_STDERR_TO_DEV_NULL),
                                 NULL,
                                 NULL,
                                 &child_pid,
                                 &standard_input,
                                 &standard_output,
                                 NULL,
                                 &error)) {
      g_printerr("%s\n", error->message);
      g_error_free(error);
      goto cleanup;
   }

   to_write = strlen(text);
   while (to_write) {
      r = write(standard_input, text, to_write);
      if (r == -1) {
         g_close(standard_input, NULL);
         goto failure;
      }
      to_write -= r;
      text += r;
   }

   g_close(standard_input, NULL);

   channel = g_io_channel_unix_new(standard_output);
   g_io_channel_set_close_on_unref(channel, FALSE);
   g_assert(channel);

   hashtable = g_hash_table_new(g_direct_hash, g_direct_equal);

again:
   status = g_io_channel_read_line_string(channel, str, NULL, &error);
   switch (status) {
   case G_IO_STATUS_ERROR:
      g_io_channel_unref(channel);
      g_printerr("%s\n", error->message);
      g_clear_error(&error);
      goto failure;
   case G_IO_STATUS_NORMAL:
      gb_source_diff_parse_line(diff, str, hashtable);
      g_string_truncate(str, 0);
      goto again;
   case G_IO_STATUS_EOF:
      if (str->len) {
         gb_source_diff_parse_line(diff, str, hashtable);
      }
      break;
   case G_IO_STATUS_AGAIN:
      goto again;
   default:
      g_assert_not_reached();
      break;
   }

   g_io_channel_unref(channel);

   gb_source_diff_queue_result(diff, hashtable, sequence);

failure:
   g_close(standard_output, NULL);

cleanup:
   g_free(argv);
   g_free(working_dir);
   g_object_unref(closure[0]);
   g_free(closure[1]);
   g_free(closure[2]);
   g_free(closure);
   g_string_free(str, TRUE);
   g_clear_pointer(&hashtable, g_hash_table_unref);

   return NULL;
}

static gboolean
gb_source_diff_parse_timeout (gpointer data)
{
   GbSourceDiffPrivate *priv;
   GbSourceDiff *diff = data;
   GtkTextIter begin;
   GtkTextIter end;
   gpointer *closure;
   gchar *path;
   gchar *text;

   g_return_val_if_fail(GB_IS_SOURCE_DIFF(diff), FALSE);

   priv = diff->priv;

   /*
    * Clear the parse handler so another may be queued.
    */
   priv->timeout_handler = 0;

   /*
    * Make sure we have a local file to work with still.
    */
   if (!priv->file || !priv->buffer || !(path = g_file_get_path(priv->file))) {
      return FALSE;
   }

   gtk_text_buffer_get_bounds(priv->buffer, &begin, &end);
   text = gtk_text_buffer_get_text(priv->buffer, &begin, &end, FALSE);

   closure = g_new0(gpointer, 4);
   closure[0] = g_object_ref(diff);
   closure[1] = path;
   closure[2] = text;
   closure[3] = GINT_TO_POINTER(++priv->seq);

   g_thread_new("gb-source-diff-worker",
                gb_source_diff_worker,
                closure);

   return FALSE;
}

void
gb_source_diff_queue_parse (GbSourceDiff *diff)
{
   GbSourceDiffPrivate *priv = diff->priv;

   if (!priv->timeout_handler && priv->file) {
      priv->timeout_handler = g_timeout_add(DEFAULT_TIMEOUT_MSEC,
                                            gb_source_diff_parse_timeout,
                                            diff);
   }
}

static void
gb_source_diff_buffer_changed (GtkTextBuffer *buffer,
                               GbSourceDiff  *diff)
{
   g_assert(GTK_IS_TEXT_BUFFER(buffer));
   g_assert(GB_IS_SOURCE_DIFF(diff));
   gb_source_diff_queue_parse(diff);
}

GbSourceDiff *
gb_source_diff_new (GFile         *file,
                    GtkTextBuffer *buffer)
{
   g_return_val_if_fail(!file || G_IS_FILE(file), NULL);
   g_return_val_if_fail(!buffer || GTK_IS_TEXT_BUFFER(buffer), NULL);

   return g_object_new(GB_TYPE_SOURCE_DIFF,
                       "file", file,
                       "buffer", buffer,
                       NULL);
}

GtkTextBuffer *
gb_source_diff_get_buffer (GbSourceDiff *diff)
{
   g_return_val_if_fail(GB_IS_SOURCE_DIFF(diff), NULL);
   return diff->priv->buffer;
}

void
gb_source_diff_set_buffer (GbSourceDiff  *diff,
                           GtkTextBuffer *buffer)
{
   GbSourceDiffPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_DIFF(diff));
   g_return_if_fail(!buffer || GTK_IS_TEXT_BUFFER(buffer));

   priv = diff->priv;

   if (priv->buffer != buffer) {
      if (priv->changed_handler) {
         g_signal_handler_disconnect(priv->buffer, priv->changed_handler);
         priv->changed_handler = 0;
      }
      g_clear_object(&priv->buffer);
      priv->buffer = buffer ? g_object_ref(buffer) : NULL;
      priv->changed_handler =
         g_signal_connect(priv->buffer,
                          "changed",
                          G_CALLBACK(gb_source_diff_buffer_changed),
                          diff);
      g_object_notify_by_pspec(G_OBJECT(diff), gParamSpecs[PROP_BUFFER]);
      gb_source_diff_queue_parse(diff);
   }
}

/**
 * gb_source_diff_get_line_state:
 * @diff: (in): A #GbSourceDiff.
 * @lineno: The line number indexed from 1.
 *
 * Gets the state of the line matching @lineno. The first line number is
 * 1, not zero.
 *
 * Returns: A #GbSourceDiffState.
 */
GbSourceDiffState
gb_source_diff_get_line_state (GbSourceDiff *diff,
                               guint         lineno)
{
   GbSourceDiffPrivate *priv;
   GbSourceDiffState state = GB_SOURCE_DIFF_SAME;
   gpointer statep;

   g_return_val_if_fail(GB_IS_SOURCE_DIFF(diff), 0);
   g_return_val_if_fail(lineno, 0);

   priv = diff->priv;

   if (priv->line_state) {
      statep = g_hash_table_lookup(priv->line_state,
                                   GINT_TO_POINTER(lineno));
      state = GPOINTER_TO_INT(statep);
   }

   return state;
}

GFile *
gb_source_diff_get_file (GbSourceDiff *diff)
{
   g_return_val_if_fail(GB_IS_SOURCE_DIFF(diff), NULL);
   return diff->priv->file;
}

void
gb_source_diff_set_file (GbSourceDiff *diff,
                         GFile        *file)
{
   GbSourceDiffPrivate *priv;

   g_return_if_fail(GB_IS_SOURCE_DIFF(diff));
   g_return_if_fail(!file || G_IS_FILE(file));

   priv = diff->priv;

   if (file != priv->file) {
      g_clear_object(&priv->file);
      priv->file = file ? g_object_ref(file) : NULL;
      g_object_notify_by_pspec(G_OBJECT(diff), gParamSpecs[PROP_FILE]);
      gb_source_diff_queue_parse(diff);
   }
}

static void
gb_source_diff_dispose (GObject *object)
{
   GbSourceDiffPrivate *priv = GB_SOURCE_DIFF(object)->priv;

   if (priv->timeout_handler) {
      g_source_remove(priv->timeout_handler);
      priv->timeout_handler = 0;
   }

   if (priv->changed_handler) {
      g_signal_handler_disconnect(priv->buffer, priv->changed_handler);
      priv->changed_handler = 0;
   }

   g_clear_pointer(&priv->line_state, g_hash_table_unref);
   g_clear_object(&priv->buffer);
   g_clear_object(&priv->file);

   G_OBJECT_CLASS(gb_source_diff_parent_class)->dispose(object);
}

static void
gb_source_diff_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
   GbSourceDiff *diff = GB_SOURCE_DIFF(object);

   switch (prop_id) {
   case PROP_BUFFER:
      g_value_set_object(value, gb_source_diff_get_buffer(diff));
      break;
   case PROP_FILE:
      g_value_set_object(value, gb_source_diff_get_file(diff));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_diff_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
   GbSourceDiff *diff = GB_SOURCE_DIFF(object);

   switch (prop_id) {
   case PROP_BUFFER:
      gb_source_diff_set_buffer(diff, g_value_get_object(value));
      break;
   case PROP_FILE:
      gb_source_diff_set_file(diff, g_value_get_object(value));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
gb_source_diff_class_init (GbSourceDiffClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->dispose = gb_source_diff_dispose;
   object_class->get_property = gb_source_diff_get_property;
   object_class->set_property = gb_source_diff_set_property;
   g_type_class_add_private(object_class, sizeof(GbSourceDiffPrivate));

   gParamSpecs[PROP_BUFFER] =
      g_param_spec_object("buffer",
                          _("Buffer"),
                          _("The GtkTextBuffer to observe."),
                          GTK_TYPE_TEXT_BUFFER,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_BUFFER,
                                   gParamSpecs[PROP_BUFFER]);

   gParamSpecs[PROP_FILE] =
      g_param_spec_object("file",
                          _("File"),
                          _("The file to observe."),
                          G_TYPE_FILE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
   g_object_class_install_property(object_class, PROP_FILE,
                                   gParamSpecs[PROP_FILE]);

   gSignals[CHANGED] = g_signal_new("changed",
                                    GB_TYPE_SOURCE_DIFF,
                                    G_SIGNAL_RUN_FIRST,
                                    0,
                                    NULL,
                                    NULL,
                                    g_cclosure_marshal_VOID__VOID,
                                    G_TYPE_NONE,
                                    0);
}

static void
gb_source_diff_init (GbSourceDiff *diff)
{
   diff->priv = G_TYPE_INSTANCE_GET_PRIVATE(diff,
                                            GB_TYPE_SOURCE_DIFF,
                                            GbSourceDiffPrivate);
}

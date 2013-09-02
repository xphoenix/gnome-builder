/* snippet-parser.c
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
#include <stdlib.h>
#include <string.h>

#include "snippet-parser.h"

struct _SnippetParser
{
   SnippetParserFunc  func;
   gpointer           user_data;

   gint               lineno;

   GList             *chunks;

   gchar             *cur_name;
   GString           *cur_text;
};

SnippetParser *
snippet_parser_new (SnippetParserFunc func,
                    gpointer          user_data)
{
   SnippetParser *parser;

   parser = g_new0(SnippetParser, 1);
   parser->func = func;
   parser->user_data = user_data;
   parser->lineno = -1;
   parser->cur_text = g_string_new(NULL);

   return parser;
}

void
snippet_parser_free (SnippetParser *parser)
{
   g_clear_pointer(&parser->cur_name, g_free);

   g_string_free(parser->cur_text, TRUE);
   parser->cur_text = NULL;

   g_list_foreach(parser->chunks, (GFunc)g_object_unref, NULL);
   g_list_free(parser->chunks);
   parser->chunks = NULL;

   g_free(parser);
}

void
snippet_parser_flush_chunk (SnippetParser *parser)
{
   GbSourceSnippetChunk *chunk;

   if (parser->cur_text->len) {
      chunk = gb_source_snippet_chunk_new();
      gb_source_snippet_chunk_set_text(chunk, parser->cur_text->str);
      parser->chunks = g_list_append(parser->chunks, chunk);
      g_string_truncate(parser->cur_text, 0);
   }
}

void
snippet_parser_finish (SnippetParser *parser)
{
   GbSourceSnippet *snippet;
   GList *iter;

   if (parser->cur_name) {
      snippet_parser_flush_chunk(parser);
      snippet = gb_source_snippet_new(parser->cur_name);
      for (iter = parser->chunks; iter; iter = iter->next) {
#if 0
         g_printerr ("%s:  Tab: %02d  Link: %02d  Text: %s\n",
                     parser->cur_name,
                     gb_source_snippet_chunk_get_tab_stop(iter->data),
                     gb_source_snippet_chunk_get_linked_chunk(iter->data),
                     gb_source_snippet_chunk_get_text(iter->data));
#endif
         gb_source_snippet_add_chunk(snippet, iter->data);
      }
      parser->func(snippet, parser->user_data);
      g_object_unref(snippet);
   }

   g_clear_pointer(&parser->cur_name, g_free);

   g_string_truncate(parser->cur_text, 0);

   g_list_foreach(parser->chunks, (GFunc)g_object_unref, NULL);
   g_list_free(parser->chunks);
   parser->chunks = NULL;
}

static void
snippet_parser_do_part_simple (SnippetParser *parser,
                               const gchar   *line)
{
   g_string_append(parser->cur_text, line);
}

static void
snippet_parser_do_part_n (SnippetParser *parser,
                          gint           n,
                          const gchar   *inner)
{
   GbSourceSnippetChunk *chunk;

   chunk = gb_source_snippet_chunk_new();
   gb_source_snippet_chunk_set_text(chunk, inner);
   gb_source_snippet_chunk_set_tab_stop(chunk, n);
   parser->chunks = g_list_append(parser->chunks, chunk);
}

static void
snippet_parser_do_part_linked (SnippetParser *parser,
                               gint           n)
{
   GbSourceSnippetChunk *chunk;

   chunk = gb_source_snippet_chunk_new();
   gb_source_snippet_chunk_set_text(chunk, NULL);
   gb_source_snippet_chunk_set_linked_chunk(chunk, n);
   parser->chunks = g_list_append(parser->chunks, chunk);
}

static gboolean
parse_variable (const gchar  *line,
                gint         *n,
                gchar       **inner,
                const gchar **endptr)
{
   gboolean has_inner = FALSE;
   char *end;
   gint brackets;
   gint i;

   *n = -1;
   *inner = NULL;
   *endptr = NULL;

   g_assert(*line == '$');

   line++;

   *endptr = line;

   if (!*line) {
      *endptr = NULL;
      return FALSE;
   }

   if (*line == '{') {
      has_inner = TRUE;
      line++;
   }

   if (!g_ascii_isdigit(*line)) {
      return FALSE;
   }

   errno = 0;
   *n = strtol(line, &end, 10);
   if (errno == ERANGE) {
      return FALSE;
   } else if (*n < 1) {
      return FALSE;
   }

   line = end;

   if (has_inner) {
      if (*line == ':') {
         line++;
      }

      brackets = 1;

      for (i = 0; line[i]; i++) {
         switch (line[i]) {
         case '{':
            brackets++;
            break;
         case '}':
            brackets--;
            break;
         default:
            break;
         }

         if (!brackets) {
            *inner = g_strndup(line, i);
            *endptr = &line[i+1];
            return TRUE;
         }
      }

      return FALSE;
   }

   *endptr = line;

   return TRUE;
}

void
snippet_parser_do_part (SnippetParser *parser,
                        const gchar   *line)
{
   const gchar *dollar;
   gchar *str;
   gchar *inner;
   gint n;

   g_assert(line);
   g_assert(*line == '\t');

   line++;

again:
   if (!(dollar = strchr(line, '$'))) {
      snippet_parser_do_part_simple(parser, line);
      return;
   }

   /*
    * Parse up to the next $ as a simple.
    * If it is $N or ${N} then it is a linked chunk w/o tabstop.
    * If it is ${N:""} then it is a chunk w/ tabstop.
    */

parse_simple:
   if (!*line) {
      return;
   }

   g_assert(dollar >= line);

   if (dollar != line) {
      str = g_strndup(line, (dollar - line));
      snippet_parser_do_part_simple(parser, str);
      g_free(str);
   }

parse_dollar:
   if (!parse_variable(dollar, &n, &inner, &line)) {
      snippet_parser_do_part_simple(parser, dollar);
      return;
   }

#if 0
   g_printerr("Parse Variable: N=%d  inner=\"%s\"\n", n, inner);
   g_printerr("  Left over: %s\n", line);
#endif

   snippet_parser_flush_chunk(parser);

   if (inner) {
      snippet_parser_do_part_n(parser, n, inner);
      g_free(inner);
   } else {
      snippet_parser_do_part_linked(parser, n);
   }

   if (line) {
      if (!*line) {
         return;
      } else if (*line == '$') {
         dollar = line;
         goto parse_dollar;
      } else {
         goto again;
      }
   }
}

void
snippet_parser_do_snippet (SnippetParser *parser,
                           const gchar   *line)
{
   parser->cur_name = g_strstrip(g_strdup(&line[8]));
}

void
snippet_parser_feed_line (SnippetParser *parser,
                          const gchar   *line)
{
   g_assert(parser);
   g_assert(line);

   parser->lineno++;

   switch (*line) {
   case '#':
      break;
   case '\t':
      snippet_parser_do_part(parser, line);
      break;
   case 's':
      if (g_str_has_prefix(line, "snippet")) {
         snippet_parser_finish(parser);
         snippet_parser_do_snippet(parser, line);
         break;
      }
      /* Fall through */
   default:
      g_warning("Invalid snippet at line %d", parser->lineno);
      break;
   }
}

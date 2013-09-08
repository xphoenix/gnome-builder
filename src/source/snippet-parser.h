/* snippet-parser.h
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

#ifndef SNIPPET_PARSER_H
#define SNIPPET_PARSER_H

#include <glib.h>

#include "gb-source-snippet.h"

G_BEGIN_DECLS

typedef struct _SnippetParser SnippetParser;

typedef void (*SnippetParserFunc) (GbSourceSnippet *snippet,
                                   gpointer         user_data);

SnippetParser *snippet_parser_new       (SnippetParserFunc  func,
                                         gpointer           user_data);
void           snippet_parser_feed_line (SnippetParser     *parser,
                                         const gchar       *line);
void           snippet_parser_free      (SnippetParser     *parser);
void           snippet_parser_finish    (SnippetParser     *parser);

G_END_DECLS

#endif /* SNIPPET_PARSER_H */

/*
 * errors.c -- Error handling.
 *
 * https://github.com/fontseca/lexemn
 *
 * Copyright (C) 2026 by Jeremy Fonseca <fontseca.dev@outlook.com>
 *
 * This file is part of Lexemn.
 *
 * Lexemn is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Lexemn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Lexemn. If not, see <https://www.gnu.org/licenses/>.
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <tgmath.h>
#include <math.h>
#include <string.h>

#include "loc.h"
#include "lexemn.h"

/* Diagnostic severity level for reported messages.  */
enum diagnostic_level
{
    /* Any kind of error that can occur.  */
    DL_ERROR,

    /* Non-fatal warning about suspicious constructs.  */
    DL_WARNING,

    /* Contextual informational note attached to a previous diagnostic.  */
    DL_NOTE,

    /* Actionable suggestion.  */
    DL_HINT,

    /* Reset marker to restore default color.  */
    DL_RESET,

    /* Sentinel to count diagnostics.  */
    DL_MAX
};

/* Maps each DIAGNOSTIC_LEVEL enum value to its human-readable string label
   and ANSI color code sequence.  */
static const struct
{ char *level; char *color; }
lookup[DL_MAX] =
{
    { .level = "error",   .color = "\033[1;31m" },
    { .level = "warning", .color = "\033[1;33m" },
    { .level = "note",    .color = "\033[1;36m" },
    { .level = "hint",    .color = "\033[1;32m" },
    { .level = "",        .color = "\033[0m"    },
};

/* Locate the starting memory address of line number LINE within the buffer
   of PAGE. */
static char const *
lookup_line_start(struct lexemn_page const *page, uint32_t line)
{
    char const *line_start = (char *)page->buf;
    char const *buf_end = line_start + page->file.st.st_size;
    uint32_t current_line = 1;
    while (current_line < line && line_start < buf_end)
    {
        size_t remaining = (size_t)(buf_end - line_start);
        char const *nl = (char *)memchr(line_start, '\n', remaining);
        if (!nl)
            break; /* No more new lines.  */
        line_start = 1 + nl;
        ++current_line;
    }
    return current_line == line && line_start <= buf_end
            ? line_start
            : nullptr;
}

/* Print a formatted diagnostic message for context LEXEMN. DIAGNOSTIC specifies
   severity level (DL_ERROR or DL_WARNING).  LOC supplies encoded file, line,
   and column coordinates, adjusted by character offset OFF.  REASON is a
   printf-style format string rendered using variadic arguments AP.  */
__attribute__((format(printf, 5, 0)))
static void
diagnose(struct lexemn const *lexemn, enum diagnostic_level diagnostic,
                loc_t loc, uint32_t off, char const *reason, va_list ap)
{
    uint32_t file = loc_file(loc);
    uint32_t line = loc_line(loc);
    uint32_t column = loc_column(loc);
    struct lexemn_page const *page = lexemn->p_vec + file;
    char const *where = page->file.path;
    FILE *sink = stderr;

    /* Display the `@shell:1:9: error:' component.  */

    fprintf(sink, "\033[1m%s\033[0m:%d:%d: %s%s:%s ",
        where,
        line,
        column + off,
        lookup[diagnostic].color,
        lookup[diagnostic].level,
        lookup[DL_RESET].color);

    /* Display the actual diagnostic message.  */

    vfprintf(sink, reason, ap);
    fprintf(sink, "\n");
    va_end(ap);

    /* Display the offensive line from source code.  */

    uint32_t digits = 1 + (uint32_t)floor(log10(line));
    uint32_t pad = 2;
    uint32_t width = digits + pad * 2;
    if (lexemn->lex_stat.is_shell)
        fprintf(sink, "%*s| ", 1 + pad * 2, "");
    else if (lexemn->lex_stat.is_fs)
        fprintf(sink, "%*s%d%*s| ", pad, "", line, pad, "");

    /* Find the offending line.  */

    char const *line_start = nullptr;
    if (lexemn->lex_stat.is_shell)
        line_start = (char *)lexemn->p_head->line_start;
    else if (lexemn->lex_stat.is_fs)
        line_start = lookup_line_start(lexemn->p_head, line);

    /* Display it and mark its offending character if DL_ERROR.  */

    for (char const *c = line_start; *c != '\n' && *c != '\0'; ++c)
    {
        bool const at_offensive_char = c == column + off + line_start - 1;
        if (at_offensive_char && diagnostic == DL_ERROR)
            fprintf(sink, "%s", lookup[DL_ERROR].color);
        fputc(*c, sink);
        if (at_offensive_char)
            fprintf(sink, "%s", lookup[DL_RESET].color);
    }

    fputc('\n', sink);

    /* Display margin alignment before the vertical bar.  */

    if (lexemn->lex_stat.is_shell)
        fprintf(sink, "%*s| ", 1 + pad * 2, "");
    else if (lexemn->lex_stat.is_fs)
        fprintf(sink, "%*s| ", width, "");

    /* Display leading whitespaces up to column start.  Tabs are preserved
       from source buffer so everything stays aligned.  */

    for (uint32_t col = 0; col < column + off - 1; ++col)
        fputc(line_start[col] == '\t' ? '\t' : ' ', sink);

    /* Display the caret that points at the offending point.  */

    fprintf(sink, "%s^%s",
        lookup[diagnostic].color,
            lookup[DL_RESET].color);
    fputc('\n', sink);
}

__attribute__((format(printf, 4, 5)))
void
error_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...)
{
    va_list ap;
    va_start(ap);
    diagnose(lexemn, DL_ERROR, loc, off, reason, ap);
    va_end(ap);
}

__attribute__((format(printf, 4, 5)))
void
warning_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...)
{
    va_list ap;
    va_start(ap);
    diagnose(lexemn, DL_WARNING, loc, off, reason, ap);
    va_end(ap);
}

__attribute__((format(printf, 4, 5)))
void
note_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...)
{
    va_list ap;
    va_start(ap);
    diagnose(lexemn, DL_NOTE, loc, off, reason, ap);
    va_end(ap);
}

__attribute__((format(printf, 4, 5)))
void
hint_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...)
{
    va_list ap;
    va_start(ap);
    diagnose(lexemn, DL_HINT, loc, off, reason, ap);
    va_end(ap);
}

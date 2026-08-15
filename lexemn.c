/*
 * lexemn.c -- Lexemn entry point.
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

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lexer.h"

/* Forward declarations.  */
static char *
stripwhite(char *);

/* When non-zero, this global means the user is done using this program.  */
static int done;

int
main(int const argc,
            char const **argv)
{
    char *line, *s;

    setlocale(LC_ALL, "");

    /* Loop reading and executing lines until the user quits.  */
    while (0 == done)
    {
        line = readline("(lexemn) ");

        if (!line)
            break;

        /* Remove leading and trailing whitespace from the line.
           Then, if there is anything left, add it to the history list
           and execute it.  */
        s = stripwhite(line);

        /* Make sure the string is null-terminated.  */
        if (s[strlen(s) + 1] != '\0')
            s[strlen(s) + 1] = '\0';

        if (*s)
        {
            struct lexer_t   lexer;
            struct tstream_t stream = { 0 };

            lex_setup(&lexer, (char unsigned const *)s);
            lex_start(&lexer, &stream);
            free(stream.tokens);
            add_history(s);
        }

        free(line);
    }

    return EXIT_SUCCESS;
}

/* Strip whitespaces from the start and the end of STRING.  Return a pointer
   into STRING.  */
static char *
stripwhite(char *string)
{
    char *s, *t;

    s = string;
    while (whitespace(*s))
        ++s;

    if (*s == '\0')
        return s;

    t = s + strlen(s) - 1;
    while (t > s && whitespace(*t))
        --t;
    *++t = '\0';

    return s;
}

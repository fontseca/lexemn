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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libgen.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "lex.h"
#include "lexemn.h"

/* Forward declarations.  */
void lexemn_cleanup(struct lexemn *);
extern void lex_start(struct lexemn *);

/* When non-zero, this global means the user is done using this program.  */
static int done;

/* Initialize the runtime context and tracking structures in LEXEMN.  */
void
lexemn_init(struct lexemn *const lexemn)
{
    /* Array of pages (files) to scan.  */

    lexemn->p_size = 0;
    lexemn->p_capacity = 1; /* Most of the cases (scanning from shell).  */
    lexemn->p_vec = malloc(lexemn->p_capacity * sizeof(struct lexemn_page));
    if (!lexemn->p_vec)
    {
        perror("malloc");
        abort();
    }

    lexemn->p_head = lexemn->p_vec;

    /* Buffer of output tokens.  */

    lexemn->stream.size =
    lexemn->stream.capacity = 0;
    lexemn->stream.tokens = nullptr;
}

/* Load input content into LEXEMN from WHENCE for lexical scanning. In file-system
   mode, WHENCE is treated as a file path. In interactive shell mode, WHENCE is
   treated directly as the input string to scan.  */
void
lexemn_load(struct lexemn *lexemn, char const *whence)
{
    struct lexemn_page *page;
    char unsigned const *page_content = nullptr;

    if (lexemn->lex_stat.is_shell)
    {
        page = lexemn->p_vec;
        lexemn->p_size = 1;
    }
    else
    {
        if (lexemn->p_size >= lexemn->p_capacity)
        {
            size_t const cap = lexemn->p_capacity < 4 ?
                4 : 2 * lexemn->p_capacity;
            struct lexemn_page *vec = realloc(lexemn->p_vec,
                cap * sizeof(struct lexemn_page));
            if (!vec)
            {
                perror("realloc");
                lexemn_cleanup(lexemn);
                abort();
            }

            lexemn->p_vec = vec;
            lexemn->p_capacity = cap;
        }

        page = lexemn->p_vec + lexemn->p_size;
        ++lexemn->p_size;
    }

    /* Point to to currently active page.  */
    lexemn->p_head = page;

    if (lexemn->lex_stat.is_shell)
    {
        page_content = (char unsigned *)whence;
        page->file.path =
        page->file.name = "@shell";
        page->file.dirname = nullptr;
        page->file.fd = -1;
        ++page->line;
    }
    else
    {
        int const fd = open(whence, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            lexemn_cleanup(lexemn);
            exit(EXIT_FAILURE);
        }

        struct stat st;
        if (fstat(fd, &st) == -1)
        {
            perror("fstat");
            close(fd);
            lexemn_cleanup(lexemn);
            exit(EXIT_FAILURE);
        }

        /* Guard against zero-length files.   */
        if (st.st_size == 0)
            page_content = (char unsigned *)"";
        else
        {
            void const *addr = mmap(nullptr, (size_t)st.st_size,
                                            PROT_READ, MAP_PRIVATE, fd, 0);
            if (addr == MAP_FAILED)
            {
                perror("mmap");
                close(fd);
                lexemn_cleanup(lexemn);
                exit(EXIT_FAILURE);
            }

            page_content = (char unsigned *)addr;
        }

        page->file.name = (char *)whence;
        page->file.path = realpath(whence, nullptr);
        page->file.dirname = dirname((char *)page->file.path);
        page->file.fd = fd;
        page->file.st = st;
        page->line = 1;
    }

    page->buf =
    page->cur =
    page->line_start = page_content;
}

/* Run all execution phases for interpreting LEXEMN.  */
void
lexemn_exec(struct lexemn *const lexemn)
{
    lex_start(lexemn);
    if (lexemn->lex_stat.in_cmd && lexemn->lex_stat.run_cmd)
    {
        lexemn->lex_stat.in_cmd =
        lexemn->lex_stat.run_cmd = false; /* Exit command state.  */
    }

    if (lexemn->lex_stat.is_shell)
    {
        free(lexemn->stream.tokens);
        lexemn->stream.tokens = nullptr;
        lexemn->stream.size =
        lexemn->stream.capacity = 0;
    }
}

/* Release resources and dynamically allocated memory held by LEXEMN.  */
void
lexemn_cleanup(struct lexemn *const lexemn)
{
    for (size_t i = 0; i < lexemn->p_size; ++i)
    {
        if (lexemn->lex_stat.is_shell)
            break;

        struct lexemn_page *target = lexemn->p_vec + i;

        if (target->file.path)
            free((char *)target->file.path);

        if (target->file.fd >= 0)
        {
            if (target->file.st.st_size > 0)
                munmap((void *)target->buf,
                    (size_t)target->file.st.st_size);
            close(target->file.fd);
        }
    }

    if (lexemn->p_vec)
    {
        free(lexemn->p_vec);
        lexemn->p_vec = nullptr;
    }

    lexemn->p_head = nullptr;
    if (lexemn->stream.tokens)
    {
        free(lexemn->stream.tokens);
        lexemn->stream.tokens = nullptr;
    }
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

int
main(int argc, char *argv[])
{
    char *line, *s;
    char prompt[32];
    struct lexemn lexemn = { 0 };

    setlocale(LC_ALL, "");
    lexemn_init(&lexemn);

    if (argc > 1)
    {
        lexemn.lex_stat.is_fs = true;
        for (int i = 1; i < argc; ++i)
            lexemn_load(&lexemn, argv[i]);
        lexemn_exec(&lexemn);
        lexemn_cleanup(&lexemn);
        return 0;
    }

    lexemn.lex_stat.is_shell = true;

    /* Loop reading and executing lines until the user quits.  */
    while (0 == done)
    {
        sprintf(prompt, "(%04d) ", 1 + lexemn.p_head->line);
        line = readline(prompt);

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
            lexemn_load(&lexemn, s);
            lexemn_exec(&lexemn);
            add_history(s);
        }

        free(line);
    }

    return EXIT_SUCCESS;
}

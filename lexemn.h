/*
 * lexemn.h -- Core execution context declarations.
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

#ifndef LEXEMN_LEXEMN_H
#define LEXEMN_LEXEMN_H

#include <sys/stat.h>

#include "lex.h"

/* Structure to track the scanning state and buffer context for a single page,
   which serves as the fundamental unit of input for scanning.  In file-system
   mode, each source file has its own instance allocated within P_VEC.  In
   interactive shell mode, a single persistent instance is reused, and its buffer
   content in BUF is replaced each time the user submits new input.  When lexing
   in shell mode, FILE.NAME and FILE.PATH are both set to `@shell'.  */
struct lexemn_page
{
    /* Pointer to the start of the input buffer.  In file-system mode, this memory
      is mapped using mmap.  Otherwise, when running in interactive shell mode, it
      points to a buffer returned by readline.  */
    char unsigned const *buf;

    /* Pointer to the current scanning position within BUF.  */
    char unsigned const *cur;

    /* Pointer to the start of the current line in BUF.  */
    char unsigned const *line_start;

    /* Current line number being processed within BUF.  In file-system mode,
       this tracks the line position within the source file.  In shell mode,
       this counter persists across inputs and increments sequentially as the
       user submits new expressions at the prompt.  */
    unsigned int line;

    /* Holds meta-data about the underlying source file. */
    struct
    {
        /* Base name of the input source file.  When running in interactive shell
           mode, NAME is set to the string literal `@shell'.  */
        char const *name;

        /* Dynamically allocated path to the source file when lexing in file-system
           mode.  When running in interactive shell mode, PATH is set to the string
           literal `@shell'.  */
        char const *path;

        /* Directory component of the source file path.  Unused in shell mode.   */
        char const *dirname;

        /* System status of the file.  Unused in shell mode.  */
        struct stat st;

        /* File descriptor.  Unused in shell mode.  */
        int fd;
    } file;
};

/* Top-level execution context encapsulating the complete global state of the
   application. This structure serves as the central root context holding all
   runtime environment configurations, active state flags, input streams, symbol
   tables, and execution artifacts throughout the program's lifecycle.

   In file-system mode, P_VEC acts as a dynamic array holding P_SIZE input pages
   allocated up to P_CAPACITY, with P_HEAD pointing to the page currently being
   scanned.  In interactive shell mode, P_VEC holds a single reused page.  */
struct lexemn
{
    /* Dynamically allocated array of input pages scheduled for scanning.
       In file-system mode, P_VEC acts as a growable vector holding P_SIZE pages
       allocated up to P_CAPACITY.  In interactive shell mode, P_VEC points to
       an array containing a single persistent page instance.  */
    struct lexemn_page *p_vec;

   /* Pointer to the active input page currently being scanned within P_VEC.
      In file-system mode, P_HEAD references the specific page undergoing lexical
      analysis as scanning progresses through P_VEC.  In interactive shell mode,
      P_HEAD continuously points to the single, reused page instance stored in
      P_VEC.  */
    struct lexemn_page *p_head;

   /* Current number of active pages stored in P_VEC.  In file-system
      mode, this tracks the number of loaded input pages.  In interactive
      shell mode, P_SIZE is always 1.  */
    size_t p_size;

    /* Maximum capacity of P_VEC before memory reallocation is required.  */
    size_t p_capacity;

    /* Stream accumulator containing tokens scanned across all P_VEC pages.  */
    struct tstream_t stream;

    /* Bitfields representing internal operational states during lexing.  */
    struct
    {
        /* Nonzero if running interactively from a shell prompt.  */
        bool is_shell : 1;

        /* Nonzero if scanning source files from the file system.  */
        bool is_fs : 1;

        /* Nonzero if currently parsing an internal meta-command.  */
        bool in_cmd : 1;

        /* Nonzero if the parsed meta-command is scheduled for execution.  */
        bool run_cmd : 1;
    } lex_stat;
};

#endif //LEXEMN_LEXEMN_H

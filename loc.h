/*
 * loc.h -- Source code location delcarations.
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

#ifndef LEXEMN_LOC_H
#define LEXEMN_LOC_H

#include <stdint.h>

/* Encodes the location of a token or AST node in source code. Basically
   It packs a file ID, line number and column number into a single 32-bits
   unsigned integer.

   +------------------+-------------------------+-----------------+
   | File ID (9 bits) |     Line (14 bits)      | Column (9 bits) |
   +------------------+-------------------------+-----------------+
   31               24 23                     10 9                0  */
typedef uint32_t loc_t;

/* Map FILE, LINE and COLUMN to a loc_t object.  */
loc_t loc_make(uint32_t file, uint32_t line, uint32_t column);

/* Return the file ID encoded within LOC.  */
uint32_t loc_file(loc_t loc);

/* Return the line number encoded within LOC.  */
uint32_t loc_line(loc_t loc);

/* Return the column number encoded within LOC.  */
uint32_t loc_column(loc_t loc);

#endif // LEXEMN_LOC_H

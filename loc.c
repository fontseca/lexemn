/*
 * loc.c -- Location of objects in source code.
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

#include "loc.h"

#define LOC_COL_BITS    9UL    /* Max number of columns.  */
#define LOC_LINE_BITS   14UL   /* Max number of lines.  */
#define LOC_FILE_BITS   9UL    /* Max number of files.  */

#define LOC_COL_SHIFT   0UL
#define LOC_LINE_SHIFT  (LOC_COL_SHIFT  + LOC_COL_BITS)
#define LOC_FILE_SHIFT  (LOC_LINE_SHIFT + LOC_LINE_BITS)

#define LOC_COL_MASK    ( ( 1UL << LOC_COL_BITS  ) - 1 )
#define LOC_LINE_MASK   ( ( 1UL << LOC_LINE_BITS ) - 1 )
#define LOC_FILE_MASK   ( ( 1UL << LOC_FILE_BITS ) - 1 )

loc_t
loc_make(uint32_t file, uint32_t line, uint32_t column)
{
    return ((file & LOC_FILE_MASK) << LOC_FILE_SHIFT)
           | ((line & LOC_LINE_MASK) << LOC_LINE_SHIFT)
           | ((column  & LOC_COL_MASK) << LOC_COL_SHIFT);
}

uint32_t
loc_file(loc_t const loc)
{
    return (loc >> LOC_FILE_SHIFT) & LOC_FILE_MASK;
}

uint32_t
loc_line(loc_t const loc)
{
    return (loc >> LOC_LINE_SHIFT) & LOC_LINE_MASK;
}

uint32_t
loc_column(loc_t const loc)
{
    return (loc >> LOC_COL_SHIFT)  & LOC_COL_MASK;
}

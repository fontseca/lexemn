/*
 * symtab.h -- Symbol table declarations.
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

#ifndef LEXEMN_SYMTAB_H
#define LEXEMN_SYMTAB_H

/* Individual symbol entry stored in a symbol table.  */
struct symbl
{
    /* Pointer to the symbol string buffer.  */
    char unsigned const *str;

    /* Length of the symbol string in bytes.  */
    size_t len;

    /* Precomputed hash code for fast lookup and comparison.  */
    unsigned int hash;
};

/* Hash table structure for identifiers lookup.  */
struct symtab
{
    /* Dynamically allocated array of pointers to symbol entries.  */
    struct symbl **symbols;

    /* Total capacity of SYMBOLS array, guaranteed to be a power of two
       (2^ORDER).  */
    size_t capacity;

    /* Number of active symbol entries currently stored in the table.  */
    size_t size;
};

/* Initialize symbol table TAB with a capacity of 2^ORDER slots.  Return
   zero on successful allocation, or a non-zero value on failure.  */
extern int32_t
symtab_init(struct symtab *tab, uint32_t order);

/* Search symbol table TAB for a symbol matching string STR of length LEN.
   If the symbol exists, return a pointer to the entry; otherwise, create
   a new symbol entry into TAB and return its pointer.  */
extern struct symbl *
symtab_lookup(struct symtab *tab,
                  char unsigned const *str, size_t len);

/* Release memory allocated for TAB->SYMBOLS and all contained entries in TAB.  */
extern void
symtab_cleanup(struct symtab *tab);

#endif //LEXEMN_SYMTAB_H

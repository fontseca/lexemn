/*
 * symtab.c -- Symbol table implementation.
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

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"

/* FNV-1a 32-bit hash to produces high-entropy bit distribution.  */
static uint32_t
hash_fnv1a(char unsigned const *key, size_t len)
{
    uint32_t hash = 2166136261U;
    for (size_t i = 0; i < len; ++i)
    {
        hash ^= (char unsigned)key[i];
        hash *= 16777619U;
    }
    return hash;
}

int
symtab_init(struct symtab *tab, uint32_t order)
{
    if (!tab)
        return -1;
    tab->size = 0;
    tab->capacity = 1U << order;
    tab->symbols = calloc(tab->capacity, sizeof(struct symbl *));
    if (!tab->symbols)
        return -1;
    return 0;
}

static int32_t
symtab_grow(struct symtab *tab)
{
    size_t old_cap = tab->capacity;
    struct symbl **old_symbols = tab->symbols;
    size_t new_cap = old_cap << 1;
    struct symbl **new_symbols = calloc(new_cap, sizeof(struct symbl *));
    if (!new_symbols)
        return -1;
    uint32_t sizemask = (uint32_t)new_cap - 1;
    for (uint32_t i = 0; i < old_cap; ++i)
    {
        struct symbl *symbol = old_symbols[i];
        if (!symbol)
            continue;
        uint32_t index = symbol->hash & sizemask;
        uint32_t hash2 = (symbol->hash >> 16) | 1;
        while (new_symbols[index])
            index = (index + hash2) & sizemask;
        new_symbols[index] = symbol;
    }
    tab->symbols = new_symbols;
    tab->capacity = new_cap;
    free(old_symbols);
    return 1;
}

struct symbl *
symtab_lookup(struct symtab *tab,
                char unsigned const *str, size_t len)
{
    uint32_t sizemask = (uint32_t)tab->capacity - 1;
    uint32_t hash = hash_fnv1a(str, len);
    uint32_t index = hash & sizemask;
    struct symbl *symbol = tab->symbols[index];
    if (symbol != nullptr)
    {
        if (symbol->hash == hash
            && symbol->len == len
            && !memcmp(symbol->str, str, len))
            return symbol;
        uint32_t hash2 = (hash >> 16) | 1;
        for (;;)
        {
            index = (index + hash2) & sizemask;
            symbol = tab->symbols[index];
            if (symbol == nullptr)
                break;
            if (symbol->hash == hash
                && symbol->len == len
                && !memcmp(symbol->str, str, len))
                return symbol;
        }
    }
    symbol = malloc(sizeof(struct symbl));
    if (!symbol)
        return nullptr;
    symbol->str = (char unsigned *)str;
    symbol->len = len;
    symbol->hash = hash;
    tab->symbols[index] = symbol;
    if (++tab->size * 4 >= tab->capacity * 3) /* Trigger growth at 75% load factor.  */
        symtab_grow(tab);
    return symbol;
}

void
symtab_cleanup(struct symtab *tab)
{
    for (size_t i = 0; i < tab->capacity; ++i)
        if (tab->symbols[i])
            free(tab->symbols[i]);
    if (tab->symbols)
    {
        free(tab->symbols);
        tab->symbols = nullptr;
    }
    tab->size =
    tab->capacity = 0;
}

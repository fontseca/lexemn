/*
 * symtab_test.c -- Tests for symbol table.
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
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "symtab.c"

static void
test_hash_fnv1a()
{
    uint32_t hash1 = hash_fnv1a((char unsigned *)"123", 3);
    uint32_t hash2 = hash_fnv1a((char unsigned *)"123", 3);
    assert(hash1 == hash2); /* Determinism.  */

    uint32_t hash3 = hash_fnv1a((char unsigned *)"", 0);
    assert(hash3 == 2166136261U); /* FNV-1a offset basis. */
}

static void
test_symtab_init()
{
    struct symtab tab;

    /* Edge case: null pointer handling.  */
    assert(symtab_init(nullptr, 2) == -1);

    /* Normal initialization.  */
    int ret = symtab_init(&tab, 2);
    assert(tab.size == 0);
    assert(tab.capacity == 4);
    assert(tab.symbols != nullptr);
    assert(ret == 0);
    symtab_cleanup(&tab);

    /* Edge case: order=0.  */
    ret = symtab_init(&tab, 0);
    assert(tab.capacity == 1);
    assert(ret == 0);
    symtab_cleanup(&tab);
}

static void
test_symtab_lookup()
{
    struct symtab tab;

    /* Normal behavior.  */
    {
        symtab_init(&tab, 1);

        struct symbl *e1 = symtab_lookup(&tab, (char unsigned *)"aBaa", 1);
        assert(e1->len == 1);
        assert(!strncmp("a", (char *)e1->str, e1->len));

        struct symbl *e2 = symtab_lookup(&tab, (char unsigned *)"aAAA__", 1);
        assert(e2->len == 1);
        assert(!strncmp("a", (char *)e2->str, e2->len));

        struct symbl *e3 = symtab_lookup(&tab, (char unsigned *)"bbBBBBAAA", 2);
        assert(e3->len == 2);
        assert(!strncmp("bb", (char *)e3->str, e3->len));

        assert(e1 == e2);
        assert(e1 != e3);
        assert(tab.size == 2);
        assert(tab.capacity == 4);

        symtab_cleanup(&tab);
    }

    /* Multibyte and UTF-8 edge cases.  */

    {
        symtab_init(&tab, 2);

        /* Full multibyte sequence lookup.  */

        struct symbl *e1 = symtab_lookup(&tab, (char unsigned *)"zß水🍌你好", 15);
        assert(e1->len == 15);
        assert(!strncmp("zß水🍌你好", (char *)e1->str, e1->len));

        /* Mid-codepoint slicing. "ß" is 2 bytes (0xC3 0x9F). Slicing at 2 bytes
           yields "z" + 0xC3.  */

        struct symbl *e_partial = symtab_lookup(&tab, (char unsigned *)"zß水🍌你好", 2);
        assert(e_partial->len == 2);
        assert(e_partial != e1);
        assert(!memcmp(e_partial->str, "z\xC3", 2));

        /* Exact multibyte deduplication from a separate stack buffer.  */

        char unsigned unicode_buf[] = "zß水🍌你好";
        struct symbl *e_dedup = symtab_lookup(&tab, unicode_buf, 15);
        assert(e1 == e_dedup);

        /* Unicode normalization differences (NFD vs. NFC).  "é" (e + U+0301 combining
           accent, 3 bytes) vs "é" (precomposed U+00E9, 2 bytes).  */

        const char unsigned nfd_e[] = "e\xCC\x81"; /* 3 bytes */
        const char unsigned nfc_e[] = "\xC3\xA9";  /* 2 bytes */

        struct symbl *e_nfd = symtab_lookup(&tab, nfd_e, 3);
        struct symbl *e_nfc = symtab_lookup(&tab, nfc_e, 2);

        assert(e_nfd->len == 3);
        assert(e_nfc->len == 2);
        assert(e_nfd != e_nfc);

        /* High-byte 4-byte UTF-8 sequence.  */

        struct symbl *e_emoji = symtab_lookup(&tab, (char unsigned *)"🍌", 4);
        assert(e_emoji->len == 4);
        assert(!memcmp(e_emoji->str, "\xF0\x9F\x8D\x8C", 4));

        assert(tab.size == 5);

        symtab_cleanup(&tab);
    }

    /* Large size.  */

    {
        symtab_init(&tab, 1);
        constexpr size_t NUM_ENTRIES = 1000;
        char key_buf[64];

        for (size_t i = 0; i < NUM_ENTRIES; ++i)
        {
            int len = snprintf(key_buf, sizeof(key_buf), "symbol_key_%zu", i);
            struct symbl *s = symtab_lookup(&tab, (char unsigned *)key_buf, (size_t)len);

            assert(s != nullptr);
            assert(s->len == (size_t)len);
            assert(memcmp(s->str, key_buf, (size_t)len) == 0);
        }

        assert(tab.size == NUM_ENTRIES);
        assert(tab.capacity >= NUM_ENTRIES);
        assert((tab.capacity & (tab.capacity - 1)) == 0); /* Capacity must remain a power of 2. */

        /* Query inserted entries.  */

        for (size_t i = 0; i < NUM_ENTRIES; ++i)
        {
            int len = snprintf(key_buf, sizeof(key_buf), "symbol_key_%zu", i);
            struct symbl *s = symtab_lookup(&tab, (char unsigned *)key_buf, (size_t)len);

            assert(s != nullptr);
            assert(s->len == (size_t)len);
            assert(memcmp(s->str, key_buf, (size_t)len) == 0);
        }

        assert(tab.size == NUM_ENTRIES);

        /* Slice prefix subsets of existing keys to ensure length-sensitivity.  */

        for (size_t i = 0; i < NUM_ENTRIES; ++i)
        {
            struct symbl *prefix_sym = symtab_lookup(&tab, (char unsigned *)key_buf, 11);
            assert(prefix_sym != nullptr);
            assert(prefix_sym->len == 11);
            assert(memcmp(prefix_sym->str, "symbol_key_", 11) == 0);
        }

        assert(tab.size == NUM_ENTRIES + 1);
        symtab_cleanup(&tab);
    }

    /* Empty strings and internal null bytes.  */
    {
        symtab_init(&tab, 2);

        struct symbl *e_empty1 = symtab_lookup(&tab, (char unsigned *)"", 0);
        struct symbl *e_empty2 = symtab_lookup(&tab, (char unsigned *)"", 0);
        assert(e_empty1->len == 0);
        assert(e_empty1 == e_empty2);

        struct symbl *e_null1 = symtab_lookup(&tab, (char unsigned *)"a\0b", 3);
        struct symbl *e_null2 = symtab_lookup(&tab, (char unsigned *)"a\0b", 3);
        struct symbl *e_null_diff = symtab_lookup(&tab, (char unsigned *)"a\0c", 3);

        assert(e_null1->len == 3);
        assert(e_null1 == e_null2);
        assert(e_null1 != e_null_diff);

        symtab_cleanup(&tab);
    }

    /* Same buffer, different length slices.  */

    {
        symtab_init(&tab, 2);
        unsigned char const *buf = (char unsigned *)"abcdef";

        struct symbl *s1 = symtab_lookup(&tab, buf, 1); /* "a"      */
        struct symbl *s2 = symtab_lookup(&tab, buf, 2); /* "ab"     */
        struct symbl *s3 = symtab_lookup(&tab, buf, 3); /* "abc"    */
        struct symbl *s4 = symtab_lookup(&tab, buf, 6); /* "abcdef" */

        assert(s1 != s2 && s2 != s3 && s3 != s4);
        assert(s1->len == 1 && s2->len == 2 && s3->len == 3 && s4->len == 6);

        /* Looking up "abc" from a separate literal buffer must match s3.  */

        struct symbl *s3_alt = symtab_lookup(&tab, (char unsigned *)"abcXYZ", 3);
        assert(s3 == s3_alt);

        symtab_cleanup(&tab);
    }

    /* Prefix and suffix matching.  */

    {
        symtab_init(&tab, 2);

        struct symbl *short_sym = symtab_lookup(&tab, (char unsigned *)"test", 4);
        struct symbl *long_sym  = symtab_lookup(&tab, (char unsigned *)"testing", 7);

        assert(short_sym != long_sym);
        assert(short_sym->len == 4);
        assert(long_sym->len == 7);

        struct symbl *sliced_sym = symtab_lookup(&tab, (char unsigned *)"testing", 4);
        assert(short_sym == sliced_sym);

        symtab_cleanup(&tab);
    }

    /* High-bit / non-ASCII unsigned byte values.  */

    {
        symtab_init(&tab, 2);

        const char unsigned high_bytes1[] = {0xFF, 0xFE, 0x80, 0x7F};
        const char unsigned high_bytes2[] = {0xFF, 0xFE, 0x80, 0x7E}; /* 1 bit diff */

        struct symbl *hb1 = symtab_lookup(&tab, high_bytes1, 4);
        struct symbl *hb2 = symtab_lookup(&tab, high_bytes1, 4);
        struct symbl *hb3 = symtab_lookup(&tab, high_bytes2, 4);

        assert(hb1 == hb2);
        assert(hb1 != hb3);
        assert(hb1->hash != hb3->hash || memcmp(hb1->str, hb3->str, 4) != 0);

        symtab_cleanup(&tab);
    }

    /* Large key lengths.  */

    {
        symtab_init(&tab, 2);

        size_t large_len = 4096;
        char unsigned *large_key1 = malloc(large_len);
        char unsigned *large_key2 = malloc(large_len);

        memset(large_key1, 'A', large_len);
        memset(large_key2, 'A', large_len);
        large_key2[large_len - 1] = 'B';

        struct symbl *lk1 = symtab_lookup(&tab, large_key1, large_len);
        struct symbl *lk2 = symtab_lookup(&tab, large_key1, large_len);
        struct symbl *lk3 = symtab_lookup(&tab, large_key2, large_len);

        assert(lk1 == lk2);
        assert(lk1 != lk3);
        assert(lk1->len == large_len);

        free(large_key1);
        free(large_key2);
        symtab_cleanup(&tab);
    }

    /* Multi-step probe traversal stability.  */

    {
        symtab_init(&tab, 2);

        struct symbl *k1 = symtab_lookup(&tab, (char unsigned *)"k1", 2);
        struct symbl *k2 = symtab_lookup(&tab, (char unsigned *)"k2", 2);

        assert(symtab_lookup(&tab, (char unsigned *)"k1", 2) == k1);
        assert(symtab_lookup(&tab, (char unsigned *)"k2", 2) == k2);

        symtab_cleanup(&tab);
    }
}

static void
test_symtab_grow()
{
    struct symtab tab;

    symtab_init(&tab, 0);

    constexpr size_t INSERT_COUNT = 1000;
    char buf[32];

    /* Insert heavy load to force multiple symtab_grow() triggers.  */

    for (size_t i = 0; i < INSERT_COUNT; ++i)
    {
        snprintf(buf, sizeof(buf), "sym_%d", (int)i);
        symtab_lookup(&tab, (char unsigned *)buf, strlen(buf));
    }

    assert(tab.size == INSERT_COUNT);

    /* Capacity should have grown 1 -> 4 -> 16 -> 64 -> 256 -> 1024 -> 4096.
       since load factor is 75%, 1000 elements requires > 1333 capacity.  */

    assert(tab.capacity >= 1024);

    /* Validate that re-hashing maintained data integrity.  */
    for (size_t i = 0; i < INSERT_COUNT; ++i)
    {
        snprintf(buf, sizeof(buf), "sym_%d", (int)i);
        struct symbl *s = symtab_lookup(&tab, (char unsigned *)buf, strlen(buf));
        assert(s != nullptr);
        assert(s->len == strlen(buf));
        assert(memcmp(s->str, buf, strlen(buf)) == 0);
    }

    assert(tab.size == INSERT_COUNT);

    symtab_cleanup(&tab);
}

int main()
{
    test_hash_fnv1a();
    test_symtab_init();
    test_symtab_lookup();
    test_symtab_grow();
    return 0;
}
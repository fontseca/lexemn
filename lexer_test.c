/*
 * lexer_test.c -- Lexer tests.
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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>

#include "lexer.h"

/* Forward function declarations.  */

/* Prints a escaped string with unescaped characters.  */
static void
rawprint(FILE *fp, const char *str);

struct expect
{
    enum token_type type;
    char const      *str;
};

struct lex_case
{
    char const    *input;           /* Test input string.  */
    struct expect *expected_tokens;        /* Expected list of tokens.  */
    size_t        expected_size;  /* Expected amount of tokens scanned.  */
};

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wmissing-field-initializers"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#ifndef CASE
#  define CASE(in, ...) \
    { \
        .input           = in, \
        .expected_tokens = (struct expect[]) { __VA_ARGS__ }, \
        .expected_size   = sizeof( (const struct expect[]) { __VA_ARGS__ } ) / sizeof(struct expect) \
    }
#endif

/* Master table with all possible test cases when lexing.  */
static struct lex_case const cases_table[] = {
#include "lexer_test.def"
};

static char *const token_spellings[MAX_TOKENS] = {
#define OP( name, _ ) "TOK_" #name ,
#define TOK( name, _ ) "TOK_" #name,
    TOK_TYPES_TABLE
#undef TOK
#undef OP
};

#ifndef TOK_NAME
#  define TOK_NAME( token ) token_spellings[((token).type)]
#endif

#ifndef TOK_IS_LITERAL
#  define TOK_IS_LITERAL( token ) \
     ( (token).type == TOK_STRING \
    || (token).type == TOK_NUMBER \
    || (token).type == TOK_CMD \
    || (token).type == TOK_CMD_ARG )
#endif

#ifndef TOK_IS_IDENT
#  define TOK_IS_IDENT( token ) \
     ( ( token ).type == TOK_NAME \
    || ( token ).type == TOK_CONST )
#endif

static void
test_lex(void)
{
    constexpr size_t n_cases = sizeof(cases_table) / sizeof(cases_table[0]);

    for (size_t case_idx = 0; case_idx < n_cases; ++case_idx)
    {
        struct tstream_t      stream = { 0 };
        struct lexer_t        lexer;
        struct lex_case const row = cases_table[case_idx];

        lex_setup(&lexer, (char unsigned const *) row.input);
        lex_start(&lexer, &stream);

        /* Assert last token is TOK_END before subtracting below.  */
        assert(stream.tokens[stream.size - 1].type == TOK_END);

        /* Assert amount of lexed tokens matches expected count.  */
        if (stream.size - 1 != row.expected_size)
        {
            fprintf(stderr, "Failed test case #%zu: Token count mismatch.\n\n", 1 + case_idx);
            fprintf(stderr, "INPUT:\n");
            rawprint(stderr, row.input);

            fprintf(stderr, "\nEXPECTED:\n");
            fprintf(stderr, "%zu tokens\n", row.expected_size);

            fprintf(stderr, "\nACTUAL:\n");
            fprintf(stderr, "%zu tokens\n\n", stream.size);

            for (size_t tok_idx = 0; tok_idx < stream.size; ++tok_idx)
            {
                struct expect const  want = row.expected_tokens[tok_idx];
                struct token_t const have = stream.tokens[tok_idx];

                if (TOK_IS_LITERAL(want))
                {
                    fprintf(stderr, "%s (`%s') → %s (`%.*s')\n",
                        TOK_NAME(want), want.str, TOK_NAME(have), (int)have.val.text.len, (char const *)have.val.text.str);
                } else
                {
                    fprintf(stderr, "%s → %s\n",
                        TOK_NAME(row.expected_tokens[tok_idx]), TOK_NAME(stream.tokens[tok_idx]));
                }
            }

            free(stream.tokens);
            assert(0 && "token count mismatch");
        }

        /* Assert each token in the stream has the right type and value, if applicable.  */
        for (size_t tok_idx = 0; tok_idx < row.expected_size; ++tok_idx)
        {
            struct expect const  want = row.expected_tokens[tok_idx];
            struct token_t const have = stream.tokens[tok_idx];

            bool const type_mismatch = want.type != have.type;
            bool const str_mismatch = TOK_IS_LITERAL(want)
                    && 0 != strncmp(want.str, (char const *) have.val.text.str, strlen(want.str));

            if (type_mismatch || str_mismatch)
            {
                fprintf(stderr, "Failed test case #%zu at token index [%zu]:\n\n", 1 + case_idx, tok_idx);
                rawprint(stderr, row.input);

                fprintf(stderr, "\nEXPECTED:\n");
                fprintf(stderr, "  type: %s\n", TOK_NAME(want));

                if (TOK_IS_LITERAL(want))
                    fprintf(stderr, "  text: `%s'\n\n", want.str);

                fprintf(stderr, "ACTUAL:\n");
                fprintf(stderr, "  type: %s\n", TOK_NAME(have));

                if (TOK_IS_LITERAL(have))
                    fprintf(stderr, "  text: `%.*s'\n\n", (int)have.val.text.len,
                            (char const *)have.val.text.str);

                free(stream.tokens);
                assert(0 && "token mismatch");
            }
        }

        free(stream.tokens);
    }
}

int
main(void)
{
    setlocale(LC_ALL, "");
    test_lex();
    return 0;
}

static void
rawprint(FILE *fp, const char *str)
{
    while (*str)
    {
        switch (*str)
        {
            case '\f': fprintf(fp, "\\f");
                break;
            case '\r': fprintf(fp, "\\r");
                break;
            case '\v': fprintf(fp, "\\v");
                break;
            case '\n': fprintf(fp, "\\n");
                break;
            case '\t': fprintf(fp, "\\t");
                break;
            case '\\': fprintf(fp, "\\\\");
                break;
            case '\"': fprintf(fp, "\\\"");
                break;
            default: fputc(*str, fp);
                break;
        }

        str++;
    }

    fprintf(fp, "\n");
}

// (a + b)^2 = (a + b) ⋅ (a + b)
// int x := sqrt ( ((a * b) / 2) +  ), y := sqrt((9b / a^2)) - 3(c - b) * sqrt(c ÷ b);
//
//
// int a := |-10|;
// set S := { 1, 2, a, -9, ( 2 + 5 ) };
//
// matrix M := [
//     [1, 2],
//     [3, 4],
//     [5, 6],
//     [7, 8],
//     [9, x]
// ];
//

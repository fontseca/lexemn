/*
 * lexer.c -- Lexical analyzer implementation.
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

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <uchar.h>
#include <stdio.h>

#include "lexer.h"

/* Reflect how the token's text is managed in memory.  */
enum spell_type : short unsigned
{
    SPELL_OPERATOR,
    SPELL_IDENT,
    SPELL_LITERAL,
    SPELL_NONE
};

struct token_spelling
{
    enum spell_type      category;
    char unsigned const *name;
};

/* Array indexed by 'lex_token_type_t' providing descriptive names and literal
   spellings for debugging and diagnostic emission.  */
[[maybe_unused]]
static struct token_spelling const token_spellings[MAX_TOKENS] = {
#define OP( name, raw ) { SPELL_OPERATOR, (char unsigned const *) raw },
#define TOK( name, raw ) { SPELL_ ## raw, (char unsigned const *) #name },
    TOK_TYPES_TABLE
#undef TOK
#undef OP
};

#define TOK_SPELL( token ) \
    ( ( token ) != nullptr \
        ? token_spellings[(token.type)].category \
            : "" )

#define TOK_NAME( token ) \
    ( ( token ) != nullptr \
        ? token_spellings[(token.type)].name \
            : "" )

/* Return non-zero value if C is a non-printable character.  */
#define IS_WHITESPACE( c ) \
    ( ( c ) == ' '  || ( c ) == '\t' || \
      ( c ) == '\n' || ( c ) == '\r' || \
      ( c ) == '\v' || ( c ) == '\f' )

/* Return non-zero if the numerical value of the character C is falls within
   the specified closed range [LOWER, UPPER].  */
#define IS_BETWEEN( c, lower, upper ) \
    ( ( c ) >= ( lower ) && ( c ) <= ( upper ) )

/* Return non-zero value if the wide character C typed as char32_t is valid
   as part of an identifier.  In addition to the standard ASCII characters
   normally used (`_', `a-z', `A-Z'), this macro accepts Universal Character
   Names (UCNs) allowed in the C programming language per ISO/IEC 9899:201x
   Annex D: Universal character names for identifiers.  */
#define IS_IDENTIFIER( c )                      \
       (00 /* Standard ASCII characters.  */    \
        || ( c ) == '_'                         \
        || IS_BETWEEN(( c ), 'A', 'Z')          \
        || IS_BETWEEN(( c ), 'a', 'z')          \
        ||                                      \
        00 /* Latin-1 supplement (part 1).  */  \
        || IS_BETWEEN(( c ), 0x00B2, 0x00B5)    \
        || IS_BETWEEN(( c ), 0x00B7, 0x00BA)    \
        || IS_BETWEEN(( c ), 0x00BC, 0x00BE)    \
        || IS_BETWEEN(( c ), 0x00C0, 0x00D6)    \
        ||                                      \
        00 /* Latin-1 supplement (part 2).  */  \
        || IS_BETWEEN(( c ), 0x00D8, 0x00F6)    \
        || IS_BETWEEN(( c ), 0x00F8, 0x00FF)    \
        ||                                      \
        00 /* Extended Latin, Greek, Cyrillic,  \
        00  Hebrew, Arabic, etc.  */            \
        || IS_BETWEEN(( c ), 0x0100, 0x167F)    \
        || IS_BETWEEN(( c ), 0x1681, 0x180D)    \
        || IS_BETWEEN(( c ), 0x180F, 0x1FFF)    \
        ||                                      \
        00 /* Formatting, directional controls, \
        00  & combining marks.  */              \
        || IS_BETWEEN(( c ), 0x200B, 0x200D)    \
        || IS_BETWEEN(( c ), 0x202A, 0x202E)    \
        || IS_BETWEEN(( c ), 0x203F, 0x2040)    \
        || IS_BETWEEN(( c ), 0x2060, 0x206F)    \
        ||                                      \
        00 /* Technical symbols, number forms,  \
        00  enclosed alphanumerics.  */         \
        || IS_BETWEEN(( c ), 0x2070, 0x218F)    \
        || IS_BETWEEN(( c ), 0x2460, 0x24FF)    \
        || IS_BETWEEN(( c ), 0x2776, 0x2793)    \
        || IS_BETWEEN(( c ), 0x2C00, 0x2DFF)    \
        || IS_BETWEEN(( c ), 0x2E80, 0x2FFF)    \
        ||                                      \
        00 /* CJK symbols & punctuation.  */    \
        || IS_BETWEEN(( c ), 0x3004, 0x3007)    \
        || IS_BETWEEN(( c ), 0x3021, 0x302F)    \
        || IS_BETWEEN(( c ), 0x3031, 0x303F)    \
        ||                                      \
        00 /* Unified East Asian scripts        \
        00  (primary CJK block).  */            \
        || IS_BETWEEN(( c ), 0x3040, 0xD7FF)    \
        ||                                      \
        00 /* CJK Compatibility ideographs &    \
        00  Arabic presentation forms.  */      \
        || IS_BETWEEN(( c ), 0xF900, 0xFD3D)    \
        || IS_BETWEEN(( c ), 0xFD40, 0xFDCF)    \
        || IS_BETWEEN(( c ), 0xFDF0, 0xFE44)    \
        || IS_BETWEEN(( c ), 0xFE47, 0xFFFD)    \
        ||                                      \
        00 /* Supplementary  ideographic planes \
        00  (plane 1 - plane 14).  Contains     \
        00  supplementary multilingual (SMP),   \
        00  supplementary ideographic (SIP),    \
        00  and tertiary ideographic (TIP)      \
        00  planes. */                          \
        || IS_BETWEEN(( c ), 0x10000, 0x1FFFD)  \
        || IS_BETWEEN(( c ), 0x20000, 0x2FFFD)  \
        || IS_BETWEEN(( c ), 0x30000, 0x3FFFD)  \
        || IS_BETWEEN(( c ), 0x40000, 0x4FFFD)  \
        || IS_BETWEEN(( c ), 0x50000, 0x5FFFD)  \
        || IS_BETWEEN(( c ), 0x60000, 0x6FFFD)  \
        || IS_BETWEEN(( c ), 0x70000, 0x7FFFD)  \
        || IS_BETWEEN(( c ), 0x80000, 0x8FFFD)  \
        || IS_BETWEEN(( c ), 0x90000, 0x9FFFD)  \
        || IS_BETWEEN(( c ), 0xA0000, 0xAFFFD)  \
        || IS_BETWEEN(( c ), 0xB0000, 0xBFFFD)  \
        || IS_BETWEEN(( c ), 0xC0000, 0xCFFFD)  \
        || IS_BETWEEN(( c ), 0xD0000, 0xDFFFD)  \
        || IS_BETWEEN(( c ), 0xE0000, 0xEFFFD))

void
lex_setup(struct lexer_t *const lexer,
                char unsigned const *const whence)
{
    lexer->buf = (char unsigned const *)whence;
    lexer->cur = (char unsigned const *)whence;
}

/* Return non-zero value if LEXER has reached the end of the input source.  */
static bool
eof(struct lexer_t const *const lexer)
{
    return '\0' == *lexer->cur;
}

/* Return non-zero value if the character I bytes ahead of the current position
   in LEXER is the end of the input source.  */
static bool
eof_at(struct lexer_t const *const lexer, uint32_t const i)
{
    return '\0' == lexer->cur[i];
}

/* Return a pointer to the current read position in the input source of LEXER.  */
static char unsigned const *
current(struct lexer_t const *lexer)
{
    return lexer->cur;
}

/* Return the character at the current read position in the input source
   of LEXER without advancing the pointer.  */
static char unsigned
peek(struct lexer_t const *const lexer)
{
    return *lexer->cur;
}

/* Return the character N bytes ahead of the current position in LEXER
   without advancing in the input source.  */
static char unsigned
peek_at(struct lexer_t const *const lexer, uint32_t const n)
{
    return *(n + lexer->cur);
}

/* Advance the current read position in LEXER by one byte.  */
static void
mov(struct lexer_t *const lexer)
{
    ++lexer->cur;
}

/* Advance the current read position in LEXER by N bytes.  */
static void
movn(struct lexer_t *const lexer, uint32_t const n)
{
    lexer->cur += n;
}

/* Return non-zero value if the character at the current position in the
   input source matches C.  */
static bool
match(struct lexer_t const *const lexer, char const c)
{
    return *lexer->cur == c;
}

/* Return non-zero value if the character N bytes ahead of the current
   position in LEXER matches C.  */
static bool
match_at(struct lexer_t const *const lexer,
                uint32_t const n, char const c)
{
    return lexer->cur[n] != '\0'
                && lexer->cur[n] == c;
}

/* Skip any white space at the current position in the input source
   of LEXER.  */
static void
skip_blank(struct lexer_t *const lexer)
{
    while (IS_WHITESPACE(*lexer->cur))
        ++lexer->cur;
}

/* Skip the comment sequence at the current position in the input source
   of LEXER.  */
static void
skip_comment(struct lexer_t *const lexer)
{
    /* This function expects the current character to be poiting at
       the first `{' in the comment opening sequence `{{*'; this is
       the reason why I move 3 bytes forward.   */
    movn(lexer, 3);

    while (!eof(lexer))
    {
        /* Look for the closing `*' character.  */
        if (!match(lexer, '*'))
        {
            mov(lexer);
            continue;
        }

        /* If found, check if it is immediately followed by the
          sequence `}}'.  */
        if (match_at(lexer, 1, '}') && match_at(lexer, 2, '}'))
        {
            /* Consume the closing comment bytes `*}}' and finish.  */
            movn(lexer, 3);
            return;
        }

        /* Move one byte forward otherwise.  */
        mov(lexer);
    }
}

/* Append TOKEN to the end of STREAM.  */
static void
tstream_push(struct tstream_t *const stream,
                    struct token_t const token)
{
    if (1 + stream->size > stream->capacity)
    {
        size_t const cap = stream->capacity < 8
                                ? 8 : 2 * stream->capacity;
        struct token_t *buffer = realloc(stream->tokens,
                                                cap * sizeof(struct token_t ));
        if (!buffer)
        {
            perror("Fatal failure");
            free(stream->tokens);
            exit(EXIT_FAILURE);
        }
        stream->tokens = buffer;
        stream->capacity = cap;
    }

    stream->tokens[stream->size++] = token;
}

/* Scan a string at the current position in the input source of LEXER
   and push it onto STREAM.  */
[[nodiscard]]
static int
lex_string(struct lexer_t *const lexer,
                struct tstream_t *const stream)
{
    struct token_t tok = { .type = TOK_STRING };

    /* The token we are going to lex should start here.  */
    mov(lexer); /* Skip opening `"'.  */
    tok.val.text.str = current(lexer);

    while (!eof(lexer) && !match(lexer, '"'))
    {
        ++tok.val.text.len;
        mov(lexer);
    }

    mov(lexer); /* Skip closing `"'.  */
    tstream_push(stream, tok);
    return 0;
}

/* Scan an identifier at the current position in the input source of LEXER
   and push it onto STREAM.  */
[[nodiscard]]
static int
lex_identifier(struct lexer_t *const lexer,
                    struct tstream_t *const stream)
{
    /* True when we have read a multibyte string.  */
    // if (offset > 1)
    // {
    //     char mbs[4]   = {0};
    //     mbstate_t st2 = {0};
    //     size_t ret;
    //     token_type_t type = TOK_UNKNOWN;
    //
    //     ret = c32rtomb(mbs, c32, &st2);
    //
    //     token = token_new(mbs, ret, type);
    //     lex_tstream_add(stream, token);
    //
    //     current += offset;
    //     continue;
    // }

    size_t offset;
    char32_t c32;
    mbstate_t st1 = {0};
    struct token_t tok = { .type = TOK_NAME };
    tok.val.text.str = current(lexer);

    /* Keep lexing the identifier.  */
    for (;;)
    {
        offset = mbrtoc32(&c32, (char const *) current(lexer), SIZE_MAX, &st1);
        if (!IS_IDENTIFIER(c32) && !isdigit(peek(lexer)))
        {
            /* Character found is not valid to be part of an identifier.  */
            break;
        }

        movn(lexer, (uint32_t)offset);
        tok.val.text.len += offset;
    }

    tstream_push(stream, tok);
    return 0;
}

/* Scan a number at the current position in the input source of LEXER and
  push it onto STREAM.  The scanned token is distinguished is between a
  decimal, hexadecimal, octal and binary number.  */
[[nodiscard]]
static int
lex_number(struct lexer_t *const lexer,
                struct tstream_t *const stream)
{
    /* True means the number being parsed has encounter a decimal point.
       This flag helps know when to stop searching for a decimal point.  */
    uint8_t point_count = 0;
    struct token_t tok = { .type = TOK_NUMBER };
    tok.val.text.str = current(lexer);

    while (isdigit(peek(lexer)) || match(lexer, '.'))
    {
        if (match(lexer, '.'))
            ++point_count;

        if (point_count > 1)
        {
            break;
        }

        ++tok.val.text.len;
        mov(lexer);

        /* For cases such as `.a', when the character after the . is not a digit.  */
        // if (1 == point_count && !isdigit(peek(lexer)) && 0 == lexer->lxm_size)
        // {
        //     lxm_reset(lexer);
        //     return (size_t)-1;
        // }
    }

    tstream_push(stream, tok);
    return 0;
}

/* Scan a  meta-command and  its arguments  (if any) and push them onto
   STREAM.

   A meta-command is a built-in shortcut processed directly in an interactive
   mode instance to manage the current session, inspect objects and execute
   local file operations.  They always begin with a backslash (\) and can take
   any number of arguments.  Here are some hypothetical examples of meta-commands:

       \df+                          {{* List built-in functions with details. *}}
       \$                            {{* List internal constants.              *}}
       \exec -f "/path/to/file.lxm"  {{* Execute source file.                  *}}

    For each of these commands, the lexer will generate the following tokens:

            ,---> T_CMD
        \df+
        \$

             ,---> T_CMD            ,---> T_CMDARG         ,---> T_CMDARG
        \exec  -f "/path/to/file.lxm" -f /path/to/file2.lxm
                `---> T_CMDARG         `---> T_CMDARG       */
static size_t
lex_cmd(struct lexer_t *const lexer,
            struct tstream_t *const stream)
{
    if (eof_at(lexer, 1) || IS_WHITESPACE(peek_at(lexer, 1)))
    {
        // error = true; /* unknown command  */
        return (size_t)-1;
    }

    /* Parse command name.  */

    struct token_t cmd = { .type = TOK_CMD };
    cmd.val.text.str = current(lexer);
    mov(lexer);

    while (!IS_WHITESPACE(peek(lexer)) && !eof(lexer))
    {
        mov(lexer);
        ++cmd.val.text.len;
    }

    tstream_push(stream, cmd);

    /* Parse command arguments (if any).  */

    /* Skip any white space in is between the command arguments.  */
    while (IS_WHITESPACE(peek(lexer)))
        mov(lexer);

    /* If end of string, then we exit.  */
    if (eof(lexer))
        return 0;

    /* As long as we are not at the end of the string,
      then we are free to scan arguments.  */
    while (!eof(lexer))
    {
        /* These arguments are expected to be separated by white spaces,
           so if any is found,  we must skip it until we find a parsable
           token.  */
        while (IS_WHITESPACE(peek(lexer)))
            mov(lexer);

        struct token_t arg = { .type = TOK_CMD_ARG };
        arg.val.text.str = current(lexer);

        /* Check if current pointer is the beginning of a string; in such case,
           then we need to parse the remaining values as the are until finding
           the closing string character. */
        if (match(lexer, '"') || match(lexer, '\''))
        {
            mov(lexer);
            ++arg.val.text.len;
            while (!eof(lexer) && !match(lexer, '"') && !match(lexer, '\''))
            {
                mov(lexer);
                ++arg.val.text.len;
            }

            mov(lexer);
            ++arg.val.text.len;
            tstream_push(stream, arg);
            continue;
        }

        /* If end of string, then we exit.  */
        if (eof(lexer))
            continue;

        /* At this point we have found a valid argument.  */
        while (!IS_WHITESPACE(peek(lexer)) && !eof(lexer))
        {
            mov(lexer);
            ++arg.val.text.len;
        }

        tstream_push(stream, arg);
    }

    return 0;
}

/* Scan a constant object and push it onto STREAM.  Constant values start
   with a `$' to differentiate them from a regular identifiers and reserved
   words.  Examples of constants:

        $E           2.7182818284590452354     {{* e          *}}
        $LOG2E       1.4426950408889634074     {{* log_2 e    *}}
        $LOG10E      0.43429448190325182765    {{* log_10 e   *}}
        $LN2         0.69314718055994530942    {{* log_e 2    *}}
        $LN10        2.30258509299404568402    {{* log_e 10   *}}
        $PI          3.14159265358979323846    {{* pi         *}}
        $PI_2        1.57079632679489661923    {{* pi/2       *}}
        $PI_4        0.78539816339744830962    {{* pi/4       *}}
        $1_PI        0.31830988618379067154    {{* 1/pi       *}}
        $2_PI        0.63661977236758134308    {{* 2/pi       *}}
        $2_SQRTPI    1.12837916709551257390    {{* 2/sqrt(pi) *}}
        $SQRT2       1.41421356237309504880    {{* sqrt(2)    *}}
        $SQRT1_2     0.70710678118654752440    {{* 1/sqrt(2)  *}}  */
static int
lex_const(struct lexer_t *const lexer,
                struct tstream_t *const stream)
{
    /* Check if the next byte after `$' is a valid character.
       It should be any of [0-9a-zA-Z_], otherwise the constant
       is ill-formed.  */
    if (eof_at(lexer, 1) || (!isalnum(peek_at(lexer, 1)) && !match_at(lexer, 1, '_')))
    {
        // error = true; /* malformed constant  */
        return -1;
    }

    struct token_t tok = { .type = TOK_CONST };
    tok.val.text.str = current(lexer);
    mov(lexer);

    ++tok.val.text.len;
    while (isalnum(peek(lexer)) || match(lexer, '_'))
    {
        mov(lexer);
        ++tok.val.text.len;
    }

    tstream_push(stream, tok);
    return 0;
}

void
lex_start(struct lexer_t *const lexer,
                struct tstream_t *stream)
{
    /* A boolean flag indicating that the lexing went wrong.  */
    bool error = false;
    (void)error;

    /* State dumb variable for repeated calls to `mbrtoc32'.  */
    mbstate_t st1 = {0};

    /* Start scanning input.  */
    while (!eof(lexer))
    {
        /* Skip any white space before the next token, if any.  */
        skip_blank(lexer);

        /* Exit execution flow if pointer is at end of file.  */
        if (eof(lexer))
            break;

        /* Reset memory for the next token to be scanned and parsed.  */
        // lxm_reset(lexer);

        /* Amount of read bytes from the multibyte string.
           Its value will be added at the end of each
           iteration to jump exactly the read bytes.  */
        size_t offset;

        /* Wide byte representation of the current character
           being parsed; used only to get the code point and
           to save the correct lexeme.  */
        char32_t c32;

        /* Extract the next multibyte character from the
           string.  As the size of the input string is and
           should remain unknown, SIZE_MAX indicates `mbrtoc32'
           read as much as it can.  Offset should be set to be
           the amount of bytes read; when its value is more than
           one, we have consumed a multibyte character.  */
        offset = mbrtoc32(&c32, (char const *) current(lexer), SIZE_MAX, &st1);

        /* Invalid input.  */
        if (offset == (size_t) -1)
        {
            error = true;
            break;
        }

        /* Truncated input.  */
        if (offset == (size_t) -2)
        {
            error = true;
            break;
        }

        // if (offset < 0)
        // {
        //     // catch EILSEQ
        // }

#ifndef ch32_case
#define ch32_case(c32, typ) \
    case c32: \
    { \
        tstream_push(stream, (struct token_t) { .type = typ }); \
        movn(lexer, (uint32_t)offset); \
        continue; \
    }
#endif

        switch (c32)
        {
            default: break;
            ch32_case(0x000000F7, TOK_DIV_2)
            ch32_case(0x0000230A, TOK_LFLOOR)
            ch32_case(0x0000230B, TOK_RFLOOR)
            ch32_case(0x00002308, TOK_LCEILING)
            ch32_case(0x00002309, TOK_RCEILING)
            ch32_case(0x00002229, TOK_SET_INTER)  /* Set operators coming below.  */
            ch32_case(0x0000222A, TOK_SET_UNION)
            ch32_case(0x00002286, TOK_SET_SUB)
            ch32_case(0x00002284, TOK_SET_NSUB)
            ch32_case(0x00002282, TOK_SET_PROPSUB)
            ch32_case(0x00002287, TOK_SET_SUPER)
            ch32_case(0x00002285, TOK_SET_NSUPER)
            ch32_case(0x00002283, TOK_SET_PROPSUPER)
            ch32_case(0x00002206, TOK_SET_SYMMDIFF)
            ch32_case(0x00002208, TOK_SET_ELEMOF)
            ch32_case(0x00002209, TOK_SET_NELEMOF)
            ch32_case(0x000000D7, TOK_SET_CARTPROD)
            ch32_case(0x000000D8, TOK_SET_EMPTY)
        }

#undef ch32_case

        /* Lex and identifier.  */
        if (IS_IDENTIFIER(c32))
        {
            (void)lex_identifier(lexer, stream);
            continue;
        }

        /* Skip a comment block.  */
        if (match(lexer, '{')
            && match_at(lexer, 1, '{')
            && match_at(lexer, 2, '*'))
        {
            skip_comment(lexer);
            continue;
        }

        /* Lex a string.  */
        if (match(lexer, '"'))
        {
            (void)lex_string(lexer, stream);
            continue;
        }

        /* Lex a number.  */
        if (isdigit(peek(lexer)) || match(lexer, '.'))
        {
            if (match(lexer, '.') && match_at(lexer, 1, '.'))
            {
                if (match_at(lexer, 2, '.'))
                {
                    tstream_push(stream, (struct token_t) { .type = TOK_ELLIPSIS });
                    movn(lexer, 3);
                    continue;
                }

                tstream_push(stream, (struct token_t) { .type = TOK_RANGE });
                movn(lexer, 2);
                continue;
            }

            (void)lex_number(lexer, stream);
            continue;
        }

        /* Set the default type of the next token to be scanned.  */
        enum token_type type = TOK_UNK;

#ifndef ch8_case1
#define ch8_case1(ch, typ) \
    case ch: \
    { \
        type = typ; \
        break; \
    }
#endif

#ifndef ch8_case2
#define ch8_case2(ch1, type1, ch2, type2) \
    case ch1: \
    { \
        type = type1; \
        if (!eof_at(lexer, 1) && match_at(lexer, 1, ch2)) \
        { \
            type = type2; \
            mov(lexer); \
        } \
        break; \
    }
#endif

#ifndef ch8_case3
#define ch8_case3(ch1, type1, ch2, type2, ch3, type3) \
    case ch1: \
    { \
        type = type1; \
        if (!eof_at(lexer, 1)) \
        { \
            if (match_at(lexer, 1, ch2)) \
            { \
                type  = type2; \
                mov(lexer); \
            } \
            else if (match_at(lexer, 1, ch3)) \
            { \
                type  = type3; \
                mov(lexer); \
            } \
        } \
        break; \
    }
#endif

#ifndef ch8_case4
#define ch8_case4(ch1, type1, ch2, type2, ch3, type3, ch4, type4) \
    case ch1: \
    { \
        type = type1; \
        if (!eof_at(lexer, 1)) \
        { \
            if (match_at(lexer, 1, ch2)) \
            { \
                type  = type2; \
                mov(lexer); \
            } \
            else if (match_at(lexer, 1, ch3)) \
            { \
                type  = type3; \
                mov(lexer); \
            } \
            else if (match_at(lexer, 1, ch4)) \
            { \
                type  = type4; \
                mov(lexer); \
            } \
        } \
        break; \
    }
#endif

        switch (peek(lexer))
        {
            default  :  break;
            case '$' :  lex_const(lexer, stream); continue;
            case '\\':  lex_cmd(lexer, stream);   continue;
            ch8_case1('(', TOK_LPAREN)
            ch8_case1(')', TOK_RPAREN)
            ch8_case1('[', TOK_LBRACKET)
            ch8_case1(']', TOK_RBRACKET)
            ch8_case1('{', TOK_LBRACE)
            ch8_case1('}', TOK_RBRACE)

            ch8_case1(',', TOK_COMMA)
            ch8_case1(';', TOK_SEMICOLON)
            ch8_case1('=', TOK_EQ)
            ch8_case1('?', TOK_QMARK)
            ch8_case1('%', TOK_MOD)
            ch8_case1('#', TOK_HASH)
            ch8_case1('@', TOK_ATSIGN)
            ch8_case1('~', TOK_COMPL)
            ch8_case1('^', TOK_XOR)
            ch8_case1('/', TOK_DIV_1)
            ch8_case2('&', TOK_AND,   '&', TOK_AND_AND)
            ch8_case2('|', TOK_OR,    '|', TOK_OR_OR)
            ch8_case2('+', TOK_PLUS,  '+', TOK_INC)
            ch8_case2('-', TOK_MINUS, '-', TOK_DEC)
            ch8_case2('*', TOK_MULT,  '*', TOK_EXP)
            ch8_case2(':', TOK_COLON, '=', TOK_ASSIGN)
            ch8_case2('!', TOK_NOT,   '=', TOK_NEQ_1)
            ch8_case3('>', TOK_GT,    '=', TOK_GTE,   '>', TOK_RSHIFT)
            ch8_case4('<', TOK_LT,    '>', TOK_NEQ_2, '=', TOK_LTE, '<', TOK_LSHIFT)
        }

#undef ch8_case1
#undef ch8_case2
#undef ch8_case3
#undef ch8_case4

        tstream_push(stream, (struct token_t) { .type = type });
        mov(lexer);
    }

    tstream_push(stream, (struct token_t) { .type = TOK_END });
}

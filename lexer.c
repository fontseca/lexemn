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
    ( token_spellings[(token.type)].category )

#define TOK_NAME( token ) \
    ( token_spellings[(token.type)].name )

/* Return non-zero value if C is a non-printable character.  */
#define is_whitespace( c ) \
    ( ( c ) == ' '  || ( c ) == '\t' || \
      ( c ) == '\n' || ( c ) == '\r' || \
      ( c ) == '\v' || ( c ) == '\f' )

/* Return non-zero if the numerical value of the character C is falls within
   the specified closed range [LOWER, UPPER].  */
#define is_between( c, lower, upper ) \
    ( ( c ) >= ( lower ) && ( c ) <= ( upper ) )

/* Return non-zero value if the wide character C typed as char32_t is valid
   as part of an identifier.  In addition to the standard ASCII characters
   normally used (`_', `a-z', `A-Z'), this macro accepts Universal Character
   Names (UCNs) allowed in the C programming language per ISO/IEC 9899:201x
   Annex D: Universal character names for identifiers.  */
#define is_identifier( c )                      \
       (00 /* Standard ASCII characters.  */    \
        || ( c ) == '_'                         \
        || is_between(( c ), 'A', 'Z')          \
        || is_between(( c ), 'a', 'z')          \
        ||                                      \
        00 /* Latin-1 supplement (part 1).  */  \
        || is_between(( c ), 0x00B2, 0x00B5)    \
        || is_between(( c ), 0x00B7, 0x00BA)    \
        || is_between(( c ), 0x00BC, 0x00BE)    \
        || is_between(( c ), 0x00C0, 0x00D6)    \
        ||                                      \
        00 /* Latin-1 supplement (part 2).  */  \
        || is_between(( c ), 0x00D8, 0x00F6)    \
        || is_between(( c ), 0x00F8, 0x00FF)    \
        ||                                      \
        00 /* Extended Latin, Greek, Cyrillic,  \
        00  Hebrew, Arabic, etc.  */            \
        || is_between(( c ), 0x0100, 0x167F)    \
        || is_between(( c ), 0x1681, 0x180D)    \
        || is_between(( c ), 0x180F, 0x1FFF)    \
        ||                                      \
        00 /* Formatting, directional controls, \
        00  & combining marks.  */              \
        || is_between(( c ), 0x200B, 0x200D)    \
        || is_between(( c ), 0x202A, 0x202E)    \
        || is_between(( c ), 0x203F, 0x2040)    \
        || is_between(( c ), 0x2060, 0x206F)    \
        ||                                      \
        00 /* Technical symbols, number forms,  \
        00  enclosed alphanumerics.  */         \
        || is_between(( c ), 0x2070, 0x218F)    \
        || is_between(( c ), 0x2460, 0x24FF)    \
        || is_between(( c ), 0x2776, 0x2793)    \
        || is_between(( c ), 0x2C00, 0x2DFF)    \
        || is_between(( c ), 0x2E80, 0x2FFF)    \
        ||                                      \
        00 /* CJK symbols & punctuation.  */    \
        || is_between(( c ), 0x3004, 0x3007)    \
        || is_between(( c ), 0x3021, 0x302F)    \
        || is_between(( c ), 0x3031, 0x303F)    \
        ||                                      \
        00 /* Unified East Asian scripts        \
        00  (primary CJK block).  */            \
        || is_between(( c ), 0x3040, 0xD7FF)    \
        ||                                      \
        00 /* CJK Compatibility ideographs &    \
        00  Arabic presentation forms.  */      \
        || is_between(( c ), 0xF900, 0xFD3D)    \
        || is_between(( c ), 0xFD40, 0xFDCF)    \
        || is_between(( c ), 0xFDF0, 0xFE44)    \
        || is_between(( c ), 0xFE47, 0xFFFD)    \
        ||                                      \
        00 /* Supplementary  ideographic planes \
        00  (plane 1 - plane 14).  Contains     \
        00  supplementary multilingual (SMP),   \
        00  supplementary ideographic (SIP),    \
        00  and tertiary ideographic (TIP)      \
        00  planes. */                          \
        || is_between(( c ), 0x10000, 0x1FFFD)  \
        || is_between(( c ), 0x20000, 0x2FFFD)  \
        || is_between(( c ), 0x30000, 0x3FFFD)  \
        || is_between(( c ), 0x40000, 0x4FFFD)  \
        || is_between(( c ), 0x50000, 0x5FFFD)  \
        || is_between(( c ), 0x60000, 0x6FFFD)  \
        || is_between(( c ), 0x70000, 0x7FFFD)  \
        || is_between(( c ), 0x80000, 0x8FFFD)  \
        || is_between(( c ), 0x90000, 0x9FFFD)  \
        || is_between(( c ), 0xA0000, 0xAFFFD)  \
        || is_between(( c ), 0xB0000, 0xBFFFD)  \
        || is_between(( c ), 0xC0000, 0xCFFFD)  \
        || is_between(( c ), 0xD0000, 0xDFFFD)  \
        || is_between(( c ), 0xE0000, 0xEFFFD))

void
lex_setup(struct lexer_t *const lexer,
                char unsigned const *const whence)
{
    lexer->buf = (char unsigned const *)whence;
    lexer->cur = (char unsigned const *)whence;
}

/* Return non-zero value if LEXER has reached the end of the input source at
   the given OFFSET.  */
static bool
eof(struct lexer_t const *const lexer, int32_t const offset)
{
    return '\0' == *(lexer->cur + offset);
}

/* Return a pointer to the current read position in the input source of LEXER.  */
static char unsigned const *
current(struct lexer_t const *lexer)
{
    return lexer->cur;
}

/* Return the character at OFFSET from the current read position in LEXER
   without advancing the pointer.  */
[[nodiscard]]
static char unsigned
peek(struct lexer_t const *const lexer, int32_t const offset)
{
    return *(lexer->cur + offset);
}

/* Return the UTF-32 code point N bytes ahead (or behind if N < 0)
   of the current read position in LEXER, without advancing.  */
[[nodiscard]]
static char32_t
peek_utf32(struct lexer_t const *const lexer,
                int32_t const offset)
{
    char unsigned const *src = lexer->cur + offset;
    size_t ret; char32_t c32; mbstate_t state = { 0 };
    /* When offset is negative, we need to point at the start of the UTF-8 character.
       For that, we walk leftward as long as pointing at a continuation byte.  */
    while (offset < 0 && (*src & 0b1100'0000) == 0b1000'0000)
        --src;
    ret = mbrtoc32(&c32, (char *)src, 4, &state);
    if (ret == (size_t)-1 || ret == (size_t)-2)
        return 0;
    return c32;
}

/* Advance the current read position in LEXER by OFFSET bytes.  */
static void
mov(struct lexer_t *const lexer, int32_t const offset)
{
    lexer->cur += offset;
}

/* Return non-zero value if the character at the current position in the
   input source matches C.  */
static bool
match(struct lexer_t *const lexer, char unsigned const c)
{
    if (eof(lexer, 0) || *lexer->cur != c)
        return false;
    return ++lexer->cur, true;
}

/* Skip any white space at the current position in the input source
   of LEXER.  */
static void
skip_blank(struct lexer_t *const lexer)
{
    while (is_whitespace(*lexer->cur))
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
    match(lexer, '{');
    match(lexer, '{');
    match(lexer, '*');

    while (!eof(lexer, 0))
    {
        /* Look for the closing `*' character.  */
        if (peek(lexer, 0) != '*')
        {
            mov(lexer, 1);
            continue;
        }

        /* If found, check if it is immediately followed by the
          sequence `}}'.  */
        if (peek(lexer, 1) == '}' && peek(lexer, 2) == '}')
        {
            /* Consume the closing comment bytes `*}}' and finish.  */
            mov(lexer, 3);
            return;
        }

        /* Move one byte forward otherwise.  */
        mov(lexer, 1);
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
    match(lexer, '"');  /* Skip opening `"'.  */
    struct token_t tok = { .type = TOK_STRING };
    tok.val.text.str = current(lexer);

    while (!eof(lexer, 0) && peek(lexer, 0) != '"')
    {
        ++tok.val.text.len;
        mov(lexer, 1);
    }

    match(lexer, '"'); /* Skip closing `"'.  */
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
        if (!is_identifier(c32) && !isdigit(peek(lexer, 0)))
        {
            /* Character found is not valid to be part of an identifier.  */
            break;
        }

        mov(lexer, (int)offset);
        tok.val.text.len += offset;
    }

    tstream_push(stream, tok);
    return 0;
}

/* Scan a number literal value at the current read position in the input source of
   LEXER and push it as a token onto STREAM. The number can be expressed in any of
   the following numerical bases: binary (prefixed with `0b' or `0B'), octal (prefixed
   with `0o' or `0O'), hexadecimal (prefixed with `0x' or `0X'), or decimal, the default
   when no prefix is provided. To enhance the readability of big literals, an optional
   underscore character (`_') can be placed between the digits.

   Decimal and hexadecimal literals allow a radix point (`.') to split the whole number
   part from the fractional part.  At most one point is permitted, otherwise a syntax
   error is raised.  Very large or small numbers can be written in scientific notation
   by using the letters `E' or `e' for decimal, and `P' or `p' for hexadecimals.  The
   significand is a real number, whereas the exponent is always a base-10 integer;
   trying to use a dot in the exponent raises a syntax error.  If the number is base-16
   and the exponent contains any of the hexadecimal letters, the number is truncated up
   to the last valid decimal digit.

   Lexemn supports arithmetic multiplication by juxtaposition to type expressions
   in a more natural way. Trying to multiply the factors `2' and `x' can be arranged
   as `2x', without having to write `2 * x'.  Note that the aforementioned symbols
   `e', `p' and `_' are completely valid identifiers on their own.  Consequently, in
   expressions such as `2e', `2eee', `2e++', `2_', `2__1', and `2e_', they lose their
   special literal meaning and are scanned as regular names rather than triggering
   syntax errors.  To prevent unexpected behavior, numeric literals must be formatted
   unambiguously.

   When two successive dots are encountered in the middle of a number, they are scanned
   as a range operator.  The operands of a range can be either integer or floating-point
   constants: `1.5..5.5' evaluates to the sequence of numbers between `1.5' and `5.5'.
   Likewise, in an expression such as `.0...10', the three consecutive dots are scanned
   as an ellipsis operator; however, if `.10' is intended to be a fractional number, it
   must be explicitly written as `.0..0.10'.  */
[[nodiscard]]
static int
lex_number(struct lexer_t *const lexer,
                struct tstream_t *const stream)
{
    enum : char unsigned
    {
        HEX = 1,
        DEC = HEX << 1,
        OCT = HEX << 2,
        BIN = HEX << 3
    };
    short unsigned radix  = DEC; /* Default number base.  */
    struct token_t number = { .type = TOK_NUMBER, .val = { .text = { .str = current(lexer) } } };
    static constexpr char unsigned mask[256] =
    {
        ['0'] = HEX | DEC | OCT | BIN,
        ['1'] = HEX | DEC | OCT | BIN,
        ['2'] = HEX | DEC | OCT,
        ['3'] = HEX | DEC | OCT,
        ['4'] = HEX | DEC | OCT,
        ['5'] = HEX | DEC | OCT,
        ['6'] = HEX | DEC | OCT,
        ['7'] = HEX | DEC | OCT,
        ['8'] = HEX | DEC,
        ['9'] = HEX | DEC,
        ['a'] = HEX,
        ['b'] = HEX,
        ['c'] = HEX,
        ['d'] = HEX,
        ['e'] = HEX,
        ['f'] = HEX,
        ['A'] = HEX,
        ['B'] = HEX,
        ['C'] = HEX,
        ['D'] = HEX,
        ['E'] = HEX,
        ['F'] = HEX,
    };

    /* Check if the character C is a legal digit for the base system of the current
       number being scanned.  */
#define valid_in_radix(c) \
    ( radix & mask[( c )] )

    if (match(lexer, '0'))
    {
        ++number.val.text.len;
        switch (peek(lexer, 0))
        {
            default: radix = DEC; break;
            case 'b': case 'B': radix = BIN; break;
            case 'o': case 'O': radix = OCT; break;
            case 'x': case 'X': radix = HEX; break;
        }

        if (radix != DEC)
        {
            char unsigned next_ch = peek(lexer, 1);

            /* If the character following the radix indicator is invalid according
               to these bases, tokenize the just consumed `0' as a regular decimal
               and let the lexer scan the indicator as an identifier.  When entering
               an expression such as `0b', it is interpreted as `0' times `b'.  */
            if ((radix == BIN || radix == OCT) && !valid_in_radix(next_ch))
                goto assmbl_number;

            /* If the expression entered is something like `0x.' or `0x..', instead
               of trying to guess how to interpret it, raise a syntax error.  */
            if (radix == HEX && next_ch == '.' && !valid_in_radix(next_ch = peek(lexer, 2)))
            {
                return -1; /* Invalid syntax: no digits in hexadecimal floating constant.  */
            }

            /* If the expression entered is something like `0xG' or `0x++', then just
               tokenize the `0' as a regular decimal, and let the scanner decide what
               to do with the remaining sequence of characters.  */
            if (radix == HEX && !valid_in_radix(next_ch))
                goto assmbl_number;

            /* Otherwise append the base indicator to the `0' of the non-decimal number.  */
            ++number.val.text.len;
            mov(lexer, 1);
        }
    }

    short unsigned exponents = 0; /* Count of exponent indicators.  */
    short unsigned points = 0;    /* Count of radix points.  */

    /* Do scan the actual number literal.  */
    while (!is_whitespace(peek(lexer, 0)) && !eof(lexer, 0))
    {
        /* Check if range or ellipsis.  */
        if (peek(lexer, 0) == '.' && peek(lexer, 1) == '.')
        {
            bool const ellipsis = peek(lexer, 2) == '.';
            tstream_push(stream, number);
            tstream_push(stream, (struct token_t) { .type = ellipsis ? TOK_ELLIPSIS : TOK_RANGE });
            mov(lexer, ellipsis ? 3 : 2);
            return 0;
        }

        auto const ch = peek(lexer, 0);

        /* Check if radix point.  */
        if (ch == '.')
        {
            /* This symbol separates the integer number from the fractional part.
               It is only supported in hexadecimal and decimal.  An syntax error
               is trigger if used in another abase.  */
            if (radix != DEC && radix != HEX)
            {
                return -1; /* Invalid syntax: radix separator is not permitted
                              in this base.  */
            }

            /* When there are multiple occurrences of a point raise a syntax error.
               An expression such as `1.2.3' is illegal.  */
            if (++points > 1)
            {
                return -1; /* Invalid syntax: too much points.  */
            }

            /* If the number is written in scientific notation, the exponent cannot
               cannot be expressed as a floating-point. An expression such as `2e1.5'
               is illegal.  */
            if (exponents)
            {
                return -1; /* Invalid syntax: no points allowed after exponent.  */
            }

            ++number.val.text.len;
            mov(lexer, 1);
            continue;
        }

        /* Check if scientific notation.  */
        if ((radix == DEC && (ch == 'e' || ch == 'E'))
            || (radix == HEX && (ch == 'p' || ch == 'P')))
        {
            /* If the number was previously accepted as written in scientific notation, but
               there is another occurrence of an `e' that is not a sequence, the expression
               is illegal due to ambiguities. For instance: `2e2e2', it can be interpreted
               in several ways, so the best case here is to raise a syntactic error.  */
            if (++exponents > 1)
            {
                return -1; /* Syntax error: too much exponent indicators in number.  */
            }

            /* The character after the exponent indicator.  */
            auto const next_ch = peek(lexer, 1);

            /* True when the number is express as `2e+6'.  */
            bool const has_sign = next_ch == '+' || next_ch == '-';

            /* Select the first digit of the exponent to later ascertain it is a valid
               base-10 number.  */
            auto const digit_ch = has_sign ? peek(lexer, 2) : next_ch;

            if (!(DEC & mask[digit_ch]))
                goto assmbl_number;

            if (has_sign)
            {
                ++number.val.text.len; /* Consume the `+' or `-'.  */
                mov(lexer, 1);
            }

            /* Consume the exponent indicator.  */
            ++number.val.text.len;
            mov(lexer, 1);
            continue;
        }

        /* Check if thousand separator.  */
        if (ch == '_')
        {
            auto const next = peek(lexer, 1);
            /* If the character following the thousand  separator is not a valid digit
               in the current  number's base, scan the number and let the lexer decide
               what do next.  For example, if entered, `1_000_a', the ending `_a' must
               be treated as an identifier,  resulting in `1_000*_a'; but if entering
               `0x1_000_a',  the `_a'  is completely permitted as part of a hexadecimal
               number.  */
            if (!valid_in_radix(next))
                goto assmbl_number;

            ++number.val.text.len;
            mov(lexer, 1);
            continue;
        }

        /* Check if the current character belongs to the numerical base of the number
           being scanned.  If the number is in scientific notation and we are scanning
           the exponent, we must assert it is in base-10.  */
        if ( !((exponents ? DEC : radix) & mask[ch]) )
        {
            /* If a digit is found so far, then the number was expressed incorrectly
               and a syntax error must be raised.  For instance the `2' in `0b01012'.  */
            if (isdigit(ch))
            {
                return -1; /* Invalid syntax: digit cannot be part of current base.  */
            }

            break;
        }

        ++number.val.text.len;
        mov(lexer, 1);
    }

assmbl_number:
    tstream_push(stream, number);
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
static int
lex_cmd(struct lexer_t *const lexer,
            struct tstream_t *const stream)
{
    if (eof(lexer, 1) || is_whitespace(peek(lexer, 1)))
    {
        // error = true; /* unknown command  */
        return -1;
    }

    /* Parse command name.  */

    struct token_t cmd = { .type = TOK_CMD };
    cmd.val.text.str = current(lexer);
    match(lexer, '\\');
    ++cmd.val.text.len;

    while (!is_whitespace(peek(lexer, 0)) && !eof(lexer, 0))
    {
        mov(lexer, 1);
        ++cmd.val.text.len;
    }

    tstream_push(stream, cmd);

    /* Parse command arguments (if any).  */

    /* Skip any white space in is between the command arguments.  */
    while (is_whitespace(peek(lexer, 0)))
        mov(lexer, 1);

    /* If end of string, then we exit.  */
    if (eof(lexer, 0))
        return 0;

    /* As long as we are not at the end of the string,
      then we are free to scan arguments.  */
    while (!eof(lexer, 0))
    {
        /* These arguments are expected to be separated by white spaces,
           so if any is found,  we must skip it until we find a parsable
           token.  */
        while (is_whitespace(peek(lexer, 0)))
            mov(lexer, 1);

        struct token_t arg = { .type = TOK_CMD_ARG };
        arg.val.text.str = current(lexer);

        /* Check if current pointer is the beginning of a string; in such case,
           then we need to parse the remaining values as the are until finding
           the closing string character. */
        if (match(lexer, '"') || match(lexer, '\''))
        {
            ++arg.val.text.len;
            while (!eof(lexer, 0) && peek(lexer, 0) != '"' && peek(lexer, 0) != '\'')
            {
                mov(lexer, 1);
                ++arg.val.text.len;
            }

            mov(lexer, 1);
            ++arg.val.text.len;
            tstream_push(stream, arg);
            continue;
        }

        /* If end of string, then we exit.  */
        if (eof(lexer, 0))
            continue;

        /* At this point we have found a valid argument.  */
        while (!is_whitespace(peek(lexer, 0)) && !eof(lexer, 0))
        {
            mov(lexer, 1);
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
    if (eof(lexer, 1) || (!isalnum(peek(lexer, 1)) && peek(lexer, 1) != '_'))
    {
        // error = true; /* malformed constant  */
        return -1;
    }

    struct token_t tok = { .type = TOK_CONST };
    tok.val.text.str = current(lexer);
    ++tok.val.text.len;
    match(lexer, '$');

    while (isalnum(peek(lexer, 0)) || peek(lexer, 0) == '_')
    {
        mov(lexer, 1);
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
    while (!eof(lexer, 0))
    {
        /* Skip any white space before the next token, if any.  */
        skip_blank(lexer);

        /* Exit execution flow if pointer is at end of file.  */
        if (eof(lexer, 0))
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
        mov(lexer, (int)offset); \
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

        /* Lex and identifier.  */
        if (is_identifier(c32))
        {
            (void)lex_identifier(lexer, stream);
            continue;
        }

        /* Skip a comment block.  */
        if (peek(lexer, 0) == '{' && peek(lexer, 1) == '{' && peek(lexer, 2) == '*')
        {
            skip_comment(lexer);
            continue;
        }

        /* Lex a string.  */
        if (peek(lexer, 0) == '"')
        {
            (void)lex_string(lexer, stream);
            continue;
        }

        /* Lex a number.  */
        if (isdigit(peek(lexer, 0)) || peek(lexer, 0) == '.')
        {
            /* Range and ellipsis check.  */
            if (peek(lexer, 0) == '.' && peek(lexer, 1) == '.')
            {
                bool const ellipsis = peek(lexer, 2) == '.';
                tstream_push(stream, (struct token_t) { .type = ellipsis ? TOK_ELLIPSIS : TOK_RANGE });
                mov(lexer, ellipsis ? 3 : 2);
                continue;
            }

            /* Dot operator check.  */
            if (peek(lexer, 0) == '.' && !isdigit(peek(lexer, 1)))
            {
                tstream_push(stream, (struct token_t) { .type = TOK_DOT });
                mov(lexer, 1);
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
        if (!eof(lexer, 1) && peek(lexer, 1) == ch2) \
        { \
            type = type2; \
            mov(lexer, 1); \
        } \
        break; \
    }
#endif

#ifndef ch8_case3
#define ch8_case3(ch1, type1, ch2, type2, ch3, type3) \
    case ch1: \
    { \
        type = type1; \
        if (!eof(lexer, 1)) \
        { \
            if (peek(lexer, 1) == ch2) \
            { \
                type  = type2; \
                mov(lexer, 1); \
            } \
            else if (peek(lexer, 1) == ch3) \
            { \
                type  = type3; \
                mov(lexer, 1); \
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
        if (!eof(lexer, 1)) \
        { \
            if (peek(lexer, 1) == ch2) \
            { \
                type  = type2; \
                mov(lexer, 1); \
            } \
            else if (peek(lexer, 1) == ch3) \
            { \
                type  = type3; \
                mov(lexer, 1); \
            } \
            else if (peek(lexer, 1) == ch4) \
            { \
                type  = type4; \
                mov(lexer, 1); \
            } \
        } \
        break; \
    }
#endif

        switch (peek(lexer, 0))
        {
            default  :  break;
            case '$' :  lex_const(lexer, stream); continue;
            case '\\':  lex_cmd(lexer, stream);   continue;
            case '+':
            {
                type = TOK_PLUS;
                if (peek(lexer, 1) == '+')
                {
                    if (is_identifier(peek_utf32(lexer, -1))
                        || is_identifier(peek_utf32(lexer, 2)))
                    {
                        type = TOK_INC;
                        mov(lexer, 1);
                    }
                }

                break;
            }
            case '-':
            {
                type = TOK_MINUS;
                if (peek(lexer, 1) == '-')
                {
                    if (is_identifier(peek_utf32(lexer, -1))
                        || is_identifier(peek_utf32(lexer, 2)))
                    {
                        type = TOK_DEC;
                        mov(lexer, 1);
                    }
                }

                break;
            }
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
            ch8_case2('*', TOK_MULT,  '*', TOK_EXP)
            ch8_case2(':', TOK_COLON, '=', TOK_ASSIGN)
            ch8_case2('!', TOK_NOT,   '=', TOK_NEQ_1)
            ch8_case3('>', TOK_GT,    '=', TOK_GTE,   '>', TOK_RSHIFT)
            ch8_case4('<', TOK_LT,    '>', TOK_NEQ_2, '=', TOK_LTE, '<', TOK_LSHIFT)
        }

        tstream_push(stream, (struct token_t) { .type = type });
        mov(lexer, 1);
    }

    tstream_push(stream, (struct token_t) { .type = TOK_END });
}

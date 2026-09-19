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
#include <sys/stat.h>

#include "lexemn.h"
#include "lex.h"
#include "errors.h"

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

/* Return the UTF-32 code point N bytes ahead (or behind if N < 0)
   of the current read position in LEXER, without advancing.  */
[[nodiscard]]
static size_t
peek_utf32(struct lexemn const *const lexemn,
                int32_t offset, char32_t *const result)
{
    char unsigned const *src = lexemn->p_head->cur + offset;
    size_t ret; char32_t c32; mbstate_t state = { 0 };
    /* When offset is negative, we need to point at the start of the UTF-8 character.
       For that, we walk leftward as long as pointing at a continuation byte.  */
    while (offset < 0 && (*src & 0b1100'0000) == 0b1000'0000)
        --src;
    ret = mbrtoc32(&c32, (char *)src, 4, &state);
    if (ret == (size_t)-1 || ret == (size_t)-2)
        return 0;
    if (result)
        *result = c32;
    return ret;
}

/* Skip any white space at the current position in the input source
   of LEXER.  */
static void
skip_blank(struct lexemn const *const lexemn)
{
    while (is_whitespace(lexemn->p_head->cur[0]))
    {
        if (lexemn->p_head->cur[0] == '\n')
        {
            ++lexemn->p_head->line;
            lexemn->p_head->line_start = lexemn->p_head->cur+1;
        }
        ++lexemn->p_head->cur;
    }
}

/* Skip the comment sequence at the current position in the input source
   of LEXER.  */
static void
skip_comment(struct lexemn const *const lexemn)
{
    lexemn->p_head->cur += 2; /* Skip the comment opening.  */
    while (lexemn->p_head->cur[0] != '\0')
    {
        /* Look for the closing `*' character.  */
        if (lexemn->p_head->cur[0] != '*')
        {
            if (lexemn->p_head->cur[0] == '\n')
            {
                ++lexemn->p_head->line;
                lexemn->p_head->line_start = lexemn->p_head->cur+1;
            }
            ++lexemn->p_head->cur;
            continue;
        }

        /* If found, check if it is immediately followed by a `/'.  */
        if (lexemn->p_head->cur[0] == '*'
            && lexemn->p_head->cur[1] == '/')
        {
            /* Consume the closing comment bytes and finish.  */
            lexemn->p_head->cur += 2;
            return;
        }

        /* Move one byte forward otherwise.  */
        ++lexemn->p_head->cur;
    }
}

/* Append TOKEN to the end of STREAM.  */
static void
tstream_push(struct lexemn *const lexemn,
                    struct token const token)
{
    if (1 + lexemn->stream.size > lexemn->stream.capacity)
    {
        size_t const cap = lexemn->stream.capacity < 8
                                ? 8 : 2 * lexemn->stream.capacity;
        struct token *buffer = realloc(lexemn->stream.tokens,
                                                cap * sizeof(struct token ));
        if (!buffer)
        {
            perror("Fatal failure");
            free(lexemn->stream.tokens);
            exit(EXIT_FAILURE);
        }
        lexemn->stream.tokens = buffer;
        lexemn->stream.capacity = cap;
    }

    lexemn->stream.tokens[lexemn->stream.size++] = token;
}

/* Scan a string at the current position in the input source of LEXER
   and push it onto STREAM.  Escaped backslashes (`\\') and double quotes
   (`\"') are consumed as part of the token.  */
static void
lex_string(struct lexemn const *const lexemn,
                struct token *const result)
{
    ++lexemn->p_head->cur; /* Skip opening `"'.  */
    result->val.text.str = lexemn->p_head->cur;

    while (lexemn->p_head->cur[0] != '\0'
            && lexemn->p_head->cur[0] != '"')
    {
        if (lexemn->p_head->cur[0] == '\0'
            || lexemn->p_head->cur[0] == '\n')
        {
            error_at(lexemn, result->loc, 0,
                "missing terminating ‘\"’ character in string literal");
            return;
        }

        if (lexemn->p_head->cur[0] == '\\')
        {
            auto const next_ch = lexemn->p_head->cur[1];
            if (next_ch == '\\' || next_ch == '"')
            {
                ++lexemn->p_head->cur;
                ++result->val.text.len;
            }
        }
        ++result->val.text.len;
        ++lexemn->p_head->cur;
    }

    if (lexemn->p_head->cur[0] == '\0'
        || lexemn->p_head->cur[0] == '\n')
    {
        error_at(lexemn, result->loc, 0,
            "missing terminating ‘\"’ character in string literal");
        return;
    }

    ++lexemn->p_head->cur; /* Skip closing `"'.  */
}

/* Scan an identifier at the current position in the input source of LEXER
   and push it onto STREAM.  */
static void
lex_identifier(struct lexemn const *const lexemn,
                    struct token *const result)
{
    size_t offset;
    char32_t c32;
    mbstate_t st1 = {0};
    result->val.text.str = lexemn->p_head->cur;

    /* Keep lexing the identifier.  */
    for (;;)
    {
        offset = mbrtoc32(&c32, (char const *) lexemn->p_head->cur, SIZE_MAX, &st1);
        if (!is_identifier(c32) && !isdigit(lexemn->p_head->cur[0]))
        {
            /* Character found is not valid to be part of an identifier.  */
            break;
        }

        lexemn->p_head->cur += offset;
        result->val.text.len += offset;
    }
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
static void
lex_number(struct lexemn const *const lexemn,
                struct token *const result)
{
    enum : char unsigned
    {
        HEX = 1,
        DEC = HEX << 1,
        OCT = HEX << 2,
        BIN = HEX << 3
    };
    short unsigned radix  = DEC; /* Default number base.  */
    result->val.text.str = lexemn->p_head->cur;
    auto const start = lexemn->p_head->cur;
    bool seen_error = false;
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

    if (lexemn->p_head->cur[0] == '0')
    {
        ++result->val.text.len;
        ++lexemn->p_head->cur;
        switch (lexemn->p_head->cur[0])
        {
            default: radix = DEC; break;
            case 'b': case 'B': radix = BIN; break;
            case 'o': case 'O': radix = OCT; break;
            case 'x': case 'X': radix = HEX; break;
        }

        if (radix != DEC)
        {
            char unsigned next_ch = lexemn->p_head->cur[1];

            /* If the character following the radix indicator is invalid according
               to these bases, tokenize the just consumed `0' as a regular decimal
               and let the lexer scan the indicator as an identifier.  When entering
               an expression such as `0b', it is interpreted as `0' times `b'.  */
            if ((radix == BIN || radix == OCT) && !valid_in_radix(next_ch))
                goto assmbl_number;

            /* If the expression entered is something like `0x.' or `0x..', instead
               of trying to guess how to interpret it, raise a syntax error.  */
            if (radix == HEX && next_ch == '.'
                && !valid_in_radix(next_ch = lexemn->p_head->cur[2]))
            {
                seen_error = true;
                error_at(lexemn, result->loc, 2,
                            "expected digits in hexadecimal floating literal");
                result->val.text.len += 2;
                lexemn->p_head->cur += 2;
                goto assmbl_number;
            }

            /* If the expression entered is something like `0xG' or `0x++', then just
               tokenize the `0' as a regular decimal, and let the scanner decide what
               to do with the remaining sequence of characters.  */
            if (radix == HEX && !valid_in_radix(next_ch))
                goto assmbl_number;

            /* Otherwise append the base indicator to the `0' of the non-decimal number.  */
            ++result->val.text.len;
            ++lexemn->p_head->cur;
        }
    }

    short unsigned exponents = 0; /* Count of exponent indicators.  */
    short unsigned points = 0;    /* Count of radix points.  */

    /* Do scan the actual number literal.  */
    while (!is_whitespace(lexemn->p_head->cur[0])
            && lexemn->p_head->cur[0] != '\0')
    {
        auto const ch = lexemn->p_head->cur[0];
        auto const ch_col = (uint32_t)(lexemn->p_head->cur - start);

        /* Check if range or ellipsis.  */
        if (ch == '.' && lexemn->p_head->cur[1] == '.')
            goto assmbl_number;

        /* Check if radix point.  */
        if (ch == '.')
        {
            /* When there are multiple occurrences of a point raise a syntax error.
               An expression such as `1.2.3' is illegal.  */
            if (++points > 1 && !seen_error)
            {
                seen_error = true;
                error_at(lexemn, result->loc, (uint32_t)result->val.text.len,
                            "too many decimal points in numeric literal");
            }

            /* This symbol separates the integer number from the fractional part.
               It is only supported in hexadecimal and decimal.  An syntax error
               is trigger if used in another abase.  */
            if (radix != DEC && radix != HEX && !seen_error)
            {
                seen_error = true;
                error_at(lexemn, result->loc, ch_col,
                            "fractional notation not supported in %s literals",
                                radix == BIN ? "binary" : "octal");
            }

            /* If the number is written in scientific notation, the exponent cannot
               cannot be expressed as a floating-point. An expression such as `2e1.5'
               is illegal.  */
            if (exponents && !seen_error)
            {
                seen_error = true;
                error_at(lexemn, result->loc, ch_col,
                    "exponent in scientific notation must be an integer");
            }

            ++result->val.text.len;
            ++lexemn->p_head->cur;
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
            if (++exponents > 1 && !seen_error)
            {
                seen_error = true;
                error_at(lexemn, result->loc, ch_col,
                    "too many exponent indicators in numeric literal");
            }

            /* The character after the exponent indicator.  */
            auto const next_ch = lexemn->p_head->cur[1];

            /* True when the number is express as `2e+6'.  */
            bool const has_sign = next_ch == '+' || next_ch == '-';

            /* Select the first digit of the exponent to later ascertain it is a valid
               base-10 number.  */
            auto const digit_ch = has_sign ? lexemn->p_head->cur[2] : next_ch;

            if (!(DEC & mask[digit_ch]))
                goto assmbl_number;

            if (has_sign)
            {
                ++result->val.text.len; /* Consume the `+' or `-'.  */
                ++lexemn->p_head->cur;
            }

            /* Consume the exponent indicator.  */
            ++result->val.text.len;
            ++lexemn->p_head->cur;
            continue;
        }

        /* Check if thousand separator.  */
        if (ch == '_')
        {
            auto const next = lexemn->p_head->cur[1];
            /* If the character following the thousand  separator is not a valid digit
               in the current  number's base, scan the number and let the lexer decide
               what do next.  For example, if entered, `1_000_a', the ending `_a' must
               be treated as an identifier,  resulting in `1_000*_a'; but if entering
               `0x1_000_a',  the `_a'  is completely permitted as part of a hexadecimal
               number.  */
            if (!valid_in_radix(next))
                goto assmbl_number;

            ++result->val.text.len;
            ++lexemn->p_head->cur;
            continue;
        }

        /* Check if the current character belongs to the numerical base of the number
           being scanned.  If the number is in scientific notation and we are scanning
           the exponent, we must assert it is in base-10.  */
        if ( !((exponents ? DEC : radix) & mask[ch]) )
        {
            /* If a digit is found so far, then the number was expressed incorrectly
               and a syntax error must be raised.  For instance the `2' in `0b01012'.  */
            if (isdigit(ch) && !seen_error)
            {
                error_at(lexemn, result->loc, ch_col,
                    "invalid digit ‘%c’ in %s literal",
                        ch, radix == BIN ? "binary" :
                                radix == OCT ? "octal" :
                                    radix == HEX ? "hexadecimal" : "decimal");
            }

            break;
        }

        ++result->val.text.len;
        ++lexemn->p_head->cur;
    }

assmbl_number:
    if (seen_error)
        result->type = TOK_UNK;
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
static void
lex_cmd(struct lexemn *const lexemn, struct token *const cmd)
{
    lexemn->lex_stat.in_cmd = true;
    lexemn->lex_stat.run_cmd = true;

    /* Parse command name.  */

    cmd->type = TOK_CMD;
    cmd->val.text.str = lexemn->p_head->cur;
    ++lexemn->p_head->cur; /* Skip the `\'. */
    ++cmd->val.text.len;

    if (lexemn->p_head->cur[0] == '\0'
        || is_whitespace(lexemn->p_head->cur[0]))
    {
        cmd->type = TOK_UNK;
        lexemn->lex_stat.run_cmd = false;
        error_at(lexemn, cmd->loc, 0,
            "ill-formed meta-command; did you miss the name?");
    }

    while (!is_whitespace(lexemn->p_head->cur[0])
            && lexemn->p_head->cur[0] != '\0')
    {
        ++lexemn->p_head->cur;
        ++cmd->val.text.len;
    }

    tstream_push(lexemn, *cmd);

    /* Parse command arguments (if any).  */

    /* Skip any white space in is between the command arguments.  */
    while (is_whitespace(lexemn->p_head->cur[0]))
        ++lexemn->p_head->cur;

    /* If end of string, then we exit.  */
    if (lexemn->p_head->cur[0] == '\0')
        return;

    /* As long as we are not at the end of the string,
      then we are free to scan arguments.  */
    while (lexemn->p_head->cur[0] != '\0')
    {
        /* These arguments are expected to be separated by white spaces,
           so if any is found,  we must skip it until we find a parsable
           token.  */
        while (is_whitespace(lexemn->p_head->cur[0]))
            ++lexemn->p_head->cur;

        struct token arg = { .type = cmd->type == TOK_UNK ? TOK_UNK : TOK_CMD_ARG };
        arg.val.text.str = lexemn->p_head->cur;

        /* Check if current pointer is the beginning of a string; in such case,
           then we need to parse the remaining values as the are until finding
           the closing string character. */
        if (lexemn->p_head->cur[0] == '"' || lexemn->p_head->cur[0] == '\'')
        {
            ++lexemn->p_head->cur;
            ++arg.val.text.len;
            while (lexemn->p_head->cur[0] != '\0'
                && lexemn->p_head->cur[0] != '"'
                && lexemn->p_head->cur[0] != '\'')
            {
                ++lexemn->p_head->cur;
                ++arg.val.text.len;
            }

            ++lexemn->p_head->cur;
            ++arg.val.text.len;
            tstream_push(lexemn, arg);
            continue;
        }

        /* If end of string, then we exit.  */
        if (lexemn->p_head->cur[0] == '\0')
            continue;

        /* At this point we have found a valid argument.  */
        while (!is_whitespace(lexemn->p_head->cur[0]) && lexemn->p_head->cur[0] != '\0')
        {
            ++lexemn->p_head->cur;
            ++arg.val.text.len;
        }

        tstream_push(lexemn, arg);
    }
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
static void
lex_const(struct lexemn const *const lexemn,
                struct token *const result)
{
    result->val.text.str = lexemn->p_head->cur;
    ++lexemn->p_head->cur; /* Skip the `$'.  */
    ++result->val.text.len;

    /* Check if the next character after `$' is a valid identifier.
       It should be any of [0-9a-zA-Z_], otherwise the constant
       is ill-formed.  */
    if (lexemn->p_head->cur[0] == '\0'
        || (!isalnum(lexemn->p_head->cur[0])
            && lexemn->p_head->cur[0] != '_'))
    {
        error_at(lexemn, result->loc, 0,
            "ill-formed constant; did you miss the name?");
        result->type = TOK_UNK;
        return;
    }

    while (isalnum(lexemn->p_head->cur[0]) || lexemn->p_head->cur[0] == '_')
    {
        ++lexemn->p_head->cur;
        ++result->val.text.len;
    }
}

static void
emit_stray_token(struct lexemn const * const lexemn,
                    struct token const *const t)
{
    error_at(lexemn, t->loc, 0, "stray symbol detected in program");
}

static void
lex_next_token(struct lexemn *const lexemn,
                    struct token *const result)
{
fresh_line:
    char unsigned const ch = lexemn->p_head->cur[0];
    uint32_t const file = (uint32_t)(lexemn->p_head - lexemn->p_vec);
    uint32_t const line = lexemn->p_head->line;
    uint32_t const column = 1 + (uint32_t)(lexemn->p_head->cur - lexemn->p_head->line_start);
    result->loc = loc_make(file, line, column);
    switch (ch)
    {
        case '\0':
            result->type = (enum token_type)-1;
            break;
        case ' ': case '\t': case '\n':
        case '\r': case '\v': case '\f':
            skip_blank(lexemn);
            goto fresh_line;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            result->type = TOK_NUMBER;
            lex_number(lexemn, result);
            break;
        case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z':
            result->type = TOK_NAME;
            lex_identifier(lexemn, result);
            break;
        case '"':
            result->type = TOK_STRING;
            lex_string(lexemn, result);
            break;
        case '\\':
            result->type = TOK_UNK;
            if (lexemn->lex_stat.is_shell)
            {
                lex_cmd(lexemn, result);
                result->type = (enum token_type)-1;
                return;
            }
            emit_stray_token(lexemn, result);
            ++lexemn->p_head->cur;
            break;
        case '$':
            result->type = TOK_CONST;
            lex_const(lexemn, result);
            return;
        case '/':
            if (lexemn->p_head->cur[1] == '*')
            {
                skip_comment(lexemn);
                goto fresh_line;
            }
            result->type = TOK_DIV_1;
            ++lexemn->p_head->cur;
            break;
        case '+':
            result->type = TOK_PLUS;
            if (lexemn->p_head->cur[1] == '+')
            {
                char32_t ch1, ch2;
                (void)peek_utf32(lexemn, -1, &ch1);
                (void)peek_utf32(lexemn, 2, &ch2);
                if (is_identifier(ch1) || is_identifier(ch2))
                {
                    result->type = TOK_INC;
                    ++lexemn->p_head->cur;
                }
            }
            ++lexemn->p_head->cur;
            break;
        case '-':
            result->type = TOK_MINUS;
            if (lexemn->p_head->cur[1] == '-')
            {
                char32_t ch1, ch2;
                (void)peek_utf32(lexemn, -1, &ch1);
                (void)peek_utf32(lexemn, 2, &ch2);
                if (is_identifier(ch1) || is_identifier(ch2))
                {
                    result->type = TOK_DEC;
                    ++lexemn->p_head->cur;
                }
            }
            ++lexemn->p_head->cur;
            break;
        case '.':
            char unsigned next = lexemn->p_head->cur[1];
            if (next == '.')
            {
                next = lexemn->p_head->cur[2];
                if (next == '.')
                {
                    result->type = TOK_ELLIPSIS;
                    lexemn->p_head->cur += 3;
                    break;
                }
                result->type = TOK_RANGE;
                lexemn->p_head->cur += 2;
                break;
            }
            if (next == '0' || next == '1' || next == '2' ||
                next == '3' || next == '4' || next == '5' ||
                next == '6' || next == '7' || next == '8' ||
                next == '9')
            {
                result->type = TOK_NUMBER;
                lex_number(lexemn, result);
                break;
            }
            result->type = TOK_DOT;
            ++lexemn->p_head->cur;
            break;
        case '&':
            result->type = TOK_AND;
            if (lexemn->p_head->cur[1] == '&')
            {
                result->type = TOK_AND_AND;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case '|':
            result->type = TOK_OR;
            if (lexemn->p_head->cur[1] == '|')
            {
                result->type = TOK_OR_OR;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case '*':
            result->type = TOK_MULT;
            if (lexemn->p_head->cur[1] == '*')
            {
                result->type = TOK_EXP;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case ':':
            result->type = TOK_COLON;
            if (lexemn->p_head->cur[1] == '=')
            {
                result->type = TOK_ASSIGN;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case '!':
            result->type = TOK_NOT;
            if (lexemn->p_head->cur[1] == '=')
            {
                result->type = TOK_NEQ_1;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case '<':
            result->type = TOK_LT;
            if (lexemn->p_head->cur[1] == '=')
            {
                result->type = TOK_LTE;
                ++lexemn->p_head->cur;
            }
            else if (lexemn->p_head->cur[1] == '>')
            {
                result->type = TOK_NEQ_2;
                ++lexemn->p_head->cur;
            }
            else if (lexemn->p_head->cur[1] == '<')
            {
                result->type = TOK_LSHIFT;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case '>':
            result->type = TOK_GT;
            if (lexemn->p_head->cur[1] == '=')
            {
                result->type = TOK_GTE;
                ++lexemn->p_head->cur;
            }
            else if (lexemn->p_head->cur[1] == '>')
            {
                result->type = TOK_RSHIFT;
                ++lexemn->p_head->cur;
            }
            ++lexemn->p_head->cur;
            break;
        case '(':
            result->type = TOK_LPAREN;
            ++lexemn->p_head->cur;
            break;
        case ')':
            result->type = TOK_RPAREN;
            ++lexemn->p_head->cur;
            break;
        case '[':
            result->type = TOK_LBRACKET;
            ++lexemn->p_head->cur;
            break;
        case ']':
            result->type = TOK_RBRACKET;
            ++lexemn->p_head->cur;
            break;
        case '{':
            result->type = TOK_LBRACE;
            ++lexemn->p_head->cur;
            break;
        case '}':
            result->type = TOK_RBRACE;
            ++lexemn->p_head->cur;
            break;
        case ',':
            result->type = TOK_COMMA;
            ++lexemn->p_head->cur;
            break;
        case ';':
            result->type = TOK_SEMICOLON;
            ++lexemn->p_head->cur;
            break;
        case '=':
            result->type = TOK_EQ;
            ++lexemn->p_head->cur;
            break;
        case '?':
            result->type = TOK_QMARK;
            ++lexemn->p_head->cur;
            break;
        case '%':
            result->type = TOK_MOD;
            ++lexemn->p_head->cur;
            break;
        case '#':
            result->type = TOK_HASH;
            ++lexemn->p_head->cur;
            break;
        case '@':
            result->type = TOK_ATSIGN;
            ++lexemn->p_head->cur;
            break;
        case '~':
            result->type = TOK_COMPL;
            ++lexemn->p_head->cur;
            break;
        case '^':
            result->type = TOK_XOR;
            ++lexemn->p_head->cur;
            break;
        default:
            result->type = TOK_UNK;
            if (ch > 0x80)
            {
                char32_t ch32;
                size_t const off = peek_utf32(lexemn, 0, &ch32);
                switch (ch32)
                {
                    default: break;
                    case 0x000000F7: result->type = TOK_DIV_2; break;
                    case 0x0000230A: result->type = TOK_LFLOOR; break;
                    case 0x0000230B: result->type = TOK_RFLOOR; break;
                    case 0x00002308: result->type = TOK_LCEILING; break;
                    case 0x00002309: result->type = TOK_RCEILING; break;
                    case 0x00002229: result->type = TOK_SET_INTER; break;  /* Set operators coming below.  */
                    case 0x0000222A: result->type = TOK_SET_UNION; break;
                    case 0x00002286: result->type = TOK_SET_SUB; break;
                    case 0x00002284: result->type = TOK_SET_NSUB; break;
                    case 0x00002282: result->type = TOK_SET_PROPSUB; break;
                    case 0x00002287: result->type = TOK_SET_SUPER; break;
                    case 0x00002285: result->type = TOK_SET_NSUPER; break;
                    case 0x00002283: result->type = TOK_SET_PROPSUPER; break;
                    case 0x00002206: result->type = TOK_SET_SYMMDIFF; break;
                    case 0x00002208: result->type = TOK_SET_ELEMOF; break;
                    case 0x00002209: result->type = TOK_SET_NELEMOF; break;
                    case 0x000000D7: result->type = TOK_SET_CARTPROD; break;
                }

                if (is_identifier(ch32))
                {
                    result->type = TOK_NAME;
                    lex_identifier(lexemn, result);
                    break;
                }

                lexemn->p_head->cur += off;
            }
            else
            {
                emit_stray_token(lexemn, result);
                ++lexemn->p_head->cur;
            }
    }
}

void
lex_start(struct lexemn *const lexemn)
{
    for (size_t file_idx = 0; file_idx < lexemn->p_size; ++file_idx)
    {
        off_t off = 0;
        lexemn->p_head = lexemn->p_vec + file_idx;
        while ((lexemn->lex_stat.is_shell && *lexemn->p_head->cur != '\0')
            || (lexemn->lex_stat.is_fs && off < lexemn->p_head->file.st.st_size))
        {
            struct token token = { 0 };
            lex_next_token(lexemn, &token);
            if (token.type != (enum token_type)-1)
                tstream_push(lexemn, token);
            if (lexemn->lex_stat.is_fs)
                off = lexemn->p_head->cur - lexemn->p_head->buf;
        }
    }

    tstream_push(lexemn, (struct token){ .type = TOK_END });
}

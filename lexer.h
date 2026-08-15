/*
 * lexer.h -- Lexical analyzer declarations.
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

#ifndef TOK_LEXER_H
#define TOK_LEXER_H

#include <stddef.h>

# ifndef TOK_TYPES_TABLE
#  define TOK_TYPES_TABLE               \
                                        \
    /* Mathematics.  */                 \
                                        \
    OP(PLUS,                     "+")   \
    OP(MINUS,                    "-")   \
    OP(MULT,                     "*")   \
    OP(DIV_1,                    "/")   \
    OP(DIV_2,                    "÷")   \
    OP(MOD,                      "%")   \
    OP(INC,                     "++")   \
    OP(DEC,                     "--")   \
    OP(EXP,                     "**")   \
                                        \
    /* Relational.  */                  \
                                        \
    OP(EQ,                       "=")   \
    OP(NEQ_1,                   "!=")   \
    OP(NEQ_2,                   "<>")   \
    OP(LT,                       "<")   \
    OP(GT,                       ">")   \
    OP(LTE,                     "<=")   \
    OP(GTE,                     ">=")   \
                                        \
    /* Logical.  */                     \
                                        \
    OP(AND_AND,                 "&&")   \
    OP(OR_OR,                   "||")   \
    OP(NOT,                      "!")   /* Factorial if postfixed.  */ \
                                        \
    /* Bitwise.  */                     \
                                        \
    OP(AND,                      "&")   \
    OP(OR,                       "|")   \
    OP(XOR,                      "^")   \
    OP(COMPL,                    "~")   \
    OP(LSHIFT,                  "<<")   \
    OP(RSHIFT,                  ">>")   \
                                        \
    /* Grouping.  */                    \
                                        \
    OP(LPAREN,                   "(")   \
    OP(RPAREN,                   ")")   \
    OP(LBRACKET,                 "[")   \
    OP(RBRACKET,                 "]")   \
    OP(LBRACE,                   "{")   \
    OP(RBRACE,                   "}")   \
    OP(LFLOOR,                   "⌊")   \
    OP(RFLOOR,                   "⌋")   \
    OP(LCEILING,                 "⌈")   \
    OP(RCEILING,                 "⌉")   \
                                        \
    /* Sets.  */                        \
                                        \
    OP(SET_INTER,                "∩")   \
    OP(SET_UNION,                "∪")   \
    OP(SET_SUB,                  "⊆")   \
    OP(SET_NSUB,                 "⊄")   \
    OP(SET_PROPSUB,              "⊂")   \
    OP(SET_SUPER,                "⊇")   \
    OP(SET_NSUPER,               "⊅")   \
    OP(SET_PROPSUPER,            "⊃")   \
    OP(SET_SYMMDIFF,             "∆")   \
    OP(SET_ELEMOF,               "∈")   \
    OP(SET_NELEMOF,              "∉")   \
    OP(SET_CARTPROD,             "×")   \
    OP(SET_EMPTY,                "Ø")   \
                                        \
    /* Miscellaneous.  */               \
                                        \
    OP(ASSIGN,                  ":=")   \
    OP(COMMA,                    ",")   \
    OP(COLON,                    ":")   \
    OP(SEMICOLON,                ";")   \
    OP(QMARK,                    "?")   \
    OP(HASH,                     "#")   \
    OP(ATSIGN,                   "@")   \
    OP(RANGE,                   "..")   \
    OP(ELLIPSIS,               "...")   \
                                        \
    TOK(NAME,                  IDENT)   /* word */ \
    TOK(CONST,                 IDENT)   /* $2_SQRTPI */ \
    TOK(NUMBER,              LITERAL)   /* One of (dec|hex|oct|bin).  */ \
    TOK(STRING,              LITERAL)   /* "string"  */ \
    TOK(CMD,                    NONE)   /* Meta-command.  */ \
    TOK(CMD_ARG,                NONE)   /* Meta-command argument.  */ \
    TOK(UNK,                    NONE)   /* Stray token.  */ \
    TOK(END,                    NONE)   /* End of tokens stream.  */
# endif

/* All token types.  */
enum token_type : short unsigned
{
#define OP( name, _ ) TOK_ ## name,
#define TOK( name, _ ) TOK_ ## name,
    TOK_TYPES_TABLE
#undef OP
#undef TOK
    MAX_TOKENS
};

struct identifier  /* FIXME: Convert into symbol table entry.  */
{
    char unsigned const *name;
    size_t               len;
};

/* An individual lexical token scanned from source code.  */
struct token_t
{
    enum token_type type;
    union
    {
        struct identifier *node; /* An identifier in the symbol table.  */
        struct
        {
            char unsigned const *str;
            size_t               len;
        } text; /* A string or a number.  */
    } val;
};

/* Container representing a sequential stream of scanned tokens.  */
struct tstream_t
{
    /* Amount of scanned tokens.  */
    size_t size;

    /* Next size when reallocating memory.  */
    size_t capacity;

    /* Underlying stream scanned tokens.  */
    struct token_t *tokens;
};

/* Lexical analyzer that transforms a raw source string into a sequential stream of
   tokens (see `struct tstream_t').  Operates purely on syntax at character level,
   flags unrecognized symbols or malformed literals, but performs no grammatical
   or semantic validation.   */
struct lexer_t
{
    /* Start of the source buffer.   */
    char unsigned const *buf;

    /* Current read position in `buf'.  */
    char unsigned const *cur;
};

/* Configure LEXER to scan input source WHENCE. */
void
lex_setup(struct lexer_t *lexer,
                char unsigned const *whence);

/* Start scanning LEXER and append all generated tokens to the token
   stream STREAM.  */
void
lex_start(struct lexer_t *lexer,
                struct tstream_t *stream);

#endif //TOK_LEXER_H

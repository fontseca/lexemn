/*
 * internal.h -- In this header are defined all the internal data structures
 * and functions that are the core for the analysers.
 *  ___       _______      ___    ___ _______   _____ ______   ________
 * |\  \     |\  ___ \    |\  \  /  /|\  ___ \ |\   _ \  _   \|\   ___  \
 * \ \  \    \ \   __/|   \ \  \/  / | \   __/|\ \  \\\__\ \  \ \  \\ \  \
 *  \ \  \    \ \  \_|/__  \ \    / / \ \  \_|/_\ \  \\|__| \  \ \  \\ \  \
 *   \ \  \____\ \  \_|\ \  /     \/   \ \  \_|\ \ \  \    \ \  \ \  \\ \  \
 *    \ \_______\ \_______\/  /\   \    \ \_______\ \__\    \ \__\ \__\\ \__\
 *     \|_______|\|_______/__/ /\ __\    \|_______|\|__|     \|__|\|__| \|__|
 *                        |__|/ \|__|
 *
 * The Lexemn Project
 * https://github.com/fontseca/lexemn
 *
 * Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>
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

#ifndef INTERNAL_H
#define INTERNAL_H

#include "utilities.h"

/* For ambiguous symbols like "|" that can mean "the cardinality of..." (if has
   an ending) or "such that", the lexer must provide enough information of the
   current context so that the parser and interpreter will be able to perform
   the expression that was meant. One way to give context to each expression
   is by using a type system. Another approach can be found by turning on the
   field we want to work with, and so if the context was set to be, let's say,
   "Set Theory", we have an expression like this one "a - b", by the context
   we can know that it is not an arithmetic subtraction, but a relative
   complement operation on two sets. */

/* The mathematical symbols that appear in the following table were extracted
   from https://www.rapidtables.com/math/symbols/Basic_Math_Symbols.html.  */

#define TOKEN_TYPES_TABLE            \
                                     \
  /* Mathematical constants. */      \
                                     \
  CONSTANT(PI,                "π")   \
  CONSTANT(E,                 "e")   \
  CONSTANT(PHI,               "𝜑")   \
                                     \
  /* Arithmetic operators. */        \
                                     \
  ARITH_OP(PLUS,              "+")   \
  ARITH_OP(MINUS,             "-")   \
  ARITH_OP(PLUS_MINUS,       "+-")   \
  ARITH_OP(MINUS_PLUS,       "-+")   \
  ARITH_OP(MULT_ASTERISK,     "*")   \
  ARITH_OP(MULT_DOT,          "⋅")   \
  ARITH_OP(DIV_SLASH,         "/")   \
  ARITH_OP(DIV_OBELUS,        "÷")   \
  ARITH_OP(EXPONENTIATION,    "^")   \
  ARITH_OP(INCREMENT,        "++")   \
  ARITH_OP(DECREMENT,        "--")   \
  ARITH_OP(MODULO,          "mod")   \
  ARITH_OP(PERCENT,           "%")   \
  ARITH_OP(PERMILE,           "‰")   \
  ARITH_OP(PERIOD,            ".")   \
  ARITH_OP(FACTORIAL,         "!")   \
                                     \
  /* Algebraic operators. */         \
                                     \
  ALG_OP(DEFINITION,         ":=")   \
  ALG_OP(OPEN_PAREN,          "(")   \
  ALG_OP(CLOSE_PAREN,         ")")   \
  ALG_OP(OPEN_BRACKET,        "[")   \
  ALG_OP(CLOSE_BRACKET,       "]")   \
  ALG_OP(OPEN_BRACE,          "{")   \
  ALG_OP(CLOSE_BRACE,         "}")   \
  ALG_OP(LEFT_FLOOR,          "⌊")   \
  ALG_OP(RIGHT_FLOOR,         "⌋")   \
  ALG_OP(LEFT_CEILING,        "⌈")   \
  ALG_OP(RIGHT_CEILING,       "⌉")   \
  ALG_OP(LEFT_ABSOLUTE,       "|")   \
  ALG_OP(RIGHT_ABSOLUTE,      "|")   \
                                     \
  /* Linear algebra. operators */    \
                                     \
  LIN_ALG_OP(SCALAR_PRODUCT,  "·")   \
  LIN_ALG_OP(VECTOR_PRODUCT,  "×")   \
  LIN_ALG_OP(TENSOR_PDOCUT,   "⊗")   \
                                     \
  /* Set theory operators. */        \
                                     \
  SET_OP(OPEN_SET,            "{")   \
  SET_OP(CLOSE_SET,           "}")   \
  SET_OP(INTERSECTION,        "∩")   \
  SET_OP(UNION,               "∪")   \
  SET_OP(SUBSET,              "⊆")   \
  SET_OP(NOT_SUBSET,          "⊄")   \
  SET_OP(PROPER_SUBSET,       "⊂")   \
  SET_OP(SUPERSET,            "⊇")   \
  SET_OP(NOT_SUPERSET,        "⊅")   \
  SET_OP(PROPER_SUPERSET,     "⊃")   \
  SET_OP(SYMMETRIC_DIFF,      "∆")   \
  SET_OP(ELEMENT_OF,          "∈")   \
  SET_OP(NOT_ELEMENT_OF,      "∉")   \
  SET_OP(CARTESIAN_PRODUCT,   "×")   \
  SET_OP(CARDINALITY_START,   "|")   \
  SET_OP(CARDINALITY_END,     "|")   \
  SET_OP(SUCH_THAT,           "|")   \
  SET_OP(EMPTY_SET,           "Ø")   \
  SET_OP(RELATIVE_COMPLEMENT, "-")   \
  SET_OP(EQUALITY,            "=")   \
                                     \
  /* Bitwise operators. */           \
                                     \
  BITW_OP(AND,                "&")   \
  BITW_OP(OR,                 "|")   \
  BITW_OP(XOR,                "⊕")   \
  BITW_OP(COMPLEMENT,         "~")   \
  BITW_OP(LEFT_SHIFT,        "<<")   \
  BITW_OP(RIGHT_SHIFT,       ">>")   \
                                     \
  /* Logical operators. */           \
                                     \
  LOGIC_OP(AND,              "&&")   \
  LOGIC_OP(OR,               "||")   \
  LOGIC_OP(NOT,               "!")   \
                                     \
  /* Relational operators. */        \
                                     \
  REL_OP(EQUAL,               "=")   \
  REL_OP(LESS_THAN,           "<")   \
  REL_OP(LESS_THAN_EQUAL,    "<=")   \
  REL_OP(GREATER_THAN,        ">")   \
  REL_OP(GREATER_THAN_EQUAL, ">=")   \
  REL_OP(NOT_EQUAL,          "<>")   \
                                     \
  /* Miscellaneous operators. */     \
                                     \
  MISC_OP(COMMA,              ",")   \
  MISC_OP(SEMICOLON,          ";")   \
  MISC_OP(HELP,               "?")

#define CONSTANT(name, symbol) LEXEMN_CONST_ ## name,
#define ARITH_OP(name, symbol) LEXEMN_ARITH_OP_ ## name,
#define ALG_OP(name, symbol) LEXEMN_ALGEBRAIC_OP_ ## name,
#define LIN_ALG_OP(name, symbol) LEXEMN_LIN_ALGEBRAIC_OP_ ## name,
#define SET_OP(name, symbol) LEXEMN_SET_OP_ ## name,
#define BITW_OP(name, symbol) LEXEMN_BITW_OP_ ## name,
#define LOGIC_OP(name, symbol) LEXEMN_LOGIC_OP_ ## name,
#define REL_OP(name, symbol) LEXEMN_REL_OP_ ## name,
#define MISC_OP(name, symbol) LEXEMN_MISC_OP_ ## name,

namespace lexemn::internal
{

  enum lexemn_token_type
  {
    TOKEN_TYPES_TABLE
    LEXEMN_KEYWORD,
    LEXEMN_IDENTIFIER,
    LEXEMN_NUMBER,
  };

#undef CONSTANT
#undef ARITH_OP
#undef ALG_OP
#undef LIN_ALG_OP
#undef SET_OP
#undef BITW_OP
#undef LOGIC_OP
#undef REL_OP
#undef MISC_OP

}


#endif

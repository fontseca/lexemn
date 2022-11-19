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

#include <string.h>

#include <memory>
#include <string>
#include <sstream>
#include <vector>

/* For ambiguous symbols like "|" that can mean "the cardinality of..." (if has
   an ending) or "such that", the lexer must provide enough information of the
   current context so that the parser and interpreter will be able to perform
   the expression that was meant.  One way to give context to each expression
   is by using a type system. Another approach can be found by turning on the
   field we want to work with, and so if the context was set to be, let's say,
   "Set Theory", we have an expression like this one "a - b", by the context
   we can know that it is not an arithmetic subtraction, but a relative
   complement operation on two sets.  */

/* The mathematical symbols that appear in the following table were extracted
   from https://www.rapidtables.com/math/symbols/Basic_Math_Symbols.html.  */

#define TOKEN_TYPES_TABLE            \
                                     \
  /* Mathematical constants.  */     \
                                     \
  CONSTANT(PI,                "π")   \
  CONSTANT(E,                 "e")   \
  CONSTANT(PHI,               "𝜑")   \
                                     \
  /* Arithmetic operators.  */       \
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
  /* Algebraic operators.  */        \
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
  /* Linear algebra operators.  */   \
                                     \
  LIN_ALG_OP(SCALAR_PRODUCT,  "·")   \
  LIN_ALG_OP(VECTOR_PRODUCT,  "×")   \
  LIN_ALG_OP(TENSOR_PDOCUT,   "⊗")   \
                                     \
  /* Set theory operators.  */       \
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
  /* Bitwise operators.  */          \
                                     \
  BITW_OP(AND,                "&")   \
  BITW_OP(OR,                 "|")   \
  BITW_OP(XOR,                "⊕")   \
  BITW_OP(COMPLEMENT,         "~")   \
  BITW_OP(LEFT_SHIFT,        "<<")   \
  BITW_OP(RIGHT_SHIFT,       ">>")   \
                                     \
  /* Logical operators.  */          \
                                     \
  LOGIC_OP(AND,              "&&")   \
  LOGIC_OP(OR,               "||")   \
  LOGIC_OP(NOT,               "!")   \
                                     \
  /* Relational operators.  */       \
                                     \
  REL_OP(EQUAL,               "=")   \
  REL_OP(LESS_THAN,           "<")   \
  REL_OP(LESS_THAN_EQUAL,    "<=")   \
  REL_OP(GREATER_THAN,        ">")   \
  REL_OP(GREATER_THAN_EQUAL, ">=")   \
  REL_OP(NOT_EQUAL,          "<>")   \
                                     \
  /* Miscellaneous operators.  */    \
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

/* The characters stream is the content of a source file.  It is the main source of
   intput that uses the lexer.  */
typedef std::ostringstream characters_stream;

/* Stream of erros that emits either the lexer, parser or interpreter.  */
typedef std::ostringstream errors_stream;

/* Types for backward compatibility with the old lexer version.  */

enum struct token_name
{
  lxmn_identifier,
  lxmn_keywork,
  lxmn_separator,
  lxmn_operator,
  lxmn_number,
  lxmn_string,
  lxmn_assignment,
  lxmn_closing_parenthesis,
  lxmn_opening_parenthesis,
};

typedef std::string token_value_t;
typedef std::pair<token_value_t, token_name> token_t;
typedef std::vector<token_t> tokens_squence_t;

/* Forward declaration for `class book'.  */

class book;

/* The type of any lexemn token.  */
enum class token_type : unsigned char
{
  TOKEN_TYPES_TABLE
  LEXEMN_KEYWORD,
  LEXEMN_IDENTIFIER,
  LEXEMN_INTEGER,
  LEXEMN_DOUBLE,
};

/* Entry node for a hash table.  */
struct hash_table_node
{
  const unsigned char *str;
  unsigned int length;
  unsigned int hash_value;
};

/* Hash table to represent the symbolic table.  */
class hash_table
{
  /* Entries to the hash table.  */
  hash_table_node **entries;

  /* Total slots in the entries array.  */
  unsigned int slots;

  /* Number of living elements.  */
  unsigned int elements;

  /* Current book.  */
  std::unique_ptr<book> pbook;

  /* Usage statistics.  */

  unsigned int searches;
  unsigned int collisions;

  void insert_hnode(hash_table_node);
};

/* Abstracts a valid Lexemn token.  */
struct lexemn_token
{
  /* The type of the token.  */
  token_type type;

  /* The actual token value as an string.  */
  const unsigned char *value;

  /* The position of the token in the token stream.  */
  unsigned int position;
};

/* Symbolizes a set of expressions written in one single page of a book.  It also
   represents a file when Lexemn is interpreting one or more files.  When flag
   `--interactive' is set, then each typed expression is treated as a single
   page and are pushed onto a stack  (this behaviour also aplies when interpreting
   several Lexemn files)  in the `class book' data structure.  */
struct page_buffer
{
  /* Constructs a new page.  */
  page_buffer(const std::shared_ptr<characters_stream>& chstream,
          const std::shared_ptr<page_buffer>& previous_buffer = nullptr) noexcept
    : prev { previous_buffer }
    , buf { chstream }
    , current_char { '\0' }
  {
  }

  /* Previous page.  */
  std::shared_ptr<page_buffer> prev;

  /* The actual content of the page.  */
  std::shared_ptr<characters_stream> buf;

  /* Current grapheme been used.  */
  unsigned char current_char; 
};

/* Abstracts a token stream the lexer feeds to the parser.  */
struct tokens_stream
{
  /* Previous tokens stream.  */
  std::unique_ptr<tokens_stream> prev;

  /* Next tokens stream.  */
  std::unique_ptr<tokens_stream> next;
  
  /* The first token in the stream.  */
  std::unique_ptr<lexemn_token> head_token;

  /* The last token in the stream.  */
  std::unique_ptr<lexemn_token> tail_token;
};

/* Arranges a sequence of page buffers in an stack.  Each page node is either an
   abstraction of logical lxm file or a single line expression when Lexemn is
   running in `--interactive' mode. */
class book
{
public:
  book() noexcept;

  /* The top of the pages buffer stack.  */
  std::shared_ptr<page_buffer> page;

  /* Count of page buffer.  */
  unsigned int pages_count;

  unsigned int push_page_from_stream(characters_stream& chstream) noexcept;

private:
  /* Current token being lexed.  */
  std::unique_ptr<lexemn_token> m_current_token;

  /* Pointer to the last tokens stream.  */
  std::unique_ptr<tokens_stream> m_tokens_stream;

  std::unique_ptr<hash_table> m_hash_table;
  std::string m_date;
  std::string m_time;
};

/* The lexical analyzer takes a raw string as its input and converts it into a
   sequence stream of lexical tokens  (see `struct tokens_stream').  These tokes
   are strings with an assigned and identified value (see `struct lexemn_token').
   The parser uses these tokens to create the parse tree.

   For the character stream "x := 1 * 2;", the lexer generates a structure of tokens
   similar to the following:

   (lexeme,   token name)
   ("x",      <LEXEMN_IDENTIFIER>)
   (":=",     <LEXEMN_ALGEBRAIC_OP_DEFINITION>)
   ("1",      <LEXEMN_INTEGER>)
   ("*",      <LEXEMN_ARITH_OP_MULT_ASTERISK>)
   ("2",      <LEXEMN_INTEGER>)
   (";",      <LEXEMN_MISC_OP_SEMICOLON>).

   The lexer does not know where each token should be located, but it can report
   about unknown tokens  (e.g., an illegal or unrecognized symbol)  and other
   malformed entities. It will not look for tokens out of space, misspelled
   keywords, mismatched types, etc.

   For the sentence "6 := f(x)", the lexer will not generate any error because
   it has no concept of the appropriate arrangement of the tokens. It is the
   parser's job to catch errors.  */
class lexer
{
  /* Determines the way that the stringified token stream is generated.  */
  enum struct strformat : unsigned char
  {
    /* Token stream in one line.  */
    ONELINE,

    /* Token stream in several lines.  One line per token.  */
    MULTILINE
  };

public:
  lexer() = default;

  ~lexer();

  lexer(lexer &&) = delete;

  lexer(std::unique_ptr<internal::book>&& pbook) noexcept;

  void book(std::unique_ptr<internal::book> pbook) noexcept;
  
  const std::unique_ptr<internal::book> &book() noexcept;

  void lex() noexcept;

private:
  tokens_squence_t make_tokens();
  
  const std::unique_ptr<lexemn_token> lex_token() noexcept;

  void lex_number() noexcept;

  void stringify_tokens(std::string &str,
      const tokens_squence_t &tokens,
          const strformat strformat = strformat::MULTILINE) noexcept;

private:
  /* Pointer to a `class book'.  When a new page is pushed onto the current
     `class book', the lexer starts lexing that page.  */
  std::unique_ptr<internal::book> m_book;
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

/*
 * lexer.h -- the lexical analyzer
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

#ifndef LEXER_H
#define LEXER_H

#include <string_view>
#include <unordered_map>

#include "types.h"

/*
 * The lexical analyzer takes a raw string as its input and will convert it
 * into a sequence of lexical tokens. These tokes are strings with an assigned
 * and identified value. The parser will use these tokens to later create the
 * parse tree.
 *
 * For the lexemn string expression
 *  "x:=1*2;",
 * the lexer will have to generate a structure of tokens similar to the
 * following:
 *   (lexeme,   token name)
 *   ("x",      <identifier>)
 *   ("1",      <literal>)
 *   ("*",      <operator>)
 *   ("2",      <literal>)
 *   (";",      <separator>).
 *
 * The lexer does not know where each token should be located, it can report
 * about not valid tokens (e.g., an illegal or unrecognized symbol) and other
 * malformed entities. It will not look for tokens out of space, misspelled
 * keywords, mismatched types, etc.
 *
 * For the sentence '6 := f(x)', the lexer will not generate any error because
 * it has no concept of the appropriate arrangement of the tokens. It is the
 * parser's job to catch this error.
 *
 * The lexer can be a convenient place to carry out some other stuff like
 * stripping out comments and white spaces between tokens.
 */

namespace lexemn::lexer
{

  /*
   * Converts a raw string expression with the Lexemn
   * grammar into a sequence of recognized tokens that
   * can be interpreted by the parser.
   * 
   * If any illegal symbol is detected, then it keeps
   * a buffer of this and any other unknown symbols
   * as an error string.
   */
   void generate_tokens(lexemn::types::tokens_squence_t& lexemes,
                  const std::string_view expression);

  /*
   * Generates a string based on the sequence of tokens
   * generated by the lexer. The string can be formatted
   * in one line (but verbose), which can be used when
   * logging into a file; or it can be formatted in
   * multiple lines using the tokens_string_format enum.
   */
  void stringify_tokens(std::string& str,
                  const lexemn::types::tokens_squence_t& tokens,
                  const utilities::tokens_string_format strformat = utilities::tokens_string_format::k_multiline);

}

#endif
/*
 * lexical-analyzer.cpp --
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

#include <iostream>
#include <string_view>
#include <regex>
#include <unordered_map>
#include <queue>

#include "lexemn/analyzers/lexical-analyzer.h"
#include "lexemn/types.h"
#include "lexemn/utilities.h"

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
 * malformed entities. It will not look for tokens out of space, mispelled
 * keywords, mistmatched types, etc.
 *
 * For the sentence '6 := f(x)', the lexer will not generate any error becasue
 * it has no concept of the appropiate arrangement of the tokens. It is the
 * parser's job to catch this error.
 *
 * The lexer can be a convenient place to carry out some other stuff like
 * stripping out comments and white spaces between tokens.
 */

namespace lexemn::lexical_analyzer
{
  std::vector<lexemn::types::token_t> generate_tokens(const std::string_view expression)
  {
    std::vector<lexemn::types::token_t> lexems { }; /* an array of all the lexems in the expression */
    std::int32_t lexem_count { -1 }; /* the count of lexems */
    std::uint8_t digit_flag { 1 }; /* is activated if the current value is not a digit */
    std::uint8_t identifier_flag { 1 };
    std::size_t i { };

    for (i = 0; i < expression.length(); ++i)
    {
      std::int8_t c = expression[i];

      /* The current character must be a null terminated
      string to seach against a valid regular expression. */

      char currentch[2] { c, '\0' };

      if (std::regex_search(currentch, lexemn::utilities::regex::digit))
      {
        /* If there is a separation, then we are talking
        about another number and we're done with the
        current one. */

        if (digit_flag)
        {
          lexems.push_back(std::make_pair("", lexemn::types::token_name_t::lxmn_number));
          lexem_count++;
          digit_flag = 0;
        }

        /* Create a string of contigous digits that form
        a single numeric value. */

        std::get<0>(lexems[lexem_count]) += currentch;
      }
      else
      {
        /* Activate flag when there is an operand, an space
        or whatever, and you find another digit, it will be
        part of the next set (string) of digits, and not the
        current one. Which means, they are not part of the
        same numeric value.
        
        For "100 + 5", we have two sets of numbers, "100"
        and "5", so there will be two elements for the
        array of numbers. */

        digit_flag = 1; /* you'll need a new entry. */
      }

      /* For identifiers. */

      if (std::regex_search(currentch, lexemn::utilities::regex::identifier))
      {
        if (identifier_flag)
        {
          lexems.push_back(std::make_pair("", lexemn::types::token_name_t::lxmn_identifier));
          lexem_count++;
          identifier_flag = 0;
        }
        std::get<0>(lexems[lexem_count]) += currentch;
      }
      else
      {
        identifier_flag = 1;
      }


      /* When arithmetic operator. */

      if (std::regex_search(currentch, lexemn::utilities::regex::arithmetic_operator))
      {
        lexems.push_back(std::make_pair(currentch, lexemn::types::token_name_t::lxmn_operator));
        lexem_count++;
      }

      /* When assignmen operator. */

      if (c == ':' && expression[i + 1] == '=')
      {
        lexems.push_back(std::make_pair(":=", lexemn::types::token_name_t::lxmn_assignment));
        lexem_count++;
      }

      /* When other operators. */

      switch (c)
      {
        case '(':
          lexems.push_back(std::make_pair(currentch, lexemn::types::token_name_t::lxmn_opening_parenthesis));
          lexem_count++;
          break;
        
        case ')':
          lexems.push_back(std::make_pair(currentch, lexemn::types::token_name_t::lxmn_closing_parenthesis));
          lexem_count++;
          break;
      }

    }

    return lexems;
  }

}
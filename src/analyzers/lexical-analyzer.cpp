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


std::regex digit("[0-9]");

namespace lexemn::lexical_analyzer
{
  bool is_valid_expression(const std::string_view expression)
  {
    return true;
  }

  std::unordered_map<types::token_name_t, types::token_value_t>
  tokenize(const std::string_view expression)
  {
    std::vector<std::string> numbers { }; /* an array of all the numbers in the expression */
    std::uint32_t number_count { 0 }; /* the count of the number of digits in the expression */
    std::uint8_t flag { 0 }; /* is activated if the current value is not a digit */

    for (const char& c : expression)
    {
      /* The current character must be a null terminated
      string to seach against a valid regular expression. */

      char currentch[2] { c, '\0' };

      if (std::regex_search(currentch, digit))
      {
        /* Avoid segfault when trying to access the first
        element by the first time (array is empty!) */

        if (numbers.empty())
        {
          numbers.push_back({ });
        }

        /* If there is a separation, then we are talking
        about another number and we're done with the
        current one. */

        if (flag)
        {
          numbers.push_back({ });
          number_count++;
          flag = 0;
        }

        /* Create a string of contigous digits that form
        a single numeric value. */

        numbers[number_count] += currentch;
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

        flag = 1; /* you'll need a new entry. */
      }

    }

    for (const std::string& str : numbers)
    {
      std::cout << "number: '" << str << "'" << '\n';
    }

    if (!is_valid_expression(expression))
      throw std::runtime_error("error: invalid token");

    return {};
  }

}
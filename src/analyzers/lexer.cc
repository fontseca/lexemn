/*
 * lexer.cc --
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
#include <iomanip>
#include <list>

#include "lexemn/analyzers/lexer.h"
#include "lexemn/utilities.h"
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

namespace lexemn::lexer
{

   void generate_tokens(lexemn::types::tokens_squence_t& lexemes,
                  const std::string_view expression)
  {
    using namespace lexemn::utilities;
    using namespace lexemn::types;
    error_stream_t lexical_errors { };
    std::int32_t nlexemes { -1 };
    std::uint8_t make_new_numeric_entry { true };
    std::uint8_t make_new_identifier_entry { true };
    std::uint8_t error_flag { 0 }; /* no error by default */
    std::size_t i { };

    for (i = 0; i < expression.length(); ++i)
    {
      std::int8_t c = expression[i];

      if (std::isblank(c))
        continue;

      /* The current character must be a null terminated
      string to seach against a valid regular expression. */

      char currentch[2] { c, '\0' };

      if (std::regex_search(currentch, regex::digit))
      {
        /* If there is a separation, then we are talking
        about another number and we're done with the
        current one. */

        if (make_new_numeric_entry)
        {
          lexemes.push_back(std::make_pair("", tokens::token_name::lxmn_number));
          nlexemes++;
          make_new_numeric_entry = false;
        }

        /* Create a string of contigous digits that form
        a single numeric value. */

        std::get<0>(lexemes[nlexemes]) += currentch;
        continue;
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

        make_new_numeric_entry = true; /* you'll need a new entry. */
      }

      /* For identifiers. */

      if (std::regex_search(currentch, regex::identifier))
      {
        if (make_new_identifier_entry)
        {
          lexemes.push_back(std::make_pair("", tokens::token_name::lxmn_identifier));
          nlexemes++;
          make_new_identifier_entry = false;
        }
        std::get<0>(lexemes[nlexemes]) += currentch;
        continue;
      }
      else
      {
        make_new_identifier_entry = true;
      }

      if (c == '+' && expression[i + 1] == '-')
      {
        lexemes.push_back(std::make_pair("+-", tokens::token_name::lxmn_operator));
        
        /* Scale i by the length of ':=' */
        
        i += 1;
        ++nlexemes;

        continue;
      }

      if (c == '-' && expression[i + 1] == '+')
      {
        lexemes.push_back(std::make_pair("-+", tokens::token_name::lxmn_operator));

        /* Scale i by the length of ':=' */
        
        i += 1;
        ++nlexemes;

        continue;
      }

      if (std::regex_search(currentch, regex::arithmetic_operator))
      {
        lexemes.push_back(std::make_pair(currentch, tokens::token_name::lxmn_operator));
        ++nlexemes;
        continue;
      }

      if (c == ':' && expression[i + 1] == '=')
      {
        lexemes.push_back(std::make_pair(":=", tokens::token_name::lxmn_assignment));
        
        /* Scale i by the length of ':=' */
        
        i += 1;
        ++nlexemes;
        
        continue;
      }

      /* When other operators. */

      switch (c)
      {
        case '(':
          lexemes.push_back(std::make_pair(currentch, tokens::token_name::lxmn_opening_parenthesis));
          ++nlexemes;
          break;
        
        case ')':
          lexemes.push_back(std::make_pair(currentch, tokens::token_name::lxmn_closing_parenthesis));
          ++nlexemes;
          break;
        
        case ';':
          lexemes.push_back(std::make_pair(currentch, tokens::token_name::lxmn_separator));
          ++nlexemes;
          break;

        /* Report unrecognized symbols. */

        default:
        {
          /* Ther's an error. */
          
          error_flag = 1;
          
          std::ostringstream err { };

          err << "lexemn: \x1B[1;31merror:\x1B[0m unknown symbol detected near `" << c << "'" << '\n';

          err << "  "                    /* left padding */
              << expression.substr(0, i) /* left chunk of expression */
              << "\x1B[1;31m"
              << c                        /* colored illegal symbol */
              << "\x1B[0m"
              << expression.substr(i + 1) /* right chunk of expression */
              << '\n'
              << std::setw(2 + i + 1)     /* calculate width for `^' */
              << "^"
              << '\n';

          lexical_errors << err.str();
        }
      }
    }

    if (error_flag)
      throw std::runtime_error(lexical_errors.str());
  }

  void stringify_tokens(std::string& str,
                  const lexemn::types::tokens_squence_t& tokens,
                  const types::tokens_string_format strformat)
  {
      std::ostringstream os { };
      bool multiline { strformat == types::tokens_string_format::k_multiline };
      std::string end { multiline ? "\n" : " " };
      std::string indent { multiline ? "  " : "" };
      std::size_t ntokens { tokens.size() };
      std::size_t i { 1 };

      os << "[" << end;
      
      for (const auto& token : tokens)
      {
        using namespace lexemn::utilities::tokens;
        os << indent << "{ `" << token.first << "', ";
        std::string laststr { i == ntokens ? " }" : " }," };

        switch(token.second)
        {
          case token_name::lxmn_number:
            os << "numeric value" << laststr << end;
            break;

          case token_name::lxmn_identifier:
            os << "identifier" << laststr << end;
            break;

          case token_name::lxmn_operator:
            os << "arithmetic operator" << laststr << end;
            break;

          case token_name::lxmn_assignment:
            os << "assignment operator" << laststr << end;
            break;

          case token_name::lxmn_opening_parenthesis:
            os << "opening parenthesis" << laststr << end;
            break;

          case token_name::lxmn_closing_parenthesis:
            os << "closing parenthesis" << laststr << end;
            break;

          case token_name::lxmn_separator:
            os << "separator" << laststr << end;
            break;

          case token_name::lxmn_keywork:
            os << "keyword" << laststr << end;
            break;

          case token_name::lxmn_string:
            os << "string" << laststr << end;
            break;
        }
        ++i;
      }
      os << "]";
      str = std::move(os.str());
  }

}
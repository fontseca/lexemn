/*
 * lexer.cc -- lexer implementation for lexemn
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
#include <iomanip>

#include "lexemn/utility.h"
#include "lexemn/internal.h"

namespace lexemn::internal
{

lexer::lexer(std::unique_ptr<internal::book>&& pbook) noexcept
  : m_book { std::move(pbook) }
{
}

lexer::~lexer() noexcept
{
  this->m_book.~unique_ptr();
}

void lexer::book(std::unique_ptr<internal::book> pbook) noexcept
{
  this->m_book = std::move(pbook);
}

const std::unique_ptr<internal::book> &lexer::book() noexcept
{
  return this->m_book;
}

const std::unique_ptr<internal::lexemn_token> lexer::lex_token() noexcept
{
  return { };
}

/* Lexes a number to  `token_type::LEXEMN_INTEGER'.  */
void lexer::lex_number() noexcept
{
}

 /* Lexes the page at the top of the stack.  */
void lexer::lex() noexcept
{
  std::string str;
  this->stringify_tokens(str, this->make_tokens());
  std::cout << str << '\n';
}

/* Converts a raw string expression with the Lexemn grammar into a sequence of
   recognized tokens that can be interpreted by the parser.  If any illegal symbol
   is detected, then it keeps a buffer of this and any other unknown symbols as an
   error string.  */
tokens_squence_t lexer::make_tokens()
{
  using namespace lexemn::utility;
  tokens_squence_t lexemes;
  errors_stream lexical_errors { };
  std::int32_t nlexemes { -1 };
  std::uint8_t make_new_numeric_entry { true };
  std::uint8_t make_new_identifier_entry { true };
  std::uint8_t error_flag { 0 }; /* no error by default */
  std::size_t i { };

  auto page = std::move(this->m_book->page);
  unsigned int buflen = page->buf->str().size();

  for (i = 0; i < buflen; ++i)
  {
    std::int8_t c = page->buf->str()[i];

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
        lexemes.push_back(std::make_pair("", token_name::lxmn_number));
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
        lexemes.push_back(std::make_pair("", token_name::lxmn_identifier));
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

    if (c == '+' && page->buf->str()[i + 1] == '-')
    {
      lexemes.push_back(std::make_pair("+-", token_name::lxmn_operator));
      
      /* Scale i by the length of ':=' */
      
      i += 1;
      ++nlexemes;

      continue;
    }

    if (c == '-' && page->buf->str()[i + 1] == '+')
    {
      lexemes.push_back(std::make_pair("-+", token_name::lxmn_operator));

      /* Scale i by the length of ':=' */
      
      i += 1;
      ++nlexemes;

      continue;
    }

    if (std::regex_search(currentch, regex::arithmetic_operator))
    {
      lexemes.push_back(std::make_pair(currentch, token_name::lxmn_operator));
      ++nlexemes;
      continue;
    }

    if (c == ':' && page->buf->str()[i + 1] == '=')
    {
      lexemes.push_back(std::make_pair(":=", token_name::lxmn_assignment));
      
      /* Scale i by the length of ':=' */
      
      i += 1;
      ++nlexemes;
      
      continue;
    }

    /* When other operators. */

    switch (c)
    {
      case '(':
        lexemes.push_back(std::make_pair(currentch, token_name::lxmn_opening_parenthesis));
        ++nlexemes;
        break;
      
      case ')':
        lexemes.push_back(std::make_pair(currentch, token_name::lxmn_closing_parenthesis));
        ++nlexemes;
        break;
      
      case ';':
        lexemes.push_back(std::make_pair(currentch, token_name::lxmn_separator));
        ++nlexemes;
        break;

      /* Report unrecognized symbols. */

      default:
      {
        /* Ther's an error. */
        
        error_flag = 1;
        
        std::ostringstream err { };

        err << "lexemn: \x1B[1;31merror:\x1B[0m unknown symbol detected near `" << c << "'" << '\n';

        // err << "  "                    /* left padding */
        //     << page->Buffer.substr(0, i) /* left chunk of expression */
        //     << "\x1B[1;31m"
        //     << c                        /* colored illegal symbol */
        //     << "\x1B[0m"
        //     << expression.substr(i + 1) /* right chunk of expression */
        //     << '\n'
        //     << std::setw(2 + i + 1)     /* calculate width for `^' */
        //     << "^"
        //     << '\n';

        lexical_errors << err.str();
      }
    }
  }

  if (error_flag)
    throw std::runtime_error(lexical_errors.str());
  return lexemes;
}


/* Generates a string based on the sequence of tokens generated by the lexer.  The
   string can be formatted in one line (but verbose), which can be used when
   logging into a file; or it can be formatted in multiple lines using the
   `strformat' enum.  */
void lexer::stringify_tokens(std::string &str,
                  const tokens_squence_t &tokens,
                  const strformat strformat) noexcept
{
    std::ostringstream os { };
    bool multiline { strformat == strformat::MULTILINE };
    std::string end { multiline ? "\n" : " " };
    std::string indent { multiline ? "  " : "" };
    std::size_t ntokens { tokens.size() };
    std::size_t i { 1 };

    os << "[" << end;
    
    for (const auto& token : tokens)
    {
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

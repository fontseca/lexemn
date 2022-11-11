/*
 * lexemn.cpp -- entry point for lexemn
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
#include <memory>
#include <readline/readline.h>
#include <readline/history.h>
#include <regex>
#include <getopt.h>
#include <cstring>

#include "lexemn/lexemn.h"
#include "lexemn/types.h"
#include "lexemn/analyzers/lexical-analyzer.h"

int32_t main(int32_t argc, char **argv)
{
  int8_t c;
  lexemn::types::running_mode_t x { false, false, false };

  while (c = getopt_long(argc, argv, "cqvd", lexemn::long_options, NULL), c ^ -1)
  {
  
    if (c == '?')
    {
      std::exit(EXIT_FAILURE);
    }

    switch (c)
    {
    case 'c':
      x.color = true;
      break;
    case 'd':
      x.debug = true;
      break;
    case 'q':
      x.quiet = true;
      break;
    case 'v':
      std::printf("LEXEMN v%s -- Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n", LXMN_VERSION);
      std::exit(EXIT_SUCCESS);
    }
  }

  /* Run in quiet mode. */

  if (!x.quiet)
  {
    lexemn::welcome(x);
  }

  const std::regex blank_regex("^[[:space:]]*$", std::regex_constants::grep);
  const char* const lexemn_prompt = x.color ? "\x1B[92m~>\x1B[0m " : "~> ";

  /* Read expressions and tokenize repeatedly. */

  while (1)
  {

    std::unique_ptr<char[], decltype(&free)> line(readline(lexemn_prompt), free);

    if (std::regex_search(line.get(), blank_regex))
      continue;

    /* FIXME: Consider using a procedure for correct
    quitting (also for ^D and ^C.) */

    if (!strcmp(line.get(), "quit()") || !strcmp(line.get(), "q()"))
    {
      line.reset(nullptr);
      std::exit(EXIT_SUCCESS);
    }

    add_history(line.get());

    try
    {
      auto tokens = lexemn::lexical_analyzer::generate_tokens(line.get());
      std::ostringstream os;
      
      os << "[" << '\n';

      for (const auto& token : tokens)
      {
        using namespace lexemn::types;

        if (token_name_t::lxmn_number == token.second)
        {
          os << "  ('" << token.first << "', numeric value)," << '\n';
        }

        if (token_name_t::lxmn_identifier == token.second)
        {
          os << "  ('" << token.first << "', identifier)," << '\n';
        }

        if (token_name_t::lxmn_operator == token.second)
        {
          os << "  ('" << token.first << "', arithmetic operator)," << '\n';
        }

        if (token_name_t::lxmn_assignment == token.second)
        {
          os << "  ('" << token.first << "', assignment operator)," << '\n';
        }

        if (token_name_t::lxmn_opening_parenthesis == token.second)
        {
          os << "  ('" << token.first << "', opening parenthesis)," << '\n';
        }

        if (token_name_t::lxmn_closing_parenthesis == token.second)
        {
          os << "  ('" << token.first << "', closing parenthesis)," << '\n';
        }

        if (token_name_t::lxmn_separator == token.second)
        {
          os << "  ('" << token.first << "', separator)," << '\n';
        }
      }

      os << "]";
      std::cout << os.str() << '\n';
    }
    catch (const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
  }

  return EXIT_SUCCESS;
}

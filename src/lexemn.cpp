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

using namespace lexemn;
using namespace lexemn::lexical_analyzer;

int32_t main(int32_t argc, char **argv)
{
  int8_t c;
  types::running_mode_t x;

  while (c = getopt_long(argc, argv, "cqvd", long_options, NULL), c ^ -1)
  {
    if (c == '?')
      exit(EXIT_FAILURE);

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
      printf("LEXEMN v%s -- Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n", LXMN_VERSION);
      exit(EXIT_SUCCESS);
      break;
    }
  }

  if (!x.quiet)
    welcome(x);

  const std::regex blank_regex("^[[:space:]]*$", std::regex_constants::grep);

  const char *lexemn_prompt;

  if (x.color)
    lexemn_prompt = "\x1B[92m~>\x1B[0m ";
  else
    lexemn_prompt = "~> ";

  while (1)
  {
    std::unique_ptr<char[], decltype(&free)> raw_expression(readline(lexemn_prompt), free);
    if (!std::regex_search(raw_expression.get(), blank_regex))
    {
      /* FIXME: Consider using a procedure for correct quitting (also for ^D and ^C) */
      if (!strcmp(raw_expression.get(), "quit()"))
      {
        raw_expression.~unique_ptr();
        exit(EXIT_SUCCESS);
      }

      add_history(raw_expression.get());
      try
      {
        tokenize(raw_expression.get());
      }
      catch (const std::exception &e)
      {
        std::cerr << e.what() << '\n';
      }
    }
  }

  return EXIT_SUCCESS;
}

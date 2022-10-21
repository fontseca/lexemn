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

#include "lexemn.h"
#include "analyzers/lexical-analyzer.h"

using namespace lexemn;
using namespace lexemn::lexical_analyzer;

int32_t main(int32_t argc, char **argv)
{
  welcome();

  const std::regex blank_regex("^[[:space:]]*$", std::regex_constants::grep);

  while (1)
  {
    std::unique_ptr<char[], decltype(&free)> raw_expression(readline("\x1B[92m(lexemn)\x1B[0m "), free);
    if (!std::regex_search(raw_expression.get(), blank_regex))
    {
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

/*
 * utilities.h -- utility objects for lexemn
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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <sstream>
#include <getopt.h>
#include <regex>

#include "lexemn/types.h"

/* Specify the current LEXEMN's version. */

#define LXMN_VERSION "0.0.1"

namespace lexemn::utilities
{

  namespace regex
  {
    extern std::regex digit;
    extern std::regex arithmetic_operator;
    extern std::regex identifier;
  }

  struct running_mode
  {
    unsigned char quiet : 1;
    unsigned char debug : 1;
    unsigned char color : 1;
  };

  extern running_mode x;

  namespace tokens
  {
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
  }

  static struct option const long_options[] = {
      {"quiet", no_argument, NULL, 'q'},
      {"color", no_argument, NULL, 'c'},
      {"debug", no_argument, NULL, 'd'},
      {"version", no_argument, NULL, 'v'},
      {NULL, 0, NULL, 0},
  };

  void welcome(running_mode x)
  {
    std::stringstream ss;

    if (x.color)
    {
      ss << "\x1B[96m ___       _______      \x1B[91m___    ___\x1B[96m _______   _____ ______   ________" << std::endl;
      ss << "|\\  \\     |\\  ___ \\    \x1B[91m|\\  \\  /  /|\x1B[96m\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\" << std::endl;
      ss << "\\ \\  \\    \\ \\   __/|   \x1B[91m\\ \\  \\/  / |\x1B[96m \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      ss << " \\ \\  \\    \\ \\  \\_|/__  \x1B[91m\\ \\    / /\x1B[96m \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\" << std::endl;
      ss << "  \\ \\  \\____\\ \\  \\_|\\ \\  \x1B[91m/     \\/\x1B[96m   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      ss << "   \\ \\_______\\ \\_______\\\x1B[91m/  /\\   \\\x1B[96m    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\" << std::endl;
      ss << "    \\|_______|\\|_______\x1B[91m/__/ /\\ __\\\x1B[96m    \\|_______|\\|__|     \\|__|\\|__| \\|__|" << std::endl;
      ss << "                       \x1B[91m|__|/ \\|__|\x1B[97m" << std::endl;
    }
    else
    {
      ss << " ___       _______      ___    ___ _______   _____ ______   ________" << std::endl;
      ss << "|\\  \\     |\\  ___ \\    |\\  \\  /  /|\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\" << std::endl;
      ss << "\\ \\  \\    \\ \\   __/|   \\ \\  \\/  / | \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      ss << " \\ \\  \\    \\ \\  \\_|/__  \\ \\    / / \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\" << std::endl;
      ss << "  \\ \\  \\____\\ \\  \\_|\\ \\  /     \\/   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      ss << "   \\ \\_______\\ \\_______\\/  /\\   \\    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\" << std::endl;
      ss << "    \\|_______|\\|_______/__/ /\\ __\\    \\|_______|\\|__|     \\|__|\\|__| \\|__|" << std::endl;
      ss << "                       |__|/ \\|__|" << std::endl;
    }

    printf("%s\n", ss.str().c_str());
    printf("An open source mathematical tool and library for interpreting algebraic\n");
    printf("expressions, plotting functions and solving equations.\033[0m\n\n");
    printf("To contribute to the LEXEMN project go to https://github.com/fontseca/lexemn\n");
    printf("Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n");
    printf("Invoke `quit()' to exit or `help()' for more information.\n\n");
  }
}
#endif /* UTILITIES_H */

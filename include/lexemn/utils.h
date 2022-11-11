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

#include <regex>

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

  extern running_mode x;

}

#endif /* UTILITIES_H */

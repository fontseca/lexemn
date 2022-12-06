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

#include <cstdio>
#include <cuchar>

#include <iostream>

#include "lexemn/charset.h"

namespace lexemn::charset
{

bool unicode_valid_in_identifier(std::uint32_t ch) noexcept
{

  if (ch >= 0x0041 && ch <= 0x005A)
  {
    return true;
  }
  else if (ch >= 0x0061 && ch <= 0x007A)
  {
    return true;
  }
  else if (ch == 0x1f34c)
  {
    return true;
  }
  else if (ch == 0x00D1)
  {
    return true;
  }
  else if (ch == 0x0393)
  {
    return true;
  }
  else if (ch == 0x03b6)
  {
    return true;
  }
  else if (ch == 0x03BE)
  {
    return true;
  }

  return false;
}

bool unicode_valid_in_number(std::uint32_t ch) noexcept
{
  if (ch >= 0x0030 && ch <= 0x0039)
  {
    return true;
  }
  return false;
}

}
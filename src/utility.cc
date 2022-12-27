/*
 * utility.cc --
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

#include <cstring>

#include "lexemn/utility.h"
#include "lexemn/error.h"

namespace lexemn::utility
{

auto duplicate_stream(internal::characters_stream src, std::size_t n) noexcept
  -> decltype(src)
{
  internal::characters_stream dest = new char[n];
  return dest == nullptr ? nullptr : static_cast<internal::characters_stream>(std::memcpy(dest, src, n));
}

auto make_error_message(std::string &error_message, const error::error_type error_type) noexcept
  -> void
{
  char buffer[0x400];
  std::sprintf(buffer, "lexemn: \x1B[1;31merror:\x1B[0m %s\n", error::errors_dictionary[error_type]);
  error_message = std::move(buffer);
}

auto make_error_message(std::string &error_message, const char *const custom_message) noexcept
  -> void
{
  char buffer[0x400];
  std::sprintf(buffer, "lexemn: \x1B[1;31merror:\x1B[0m %s\n", custom_message);
  error_message = std::move(buffer);
}

}
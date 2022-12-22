/*
 * charset.cc -- character set handling
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

#include "lexemn/charset.h"

namespace lexemn::charset
{

[[nodiscard]] static inline constexpr auto is_in_range(const std::uint32_t x,
  const std::uint32_t lower, const std::uint32_t upper) noexcept
  -> bool
{
  return x >= lower and x <= upper;
}

[[nodiscard]] auto unicode_valid_in_identifier(const std::uint32_t ch) noexcept
  -> bool
{
  /* These are the valid extended characters found in
     ISO/IEC 9899:201x Annex D.  */

  switch (ch)
  {
    case 0x00A8:
    case 0x00AA:
    case 0x00AD:
    case 0x00AF:
    case 0x2054:
      return true;
  }

  return (
    (is_in_range(ch, 0x0041, 0x005A) or is_in_range(ch, 0x0061, 0x0102))

    or

    (is_in_range(ch, 0x00B2, 0x00B5) or is_in_range(ch, 0x00B7, 0x00BA)
    or is_in_range(ch, 0x00BC, 0x00BE) or is_in_range(ch, 0x00C0, 0x00D6))

    or

    (is_in_range(ch, 0x00D8, 0x00F6) or is_in_range(ch, 0x00F8, 0x00FF))

    or

    (is_in_range(ch, 0x0100, 0x167F) or is_in_range(ch, 0x1681, 0x180D)
    or is_in_range(ch, 0x180F, 0x1FFF))

    or

    (is_in_range(ch, 0x200B, 0x200D) or is_in_range(ch, 0x202A, 0x202E)
    or is_in_range(ch, 0x203F, 0x2040) or is_in_range(ch, 0x2060, 0x206F))

    or

    (is_in_range(ch, 0x2070, 0x218F) or is_in_range(ch, 0x2460, 0x24FF)
    or is_in_range(ch, 0x2776, 0x2793) or is_in_range(ch, 0x2C00, 0x2DFF)
    or is_in_range(ch, 0x2E80, 0x2FFF))

    or

    (is_in_range(ch, 0x3004, 0x3007) or is_in_range(ch, 0x3021, 0x302F)
    or is_in_range(ch, 0x3031, 0x303F))

    or
    
    (is_in_range(ch, 0x3040, 0xD7FF))

    or

    (is_in_range(ch, 0xF900, 0xFD3D) or is_in_range(ch, 0xFD40, 0xFDCF)
    or is_in_range(ch, 0xFDF0, 0xFE44) or is_in_range(ch, 0xFE47, 0xFFFD))

    or

    (is_in_range(ch, 0x10000, 0x1FFFD) or is_in_range(ch, 0x20000, 0x2FFFD)
    or is_in_range(ch, 0x30000, 0x3FFFD) or is_in_range(ch, 0x40000, 0x4FFFD)
    or is_in_range(ch, 0x50000, 0x5FFFD) or is_in_range(ch, 0x60000, 0x6FFFD)
    or is_in_range(ch, 0x70000, 0x7FFFD) or is_in_range(ch, 0x80000, 0x8FFFD)
    or is_in_range(ch, 0x90000, 0x9FFFD) or is_in_range(ch, 0xA0000, 0xAFFFD)
    or is_in_range(ch, 0xB0000, 0xBFFFD) or is_in_range(ch, 0xC0000, 0xCFFFD)
    or is_in_range(ch, 0xD0000, 0xDFFFD) or is_in_range(ch, 0xE0000, 0xEFFFD))
  );
}

[[nodiscard]] auto unicode_valid_in_number(const std::uint32_t ch) noexcept
  -> bool
{
  return is_in_range(ch, 0x0030, 0x0039);
}

}
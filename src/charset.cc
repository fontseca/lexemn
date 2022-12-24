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

#ifdef _WIN32
/* To see the full implementation for mk_wcwidth() and related functions,
   visit this site `https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c'.  */

struct interval final
{
  std::int32_t lower;
  std::int32_t upper;
};

/* Auxiliary function for binary search in interval table.  */
static auto binary_search(const wchar_t ucs, const interval *table, std::int32_t max) noexcept
  -> std::int32_t
{
  std::int32_t min = 0;
  std::int32_t mid;

  if (ucs < table[0].lower || ucs > table[max].upper)
  {
    return 0;
  }

  while (max >= min)
  {
    mid = (min + max) / 2;

    if (ucs > table[mid].upper)
    {
      min = mid + 1;
    }
    else if (ucs < table[mid].lower)
    {
      max = mid - 1;
    }
    else
    {
      return 1;
    }
  }

  return 0;
}

static auto mk_wcwidth(const wchar_t ucs) noexcept
  -> std::int32_t
{
  static const interval combining[] =
  {
    { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
    { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
    { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
    { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
    { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
    { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
    { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
    { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
    { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
    { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
    { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
    { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
    { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
    { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
    { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
    { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
    { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
    { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
    { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
    { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
    { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
    { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
    { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
    { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
    { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
    { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
    { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
    { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
    { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
    { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
    { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
    { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
    { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
    { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
    { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
    { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
    { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
    { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
    { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
    { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
    { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
    { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
    { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
    { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
    { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
    { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
    { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
    { 0xE0100, 0xE01EF }
  };

  /* Test for 8-bit control characters.  */

  if (ucs == 0)
  {
    return 0;
  }

  if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
  {
    return -1;
  }

  /* Binary search in table of non-spacing characters.  */

  if (binary_search(ucs, combining, sizeof(combining) / sizeof(interval) - 1))
  {
    return 0;
  }

  /* If we arrive here, `ucs' is not a combining or C0/C1 control character.  */

  return 1 + 
    (ucs >= 0x1100 &&
     (ucs <= 0x115f ||                    /* Hangul Jamo init. consonants.  */
      ucs == 0x2329 || ucs == 0x232a ||
      (ucs >= 0x2e80 && ucs <= 0xa4cf &&
       ucs != 0x303f) ||                  /* CJK ... Yi.  */
      (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables.  */
      (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs.  */
      (ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms.  */
      (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms.  */
      (ucs >= 0xff00 && ucs <= 0xff60) || /* Fullwidth Forms.  */
      (ucs >= 0xffe0 && ucs <= 0xffe6) ||
      (ucs >= 0x20000 && ucs <= 0x2fffd) ||
      (ucs >= 0x30000 && ucs <= 0x3fffd)));
}
#endif

/* Gets the offset from the beginning of `str' to `str + end'. This function
   takes into account universal characters.  */
[[nodiscard]]
auto offset(const char *str, const std::ptrdiff_t end) noexcept
  -> std::uint32_t
{
  char32_t c32;
  std::mbstate_t state { };
  std::uint32_t offset { 0 };
  const char *it = str;

  while(std::size_t rc = std::mbrtoc32(&c32, it, (str + end) - it, &state))
  {
    if(rc == static_cast<std::size_t>(-1)
    or rc == static_cast<std::size_t>(-2))
    {
      break;
    }

    it += rc;

#if defined(unix) || defined(__unix__) || defined(__unix)
    offset += wcwidth(c32);
#else
    offset += mk_wcwidth(c32);
#endif
  }

  return offset;
}

}
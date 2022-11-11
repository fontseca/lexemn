/*
 * utilities.cc --
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

#include "lexemn/utilities.h"

namespace lexemn::utilities
{
  namespace regex
  {
    std::regex digit("^(([0-9]*)|(([0-9]*)\\.([0-9]*)))$");
    std::regex arithmetic_operator("[\\^+\\-*\\/]");
    std::regex identifier("[a-zA-Z]");
  }

  /* Represents the way that Lexem will be running. */

  running_mode x
  {
    .quiet { 0 },
    .debug { 0 },
    .color { 0 },
  };

  /* Verbose welcoming message. */

  void lexemn_welcome_message(const running_mode& x)
  {
    std::ostringstream os;

    if (x.color)
    {
      os << "\x1B[96m ___       _______      \x1B[91m___    ___\x1B[96m _______   _____ ______   ________" << std::endl;
      os << "|\\  \\     |\\  ___ \\    \x1B[91m|\\  \\  /  /|\x1B[96m\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\" << std::endl;
      os << "\\ \\  \\    \\ \\   __/|   \x1B[91m\\ \\  \\/  / |\x1B[96m \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      os << " \\ \\  \\    \\ \\  \\_|/__  \x1B[91m\\ \\    / /\x1B[96m \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\" << std::endl;
      os << "  \\ \\  \\____\\ \\  \\_|\\ \\  \x1B[91m/     \\/\x1B[96m   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      os << "   \\ \\_______\\ \\_______\\\x1B[91m/  /\\   \\\x1B[96m    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\" << std::endl;
      os << "    \\|_______|\\|_______\x1B[91m/__/ /\\ __\\\x1B[96m    \\|_______|\\|__|     \\|__|\\|__| \\|__|" << std::endl;
      os << "                       \x1B[91m|__|/ \\|__|\x1B[97m" << std::endl;
    }
    else
    {
      os << " ___       _______      ___    ___ _______   _____ ______   ________" << std::endl;
      os << "|\\  \\     |\\  ___ \\    |\\  \\  /  /|\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\" << std::endl;
      os << "\\ \\  \\    \\ \\   __/|   \\ \\  \\/  / | \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      os << " \\ \\  \\    \\ \\  \\_|/__  \\ \\    / / \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\" << std::endl;
      os << "  \\ \\  \\____\\ \\  \\_|\\ \\  /     \\/   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\" << std::endl;
      os << "   \\ \\_______\\ \\_______\\/  /\\   \\    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\" << std::endl;
      os << "    \\|_______|\\|_______/__/ /\\ __\\    \\|_______|\\|__|     \\|__|\\|__| \\|__|" << std::endl;
      os << "                       |__|/ \\|__|" << std::endl;
    }

    std::printf("%s\n"
      "An open source mathematical tool and library for interpreting algebraic\n"
      "expressions, plotting functions and solving equations.\033[0m\n\n"
      "To contribute to the Lexemn project go to https://github.com/fontseca/lexemn\n"
      "Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n\n"
      "Invoke `help()' for further information; and `quit()' or `q()' to exit.\n\n",
      os.str().c_str());
  }
}
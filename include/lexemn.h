/*
 * lexemn.h --
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

#ifndef LEXEMN_H
#define LEXEMN_H

#include <iostream>
#include <sstream>

namespace lexemn
{
  void welcome()
  {
    std::stringstream ss;

    ss << "\x1B[97mWelcome to" << std::endl;
    ss << "\x1B[96m ___       _______      \x1B[91m___    ___\x1B[96m _______   _____ ______   ________" << std::endl;
    ss << "|\\  \\     |\\  ___ \\    \x1B[91m|\\  \\  /  /|\x1B[96m\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\" << std::endl;
    ss << "\\ \\  \\    \\ \\   __/|   \x1B[91m\\ \\  \\/  / |\x1B[96m \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\" << std::endl;
    ss << " \\ \\  \\    \\ \\  \\_|/__  \x1B[91m\\ \\    / /\x1B[96m \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\" << std::endl;
    ss << "  \\ \\  \\____\\ \\  \\_|\\ \\  \x1B[91m/     \\/\x1B[96m   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\" << std::endl;
    ss << "   \\ \\_______\\ \\_______\\\x1B[91m/  /\\   \\\x1B[96m    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\" << std::endl;
    ss << "    \\|_______|\\|_______\x1B[91m/__/ /\\ __\\\x1B[96m    \\|_______|\\|__|     \\|__|\\|__| \\|__|" << std::endl;
    ss << "                       \x1B[91m|__|/ \\|__|\x1B[97m." << std::endl;

    printf("%s\n", ss.str().c_str());
    printf("An open source mathematical tool and library for interpreting algebraic\n");
    printf("expressions, plotting functions and solving equations.\033[0m\n\n");
    printf("To contribute to the LEXEMN project go to https://github.com/fontseca/lexemn\n");
    printf("Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n");
    printf("Invoke `quit()' to exit or `help()' for more information.\n\n");
  }
}
#endif

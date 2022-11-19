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

#include <readline/readline.h>
#include <readline/history.h>
#include <getopt.h>
#include <regex>

#include "lexemn/internal.h"

#define VERSION  "v0.1"               /* Latest Lexemn version.  */
#define PROGNAME "lexemn"

#define C_BEGIN "\x1B[92m"            /* Green color.  */
#define C_END   "\x1B[0m "            /* Default color.  */

#define C_PROMPT(P) C_BEGIN P C_END   /* Colored prompt.  */
#define UNC_PROMPT(P) P " "           /* Uncolored prompt.  */

/* Abstracts the way Lexemn runs.  */
struct running_mode
{
  unsigned char quiet : 1;
  unsigned char debug : 1;
  unsigned char color : 1;
} x { .quiet { false }, .debug { false }, .color { false } };

#define PROMPT(P) x.color  \
  ? C_PROMPT(#P)     \
  : UNC_PROMPT(#P)

static struct option const long_options[] =
{
    {"quiet", no_argument, NULL, 'q'},
    {"color", no_argument, NULL, 'c'},
    {"debug", no_argument, NULL, 'd'},
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 0},
    {NULL, 0, NULL, 0},
};

/* Displays verbose welcoming.  */
void emit_welcome()
{
  if (x.color)
  {
    std::fputs("\
\x1B[96m ___       _______      \x1B[91m___    ___\x1B[96m _______   _____ ______   ________\n\
|\\  \\     |\\  ___ \\    \x1B[91m|\\  \\  /  /|\x1B[96m\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\\n\
\\ \\  \\    \\ \\   __/|   \x1B[91m\\ \\  \\/  / |\x1B[96m \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\\n\
 \\ \\  \\    \\ \\  \\_|/__  \x1B[91m\\ \\    / /\x1B[96m \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\\n\
  \\ \\  \\____\\ \\  \\_|\\ \\  \x1B[91m/     \\/\x1B[96m   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\\n\
   \\ \\_______\\ \\_______\\\x1B[91m/  /\\   \\\x1B[96m    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\\n\
    \\|_______|\\|_______\x1B[91m/__/ /\\ __\\\x1B[96m    \\|_______|\\|__|     \\|__|\\|__| \\|__|\n\
                       \x1B[91m|__|/ \\|__|\x1B[97m\n\
\n", stdout);
  }
  else
  {
    std::fputs("\
 ___       _______      ___    ___ _______   _____ ______   ________\n\
|\\  \\     |\\  ___ \\    |\\  \\  /  /|\\  ___ \\ |\\   _ \\  _   \\|\\   ___  \\\n\
\\ \\  \\    \\ \\   __/|   \\ \\  \\/  / | \\   __/|\\ \\  \\\\\\__\\ \\  \\ \\  \\\\ \\  \\\n\
 \\ \\  \\    \\ \\  \\_|/__  \\ \\    / / \\ \\  \\_|/_\\ \\  \\\\|__| \\  \\ \\  \\\\ \\  \\\n\
  \\ \\  \\____\\ \\  \\_|\\ \\  /     \\/   \\ \\  \\_|\\ \\ \\  \\    \\ \\  \\ \\  \\\\ \\  \\\n\
   \\ \\_______\\ \\_______\\/  /\\   \\    \\ \\_______\\ \\__\\    \\ \\__\\ \\__\\\\ \\__\\\n\
    \\|_______|\\|_______/__/ /\\ __\\    \\|_______|\\|__|     \\|__|\\|__| \\|__|\n\
                       |__|/ \\|__|\n\
\n", stdout);
  }

  std::fputs("\
An open source mathematical tool and library for interpreting algebraic\n\
expressions, plotting functions and solving equations.\033[0m\n\n\
To contribute to the Lexemn project go to https://github.com/fontseca/lexemn\n\
Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n\n\
Invoke `help()' for further information; and `quit()' or `q()' to exit.\n\
\n", stdout);
}

/* Displays usage guide.  */
void usage(std::int32_t status)
{
  if (EXIT_SUCCESS ^ status)
  {
    std::fprintf(stderr, "Try '%s --help' for more information.\n", PROGNAME);
  }
  else
  {
    std::printf("\
Usage: %s [OPTION]... [FILE]...\n\
\n", PROGNAME);

    std::fputs("\
An open source mathematical tool and library for interpreting algebraic\n\
expressions, plotting functions and solving equations.\n\
\n", stdout);

    std::fputs("\
  -q, --quiet                  do not display welcome message\n\
  -c, --color                  be colourful\n\
  -d, --debug                  display debugging messages\n\
  -h, --help                   display this information\n\
  -v, --version                display current version\n\
      --verbose                explain what is being done\n\
\n", stdout);

    std::fprintf(stdout, "\
%s %s Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n",
        PROGNAME, VERSION);
  }
 
  std::exit(status);
}

/* Displays current version.  */
void emit_version()
{
  std::fprintf(stdout, "\
%s %s Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>\n",
    PROGNAME, VERSION);
  std::exit(EXIT_SUCCESS);
}

std::int32_t main(std::int32_t argc, char **argv)
{
  char c;

  while (c = getopt_long(
        argc, argv, "cqvdh", long_options, NULL), c ^ -1)
  {
    if (c == '?')
    {
      ::usage(EXIT_FAILURE);
    }

    switch (c)
    {
    case 'c':
    {
      x.color = true;
      break;
    }
    case 'd':
    {
      x.debug = true;
      break;
    }
    case 'q':
    {
      x.quiet = true;
      break;
    }
    case 'h':
    {
      ::usage(EXIT_SUCCESS);
      break;
    }
    case 'v':
    {
      ::emit_version();
      break;
    }
    default:
      ::usage(EXIT_FAILURE);
    }
  }

  /* Run in verbose mode.  */

  if (!x.quiet)
  {
    emit_welcome();
  }

  /* Initialize lexer with a book by default.  */

  lexemn::internal::lexer lexer(
        std::make_unique<lexemn::internal::book>());

  for (;;)
  {
    std::unique_ptr<char[], decltype(&std::free)> line(
          readline(PROMPT(::/)), std::free);

#undef PROMPT
#undef C_PROMPT
#undef UNC_PROMPT

    if (std::regex_search(line.get(),
        std::regex("^[[:space:]]*$",
            std::regex_constants::grep)))
    {
      continue;
    }

    /* FIXME: Consider using a procedure for correct quitting when recieving a
       signal.  */

    if (!strcmp(line.get(), "quit()")
          || !strcmp(line.get(), "q()"))
    {
      line.reset(nullptr);
      lexer.~lexer();
      std::exit(EXIT_SUCCESS);
    }

    add_history(line.get());

    lexemn::internal::characters_stream chstream { line.get() };

    lexer.book()->push_page_from_stream(chstream);
    lexer.lex();
  }

  return EXIT_SUCCESS;
}

/*
 * lexer.h -- the lexical analyzer
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

#ifndef LEXER_H
#define LEXER_H

/*
 * The lexical analyzer takes a raw string as its input and will convert it
 * into a sequence of lexical tokens. These tokes are strings with an assigned
 * and identified value. The parser will use these tokens to later create the
 * parse tree.
 *
 * For the lexemn string expression
 *  "x:=1*2;",
 * the lexer will have to generate a structure of tokens similar to the
 * following:
 *   (lexeme,   token name)
 *   ("x",      <identifier>)
 *   ("1",      <literal>)
 *   ("*",      <operator>)
 *   ("2",      <literal>)
 *   (";",      <separator>).
 *
 * The lexer does not know where each token should be located, it can report
 * about not valid tokens (e.g., an illegal or unrecognized symbol) and other
 * malformed entities. It will not look for tokens out of space, mispelled
 * keywords, mistmatched types, etc.
 *
 * For the sentence '6 := f(x)', the lexer will not generate any error becasue
 * it has no concept of the appropiate arrangement of the tokens. It is the
 * parser's job to catch this error.
 *
 * The lexer can be a convenient place to carry out some other stuff like
 * stripping out comments and white spaces between tokens.
 */

class lexer
{
};

#endif
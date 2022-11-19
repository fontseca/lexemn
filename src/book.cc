/*
 * book.cc -- lexemn book implementation
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

#include <iostream>

#include "lexemn/internal.h"

namespace lexemn::internal
{

book::book() noexcept
  : page { nullptr }
  , pages_count { 0 }
  , m_current_token { nullptr }
  , m_token_stream { nullptr }
  , m_hash_table { nullptr }
  , m_date { "" }
  , m_time { "" }
{
}

/* Allocates a new page buffer based on the given stream of charactrs.  Then
   insertes the buffer onto the top of the pages stack of the current book, and
   laters increments pages count in one unit.  */
void book::push_page_from_stream(characters_stream& chstream) noexcept
{
  if (this->page == nullptr)
  {
    this->page = std::make_shared<page_buffer>(
          std::make_shared<characters_stream>(std::move(chstream)));
  }
  else
  {
    this->page = std::make_shared<page_buffer>(
          std::make_shared<characters_stream>(std::move(chstream)), this->page);
  }
  this->pages_count++;
}

}
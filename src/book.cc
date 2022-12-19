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

#include <cstring>

#include <iostream>

#include "lexemn/internal.h"
#include "lexemn/utility.h"

namespace lexemn::internal
{

/* Constructs a Lexemn book.  */
lexemn_book::lexemn_book() noexcept
  : m_head { nullptr }
  , m_pages_count { 0 }
  , m_symbol_table { nullptr }
  , m_date { "" }
  , m_time { "" }
{
  m_base_buffer = std::make_unique<tokens_buffer>();
  init_token_buffer(DEREFER(m_base_buffer), TOKENS_BUFFER_SIZE);
  m_current_buffer = DEREFER(m_base_buffer);
  m_next = DEREFER(m_base_buffer->base);
}

/* Initializes a token buffer.  */
auto lexemn_book::init_token_buffer(tokens_buffer *buff,
  std::uint32_t count) noexcept
  -> void
{
  buff->base = std::make_unique<lexemn_token[]>(count);
  buff->limit = DEREFER(buff->base) + count;
  buff->next = nullptr;
}

/* Allocates the next buffer of tokens.  */
auto lexemn_book::next_tokens_buffer(tokens_buffer *buff,
  std::uint32_t count) noexcept
  -> tokens_buffer *
{
  if (buff->next == nullptr)
  {
    buff->next = std::make_unique<tokens_buffer>();
    buff->next->prev = m_current_buffer;
    init_token_buffer(DEREFER(buff->next), count);
  }

  return DEREFER(buff->next);
}

/* Allocates a new page buffer based on the given stream of charactrs.  Then
   insertes the buffer onto the top of the pages stack of the current book, and
   laters increments pages count in one unit.  Returns the pages count.  */
auto lexemn_book::push_page_from_stream(const characters_stream chstream) noexcept
  -> std::uint32_t
{
  if (m_head == nullptr)
  {
    m_head = std::make_unique<lexemn_page>();
    m_head->len = 1 + std::strlen(chstream);
    m_head->buffer = utility::duplicate_stream(chstream, m_head->len);
    m_head->current = m_head->buffer;
    m_head->line = m_head->current;
    m_head->prev = nullptr;
  }
  else
  {
    auto new_page = std::make_unique<lexemn_page>();
    new_page->len = 1 + std::strlen(chstream);
    new_page->buffer = utility::duplicate_stream(chstream, new_page->len);
    new_page->current = new_page->buffer;
    new_page->line = new_page->current;
    new_page->prev = DEREFER(m_head);
    m_head = std::move(new_page);
  }

  return m_pages_count++;
}

}
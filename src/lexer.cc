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

#include <cctype>
#include <climits>
#include <cuchar>

#include <iostream>
#include <string_view>
#include <iomanip>
#include <string>

#include "lexemn/internal.h"
#include "lexemn/utility.h"
#include "lexemn/charset.h"

namespace lexemn::internal
{

lexer::lexer(lexemn_book::pointer &&pbook) noexcept
  : m_book { std::move(pbook) }
{ }

lexer::~lexer() noexcept
{
  RESET(m_book, nullptr);
}

/* Sets the book to be lexed.  */
auto lexer::set_book(lexemn_book::pointer pbook) noexcept
  -> void
{
  m_book = std::move(pbook);
}

/* Gets the book to be lexed.  */
auto lexer::get_book() const noexcept
  -> const lexemn_book::pointer &
{
  return m_book;
}

auto lexer::lex_token() noexcept
  -> const lexemn_token::pointer
{
  return { };
}

/* Adds a new error to the errors stream.  */
auto lexer::enqueue_error() noexcept
  -> void
{
  std::ostringstream err { };
  std::ptrdiff_t diff = m_book->m_head->current - m_book->m_head->line;
  auto begin = m_book->m_head->line;
  auto st = m_book->m_head->line;

  err << "lexemn: \x1B[1;31merror:\x1B[0m unknown symbol detected near ‘"
    << *m_book->m_head->current << "’" << '\n';

  err << "     |  ";

  for (;; ++st)
  {
    if (*begin == '\n' || *begin == EOF || *begin == '\0')
    {
      break;
    }
    else if (st == (m_book->m_head->line + diff)) /* color unwanted symbol */
    {
      err << "\x1B[1;31m" << *begin << "\x1B[0m";
      ++begin;
      continue;
    }
    else
    {
      err << *begin;
      ++begin;
    }
  }

  err << '\n' << "     |  " << "\x1B[1;31m"
    << std::setw(diff + 1) << "^" << "\x1B[0m" << '\n';
  m_errors << err.str() << '\n';
}

/* Flushes errors to stderr.  */
[[nodiscard]] auto lexer::flush_errors()
  -> bool
{
  std::cerr << m_errors.str();
  m_errors.clear();
  m_errors.str(std::string());
  return m_errors.str().empty();
}

/* Lexes a number to  `token_type::LEXEMN_INTEGER'.  */
auto lexer::lex_number() noexcept
  -> void
{
  auto result { allocate_token(LEXEMN_NUMBER) };
  auto current = &m_book->m_head->current;

  /* When a number is preceded by `-' or `+'.  Generally this should be
     executed once.  */
  switch (**current)
  {
    case '+': case '-':
    {
      result->value += **current, ++*current;
      break;
    }
  }

  if ('.' == **current)
  {
      result->value += **current, ++*current;
  }

  for (;;)
  {
    switch (**current)
    {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      {
        result->value += **current, ++*current;
        continue;
      }
    }

    if (**current == '.')
    {
      enqueue_error();
    }

    /* When `e' or `E'.  */
    switch (**current)
    {
      case 'e': case 'E':
      {
        switch (*(*current + 1))
        {
          case '+': case '-':
          {
            switch (*(*current + 2))
            {
              case '0': case '1': case '2': case '3': case '4':
              case '5': case '6': case '7': case '8': case '9':
              {
                goto increment;
              }
            }
            break;
          }

          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            goto increment;
          }
        }
      }
    }

    break;

    increment:
    result->value += **current; /* Add either `e' or `E',  */
    result->value += *(*current + 1); /* then add `+', `-' or any of [0-9].  */
    *current += 2;
  }

  push_token(std::move(result));
}

/* Allocates room for a new token.  */
auto lexer::allocate_token(const token_type type) const noexcept
  -> lexemn_token::pointer
{
  auto token = std::make_unique<lexemn_token>();
  token->type = type;
  return token;
}

/* Adds a token to the current buffer of tokens.  */
auto lexer::push_token(lexemn_token::pointer token) noexcept
  -> void
{
  if (nullptr not_eq m_book->m_current_buffer->base)
  {
    /* If the base of the current buffer of tokens needs more room,  */

    if (m_book->m_current_buffer->limit == m_book->m_next)
    {
      std::cout << "making reallocation..." << '\n';

      std::ptrdiff_t capacity =
        2 * static_cast<std::ptrdiff_t>(m_book->m_current_buffer->limit
          - DEREFER(m_book->m_current_buffer->base));

      /* then allocate twice its previous capacity.  */

      auto base = new lexemn_token[capacity];

      auto token = DEREFER(m_book->m_current_buffer->base);
      std::int32_t i { -1 };

      while (token not_eq m_book->m_next)
        *(base + ++i) = std::move(*token++);

      /* (Where the next token will be in the new base.)  */

      std::ptrdiff_t current = m_book->m_next
        - DEREFER(m_book->m_current_buffer->base);

      RESET(m_book->m_current_buffer->base, base);

      m_book->m_current_buffer->limit = DEREFER(
        m_book->m_current_buffer->base) + capacity;
      
      m_book->m_next = DEREFER(m_book->m_current_buffer->base) + current;
    }

    *m_book->m_next = std::move(*token);
    ++m_book->m_next;
  }
}

/* Lexes an identifier.  */
auto lexer::lex_identifier() noexcept
  -> void
{
  auto result { allocate_token(LEXEMN_IDENTIFIER) };
  char32_t c32 { };
  while (std::mbrtoc32(&c32, m_book->m_head->current,
    m_book->m_head->len, nullptr))
  {
    if (!charset::unicode_valid_in_identifier(c32))
      break;
    result->value += *m_book->m_head->current;
    ++m_book->m_head->current;
  }

  push_token(std::move(result));
}

/* Skip blanks.  */
auto lexer::skip_blank() noexcept
  -> void
{
  do
    ++m_book->m_head->current;
  while(blank());
}

/* Checks if the current character is EOL.  */
[[nodiscard]] auto inline lexer::eol() const noexcept
  -> bool
{
  return *m_book->m_head->current == '\n';
}

/* Checks if the current character is EOS (End Of String).  */
[[nodiscard]] auto inline lexer::eos() const noexcept
  -> bool
{
  return *m_book->m_head->current == '\0';
}

/* Checks if the current character is EOF.  */
[[nodiscard]] auto inline lexer::eof() const noexcept
  -> bool
{
  return *m_book->m_head->current == EOF;
}

/* Checks if the current character is blank.  */
[[nodiscard]] auto inline lexer::blank() const noexcept
  -> bool
{
  return std::isspace(
    static_cast<std::uint8_t>(*m_book->m_head->current))
      or std::isblank(
        static_cast<std::uint8_t>(*m_book->m_head->current));
}

/* Converts a raw string expression with the Lexemn grammar into a sequence of
   recognized tokens that can be interpreted by the parser.  If any illegal symbol
   is detected, adds errors to the erros stream.  */
auto lexer::lex() noexcept
  -> void
{
  /* Current character in the source stream.  */
  characters_stream *current = &m_book->m_head->current;
  std::size_t rc;
  char32_t c32;

  while ((rc = std::mbrtoc32(&c32, *current,
    m_book->m_head->len, nullptr)))
  {
    if (blank())
    {
      skip_blank();
      continue;
    }

    if (eol())
    {
      ++*current;
      m_book->m_head->line = *current;

      /* As soon as the current caracter is not one of these...  */

      if (not eos()
        and not eol()
          and not eof())
      {
        /* allocate a new buffer for the next line  */

        std::cout << "making a new buffer...\n";
        m_book->m_current_buffer = m_book->next_tokens_buffer(
          m_book->m_current_buffer, TOKENS_BUFFER_SIZE);
        
        /* and then point to the new buffer.  */
        m_book->m_next = DEREFER(m_book->m_current_buffer->base);
        std::cout << "made." << '\n';
      }

      continue;
    }

    switch(**current)
    {
      case '.':
      {
        if (charset::unicode_valid_in_number(*(*current + 1)))
        {
          lex_number();
          continue;
        }
        break;
      }

      case '+': case '-':
      {
        switch (*(*current + 1))
        {
          case '.':
          {
            if (charset::unicode_valid_in_number(*(*current + 2)))
            {
              lex_number();
              continue;
            }
            break;
          }
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
          {
            lex_number();
            continue;
          }
        } 
        if (charset::unicode_valid_in_number(*(*current + 1)))
        {
          lex_number();
          continue;
        }
     /* else
        {
          lex_arithmetic_operator();
          continue;
        }  */
        break;
      }

   /* case '+': case '-': case '*':
      case '⋅': case '/': case '÷':
      case '^': case '%': case '‰':
      case '.': case '!':
      {
        lex_arithmetic_operator();
        break;
      } */
    }
    
    if (charset::unicode_valid_in_identifier(
      static_cast<unsigned int>(c32)))
    {
      lex_identifier();
      continue;
    }
    else if (std::isdigit(static_cast<std::uint8_t>(**current)))
    {
      lex_number();
      continue;
    }
    else if (eos() or eof())
    {
      return;
    }

    *current += rc;
  }

  if (not(m_errors.str().empty()))
  {
    (void) flush_errors();
  }
  else
  {
    stringify_token_buffer();
  }
}

/* Generates a string based on the sequence of tokens generated by the lexer.  */
auto lexer::stringify_token_buffer() const noexcept
  -> void
{
  auto buf = m_book->m_current_buffer;

  if (nullptr not_eq buf)
  {
    std::puts(" [");
    auto token = DEREFER(buf->base);
    while (token not_eq m_book->m_next)
    {
      std::printf("  { \"%s\", %s },\n", token->value.data(),
        token_spellings[token->type].name );
      ++token;
    }
    std::puts("]");
  }
}

}

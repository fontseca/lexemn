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
#include <cstdarg>
#include <chrono>

#include "lexemn/internal.h"
#include "lexemn/utility.h"
#include "lexemn/charset.h"
#include "lexemn/error.h"

namespace lexemn::internal
{

/* The number of vertibal bars `|' in an line.  */
static std::uint32_t vertical_bars { 0 };

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
auto lexer::enqueue_error(const error::error_type error_type) noexcept
  -> void
{
  std::ostringstream error { };
  char error_header_buffer[0x400];
  std::string error_type_message;
  std::ptrdiff_t offset = m_book->m_head->current - m_book->m_head->line;
  auto iterator = m_book->m_head->line;

  utility::make_error_message(error_type_message, error_type);

  const char current_as_string[2] = { *m_book->m_head->current, '\0' };

  std::sprintf(error_header_buffer, error_type_message.c_str(), current_as_string);

  error << std::move(error_header_buffer)
        << "     |  ";

  for (;; ++iterator)
  {
    if ('\n'  == *iterator
      || EOF  == *iterator
      || '\0' == *iterator)
    {
      break;
    }
    else if (m_book->m_head->line + offset == iterator)
    {
      error << "\x1B[1;31m"
            << *iterator
            << "\x1B[0m";
      continue;
    }
    else
    {
      error << *iterator;
    }
  }

  error << "\n     |  "
        << std::string(charset::offset(m_book->m_head->line, offset), ' ')
        << "\x1B[1;31m^\x1B[0m\n";

  m_errors << error.str() << '\n';
}

/* Adds a new error to the errors stream.  The error that was in the current line
   must be in the range [begin, end].  */
auto lexer::enqueue_error(const error::error_type error_type,
  const std::uint32_t begin, const std::uint32_t end) noexcept
  -> void
{
  std::ostringstream error { };
  char error_header_buffer[0x400];
  auto iterator = m_book->m_head->line;
  std::string error_type_message;

  utility::make_error_message(error_type_message, error_type);
  std::sprintf(error_header_buffer, error_type_message.c_str(), m_book->m_head->current);

  error << std::move(error_header_buffer)
        << "     |  ";

  for (;; ++iterator)
  {
    if ('\n'  == *iterator
      || EOF  == *iterator
      || '\0' == *iterator)
    {
      break;
    }

    if (iterator == (m_book->m_head->line + begin))
    {
      error << "\x1B[1;31m";
    }

    error << *iterator;

    if (iterator == (m_book->m_head->line + end))
    {
      error << "\x1B[0m";
    }
  }

  error << "\n     |  "
        << std::string(charset::offset(m_book->m_head->line, begin), ' ')
        << "\x1B[1;31m^"
        << std::string(end - begin, '~') << "\x1B[0m\n";

  m_errors << error.str() << '\n';
}

/* Enqueues a new error at the given position `at' with a custom format message in
   `format'. */
auto lexer::enqueue_error(const std::uint32_t at, const char *const format, ...) noexcept
  -> void
{
  char formatted_buffer[0x300];
  auto iterator = m_book->m_head->line;
  std::ostringstream erroneous_sentence { };
  std::va_list arguments;
  std::string error_type_message;

  va_start(arguments, format);

  std::vsprintf(formatted_buffer, format, arguments);
  utility::make_error_message(error_type_message, formatted_buffer);

  for(;; ++iterator)
  {
    if ('\n' == *iterator
      || EOF == *iterator
      || '\0' == *iterator)
    {
      break;
    }

    if (m_book->m_head->line + at == iterator)
    {
      erroneous_sentence << "\x1B[1;31m" << *iterator << "\x1B[0m";
      continue;
    }

   erroneous_sentence << *iterator;
  }

  va_end(arguments);

  m_errors << error_type_message
           << "     |  " << erroneous_sentence.str()
           << "\n     |  "
           << std::string(charset::offset(m_book->m_head->line, at), ' ')
           << "\x1B[1;31m^\x1B[0m\n";
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

/* Lexes a miscellaneous operator.  */
auto lexer::lex_misc_operator() noexcept
  -> void
{
  auto result { allocate_token() };

  switch(*m_book->m_head->current)
  {
    case ';':
    {
      result->type = token_type::LEXEMN_MISC_OP_SEMICOLON;
      break;
    }
    case ',':
    {
      result->type = token_type::LEXEMN_MISC_OP_COMMA;
      break;
    }
    case '?':
    {
      result->type = token_type::LEXEMN_MISC_OP_HELP;
      break;
    }
  }

  result->value = { *m_book->m_head->current, '\0' };
  push_token(std::move(result));
  ++m_book->m_head->current;
}

/* Lexes an algebraic operator.  */
auto lexer::lex_algebraic_operator() noexcept
  -> void
{
  auto result { allocate_token() };

  if (*m_book->m_head->current == '|')
  {
    result->type = ++vertical_bars & 1
      ? token_type::LEXEMN_ALGEBRAIC_OP_LEFT_ABSOLUTE
      : token_type::LEXEMN_ALGEBRAIC_OP_RIGHT_ABSOLUTE;
    result->value = "|";
    ++m_book->m_head->current;
    goto do_push;
  }

  if (*m_book->m_head->current == ':'
    and *(m_book->m_head->current + 1) == '=')
  {
    result->type = token_type::LEXEMN_ALGEBRAIC_OP_DEFINITION;
    result->value = ":=";
    m_book->m_head->current += 2;
    goto do_push;
  }

  switch(*m_book->m_head->current)
  {
    case '(':
    {
      result->type = token_type::LEXEMN_ALGEBRAIC_OP_OPEN_PAREN;
      break;
    }

    case ')':
    {
      result->type = token_type::LEXEMN_ALGEBRAIC_OP_CLOSE_PAREN;
      break;
    }

    case '[':
    {
      result->type = token_type::LEXEMN_ALGEBRAIC_OP_OPEN_BRACKET;
      break;
    }

    case ']':
    {
      result->type = token_type::LEXEMN_ALGEBRAIC_OP_CLOSE_BRACKET;
      break;
    }

    case '{':
    {
      result->type = token_type::LEXEMN_ALGEBRAIC_OP_OPEN_BRACE;
      break;
    }
  }

  result->value = { *m_book->m_head->current, '\0' };
  ++m_book->m_head->current;

do_push:
  push_token(std::move(result));
}

/* Lexes an arithmetic operator.  */
auto lexer::lex_arithmetic_operator() noexcept
  -> void
{
  auto result { allocate_token() };

  if (*m_book->m_head->current == '+'
    and *(m_book->m_head->current + 1) == '-')
  {
    result->type = token_type::LEXEMN_ARITH_OP_PLUS_MINUS;
    result->value = "+-";
    m_book->m_head->current += 2;
    goto do_push;
  }

  if (*m_book->m_head->current == '+'
    and *(m_book->m_head->current + 1) == '+')
  {
    result->type = token_type::LEXEMN_ARITH_OP_INCREMENT;
    result->value = "++";
    m_book->m_head->current += 2;
    goto do_push;
  }

  if (*m_book->m_head->current == '-'
    and *(m_book->m_head->current + 1) == '+')
  {
    result->type = token_type::LEXEMN_ARITH_OP_MINUS_PLUS;
    result->value = "-+";
    m_book->m_head->current += 2;
    goto do_push;
  }

  if (*m_book->m_head->current == '-'
    and *(m_book->m_head->current + 1) == '-')
  {
    result->type = token_type::LEXEMN_ARITH_OP_DECREMENT;
    result->value = "--";
    m_book->m_head->current += 2;
    goto do_push;
  }

  switch(*m_book->m_head->current)
  {
    case '-':
    {
      result->type = token_type::LEXEMN_ARITH_OP_MINUS;
      break;
    }

    case '+':
    {
      result->type = token_type::LEXEMN_ARITH_OP_PLUS;
      break;
    }

    case '*':
    {
      result->type = token_type::LEXEMN_ARITH_OP_MULT_ASTERISK;
      break;
    }

    case '/':
    {
      result->type = token_type::LEXEMN_ARITH_OP_DIV_SLASH;
      break;
    }

    case '^':
    {
      result->type = token_type::LEXEMN_ARITH_OP_EXPONENTIATION;
      break;
    }

    case '%':
    {
      result->type = token_type::LEXEMN_ARITH_OP_PERCENT;
      break;
    }

    case '!':
    {
      result->type = token_type::LEXEMN_ARITH_OP_FACTORIAL;
      break;
    }

    /* FIXME: Correct ambiguous case.   */
    case '.':
    {
      result->type = token_type::LEXEMN_ARITH_OP_PERIOD;
      break;
    }
  }

  result->value = { *m_book->m_head->current, '\0' };
  ++m_book->m_head->current;

do_push:
  push_token(std::move(result));
}

/* Lexes a number to  `token_type::LEXEMN_INTEGER'.  */
auto lexer::lex_number() noexcept
  -> void
{
  auto result { allocate_token(token_type::LEXEMN_NUMBER) };
  auto current = &m_book->m_head->current;
  std::uint32_t decimal_points { 0 };
  const std::ptrdiff_t number_begins_at = m_book->m_head->current - m_book->m_head->line;
  std::ptrdiff_t number_ends_at;

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
      result->value += **current, ++*current;
      ++decimal_points;
      continue;
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

  number_ends_at = m_book->m_head->current - m_book->m_head->line - 1;

  if (decimal_points > 1)
  {
    enqueue_error(error::error_type::too_many_decimal_points,
      number_begins_at, number_ends_at);
    return;
  }

  push_token(std::move(result));
}

/* Allocates room for a new token.  */
auto lexer::allocate_token(const token_type type) const noexcept
  -> lexemn_token::pointer
{
  auto token = std::make_unique<lexemn_token>();
  token->type = type;
  token->value = "";
  token->position = 0;
  return token;
}

/* Allocates room for a new token with a default type and content.  */
auto lexer::allocate_token() const noexcept
  -> lexemn_token::pointer
{
  return allocate_token(token_type::LEXEMN_CONST_PI);
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
      std::ptrdiff_t new_base_capacity =
        2 * static_cast<std::ptrdiff_t>(m_book->m_current_buffer->limit
          - DEREFER(m_book->m_current_buffer->base));

      /* then allocate twice its previous capacity.  */

#ifdef DEBUGGING
        char message[0x64];
        sprintf(message, "expanding base of tokens buffer; new capacity is %ld", new_base_capacity);
        log(message);
#endif

      auto new_base_of_tokens = new lexemn_token[new_base_capacity];

      auto base_token_of_current_buffer = DEREFER(m_book->m_current_buffer->base);
      std::int32_t i { -1 };

      while (base_token_of_current_buffer not_eq m_book->m_next)
        *(new_base_of_tokens + ++i) = std::move(*base_token_of_current_buffer++);

      /* (Where the next token will be in the new base.)  */

      std::ptrdiff_t current = m_book->m_next
        - DEREFER(m_book->m_current_buffer->base);

      RESET(m_book->m_current_buffer->base, new_base_of_tokens);

      m_book->m_current_buffer->limit = DEREFER(
        m_book->m_current_buffer->base) + new_base_capacity;
      
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
  auto result { allocate_token(token_type::LEXEMN_IDENTIFIER) };
  char32_t c32 { };

  while (std::mbrtoc32(&c32, m_book->m_head->current, m_book->m_head->len, nullptr))
  {
    if (not charset::unicode_valid_in_identifier(c32))
    {
      break;
    }

    result->value += *m_book->m_head->current;
    ++m_book->m_head->current;
  }

  push_token(std::move(result));
}

/* Skips any blank character.  */
auto lexer::skip_blank() noexcept
  -> void
{
  while(blank())
  {
    ++m_book->m_head->current;
  }
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
  return std::isblank(static_cast<std::uint8_t>(*m_book->m_head->current));
}

/* Converts a raw string expression with the Lexemn grammar into a sequence of
   recognized tokens that can be interpreted by the parser.  If any illegal symbol
   is detected, adds errors to the erros stream.  */
auto lexer::lex() noexcept
  -> void
{
  /* Current character in the source stream.  */
  characters_stream *current = std::addressof(m_book->m_head->current);
  std::size_t offset;
  char32_t code_point;
  bool next_line_needs_buffer { true };

  while ((offset = std::mbrtoc32(&code_point, *current, m_book->m_head->len, nullptr)))
  {
    if (blank())
    {
      skip_blank();
      continue;
    }

    /* Move cursor to the next line.  */
    if (eol())
    {
      ++*current;
      m_book->m_head->line = *current;
      next_line_needs_buffer = true;
      continue;
    }

    if (not blank() and not eol() and not eof()
      and not eos() and next_line_needs_buffer)
    {
      vertical_bars = 0;

      /* Allocate the first buffer of tokens.  */

      if (m_book->m_current_buffer == nullptr)
      {
        m_book->m_base_buffer = std::make_unique<tokens_buffer>();
        m_book->init_token_buffer(DEREFER(m_book->m_base_buffer), TOKENS_BUFFER_SIZE);
        m_book->m_current_buffer = DEREFER(m_book->m_base_buffer);
        m_book->m_next = DEREFER(m_book->m_base_buffer->base);
#ifdef DEBUGGING
        log("making first buffer of tokens");
#endif
      }
      else
      {
        /* Allocate a new buffer for the next line,  */

#ifdef DEBUGGING
        log("making a new buffer of tokens");
#endif
        m_book->m_current_buffer = m_book->next_tokens_buffer(
          m_book->m_current_buffer, TOKENS_BUFFER_SIZE);

        /* and then point to the new buffer.  */
        m_book->m_next = DEREFER(m_book->m_current_buffer->base);
      }

      next_line_needs_buffer = false;
    }

    /* When an arithmetic operator's code point is universal...  */

    if (code_point == 0x22C5)
    {
      auto result { allocate_token(token_type::LEXEMN_ARITH_OP_MULT_DOT) };
      result->value = "⋅";
      push_token(std::move(result));
      *current += std::mblen(*current, MB_CUR_MAX);
      continue;
    }
    else if (code_point == 0x00F7)
    {
      auto result { allocate_token(token_type::LEXEMN_ARITH_OP_DIV_OBELUS) };
      result->value = "÷";
      push_token(std::move(result));
      *current +=  std::mblen(*current, MB_CUR_MAX);
      continue;
    }
    else if (code_point == 0x2030)
    {
      auto result { allocate_token(token_type::LEXEMN_ARITH_OP_PERMILE) };
      result->value = "‰";
      push_token(std::move(result));
      *current += std::mblen(*current, MB_CUR_MAX);
      continue;
    }

    /* When an algebraic operator's code point is universal...   */

    else if (code_point == 0x230A)
    {
      auto result { allocate_token(token_type::LEXEMN_ALGEBRAIC_OP_LEFT_FLOOR) };
      result->value = "⌊";
      push_token(std::move(result));
      *current += std::mblen(*current, MB_CUR_MAX);
      continue;
    }
    else if (code_point == 0x230B)
    {
      auto result { allocate_token(token_type::LEXEMN_ALGEBRAIC_OP_RIGHT_FLOOR) };
      result->value = "⌋";
      push_token(std::move(result));
      *current +=  std::mblen(*current, MB_CUR_MAX);
      continue;
    }
    else if (code_point == 0x2308)
    {
      auto result { allocate_token(token_type::LEXEMN_ALGEBRAIC_OP_LEFT_CEILING) };
      result->value = "⌈";
      push_token(std::move(result));
      *current += std::mblen(*current, MB_CUR_MAX);
      continue;
    }
    else if (code_point == 0x2309)
    {
      auto result { allocate_token(token_type::LEXEMN_ALGEBRAIC_OP_RIGHT_CEILING) };
      result->value = "⌉";
      push_token(std::move(result));
      *current += std::mblen(*current, MB_CUR_MAX);
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

      case '=':
      {
        char current_as_string[2] = { **current, '\0' };
        enqueue_error(m_book->m_head->current - m_book->m_head->line,
          error::errors_dictionary[error::error_type::stray_in_program_with_suggestion],
            current_as_string, ":=");
        ++*current;
        continue;
      }

      case '+':
      case '-':
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
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
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
        else
        {
          lex_arithmetic_operator();
          continue;
        }

        break;
      }

      case ',':
      case ';':
      case '?':
      {
        lex_misc_operator();
        continue;
      }

      case ':':
      {
        if (*(m_book->m_head->current + 1) == '=')
        {
          lex_algebraic_operator();
          continue;
        }
        else
        {
          char current_as_string[2] = { **current, '\0' };
          enqueue_error(m_book->m_head->current - m_book->m_head->line,
            error::errors_dictionary[error::error_type::stray_in_program_with_suggestion],
              current_as_string, ":=");
          ++*current;
          continue;
        }

        break;
      }

      case '(':
      case ')':
      case '[':
      case ']':
      case '{':
      case '}':
      case '|':
        lex_algebraic_operator();
        continue;

      case '*':
      case '/':
      case '^':
      case '%':
      case '!':
        lex_arithmetic_operator();
        continue;
    }

    if (charset::unicode_valid_in_identifier(
      static_cast<unsigned int>(code_point)))
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
    else
    {
      enqueue_error(error::error_type::unknown_symbol_detected);
    }

    *current += offset;
  }

  if (not(m_errors.str().empty()))
  {
    (void) flush_errors();
  }
#ifdef DEBUGGING
  else
  {
    stringify_token_buffer();
  }
#endif
}

/* Generates a string based on the sequence of tokens generated by the lexer.  */
auto lexer::stringify_token_buffer() const noexcept
  -> void
{
  const auto current_buffer = m_book->m_current_buffer;

  if (nullptr not_eq current_buffer)
  {
    auto token = DEREFER(current_buffer->base);

    std::puts("\n[");

    while (token not_eq m_book->m_next)
    {
      std::printf("  { `%s', %s },\n", token->value.data(),
        token_spellings[static_cast<std::uint32_t>(token->type)].name.c_str() );
      ++token;
    }

    std::puts("]\n");
  }
}

/* Logs `message' to stdout.  */
auto inline lexer::log(const std::string_view message) const noexcept
  -> void
{
  std::fprintf(stdout, "lexemn: \x1B[1;71mdebugging:\x1B[0m %s\n", message.data());
}

}

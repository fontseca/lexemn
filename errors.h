/*
 * errors.h -- Error handling declarations.
 *
 * https://github.com/fontseca/lexemn
 *
 * Copyright (C) 2026 by Jeremy Fonseca <fontseca.dev@outlook.com>
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

#ifndef LEXEMN_ERRORS_H
#define LEXEMN_ERRORS_H

/* Report a diagnostic error for context LEXEMN at source position LOC and character
   offset OFF.  REASON is a printf-style format string followed by additional
   arguments.  */
extern void
error_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...);

/* Report a diagnostic warning for context LEXEMN at source position LOC and character
   offset OFF.  REASON is a printf-style format string followed by additional arguments.  */
extern void
warning_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...);

/* Report a diagnostic note for context LEXEMN at source position LOC and character
   offset OFF.  REASON is a printf-style format string followed by additional arguments.  */
extern void
note_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...);

/* Report a diagnostic hint for context LEXEMN at source position LOC and character
   offset OFF.  REASON is a printf-style format string followed by additional arguments.  */
extern void
hint_at(struct lexemn const *lexemn, loc_t loc,
                uint32_t off, char const *reason, ...);

#endif //LEXEMN_ERRORS_H

#
#  Root CMakeLists.txt file for Lexemn.
#
#  https://github.com/fontseca/lexemn
#
#  Copyright (C) 2026 by Jeremy Fonseca <fontseca.dev@outlook.com>
#
#  This file is part of Lexemn.
#
#  Lexemn is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  Lexemn is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along with
#  Lexemn. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)
project(lexemn LANGUAGES C VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)

enable_testing()

# Flags for GCC and Clang
set(COMPILE_FLAGS
    -Wall
    -Wextra
    -Wpedantic               # Issue all warnings demanded by strict ISO C
    -Werror                  # Treat all warnings as errors
    -Wshadow                 # Warn when a local variable shadows another variable
    -Wconversion             # Warn on implicit conversions that may alter values
    -Wsign-conversion        # Warn on implicit sign conversions
    -Wformat=2               # Strong check for printf/scanf format strings
    -Wnull-dereference       # Warn on path execution dereferencing NULL pointers
    -Wdouble-promotion       # Warn if float is implicitly promoted to double
    -Wimplicit-fallthrough   # Require explicit fallthrough annotations in switch statements
    -Winit-self              # Warn about uninitialized variables initialized to themselves
    -Wstrict-prototypes      # Require explicit function prototypes
    -Wno-unused-parameter    # Suppress unused parameter noise if needed)
)

add_library(lexer OBJECT lexer.c lexer.h)

target_include_directories(lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(lexer PRIVATE ${COMPILE_FLAGS})

# Main executable
add_executable(lexemn.out lexemn.c)
target_link_libraries(lexemn.out PRIVATE lexer readline)
target_compile_options(lexemn.out PRIVATE ${COMPILE_FLAGS})

# Unit test executable
add_executable(tests.out lexer_test.c)
target_link_libraries(tests.out PRIVATE lexer)
target_compile_options(tests.out PRIVATE ${COMPILE_FLAGS})

# CTest Registration (pointed to executable build artifact target)
add_test(NAME run_unit_test COMMAND tests.out)

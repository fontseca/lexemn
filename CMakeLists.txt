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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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

# Core Libraries.

add_library(lex STATIC lex.c lex.h errors.c loc.c)
target_link_libraries(lex PRIVATE m)
target_include_directories(lex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(lex PRIVATE ${COMPILE_FLAGS})

# Main executable

add_executable(lexemn.out lexemn.c errors.c loc.c)
target_link_libraries(lexemn.out PRIVATE lex readline m)
target_compile_options(lexemn.out PRIVATE ${COMPILE_FLAGS})

# Helper object target for test compilation of lexemn.c

add_library(lexemn_test_obj OBJECT lexemn.c)
target_compile_options(lexemn_test_obj PRIVATE ${COMPILE_FLAGS})
target_compile_definitions(lexemn_test_obj PRIVATE main=lexemn_main)
target_include_directories(lexemn_test_obj PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Unit tests.

add_executable(test_lex.out lex_test.c errors.c loc.h $<TARGET_OBJECTS:lexemn_test_obj>)
target_link_libraries(test_lex.out PRIVATE lex readline m)
target_compile_options(test_lex.out PRIVATE ${COMPILE_FLAGS})
add_test(NAME test_lex COMMAND test_lex.out)

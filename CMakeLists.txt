# 
#  Root CMakeLists.txt file for Lexemn.
#   ___       _______      ___    ___ _______   _____ ______   ________
#  |\  \     |\  ___ \    |\  \  /  /|\  ___ \ |\   _ \  _   \|\   ___  \
#  \ \  \    \ \   __/|   \ \  \/  / | \   __/|\ \  \\\__\ \  \ \  \\ \  \
#   \ \  \    \ \  \_|/__  \ \    / / \ \  \_|/_\ \  \\|__| \  \ \  \\ \  \
#    \ \  \____\ \  \_|\ \  /     \/   \ \  \_|\ \ \  \    \ \  \ \  \\ \  \
#     \ \_______\ \_______\/  /\   \    \ \_______\ \__\    \ \__\ \__\\ \__\
#      \|_______|\|_______/__/ /\ __\    \|_______|\|__|     \|__|\|__| \|__|
#                         |__|/ \|__|
# 
#  The Lexemn Project
#  https://github.com/fontseca/lexemn
# 
#  Copyright (C) 2022 by Jeremy Fonseca <fonseca.dev@outlook.com>
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

cmake_minimum_required(VERSION 3.24)

set(NAME lexemn)
set(OUTDIR bin)
set(INCLUDEDIR ${CMAKE_SOURCE_DIR}/include)
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)

if (MSVC)
  set(
    CXXFLAGS
      /WX
      /W4
      /fpermissive
  )
else()
  set(
    CXXFLAGS
      -g
      -O3
      -Wall
      -Wextra
      -Werror
      -pedantic
      -fpermissive
      -fstrict-enums
      -fconcepts
      -fcoroutines
  )
endif()

list(
  APPEND
    CMAKE_MODULE_PATH
      ${CMAKE_SOURCE_DIR}/cmake
)

include(message_color)
include(message_log)

project(
  ${NAME}
  VERSION 0.1
  LANGUAGES C CXX
  DESCRIPTION "A high level programming language for interpreting and evaluating mathematical expressions."
  HOMEPAGE_URL "https://github.com/fontseca/lexemn"
)

message_project()

add_executable(
  ${PROJECT_NAME}
)

target_include_directories(
  ${PROJECT_NAME}
    PRIVATE
      ${INCLUDEDIR}
)

target_compile_options(
  ${PROJECT_NAME}
    PRIVATE
      ${CXXFLAGS}
)

target_compile_features(
  ${PROJECT_NAME}
    PRIVATE
      cxx_std_23
      c_std_23
)

target_sources(
  ${PROJECT_NAME}
    PRIVATE
      ${SRCDIR}/book.cc
      ${SRCDIR}/charset.cc
      ${SRCDIR}/lexer.cc
      ${SRCDIR}/utility.cc
      ${SRCDIR}/lexemn.cc
)

set_target_properties(
  ${PROJECT_NAME}
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTDIR}
)

if (UNIX)
  find_library(
    GNU_READLINE
    NAMES readline
    PATHS /usr/lib/ /usr/local/lib/
  )

  if (GNU_READLINE)
    message("-- GNU Readline found")
    target_link_libraries(
      ${PROJECT_NAME}
        PRIVATE
          ${GNU_READLINE}
    )
    else()
      message(FATAL_ERROR "GNU Readline not found")
  endif()
endif()

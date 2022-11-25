# 
#  Makefile for Lexemn
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

.SUFFIXES:
.SILENT:

SHELL      := /bin/bash

OUTDIR     := ./bin
INCLUDEDIR := ./include
SRCDIR     := ./src
OBJDIR     := ./obj

CXX        := g++
CXXFLAGS   := -g -std=c++23 -Wall -Wextra -Werror -pedantic
LIBS       := -lreadline

NAME       := $(addprefix $(OUTDIR)/, lexemn)
OBJS       := $(patsubst $(addprefix $(SRCDIR)/, %.cc),    \
                      $(addprefix $(OBJDIR)/, %.o),      \
							               $(wildcard $(addprefix $(SRCDIR)/, *.cc)))

ECHO_GDONE := echo -e "\e[1;92m(Done)\e[0m"
ECHO_RDONE := echo -e "\e[1;91m(Done)\e[0m"

.PHONY: all

all: question $(OBJDIR) $(NAME)

$(NAME): $(OBJS) | $(OUTDIR)
	echo -n "Linking into $(NAME)... "
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(LIBS)
	$(ECHO_GDONE)

$(OUTDIR):
	echo -n "Creating '$(OUTDIR)' directory... "
	mkdir $(OUTDIR)
	$(ECHO_GDONE)

$(OBJDIR):
	echo -n "Creating '$(OBJDIR)' directory... "
	mkdir $(OBJDIR)
	$(ECHO_GDONE)

$(addprefix $(OBJDIR)/, %.o): $(addprefix $(SRCDIR)/, %.cc)
	echo -n "Building '$<'... "
	$(CXX) -c -o $@ $(CXXFLAGS) $< -I $(INCLUDEDIR)
	$(ECHO_GDONE)

.PHONY: question

question:
	make --question $(NAME) &&   \
		echo "Everything up to date." || true

.PHONY: clean

# Clean object and binary files.
clean:
	if [ -e $(NAME) ]; then   \
		echo -n "Removing '$(NAME)' file... ";   \
		rm $(NAME);   \
		$(ECHO_RDONE);   \
	fi;   \
	for file in $(OBJS); do   \
		if [ -e $$file ]; then   \
			echo -n "Removing '$$file' file... ";   \
			rm $$file;   \
			$(ECHO_RDONE);   \
		fi;   \
	done;

.PHONY: cleandir

# Clean output and object directories.
cleandir:
	if [ -e $(OUTDIR) ]; then   \
		echo -n "Removing '$(OUTDIR)' directory... ";   \
		rmdir $(OUTDIR);   \
		$(ECHO_RDONE);   \
	fi;   \
	if [ -e $(OBJDIR) ]; then   \
		echo -n "Removing '$(OBJDIR)' directory... ";   \
		rmdir $(OBJDIR);   \
		$(ECHO_RDONE);   \
	fi;

# Lexemn Programming Language

This is the source code of the domain specific interpreted
programming language named Lexemn. It was named after the word lexeme
(which is the cornerstone in the heart of any natural or formal
language). The word Lexemn is pronounced smonething like <i>/'lɛkszæm/</i>.

![Lexemn Logo](./assets/lexemn-logo.png)

# Overview

Lexemn is a high level programming language for interpreting and
evaluating mathematical expressions. It will also feature the capability
to plot functions and solve basic equations.

Lexemn is an object oriented language with a libraries that support
basic mathematical abstractions such as matrixes and vectors. These
are the abstractions that lengthen the power of the language.

# Language Syntax

The syntax of Lexemn is pretty similar to mathematical expressions. It
supports Unicode graphemes to extend the language power by naming
familiar function or symbols such as π. Although it supports these
symbols, it is not an imposition to use them in systems that do not
have compatibility with Unicode.

## Function Definitions

![Function definitions](./assets/Screenshot%20from%202022-12-19%2011-50-35.png)

The code snippet above is a shorthand version for the following code:

![Function definitions](./assets/Screenshot%20from%202022-12-19%2011-52-40.png)

## Variable Definitions

![Variable definitions](./assets/Screenshot%20from%202022-12-19%2012-00-27.png)

![Variable definitions](./assets/Screenshot%20from%202022-12-19%2012-03-29.png)

## Defining a Class


![Defining a class](./assets/Screenshot%20from%202022-12-19%2012-05-40.png)

# Code Structure

Here is an explanation of the project's structure and what you can
find inside each directory:

- `src/`: Holds the core implementations for the language.

- `grammar/`: Holds the grammar for the Lexemn language.

- `include/`: Declarations for the abstractions used by the language.

- `doc/`: The official documentation of the language.

# Terminology

The interpreter uses an abstraction called **book** (or **Lexemn book**) whose purpose is to arrange a collection of **pages** (or **Lexemn pages**) during the lifecycle of the program. When the interpreter starts running, a new book created and it serves as a session that stores in memory a log of the user inserted expressions and computations.

A **page** abstracts a file with correct Lexemn instructions. When the interpreter is running in interactive mode, each expression entered by the user is treated as a single page and stored in a unique book.

When the interpreter is fed with source files, it creates a single book and transforms those files into pages arranged by that book.

The term page was chosen because it can be thought as a physical page filled with mathematical symbols and epxression in a book.

# Building

Lexemn is currently only available for GNU/Linux. To run it, you must first
install the dependencies in your system by running the `bootstrap.sh` script.
It will install GNU Readline, which is the only dependency by the moment.

## Build with GNU Make

After that, you will need GNU Make to compile the project. Once you have it, try running

`make`

and it will create a binary fila inside the `bin` directory and it will send all the object files to the `obj` directory.

To clean completely the project, run:

`make clean`

to remove the binary and the object files, and after that run

`make cleandir`

which is going to remove the `bin` and `obj` direcotries.

## Build with CMake

You can also build Lexemn with CMake. So, first make a directory called `build` at the root. Then,

  `$ cd build`,

and after that run:

 `$ cmake -DCMAKE_BUILD_TYPE=Debug -S .. -B .`,

and then:

 `$ cmake --build .`.

# Running

If you built Lexemn with GNU Make, you can run it by simply executing the binary file inside the `bin`
directory:

`./bin/lexemn`. Otherwise, there will be a `bin` directory inside the `build` folder.

If you do not specify any file with the extension `lxmn', the interpreter will run in interactive mode. If you pass one or more files, it will also run in interactive mode, but this time you will be able to access defined objects inside any of those files.

# Feature Requests

If you would like to add a feature either to the language, please try to get some sort of consensus that it is a worthwhile addition and open an issue with the lable `enchancement`. Please try to include enough description and justification about your new proposition.

# Reporting Bugs

If you found a bug or have any question, you can open an issue with the lables `bug` or `question` respectively.

# Achievements

- The first implementation of the lexer detects lexical erros.

![Lexical erros detected.](./assets/Screenshot%20from%202022-11-11%2012-15-28.png)

# To-Do

If you are interested in helping, here are some task you might want to accomplish:

- Complete the language grammar.
- Write the parser.

# Disclaimer

The development of Lexemn is currently regarded as an experiment that aims the study concepts and abstractions which are found in the construction of all (or some of) the stages of compilers and interpreters. There is absolutely no warranty that either the interpreter or programming language performs correcty what you might be expecting due to lack of completeness in its initital development phase.

The specific domain of the language is not guaranteed to  stay throughout its development because it is a subject that might be possibly reviewed in the future. You must also know that the programming language grammar is another subject that can easely be reviewed in the future.

Although Lexemn is regarded as an experimental programming language, once the first releases are available, it can be employed in the following cases:

- For students to write, test and maintain mathematical expressions in a safe places (e.g., GitHub).

- For computer programmers who are specialized in mathematics and want to try a new alternative to another similar programming language.

- For software that needs a new alternative to a similar programming language.

For further information, don't be shy and contact the maintainer of the language [by sending a mail to fonseca.dev@outlook.com](mailto:fonseca.dev@outlook.com).

# Jia
Jia Programming Language
## Background
As someone who has always been interested in inventing a new programming language,
I start this project, Jia, to make my idea come true.
Deeply inspired by LISP, Jia is a functional programming language,
but it does not need parenthesis, "(" or ")", which is the biggest difference between Jia and LISP.
Instead, Jia uses the capital letters and dot, ".", as we normally do in English,
to to indicate the start and end of a function.

For example, instead of 

    (add 1 2) 
in LISP, Jia's expression should be:

    Add 1 2.
Currently, I have realized integer sum (less than 10), recursion, and variable definition in the parser.
As I develop the parser, we will discuss more syntaxes and features of Jia in the future.
## Install
Clone or download this repository and compile the src file.
You can use the following command in terminals of Unix, macOS, Linux, and any other Unix-like systems.

    gcc -g -Wall -Werror -o jia jia.c
## Usage
This program is a parser of Jia programming language.
## Contributing
PRs accepted.
## License
GNU-3.0 © Jiacheng Huang
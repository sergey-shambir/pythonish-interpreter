#!/usr/bin/env bash

# -q (Quite) to skip report file Grammar.out
# -l to skip #line directives in generated code
# -s to print brief parser statistics
lemon -q -s -l Grammar.lemon
rm -f Grammar.cpp
mv Grammar.c Grammar.cpp

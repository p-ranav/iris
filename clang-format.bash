#!/usr/bin/env bash
find ./include ./samples -type f \( -iname \*.cpp -o -iname \*.hpp \) | xargs clang-format -style=llvm -i

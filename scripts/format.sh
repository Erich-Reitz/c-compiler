#!/bin/bash


find src/  -iname *.cpp | xargs clang-format-16 -i
find include/  -iname *.hpp | xargs clang-format-16 -i

clang-format-16 -i test_runner.cc
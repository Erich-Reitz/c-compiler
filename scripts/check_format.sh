#!/bin/bash

clang-format-16 -n -Werror --dry-run src/*.cpp include/*.hpp
clang-format-16 -n -Werror --dry-run test_runner.cc
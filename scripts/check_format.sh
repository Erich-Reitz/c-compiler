#!/bin/bash

clang-format -n -Werror --dry-run src/*.cpp include/*.hpp

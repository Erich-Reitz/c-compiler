#!/bin/bash

clang-format -n -Werror --dry-run src/*.cpp include/*.hpp

clang-format -n -Werror --dry-run test_runner.cc
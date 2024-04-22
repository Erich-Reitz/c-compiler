#!/bin/bash

clang-format-16 -i src/*.cpp include/*.hpp

clang-format-16 -i test_runner.cc
#!/bin/bash

clang-format -i src/*.cpp include/*.hpp

clang-format -i test_runner.cc
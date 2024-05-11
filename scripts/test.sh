#!/bin/bash

mkdir -p tmp

cmake -D CMAKE_CXX_COMPILER=g++-13 -S . -B build
cmake --build build

# Check for build success
if [ $? -eq 0 ]; then
    echo "Build succeeded."
else
    echo "Build failed."
    exit 1
fi

# Run tests with filter
if [ -n "$1" ]; then
    ./build/bin/test_runner --gtest_filter="*$1*"
else
    ./build/bin/test_runner
fi

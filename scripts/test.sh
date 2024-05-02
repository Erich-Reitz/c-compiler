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

# Run tests
./build/bin/test_runner
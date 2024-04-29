#!/bin/bash


cmake -D CMAKE_CXX_COMPILER=g++-13 -S . -B build
ulimit -c unlimited
cmake --build build


# Check for build success
if [ $? -eq 0 ]; then
    echo "Build succeeded."
else
    echo "Build failed."
    exit 1
fi

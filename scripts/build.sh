#!/bin/bash

cd "$(dirname "$0")"/../build

cmake ..

cmake --build . 

# Check for build success
if [ $? -eq 0 ]; then
    echo "Build succeeded."
else
    echo "Build failed."
    exit 1
fi

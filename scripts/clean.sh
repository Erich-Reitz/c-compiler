#!/bin/bash

if [ -d "build" ]; then
    rm -r build
fi

if [ -d "tmp" ]; then
    rm -r tmp
fi
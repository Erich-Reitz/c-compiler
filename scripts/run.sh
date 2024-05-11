#!/bin/bash

# Variables for paths

asm_path="test.asm"
object_path="test.o"
binary_path="test.out"

nasm -f elf64 -o $object_path $asm_path
if [ $? -ne 0 ]; then
    echo "NASM compilation failed."
    exit 1
fi

gcc -o $binary_path $object_path -nostartfiles -lc -fPIE
if [ $? -ne 0 ]; then
    echo "GCC linking failed."
    exit 1
fi

echo "Compilation and linking successful. Binary created at $binary_path"

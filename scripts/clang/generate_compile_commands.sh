#!/bin/bash
# Script to generate compile_commands.json using bear
# Returns 0 if successful, 1 if failed
set -e
echo "Generating compile_commands.json..."
if ! command -v bear >/dev/null 2>&1; then
    echo "bear not found. Please install it first:"
    echo "sudo apt install bear"
    exit 1
fi
if [ ! -f "Makefile" ]; then
    echo "Makefile not found in current directory"
    exit 1
fi
echo "Running bear to capture compilation commands..."
bear -- make clean && bear -- make all -j16
if [ -f "compile_commands.json" ]; then
    echo "compile_commands.json generated successfully"
    echo "File size: $(wc -l < compile_commands.json) entries"
    exit 0
else
    echo "Failed to generate compile_commands.json"
    exit 1
fi 
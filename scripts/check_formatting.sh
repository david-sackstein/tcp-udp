#!/bin/bash

# Script to check clang-format formatting without modifying files
# Returns 0 if formatting is correct, 1 if formatting issues found

set -e

echo "Checking code formatting..."

# Check if clang-format is available
if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found. Skipping format check."
    exit 0
fi

# Check if .clang-format file exists
if [ ! -f ".clang-format" ]; then
    echo "Warning: .clang-format configuration file not found"
fi

# Check formatting of all C++ source files
echo "Checking C++ source files..."
if find src include -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror; then
    echo "Format check passed."
    exit 0
else
    echo "Format check failed. Run 'make format' to fix formatting issues."
    exit 1
fi 
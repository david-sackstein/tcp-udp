#!/bin/bash

# Script to apply clang-format to all source files
# Returns 0 if successful, 1 if failed

set -e

echo "Applying clang-format to source files..."

# Check if clang-format is available
if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found. Skipping formatting."
    exit 0
fi

# Check if .clang-format file exists
if [ ! -f ".clang-format" ]; then
    echo "Warning: .clang-format configuration file not found"
fi

# Find and format all C++ source files
echo "Formatting C++ source files..."
find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i

echo "Formatting completed successfully." 
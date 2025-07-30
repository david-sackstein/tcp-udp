#!/bin/bash
# Script to run clang-tidy using compile_commands.json
# Returns 0 if successful, 1 if failed
set -e
echo "Running clang-tidy with compile_commands.json..."
if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy not found. Skipping static analysis."
    exit 0
fi
if [ ! -f "compile_commands.json" ]; then
    echo "compile_commands.json not found. Please generate it first:"
    echo "./scripts/clang/generate_compile_commands.sh"
    exit 1
fi
echo "Running clang-tidy on C++ source files..."
# Ensure we're in the right directory and use xargs -n 1 to process one file at a time
cd "$(dirname "$0")/../.."  # Go to project root
find src include -name '*.cpp' -o -name '*.h' | xargs -n 1 clang-tidy -header-filter=".*"
echo "clang-tidy analysis completed." 
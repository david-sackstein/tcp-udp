#!/bin/bash
# Script to run clang-tidy with automatic fixes using compile_commands.json
# Returns 0 if successful, 1 if failed
set -e
echo "Running clang-tidy with automatic fixes using compile_commands.json..."
if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy not found. Skipping static analysis."
    exit 0
fi
if [ ! -f "compile_commands.json" ]; then
    echo "compile_commands.json not found. Please generate it first:"
    echo "./scripts/clang/generate_compile_commands.sh"
    exit 1
fi
echo "Running clang-tidy with automatic fixes on C++ source files..."
find src include -name '*.cpp' -o -name '*.h' | xargs clang-tidy --fix
echo "clang-tidy analysis with fixes completed." 
#!/bin/bash
# run_clang_tidy.sh
# Run clang-tidy with or without automatic fixes on all C++ source/header files using compile_commands.json

set -euo pipefail

# Check clang-tidy availability
if ! command -v clang-tidy >/dev/null 2>&1; then
  echo "clang-tidy not found. Please install it before running this script."
  exit 1
fi

# Check compile_commands.json presence
if [ ! -f "compile_commands.json" ]; then
  echo "Error: compile_commands.json not found in $(pwd)."
  echo "Generate it first, e.g.:"
  echo "  bear -- make"
  echo "  or ./scripts/clang/generate_compile_commands.sh"
  exit 1
fi

# Usage helper
usage() {
  echo "Usage: $0 [--fix|--no-fix]"
  echo "  --fix     Apply automatic fixes where possible"
  echo "  --no-fix  Only run checks without applying fixes"
  exit 1
}

# Parse argument
if [ $# -eq 0 ]; then
  usage
fi

FIX_FLAG=""
case "$1" in
  --fix)
    FIX_FLAG="--fix"
    ;;
  --no-fix)
    FIX_FLAG=""
    ;;
  *)
    usage
    ;;
esac
shift

echo "Running clang-tidy ${FIX_FLAG:+ ' automatic fixes'}..."

# Find all .cpp and .h files in src and include directories
# Note: grouping (-name '*.cpp' -o -name '*.h') is important
FILES=$(find src include \( -name '*.cpp' -o -name '*.h' \))

if [ -z "$FILES" ]; then
  echo "No C++ source or header files found under src/ or include/"
  exit 0
fi

# Run clang-tidy on each file using parallelism of 16 if available
echo "$FILES" | xargs -P 16 -n 1 clang-tidy -p=. -extra-arg=-fno-color-diagnostics $FIX_FLAG -header-filter=".*"

echo "clang-tidy run completed."

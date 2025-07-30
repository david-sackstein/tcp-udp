#!/bin/bash
# clang_format.sh - Check or fix code formatting using clang-format
# Usage:
#   ./clang_format.sh --check   # Check formatting, exit non-zero if issues
#   ./clang_format.sh --fix     # Apply formatting fixes in-place

set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found. Skipping formatting."
    exit 0
fi

if [ ! -f ".clang-format" ]; then
    echo "Warning: .clang-format configuration file not found"
fi

if [ $# -ne 1 ]; then
    echo "Usage: $0 --check|--fix"
    exit 1
fi

MODE="$1"
FILES=$(find src include \( -name '*.cpp' -o -name '*.h' \))

if [ -z "$FILES" ]; then
    echo "No C++ source or header files found under src/ or include/"
    exit 0
fi

case "$MODE" in
    --check)
        echo "Checking code formatting..."
        if echo "$FILES" | xargs clang-format --dry-run --Werror; then
            echo "Format check passed."
            exit 0
        else
            echo "Format check failed. Run '$0 --fix' to fix formatting issues."
            exit 1
        fi
        ;;
    --fix)
        echo "Applying clang-format fixes..."
        echo "$FILES" | xargs clang-format -i
        echo "Formatting completed successfully."
        ;;
    *)
        echo "Invalid option: $MODE"
        echo "Usage: $0 --check|--fix"
        exit 1
        ;;
esac

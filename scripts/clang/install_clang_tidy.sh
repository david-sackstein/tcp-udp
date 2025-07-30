#!/bin/bash

# Script to install clang-tidy
# Returns 0 if successful, 1 if failed

set -e

echo "Installing clang-tidy..."

# Check if clang-tidy is already available
if command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy is already installed:"
    clang-tidy --version
    exit 0
fi

# Try to install clang-tidy
if command -v apt >/dev/null 2>&1; then
    echo "Installing via apt..."
    sudo apt update && sudo apt install -y clang-tidy
    
    if [ -f /usr/bin/clang-tidy ]; then
        echo "clang-tidy installed successfully"
        clang-tidy --version
        exit 0
    else
        echo "Failed to install clang-tidy via apt"
        exit 1
    fi
else
    echo "apt not found. Please install clang-tidy manually."
    exit 1
fi 
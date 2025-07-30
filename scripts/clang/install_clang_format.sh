#!/bin/bash

# Script to install clang-format-17
# Returns 0 if successful, 1 if failed

set -e

echo "Installing clang-format-17..."

# Check if clang-format is already available
if command -v clang-format >/dev/null 2>&1; then
    echo "clang-format is already installed:"
    clang-format --version
    exit 0
fi

# Try to install clang-format-17
if command -v apt >/dev/null 2>&1; then
    echo "Installing via apt..."
    sudo apt update && sudo apt install -y clang-format-17
    
    # Create symlink for easier access
    if [ -f /usr/bin/clang-format-17 ]; then
        sudo ln -sf /usr/bin/clang-format-17 /usr/local/bin/clang-format
        echo "clang-format-17 installed successfully"
        clang-format --version
        exit 0
    else
        echo "Failed to install clang-format-17 via apt"
        exit 1
    fi
else
    echo "apt not found. Please install clang-format manually."
    exit 1
fi 
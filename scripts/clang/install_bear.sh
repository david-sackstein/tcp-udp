#!/bin/bash
# Script to install bear for generating compile_commands.json
# Returns 0 if successful, 1 if failed
set -e
echo "Installing bear..."
if command -v bear >/dev/null 2>&1; then
    echo "bear is already installed:"
    bear --version
    exit 0
fi
if command -v apt >/dev/null 2>&1; then
    echo "Installing via apt..."
    sudo apt update && sudo apt install -y bear
    if [ -f /usr/bin/bear ]; then
        echo "bear installed successfully"
        bear --version
        exit 0
    else
        echo "Failed to install bear via apt"
        exit 1
    fi
else
    echo "apt not found. Please install bear manually."
    exit 1
fi 
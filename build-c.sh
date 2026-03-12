#!/usr/bin/env bash
set -euo pipefail

# Build the C version of vibe-journal
# Usage: ./build-c.sh

echo "Building vibe-journal (C version)..."
echo "Use 'make cpp' if you want to build the C++/gtkmm variant (see README)"

# Check for required packages
if ! pkg-config --exists gtk+-3.0; then
    echo "GTK+ 3.0 development libraries not found."
    echo "On Ubuntu/Debian, install with:"
    echo "  sudo apt update && sudo apt install libgtk-3-dev libsqlite3-dev pkg-config"
    exit 1
fi

if ! pkg-config --exists sqlite3; then
    echo "SQLite3 development libraries not found."
    echo "On Ubuntu/Debian, install with:"
    echo "  sudo apt update && sudo apt install libsqlite3-dev"
    exit 1
fi

# Build the executable
make clean
make

echo "Built: ./vibe-journal"
echo "Run with: ./vibe-journal"
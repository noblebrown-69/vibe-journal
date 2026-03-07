#!/usr/bin/env bash
set -euo pipefail

# Build a one-file executable for Linux (Ubuntu-compatible).
# Usage: ./build.sh

pkill -f main.py || true

if ! command -v pipx >/dev/null 2>&1; then
  echo "pipx not found. On Ubuntu install it with:"
  echo "  sudo apt update && sudo apt install pipx"
  exit 1
fi

pipx install pyinstaller

PYI_BIN=${HOME}/.local/bin/pyinstaller
if [ ! -x "$PYI_BIN" ]; then
  PYI_BIN="$(command -v pyinstaller || true)"
fi

if [ -z "$PYI_BIN" ]; then
  echo "PyInstaller not found after install; check your PATH or install pyinstaller manually." >&2
  exit 2
fi

"$PYI_BIN" --onefile --name vibe-journal main.py

echo "Built: dist/vibe-journal"

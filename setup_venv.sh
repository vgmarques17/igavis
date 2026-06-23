#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="$ROOT_DIR/igavis_env"
PYTHON_CMD="python3"

if ! command -v "$PYTHON_CMD" >/dev/null 2>&1; then
  echo "Error: python3 is required but not installed." >&2
  exit 1
fi

echo "Creating virtual environment at: $VENV_DIR"
$PYTHON_CMD -m venv "$VENV_DIR"
"$VENV_DIR/bin/python" -m pip install --upgrade pip setuptools wheel

echo "Installing Python dependencies from requirements.txt"
"$VENV_DIR/bin/python" -m pip install -r "$ROOT_DIR/requirements.txt"

echo "Installing igavis package in editable mode"
cd "$ROOT_DIR"
"$VENV_DIR/bin/python" -m pip install -e .

echo "Building IGB helper library"
cd "$ROOT_DIR/src/igavis/io/igb"
if command -v gcc >/dev/null 2>&1; then
  gcc -fPIC -shared -o libigb.so header.c igbwrapper.c
  echo "Built $ROOT_DIR/src/igavis/io/igb/libigb.so"
else
  echo "Warning: gcc not found. Skipping IGB helper library build."
  echo "Install gcc and rerun ./setup_venv.sh to build libigb.so."
fi

echo "Setup complete. Activate the virtual environment with: source \"$VENV_DIR/bin/activate\""
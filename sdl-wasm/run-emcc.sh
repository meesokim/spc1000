#!/bin/bash
# run-emcc.sh - Run emcc, installing it via emsdk if not present.

set -e

# Target directory for emsdk (inside the workspace)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EMSDK_DIR="${SCRIPT_DIR}/emsdk"

# Check if emcc is already in PATH
if command -v emcc >/dev/null 2>&1; then
    exec emcc "$@"
fi

# Check if emsdk is already installed locally in the project
if [ ! -d "$EMSDK_DIR" ]; then
    echo "emcc not found in PATH. Downloading and installing Emscripten SDK..."
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
    
    # Run installation steps
    (
        cd "$EMSDK_DIR"
        ./emsdk install latest
        ./emsdk activate latest
    )
fi

# Source the emsdk environment variables
if [ -f "$EMSDK_DIR/emsdk_env.sh" ]; then
    # shellcheck source=/dev/null
    source "$EMSDK_DIR/emsdk_env.sh" >/dev/null 2>&1
else
    echo "Error: emsdk_env.sh not found in $EMSDK_DIR" >&2
    exit 1
fi

# Run emcc with the arguments passed to this script
exec emcc "$@"

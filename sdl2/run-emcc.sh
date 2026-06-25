#!/bin/bash
# run-emcc.sh - Run emcc/em++ command, installing it via emsdk if not present.

set -e

# Target directory for emsdk (default to ~/emsdk to match Makefile)
EMSDK_DIR="${HOME}/emsdk"

# The command to execute (emcc or em++)
CMD="$1"
shift

# Check if the command is already in PATH
if command -v "$CMD" >/dev/null 2>&1; then
    exec "$CMD" "$@"
fi

# Check if emsdk is already installed
if [ ! -d "$EMSDK_DIR" ]; then
    echo "Command '$CMD' not found in PATH. Installing Emscripten SDK at $EMSDK_DIR..."
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

# Run the command with the arguments passed to this script
exec "$CMD" "$@"

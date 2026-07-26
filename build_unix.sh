#!/bin/bash
# Build script for Vircon32 Web Emulator on Linux/Mac
# Requires Emscripten SDK to be installed and activated

set -e

echo "========================================"
echo "Vircon32 Web Emulator Build Script"
echo "========================================"
echo ""

# Check if emcmake is available
if ! command -v emcmake &> /dev/null; then
    echo "ERROR: Emscripten not found!"
    echo "Please install Emscripten SDK and activate it using source emsdk_env.sh"
    echo "Download from: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

echo "Emscripten found: $(emcmake --version)"
echo ""

# Set paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INSTALL_DIR="$SCRIPT_DIR/output"

echo "Source directory: $SCRIPT_DIR"
echo "Build directory: $BUILD_DIR"
echo "Output directory: $INSTALL_DIR"
echo ""

# Clean previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

if [ -d "$INSTALL_DIR" ]; then
    echo "Cleaning previous output..."
    rm -rf "$INSTALL_DIR"
fi

# Create build directory
echo "Creating build directory..."
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cd "$BUILD_DIR"
emcmake cmake -DCMAKE_BUILD_TYPE=Release "$SCRIPT_DIR"

# Build
echo "Building..."
emmake make -j$(nproc)

# Create output directory
echo "Creating output directory..."
mkdir -p "$INSTALL_DIR"

# Copy files to output
echo "Copying files to output..."
cp Vircon32Web.html "$INSTALL_DIR/"
cp Vircon32Web.js "$INSTALL_DIR/"
cp Vircon32Web.wasm "$INSTALL_DIR/"
if [ -f "Vircon32Web.data" ]; then
    cp Vircon32Web.data "$INSTALL_DIR/"
fi
cp web_interface.js "$INSTALL_DIR/"
if [ -f "$SCRIPT_DIR/logo.png" ]; then
    cp "$SCRIPT_DIR/logo.png" "$INSTALL_DIR/"
fi

cd "$SCRIPT_DIR"

echo ""
echo "========================================"
echo "Build completed successfully!"
echo "========================================"
echo "Output files are in: $INSTALL_DIR"
echo ""
echo "To test the emulator:"
echo "1. Start a local web server in the output directory"
echo "   Example: python3 -m http.server 8000"
echo "2. Open http://localhost:8000 in your browser"
echo ""

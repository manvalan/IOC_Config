#!/bin/bash

# IOC_Config Build Script
# This script handles the building of the IOC_Config library

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="build"
BUILD_EXAMPLES="${BUILD_EXAMPLES:-ON}"
BUILD_TESTS="${BUILD_TESTS:-ON}"

echo -e "${GREEN}IOC_Config Library Build Script${NC}"
echo "=================================="
echo "Build Type: $BUILD_TYPE"
echo "Build Examples: $BUILD_EXAMPLES"
echo "Build Tests: $BUILD_TESTS"
echo ""

# Clean build directory if requested
if [ "$1" == "clean" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cd "$BUILD_DIR"
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_EXAMPLES="$BUILD_EXAMPLES" \
    -DBUILD_TESTS="$BUILD_TESTS"

# Build
echo -e "${YELLOW}Building...${NC}"
cmake --build . --config "$BUILD_TYPE"

# Run tests if building tests
if [ "$BUILD_TESTS" = "ON" ]; then
    echo -e "${YELLOW}Running tests...${NC}"
    ctest --output-on-failure
    echo -e "${GREEN}Tests passed!${NC}"
fi

cd ..

# Display build information
echo ""
echo -e "${GREEN}Build completed successfully!${NC}"
echo ""
echo "Library files:"
echo "  Static:  $BUILD_DIR/libioc_config.a"
if [ "$(uname)" = "Darwin" ]; then
    echo "  Shared:  $BUILD_DIR/libioc_config.dylib"
else
    echo "  Shared:  $BUILD_DIR/libioc_config.so"
fi

if [ "$BUILD_EXAMPLES" = "ON" ]; then
    echo ""
    echo "Example programs:"
    echo "  $BUILD_DIR/examples/example_basic"
    echo "  $BUILD_DIR/examples/example_converter"
    echo "  $BUILD_DIR/examples/example_config_builder"
fi

echo ""
echo "To install the library:"
echo "  cd $BUILD_DIR"
echo "  sudo cmake --install ."
echo ""
echo "To use in your project, link with:"
echo "  -lioc_config -I/usr/local/include"
echo ""

#!/bin/bash

# SmartSpectra API Wrapper Build Script
# This script builds the SmartSpectra API wrapper server

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
PARALLEL_JOBS=${PARALLEL_JOBS:-$(sysctl -n hw.ncpu)}

echo -e "${BLUE}SmartSpectra API Wrapper Build Script${NC}"
echo "======================================"
echo "Build type: $BUILD_TYPE"
echo "Build directory: $BUILD_DIR"
echo "Install prefix: $INSTALL_PREFIX"
echo "Parallel jobs: $PARALLEL_JOBS"
echo ""

# Function to print status messages
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the correct directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the api/ directory."
    exit 1
fi

# Check for required dependencies
print_status "Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake is required but not installed."
    exit 1
fi

# Check for pkg-config
if ! command -v pkg-config &> /dev/null; then
    print_error "pkg-config is required but not installed."
    exit 1
fi

# Check for OpenCV
if ! pkg-config --exists opencv4; then
    if ! pkg-config --exists opencv; then
        print_error "OpenCV is required but not found."
        print_error "Please install OpenCV development packages."
        exit 1
    fi
fi

# Check for glog
if ! pkg-config --exists libglog; then
    print_warning "glog not found via pkg-config. Build may fail if not available."
fi

print_status "Dependencies check completed."

# Create build directory
print_status "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
print_status "Building (using $PARALLEL_JOBS parallel jobs)..."
cmake --build . --parallel "$PARALLEL_JOBS"

# Check if build was successful
if [ $? -eq 0 ]; then
    print_status "Build completed successfully!"
    echo ""
    echo "Executable location: $BUILD_DIR/smartspectra_api_server"
    echo ""
    echo "To run the server:"
    echo "  export SMARTSPECTRA_API_KEY=\"your_api_key_here\""
    echo "  ./$BUILD_DIR/smartspectra_api_server"
    echo ""
    echo "To install system-wide:"
    echo "  sudo cmake --install $BUILD_DIR"
else
    print_error "Build failed!"
    exit 1
fi

# Optional: Run tests if available
if [ -f "test/CMakeLists.txt" ] || [ -d "tests" ]; then
    echo ""
    read -p "Run tests? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Running tests..."
        ctest --output-on-failure
    fi
fi

print_status "Build script completed successfully!"

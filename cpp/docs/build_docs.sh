#!/usr/bin/env bash
set -e

# Build the Doxygen documentation

BUILD_DIR="cmake-build-release"
mkdir -p "$BUILD_DIR"

cmake -S . -B "$BUILD_DIR" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCS=ON
cmake --build "$BUILD_DIR" --target doxygen

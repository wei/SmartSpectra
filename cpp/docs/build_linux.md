# Building & Packaging SmartSpectra C++ SDK on Ubuntu / Linux Mint

This document describes how to build the SmartSpectra SDK from source on Ubuntu 22.04 or Mint 21. Note that on these systems, since installable packages are available, this step is _not_ required if you just want to develop a standalone C++ application using the SDK (see [Supported Systems & Architectures](../README.md#supported-systems--architectures) for more details on supported platforms and package availability). However, building from source allows you to build the SDK examples, understand how SDK source code works, and even contribute your own desired features to the SDK.

All commands below demonstrate how to build from a terminal (e.g., Ctrl+Alt+T by default on Ubuntu or Activities (Super key) → Terminal). We encourage you to use IDEs, editors, and other GUI tools that wrap around any of the given terminal commands at your discretion.

## Table of Contents
- [Installing Build Tools](#installing-build-tools)
- [Setting Up Build Dependencies](#setting-up-build-dependencies)
- [Building for the Host System](#building-for-the-host-system)
- [CMake Options](#cmake-options)
- [Cross-compiling for Linux Arm64](#cross-compiling-for-linux-arm64)


## Installing Build Tools
1. Install `git` unless it's not already installed (e.g., `sudo apt install git` on Ubuntu).
2. CMake 3.27.0 or newer is required.
    - **Ubuntu 22.04** comes with a default cmake version that is too old. If your system-default cmake is older (run `cmake --version` to check this), follow the [directions from Kitware](https://apt.kitware.com/) (publishers of CMake) to get it set up if your cmake version is below the requirement.
3. Ninja 1.10 (or newer) or `make` will work to build the SDK on Linux.
    - Run `sudo apt install ninja-build` to install Ninja. 
    - To make sure `make` is installed, run `sudo apt-install build-essential`.

## Setting Up Build Dependencies

Install or build the Physiology Edge library. 

*Side Note*: the **Physiology Edge** library, packaged as `libphysiologyedge-dev`, is the C++ backend that *all* SmartSpectra SDKs rely on to provide a uniform experience across all languages and platforms. This is where some of the computation and communication with Physiology Core API on the cloud takes place. Meanwhile,
 the **SmartSpectra C++ SDK**, packages as `libsmartspectra-dev`, is the C++ open-source library that wraps around the Physiology Edge library to provide a convenient, simple API for C++ developers.

- *Ubuntu 22.04 / Mint 21*: you can install via Debian package [from the Presage Debian Repository](../README.md#setting-up-the-presage-debian-repository). *Note:* be sure to follow the below directions to only grab what's needed for the build, so that the SDK itself does not get installed from the repository.

1. Update the `apt` database:
    ```bash
    sudo apt update
    ```
2. Install (or upgrade) the Physiology Edge library:
    ```bash
    sudo apt install libphysiologyedge-dev
    ```

- *Other linux systems*: (partners-only) contact support (<[support@presagetech.com](mailto:support@presagetech.com)>) to obtain a source package and build instructions.

## Building for the Host System
1. Clone this repository.
    ```shell
    git clone https://github.com/Presage-Security/SmartSpectra/
     ```
2. Navigate to the C++ SDK root within the cloned repository, e.g.:
    ```sheel
    cd SmartSpectra/cpp
    ```
3. Build the SDK with examples using either Ninja or Make.
    - Using Ninja:
        ```shell
        mkdir build
        cd build
        cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON ..
        ninja
        ```
    - Using Make (on macOS, substitute `$(sysctl -n hw.ncpu)` for `$(nproc)` below):
        ```shell
        mkdir build
        cd build
        cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON ..
        make -j$(nproc)
        ```
## CMake Options

Adjust CMake build flags in the above `cmake` calls as needed.
- If you don't want to build the examples, change `-DBUILD_SAMPLES=ON` to `-DBUILD_SAMPLES=OFF`.
- For a debug build, change `-DCMAKE_BUILD_TYPE=Release` to `-DCMAKE_BUILD_TYPE=Debug`.
- The CMake GUI application (`sudo apt install cmake-gui`) is the graphical counterpart of the command-line `cmake` tool that will display all available CMake options when provided the source (e.g., `SmartSpectra/cpp`) and build (e.g., `SmartSpectra/cpp/build`) directories.

### Cross-compiling for Linux Arm64

You can cross-compile the SDK and examples for the Linux `arm64` architecture when building on an `amd64` Linux machine.

1. Install the required cross-compilation tools:
    ```bash
    sudo apt install crossbuild-essential-arm64 gcc-arm-linux-gnueabi binutils-arm-linux-gnueabi
    ```

2. Specify the provided toolchain file to CMake. E.g., from within `SmartSpectra/cpp` (assuming `SmartSpectra` is the root of the repository), run :
    ```bash
    mkdir build-arm64
    cd build-arm64
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON -DCMAKE_TOOLCHAIN_FILE=../toolchains/linux_arm64_toolchain.cmake ..
    make -j$(nproc)
    ```
## Packaging

Use `cpack` to generate the desired package in the build folder. E.g., from within `SmartSpectra/cpp` (assuming `SmartSpectra` is the root of the repository), run:

```bash
cd build
cpack -G DEB
```

For the above example, if there are no errors in the output, you should see a `*.deb` package file appear in the `build` directory.

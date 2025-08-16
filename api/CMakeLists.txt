cmake_minimum_required(VERSION 3.16)
project(SmartSpectraAPIWrapper VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# Find glog
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLOG REQUIRED libglog)

# Find protobuf
find_package(Protobuf REQUIRED)

# Find absl
find_package(absl REQUIRED)

# Include SmartSpectra SDK
# Assuming SmartSpectra SDK is built and installed in the parent cpp directory
set(SMARTSPECTRA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../cpp")
set(SMARTSPECTRA_INCLUDE_DIR "${SMARTSPECTRA_ROOT}")
set(SMARTSPECTRA_LIB_DIR "${SMARTSPECTRA_ROOT}/build")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SMARTSPECTRA_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${GLOG_INCLUDE_DIRS}
)

# Add Crow C++ framework using FetchContent
include(FetchContent)
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.8.0
)
FetchContent_MakeAvailable(crow)

# Add nlohmann/json for JSON handling
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Add UUID library for session ID generation
FetchContent_Declare(
    stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
    GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(stduuid)

# Source files
set(API_SOURCES
    src/main.cpp
    src/FrameBufferVideoSource.cpp
)

# Create the executable
add_executable(smartspectra_api_server ${API_SOURCES})

# Link libraries
target_link_libraries(smartspectra_api_server
    PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
    stduuid
    ${OpenCV_LIBS}
    ${GLOG_LIBRARIES}
    Threads::Threads
    ${Protobuf_LIBRARIES}
)

# Link SmartSpectra SDK libraries
# Note: These library names may need to be adjusted based on actual SmartSpectra build
target_link_libraries(smartspectra_api_server
    PRIVATE
    # Add SmartSpectra libraries here when available
    # For now, we'll assume they're available in the system
)

# Compiler definitions
target_compile_definitions(smartspectra_api_server
    PRIVATE
    CROW_ENABLE_SSL=0  # Disable SSL for simplicity, can be enabled later
    CROW_ENABLE_COMPRESSION=1
)

# Include SmartSpectra headers
target_include_directories(smartspectra_api_server
    PRIVATE
    ${SMARTSPECTRA_INCLUDE_DIR}
)

# Installation
install(TARGETS smartspectra_api_server
    RUNTIME DESTINATION bin
)

# Copy sample files
install(DIRECTORY samples/
    DESTINATION share/smartspectra-api/samples
    FILES_MATCHING PATTERN "*.html" PATTERN "*.js"
)

# Copy documentation
install(FILES docs/openapi.yaml
    DESTINATION share/smartspectra-api/docs
    OPTIONAL
)

# Print configuration summary
message(STATUS "SmartSpectra API Wrapper Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenCV version: ${OpenCV_VERSION}")
message(STATUS "  SmartSpectra root: ${SMARTSPECTRA_ROOT}")

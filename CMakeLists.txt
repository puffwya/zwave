cmake_minimum_required(VERSION 3.16)
project(RaycasterGame LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Organize build files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include src folder
add_subdirectory(src)

# Find SDL2 using Homebrew
find_package(SDL2 REQUIRED)

# If CMake cannot find SDL2, help manually
if (NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found. Make sure you installed it with Homebrew: brew install sdl2")
endif()


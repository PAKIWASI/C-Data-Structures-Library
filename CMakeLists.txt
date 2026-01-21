cmake_minimum_required(VERSION 3.20)
project(hashmap LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Debug configuration with sanitizers
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address -fsanitize=undefined")

# Release configuration
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")


file(GLOB SRC_FILES "src/*.c")

add_executable(main ${SRC_FILES})
target_include_directories(main PRIVATE include tests)

# Debug build (default) - with ASan, no optimizations, full debug info
#cmake -B build -G Ninja
#ninja -C build


# Release build - full optimizations, no debug info, no ASan
#cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
#ninja -C build

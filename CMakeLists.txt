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

# Debug configuration
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fsanitize=address -fsanitize=leak")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address -fsanitize=leak")

# Release configuration
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

# RelWithDebInfo configuration (optimized but with debug symbols)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "")

file(GLOB SRC_FILES "src/*.c")
file(GLOB HEADER_FILES "include/*.h")

add_executable(main ${SRC_FILES} ${HEADER_FILES})
target_include_directories(main PRIVATE include)

# Debug build (default) - with ASan, no optimizations, full debug info
#cmake -B build -G Ninja
#ninja -C build

# Release build - full optimizations, no debug info, no ASan
#cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
#ninja -C build

# RelWithDebInfo - optimized but debuggable (good middle ground)
#cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
#ninja -C build




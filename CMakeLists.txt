cmake_minimum_required(VERSION 3.20)

project(hashmap LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SRC_FILES "src/*.c")
file(GLOB HEADER_FILES "include/*.h")

add_executable(main ${SRC_FILES} ${HEADER_FILES})

target_include_directories(main PRIVATE include)


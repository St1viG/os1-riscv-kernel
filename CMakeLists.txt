cmake_minimum_required(VERSION 3.16)
project(os1 C CXX ASM)

# This CMakeLists is for CLion indexing only. The real build is the RISC-V
# cross-compile driven by ./Makefile inside Docker (see the `kernel` target
# below). Do not try to run the auto-generated CMake executable target —
# it will not link on the host toolchain. Use the `kernel` run config.

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Match the include search the Makefile gives the cross-compiler. The project
# root is on the path because sources do `#include "./lib/hw.h"` and
# `#include "../h/MemoryAllocator.hpp"`.
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/h
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/src
)

add_compile_definitions(DEBUG_PRINT=0)

file(GLOB_RECURSE OS1_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.S"
    "${CMAKE_SOURCE_DIR}/h/*.hpp"
    "${CMAKE_SOURCE_DIR}/h/*.h"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.hpp"
    "${CMAKE_SOURCE_DIR}/tests/*.S"
    "${CMAKE_SOURCE_DIR}/lib/*.h"
)

# Indexing-only target. Excluded from `all` so CLion's hammer-button won't
# accidentally try to link it with the host toolchain.
add_executable(os1_indexing EXCLUDE_FROM_ALL ${OS1_SOURCES})

# ---- Real build: shells out to the Docker container ----
set(DOCKER_IMAGE os1-build)
set(DOCKER_RUN docker run --rm -v ${CMAKE_SOURCE_DIR}:/kernel ${DOCKER_IMAGE})

add_custom_target(docker_image
    COMMAND docker build -t ${DOCKER_IMAGE} ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building Docker image ${DOCKER_IMAGE}"
    USES_TERMINAL
)

add_custom_target(kernel
    COMMAND ${DOCKER_RUN} make
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS docker_image
    COMMENT "Building kernel via Docker"
    USES_TERMINAL
)

add_custom_target(kernel_clean
    COMMAND ${DOCKER_RUN} make clean
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL
)

add_custom_target(kernel_qemu
    COMMAND docker run --rm -it -v ${CMAKE_SOURCE_DIR}:/kernel ${DOCKER_IMAGE} make qemu
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS kernel
    USES_TERMINAL
)

################################################################################
# Copyright (c) 2019-2021 Vladislav Trifochkin
#
# This file is part of `modulus-lib`.
#
# Changelog:
#      20??.??.?? Initial version
#      2022.01.10 Refactored.
################################################################################
cmake_minimum_required (VERSION 3.5)
project(modulus-lib-ALL CXX)

option(MODULUS__BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(MODULUS__BUILD_TESTS "Build tests" OFF)
option(MODULUS__BUILD_DEMO "Build examples/demo" OFF)
option(MODULUS__ENABLE_COVERAGE "Build tests with coverage support" OFF)

if (NOT PORTABLE_TARGET__ENABLED)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/3rdparty/portable-target/cmake/v2)
    include(PortableTarget)
    portable_target(SET ENABLE_TRACE ON)
endif()

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if (MODULUS__BUILD_STRICT)
    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

include(${CMAKE_CURRENT_LIST_DIR}/library.cmake)

if (MODULUS__BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()

if (MODULUS__BUILD_DEMO AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/demo)
    add_subdirectory(demo)
endif()

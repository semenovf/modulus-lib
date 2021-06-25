################################################################################
# Copyright (c) 2019-2021 Vladislav Trifochkin
#
# This file is part of [modulus-lib](https://github.com/semenovf/modulus-lib) library.
#
# Changelog:
#      20??.??.?? Initial version
#      2021.06.25 
################################################################################
cmake_minimum_required (VERSION 3.5)
project(modulus-lib-ALL CXX)

option(BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_DEMO "Build examples/demo" ON)
option(ENABLE_COVERAGE "Build tests with coverage support" OFF)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if (BUILD_STRICT)
    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

include(${CMAKE_CURRENT_LIST_DIR}/3rdparty/pfs/common/library.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/library.cmake)

if (BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()

if (BUILD_DEMO)
    add_subdirectory(demo)
endif()

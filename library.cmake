################################################################################
# Copyright (c) 2019-2021 Vladislav Trifochkin
#
# This file is part of [modulus-lib](https://github.com/semenovf/modulus-lib) library.
#
# Changelog:
#      2021.06.25 Initial version.
################################################################################
cmake_minimum_required (VERSION 3.5)
project(modulus-lib CXX)

add_library(${PROJECT_NAME} INTERFACE)
add_library(pfs::modulus ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${PROJECT_NAME} INTERFACE pfs::common)

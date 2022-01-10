################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `modulus-lib`.
#
# Changelog:
#      2021.06.25 Initial version.
#      2021.06.25 Refactored using `portable_target`.
################################################################################
cmake_minimum_required (VERSION 3.11)
project(modulus-lib CXX)

if (NOT TARGET pfs::common)
    portable_target(INCLUDE_PROJECT
        ${CMAKE_CURRENT_LIST_DIR}/3rdparty/pfs/common/library.cmake)
endif()

portable_target(LIBRARY ${PROJECT_NAME} INTERFACE ALIAS pfs::modulus)
portable_target(INCLUDE_DIRS ${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
portable_target(LINK ${PROJECT_NAME} pfs::common)

cmake_minimum_required (VERSION 3.5.1)
project(pfs-modulus CXX)

option(pfs_modulus_BUILD_TESTS "Build tests" ON)

add_subdirectory(demo)

if (pfs_modulus_BUILD_TESTS)
    enable_testing()
    include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
    add_subdirectory(tests)
endif()

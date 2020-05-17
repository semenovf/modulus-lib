cmake_minimum_required (VERSION 3.5)
project(pfs-modulus CXX)

option(pfs_modulus_BUILD_TESTS "Build tests" OFF)
option(pfs_modulus_BUILD_DEMO  "Build demo" OFF)

add_library(pfs-modulus INTERFACE)

if (UNIX)
    target_link_libraries(pfs-modulus INTERFACE dl)
endif()

target_include_directories(pfs-modulus INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

if (NOT pfs-common_DIR)
    target_include_directories(pfs-modulus INTERFACE ${CMAKE_CURRENT_LIST_DIR}/pfs-common/include)
else()
    target_include_directories(pfs-modulus INTERFACE ${pfs-common_DIR}/include)
endif()

if (pfs_modulus_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if (pfs_modulus_BUILD_DEMO)
    add_subdirectory(demo)
endif()

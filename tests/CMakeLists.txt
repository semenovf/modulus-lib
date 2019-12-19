project(pfs_modulus_TESTS)
option(ENABLE_COVERAGE "Build tests with coverage support" OFF)

find_package(Threads)

if (ENABLE_COVERAGE)
    message(STATUS "Coverage feature enabled")
    if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "Coverage feature requires GCC")
    endif()

    set(CMAKE_CXX_FLAGS "-g -O0 --coverage")
    set(CMAKE_C_FLAGS "-g -O0 --coverage")
endif()

add_executable(sigslot sigslot.cpp)
add_test(NAME sigslot COMMAND sigslot)

add_executable(active_queue active_queue.cpp)
target_link_libraries(active_queue ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME active_queue COMMAND active_queue)

add_executable(modulus modulus.cpp)
target_link_libraries(modulus ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME modulus COMMAND modulus)
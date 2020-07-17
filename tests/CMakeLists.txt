option(ENABLE_COVERAGE "Build tests with coverage support" OFF)

if (ENABLE_COVERAGE)
    message(STATUS "Coverage feature enabled")
    if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(FATAL_ERROR "Coverage feature requires GCC")
    endif()

    set(CMAKE_CXX_FLAGS "-g -O0 --coverage")
    set(CMAKE_C_FLAGS "-g -O0 --coverage")
endif()

add_executable(sigslot sigslot.cpp)
target_link_libraries(sigslot PRIVATE pfs::modulus)
add_test(NAME sigslot COMMAND sigslot)

# FIXME Uses some C++14 features (need workaround for C++11)
# add_executable(primal_binder primal_binder.cpp)
# target_link_libraries(primal_binder pfs::modulus)
# add_test(NAME primal_binder COMMAND primal_binder)

add_executable(legacy_binder legacy_binder.cpp)
target_link_libraries(legacy_binder pfs::modulus)
add_test(NAME legacy_binder COMMAND legacy_binder)

add_executable(active_queue active_queue.cpp)
target_link_libraries(active_queue PRIVATE pfs::modulus)
add_test(NAME active_queue COMMAND active_queue)

add_executable(timer timer.cpp)
target_link_libraries(timer PRIVATE pfs::modulus)

add_executable(modulus modulus.cpp)
target_link_libraries(modulus PRIVATE pfs::modulus)

if (UNIX)
    target_link_libraries(modulus PRIVATE dl)
endif()

add_test(NAME modulus COMMAND modulus)

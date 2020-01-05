find_package(Threads REQUIRED)

add_executable(demo main.cpp)
target_link_libraries(demo Threads::Threads)

if (UNIX)
    target_link_libraries(demo dl)
endif()

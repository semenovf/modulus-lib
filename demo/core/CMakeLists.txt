find_package(Threads REQUIRED)

add_executable(demo main.cpp)
target_link_libraries(demo Threads::Threads pfs-modulus)

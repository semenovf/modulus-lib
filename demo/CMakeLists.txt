include_directories(${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/pfs-common/include
        shared)

add_subdirectory(mod-db)
add_subdirectory(mod-link)
add_subdirectory(mod-ui)

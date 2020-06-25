include_directories(shared)

add_subdirectory(core)
add_subdirectory(mod-async)
add_subdirectory(mod-async-other)
add_subdirectory(mod-db)
add_subdirectory(mod-link)
add_subdirectory(mod-ui)
add_subdirectory(mod-ui-dialog)

set_target_properties(demo
    mod-async
    mod-async-other
    mod-db
    mod-link
    mod-ui
    mod-ui-dialog
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

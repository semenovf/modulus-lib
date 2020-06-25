add_library(mod-ui-dialog SHARED module.cpp module.hpp)
target_link_libraries(mod-ui-dialog PRIVATE pfs::modulus)

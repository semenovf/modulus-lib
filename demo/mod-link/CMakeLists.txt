add_library(mod-link SHARED module.cpp module.hpp)
target_link_libraries(mod-link PRIVATE pfs::modulus)

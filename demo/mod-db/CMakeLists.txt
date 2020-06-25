add_library(mod-db SHARED module.cpp module.hpp)
target_link_libraries(mod-db PRIVATE pfs::modulus)

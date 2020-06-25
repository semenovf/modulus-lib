add_library(mod-ui SHARED module.cpp module.hpp)
target_link_libraries(mod-ui PRIVATE pfs::modulus)

add_library(mod-async SHARED module.cpp module.hpp)
target_link_libraries(mod-async PRIVATE pfs::modulus)

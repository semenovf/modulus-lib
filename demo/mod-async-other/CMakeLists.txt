add_library(mod-async-other SHARED module.cpp module.hpp)
target_link_libraries(mod-async-other PRIVATE pfs::modulus)

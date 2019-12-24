#include "module.hpp"

extern "C" {

modulus::basic_module * __module_ctor__ (void)
{
    return new mod::db::module;
}

void  __module_dtor__ (modulus::basic_module * m)
{
    delete m;
}

} // extern "C"

namespace mod {
namespace db {

PFS_MODULE_EMITTERS_BEGIN(module)
PFS_MODULE_EMITTERS_END

PFS_MODULE_DETECTORS_BEGIN(module)
PFS_MODULE_DETECTORS_END

}} // namespace mod::db

////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.23 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "modulus.hpp"

namespace mod {
namespace db {

class module : public modulus::module
{
public:
    PFS_MODULE_EMITTERS_EXTERN
    PFS_MODULE_DETECTORS_EXTERN
};

}} // namespace mod::db

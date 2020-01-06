////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019,2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.01.05 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "modulus.hpp"

namespace mod {
namespace ui {
namespace dialog {

class module : public modulus::slave_module
{
public:
    MODULUS_DECL_EMITTERS
    MODULUS_DECL_DETECTORS

private:
    virtual bool on_loaded () override;
    virtual bool on_start () override;
    virtual bool on_finish () override;

    void onUiReady (bool ready);
};

}}} // namespace mod::ui::dialog

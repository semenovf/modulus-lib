////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019,2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.02.19 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "modulus.hpp"
#include "Printer.hpp"
#include <memory>

namespace mod {
namespace async {

class module : public modulus::async_module
{
    std::unique_ptr<Printer> _printer;

public:
    MODULUS_DECL_EMITTERS
    MODULUS_DECL_DETECTORS

private:
    virtual bool on_loaded () override;
    virtual bool on_start (modulus::settings_type const &) override;
    virtual bool on_finish () override;
    virtual int run () override;

    modulus::sigslot_ns::signal<> emitOnStartTest;
    void onStartTest ();
};

}} // namespace mod::async

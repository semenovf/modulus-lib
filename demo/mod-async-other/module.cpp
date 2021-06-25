////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019,2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.02.19 Initial version.
////////////////////////////////////////////////////////////////////////////////
#include "module.hpp"
#include "api.hpp"
#include <iostream>

#if _MSC_VER
#   define DLL_EXPORT __declspec(dllexport)
#else
#   define DLL_EXPORT
#endif

extern "C" {

DLL_EXPORT modulus::basic_module * __module_ctor__ (void)
{
    return new mod::async::other::module;
}

DLL_EXPORT void  __module_dtor__ (modulus::basic_module * m)
{
    delete m;
}

} // extern "C"

namespace mod {
namespace async {
namespace other {

MODULUS_BEGIN_EMITTERS(module)
    MODULUS_EMITTER(API_ON_START_TEST, emitOnStartTest)
MODULUS_END_EMITTERS

MODULUS_BEGIN_DETECTORS(module)
    MODULUS_DETECTOR(API_ON_START_TEST, module::onStartTest)
MODULUS_END_DETECTORS

bool module::on_loaded ()
{
    log_debug("on_loaded()");
    return true;
}

bool module::on_start (modulus::settings_type const &)
{
    log_debug("on_start()");
    _printer.reset(new Printer);
    emitOnStartTest();
    return true;
}

bool module::on_finish ()
{
    log_debug("on_finish()");
    _printer.reset(nullptr);
    return true;
}

void module::onStartTest ()
{
    assert(_printer);

    if (_printer) {
        _printer->print("*** On start test ***");
    } else {
        log_error("printer is NULL");
    }
}

int module::run ()
{
    log_debug("run()");

    acquire_timer(5, 1, [this] {
        this->log_debug("One-second periodic timer fired");
    });

    while (! is_quit()) {
        if (this->has_pending_events()) {
            this->process_events(10);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}

}}} // namespace mod::async::other

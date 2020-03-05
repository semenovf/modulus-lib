////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019,2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.01.05 Initial version.
////////////////////////////////////////////////////////////////////////////////
#include "module.hpp"
#include "api.hpp"
#include <iostream>

extern "C" {

modulus::basic_module * __module_ctor__ (void)
{
    return new mod::ui::module;
}

void  __module_dtor__ (modulus::basic_module * m)
{
    delete m;
}

} // extern "C"

namespace mod {
namespace ui {

MODULUS_BEGIN_EMITTERS(module)
      MODULUS_EMITTER(API_UI_READY, emitUiReady)
    , MODULUS_EMITTER(API_ON_START_TEST, emitOnStartTest)
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
    std::puts("+++ mod-ui +++");
    log_debug("on_start()");
//     emitOnStartTest();
    _printer.reset(new Printer);
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
    if (_printer) {
        _printer->print("*** On start test ***");
    } else {
        log_error("printer is NULL");
    }
}

int module::run ()
{
    log_debug("run()");
    emitUiReady();

    acquire_timer(5, 1, [this] {
        this->log_debug("One-second periodic timer fired");
    });

    acquire_timer(2, 0, [] {
        std::cout << "One-shot Timer fired\n";
    });

    while (! is_quit()) {
        // FIXME Use condition_variable to wait until callback queue will not be empty.
        if (this->has_pending_events()) {
            this->process_events(10);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    return 0;
}

}} // namespace mod::ui

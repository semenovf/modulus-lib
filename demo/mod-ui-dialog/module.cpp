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

#if _MSC_VER
#   define DLL_EXPORT __declspec(dllexport)
#else
#   define DLL_EXPORT
#endif

extern "C" {

DLL_EXPORT modulus::basic_module * __module_ctor__ (void)
{
    return new mod::ui::dialog::module;
}

DLL_EXPORT void  __module_dtor__ (modulus::basic_module * m)
{
    delete m;
}

} // extern "C"

namespace mod {
namespace ui {
namespace dialog {

MODULUS_BEGIN_EMITTERS(module)
    MODULUS_EMITTER(API_ON_START_TEST, emitOnStartTest)
MODULUS_END_EMITTERS

MODULUS_BEGIN_DETECTORS(module)
      MODULUS_DETECTOR(API_UI_READY, module::onUiReady)
    , MODULUS_DETECTOR(API_ON_START_TEST, module::onStartTest)
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

void module::onUiReady (bool ready)
{
    log_debug(modulus::string_type("UI Ready: ") + (ready ? "yes" : "no"));
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

}}} // namespace mod::ui::dialog

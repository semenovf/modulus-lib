////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.01.05 Initial version
////////////////////////////////////////////////////////////////////////////////
#include "api.hpp"
#include "modulus.hpp"
#include <algorithm>
#include <vector>

static modulus::dispatcher * pdisp = nullptr;

extern "C" void quit_app (int signum)
{
    assert(pdisp);
    pdisp->signal_handler(signum);
}

static modulus::api_item_type API[] = {
    { API_UI_READY
        , modulus::make_mapper()
        , "User Interface loaded and ready to process user activity"}
};

int main (int argc, char * argv[])
{
    pfs::default_settings settings;
    pfs::simple_logger logger;
    modulus::dispatcher dispatcher(API, sizeof(API) / sizeof(API[0]), settings, logger);
    pdisp = & dispatcher;
    pdisp->set_quit_handler(quit_app);

    std::vector<std::pair<std::string,std::string>> modules{
          { "mod-db"         , "" }
        , { "mod-link"       , "" }
        , { "mod-ui"         , "" }
        , { "mod-ui-dialog"  , "mod-ui" }
        , { "mod-async"      , "" }
        , { "mod-async-other", "" }
    };

    auto ready = std::all_of(modules.cbegin(), modules.cend()
            , [& dispatcher] (std::pair<std::string,std::string> const & p) {
                return dispatcher.register_module_for_name(p);
            });

    if (! ready)
        return -1;

    return dispatcher.exec();
}

////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.19 Initial version (inhereted from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/modulus.hpp"
#include <cstring>
#include <limits>

using modulus_ns = pfs::modulus<>;

class emitter_module : public modulus_ns::module
{
public:
    emitter_module () : modulus_ns::module()
    {}

    ~emitter_module ()
    {}

    virtual bool on_start () override
    {
        emitZeroArg();

        emitOneArg(true);

        emitTwoArgs(true, 'c');

        emitThreeArgs(true, 'c'
                , std::numeric_limits<short>::max());

        emitFourArgs(true, 'c'
                , std::numeric_limits<short>::max()
                , std::numeric_limits<int>::max());

        emitFiveArgs ( true, 'c'
                , std::numeric_limits<short>::max()
                , std::numeric_limits<int>::max()
                , std::numeric_limits<int>::max() );

        emitSixArgs ( true, 'c'
                , std::numeric_limits<short>::max()
                , std::numeric_limits<int>::max()
                , std::numeric_limits<int>::max()
                , "Hello, World!" );

        return true;
    }

    virtual bool on_finish () override
    {
        return true;
    }

    PFS_MODULE_EMITTERS_INLINE_BEGIN
          PFS_MODULE_EMITTER(0, emitZeroArg)
        , PFS_MODULE_EMITTER(1, emitOneArg)
        , PFS_MODULE_EMITTER(2, emitTwoArgs)
        , PFS_MODULE_EMITTER(3, emitThreeArgs)
        , PFS_MODULE_EMITTER(4, emitFourArgs)
        , PFS_MODULE_EMITTER(5, emitFiveArgs)
        , PFS_MODULE_EMITTER(6, emitSixArgs)
    PFS_MODULE_EMITTERS_END

public: /*signal*/
    modulus_ns::sigslot_ns::signal<> emitZeroArg;
    modulus_ns::sigslot_ns::signal<bool> emitOneArg;
    modulus_ns::sigslot_ns::signal<bool, char> emitTwoArgs;
    modulus_ns::sigslot_ns::signal<bool, char, short> emitThreeArgs;
    modulus_ns::sigslot_ns::signal<bool, char, short, int> emitFourArgs;
    modulus_ns::sigslot_ns::signal<bool, char, short, int, long> emitFiveArgs;
    modulus_ns::sigslot_ns::signal<bool, char, short, int, long, const char *> emitSixArgs;
};

class detector_module : public modulus_ns::module
{
    int _counter = 0;

public:
    detector_module () : modulus_ns::module()
    {}

    ~detector_module ()
    {}

    virtual bool on_start () override
    {
        return true;
    }

    virtual bool on_finish () override
    {
        return true;
    }

    PFS_MODULE_DETECTORS_INLINE_BEGIN
          PFS_MODULE_DETECTOR(0, detector_module::onZeroArg)
        , PFS_MODULE_DETECTOR(1, detector_module::onOneArg)
        , PFS_MODULE_DETECTOR(2, detector_module::onTwoArgs)
        , PFS_MODULE_DETECTOR(3, detector_module::onThreeArgs)
        , PFS_MODULE_DETECTOR(4, detector_module::onFourArgs)
        , PFS_MODULE_DETECTOR(5, detector_module::onFiveArgs)
        , PFS_MODULE_DETECTOR(6, detector_module::onSixArgs)
    PFS_MODULE_DETECTORS_END

private:
    void onZeroArg ()
    {
        _counter++;
    }

    void onOneArg (bool ok)
    {
        _counter++;
        CHECK(ok);
    }

    void onTwoArgs (bool ok, char ch)
    {
        _counter++;
        CHECK(ok);
        CHECK(ch == 'c');
    }

    void onThreeArgs (bool ok, char, short i)
    {
        _counter++;
        CHECK(ok);
        CHECK(i == std::numeric_limits<short>::max());
    }

    void onFourArgs (bool ok, char, short, int i)
    {
        _counter++;
        CHECK(ok);
        CHECK(i == std::numeric_limits<int>::max());
    }

    void onFiveArgs (bool ok, char, short, int, long i)
    {
        _counter++;
        CHECK(ok);
        CHECK(i == std::numeric_limits<int>::max());
    }

    void onSixArgs (bool ok, char, short, int, long, char const * hello)
    {
        _counter++;
        CHECK(ok);
        CHECK(std::strcmp("Hello, World!", hello) == 0);
    }
};

class async_module : public modulus_ns::async_module
{
public:
    async_module () : modulus_ns::async_module()
    {}

    int run ()
    {
        int i = 3;
        while (! is_quit() && i--) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            call_all();
        }

        quit();
    }
};

class slave_module : public modulus_ns::slave_module
{
    int _counter = 0;

public:
    slave_module () : modulus_ns::slave_module()
    {
//         set_master(master);
    }

    ~slave_module ()
    {}

    virtual bool on_start () override
    {
        emitOneArg(true);
        emitTwoArgs(true, 'c');

        return true;
    }

    virtual bool on_finish () override
    {
        return true;
    }

    PFS_MODULE_EMITTERS_INLINE_BEGIN
          PFS_MODULE_EMITTER(1, emitOneArg)
        , PFS_MODULE_EMITTER(2, emitTwoArgs)
    PFS_MODULE_EMITTERS_END

    PFS_MODULE_DETECTORS_INLINE_BEGIN
          PFS_MODULE_DETECTOR (1, slave_module::onOneArg)
        , PFS_MODULE_DETECTOR (2, slave_module::onTwoArgs)
    PFS_MODULE_DETECTORS_END

public: /*signal*/
    modulus_ns::sigslot_ns::signal<bool> emitOneArg;
    modulus_ns::sigslot_ns::signal<bool, char> emitTwoArgs;

public: /*slots*/
    void onOneArg (bool ok)
    {
        _counter++;
        CHECK_MESSAGE(ok, "from slave_module: onOneArg(bool)");
    }

    void onTwoArgs (bool ok, char ch)
    {
        CHECK(ok);
        CHECK_MESSAGE(ch == 'c', "from slave_module: onTwoArgs(true, 'c')");
    }
};

static modulus_ns::api_item_type API[] = {
      { 0 , modulus_ns::make_mapper<>(), "ZeroArg()" }
    , { 1 , modulus_ns::make_mapper<bool>(), "OneArg(bool b)\n\t boolean value" }
    , { 2 , modulus_ns::make_mapper<bool, char>(), "TwoArgs(bool b, char ch)" }
    , { 3 , modulus_ns::make_mapper<bool, char, short>(), "ThreeArgs(bool b, char ch, short n)" }
    , { 4 , modulus_ns::make_mapper<bool, char, short, int>(), "FourArgs description" }
    , { 5 , modulus_ns::make_mapper<bool, char, short, int, long>(), "FiveArgs description" }
    , { 6 , modulus_ns::make_mapper<bool, char, short, int, long, const char*>(), "SixArgs description" }
};

TEST_CASE("Modulus basics") {

    pfs::simple_logger logger;
    modulus_ns::dispatcher dispatcher(API, sizeof(API) / sizeof(API[0]), logger);

    CHECK(dispatcher.register_regular_module<emitter_module>("emitter_module"));
    CHECK(dispatcher.register_regular_module<detector_module>("detector_module"));
    CHECK(dispatcher.register_async_module<async_module>("async_module", & async_module::run));
    CHECK(dispatcher.register_slave_module<slave_module>("slave_module", "async_module"));

    CHECK_FALSE(dispatcher.register_module_for_name("module-for-test-app-nonexistence"));

    CHECK(dispatcher.count() == 4);
    CHECK(dispatcher.exec() == 0);
}


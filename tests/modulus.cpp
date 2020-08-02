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
#include <string>

using modulus = pfs::modulus<>;

struct Data
{
    std::string name {"a"};
    std::string value {"b"};
};

class emitter_module : public modulus::module
{
public:
    emitter_module () : modulus::module()
    {}

    ~emitter_module ()
    {}

    virtual bool on_start (modulus::settings_type const &) override
    {
        emitZeroArg();

        emitOneArg(true);

        emitTwoArgs(true, 'c');

        emitThreeArgs(true, 'c'
                , std::numeric_limits<short>::max());

        emitFourArgs(true, 'c'
                , std::numeric_limits<short>::max()
                , std::numeric_limits<int>::max());

        emitFiveArgs(true, 'c'
                , std::numeric_limits<short>::max()
                , std::numeric_limits<int>::max()
                , std::numeric_limits<int>::max());

        emitSixArgs(true, 'c'
                , std::numeric_limits<short>::max()
                , std::numeric_limits<int>::max()
                , std::numeric_limits<int>::max()
                , "Hello, World!");

        Data d;
        d.name = std::string{"Name"};
        d.value = std::string{"Value"};

        emitData(std::move(d));

        return true;
    }

    virtual bool on_finish () override
    {
        return true;
    }

    MODULUS_BEGIN_INLINE_EMITTERS
          MODULUS_EMITTER(0, emitZeroArg)
        , MODULUS_EMITTER(1, emitOneArg)
        , MODULUS_EMITTER(2, emitTwoArgs)
        , MODULUS_EMITTER(3, emitThreeArgs)
        , MODULUS_EMITTER(4, emitFourArgs)
        , MODULUS_EMITTER(5, emitFiveArgs)
        , MODULUS_EMITTER(6, emitSixArgs)
        , MODULUS_EMITTER(7, emitData)
    MODULUS_END_EMITTERS

public: /*signal*/
    modulus::sigslot_ns::signal<> emitZeroArg;
    modulus::sigslot_ns::signal<bool> emitOneArg;
    modulus::sigslot_ns::signal<bool, char> emitTwoArgs;
    modulus::sigslot_ns::signal<bool, char, short> emitThreeArgs;
    modulus::sigslot_ns::signal<bool, char, short, int> emitFourArgs;
    modulus::sigslot_ns::signal<bool, char, short, int, long> emitFiveArgs;
    modulus::sigslot_ns::signal<bool, char, short, int, long, const char *> emitSixArgs;
    modulus::sigslot_ns::signal<Data> emitData;
};

class detector_module : public modulus::module
{
    int _counter = 0;

public:
    detector_module () : modulus::module()
    {}

    ~detector_module ()
    {}

    virtual bool on_start (modulus::settings_type const &) override
    {
        return true;
    }

    virtual bool on_finish () override
    {
        return true;
    }

    MODULUS_BEGIN_INLINE_DETECTORS
          MODULUS_DETECTOR(0, detector_module::onZeroArg)
        , MODULUS_DETECTOR(1, detector_module::onOneArg)
        , MODULUS_DETECTOR(2, detector_module::onTwoArgs)
        , MODULUS_DETECTOR(3, detector_module::onThreeArgs)
        , MODULUS_DETECTOR(4, detector_module::onFourArgs)
        , MODULUS_DETECTOR(5, detector_module::onFiveArgs)
        , MODULUS_DETECTOR(6, detector_module::onSixArgs)
        , MODULUS_DETECTOR(7, detector_module::onData)
    MODULUS_END_DETECTORS

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

    void onData (Data const & d)
    {
        _counter++;
        CHECK(d.name == "Name");
        CHECK(d.value == "Value");
    }
};

class async_module : public modulus::async_module
{
public:
    async_module () : modulus::async_module()
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

class slave_module : public modulus::slave_module
{
    int _counter = 0;

public:
    slave_module () : modulus::slave_module()
    {}

    ~slave_module ()
    {}

    virtual bool on_start (modulus::settings_type const &) override
    {
        emitOneArg(true);
        emitTwoArgs(true, 'c');

        return true;
    }

    virtual bool on_finish () override
    {
        return true;
    }

    MODULUS_BEGIN_INLINE_EMITTERS
          MODULUS_EMITTER(1, emitOneArg)
        , MODULUS_EMITTER(2, emitTwoArgs)
    MODULUS_END_EMITTERS

    MODULUS_BEGIN_INLINE_DETECTORS
          MODULUS_DETECTOR(1, slave_module::onOneArg)
        , MODULUS_DETECTOR(2, slave_module::onTwoArgs)
        , MODULUS_DETECTOR(7, slave_module::onData)
    MODULUS_END_DETECTORS

public: /*signal*/
    modulus::sigslot_ns::signal<bool> emitOneArg;
    modulus::sigslot_ns::signal<bool, char> emitTwoArgs;

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

    void onData (Data const & d)
    {
        _counter++;
        CHECK_MESSAGE(d.name == "Name", "from slave_module: onData()");
        CHECK_MESSAGE(d.value == "Value", "from slave_module: onData()");
    }
};

static modulus::api_item_type API[] = {
      { 0 , modulus::make_mapper<>(), "ZeroArg()" }
    , { 1 , modulus::make_mapper<bool>(), "OneArg(bool b)\n\t boolean value" }
    , { 2 , modulus::make_mapper<bool, char>(), "TwoArgs(bool b, char ch)" }
    , { 3 , modulus::make_mapper<bool, char, short>(), "ThreeArgs(bool b, char ch, short n)" }
    , { 4 , modulus::make_mapper<bool, char, short, int>(), "FourArgs description" }
    , { 5 , modulus::make_mapper<bool, char, short, int, long>(), "FiveArgs description" }
    , { 6 , modulus::make_mapper<bool, char, short, int, long, const char*>(), "SixArgs description" }
    , { 7 , modulus::make_mapper<Data>(), "Data description" }
};

TEST_CASE("Modulus basics") {

    pfs::default_settings settings;
    pfs::simple_logger logger;
    modulus::dispatcher dispatcher(API, sizeof(API) / sizeof(API[0]), settings, logger);

    CHECK(dispatcher.register_module<emitter_module>(std::make_pair("emitter_module", "")));
    CHECK(dispatcher.register_module<detector_module>(std::make_pair("detector_module", "")));
    CHECK(dispatcher.register_module<async_module>(std::make_pair("async_module", "")));
    CHECK(dispatcher.register_module<slave_module>(std::make_pair("slave_module", "async_module")));

    CHECK_FALSE(dispatcher.register_module_for_name(std::make_pair("module-for-test-app-nonexistence", "")));

    CHECK(dispatcher.count() == 4);
    CHECK(dispatcher.exec() == 0);
}


////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.19 Initial version (inherited from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/active_queue.hpp"
#include "pfs/sigslot.hpp"

////////////////////////////////////////////////////////////////////////////////
// Direct signals / slots
////////////////////////////////////////////////////////////////////////////////
namespace t0 {

using sigslot = pfs::sigslot<>;

class A : public sigslot::slot_holder
{
public:
    int counter = 0;

public:
    void slot () { counter++; }
    void slot (int) { counter++; }
    void slot (int, double) { counter++; }
    void slot (int, double, std::string const &) { counter++; }
};

} // namespace t0

TEST_CASE("Direct signals / slots") {

    using t0::A;
    using t0::sigslot;

    A a;
    sigslot::signal<> sig0;
    sigslot::signal<int> sig1;
    sigslot::signal<int, double> sig2;
    sigslot::signal<int, double, std::string const &> sig3;

    sig0.connect(& a, static_cast<void (A::*)()>(& A::slot));
    sig1.connect(& a, static_cast<void (A::*)(int)>(& A::slot));
    sig2.connect(& a, static_cast<void (A::*)(int, double)>(& A::slot));
    sig3.connect(& a, static_cast<void (A::*)(int, double, std::string const &)>(& A::slot));

    sig0();
    sig1(42);
    sig2(42, 3.14f);
    sig3(42, 3.14f, "hello");

    CHECK(a.counter == 4);

    sigslot::emit_signal(sig0);
    sigslot::emit_signal(sig1, 42);
    sigslot::emit_signal(sig2, 42, 3.14f);
    sigslot::emit_signal(sig3, 42, 3.14f, "hello");

    CHECK(a.counter == 8);
}

////////////////////////////////////////////////////////////////////////////////
// Queued signals / slots
////////////////////////////////////////////////////////////////////////////////
namespace t1 {

using active_queue = pfs::active_queue<>;
using sigslot = pfs::sigslot<active_queue>;

class B : public sigslot::queued_slot_holder
{
public:
    int counter = 0;

public:
    void slot () { counter++; }
    void slot (int) { counter++; }
    void slot (int, double) { counter++; }
    void slot (int, double, std::string const &) { counter++; }
};

} // namespace t1

TEST_CASE("Queued signals / slots") {
    using t1::B;
    using t1::sigslot;

    B b;
    sigslot::signal<> sig0;
    sigslot::signal<int> sig1;
    sigslot::signal<int, double> sig2;
    sigslot::signal<int, double, std::string const &> sig3;

    sig0.connect(& b, static_cast<void (B::*)()>(& B::slot));
    sig1.connect(& b, static_cast<void (B::*)(int)>(& B::slot));
    sig2.connect(& b, static_cast<void (B::*)(int, double)>(& B::slot));
    sig3.connect(& b, static_cast<void (B::*)(int, double, std::string const &)>(& B::slot));

    sig0();
    sig1(42);
    sig2(42, 3.14f);
    sig3(42, 3.14f, "hello");

    CHECK(b.callback_queue().count() == 4);
    CHECK(b.counter == 0);

    b.callback_queue().call_all();

    CHECK(b.callback_queue().count() == 0);
    CHECK(b.counter == 4);

    sigslot::emit_signal(sig0);
    sigslot::emit_signal(sig1, 42);
    sigslot::emit_signal(sig2, 42, 3.14f);
    sigslot::emit_signal(sig3, 42, 3.14f, "hello");

    CHECK(b.callback_queue().count() == 4);
    CHECK(b.counter == 4);

    b.callback_queue().call_all();

    CHECK(b.callback_queue().count() == 0);

    CHECK(b.counter == 8);
}

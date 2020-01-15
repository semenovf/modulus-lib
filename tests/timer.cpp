////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.01.15 Initial version
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/timer.hpp"

////////////////////////////////////////////////////////////////////////////////
// Direct signals / slots
////////////////////////////////////////////////////////////////////////////////
// namespace t0 {
//
// using sigslot = pfs::sigslot<>;
//
// class A : public sigslot::slot_holder
// {
// public:
//     int counter = 0;
//
// public:
//     void slot () { counter++; }
//     void slot (int) { counter++; }
//     void slot (int, double) { counter++; }
//     void slot (int, double, std::string const &) { counter++; }
// };
//
// } // namespace t0
//
TEST_CASE("Basic timer") {
//
//     using t0::A;
//     using t0::sigslot;
//
//     A a;
//     sigslot::signal<> sig0;
//     sigslot::signal<int> sig1;
//     sigslot::signal<int, double> sig2;
//     sigslot::signal<int, double, std::string const &> sig3;
//
//     sig0.connect(& a, static_cast<void (A::*)()>(& A::slot));
//     sig1.connect(& a, static_cast<void (A::*)(int)>(& A::slot));
//     sig2.connect(& a, static_cast<void (A::*)(int, double)>(& A::slot));
//     sig3.connect(& a, static_cast<void (A::*)(int, double, std::string const &)>(& A::slot));
//
//     sig0();
//     sig1(42);
//     sig2(42, 3.14f);
//     sig3(42, 3.14f, "hello");
//
//     CHECK(a.counter == 4);
//
//     sigslot::emit_signal(sig0);
//     sigslot::emit_signal(sig1, 42);
//     sigslot::emit_signal(sig2, 42, 3.14f);
//     sigslot::emit_signal(sig3, 42, 3.14f, "hello");
//
//     CHECK(a.counter == 8);
}

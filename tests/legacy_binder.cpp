////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.06.21 Initial version
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define ANKERL_NANOBENCH_IMPLEMENT
#include "doctest.h"
#include "nanobench.h"
#include "pfs/legacy_binder.hpp"
#include <functional>

void sum (int a, int b, int & result)
{
    result = a + b;
}

void sum_overloaded (int a, int b, int & result)
{
    result = a + b;
}

void sum_overloaded (int a, int b, int c, int & result)
{
    result = a + b + c;
}

int sum_int (int a, int b)
{
    return a + b;
}

struct A
{
    void sum (int a, int b, int & result)
    {
        result = a + b;
    }

    void sum_overloaded (int a, int b, int & result)
    {
        result = a + b;
    }

    void sum_overloaded (int a, int b, int c, int & result)
    {
        result = a + b + c;
    }

    int sum_int (int a, int b)
    {
        return a + b;
    }
};

TEST_CASE("regular_function_invoker")
{
    using invoker = pfs::legacy::regular_function_invoker<void>;
    using invoker_int = pfs::legacy::regular_function_invoker<int>;

    int result = 0;

    invoker{}.invoke(sum, 2, 3, result);
    CHECK(result == 5);

    invoker{}.invoke(static_cast<void (*)(int, int, int &)>(sum_overloaded), 3, 4, result);
    CHECK(result == 7);

    invoker{}.invoke(static_cast<void (*)(int, int, int, int &)>(sum_overloaded), 3, 4, 5, result);
    CHECK(result == 12);

    CHECK(invoker_int{}.invoke(sum_int, 2, 3) == 5);
}

TEST_CASE("member_function_invoker")
{
    using invoker = pfs::legacy::member_function_invoker<void>;
    using invoker_int = pfs::legacy::member_function_invoker<int>;

    A a;
    int result = 0;

    invoker{}.invoke(& A::sum, & a, 2, 3, result);
    CHECK(result == 5);

    invoker{}.invoke(static_cast<void (A::*)(int, int, int &)>(& A::sum_overloaded), & a, 3, 4, result);
    CHECK(result == 7);

    invoker{}.invoke(static_cast<void (A::*)(int, int, int, int &)>(& A::sum_overloaded), & a, 3, 4, 5, result);
    CHECK(result == 12);

    CHECK(invoker_int{}.invoke(& A::sum_int, & a, 2, 3) == 5);
}

TEST_CASE("basic")
{
    {
        int result;
        auto f1 = pfs::legacy::bind(sum, 1, 2, result);
        f1();
        CHECK(result == 3);

        auto f2 = pfs::legacy::bind(
              static_cast<void (*)(int, int, int &)>(sum_overloaded)
            , 3, 4, result);
        f2();
        CHECK(result == 7);

        auto f3 = pfs::legacy::bind(
              static_cast<void (*)(int, int, int, int &)>(sum_overloaded)
            , 1, 2, 3, result);
        f3();
        CHECK(result == 6);
    }

    {
        int result;
        A a;
        auto f1 = pfs::legacy::bind(& A::sum, & a, 1, 2, result);
        f1();
        CHECK(result == 3);

        auto f2 = pfs::legacy::bind(
              static_cast<void (A::*)(int, int, int &)>(& A::sum_overloaded)
            , & a, 3, 4, result);
        f2();
        CHECK(result == 7);

        auto f3 = pfs::legacy::bind(
              static_cast<void (A::*)(int, int, int, int &)>(& A::sum_overloaded)
            , & a, 1, 2, 3, result);
        f3();
        CHECK(result == 6);
    }
}

static int __count = 0;
void test ()
{
    __count++;
}

TEST_CASE("benchmark")
{
    ankerl::nanobench::Bench().run("pfs::legacy::binder", [&] {
        auto f = pfs::legacy::bind(test);
        for (int i = 0, count = std::numeric_limits<uint16_t>::max() * 100; i < count; i++) {
            f();
        }
        //ankerl::nanobench::doNotOptimizeAway(d);
    });

    ankerl::nanobench::Bench().run("std::binder", [&] {
        auto f = std::bind(test);
        for (int i = 0, count = std::numeric_limits<uint16_t>::max() * 100; i < count; i++) {
            f();
        }
    });

}


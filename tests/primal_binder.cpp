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
#include "pfs/primal_binder.hpp"
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
};

template <typename F, typename... Args>
void checkType (F f, Args... args)
{
    using return_t = typename std::result_of<F(Args...)>::type;

    std::cout << "int:    " << std::is_same<int, return_t>::value << "\n";
    std::cout << "double: " << std::is_same<double, return_t>::value << "\n";
    std::cout << "float:  " << std::is_same<float, return_t>::value << "\n";
    std::cout << std::endl;
}

TEST_CASE("basic")
{
    using namespace pfs::primal;

    {
        int result;
        auto f1 = pfs::primal::bind(sum, 1, 2, result);
        f1();
        CHECK(result == 3);

        auto f2 = pfs::primal::bind(
              static_cast<void (*)(int, int, int &)>(sum_overloaded)
            , 3, 4, result);
        f2();
        CHECK(result == 7);

        auto f3 = pfs::primal::bind(
              static_cast<void (*)(int, int, int, int &)>(sum_overloaded)
            , 1, 2, 3, result);
        f3();
        CHECK(result == 6);
    }

    {
        int result;
        A a;
        auto f1 = bind(& A::sum, & a, 1, 2, result);
        f1();
        CHECK(result == 3);

        auto f2 = pfs::primal::bind(
              static_cast<void (A::*)(int, int, int &)>(& A::sum_overloaded)
            , & a, 3, 4, result);
        f2();
        CHECK(result == 7);

        auto f3 = pfs::primal::bind(
              static_cast<void (A::*)(int, int, int, int &)>(& A::sum_overloaded)
            , & a, 1, 2, 3, result);
        f3();
        CHECK(result == 6);
    }
}

static int __count = 0;
void test ()
{
    //std::cout << "Counter: " << __count++ << "\n";
    __count++;
}

TEST_CASE("benchmark")
{
    ankerl::nanobench::Bench().run("pfs::primal::binder", [&] {
        auto f = pfs::primal::bind(test);
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

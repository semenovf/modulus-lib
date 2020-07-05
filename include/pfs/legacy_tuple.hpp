////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.07.05 Initial version (inhereted from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#pragma once
namespace pfs {
namespace legacy {

template <typename _A1 = void
        , typename _A2 = void
        , typename _A3 = void
        , typename _A4 = void
        , typename _A5 = void
        , typename _A6 = void
        , typename _A7 = void
        , typename _A8 = void
        , typename _A9 = void>
struct tuple;

template <>
struct tuple<>
{
    tuple () {}
};

template <typename _A1>
struct tuple<_A1>
{
    _A1 _a1;

    tuple (_A1 && a1)
        : _a1(std::forward<_A1>(a1))
    {}
};

template <typename _A1
        , typename _A2>
struct tuple<_A1, _A2>
{
    _A1 _a1;
    _A2 _a2;

    tuple (_A1 && a1, _A2 && a2)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
    {}
};

template <typename _A1
        , typename _A2
        , typename _A3>
struct tuple<_A1, _A2, _A3>
{
    _A1 _a1;
    _A2 _a2;
    _A3 _a3;

    tuple (_A1 && a1, _A2 && a2, _A3 && a3)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
        , _a3(std::forward<_A3>(a3))
    {}
};

template <typename _A1
        , typename _A2
        , typename _A3
        , typename _A4>
struct tuple<_A1, _A2, _A3, _A4>
{
    _A1 _a1;
    _A2 _a2;
    _A3 _a3;
    _A4 _a4;

    tuple (_A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
        , _a3(std::forward<_A3>(a3))
        , _a4(std::forward<_A4>(a4))
    {}
};

template <typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5>
struct tuple<_A1, _A2, _A3, _A4, _A5>
{
    _A1 _a1;
    _A2 _a2;
    _A3 _a3;
    _A4 _a4;
    _A5 _a5;

    tuple (_A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
        , _a3(std::forward<_A3>(a3))
        , _a4(std::forward<_A4>(a4))
        , _a5(std::forward<_A5>(a5))
    {}
};

template <typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5
        , typename _A6>
struct tuple<_A1, _A2, _A3, _A4, _A5, _A6>
{
    _A1 _a1;
    _A2 _a2;
    _A3 _a3;
    _A4 _a4;
    _A5 _a5;
    _A6 _a6;

    tuple (_A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5, _A6 && a6)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
        , _a3(std::forward<_A3>(a3))
        , _a4(std::forward<_A4>(a4))
        , _a5(std::forward<_A5>(a5))
        , _a6(std::forward<_A6>(a6))
    {}
};

template <typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5
        , typename _A6
        , typename _A7>
struct tuple<_A1, _A2, _A3, _A4, _A5, _A6, _A7>
{
    _A1 _a1;
    _A2 _a2;
    _A3 _a3;
    _A4 _a4;
    _A5 _a5;
    _A6 _a6;
    _A7 _a7;

    tuple (_A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5, _A6 && a6, _A7 && a7)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
        , _a3(std::forward<_A3>(a3))
        , _a4(std::forward<_A4>(a4))
        , _a5(std::forward<_A5>(a5))
        , _a6(std::forward<_A6>(a6))
        , _a7(std::forward<_A7>(a7))
    {}
};

template <typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5
        , typename _A6
        , typename _A7
        , typename _A8>
struct tuple<_A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8>
{
    _A1 _a1;
    _A2 _a2;
    _A3 _a3;
    _A4 _a4;
    _A5 _a5;
    _A6 _a6;
    _A7 _a7;
    _A8 _a8;

    tuple (_A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5, _A6 && a6, _A7 && a7, _A8 && a8)
        : _a1(std::forward<_A1>(a1))
        , _a2(std::forward<_A2>(a2))
        , _a3(std::forward<_A3>(a3))
        , _a4(std::forward<_A4>(a4))
        , _a5(std::forward<_A5>(a5))
        , _a6(std::forward<_A6>(a6))
        , _a7(std::forward<_A7>(a7))
        , _a8(std::forward<_A8>(a8))
    {}
};

}} // namespace pfs::legacy

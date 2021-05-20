////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.06.22 Initial version (inherited from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pfs/type_traits.hpp"
#include "pfs/legacy_tuple.hpp"

namespace pfs {
namespace legacy {

template <typename R>
struct regular_function_invoker
{
    template <typename _F, typename... Args>
    R invoke (_F && f, Args &&... args) const
    {
        return f(std::forward<Args>(args)...);
    }
};

template <>
struct regular_function_invoker<void>
{
    template <typename _F, typename ...Args>
    void invoke (_F && f, Args &&... args) const
    {
        f(std::forward<Args>(args)...);
    }
};

template <typename R>
struct member_function_invoker
{
    template <typename _F, typename ClassT, typename ...Args>
    inline R invoke (_F && f, ClassT * o, Args &&... args) const
    {
        return (o->*f)(std::forward<Args>(args)...);
    }
};

template <>
struct member_function_invoker<void>
{
    template <typename _F, typename ClassT, typename ...Args>
    inline void invoke (_F && f, ClassT * o, Args &&... args) const
    {
        (o->*f)(std::forward<Args>(args)...);
    }
};

template <typename R>
class binder_interface
{
protected:
    virtual R invoke () const = 0;

public:
    virtual ~binder_interface () {}

    R operator () () const
    {
        return invoke();
    }
};

template <typename _R, typename _F
    , typename _A1 = void
    , typename _A2 = void
    , typename _A3 = void
    , typename _A4 = void
    , typename _A5 = void
    , typename _A6 = void
    , typename _A7 = void
    , typename _A8 = void>
class basic_binder: public binder_interface<_R>
{
public:
    using result_type = _R;

protected:
    using invoker_type = typename std::conditional<
          std::is_member_function_pointer<_F>::value
        , member_function_invoker<_R>
        , regular_function_invoker<_R>>::type;

    invoker_type _invoker;
    _F _func;
    tuple<_A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8> _args;

public:
    template <typename ...Args>
    basic_binder (_F && f, Args &&... args)
        : _func(std::forward<_F>(f))
        , _args(std::forward<Args>(args)...)
    {}

//     binder (_F && f, _A1 && a1)
//         : _func(std::forward<_F>(f))
//         , _args(std::forward<_A1>(a1))
//     {}
//
//     binder (_F && f, _A1 && a1, _A2 && a2)
//         : _func(std::forward<_F>(f))
//         , _args(std::forward<_A1>(a1), std::forward<_A2>(a2))
//     {}
//
//     binder (_F && f, _A1 && a1, _A2 && a2, _A3 && a3)
//         : _func(std::forward<_F>(f))
//         , _args(std::forward<_A1>(a1)
//             , std::forward<_A2>(a2)
//             , std::forward<_A3>(a3))
//     {}
//
//     binder (_F && f, _A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4)
//         : _func(std::forward<_F>(f))
//         , _args(std::forward<_A1>(a1)
//             , std::forward<_A2>(a2)
//             , std::forward<_A3>(a3)
//             , std::forward<_A4>(a4))
//     {}
// protected:
//     virtual _R invoke () const override
//     {
//         return _invoker.invoke(_func, _args._a1, _args._a2, _args._a3);
//     }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2
    , typename _A3
    , typename _A4
    , typename _A5
    , typename _A6
    , typename _A7
    , typename _A8>
class binder
    : public basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8>
{
    using parent = basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2
            , this->_args._a3
            , this->_args._a4
            , this->_args._a5
            , this->_args._a6
            , this->_args._a7
            , this->_args._a8);
    }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2
    , typename _A3
    , typename _A4
    , typename _A5
    , typename _A6
    , typename _A7>
class binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, void>
    : public basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7>
{
    using parent = basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2
            , this->_args._a3
            , this->_args._a4
            , this->_args._a5
            , this->_args._a6
            , this->_args._a7);
    }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2
    , typename _A3
    , typename _A4
    , typename _A5
    , typename _A6>
class binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6, void, void>
    : public basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6>
{
    using parent = basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5, _A6>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2
            , this->_args._a3
            , this->_args._a4
            , this->_args._a5
            , this->_args._a6);
    }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2
    , typename _A3
    , typename _A4
    , typename _A5>
class binder<_R, _F, _A1, _A2, _A3, _A4, _A5, void, void, void>
    : public basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5>
{
    using parent = basic_binder<_R, _F, _A1, _A2, _A3, _A4, _A5>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2
            , this->_args._a3
            , this->_args._a4
            , this->_args._a5);
    }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2
    , typename _A3
    , typename _A4>
class binder<_R, _F, _A1, _A2, _A3, _A4, void, void, void, void>
    : public basic_binder<_R, _F, _A1, _A2, _A3, _A4>
{
    using parent = basic_binder<_R, _F, _A1, _A2, _A3, _A4>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2
            , this->_args._a3
            , this->_args._a4);
    }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2
    , typename _A3>
class binder<_R, _F, _A1, _A2, _A3, void, void, void, void, void>
    : public basic_binder<_R, _F, _A1, _A2, _A3>
{
    using parent = basic_binder<_R, _F, _A1, _A2, _A3>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2
            , this->_args._a3);
    }
};

template <typename _R, typename _F
    , typename _A1
    , typename _A2>
class binder<_R, _F, _A1, _A2, void, void, void, void, void, void>
    : public basic_binder<_R, _F, _A1, _A2>
{
    using parent = basic_binder<_R, _F, _A1, _A2>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1
            , this->_args._a2);
    }
};

template <typename _R, typename _F
    , typename _A1>
class binder<_R, _F, _A1, void, void, void, void, void, void, void>
    : public basic_binder<_R, _F, _A1>
{
    using parent = basic_binder<_R, _F, _A1>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func
            , this->_args._a1);
    }
};

template <typename _R, typename _F>
class binder<_R, _F, void, void, void, void, void, void, void, void>
    : public basic_binder<_R, _F>
{
    using parent = basic_binder<_R, _F>;

public:
    using parent::parent;

protected:
    virtual _R invoke () const override
    {
        return this->_invoker.invoke(this->_func);
    }
};

template <typename _F
        , typename _A1 = void
        , typename _A2 = void
        , typename _A3 = void
        , typename _A4 = void
        , typename _A5 = void
        , typename _A6 = void
        , typename _A7 = void
        , typename _A8 = void>
inline binder<void, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8> bind (
    _F && f, _A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5, _A6 && a6, _A7 && a7, _A8 && a8)
{
    return binder<void, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2)
        , std::forward<_A3>(a3)
        , std::forward<_A4>(a4)
        , std::forward<_A5>(a5)
        , std::forward<_A6>(a6)
        , std::forward<_A7>(a7)
        , std::forward<_A8>(a8));
}

template <typename _F
        , typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5
        , typename _A6
        , typename _A7>
inline binder<void, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, void> bind (
    _F && f, _A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5, _A6 && a6, _A7 && a7)
{
    return binder<void, _F, _A1, _A2, _A3, _A4, _A5, _A6, _A7, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2)
        , std::forward<_A3>(a3)
        , std::forward<_A4>(a4)
        , std::forward<_A5>(a5)
        , std::forward<_A6>(a6)
        , std::forward<_A7>(a7));
}

template <typename _F
        , typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5
        , typename _A6>
inline binder<void, _F, _A1, _A2, _A3, _A4, _A5, _A6, void, void> bind (
    _F && f, _A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5, _A6 && a6)
{
    return binder<void, _F, _A1, _A2, _A3, _A4, _A5, _A6, void, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2)
        , std::forward<_A3>(a3)
        , std::forward<_A4>(a4)
        , std::forward<_A5>(a5)
        , std::forward<_A6>(a6));
}

template <typename _F
        , typename _A1
        , typename _A2
        , typename _A3
        , typename _A4
        , typename _A5>
inline binder<void, _F, _A1, _A2, _A3, _A4, _A5, void, void, void> bind (
    _F && f, _A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4, _A5 && a5)
{
    return binder<void, _F, _A1, _A2, _A3, _A4, _A5, void, void, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2)
        , std::forward<_A3>(a3)
        , std::forward<_A4>(a4)
        , std::forward<_A5>(a5));
}

template <typename _F
        , typename _A1
        , typename _A2
        , typename _A3
        , typename _A4>
inline binder<void, _F, _A1, _A2, _A3, _A4, void, void, void, void> bind (
    _F && f, _A1 && a1, _A2 && a2, _A3 && a3, _A4 && a4)
{
    return binder<void, _F, _A1, _A2, _A3, _A4, void, void, void, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2)
        , std::forward<_A3>(a3)
        , std::forward<_A4>(a4));
}

template <typename _F
        , typename _A1
        , typename _A2
        , typename _A3>
inline binder<void, _F, _A1, _A2, _A3, void, void, void, void, void> bind (
    _F && f, _A1 && a1, _A2 && a2, _A3 && a3)
{
    return binder<void, _F, _A1, _A2, _A3, void, void, void, void, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2)
        , std::forward<_A3>(a3));
}

template <typename _F
        , typename _A1
        , typename _A2>
inline binder<void, _F, _A1, _A2, void, void, void, void, void, void> bind (
    _F && f, _A1 && a1, _A2 && a2)
{
    return binder<void, _F, _A1, _A2, void, void, void, void, void, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1)
        , std::forward<_A2>(a2));
}

template <typename _F
        , typename _A1>
inline binder<void, _F, _A1, void, void, void, void, void, void, void> bind (
    _F && f, _A1 && a1)
{
    return binder<void, _F, _A1, void, void, void, void, void, void, void>(
          std::forward<_F>(f)
        , std::forward<_A1>(a1));
}

template <typename _F>
inline binder<void, _F, void, void, void, void, void, void, void, void> bind (
    _F && f)
{
    return binder<void, _F, void, void, void, void, void, void, void, void>(
        std::forward<_F>(f));
}

}} // pfs::legacy


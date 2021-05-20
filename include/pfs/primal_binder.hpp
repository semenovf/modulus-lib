////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2020.06.21 Initial version (inherited from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <type_traits>
#include <tuple>

namespace pfs {
namespace primal {

template <typename R>
class basic_binder
{
protected:
    virtual R invoke () const = 0;

public:
    virtual ~basic_binder () {}

    R operator () () const
    {
        return invoke();
    }
};

template <>
class basic_binder<void>
{
protected:
    virtual void invoke () const = 0;

public:
    virtual ~basic_binder () {}

    void operator () () const
    {
        invoke();
    }
};

template <typename R, typename F, typename ...Args>
class binder: public basic_binder<R>
{
    using result_type = R;

    struct regular_function_invoker
    {
        template </*typename R1, */typename... U>
        inline
        void invoke (F const & func, U &&... args) const
        {
            func(std::forward<U>(args)...);
        }
    };

    struct member_function_invoker
    {
        template </*typename R1, */typename ClassT, typename ...U>
        inline
        void invoke (F const & func, ClassT * o, U &&... args) const
        {
            (o->*func)(std::forward<U>(args)...);
        }
    };

    using invoker_type = typename std::conditional<
          std::is_member_function_pointer<F>::value
        , member_function_invoker
        , regular_function_invoker>::type;

    invoker_type _invoker;
    F _func;
    std::tuple<Args...> _args;

public:
    binder (F const & func, Args &&... args)
      : _func(func)
      , _args(std::forward<Args>(args)...)
    {}

    void operator () () const
    {
        invoke();
    }

protected:
    // FIXME std::index_sequence is C++14 feature
    template <size_t ...S>
    inline result_type invoke_tuple (std::index_sequence<S...>) const
    {
        return _invoker.invoke(_func, std::get<S>(_args)...);
    }

    result_type invoke () const
    {
        constexpr std::size_t nargs
            = std::tuple_size<std::tuple<Args...>>::value;

        return invoke_tuple(std::make_index_sequence<nargs>());
    }
};

template <typename F, typename... Args>
inline binder<void, F, Args...> bind (F && func, Args &&... args)
{
    return binder<void, F, Args...>(func, std::forward<Args>(args)...);
}

}} // pfs::primal

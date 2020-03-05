////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.19 Initial version (inhereted from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cassert>
#include <list>
#include <memory>
#include <mutex>
#include <set>

namespace pfs {

class fake_active_queue
{
public:
    using size_type = std::size_t;

public:
    fake_active_queue () {}

    bool empty () const { return true; }
    size_type count () const { return 0; }
    size_type size () const { return 0; }
    void clear () {}
    void call () {}
    void call (int) {}
    void call_all () {}

    template <typename F, typename ...Args>
    void push (F &&, Args &&...) {}
};

template <typename ActiveQueue = fake_active_queue
        , typename BasicLockable = std::mutex>
struct sigslot
{
    using callback_queue_type = ActiveQueue;
    using mutex_type = BasicLockable;

    class basic_slot_holder;

    // see [std::make_unique](http://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)
    // `Possible Implementation` section.
    template<typename T, typename ...Args>
    static inline std::unique_ptr<T> make_unique (Args &&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename SignalType, typename ...Args>
    static inline void emit_signal (SignalType & sig, Args &&... args)
    {
        sig.emit_signal(std::forward<Args>(args)...);
    }

////////////////////////////////////////////////////////////////////////////////
// connection_base
////////////////////////////////////////////////////////////////////////////////
    template <typename ...Args>
    class basic_connection
    {
    public:
        virtual ~basic_connection () {}
        virtual basic_slot_holder * get_slot_holder () const = 0;
        virtual void emit_signal (Args &&...) = 0;
    };

    class basic_signal : public mutex_type
    {
    public:
        virtual ~basic_signal () {}
        virtual void slot_disconnect (basic_slot_holder * pslot) = 0;
    };

////////////////////////////////////////////////////////////////////////////////
// basic_slot_holder
////////////////////////////////////////////////////////////////////////////////
    class basic_slot_holder : public mutex_type
    {
    protected:
        using sender_set = std::set<basic_signal *>;
        using const_iterator = typename sender_set::const_iterator ;

        sender_set _senders;
        std::unique_ptr<callback_queue_type> _queue_ptr;

    public:
        basic_slot_holder ()
        {}

        virtual bool use_queued_slots () const = 0;

        virtual bool is_slave () const
        {
            return false;
        }

        virtual basic_slot_holder * master () const
        {
            assert(this->is_slave());
            assert(false); // Must be reimplemented in slave_slot_holder
            return nullptr;
        }

        void signal_connect (basic_signal * sender)
        {
            std::lock_guard<mutex_type> lock(*this);
            _senders.insert(sender);
        }

        void signal_disconnect (basic_signal * sender)
        {
            std::lock_guard<mutex_type> lock(*this);
            _senders.erase(sender);
        }

        virtual ~basic_slot_holder ()
        {
            disconnect_all();
        }

        void disconnect_all ()
        {
            std::lock_guard<mutex_type> lock(*this);
            const_iterator it = _senders.begin();
            const_iterator last = _senders.end();

            while (it != last) {
                (*it)->slot_disconnect(this);
                ++it;
            }

            _senders.erase(_senders.begin(), _senders.end());
        }

        size_t count () const
        {
            return _senders.size();
        }

        callback_queue_type & callback_queue ()
        {
            return *_queue_ptr;
        }

        callback_queue_type const & callback_queue () const
        {
            return *_queue_ptr;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// slot_holder
////////////////////////////////////////////////////////////////////////////////
    class slot_holder : public basic_slot_holder
    {
    public:
        slot_holder () : basic_slot_holder ()
        {}

        virtual bool use_queued_slots () const override
        {
            return false;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// queued_slot_holder
////////////////////////////////////////////////////////////////////////////////
    class queued_slot_holder : public basic_slot_holder
    {
    public:
        queued_slot_holder () : basic_slot_holder ()
        {
            this->_queue_ptr = make_unique<callback_queue_type>();
        }

        virtual bool use_queued_slots () const override
        {
            return true;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// slave_slot_holder
////////////////////////////////////////////////////////////////////////////////
    class slave_slot_holder : public basic_slot_holder
    {
        basic_slot_holder * _master;

    public:
        slave_slot_holder (basic_slot_holder * master) : _master(master) {}
        virtual bool use_queued_slots () const override { return false; }
        virtual bool is_slave () const override { return true; }
        virtual basic_slot_holder * master () const { return _master; }
    };


////////////////////////////////////////////////////////////////////////////////
// signal_base
////////////////////////////////////////////////////////////////////////////////
    template <typename ...Args>
    class signal_base : public basic_signal
    {
    public:
        using connections_list = std::list<basic_connection<Args...> *>  ;

        signal_base ()
        {}

        ~signal_base ()
        {
            disconnect_all();
        }

        void disconnect_all ()
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = _connected_slots.begin();
            auto last = _connected_slots.end();

            while (it != last) {
                (*it)->get_slot_holder()->signal_disconnect(this);
                delete *it;

                ++it;
            }

            _connected_slots.erase(_connected_slots.begin(), _connected_slots.end());
        }

        void disconnect (basic_slot_holder * pclass)
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = _connected_slots.begin();
            auto last = _connected_slots.end();

            while (it != last) {
                if ((*it)->get_slot_holder() == pclass) {
                    delete *it;
                    _connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }

                ++it;
            }
        }

        void slot_disconnect (basic_slot_holder * pslot)
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = _connected_slots.begin();
            auto last = _connected_slots.end();

            while (it != last) {
                auto next = it;
                ++next;

                if ((*it)->get_slot_holder() == pslot) {
                    _connected_slots.erase(it);
                }

                it = next;
            }
        }

        bool is_connected () const
        {
            return _connected_slots.size() > 0;
        }

    protected:
        connections_list _connected_slots;
    };

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
    template <typename SlotHolderClass, typename ...Args>
    class connection : public basic_connection<Args...>
    {
    public:
        connection ()
            : _pobject(nullptr)
            , _pmemfun(nullptr)
        {}

        connection (SlotHolderClass * pobject, void (SlotHolderClass::*pmemfun)(Args...))
            : _pobject(pobject)
            , _pmemfun(pmemfun)
        {}

        virtual void emit_signal (Args &&... args)
        {
            using method_type = void (SlotHolderClass::*)(Args...);

            if (_pobject->use_queued_slots()) {
                SlotHolderClass * pobject = _pobject;
                method_type pmemfun = _pmemfun;

                //_pobject->callback_queue().template push<method_type, SlotHolderClass
                //        , Args...>(std::move(pmemfun), std::move(pobject), std::forward<Args>(args)...);
                _pobject->callback_queue().push(std::move(pmemfun)
                        , std::move(pobject)
                        , std::forward<Args>(args)...);
            } else if (_pobject->is_slave()) {
                SlotHolderClass * pobject = _pobject;
                method_type pmemfun = _pmemfun;

                // _pobject->master()->callback_queue().template push<SlotHolderClass
                //           , Args...>(_pmemfun, _pobject, std::forward<Args>(args)...);
                _pobject->master()->callback_queue().push(std::move(pmemfun)
                        , std::move(pobject)
                        , std::forward<Args>(args)...);
            } else {
                (_pobject->*_pmemfun)(std::forward<Args>(args)...);
            }
        }

        virtual basic_slot_holder * get_slot_holder () const
        {
            return _pobject;
        }

    private:
        SlotHolderClass * _pobject {nullptr};
        void (SlotHolderClass::* _pmemfun)(Args...);
    };

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
    template <typename ...Args>
    class signal : public signal_base<Args...>
    {
        using base_class = signal_base<Args...>;

    public:
        signal () {}

        template <typename SlotHolderClass>
        void connect (SlotHolderClass * pclass, void (SlotHolderClass::*pmemfun)(Args...))
        {
            std::lock_guard<mutex_type> lock(*this);
            connection<SlotHolderClass, Args...> * conn =
                    new connection<SlotHolderClass, Args...>(pclass, pmemfun);
            this->_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }

        void emit_signal (Args &&... args)
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = this->_connected_slots.cbegin();
            auto last = this->_connected_slots.cend();

            while (it != last) {
                auto next = it;
                ++next;

                (*it)->emit_signal(std::forward<Args>(args)...);

                it = next;
            }
        }

        void operator () (Args &&... args)
        {
            emit_signal(std::forward<Args>(args)...);
        }
    };
}; // struct sigslot

} // pfs

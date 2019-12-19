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

    class basic_has_slots;

    // see [std::make_unique](http://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)
    // `Possible Implementation` section.
    template<typename T, typename ...Args>
    static inline std::unique_ptr<T> make_unique (Args &&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename SignalType, typename ...Args>
    inline void emit_signal (SignalType & sig)
    {
        sig.emit_signal();
    }

////////////////////////////////////////////////////////////////////////////////
// connection_base
////////////////////////////////////////////////////////////////////////////////
    template <typename ...Args>
    class basic_connection
    {
    public:
        virtual ~basic_connection () {}
        virtual basic_has_slots * getdest () const = 0;
        virtual void emit_signal (Args &&...) = 0;
//         virtual basic_connection<Args...> * clone () = 0;
//         virtual basic_connection<Args...> * duplicate (basic_has_slots * pnewdest) = 0;
    };

    class basic_signal : public mutex_type
    {
    public:
        virtual ~basic_signal () {}
        virtual void slot_disconnect (basic_has_slots * pslot) = 0;

        // Used inside has_slots copy constructor
//         virtual void slot_duplicate (const basic_has_slots * poldslot, basic_has_slots * pnewslot) = 0;
    };

////////////////////////////////////////////////////////////////////////////////
// basic_has_slots
////////////////////////////////////////////////////////////////////////////////
    class basic_has_slots : public mutex_type
    {
    protected:
        using sender_set = std::set<basic_signal *>;
        using const_iterator = typename sender_set::const_iterator ;

        sender_set _senders;
        std::unique_ptr<callback_queue_type> _queue_ptr;
//         std::unique_ptr<callback_queue_type> _priority_queue_ptr;

    public:
        basic_has_slots ()
        {}

        virtual bool use_queued_slots () const = 0;

        virtual bool is_slave () const
        {
            return false;
        }

        virtual basic_has_slots * master () const
        {
            assert(this->is_slave());
            return 0;
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

        virtual ~basic_has_slots ()
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

//         callback_queue_type & priority_callback_queue ()
//         {
//             return *_priority_queue_ptr;
//         }
//
//         callback_queue_type const & priority_callback_queue () const
//         {
//             return *_priority_queue_ptr;
//         }
    };

////////////////////////////////////////////////////////////////////////////////
// has_slots
////////////////////////////////////////////////////////////////////////////////
    class has_slots : public basic_has_slots
    {
    public:
        has_slots () : basic_has_slots () {}
        virtual bool use_queued_slots () const override { return false; }
    };

    class has_queued_slots : public basic_has_slots
    {
    public:
        has_queued_slots () : basic_has_slots ()
        {
//             this->_priority_queue_ptr = make_unique<callback_queue_type>();
            this->_queue_ptr = make_unique<callback_queue_type>();
        }

        virtual bool use_queued_slots () const override { return true; }
    };

////////////////////////////////////////////////////////////////////////////////
// has_slave_slots
////////////////////////////////////////////////////////////////////////////////
    class has_slave_slots : public basic_has_slots
    {
        basic_has_slots * _master;

    public:
        has_slave_slots (basic_has_slots * master) : _master(master) {}
        virtual bool use_queued_slots () const override { return false; }
        virtual bool is_slave () const override { return true; }
        virtual basic_has_slots * master () const { return _master; }
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

//         void slot_duplicate (basic_has_slots const * oldtarget
//                 , basic_has_slots * newtarget)
//         {
//             std::lock_guard<mutex_type> lock(*this);
//             auto it = _connected_slots.begin();
//             auto last = _connected_slots.end();
//
//             while (it != last) {
//                 if ((*it)->getdest() == oldtarget) {
//                     _connected_slots.push_back((*it)->duplicate(newtarget));
//                 }
//
//                 ++it;
//             }
//         }

        void disconnect_all ()
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = _connected_slots.begin();
            auto last = _connected_slots.end();

            while (it != last) {
                (*it)->getdest()->signal_disconnect(this);
                delete *it;

                ++it;
            }

            _connected_slots.erase(_connected_slots.begin(), _connected_slots.end());
        }

        void disconnect (basic_has_slots * pclass)
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = _connected_slots.begin();
            auto last = _connected_slots.end();

            while (it != last) {
                if ((*it)->getdest() == pclass) {
                    delete *it;
                    _connected_slots.erase(it);
                    pclass->signal_disconnect(this);
                    return;
                }

                ++it;
            }
        }

        void slot_disconnect (basic_has_slots * pslot)
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = _connected_slots.begin();
            auto last = _connected_slots.end();

            while (it != last) {
                auto next = it;
                ++next;

                if ((*it)->getdest() == pslot) {
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
    template <typename HasSlotsClass, typename ...Args>
    class connection : public basic_connection<Args...>
    {
    public:
        connection ()
            : _pobject(0)
            , _pmemfun(0)
        {}

        connection (HasSlotsClass * pobject, void (HasSlotsClass::*pmemfun)(Args...))
            : _pobject(pobject)
            , _pmemfun(pmemfun)
        {}

//         virtual basic_connection<Args...> * clone ()
//         {
//             return new connection<HasSlotsClass, Args...>(*this);
//         }

//         virtual basic_connection<Args...> * duplicate (basic_has_slots * pnewdest)
//         {
//             return new connection<HasSlotsClass, Args...>(static_cast<HasSlotsClass *>(pnewdest), _pmemfun);
//         }

        virtual void emit_signal (Args &&... args)
        {
            using method_type = void (HasSlotsClass::*)(Args...);

            if (_pobject->use_queued_slots()) {
                HasSlotsClass * pobject = _pobject;
                method_type pmemfun = _pmemfun;

//                 _pobject->callback_queue().template push<method_type, HasSlotsClass
//                         , Args...>(std::move(pmemfun), std::move(pobject), std::forward<Args>(args)...);
                _pobject->callback_queue().push(std::move(pmemfun)
                        , std::move(pobject)
                        , std::forward<Args>(args)...);
            } else if (_pobject->is_slave()) {
//                 _pobject->master()->callback_queue().template push<HasSlotsClass
//                         , Args...>(_pmemfun, _pobject, std::forward<Args>(args)...);
            } else {
                (_pobject->*_pmemfun)(std::forward<Args>(args)...);
            }
        }

        virtual basic_has_slots * getdest () const
        {
            return _pobject;
        }

    private:
        HasSlotsClass * _pobject;
        void (HasSlotsClass::* _pmemfun)(Args...);
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

        template <typename HasSlotsClass>
        void connect (HasSlotsClass * pclass, void (HasSlotsClass::*pmemfun)(Args...))
        {
            std::lock_guard<mutex_type> lock(*this);
            connection<HasSlotsClass, Args...> * conn =
                    new connection<HasSlotsClass, Args...>(pclass, pmemfun);
            this->_connected_slots.push_back(conn);
            pclass->signal_connect(this);
        }

        void emit_signal (Args &&... args)
        {
            std::lock_guard<mutex_type> lock(*this);
            auto it = this->_connected_slots.cbegin();
            auto last = this->_connected_slots.cend();

            while(it != last) {
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

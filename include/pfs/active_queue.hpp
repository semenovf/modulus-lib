////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.19 Initial version (inhereted from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace pfs {

template <typename T>
using default_queue_container = std::deque<T>;

template <template <typename> class QueueContainer = default_queue_container
    // see [C++ concepts: BasicLockable](http://en.cppreference.com/w/cpp/concept/BasicLockable)>
    , typename BasicLockable = std::mutex
    , int GcThreshold = 256>
class active_queue
{
private:
    enum state_enum {
          FREE      // Candidate for memory deallocation
        , BUSY
        , PROCESSING
    };

public:
    // TODO Replace std::function<void ()> - too slow operations
    using value_type = std::pair<state_enum, std::function<void ()>>;

    using mutex_type = BasicLockable;
    using queue_container_type = QueueContainer<value_type>;
    using size_type = typename queue_container_type::size_type;
    using iterator = typename queue_container_type::iterator;

    static constexpr size_type GC_THRESHOLD = GcThreshold;

private:
    queue_container_type _queue;
    std::atomic_size_t   _count;
    mutable mutex_type   _mutex;

    // TODO Use cv to control queue fullness
    //mutable mutex_type      _non_empty_queue_mutex;
    std::condition_variable _non_empty_queue_cv;

private:
    void push_helper (std::function<void ()> && func)
    {
        std::unique_lock<mutex_type> locker(_mutex);

        if (_queue.size() > GC_THRESHOLD && _queue.begin()->first == FREE)
            gc();

        _queue.emplace_back(std::make_pair(BUSY, std::move(func)));

        ++_count;
        _non_empty_queue_cv.notify_one();
    }

    iterator front_busy ()
    {
        iterator pos = _queue.begin();
        iterator end = _queue.end();

        // TODO Need an optimization to access to the first BUSY element
        while (pos != end) {
            if (pos->first == BUSY) {
                pos->first = PROCESSING;
                break;
            }
            ++pos;
        }

        return pos;
    }

private:
    // Garbage collector
    void gc ()
    {
        iterator pos = _queue.begin();
        iterator end = _queue.end();

        while (pos != end && pos->first == FREE)
            ++pos;

        _queue.erase(_queue.begin(), pos);
    }

public:
    active_queue () : _count(0) {}

    virtual ~active_queue ()
    {
        // Do not call `call_all` method and `gc` methods
        clear();
    }

    bool empty () const
    {
        return _count == 0;
    }

    /**
     * @return Number of elements ready to call or in process (already calling).
     */
    size_type count () const
    {
        return _count;
    }

    size_type size () const
    {
        std::unique_lock<mutex_type> locker(_mutex);
        return _queue.size();
    }

    void clear ()
    {
        std::unique_lock<mutex_type> locker(_mutex);
        _queue.clear();
    }

    template <class F, typename ...Args>
    void push (F && f, Args &&... args)
    {
        push_helper(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    }

    void call ()
    {
        std::unique_lock<mutex_type> locker(_mutex);

        iterator pos = front_busy();

        if (pos != _queue.end()) {
            --_count;
            locker.unlock();

            // Process function
            pos->second();

            // Destroy item
            std::function<void ()> tmp;
            tmp.swap(pos->second);

            locker.lock();

            pos->first = FREE;
        }
    }

    void call (int max_count)
    {
        if (max_count > 0) {
            while (!this->empty() && max_count--)
                call();
        }
    }

    void call_all ()
    {
        while (!this->empty())
            call();
    }
};

} // namespace pfs

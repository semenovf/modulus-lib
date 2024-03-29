////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.19 Initial version (inherited from https://github.com/semenovf/pfs).
//      2020.01.13 Added support for module configuration (pass user data, application settings).
//      2020.05.21 Added support for dispatcher-dependent slave modules. (v2.1)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "active_queue.hpp"
#include "sigslot.hpp"
#include "timer.hpp"
#include "pfs/fmt.hpp"
#include "pfs/filesystem.hpp"
#include "pfs/dynamic_library.hpp"
#include <algorithm>
#include <atomic>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <cstdio>

#if _POSIX_C_SOURCE
#   include <cassert>
#   include <csignal>
#endif

#if _MSC_VER
#   include "pfs/windows.hpp"
#endif

#if ANDROID
#   include <android/log.h>
#endif

namespace pfs {

template <typename ResultType, typename T>
struct lexical_caster
{
    static ResultType cast (T const &);
};

template <typename T>
struct lexical_caster<std::string, T>
{
    static inline std::string cast (T const & arg)
    {
        using std::to_string;
        return to_string(arg);
    }
};

template <>
struct lexical_caster<std::string, std::string>
{
    static inline std::string cast(std::string const& arg)
    {
        return arg;
    }
};

#if _MSC_VER
template <>
struct lexical_caster<std::string, std::wstring>
{
    static inline std::string cast (std::wstring const & arg)
    {
        return windows::utf8_encode(arg.c_str(), static_cast<int>(arg.size()));
    }
};

template <>
struct lexical_caster<std::wstring, std::string>
{
    static inline std::wstring cast (std::string const & arg)
    {
        return windows::utf8_decode(arg.c_str(), static_cast<int>(arg.size()));
    }
};

template <>
struct lexical_caster<std::wstring, std::wstring>
{
    static inline std::wstring cast(std::wstring const & arg)
    {
        return arg;
    }
};

#endif

template <typename ResultType, typename T>
inline ResultType lexical_cast (T const & arg)
{
    using pfs::lexical_caster;
    return lexical_caster<ResultType, T>::cast(arg);
}

class basic_dispatcher
{
#if _POSIX_C_SOURCE
    std::vector<int> _quit_signums{
          SIGHUP
        , SIGINT
        , SIGQUIT
        , SIGILL
        , SIGABRT
        , SIGFPE
    };
#endif

public:
    virtual void quit () = 0;

    void set_quit_signals (std::vector<int> const & signums)
    {
#if _POSIX_C_SOURCE
        _quit_signums = signums;
#endif
    }

    /**
     * @brief Must be called from C-linkage user-defined signal handler
     *
     * @code
     *      static pfs::modulus<>::dispatcher * pdisp = nullptr;
     *      ...
     *      extern "C" void quit_app (int signum)
     *      {
     *          assert(pdisp);
     *          pdisp->signal_handler(signum);
     *      }
     *      ...
     *      pdisp = & dispatcher;
     *      pdisp->set_quit_handler(quit_app);
     * @endcode
     */
    void signal_handler (int signum)
    {
#if _POSIX_C_SOURCE
        auto ready = std::any_of(_quit_signums.cbegin(), _quit_signums.cend()
                , [signum] (int n) { return signum == n; });
        if (ready)
            this->quit();
#endif
    }

    /**
     */
    bool set_quit_handler (void (* handler)(int))
    {
#if _POSIX_C_SOURCE
        struct sigaction act;
        act.sa_handler = handler;
        act.sa_flags = 0;

        // Block every signal during the handler
        sigfillset(& act.sa_mask);

        auto ready = std::all_of(_quit_signums.cbegin(), _quit_signums.cend()
                , [& act] (int signum) {
                        return sigaction(signum, & act, nullptr) >= 0;
                });

        return ready;
#else
        return true;
#endif
    }

    basic_dispatcher ()
    {}

    virtual ~basic_dispatcher ()
    {
#if _POSIX_C_SOURCE
        // Restore default handlers for quit signals
        set_quit_handler(SIG_DFL);
#endif
    }
};

template <typename KeyType, typename ValueType>
using default_associative_container = std::map<KeyType, ValueType>;

template <typename T>
using default_sequence_container = std::list<T>;

template <typename T>
using default_queue_container =  active_queue_details::default_queue_container<T>;

using default_basic_lockable = std::mutex;
using default_timer_pool = timer_pool<>;

struct default_settings {};

struct simple_logger
{
#if ANDROID
    void info (std::string const & msg)
    {
        __android_log_write(ANDROID_LOG_INFO, "modulus", msg.c_str());
    }

    void debug (std::string const & msg)
    {
        __android_log_write(ANDROID_LOG_DEBUG, "modulus", msg.c_str());
    }

    void warn (std::string const & msg)
    {
        __android_log_write(ANDROID_LOG_WARN, "modulus", msg.c_str());
    }

    void error (std::string const & msg)
    {
        __android_log_write(ANDROID_LOG_ERROR, "modulus", msg.c_str());
    }
#else
    void info (std::string const & msg)  { fprintf(stdout, "%s\n", msg.c_str()); }
    void debug (std::string const & msg) { fprintf(stdout, "-- %s\n", msg.c_str()); }
    void warn (std::string const & msg)  { fprintf(stderr, "WARN: %s\n", msg.c_str()); }
    void error (std::string const & msg) { fprintf(stderr, "ERROR: %s\n", msg.c_str()); }
#endif
};

template <bool OldBehaviour = true
        , typename StringType = std::string
        , typename LoggerType = simple_logger
        , typename SettingsType = default_settings
        , typename TimerPool = default_timer_pool

        , typename ActiveQueueFunctionItem = std::function<void ()>

        // For storing API map and module specs
        , template <typename, typename> class AssociativeContainer = default_associative_container

        // For storing runnable modules and threads
        , template <typename> class SequenceContainer = default_sequence_container

        // Active queue configuration
        , template <typename> class QueueContainer = default_queue_container

        // see [C++ concepts: BasicLockable](http://en.cppreference.com/w/cpp/concept/BasicLockable)>
        , typename BasicLockable = default_basic_lockable>
struct modulus
{
    class basic_module;
    class module;
    class async_module;
    class dispatcher;

    using settings_type = SettingsType;
    using logger_type = LoggerType;
    using string_type = StringType;
    using timer_pool_type = TimerPool;
    using timer_id = typename timer_pool_type::timer_id;
    using callback_queue_type = active_queue<ActiveQueueFunctionItem, QueueContainer>;

    using sigslot_ns = sigslot<callback_queue_type, BasicLockable>;
    using emitter_type  = typename sigslot_ns::template signal<>;

    using detector_handler = void (basic_module::*)(void *);
    typedef struct { int id; void * emitter; }            emitter_mapper_pair;
    typedef struct { int id; detector_handler detector; } detector_mapper_pair;

    using module_ctor_t = basic_module * (*)(void);
    using module_dtor_t = void  (*)(basic_module *);

    // see [std::make_unique](http://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)
    // `Possible Implementation` section.
    template<typename T, typename ...Args>
    static inline std::unique_ptr<T> make_unique (Args &&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // see [Alternatives of static_pointer_cast for unique_ptr](https://stackoverflow.com/questions/36120424/alternatives-of-static-pointer-cast-for-unique-ptr/36120483)
    template <typename TargetType, typename SourceType>
    static inline std::unique_ptr<TargetType> static_unique_pointer_cast (std::unique_ptr<SourceType> && src)
    {
        return std::unique_ptr<TargetType>{static_cast<TargetType *>(src.release())};
    }

    template <typename T>
    static inline string_type concat (T const & arg)
    {
        using pfs::lexical_cast;
        return lexical_cast<string_type>(arg);
    }

    template <typename T, typename ...Ts>
    static inline string_type concat (T const & arg, Ts const &... args)
    {
        using pfs::lexical_cast;
        return lexical_cast<string_type>(arg) + concat(args...);
    }

    struct detector_pair
    {
        basic_module *   mod;
        detector_handler detector;

        detector_pair () : mod(0), detector(0) {}
        detector_pair (basic_module * p, detector_handler d) : mod(p), detector(d) {}
    };

    struct module_spec
    {
        std::shared_ptr<basic_module>    pmodule;
        std::shared_ptr<pfs::dynamic_library> pdl;
    };

    struct module_deleter
    {
        module_dtor_t _deleter;

        module_deleter (module_dtor_t deleter)
            : _deleter(deleter)
        {}

        void operator () (basic_module * p) const
        {
            _deleter(p);
        }
    };

    struct api_item_type;

    using api_map_type = AssociativeContainer<int, api_item_type *>;
    using module_spec_map_type = AssociativeContainer<string_type, module_spec>;
    using thread_function = int (basic_module::*)(settings_type const &);
    using runnable_sequence_type = SequenceContainer<std::pair<basic_module *, thread_function>>;
    using thread_sequence_type = SequenceContainer<std::unique_ptr<std::thread>>;

////////////////////////////////////////////////////////////////////////////////
// basic_module
////////////////////////////////////////////////////////////////////////////////
    class basic_module : public sigslot_ns::basic_slot_holder
    {
        friend class dispatcher;
        friend class async_module;

    public:
        using string_type = StringType;
        using emitter_mapper_pair = modulus::emitter_mapper_pair;

        // MSVC do not want 'detector_mapper_pair' definition in upper level, so duplicate here
        typedef struct { int id; detector_handler detector; } detector_mapper_pair;
        //using detector_mapper_pair = modulus::detector_mapper_pair;

        using detector_handler = modulus::detector_handler;
        using thread_function = typename modulus::thread_function;

    protected:
        string_type  _name;
        dispatcher * _pdispatcher = nullptr;
        bool         _started = false;

    public:
        void quit ()
        {
            _pdispatcher->quit();
        }

        void log_info (string_type const & s)
        {
            _pdispatcher->log_info(this, s);
        }

        void log_debug (string_type const & s)
        {
            _pdispatcher->log_debug(this, s);
        }

        void log_warn (string_type const & s)
        {
            _pdispatcher->log_warn(this, s);
        }

        void log_error (string_type const & s)
        {
            _pdispatcher->log_error(this, s);
        }

        /**
         * Acquire timer with callback processed from module's queue.
         */
        timer_id acquire_timer (double delay
                , double period
                , typename timer_pool_type::callback_type && callback)
        {
            return _pdispatcher->acquire_timer(this
                    , delay
                    , period
                    , std::forward<typename timer_pool_type::callback_type>(callback));
        }

        /**
         * Acquire timer with callback processed from dispatcher queue
         */
        timer_id acquire_timer_dispatcher (double delay
                , double period
                , typename timer_pool_type::callback_type && callback)
        {
            return _pdispatcher->acquire_timer(delay, period
                    , std::forward<typename timer_pool_type::callback_type>(callback));
        }

        inline void destroy_timer (timer_id id)
        {
            _pdispatcher->destroy_timer(id);
        }

    protected:
        basic_module () noexcept : sigslot_ns::basic_slot_holder()
        {}

        void set_dispatcher (dispatcher * pdisp) noexcept
        {
            _pdispatcher = pdisp;
        }

        void set_name (string_type const & name) noexcept
        {
            _name = name;
        }

        virtual bool on_start_wrapper (settings_type const & settings)
        {
            _started = this->on_start(settings);

            if (! _started) {
                _pdispatcher->log_error(concat(_name
                    , string_type(": failed to start module")));
            }

            return _started;
        }

        virtual void on_finish_wrapper ()
        {
            if (! this->on_finish()) {
                _pdispatcher->log_warn(concat(_name
                    , string_type(": failed to finalize module")));
            }
        }

    public:
        virtual ~basic_module () {}

        string_type const & name() const noexcept
        {
            return _name;
        }

        bool is_registered () const noexcept
        {
            return _pdispatcher != 0 ? true : false;
        }

        bool is_started () const noexcept
        {
            return _started;
        }

        dispatcher * get_dispatcher () noexcept
        {
            return _pdispatcher;
        }

        dispatcher const * get_dispatcher () const noexcept
        {
            return _pdispatcher;
        }

        virtual emitter_mapper_pair const * get_emitters (int & count)
        {
            count = 0;
            return nullptr;
        }

        virtual detector_mapper_pair const * get_detectors (int & count)
        {
            count = 0;
            return nullptr;
        }

        bool is_quit () const
        {
            return _pdispatcher->is_quit();
        }

        /**
         * @brief Module's on_loaded() method called after loaded and before registration.
         * @return
         */
        virtual bool on_loaded ()
        {
            return true;
        }

        /**
         * @brief Module's on_start() method called after loaded and connection completed.
         */
        virtual bool on_start (settings_type const &)
        {
            return true;
        }

        virtual bool on_finish ()
        {
            return true;
        }
    };// basic_module

////////////////////////////////////////////////////////////////////////////////
// module
////////////////////////////////////////////////////////////////////////////////
    class module : public basic_module
    {
    protected:
        module () noexcept : basic_module()
        {}

    public:
        bool use_queued_slots () const override final
        {
            return false;
        }

        bool is_slave () const override final
        {
            return false;
        }

        virtual typename sigslot_ns::basic_slot_holder * master () const
        {
            return this->_pdispatcher;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// async_module
////////////////////////////////////////////////////////////////////////////////
    class async_module : public basic_module
    {
        friend class dispatcher;

        using slaves_sequence_type = SequenceContainer<basic_module *>;

        slaves_sequence_type _slaves;

    private:
        bool on_start_wrapper (settings_type const & settings) override
        {
            bool success = true;

            // 1. Launch on_start() method for async module.
            this->_started = basic_module::on_start_wrapper(settings);

            if (this->_started) {
                this->_pdispatcher->module_started(this->name());
            } else  {
                success = false;
            }

            // 2. Launch on_start() methods for linked modules.
            if (success) {
                for (auto slave: _slaves) {
                    if (slave->on_start_wrapper(settings)) {
                        this->_pdispatcher->module_started(slave->name());
                    } else {
                        success = false;
                    }
                }
            }

            // 3. Notify dispatcher about starting stage finished
            // for this module.
            this->_pdispatcher->notify_module_started(success);

            return success;
        }

        void on_finish_wrapper () override
        {
            for (auto slave: _slaves)
                slave->on_finish_wrapper();

            basic_module::on_finish_wrapper();
        }

        // Called from dispatcher
        int thread_function_wrapper (settings_type const & settings)
        {
            // Steps 1, 2, 3
            if (!on_start_wrapper(settings))
                return dispatcher::exit_status::failure;

            // 4. Wait special condition (all modules finished starting stage)
            // from dispatcher.
            while (!this->_pdispatcher->all_modules_started())
                std::this_thread::sleep_for(std::chrono::microseconds(100));

            if (! this->_pdispatcher->_atomic_modules_started_successfully.load()) {
                this->quit();
                return dispatcher::exit_status::failure;
            }

            // 5. If OK process deferred (queued events/callbacks) and for
            // receiving quit() if starting failure.
            process_events();

            if (this->is_quit())
                return dispatcher::exit_status::failure;

            auto rc = run();

            // Process remaining events
            process_events();

            on_finish_wrapper();

            return dispatcher::exit_status::success;
        }

    public:
        async_module () : basic_module()
        {
            this->_queue_ptr = make_unique<callback_queue_type>();
        }

        bool use_queued_slots () const override final
        {
            return true;
        }

        virtual bool is_slave () const override final
        {
            return false;
        }

        /**
         * @see process_events.
         */
        void call_all ()
        {
            this->callback_queue().call_all();
        }

        /**
         * @brief process_events() is a synonym for call_all().
         * @see call_all.
         */
        void process_events ()
        {
            this->call_all();
        }

        void process_events (int max_count)
        {
            this->callback_queue().call(max_count);
        }

        bool has_pending_events () const
        {
            return !this->callback_queue().empty();
        }

        void add_slave (basic_module * m)
        {
            _slaves.push_back(m);
        }

        virtual bool on_before_run ()
        {
            return true;
        }

        virtual void on_after_run ()
        {}

        virtual int run ()
        {
            if (!on_before_run())
                return -1;

            auto pqueue = & this->callback_queue();
            auto wait_period = this->_pdispatcher->wait_period();

            while (! this->is_quit()) {
                pqueue->wait_for(wait_period);
                pqueue->call_all();
            }

            on_after_run();
            return 0;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// slave_module
////////////////////////////////////////////////////////////////////////////////
    class slave_module : public basic_module
    {
        friend class async_module;

        typename sigslot_ns::basic_slot_holder * _master {nullptr};

    public:
        slave_module () : basic_module()
        {}

        bool use_queued_slots () const override final
        {
            return false;
        }

        bool is_slave () const override final
        {
            return true;
        }

        typename sigslot_ns::basic_slot_holder * master () const override
        {
            return _master;
        }

        void set_master (async_module * master)
        {
            _master = master;
            master->add_slave(this);
        }

        void set_master (dispatcher * master)
        {
            _master = master;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// SigSlot Mapper
////////////////////////////////////////////////////////////////////////////////
    struct basic_sigslot_mapper
    {
        virtual ~basic_sigslot_mapper () {}
        virtual void connect_all () = 0;
        virtual void disconnect_all () = 0;
        virtual void append_emitter (emitter_type * em) = 0;
        virtual void append_detector (basic_module * m, detector_handler d) = 0;
    };

    struct api_item_type
    {
        int id;
        std::unique_ptr<basic_sigslot_mapper> mapper;
        string_type desc;
    };

    template <typename EmitterType, typename DetectorType>
    struct sigslot_mapper : basic_sigslot_mapper
    {
        using emitter_sequence = SequenceContainer<EmitterType *>;
        using detector_sequence = SequenceContainer<detector_pair>;

        emitter_sequence  emitters;
        detector_sequence detectors;

        virtual void connect_all () override
        {
            if (emitters.size() == 0 || detectors.size() == 0)
                return;

            auto last_emitter_it = emitters.cend();
            auto last_detector_it = detectors.cend();

            for (auto ite = emitters.cbegin(); ite != last_emitter_it; ++ite) {
                for (auto itd = detectors.cbegin(); itd != last_detector_it; ++itd) {
                    EmitterType * em = *ite;
                    em->connect(itd->mod, reinterpret_cast<DetectorType> (itd->detector));
                }
            }
        }

        virtual void disconnect_all () override
        {
            auto last = emitters.cend();

            for (auto it = emitters.cbegin(); it != last; it++) {
                EmitterType * em = *it;
                em->disconnect_all();
            }
        }

        virtual void append_emitter (emitter_type * e) override
        {
            emitters.push_back(reinterpret_cast<EmitterType*>(e));
        }

        virtual void append_detector (basic_module * m, detector_handler d) override
        {
            detectors.push_back(detector_pair(m, d));
        }
    };

    template <typename ...Args>
    static std::unique_ptr<basic_sigslot_mapper> make_mapper ()
    {
        using concrete_mapper_type = sigslot_mapper<
                  typename sigslot_ns::template signal<Args...>
                , void (basic_module::*)(Args...)>;
        return static_unique_pointer_cast<basic_sigslot_mapper>(make_unique<concrete_mapper_type>());
    }

////////////////////////////////////////////////////////////////////////////////
// dispatcher
////////////////////////////////////////////////////////////////////////////////
    class dispatcher final : public basic_dispatcher, public sigslot_ns::queued_slot_holder
    {
        friend class basic_module;
        friend class module;
        friend class async_module;

        using slaves_sequence_type = SequenceContainer<basic_module *>;

    public:
        using string_type = modulus::string_type;
        using logger_type = modulus::logger_type;

        struct exit_status
        {
            enum {
                  success = 0
                , failure = -1
            };
        };

        typename sigslot_ns::template signal<string_type const &> module_registered;
        typename sigslot_ns::template signal<string_type const &> module_unregistered;
        typename sigslot_ns::template signal<string_type const &> module_started;

    private:
        void connect_all ()
        {
            auto first = _api.begin();
            auto last  = _api.end();

            for (; first != last; ++first) {
                first->second->mapper->connect_all();
            }
        }

        void disconnect_all ()
        {
            auto first = _api.begin();
            auto last  = _api.end();

            for (; first != last; ++first) {
                first->second->mapper->disconnect_all();
            }
        }

        void unregister_all ()
        {
            _runnable_modules.clear();

            auto first = _module_spec_map.begin();
            auto last = _module_spec_map.end();

            for (; first != last; ++first) {
                module_spec & modspec = first->second;
                std::shared_ptr<basic_module> & pmodule = modspec.pmodule;
                log_debug(concat(pmodule->name(), string_type(": unregistered")));

                this->module_unregistered(pmodule->name());

                // Need to destroy pmodule before dynamic library will be
                // destroyed automatically
                pmodule.reset();
            }

            _module_spec_map.clear();
        }

        bool start ()
        {
            assert(_psettings);

            bool success = true;

            // Launch `on_start` method for regular modules
            auto first = _module_spec_map.begin();
            auto last  = _module_spec_map.end();

            for (; first != last; ++first) {
                module_spec modspec = first->second;
                std::shared_ptr<basic_module> pmodule = modspec.pmodule;

                bool is_regular_module = !pmodule->is_slave()
                    && !pmodule->use_queued_slots();

                if (is_regular_module) {
                    if (! pmodule->on_start_wrapper(*_psettings))
                        success = false;
                    else
                        this->module_started(pmodule->name());
                }
            }

            // Launch `on_start` method for master module (if specified)
            // and his linked modules (see `thread_function_wrapper` also)
            if (success && _main_module_ptr)
                success = _main_module_ptr->on_start_wrapper(*_psettings);

            // Redirect log ouput.
            if (success) {
                info_printer  = & dispatcher::async_print_info;
                debug_printer = & dispatcher::async_print_debug;
                warn_printer  = & dispatcher::async_print_warn;
                error_printer = & dispatcher::async_print_error;
            }

            return success;
        }

        void finalize (bool was_success_start)
        {
            // Destroy timer pool
            _ptimer_pool.reset(nullptr);

            if (was_success_start)
                this->_queue_ptr->call_all();
            else
                this->_queue_ptr->clear();

            info_printer  = & dispatcher::sync_print_info;
            debug_printer = & dispatcher::sync_print_debug;
            warn_printer  = & dispatcher::sync_print_warn;
            error_printer = & dispatcher::sync_print_error;

            if (_module_spec_map.size() > 0) {
                auto imodule      = _module_spec_map.begin();
                auto imodule_last = _module_spec_map.end();

                for (; imodule != imodule_last; ++imodule) {
                    module_spec & modspec = imodule->second;

                    // Launch on_finish() method for regular and dispatcher's
                    // linked modules. Async modules with linked modules
                    // finalized at async module's thread function
                    if (modspec.pmodule->is_started()) {

                        bool is_regular_module = !modspec.pmodule->is_slave()
                            && !modspec.pmodule->use_queued_slots();

                        bool is_dispatcher_slave_module = modspec.pmodule->is_slave()
                            && modspec.pmodule->master() == this;

                        if (is_regular_module || is_dispatcher_slave_module) {
                            // Do not 'Call deferred callbacks in modules'
                            // to avoid segmentation fault when signal handlers
                            // depends on varibales which lifetime limited by
                            // run() method

                            // // Call deferred callbacks in modules
                            // if (modspec.pmodule->use_queued_slots())
                            //      static_cast<async_module *>(modspec.pmodule.get())->call_all();

                            modspec.pmodule->on_finish_wrapper();
                        }
                    }
                }

                // Launch on_finish() method for main module and it's
                // linked modules.
                if (_main_module_ptr)
                    _main_module_ptr->on_finish_wrapper();

                disconnect_all();
                unregister_all();
            }

            if (was_success_start)
                this->_queue_ptr->call_all();
            else
                this->_queue_ptr->clear();
        }

        /*
         * Internal dispatcher loop.
         */
        void run ()
        {
            auto first = _module_spec_map.begin();
            auto last  = _module_spec_map.end();

            bool ok = true;

            // 1. Launch `on_start` method for dispatcher's slave modules
            for (; first != last; ++first) {
                module_spec modspec = first->second;
                std::shared_ptr<basic_module> pmodule = modspec.pmodule;

                bool is_dispatcher_slave_module = pmodule->is_slave()
                    && pmodule->master() == this;

                if (is_dispatcher_slave_module) {
                    if (! pmodule->on_start_wrapper(*_psettings)) {
                        ok = false;
                    } else {
                        this->module_started(pmodule->name());
                    }
                }
            }

            // 2. Notify about starting stage finished
            // for this module (the result is no matter).
            notify_module_started(ok);

            // 3. Wait special condition (all modules finished starting stage)
            // from dispatcher.
            while (!all_modules_started())
                std::this_thread::sleep_for(std::chrono::microseconds(100));

            if (! _atomic_modules_started_successfully.load())
                this->quit();

            // Run main loop
            auto pqueue = & this->callback_queue();
            pqueue->call_all();

            if (OldBehaviour) {
                while (! _quit_flag) {
                    if (pqueue->empty()) {
                        std::this_thread::sleep_for(std::chrono::microseconds(100));
                        continue;
                    }

                    // TODO Make configurable constant (5)
                    pqueue->call(5);
                }
            } else {
                // New behaviour
                while (! _quit_flag) {
                    pqueue->wait_for(_wait_period);
                    pqueue->call_all();
                }
            }

            // Destroy all timers before modules will destroyed
            _ptimer_pool->destroy_all();

            pqueue->call_all();
        }

        int exec_main ()
        {
            int r = exit_status::success;

            thread_sequence_type thread_pool;

            auto runnable_it   = _runnable_modules.begin();
            auto runnable_last = _runnable_modules.end();

            for (; runnable_it != runnable_last; ++runnable_it) {
                basic_module * m = runnable_it->first;
                thread_function tfunc = runnable_it->second;

                // Run module if it is not a main module
                if (m != _main_module_ptr)
                    thread_pool.emplace_back(new std::thread(tfunc, m, *_psettings));
            }

            // Run module if it is a main module
            if (_main_module_ptr) {
                // Run dispatcher loop in separate thread
                std::thread dthread(& dispatcher::run, this);

                // And call main module function
                if (_main_module_ptr->use_queued_slots()) {
                    r = static_cast<async_module *>(_main_module_ptr)->run();
                }

                dthread.join();
            } else {
                this->run();
            }

            for (auto & pth: thread_pool) {
                if (pth->joinable())
                    pth->join();
            }

            return r;
        }

    public:
        dispatcher (dispatcher const &) = delete;
        dispatcher & operator = (dispatcher const &) = delete;

        dispatcher (api_item_type * mapper, int n
                , settings_type & settings
                , logger_type & logger)
            : basic_dispatcher()
            , info_printer(& dispatcher::sync_print_info)
            , debug_printer(& dispatcher::sync_print_debug)
            , warn_printer(& dispatcher::sync_print_warn)
            , error_printer(& dispatcher::sync_print_error)
            , _quit_flag(0)
            , _main_module_ptr(nullptr)
            , _psettings(& settings)
            , _plog(& logger)
        {
            assert(_plog);

            // Initialize timer pool
            _ptimer_pool.reset(new timer_pool_type);

            register_api(mapper, n);
        }

        virtual ~dispatcher ()
        {
            finalize(false);
        }

        virtual void quit () override
        {
            _quit_flag.store(1);
        }

        bool is_quit () const
        {
            return (_quit_flag.load() != 0);
        }

        void set_wait_period (intmax_t value)
        {
            _wait_period = value;
        }

        intmax_t wait_period () const noexcept
        {
            return _wait_period;
        }

        int exec ()
        {
            int r = exit_status::failure;

            connect_all();

            auto success_start = start();

            if (success_start)
                r = exec_main();

            finalize(success_start);

            return r;
        }

        void register_api (api_item_type * mapper, int n)
        {
            for (int i = 0; i < n; ++i) {
                _api.insert(std::make_pair(mapper[i].id, & mapper[i]));
            }
        }

    //    /**
    //     * @brief Output summary of emitters/detectors utilization.
    //     * TODO Implement
    //     */
    //    void print_api_connections () {}
    //
    //    /**
    //     * @brief Output summary of incomplete emitters/detectors utilization.
    //     * TODO Implement
    //     */
    //    void print_api_incomplete_connections () {}

    private:
        bool register_module_helper (std::pair<string_type, string_type> const & name
                , module_spec const & modspec)
        {
            int nemitters, ndetectors;

            if (!modspec.pmodule)
                return false;

            auto pmodule = modspec.pmodule;
            auto const & module_name = name.first;
            auto const & dep_module_name = name.second;

            if (_module_spec_map.find(module_name) != _module_spec_map.end()) {
                log_error(concat(module_name
                    , string_type(": module already registered")));
                return false;
            }

            pmodule->set_dispatcher(this);
            pmodule->set_name(module_name);

            if (pmodule->use_queued_slots()) {
                this->_runnable_modules.emplace_back(std::make_pair(& *modspec.pmodule
                    , static_cast<typename async_module::thread_function>(
                        & async_module::thread_function_wrapper)));
            } else {
                if (pmodule->is_slave()) {
                    // Master is dispatcher
                    if (dep_module_name == "") {
                        std::static_pointer_cast<slave_module>(modspec.pmodule)
                            ->set_master(this);
                    } else {
                        basic_module * master = this->find_registered_module(dep_module_name);

                        if (!master) {
                            log_error(concat(dep_module_name
                                , string_type(": module not found")));
                            return false;
                        }

                        if (!master->use_queued_slots()) {
                            log_error(concat(dep_module_name
                                , string_type(": module must be asynchronous")));
                            return false;
                        }

                        std::static_pointer_cast<slave_module>(modspec.pmodule)
                            ->set_master(static_cast<async_module *>(master));
                    }
                }
            }

            if (!pmodule->on_loaded()) {
                log_error(concat(pmodule->name()
                    , string_type(": on_loaded stage failed")));
                return false;
            }

            emitter_mapper_pair const * emitters = pmodule->get_emitters(nemitters);
            detector_mapper_pair const * detectors = reinterpret_cast<detector_mapper_pair const*>(pmodule->get_detectors(ndetectors));

            auto it_end = _api.end();

            if (emitters) {
                for (int i = 0; i < nemitters; ++i) {
                    int emitter_id(emitters[i].id);
                    typename api_map_type::iterator it = _api.find(emitter_id);

                    if (it != it_end) {
                        it->second->mapper->append_emitter(reinterpret_cast<emitter_type *>(emitters[i].emitter));
                    } else {
                        log_warn(concat(pmodule->name()
                            , string_type(": emitter '")
                            , lexical_cast<string_type>(emitters[i].id)
                            , string_type("' not found while registering module, "
                                "may be signal/slot mapping is not supported for this application")));
                    }
                }
            }

            if (detectors) {
                for (int i = 0; i < ndetectors; ++i) {
                    int detector_id(detectors[i].id);
                    typename api_map_type::iterator it = _api.find(detector_id);

                    if (it != it_end) {
                        it->second->mapper->append_detector(pmodule.get(), detectors[i].detector);
                    } else {
                        log_warn(concat(pmodule->name()
                            , string_type(": detector '")
                            , lexical_cast<string_type>(detectors[i].id)
                            , string_type("' not found while registering module, "
                                "may be signal/slot mapping is not supported for this application")));
                    }
                }
            }

            _module_spec_map.insert(std::make_pair(pmodule->name(), modspec));
            log_debug(concat(pmodule->name(), string_type(": registered")));

            this->module_registered(pmodule->name());

            return true;
        }

        template <typename PathIt>
        module_spec module_for_path (filesystem::path const & path, PathIt first, PathIt last)
        {
            static char const * module_ctor_name = "__module_ctor__";
            static char const * module_dtor_name = "__module_dtor__";

            filesystem::path dlpath(path);

            if (path.is_relative()) {
                if (first == last) {
                    dlpath = fs::path(".") / path;
                } else {
                    while (first != last) {
                        if (fs::exists(*first / path)) {
                            dlpath = *first / path;
                            break;
                        }
                        ++first;
                    }
                }
            }

            if (!filesystem::exists(dlpath)) {
#if ANDROID
                __android_log_print(ANDROID_LOG_ERROR, "modulus"
                    , "module not found: %s\n", dlpath.c_str());
#else
                fmt::print(stderr, "module not found: {}\n", filesystem::utf8_encode(dlpath));
#endif
                return module_spec{};
            }


            std::error_code ec;
            auto pdl = std::make_shared<pfs::dynamic_library>(dlpath, ec);

            if (ec) {
                // This is a critical section, so log output must not depends on logger
#if ANDROID
                __android_log_print(ANDROID_LOG_ERROR, "modulus"
                    , "open module failed: %s: %s\n"
                    , dlpath.c_str()
                    , ec.message().c_str());
#else
            fmt::print(stderr, "open module failed: {}: {}\n"
                , filesystem::utf8_encode(dlpath)
                , ec.message());

#endif
                return module_spec{};
            }

            auto module_ctor = pdl->resolve<basic_module*(void)>(module_ctor_name, ec);

            if (ec) {
                log_error(concat(dlpath.native()
                    , string_type(": failed to resolve constructor `")
                    , string_type(module_ctor_name)
                    , string_type("' for module: ")
                    , ec.message()));

                return module_spec{};
            }

            auto module_dtor = pdl->resolve<void(basic_module *)>(module_dtor_name, ec);

            if (ec) {
                log_error(concat(dlpath.native()
                    , string_type(": failed to resolve `dtor' for module: ")
                    , ec.message()));

                return module_spec{};
            }

            basic_module * ptr = module_ctor();

            if (!ptr)
                return module_spec{};

            module_spec modspec;
            modspec.pdl = pdl;
            modspec.pmodule = std::shared_ptr<basic_module>(ptr, module_deleter(module_dtor));

            return modspec;
        }

        template <typename PathIt>
        module_spec module_for_name (std::pair<string_type, string_type> const & name
                , PathIt first, PathIt last)
        {
            auto dl_filename = lexical_cast<std::string>(name.first);
            auto modpath = pfs::dynamic_library::build_filename(dl_filename);
            return module_for_path<PathIt>(modpath, first, last);
        }

    public:
        template <typename ModuleClass, typename ...Args>
        bool register_module (std::pair<string_type, string_type> const & name
                , Args &&... args)
        {
            module_spec modspec;
            auto pmodule = std::make_shared<ModuleClass>(std::forward<Args>(args)...);
            modspec.pmodule = std::static_pointer_cast<basic_module>(pmodule);

            return register_module_helper(name, modspec);
        }

        /**
         * @brief Register module represented as shared object specified by @a path.
         */
        template <typename PathIt>
        bool register_module_for_path (string_type const & path
                , std::pair<string_type, string_type> const & name
                , PathIt first, PathIt last)
        {
            module_spec modspec = module_for_path<PathIt>(path, first, last);

            if (modspec.pmodule)
                return register_module_helper(name, modspec);

            return false;
        }

        bool register_module_for_path (string_type const & path
                , std::pair<string_type, string_type> const & name)
        {
            fs::path const * nullpath = nullptr;
            return register_module_for_path(path, name, nullpath, nullpath);
        }

        /**
         * @brief Register module represented as shared object specified by @a name.
         *
         * @details Actual path for shared object based on @a name constructed
         */
        template <typename PathIt>
        bool register_module_for_name (std::pair<string_type, string_type> const & name
                , PathIt first, PathIt last)
        {
            module_spec modspec = module_for_name<PathIt>(name, first, last);

            if (modspec.pmodule)
                return register_module_helper(name, modspec);

            return false;
        }

        bool register_module_for_name (std::pair<string_type, string_type> const & name)
        {
            fs::path const * nullpath = nullptr;
            return register_module_for_name(name, nullpath, nullpath);
        }

        /**
         * @brief Sets async module @a name to execute in main thread.
         */
        bool set_main_module (string_type const & name)
        {
            basic_module * main_module = find_registered_module(name);

            if (!main_module) {
                log_error(concat(name, string_type(": main module not found")));
                return false;
            }

            if (!main_module->use_queued_slots()) {
                log_error(concat(name, string_type(": main module must be asynchronous")));
                return false;
            }

            _main_module_ptr = main_module;
            return true;
        }

        size_t count () const
        {
            return _module_spec_map.size();
        }

        struct timer_callback_helper
        {
            typename timer_pool_type::callback_type callback;
            basic_module * m {nullptr};
            dispatcher * d {nullptr};
            timer_id timerid {0};
            bool is_single_shot_timer {false};

            void operator () ()
            {
                if (m) {
                    if (m->use_queued_slots()) {
                        // Do not std::move callback as it may be periodic
                        m->callback_queue().push(callback);
                    } else if (m->is_slave()) {
                        // Do not std::move callback as it may be periodic
                        m->master()->callback_queue().push(callback);
                    } else {
                        callback();
                    }

                    if (is_single_shot_timer)
                        m->destroy_timer(timerid);

                } else if (d) {
                    d->callback_queue().push(callback);

                    if (is_single_shot_timer)
                        d->destroy_timer(timerid);
                } else {
                    // NOTE This will never be called in modulus
                    callback();
                }
            }
        };

        void notify_module_started (bool ok)
        {
            if (!ok)
                _atomic_modules_started_successfully.store(false);
            ++_atomic_modules_started_counter;
        }

        bool all_modules_started () const
        {
            return _atomic_modules_started_counter == _runnable_modules.size();
        }

        /**
         * Acquire timer with callback processed from module's queue
         * or called direct if @a m is @c nullptr.
         */
        inline timer_id acquire_timer (
                  basic_module * m
                , double delay
                , double period
                , typename timer_pool_type::callback_type && callback)
        {
            timer_callback_helper timer_callback;
            timer_callback.m = m;
            timer_callback.callback = std::move(callback);
            timer_callback.is_single_shot_timer = (period == double{0});
            timer_callback.timerid = _ptimer_pool->create(delay, period, std::move(timer_callback));

            return timer_callback.timerid;
        }

        /**
         * Acquire timer with callback processed from dispatcher queue
         */
        inline timer_id acquire_timer (
                  double delay
                , double period
                , typename timer_pool_type::callback_type && callback)
        {
            timer_callback_helper timer_callback;
            timer_callback.d = this;
            timer_callback.callback = std::move(callback);
            timer_callback.is_single_shot_timer = (period == double{0});
            timer_callback.timerid = _ptimer_pool->create(delay, period, std::move(timer_callback));
            return timer_callback.timerid;
        }

        inline void destroy_timer (timer_id id)
        {
            // _ptimer_pool may be already destroyed (i.e. on finalize())
            if (_ptimer_pool)
                _ptimer_pool->destroy(id);
        }

    public: // slots
        void log_info (basic_module const * m, string_type const & s)
        {
            (this->*info_printer)(m, s);
        }

        void log_debug (basic_module const * m, string_type const & s)
        {
            (this->*debug_printer)(m, s);
        }

        void log_warn (basic_module const * m, string_type const & s)
        {
            (this->*warn_printer)(m, s);
        }

        void log_error (basic_module const * m, string_type const & s)
        {
            (this->*error_printer)(m, s);
        }

        void log_info (string_type const & s)
        {
            log_info(0, s);
        }

        void log_debug (string_type const & s)
        {
            log_debug(0, s);
        }

        void log_warn (string_type const & s)
        {
            log_warn(0, s);
        }

        void log_error (string_type const & s)
        {
            log_error(0, s);
        }

    protected:
        void sync_print_info (basic_module const * m, string_type const & s)
        {
            _plog->info(m != 0 ? m->name() + ": " + s : s);
        }

        void sync_print_debug (basic_module const * m, string_type const & s)
        {
            _plog->debug(m != 0 ? m->name() + ": " + s : s);
        }

        void sync_print_warn (basic_module const * m, string_type const & s)
        {
            _plog->warn(m != 0 ? m->name() + ": " + s : s);
        }

        void sync_print_error (basic_module const * m, string_type const & s)
        {
            _plog->error(m != 0 ? m->name() + ": " + s : s);
        }

        void async_print_info (basic_module const * m, string_type const & s)
        {
            this->_queue_ptr->push(& logger_type::info
                    , _plog
                    , (m != 0 ? m->name() + ": " + s : s));
        }

        void async_print_debug (basic_module const * m, string_type const & s)
        {
            this->_queue_ptr->push(& logger_type::debug
                    , _plog
                    , (m != 0 ? m->name() + ": " + s : s));
        }

        void async_print_warn (basic_module const * m, string_type const & s)
        {
            this->_queue_ptr->push(& logger_type::warn
                    , _plog
                    , (m != 0 ? m->name() + ": " + s : s));
        }

        void async_print_error (basic_module const * m, string_type const & s)
        {
            this->_queue_ptr->push(& logger_type::error
                    , _plog
                    , (m != 0 ? m->name() + ": " + s : s));
        }

        basic_module * find_registered_module (string_type const & name)
        {
            auto first = _module_spec_map.begin();
            auto last  = _module_spec_map.end();

            for (; first != last; ++first) {
                module_spec modspec = first->second;
                auto pmodule = modspec.pmodule;

                if (pmodule->name() == name)
                    return pmodule.get();
            }

            return nullptr;
        }

    private:
        void (dispatcher::*info_printer) (basic_module const * m, string_type const & s);
        void (dispatcher::*debug_printer) (basic_module const * m, string_type const & s);
        void (dispatcher::*warn_printer) (basic_module const * m, string_type const & s);
        void (dispatcher::*error_printer) (basic_module const * m, string_type const & s);

        std::atomic_int         _quit_flag;
        std::atomic_int         _atomic_modules_started_counter {-1};
        std::atomic_bool        _atomic_modules_started_successfully {true};
        api_map_type            _api;
        module_spec_map_type    _module_spec_map;
        runnable_sequence_type  _runnable_modules;  // modules run in a separate threads
        basic_module *          _main_module_ptr;
        settings_type *         _psettings {nullptr};
        logger_type *           _plog {nullptr};
        std::unique_ptr<timer_pool_type> _ptimer_pool;
        intmax_t                _wait_period {10000}; // wait period in microseconds (default is 10 milliseconds)

    }; // class dispatcher
}; // struct modulus

} // namespace pfs

#define MODULUS_EMITTER(id, em) { id , reinterpret_cast<void *>(& em) }
#define MODULUS_DETECTOR(id, dt) { id , reinterpret_cast<detector_handler>(& dt) }

#define MODULUS_DECL_EMITTERS                                                  \
    virtual emitter_mapper_pair const *                                        \
    get_emitters (int & count) override;

#define MODULUS_BEGIN_EMITTERS(MODULE_CLASS)                                   \
    MODULE_CLASS::emitter_mapper_pair const *                                  \
    MODULE_CLASS::get_emitters (int & count)                                   \
    {                                                                          \
        static emitter_mapper_pair __emitter_mapper[] = {

#define MODULUS_BEGIN_INLINE_EMITTERS                                          \
    virtual emitter_mapper_pair const *                                        \
    get_emitters (int & count) override                                        \
    {                                                                          \
        static emitter_mapper_pair __emitter_mapper[] = {

#define MODULUS_END_EMITTERS                                                   \
    };                                                                         \
    count = sizeof(__emitter_mapper)/sizeof(__emitter_mapper[0]);              \
    return & __emitter_mapper[0];                                              \
}

#define MODULUS_DECL_DETECTORS                                                 \
    virtual detector_mapper_pair const *                                       \
    get_detectors (int & count) override;

#define MODULUS_BEGIN_DETECTORS(MODULE_CLASS)                                  \
    MODULE_CLASS::detector_mapper_pair const *                                 \
    MODULE_CLASS::get_detectors (int & count)                                  \
    {                                                                          \
        static detector_mapper_pair __detector_mapper[] = {

#define MODULUS_BEGIN_INLINE_DETECTORS                                         \
virtual detector_mapper_pair const *                                           \
get_detectors (int & count) override                                           \
{                                                                              \
    static detector_mapper_pair __detector_mapper[] = {

#define MODULUS_END_DETECTORS                                                  \
    };                                                                         \
    count = sizeof(__detector_mapper)/sizeof(__detector_mapper[0]);            \
    return & __detector_mapper[0];                                             \
}

#if _MSC_VER
#   define PFS_EXPORT_MODULE __declspec(dllexport)
#else
#   define PFS_EXPORT_MODULE
#endif

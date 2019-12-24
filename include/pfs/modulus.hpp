////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-modulus](https://github.com/semenovf/pfs-modulus) library.
//
// Changelog:
//      2019.12.19 Initial version (inhereted from https://github.com/semenovf/pfs)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "active_queue.hpp"
#include "sigslot.hpp"
#include "pfs/filesystem.hpp"
#include "pfs/dynamic_library.hpp"
#include "pfs/safeformat.hpp"
#include <atomic>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <cstdio>

namespace pfs {

struct basic_dispatcher
{
    virtual void quit () = 0;

#if defined(_WIN32) || defined(_WIN64)
    basic_dispatcher ()
    {}

    virtual ~basic_dispatcher ()
    {}
#else
//     bool activate_posix_signal_handling ();
//     void deactivate_posix_signal_handling ();
//
//     basic_dispatcher ()
//     {
//         (void)activate_posix_signal_handling();
//     }
//
//     virtual ~basic_dispatcher ()
//     {
//         this->deactivate_posix_signal_handling();
//     }
#endif
};

template <typename KeyType, typename ValueType>
using default_associative_container = std::map<KeyType, ValueType>;

template <typename T>
using default_sequence_container = std::list<T>;

template <typename T>
using default_queue_container = std::deque<T>;

struct simple_logger
{
    void info (std::string const & msg)  { fprintf(stdout, "%s\n", msg.c_str()); }
    void debug (std::string const & msg) { fprintf(stdout, "%s\n", msg.c_str()); }
    void warn (std::string const & msg)  { fprintf(stderr, "%s\n", msg.c_str()); }
    void error (std::string const & msg) { fprintf(stderr, "%s\n", msg.c_str()); }
};

template <typename LoggerType = simple_logger
        , typename StringType = std::string

        // For storing API map and module specs
        , template <typename, typename> class AssociativeContainer = default_associative_container

        // For storing runnable modules and threads
        , template <typename> class SequenceContainer = default_sequence_container

        // Active queue configuration
        , template <typename> class QueueContainer = default_queue_container

        // see [C++ concepts: BasicLockable](http://en.cppreference.com/w/cpp/concept/BasicLockable)>
        , typename BasicLockable = std::mutex
        , int GcThreshold = 256>
struct modulus
{
    class basic_module;
    class module;
    class async_module;
    class dispatcher;

    using logger_type = LoggerType;
    using string_type = StringType;

    using callback_queue_type = active_queue<QueueContainer
        , BasicLockable
        , GcThreshold>;

    using sigslot_ns = sigslot<callback_queue_type, BasicLockable>;
    using emitter_type  = typename sigslot_ns::template signal<>;

    using fmt = safeformat;

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
        std::shared_ptr<dynamic_library> pdl;
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
    using thread_function = int (basic_module::*)();
    using runnable_sequence_type = SequenceContainer<std::pair<basic_module *, thread_function>>;
    using thread_sequence_type = SequenceContainer<std::shared_ptr<std::thread>>;

////////////////////////////////////////////////////////////////////////////////
// basic_module
////////////////////////////////////////////////////////////////////////////////
    class basic_module : public sigslot_ns::basic_slot_holder
    {
        friend class dispatcher;

    public:
        using string_type = StringType;
        using emitter_mapper_pair = modulus::emitter_mapper_pair;
        using detector_mapper_pair = modulus::detector_mapper_pair;
        using detector_handler = modulus::detector_handler;
        using thread_function = modulus::thread_function;

    public: // signals
        typename sigslot_ns::template signal<string_type const &, bool &> emit_module_registered;

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
        virtual bool on_start ()
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
        friend class dispatcher;

    protected:
        module () noexcept : basic_module()
        {}

    public:
        virtual bool use_queued_slots () const override
        {
            return false;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// async_module
////////////////////////////////////////////////////////////////////////////////
    class async_module : public basic_module
    {
    public:
        async_module () : basic_module()
        {
            this->_queue_ptr = make_unique<callback_queue_type>();
        }

        virtual bool use_queued_slots () const override
        {
            return true;
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
            return !(this->priority_callback_queue().empty()
                    && this->callback_queue().empty());
        }
    };

////////////////////////////////////////////////////////////////////////////////
// slave_module
////////////////////////////////////////////////////////////////////////////////
    class slave_module : public basic_module
    {
        async_module * _master = nullptr;

    public:
        slave_module () : basic_module()
        {}

        virtual bool use_queued_slots () const override
        {
            return false;
        }

        virtual bool is_slave () const override
        {
            return true;
        }

        virtual typename sigslot_ns::basic_slot_holder * master () const
        {
            return _master;
        }

        void set_master (async_module * master)
        {
            _master = master;
        }
    };

////////////////////////////////////////////////////////////////////////////////
// SigSlot Mapper
////////////////////////////////////////////////////////////////////////////////
    struct basic_sigslot_mapper
    {
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

        virtual void connect_all ()
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

        virtual void disconnect_all ()
        {
            auto last = emitters.cend();

            for (auto it = emitters.cbegin(); it != last; it++) {
                EmitterType * em = *it;
                em->disconnect_all();
            }
        }

        virtual void append_emitter (emitter_type * e)
        {
            emitters.push_back(reinterpret_cast<EmitterType*>(e));
        }

        virtual void append_detector (basic_module * m, detector_handler d)
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
    class dispatcher : basic_dispatcher, public sigslot_ns::queued_slot_holder
    {
        friend class basic_module;
        friend class module;
        friend class async_module;

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
                pmodule->emit_module_registered.disconnect(this);

                log_debug(fmt("%s: unregistered") % (pmodule->name()));

                // Need to destroy pmodule before dynamic library will be destroyed automatically
                pmodule.reset();
            }

            _module_spec_map.clear();
        }

        bool start ()
        {
            bool ok = true;

            auto first = _module_spec_map.begin();
            auto last  = _module_spec_map.end();

            for (; first != last; ++first) {
                module_spec modspec = first->second;
                std::shared_ptr<basic_module> pmodule = modspec.pmodule;

                if (pmodule->is_slave() && pmodule->master() == 0) {
                    log_error(fmt("module is slave but has no master: %s") % pmodule->name());
                    ok = false;
                }

                if (ok) {
                    if (! pmodule->on_start()) {
                        log_error(fmt("failed to start module: %s") % pmodule->name());
                        ok = false;
                        pmodule->_started = false;
                    } else {
                        pmodule->_started = true;
                    }
                }
            }

            //
            // All modules started successfully.
            // Redirect log ouput.
            if (ok) {
                info_printer  = & dispatcher::async_print_info;
                debug_printer = & dispatcher::async_print_debug;
                warn_printer  = & dispatcher::async_print_warn;
                error_printer = & dispatcher::async_print_error;
            }

            return ok;
        }

        void finalize ()
        {
            this->_queue_ptr->call_all();

            info_printer  = & dispatcher::sync_print_info;
            debug_printer = & dispatcher::sync_print_debug;
            warn_printer  = & dispatcher::sync_print_warn;
            error_printer = & dispatcher::sync_print_error;

            if (_module_spec_map.size() > 0) {
                auto imodule      = _module_spec_map.begin();
                auto imodule_last = _module_spec_map.end();

                for (; imodule != imodule_last; ++imodule) {
                    module_spec & modspec = imodule->second;

                    if (modspec.pmodule->is_started()) {
                        // Do not 'Call deferred callbacks in modules'
                        // to avoid segmentation fault when signal handlers depends on varibales
                        // which lifetime limited by run() method
                        //
        //                 // Call deferred callbacks in modules
        //                 if (modspec.pmodule->use_queued_slots())
        //                     static_cast<async_module *>(modspec.pmodule.get())->call_all();

                        modspec.pmodule->on_finish();
                    }
                }

                disconnect_all();
                unregister_all();
            }

            this->_queue_ptr->call_all();
        }

        /**
         * @brief Internal dispatcher loop.
         */
        void run ()
        {
            auto pqueue = & this->callback_queue();

            while (! _quit_flag) {
                // TODO Use condition_variable to wait until _callback_queue will not be empty.
                if (pqueue->empty()) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                }

                // TODO Make configurable constant (5)
                pqueue->call(5);
            }

            pqueue->call_all();
        }

        int exec_main ()
        {
            int r = exit_status::success;

            thread_sequence_type thread_pool;

            auto irunnable      = _runnable_modules.begin();
            auto irunnable_last = _runnable_modules.end();

            thread_function master_thread_function = 0;

            for (; irunnable != irunnable_last; ++irunnable) {
                basic_module * m = irunnable->first;
                thread_function tfunc = irunnable->second;

                // Handle deferred calls after start stage and before thread method call (run()).
                m->_queue_ptr->call_all();

                // Run module if it is not a master
                if (m != _master_module_ptr)
                    thread_pool.push_back(std::make_shared<std::thread>(tfunc, m));
                else
                    master_thread_function = tfunc;
            }

            // Run module if it is a master
            if (master_thread_function) {
                // Run dispatcher loop in separate thread
                std::thread dthread(& dispatcher::run, this);

                // And call master function
                r = (_master_module_ptr->*master_thread_function)();

                dthread.join();
            } else {
                this->run();
            }

            auto ithread      = thread_pool.begin();
            auto ithread_last = thread_pool.end();

            for (; ithread != ithread_last; ++ithread) {
                (*ithread)->join();
            }

            return r;
        }

    public:
        dispatcher (dispatcher const &) = delete;
        dispatcher & operator = (dispatcher const &) = delete;

        dispatcher (api_item_type * mapper, int n, logger_type & logger)
            : basic_dispatcher()
            , info_printer(& dispatcher::sync_print_info)
            , debug_printer(& dispatcher::sync_print_debug)
            , warn_printer(& dispatcher::sync_print_warn)
            , error_printer(& dispatcher::sync_print_error)
            , _quit_flag(0)
            , _master_module_ptr(nullptr)
            , _plog(& logger)
        {
            register_api(mapper, n);
        }

        virtual ~dispatcher ()
        {
            finalize();
        }

        virtual void quit () override
        {
            _quit_flag.store(1);
        }

        bool is_quit () const
        {
            return (_quit_flag.load() != 0);
        }

        int exec ()
        {
            int r = exit_status::failure;

            connect_all();

            if (start()) {
                r = exec_main();
            }

            finalize();

            return r;
        }

        void register_api (api_item_type * mapper, int n)
        {
            for (int i = 0; i < n; ++i) {
                _api.insert(std::make_pair(mapper[i].id, & mapper[i]));
            }
        }

//         void add_search_path (filesystem::path const & dir)
//         {
//             if (!dir.empty())
//                 _searchdirs.push_back(dir);
//         }
//
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
        template <typename ModuleClass, typename ...Args>
        void register_module_helper (module_spec * pmodspec
                , string_type const & name
                , Args &&... args)
        {
            auto pmodule = std::make_shared<ModuleClass>(std::forward<Args>(args)...);
            pmodspec->pmodule = std::static_pointer_cast<basic_module>(pmodule);
            pmodspec->pmodule->set_dispatcher(this);
            pmodspec->pmodule->set_name(name);
        }

    public:
        template <typename ModuleClass, typename ...Args>
        bool register_regular_module (string_type const & name, Args &&... args)
        {
            module_spec modspec;
            register_module_helper<ModuleClass>(& modspec, name, std::forward<Args>(args)...);
            return register_module(modspec);
        }

        template <typename ModuleClass, typename ...Args>
        bool register_async_module (string_type const & name
                , int (ModuleClass::* thread_func) ()
                , Args &&... args)
        {
            module_spec modspec;
            register_module_helper<ModuleClass>(& modspec, name, std::forward<Args>(args)...);
            this->_runnable_modules.emplace_back(std::make_pair(& *modspec.pmodule
                    , static_cast<typename async_module::thread_function>(thread_func)));
            return register_module(modspec);
        }

        template <typename ModuleClass, typename ...Args>
        bool register_slave_module (string_type const & name
                , string_type const & async_module_name
                , Args &&... args)
        {
            module_spec modspec;
            register_module_helper<ModuleClass>(& modspec, name, std::forward<Args>(args)...);

            basic_module * master = this->find_registered_module(async_module_name);

            if (!master) {
                log_error(fmt("%s: active module not found") % async_module_name);
                return false;
            }

            if (!master->use_queued_slots()) {
                log_error(fmt("%s: module must be asynchronous") % name);
                return false;
            }

            std::static_pointer_cast<slave_module>(modspec.pmodule)->set_master(static_cast<async_module *>(master));

            return register_module(modspec);
        }

        /**
         * @brief Register module represented as shared object specified by @a path.
         */
        bool register_module_for_path (string_type const & path
                , string_type const & name)
        {
            module_spec modspec = module_for_path(path);

            if (modspec.pmodule) {
                modspec.pmodule->set_name(name);
                return register_module(modspec);
            }

            return false;
        }

        /**
         * @brief Register module represented as shared object specified by @a name.
         *
         * @details Actual path for shared object based on @a name constructed
         */
        bool register_module_for_name (string_type const & name)
        {
            module_spec modspec = module_for_name(name);

            if (modspec.pmodule) {
                modspec.pmodule->set_name(name);
                return register_module(modspec);
            }

            return false;
        }

        // TODO Unsuitbale member name, rename it
        bool set_master_module (string_type const & name)
        {
            basic_module * master = find_registered_module(name);

            if (!master) {
                // TODO
                //_logger.error(fmt("%s: master module not found") % name);
                return false;
            }

            if (!master->use_queued_slots()) {
                // TODO
                //_logger.error(fmt("%s: module must be asynchronous") % name);
                return false;
            }

            _master_module_ptr = master;
            return true;
        }

        size_t count () const
        {
            return _module_spec_map.size();
        }

        bool is_module_registered (string_type const & modname) const
        {
            return _module_spec_map.find(modname) != _module_spec_map.end();
        }

    public: // slots
        void module_registered (string_type const & pname, bool & result)
        {
            result = is_module_registered(pname);
        }

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
        module_spec module_for_path (fs::path const & path)
        {
            static char const * module_ctor_name = "__module_ctor__";
            static char const * module_dtor_name = "__module_dtor__";

            auto pdl = std::make_shared<dynamic_library>();
            std::error_code ec;

            filesystem::path dlpath(path);

            if (path.is_relative()) {
                dlpath = filesystem::path(".") / path;
            }

            //if (!pdl->open(dlpath, _searchdirs, ec)) {
            if (!pdl->open(path, ec)) {
                log_error(fmt("%s: %s") % dlpath.c_str() % ec.message());
                return module_spec();
            }

            dynamic_library::symbol_type ctor = pdl->resolve(module_ctor_name, ec);

            if (!ctor) {
                log_error(fmt("%s: failed to resolve `ctor' for module: %s")
                        % dlpath.c_str()
                        % ec.message());

                return module_spec();
            }

            dynamic_library::symbol_type dtor = pdl->resolve(module_dtor_name, ec);

            if (!dtor) {
                log_error(fmt("%s: failed to resolve `dtor' for module: %s")
                        % dlpath.c_str()
                        % ec.message());

                return module_spec();
            }

            module_ctor_t module_ctor = void_func_ptr_cast<module_ctor_t>(ctor);
            module_dtor_t module_dtor = void_func_ptr_cast<module_dtor_t>(dtor);

            basic_module * ptr = module_ctor();

            if (!ptr)
                return module_spec();

            module_spec result;
            result.pdl = pdl;
            result.pmodule = std::shared_ptr<basic_module>(ptr, module_deleter(module_dtor));

            return result;
        }

        module_spec module_for_name (string_type const & name)
        {
            auto modpath = dynamic_library::build_dl_filename(name);
            return module_for_path(modpath);
        }

        bool register_module (module_spec const & modspec)
        {
            using std::to_string;
            int nemitters, ndetectors;

            if (!modspec.pmodule)
                return false;

            auto pmodule = modspec.pmodule;

            if (_module_spec_map.find(pmodule->name()) != _module_spec_map.end()) {
                log_error(fmt("%s: module already registered") % (pmodule->name()));
                return false;
            }

            if (!pmodule->on_loaded()) {
                log_error(fmt("%s: on_loaded stage failed") % pmodule->name());
                return false;
            }

            emitter_mapper_pair const * emitters = pmodule->get_emitters(nemitters);
            detector_mapper_pair const * detectors = pmodule->get_detectors(ndetectors);

            auto it_end = _api.end();

            if (emitters) {
                for (int i = 0; i < nemitters; ++i) {
                    int emitter_id(emitters[i].id);
                    typename api_map_type::iterator it = _api.find(emitter_id);

                    if (it != it_end) {
                        it->second->mapper->append_emitter(reinterpret_cast<emitter_type *>(emitters[i].emitter));
                    } else {
                        log_warn(fmt("%s: emitter '%s' not found while registering module, "
                                "may be signal/slot mapping is not supported for this application")
                                % pmodule->name()
                                % to_string(emitters[i].id));
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
                        log_warn(fmt("%s: detector '%s' not found while registering module, "
                                "may be signal/slot mapping is not supported for this application")
                                % pmodule->name()
                                % to_string(detectors[i].id));
                    }
                }
            }

            pmodule->emit_module_registered.connect(this, & dispatcher::module_registered);
            _module_spec_map.insert(std::make_pair(pmodule->name(), modspec));

            log_debug(fmt("%s: registered") % (pmodule->name()));

            return true;
        }

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
//             this->_queue_ptr->template push_method<logger_type, string_type const &>(
//                     & logger_type::info, _plog, (m != 0 ? m->name() + ": " + s : s));
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

        std::atomic_int  _quit_flag;
//         filesystem::pathlist   _searchdirs;
        api_map_type           _api;
        module_spec_map_type   _module_spec_map;
        runnable_sequence_type _runnable_modules;  // modules run in a separate threads
        basic_module *         _master_module_ptr; // TODO Unsuitable member name, rename it
        logger_type *          _plog = nullptr;
    }; // class dispatcher
}; // struct modulus

//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// filesystem::pathlist modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::module_paths () const
// {
//     filesystem::pathlist result;
//
//     typename module_spec_map_type::const_iterator first = _module_spec_map.cbegin();
//     typename module_spec_map_type::const_iterator last  = _module_spec_map.cend();
//
//     for (; first != last; ++first) {
//         module_spec modspec = first->second;
//         result.push_back(modspec.pdl->path());
//     }
//
//     return result;
// }
} // namespace pfs

#ifndef PFS_MODULE_EXPORT
#   if defined(_WIN32) || defined(_WIN64)
#       if PFS_MODULE_EXPORTS
#           define PFS_MODULE_API __declspec(dllexport)
#       else
#           define PFS_MODULE_API __declspec(dllimport)
#       endif
#   else
#       define PFS_MODULE_EXPORT
#   endif
#endif

// #define PFS_EMITTER_CAST(e)     reinterpret_cast<void *>(& e)
// #define PFS_DETECTOR_CAST(slot) reinterpret_cast<detector_handler>(& slot)

#define PFS_MODULE_EMITTER(id, em) { id , reinterpret_cast<void *>(& em) } //PFS_EMITTER_CAST(em) }
#define PFS_MODULE_DETECTOR(id, dt) { id , reinterpret_cast<detector_handler>(& dt) } // PFS_DETECTOR_CAST(dt) }

#define PFS_MODULE_EMITTERS_EXTERN                                             \
    virtual emitter_mapper_pair const *                                        \
    get_emitters (int & count) override;

#define PFS_MODULE_EMITTERS_BEGIN(XMOD)                                        \
    XMOD::emitter_mapper_pair const *                                          \
    XMOD::get_emitters (int & count)                                           \
    {                                                                          \
        static emitter_mapper_pair __emitter_mapper[] = {

#define PFS_MODULE_EMITTERS_INLINE_BEGIN                                       \
    virtual emitter_mapper_pair const *                                        \
    get_emitters (int & count) override                                        \
    {                                                                          \
        static emitter_mapper_pair __emitter_mapper[] = {

#define PFS_MODULE_EMITTERS_END                                                \
    };                                                                         \
    count = sizeof(__emitter_mapper)/sizeof(__emitter_mapper[0]);              \
    return & __emitter_mapper[0];                                              \
}

#define PFS_MODULE_DETECTORS_EXTERN                                            \
    virtual detector_mapper_pair const *                                       \
    get_detectors (int & count) override;

#define PFS_MODULE_DETECTORS_BEGIN(XMOD)                                       \
    XMOD::detector_mapper_pair const *                                         \
    XMOD::get_detectors (int & count)                                          \
    {                                                                          \
        static detector_mapper_pair __detector_mapper[] = {

#define PFS_MODULE_DETECTORS_INLINE_BEGIN                                      \
virtual detector_mapper_pair const *                                           \
get_detectors (int & count) override                                           \
{                                                                              \
    static detector_mapper_pair __detector_mapper[] = {

#define PFS_MODULE_DETECTORS_END                                               \
    };                                                                         \
    count = sizeof(__detector_mapper)/sizeof(__detector_mapper[0]);            \
    return & __detector_mapper[0];                                             \
}

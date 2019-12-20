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
#include <pfs/dynamic_library.hpp>
// #include <pfs/cxxlang.hpp>
// #include <pfs/operationsystem.hpp>
// #include <pfs/bits/timer.h>
// #include <pfs/type_traits.hpp>
// #include <pfs/atomic.hpp>
#include <pfs/sigslot.hpp>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
// #include <pfs/stringlist.hpp>

// #include <pfs/active_map.hpp>
// #include <pfs/logger.hpp>

// #include <pfs/safeformat.hpp>
// #include <pfs/datetime.hpp>
// #include <pfs/chrono.hpp>
// #include <pfs/thread.hpp>
// #include <pfs/io/file.hpp>
// #include <pfs/io/iterator.hpp>

namespace pfs {

// #define PFS_MODULE_CTOR_NAME "__module_ctor__"
// #define PFS_MODULE_DTOR_NAME "__module_dtor__"

struct basic_dispatcher
{
    virtual void quit () = 0;

// #if PFS_OS_POSIX
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
// #else

    basic_dispatcher ()
    {}

    virtual ~basic_dispatcher ()
    {}

// #endif // PFS_OS_POSIX
};

// #define PFS_MODULUS_TEMPLETE_SIGNATURE typename StringType                     \
//     , template <typename, typename> class AssociativeContainer                 \
//     , template <typename> class SequenceContainer                              \
//     , template <typename> class QueueContainer                           \
//     , typename BasicLockable                                                   \
//     , int GcThreshold
//
// #define PFS_MODULUS_TEMPLETE_ARGS StringType                                   \
//     , AssociativeContainer                                                     \
//     , SequenceContainer                                                        \
//     , QueueContainer                                                     \
//     , BasicLockable                                                            \
//     , GcThreshold

template <typename KeyType, typename ValueType>
using default_associative_container = std::map<KeyType, ValueType>;

template <typename T>
using default_sequence_container = std::list<T>;

template <typename T>
using default_queue_container = std::deque<T>;

template <typename StringType = std::string
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

    using callback_queue_type = active_queue<QueueContainer
        , BasicLockable
        , GcThreshold>;

//     typedef pfs::active_map<int // timer id type
//         , void
//         , AssociativeContainer
//         , BasicLockable> timer_callback_map;

    using sigslot_ns = sigslot<callback_queue_type, BasicLockable>;

    using string_type = StringType;
//     typedef log<sigslot_ns, SequenceContainer> log_ns;
//     typedef typename log_ns::logger logger_type;
    using emitter_type  = typename sigslot_ns::template signal<> ;

    typedef void (basic_module::* detector_handler)(void *);
    typedef struct { int id; void * emitter; }            emitter_mapper_pair;
    typedef struct { int id; detector_handler detector; } detector_mapper_pair;

    typedef basic_module * (* module_ctor_t)(dispatcher * pdisp, char const * name, void *);
    typedef void  (* module_dtor_t)(basic_module *);

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

//     typedef AssociativeContainer<int, api_item_type *>     api_map_type;
//     typedef AssociativeContainer<string_type, module_spec> module_spec_map_type;
//     typedef int (basic_module::*thread_function)();
//     typedef SequenceContainer<pfs::pair<basic_module *, thread_function> > runnable_sequence_type;
//     typedef SequenceContainer<shared_ptr<pfs::thread> >                    thread_sequence_type;
//
//     class basic_module : public sigslot_ns::basic_has_slots
//     {
//         friend class dispatcher;
//
//     public:
//         typedef StringType string_type;
//         typedef modulus::emitter_mapper_pair  emitter_mapper_pair;
//         typedef modulus::detector_mapper_pair detector_mapper_pair;
//         typedef modulus::detector_handler     detector_handler;
//         typedef modulus::thread_function      thread_function;
//
//     public: // signals
//         typename sigslot_ns::template signal2<string_type const &, bool &> emit_module_registered;
//
//     public:
//         // TODO OBSOLETE, use quit()
//         void emit_quit ()
//         {
//             _pdispatcher->quit();
//         }
//
//         void quit ()
//         {
//             _pdispatcher->quit();
//         }
//
//         void print_info (string_type const & s)
//         {
//             _pdispatcher->print_info(this, s);
//         }
//
//         void print_debug (string_type const & s)
//         {
//             _pdispatcher->print_debug(this, s);
//         }
//
//         void print_warn (string_type const & s)
//         {
//             _pdispatcher->print_warn(this, s);
//         }
//
//         void print_error (string_type const & s)
//         {
//             _pdispatcher->print_error(this, s);
//         }
//
//     protected:
//         string_type  _name;
//         dispatcher * _pdispatcher;
//         bool         _started;
//
//     protected:
//         basic_module (dispatcher * pdisp)
//             : _pdispatcher(pdisp)
//             , _started(false)
//         {}
//
//         void set_name (string_type const & name)
//         {
//             _name = name;
//         }
//
//     public:
//         virtual ~basic_module () {}
//
//         string_type const & name() const
//         {
//             return _name;
//         }
//
//         bool is_registered () const
//         {
//             return _pdispatcher != 0 ? true : false;
//         }
//
//         bool is_started () const
//         {
//             return _started;
//         }
//
//         dispatcher * get_dispatcher ()
//         {
//             return _pdispatcher;
//         }
//
//         dispatcher const * get_dispatcher () const
//         {
//             return _pdispatcher;
//         }
//
//         void register_thread_function (thread_function tfunc)
//         {
//             _pdispatcher->_runnable_modules.push_back(pfs::make_pair(this, tfunc));
//         }
//
//         virtual emitter_mapper_pair const * get_emitters (int & count)
//         {
//             count = 0;
//             return 0;
//         }
//
//         virtual detector_mapper_pair const * get_detectors (int & count)
//         {
//             count = 0;
//             return 0;
//         }
//
//         bool is_quit () const
//         {
//             return _pdispatcher->is_quit();
//         }
//
//         /**
//          * @brief Module's on_loaded() method called after loaded and before registration.
//          * @return
//          */
//         virtual bool on_loaded ()
//         {
//             return true;
//         }
//
//         /**
//          * @brief Module's on_start() method called after loaded and connection completed.
//          */
//         virtual bool on_start ()
//         {
//             return true;
//         }
//
//         virtual bool on_finish ()
//         {
//             return true;
//         }
//     };// basic_module
//
//     //
//     // Body must be identical to sigslot's has_slots
//     // TODO Reimeplement to avoid code duplication
//     //
//     class module : public basic_module
//     {
//     public:
//         module (dispatcher * pdisp) : basic_module(pdisp) {}
//         virtual bool use_async_slots () const override { return false; }
//
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) ())
//         {
//             this->get_dispatcher()->start_timer(timer_type, seconds, f);
//         }
//
//         template <typename Arg1>
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) (Arg1), Arg1 a1)
//         {
//             this->get_dispatcher()->start_timer(timer_type, seconds, f, a1);
//         }
//     };
//
//     //
//     // Body must be identical to sigslot's has_async_slots
//     // TODO Reimeplement to avoid code duplication
//     //
//     class async_module : public basic_module
//     {
//     public:
//         async_module (dispatcher * pdisp) : basic_module(pdisp)
//         {
//             this->_priority_queue_ptr = make_unique<callback_queue_type>();
//             this->_queue_ptr = make_unique<callback_queue_type>();
//         }
//
//         virtual bool use_async_slots () const override { return true; }
//
//         /**
//          * @see process_events.
//          */
//         void call_all ()
//         {
//             this->priority_callback_queue().call_all();
//             this->callback_queue().call_all();
//         }
//
//         /**
//          * @brief process_events() is a synonym for call_all().
//          * @see call_all.
//          */
//         void process_events ()
//         {
//             this->call_all();
//         }
//
//         void process_events (int max_count)
//         {
//             this->priority_callback_queue().call_all();
//             this->callback_queue().call(max_count);
//         }
//
//         bool has_pending_events () const
//         {
//             return !(this->priority_callback_queue().empty()
//                     && this->callback_queue().empty());
//         }
//
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) ())
//         {
//             this->get_dispatcher()->start_timer(timer_type, seconds, *this, f);
//         }
//
//         template <typename Arg1>
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) (Arg1), Arg1 a1)
//         {
//             this->get_dispatcher()->template start_timer<Arg1>(timer_type, seconds, *this, f, a1);
//         }
//     };
//
//     class slave_module : public basic_module
//     {
//         async_module * _master;
//
//     public:
//         slave_module (dispatcher * pdisp) : basic_module(pdisp), _master(0) {}
//         virtual bool use_async_slots () const override { return false; }
//         virtual bool is_slave () const override { return true; }
//         virtual typename sigslot_ns::basic_has_slots * master () const { return _master; }
//         void set_master (async_module * master) { _master = master; }
//
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) ())
//         {
//             this->get_dispatcher()->start_timer(timer_type, seconds, *_master, f);
//         }
//
//         template <typename Arg1>
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) (Arg1), Arg1 a1)
//         {
//             this->get_dispatcher()->start_timer(timer_type, seconds, *_master, f, a1);
//         }
//     };
//
//     struct basic_sigslot_mapper
//     {
//         virtual void connect_all () = 0;
//         virtual void disconnect_all () = 0;
//         virtual void append_emitter (emitter_type *e) = 0;
//         virtual void append_detector (basic_module * m, detector_handler d) = 0;
//     };
//
//     struct api_item_type
//     {
//         int                                   id;
//         pfs::shared_ptr<basic_sigslot_mapper> mapper;
//         string_type                           desc;
//     };
//
//     template <typename EmitterType, typename DetectorType>
//     struct sigslot_mapper : basic_sigslot_mapper
//     {
//         typedef SequenceContainer<EmitterType *> emitter_sequence;
//         typedef SequenceContainer<detector_pair> detector_sequence;
//
//         emitter_sequence  emitters;
//         detector_sequence detectors;
//
//         virtual void connect_all ()
//         {
//             if (emitters.size() == 0 || detectors.size() == 0)
//                 return;
//
//             typename emitter_sequence::const_iterator itEnd = emitters.cend();
//             typename detector_sequence::const_iterator itdEnd = detectors.cend();
//
//             for (typename emitter_sequence::const_iterator it = emitters.cbegin(); it != itEnd; it++) {
//                 for (typename detector_sequence::const_iterator itd = detectors.cbegin(); itd != itdEnd; itd++) {
//                     EmitterType * em = *it;
//                     em->connect(itd->mod, reinterpret_cast<DetectorType> (itd->detector));
//                 }
//             }
//         }
//
//         virtual void disconnect_all ()
//         {
//             typename emitter_sequence::const_iterator itEnd = emitters.cend();
//
//             for (typename emitter_sequence::const_iterator it = emitters.cbegin(); it != itEnd; it++) {
//                 EmitterType * em = *it;
//                 em->disconnect_all();
//             }
//         }
//
//         virtual void append_emitter (emitter_type * e)
//         {
//             emitters.push_back(reinterpret_cast<EmitterType*>(e));
//         }
//
//         virtual void append_detector (basic_module * m, detector_handler d)
//         {
//             detectors.push_back(detector_pair(m, d));
//         }
//     };
//
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::signal0, void (basic_module::*)()> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal1<A1>, void (basic_module::*)(A1)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal2<A1,A2>, void (basic_module::*)(A1,A2)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2, typename A3>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal3<A1,A2,A3>, void (basic_module::*)(A1,A2,A3)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2, typename A3, typename A4>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal4<A1,A2,A3,A4>, void (basic_module::*)(A1,A2,A3,A4)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2, typename A3, typename A4, typename A5>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal5<A1,A2,A3,A4,A5>, void (basic_module::*)(A1,A2,A3,A4,A5)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal6<A1,A2,A3,A4,A5,A6>, void (basic_module::*)(A1,A2,A3,A4,A5,A6)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal7<A1,A2,A3,A4,A5,A6,A7>, void (basic_module::*)(A1,A2,A3,A4,A5,A6,A7)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
//     static pfs::shared_ptr<basic_sigslot_mapper> make_mapper ()
//     {
//         typedef sigslot_mapper<typename sigslot_ns::template signal8<A1,A2,A3,A4,A5,A6,A7,A8>, void (basic_module::*)(A1,A2,A3,A4,A5,A6,A7,A8)> concrete_mapper_type;
//         return pfs::static_pointer_cast<basic_sigslot_mapper>(pfs::make_shared<concrete_mapper_type>());
//     }
//
//     class dispatcher : basic_dispatcher, public sigslot_ns::has_async_slots
//     {
//         friend class basic_module;
//         friend class module;
//         friend class async_module;
//
//         typedef safeformat fmt;
//
//     public:
//         typedef modulus::string_type string_type;
//         typedef modulus::logger_type logger_type;
//
//         struct exit_status
//         {
//             enum {
//                   success = 0
//                 , failure = -1
//             };
//         };
//
//     private:
//     #if __cplusplus >= 201103L
//         dispatcher (dispatcher const &) = delete;
//         dispatcher & operator = (dispatcher const &) = delete;
//     #else
//         dispatcher (dispatcher const &);
//         dispatcher & operator = (dispatcher const &);
//     #endif
//
//     private:
//         void connect_all ();
//         void disconnect_all ();
//         void unregister_all ();
//         bool start ();
//         void finalize ();
//
//         /**
//          * @brief Internal dispatcher loop.
//          */
//         void run ();
//
//         int  exec_main ();
//
//     public:
//         dispatcher (api_item_type * mapper, int n);
//
//         virtual ~dispatcher ()
//         {
//             finalize();
//         }
//
//         virtual void quit () override
//         {
//             _quitfl.store(1);
//         }
//
//         bool is_quit () const
//         {
//             return (_quitfl.load() != 0);
//         }
//
//         int exec ();
//
//         void start_timer (timer_type_enum timer_type, double seconds, async_module & m, void (* f) ())
//         {
//             start_timer(timer_type, seconds, & m.priority_callback_queue(), f);
//         }
//
//         template <typename Arg1>
//         void start_timer (timer_type_enum timer_type, double seconds, async_module & m, void (* f) (Arg1), Arg1 a1)
//         {
//             start_timer<Arg1>(timer_type, seconds, & m.priority_callback_queue(), f, a1);
//         }
//
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) ())
//         {
//             start_timer(timer_type, seconds, & this->priority_callback_queue(), f);
//         }
//
//         template <typename Arg1>
//         void start_timer (timer_type_enum timer_type, double seconds, void (* f) (Arg1), Arg1 a1)
//         {
//             start_timer<Arg1>(timer_type, seconds, & this->priority_callback_queue(), f, a1);
//         }
//
//         void stop_timer (timer_id_t id)
//         {
//             pfs::unique_lock<BasicLockable> locker(_timer_mutex);
//             timer_unset(id);
//             _timer_callbacks.erase(id);
//         }
//
//         void register_api (api_item_type * mapper, int n);
//
//         void add_search_path (filesystem::path const & dir)
//         {
//             if (!dir.empty())
//                 _searchdirs.push_back(dir);
//         }
//
//     //    /**
//     //     * @brief Output summary of emitters/detectors utilization.
//     //     * TODO Implement
//     //     */
//     //    void print_api_connections () {}
//     //
//     //    /**
//     //     * @brief Output summary of incomplete emitters/detectors utilization.
//     //     * TODO Implement
//     //     */
//     //    void print_api_incomplete_connections () {}
//
//         /**
//          * @brief Register modules enumerated in configuration file (i.e. JSON) specified by @a path.
//          *
//          * @param path Configuration file path.
//          * @return @c true if modules registered successfully. @c false otherwise.
//          */
//         template <typename PropertyTree>
//         bool register_modules (filesystem::path const & path);
//
//         /**
//          * @brief Register modules enumerated in JSON instance specified by @a conf.
//          *
//          * @param conf JSON instance contained modules for registration.
//          * @return @c true if modules registered successfully. @c false otherwise.
//          */
//         template <typename Json>
//         bool register_modules (Json const & conf);
//
//         bool register_local_module (basic_module * pmodule, string_type const & name);
//
//         bool register_module_for_path (filesystem::path const & path
//                 , string_type const & name
//                 , const char * class_name = 0
//                 , void * mod_data = 0);
//
//         bool register_module_for_name (string_type const & name
//                 , char const * class_name = 0
//                 , void * mod_data = 0);
//
//         bool set_dependences (string_type const & name, string_type const & depends_name);
//
//         // TODO Unsuitbale member name, rename it
//         bool set_master_module (string_type const & name);
//
//         size_t count () const
//         {
//             return _module_spec_map.size();
//         }
//
//         bool is_module_registered (string_type const & modname) const
//         {
//             return _module_spec_map.find(modname) != _module_spec_map.end();
//         }
//
//         logger_type & get_logger () { return _logger; }
//         logger_type const & get_logger () const { return _logger; }
//
//         void set_log_directory (filesystem::path const & dir)
//         {
//             _log_directory = dir;
//         }
//
//     public: // slots
//         void module_registered ( string_type const & pname, bool & result )
//         {
//             result = is_module_registered ( pname );
//         }
//
//         void print_info (basic_module const * m, string_type const & s)
//         {
//             (this->*info_printer)(m, s);
//         }
//
//         void print_debug (basic_module const * m, string_type const & s)
//         {
//             (this->*debug_printer)(m, s);
//         }
//
//         void print_warn (basic_module const * m, string_type const & s)
//         {
//             (this->*warn_printer)(m, s);
//         }
//
//         void print_error (basic_module const * m, string_type const & s)
//         {
//             (this->*error_printer)(m, s);
//         }
//
//         void print_info (string_type const & s)
//         {
//             print_info(0, s);
//         }
//
//         void print_debug (string_type const & s)
//         {
//             print_debug(0, s);
//         }
//
//         void print_warn (string_type const & s)
//         {
//             print_warn(0, s);
//         }
//
//         void print_error (string_type const & s)
//         {
//             print_error(0, s);
//         }
//
//         void connect_console_appenders ();
//         void disconnect_console_appenders ();
//
//         filesystem::pathlist module_paths () const;
//
//     protected:
//         module_spec module_for_path ( filesystem::path const & path
//                 , char const * class_name = 0
//                 , void * mod_data = 0 );
//
//         module_spec module_for_name ( string_type const & name
//                 , char const * class_name = 0
//                 , void * mod_data = 0 )
//         {
//             filesystem::path modpath = build_so_filename(name);
//             return module_for_path ( modpath, class_name, mod_data );
//         }
//
//         bool register_module ( module_spec const & modspec );
//
//         void sync_print_info (basic_module const * m, string_type const & s)
//         {
//             _logger.info(m != 0 ? m->name() + ": " + s : s);
//         }
//
//         void sync_print_debug (basic_module const * m, string_type const & s)
//         {
//             _logger.debug(m != 0 ? m->name() + ": " + s : s);
//         }
//
//         void sync_print_warn (basic_module const * m, string_type const & s)
//         {
//             _logger.warn(m != 0 ? m->name() + ": " + s : s);
//         }
//
//         void sync_print_error (basic_module const * m, string_type const & s)
//         {
//             _logger.error(m != 0 ? m->name() + ": " + s : s);
//         }
//
//         void async_print_info (basic_module const * m, string_type const & s)
//         {
//             this->_queue_ptr->template push_method<logger_type, string_type const &>(
//                     & logger_type::info, & _logger, (m != 0 ? m->name() + ": " + s : s));
//         }
//
//         void async_print_debug (basic_module const * m, string_type const & s)
//         {
//             this->_queue_ptr->template push_method<logger_type, string_type const &>(
//                     & logger_type::debug, & _logger, (m != 0 ? m->name() + ": " + s : s));
//         }
//
//         void async_print_warn (basic_module const * m, string_type const & s)
//         {
//             this->_queue_ptr->template push_method<logger_type, string_type const &>(
//                     & logger_type::warn, & _logger, (m != 0 ? m->name() + ": " + s : s));
//         }
//
//         void async_print_error (basic_module const * m, string_type const & s)
//         {
//             this->_queue_ptr->template push_method<logger_type, string_type const &>(
//                     & logger_type::error, & _logger, (m != 0 ? m->name() + ": " + s : s));
//         }
//
//         basic_module * find_registered_module (string_type const & name);
//
//         static void deferred_caller (callback_queue_type * q, void (* f) ())
//         {
//             q->push_function(f);
//         }
//
//         template <typename Arg1>
//         static void deferred_caller (callback_queue_type * q, void (* f) (Arg1 a1), Arg1 a1)
//         {
//             q->template push_function<Arg1>(f, a1);
//         }
//
//         void start_timer (timer_type_enum timer_type, double seconds, callback_queue_type * q, void (* f) ())
//         {
//             pfs::unique_lock<BasicLockable> locker(_timer_mutex);
//             timer_id_t id = timer_set(timer_type, seconds);
//             _timer_callbacks.insert_function(id, deferred_caller, q, f);
//         }
//
//         template <typename Arg1>
//         void start_timer (timer_type_enum timer_type, double seconds, callback_queue_type * q, void (* f) (Arg1), Arg1 a1)
//         {
//             pfs::unique_lock<BasicLockable> locker(_timer_mutex);
//             timer_id_t id = timer_set(timer_type, seconds);
//             _timer_callbacks.template insert_function<callback_queue_type *, void (*) (Arg1), Arg1>(id, deferred_caller, q, f, a1);
//         }
//
//     private:
//         void (dispatcher::*info_printer) (basic_module const * m, string_type const & s);
//         void (dispatcher::*debug_printer) (basic_module const * m, string_type const & s);
//         void (dispatcher::*warn_printer) (basic_module const * m, string_type const & s);
//         void (dispatcher::*error_printer) (basic_module const * m, string_type const & s);
//
//         atomic_int             _quitfl; // quit flag
//         filesystem::pathlist   _searchdirs;
//         api_map_type           _api;
//         module_spec_map_type   _module_spec_map;
//         runnable_sequence_type _runnable_modules;  // modules run in a separate threads
//         basic_module *         _master_module_ptr; // TODO Unsuitbale member name, rename it
//         filesystem::path       _log_directory;
//         logger_type            _logger;
//
//         BasicLockable       _timer_mutex;
//         timer_callback_map  _timer_callbacks;
//
//         // Console appenders
//         typename log_ns::appender * _cout_appender_ptr;
//         typename log_ns::appender * _cerr_appender_ptr;
//     }; // class dispatcher
}; // struct modulus

// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::dispatcher (
//         api_item_type * mapper
//         , int n)
//     : basic_dispatcher()
//     , info_printer(& dispatcher::sync_print_info)
//     , debug_printer(& dispatcher::sync_print_debug)
//     , warn_printer(& dispatcher::sync_print_warn)
//     , error_printer(& dispatcher::sync_print_error)
//     , _master_module_ptr(0)
//     , _quitfl(0)
// {
//     // Initialize default logger
//     _cout_appender_ptr = & _logger.template add_appender<typename log_ns::stdout_appender>();
//     _cerr_appender_ptr = & _logger.template add_appender<typename log_ns::stderr_appender>();
//     _cout_appender_ptr->set_pattern("%d{ABSOLUTE} [%p]: %m");
//     _cerr_appender_ptr->set_pattern("%d{ABSOLUTE} [%p]: %m");
//
//     connect_console_appenders();
//
//     register_api(mapper, n);
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::connect_console_appenders ()
// {
//     _logger.connect(log_ns::priority::trace   , *_cout_appender_ptr);
//     _logger.connect(log_ns::priority::debug   , *_cout_appender_ptr);
//     _logger.connect(log_ns::priority::info    , *_cout_appender_ptr);
//     _logger.connect(log_ns::priority::warn    , *_cerr_appender_ptr);
//     _logger.connect(log_ns::priority::error   , *_cerr_appender_ptr);
//     _logger.connect(log_ns::priority::critical, *_cerr_appender_ptr);
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::disconnect_console_appenders ()
// {
//     _logger.disconnect(*_cout_appender_ptr);
//     _logger.disconnect(*_cerr_appender_ptr);
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_api (
//           api_item_type * mapper
//         , int n)
// {
//     for (int i = 0; i < n; ++i) {
//         _api.insert(mapper[i].id, & mapper[i]);
//     }
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::connect_all ()
// {
//     typename api_map_type::iterator first = _api.begin();
//     typename api_map_type::iterator last  = _api.end();
//
//     for (; first != last; ++first) {
//         first->second->mapper->connect_all();
//     }
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::run ()
// {
//     pfs::unique_lock<BasicLockable> locker(_timer_mutex, DEFER_LOCK);
//
//     while (! _quitfl) {
//         locker.lock();
//
//         int ntimers = timer_count();
//
//         if (ntimers > 0) {
//             for (int i = 0; i < ntimers; ++i) {
//                 if (timer_passed(i)) {
//                     if (timer_is_periodic(i)) {
//                         _timer_callbacks.call(i);
//                     } else {
//                         _timer_callbacks.call_and_erase(i);
//                     }
//
//                     timer_reset(i);
//                 }
//             }
//         }
//
//         locker.unlock();
//
//         // FIXME Use condition_variable to wait until _callback_queue will not be empty.
//         if (this->_queue_ptr->empty()) {
//             pfs::this_thread::sleep_for(pfs::chrono::microseconds(100));
//             continue;
//         }
//
//         this->_queue_ptr->call(5);
//     }
//
//     this->_queue_ptr->call_all();
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::start ()
// {
//     bool ok = true;
//
//     typename module_spec_map_type::iterator first = _module_spec_map.begin();
//     typename module_spec_map_type::iterator last  = _module_spec_map.end();
//
//     for (; first != last; ++first) {
//         module_spec modspec = first->second;
//         shared_ptr<basic_module> pmodule = modspec.pmodule;
//
//         if (pmodule->is_slave() && pmodule->master() == 0) {
//             _logger.error(fmt("module is slave but has no master: %s") % pmodule->name());
//             ok = false;
//         }
//
//         if (ok) {
//             if (! pmodule->on_start()) {
//                 _logger.error(fmt("failed to start module: %s") % pmodule->name());
//                 ok = false;
//                 pmodule->_started = false;
//             } else {
//                 pmodule->_started = true;
//             }
//         }
//     }
//
//     //
//     // All modules started successfully.
//     // Redirect log ouput.
//     //
//     if (ok) {
//         info_printer  = & dispatcher::async_print_info;
//         debug_printer = & dispatcher::async_print_debug;
//         warn_printer  = & dispatcher::async_print_warn;
//         error_printer = & dispatcher::async_print_error;
//     }
//
//     //
//     // Start timer
//     //
//     if (ok) {
//         if (timer_init() != 0) {
//             print_error(fmt("failed to initialize timer: %s")
//                     % pfs::to_string(pfs::get_last_system_error()));
//             ok = false;
//         }
//     }
//
//     return ok;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::finalize ()
// {
//     this->_queue_ptr->call_all();
//
//     _timer_callbacks.clear();
//     timer_done();
//
//     info_printer  = & dispatcher::sync_print_info;
//     debug_printer = & dispatcher::sync_print_debug;
//     warn_printer  = & dispatcher::sync_print_warn;
//     error_printer = & dispatcher::sync_print_error;
//
//     if (_module_spec_map.size() > 0) {
//         typename module_spec_map_type::iterator imodule      = _module_spec_map.begin();
//         typename module_spec_map_type::iterator imodule_last = _module_spec_map.end();
//
//         for (; imodule != imodule_last; ++imodule) {
//             module_spec & modspec = imodule->second;
//
//             if (modspec.pmodule->is_started()) {
//
//                 // Do not 'Call deferred callbacks in modules'
//                 // to avoid segmentation fault when signal handlers depends on varibales
//                 // which lifetime limited by run() method
//                 //
// //                 // Call deferred callbacks in modules
// //                 if (modspec.pmodule->use_async_slots())
// //                     static_cast<async_module *>(modspec.pmodule.get())->call_all();
//
//                 modspec.pmodule->on_finish();
//             }
//         }
//
//         disconnect_all();
//         unregister_all();
//     }
//
//     this->_queue_ptr->call_all();
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::disconnect_all ()
// {
//     typename api_map_type::iterator first = _api.begin();
//     typename api_map_type::iterator last  = _api.end();
//
//     for (; first != last; ++first) {
//         first->second->mapper->disconnect_all();
//     }
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// void modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::unregister_all ()
// {
//     _runnable_modules.clear();
//
//     typename module_spec_map_type::iterator first = _module_spec_map.begin();
//     typename module_spec_map_type::iterator last = _module_spec_map.end();
//
//     for (; first != last; ++first) {
//         module_spec & modspec = first->second;
//         shared_ptr<basic_module> & pmodule = modspec.pmodule;
//         pmodule->emit_module_registered.disconnect(this);
//         _logger.debug(fmt("%s: unregistered") % (pmodule->name()));
//
//         // Need to destroy pmodule before dynamic library will be destroyed automatically
//         pmodule.reset();
//     }
//
//     _module_spec_map.clear();
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_local_module (
//           basic_module * pmodule
//         , string_type const & name)
// {
//     module_spec modspec;
//     modspec.pmodule = shared_ptr<basic_module>(pmodule);
//     modspec.pmodule->set_name(name);
//     return register_module(modspec);
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_module_for_name (
//           string_type const & name
//         , char const * class_name
//         , void * mod_data)
// {
//     module_spec modspec = module_for_name(name, class_name, mod_data);
//
//     if (modspec.pmodule) {
//         modspec.pmodule->set_name(name);
//         return register_module(modspec);
//     }
//     return false;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// typename modulus<PFS_MODULUS_TEMPLETE_ARGS>::module_spec
// modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::module_for_path (
//           filesystem::path const & path
//         , char const * class_name
//         , void * mod_data)
// {
//     shared_ptr<dynamic_library> pdl = make_shared<dynamic_library>();
//     error_code ec;
//
//     filesystem::path dlpath(path);
//
//     if (path.is_relative()) {
//         dlpath = filesystem::path(".") / path;
//     }
//
//     if (!pdl->open(dlpath, _searchdirs, ec)) {
//         _logger.error(fmt("%s: %s")
//                 % (to_string(dlpath))
//                 % (to_string(ec)));
//         return module_spec();
//     }
//
//     dynamic_library::symbol_type ctor = pdl->resolve(PFS_MODULE_CTOR_NAME, ec);
//
//     if (!ctor) {
//         _logger.error(fmt("%s: failed to resolve `ctor' for module: %s")
//                 % (to_string(dlpath))
//                 % (to_string(ec)));
//
//         return module_spec();
//     }
//
//     dynamic_library::symbol_type dtor = pdl->resolve(PFS_MODULE_DTOR_NAME, ec);
//
//     if (!dtor) {
//         _logger.error(fmt("%s: failed to resolve `dtor' for module: %s")
//                 % (to_string(dlpath))
//                 % (to_string(ec)));
//
//         return module_spec();
//     }
//
//     module_ctor_t module_ctor = pfs::void_func_ptr_cast<module_ctor_t>(ctor);
//     module_dtor_t module_dtor = pfs::void_func_ptr_cast<module_dtor_t>(dtor);
//
//     //module * ptr = reinterpret_cast<module *>(module_ctor(class_name, mod_data));
//     basic_module * ptr = module_ctor(this, class_name, mod_data);
//
//     if (!ptr)
//         return module_spec();
//
//     module_spec result;
//     result.pdl = pdl;
//     result.pmodule = shared_ptr<basic_module>(ptr, module_deleter(module_dtor));
//
//     return result;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_module (
//         module_spec const & modspec)
// {
//     int nemitters, ndetectors;
//
//     if (!modspec.pmodule)
//         return false;
//
//     shared_ptr<basic_module> pmodule = modspec.pmodule;
//
//     if (_module_spec_map.find(pmodule->name()) != _module_spec_map.end()) {
//         _logger.error(fmt("%s: module already registered") % (pmodule->name()));
//         return false;
//     }
//
//     if (!pmodule->on_loaded()) {
//         _logger.error(fmt("%s: on_loaded stage failed") % pmodule->name());
//         return false;
//     }
//
//     emitter_mapper_pair const * emitters = pmodule->get_emitters(nemitters);
//     detector_mapper_pair const * detectors = pmodule->get_detectors(ndetectors);
//
//     typename api_map_type::iterator it_end = _api.end();
//
//     if (emitters) {
//         for (int i = 0; i < nemitters; ++i) {
//             int emitter_id(emitters[i].id);
//             typename api_map_type::iterator it = _api.find(emitter_id);
//
//             if (it != it_end) {
//                 it->second->mapper->append_emitter(reinterpret_cast<emitter_type *>(emitters[i].emitter));
//             } else {
//                 _logger.warn(fmt("%s: emitter '%s' not found while registering module, "
//                         "may be signal/slot mapping is not supported for this application")
//                         % (pmodule->name())
//                         % (to_string(emitters[i].id)));
//             }
//         }
//     }
//
//     if (detectors) {
//         for (int i = 0; i < ndetectors; ++i) {
//             int detector_id(detectors[i].id);
//             typename api_map_type::iterator it = _api.find(detector_id);
//
//             if (it != it_end) {
//                 it->second->mapper->append_detector(pmodule.get(), detectors[i].detector);
//             } else {
//                 _logger.warn(fmt("%s: detector '%s' not found while registering module, "
//                         "may be signal/slot mapping is not supported for this application")
//                         % (pmodule->name())
//                         % (to_string(detectors[i].id)));
//             }
//         }
//     }
//
//     pmodule->emit_module_registered.connect(this, & dispatcher::module_registered);
//     _module_spec_map.insert(pmodule->name(), modspec);
//
//     // Module must be run in a separate thread.
//     //
// //    if (pmodule->run) {
// //        _runnable_modules.push_back(pmodule);
// //        print_debug(0, current_datetime(), fmt("%s: registered as threaded") % (pmodule->name()));
// //    } else {
//         _logger.debug(fmt("%s: registered") % (pmodule->name()));
// //    }
//
//     return true;
// }
//
// /**
//  * @brief Load module descriptions in JSON format from UTF-8-encoded file
//  *        and register specified modules.
//  * @param path Path to file.
//  * @return @c true on successful loading and registration, @c false otherwise.
//  */
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// template <typename PropertyTree>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_modules (
//         filesystem::path const & path)
// {
//     pfs::error_code ec;
//
//     if (!pfs::filesystem::exists(path, ec)) {
//         if (ec) {
//             _logger.error(fmt("`%s': %s")
//                     % pfs::to_string(path)
//                     % pfs::to_string(ec));
//         } else {
//             _logger.error(fmt("`%s': file not found")
//                     % pfs::to_string(path));
//         }
//         return false;
//     }
//
//     pfs::io::device_ptr file = pfs::io::open_device<pfs::io::file>(
//             pfs::io::open_params<pfs::io::file>(path, pfs::io::read_only), ec);
//
//     if (ec) {
//         _logger.error(fmt("`%s`: file open failure: %s")
//                 % pfs::to_string(path)
//                 % pfs::to_string(ec));
//         return false;
//     }
//
//     io::input_iterator<char> first(file);
//     io::input_iterator<char> last;
//     string_type content = read_all_u8<string_type>(first, last);
//
//     PropertyTree conf;
//     ec = conf.parse(content);
//
//     if (ec) {
//         _logger.error(fmt("%s: invalid configuration: %s")
//                 (to_string(path))
//                 (to_string(ec)).str());
//         return false;
//     }
//
//     return register_modules<PropertyTree>(conf);
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// template <typename PropertyTree>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_modules (
//         PropertyTree const & conf)
// {
//     PropertyTree disp = conf["dispatcher"];
//
//     if (! disp.is_null()) {
//         if (not disp.is_object()) {
//             _logger.error("dispatcher configuration error");
//             return false;
//         }
//
//         PropertyTree dlog = disp["log"];
//
//         if (dlog.is_object()) {
//             logger_type logger;
//             typename PropertyTree::const_iterator it = dlog.cbegin();
//             typename PropertyTree::const_iterator last = dlog.cend();
//
//             for (; it != last; ++it) {
//                 string_type name = it.key();
//                 PropertyTree priority = *it;
//                 stringlist<string_type> priorities;
//                 typename log_ns::appender * pappender = 0;
//
//                 if (name == "stdout") {
//                     pappender = & logger.template add_appender<typename log_ns::stdout_appender>();
//                 } else if (name == "stderr") {
//                     pappender = & logger.template add_appender<typename log_ns::stderr_appender>();
//                 } else {
//                     // Construct path from pattern
//                     filesystem::path path(to_string(current_datetime(), name)); // `name` is a pattern here
//
//                     if (path.is_relative() && !_log_directory.empty())
//                         path = _log_directory / path;
//
//                     pappender = & logger.template add_appender<typename log_ns::file_appender>(path);
//
//                     if (! pappender->is_open()) {
//                         pfs::error_code ec = pfs::get_last_system_error();
//                         _logger.error(fmt("Failed to create/open log file: %s: %s")
//                                      (to_string(path))
//                                      (to_string(ec)).str());
//                         return false;
//                     }
//                 }
//
//                 PFS_ASSERT(pappender);
//
//                 // FIXME must be configurable {
//                 pappender->set_pattern("%d{ABSOLUTE} [%p]: %m");
//                 pappender->set_priority_text(log_ns::priority::trace, "T");
//                 pappender->set_priority_text(log_ns::priority::debug, "D");
//                 pappender->set_priority_text(log_ns::priority::info , "I");
//                 pappender->set_priority_text(log_ns::priority::warn , "W");
//                 pappender->set_priority_text(log_ns::priority::error, "E");
//                 pappender->set_priority_text(log_ns::priority::fatal, "F");
//                 // }
//
//                 if (priority.is_string()) {
//                     priorities.push_back(priority.template get<string_type>());
//                 } else if (priority.is_array()) {
//                     for (typename PropertyTree::const_iterator pri = priority.cbegin()
//                             ; pri != priority.cend(); ++pri) {
//                         priorities.push_back(pri->template get<string_type>());
//                     }
//                 }
//
//                 for (typename stringlist<string_type>::const_iterator pri = priorities.cbegin()
//                         ; pri != priorities.cend(); ++pri) {
//                     if (*pri == "all") {
//                         logger.connect(*pappender);
//                     } else if (*pri == "trace") {
//                         logger.connect(log_ns::priority::trace, *pappender);
//                     } else if (*pri == "debug") {
//                         logger.connect(log_ns::priority::debug, *pappender);
//                     } else if (*pri == "info") {
//                         logger.connect(log_ns::priority::info, *pappender);
//                     } else if (*pri == "warn") {
//                         logger.connect(log_ns::priority::warn, *pappender);
//                     } else if (*pri == "error") {
//                         logger.connect(log_ns::priority::error, *pappender);
//                     } else if (*pri == "fatal") {
//                         logger.connect(log_ns::priority::fatal, *pappender);
//                     } else {
//                         _logger.error(fmt("Invalid log level name "
//                                          "(must be 'all', 'trace', 'debug', 'info', 'warn', 'error' or 'fatal'): '%s'")
//                                      % *pri);
//                         return false;
//                     }
//                 }
//             }
//
//             _logger.swap(logger);
//         }
//     }
//
//     PropertyTree modules = conf["modules"];
//     typename PropertyTree::iterator it = modules.begin();
//     typename PropertyTree::iterator it_end = modules.end();
//
//     bool result = true;
//
//     for (; it != it_end; ++it) {
//         if (it->is_object()) {
//             string_type name_str = (*it)["name"].template get<string_type>();
//             string_type path_str = (*it)["path"].template get<string_type>();
//             bool is_active  = (*it)["active"].template get<bool>();
//             bool is_master  = (*it)["master-module"].template get<bool>();
//             string_type depends_name_str = (*it)["depends"].template get<string_type>();
//
//             if (name_str.empty()) {
//                 _logger.error("found anonymous module");
//                 return false;
//             }
//
//             if (is_active) {
//                 bool rc = false;
//
//                 if (path_str.empty())
//                     rc = register_module_for_name(name_str, 0, & *it);
//                 else
//                     rc = register_module_for_path(
//                              filesystem::path(path_str)
//                              , name_str, 0, & *it);
//
//                 if (rc) {
//                     // Module is slave.
//                     // Set master module for it.
//                     if (!depends_name_str.empty())
//                         rc = set_dependences(name_str, depends_name_str);
//                 }
//
//                 if (rc) {
//                     if (is_master) {
//                         rc = set_master_module(name_str);
//                     }
//                 }
//
//                 result = rc;
//             } else {
//                 _logger.debug(fmt("%s: module is inactive")(name_str).str());
//             }
//         }
//     }
//
//     return result;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::register_module_for_path (
//           filesystem::path const & path
//         , string_type const & name
//         , const char * class_name
//         , void * mod_data)
// {
//     module_spec modspec = module_for_path(path, class_name, mod_data);
//
//     if (modspec.pmodule) {
//         modspec.pmodule->set_name(name);
//         return register_module(modspec);
//     }
//     return false;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// typename modulus<PFS_MODULUS_TEMPLETE_ARGS>::basic_module *
// modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::find_registered_module (string_type const & name)
// {
//     typename module_spec_map_type::iterator first = _module_spec_map.begin();
//     typename module_spec_map_type::iterator last  = _module_spec_map.end();
//
//     for (; first != last; ++first) {
//         module_spec modspec = first->second;
//         shared_ptr<basic_module> pmodule = modspec.pmodule;
//
//         if (pmodule->name() == name)
//             return pmodule.get();
//     }
//
//     return static_cast<basic_module *>(0);
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::set_dependences (
//           string_type const & name
//         , string_type const & depends_name)
// {
//     basic_module * slave  = find_registered_module(name);
//
//     if (!slave) {
//         _logger.error(fmt("%s: module not found") % name);
//         return false;
//     }
//
//     basic_module * master = find_registered_module(depends_name);
//
//     if (!master) {
//         _logger.error(fmt("%s: module not found") % depends_name);
//         return false;
//     }
//
//     if (!slave->is_slave()) {
//         _logger.error(fmt("%s: module is not a slave") % name);
//         return false;
//     }
//
//     if (!master->use_async_slots()) {
//         _logger.error(fmt("%s: module must be asynchronous") % depends_name);
//         return false;
//     }
//
//     static_cast<slave_module *>(slave)->set_master(static_cast<async_module *>(master));
//
//     return true;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// bool modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::set_master_module (
//         string_type const & name)
// {
//     basic_module * master = find_registered_module(name);
//
//     if (!master) {
//         _logger.error(fmt("%s: master module not found") % name);
//         return false;
//     }
//
//     if (!master->use_async_slots()) {
//         _logger.error(fmt("%s: module must be asynchronous") % name);
//         return false;
//     }
//
//     _master_module_ptr = master;
//     return true;
//
// //     typename module_spec_map_type::iterator first = _module_spec_map.begin();
// //     typename module_spec_map_type::iterator last  = _module_spec_map.end();
// //
// //     for (; first != last; ++first) {
// //         module_spec modspec = first->second;
// //         shared_ptr<basic_module> pmodule = modspec.pmodule;
// //
// //         if (pmodule->name() == name)
// //             _master_module_ptr = pmodule.get();
// //     }
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// int modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::exec_main ()
// {
//     int r = exit_status::success;
//
//     thread_sequence_type thread_pool;
//
//     typename runnable_sequence_type::iterator irunnable      = _runnable_modules.begin();
//     typename runnable_sequence_type::iterator irunnable_last = _runnable_modules.end();
//
//     thread_function master_thread_function = 0;
//
//     for (; irunnable != irunnable_last; ++irunnable) {
//         basic_module * m = irunnable->first;
//         thread_function tfunc = irunnable->second;
//
//         // Handle deferred calls after start stage and before thread method call (run()).
//         m->_queue_ptr->call_all();
//
//         // Run module if it is not a master
//         if (m != _master_module_ptr)
//             thread_pool.push_back(pfs::make_shared<thread>(tfunc, m));
//         else
//             master_thread_function = tfunc;
//     }
//
//     // Run module if it is a master
//     if (master_thread_function) {
//         // Run dispatcher loop in separate thread
//         thread dthread(& dispatcher::run, this);
//
//         // And call master function
//         r = (_master_module_ptr->*master_thread_function)();
//
//         dthread.join();
//     } else {
//         this->run();
//     }
//
//     typename thread_sequence_type::iterator ithread      = thread_pool.begin();
//     typename thread_sequence_type::iterator ithread_last = thread_pool.end();
//
//     for (; ithread != ithread_last; ++ithread) {
//         (*ithread)->join();
//     }
//
//     return r;
// }
//
// template <PFS_MODULUS_TEMPLETE_SIGNATURE>
// int modulus<PFS_MODULUS_TEMPLETE_ARGS>::dispatcher::exec ()
// {
//     int r = exit_status::failure;
//
//     connect_all();
//
//     if (start()) {
//         r = exec_main();
//     }
//
//     finalize();
//
//     return r;
// }
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

// #define PFS_MODULE_API          extern "C" PFS_DLL_API
//
// #define PFS_DETECTOR_CAST(slot) reinterpret_cast<detector_handler>(& slot)
// #define PFS_EMITTER_CAST(e)     reinterpret_cast<void *>(& e)
//
// #define PFS_MODULE_EMITTER(id, em) { id , PFS_EMITTER_CAST(em) }
// #define PFS_MODULE_DETECTOR(id, dt) { id , PFS_DETECTOR_CAST(dt) }
//
// #define PFS_MODULE_EMITTERS_EXTERN                                             \
//     emitter_mapper_pair const *                                                \
//     get_emitters (int & count) override;
//
// #define PFS_MODULE_EMITTERS_BEGIN(XMOD)                                        \
// XMOD::emitter_mapper_pair const *                                              \
// XMOD::get_emitters (int & count)                                               \
// {                                                                              \
//     static emitter_mapper_pair __emitter_mapper[] = {
//
// #define PFS_MODULE_EMITTERS_INLINE_BEGIN                                       \
// emitter_mapper_pair const *                                                    \
// get_emitters (int & count) override                                            \
// {                                                                              \
//     static emitter_mapper_pair __emitter_mapper[] = {
//
// #define PFS_MODULE_EMITTERS_END                                                \
//     };                                                                         \
//     count = sizeof(__emitter_mapper)/sizeof(__emitter_mapper[0]);              \
//     return & __emitter_mapper[0];                                              \
// }
//
// #define PFS_MODULE_DETECTORS_EXTERN                                            \
// detector_mapper_pair const *                                                   \
// get_detectors (int & count) override;
//
// #define PFS_MODULE_DETECTORS_BEGIN(XMOD)                                       \
// XMOD::detector_mapper_pair const *                                             \
// XMOD::get_detectors (int & count)                                              \
// {                                                                              \
//     static detector_mapper_pair __detector_mapper[] = {
//
// #define PFS_MODULE_DETECTORS_INLINE_BEGIN                                      \
// detector_mapper_pair const *                                                   \
// get_detectors (int & count) override                                           \
// {                                                                              \
//     static detector_mapper_pair __detector_mapper[] = {
//
// #define PFS_MODULE_DETECTORS_END                                               \
//     };                                                                         \
//     count = sizeof(__detector_mapper)/sizeof(__detector_mapper[0]);            \
//     return & __detector_mapper[0];                                             \
// }

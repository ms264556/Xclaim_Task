#ifndef _XTSC_H_
#define _XTSC_H_

// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file xtsc.h
 *
 * @see xtsc_text_logging_macros
 */


#include <ostream>
#include <string>
#include <set>
#include <iomanip>
#include <xtsc/xtsc_types.h>
#include <xtsc/xtsc_exception.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_wire_write_if.h>
#include <xtsc/xtsc_mode_switch_if.h>
#include <log4xtensa/log4xtensa.h>


// NSPP = N=number of ports, SPP = sc_port_policy (N.A. prior to SystemC 2.2)
#if SYSTEMC_VERSION < 20060505
#define NSPP 1
#else
#define NSPP 1, sc_core::SC_ZERO_OR_MORE_BOUND
#endif


/**
 * All xtsc library objects (non-member functions, non-member data, and classes including
 * xtsc_core and xtsc_cohctrl) and their associated typedef and enum types are in the
 * xtsc namespace.
 *
 * The xtsc_comp library objects (xtsc_arbiter, xtsc_dma_engine, xtsc_lookup,
 * xtsc_lookup_driver, xtsc_lookup_pin, xtsc_master, xtsc_memory, xtsc_memory_pin,
 * xtsc_memory_trace, xtsc_mmio, xtsc_pin2tlm_memory_transactor, xtsc_queue,
 * xtsc_queue_consumer, xtsc_queue_pin, xtsc_queue_producer, xtsc_router, xtsc_slave,
 * xtsc_tlm2pin_memory_transactor, xtsc_wire, xtsc_wire_logic, and xtsc_wire_source) are
 * in the xtsc_component namespace.
 */
namespace xtsc {

class xtsc_core;


/// typedef to indicate a dummy variable used to anchor doxygen documentation comments.
typedef int Readme;



/**
 * Summary of macros to disable or to do text logging.
 *
 * The following macros may be passed to the compiler to disable text logging at compile
 * time.
 *
 * \verbatim
   XTSC_DISABLE_LOGGING             Disable all text and binary logging (FATAL, ERROR
                                    and WARN text messages will be sent to stderr).
   XTSC_DISABLE_FATAL_LOGGING       Disable all text logging (FATAL, ERROR, and WARN
                                    messages will be sent to stderr).
   XTSC_DISABLE_ERROR_LOGGING       Disable all text logging at ERROR level and below
                                    (ERROR and WARN messages will be sent to stderr).
   XTSC_DISABLE_WARN_LOGGING        Disable all text logging at WARN level and below
                                    (WARN messages will be sent to stderr).
   XTSC_DISABLE_NOTE_LOGGING        Disable all text logging at NOTE level and below.
   XTSC_DISABLE_INFO_LOGGING        Disable all text logging at INFO level and below.
   XTSC_DISABLE_VERBOSE_LOGGING     Disable all text logging at VERBOSE level and below.
   XTSC_DISABLE_DEBUG_LOGGING       Disable all text logging at DEBUG level and below.
   XTSC_DISABLE_TRACE_LOGGING       Disable all text logging at TRACE level and below.
   \endverbatim
 *
 *
 * Unless compiled out using the above macros, the following macros may be used in
 * source code to do text logging at the specified level.
 *
 * \verbatim
   XTSC_FATAL(logger, msg)
   XTSC_ERROR(logger, msg)
   XTSC_WARN(logger, msg)
   XTSC_NOTE(logger, msg)
   XTSC_INFO(logger, msg)
   XTSC_VERBOSE(logger, msg)
   XTSC_DEBUG(logger, msg)
   XTSC_TRACE(logger, msg)
   \endverbatim
 *
 * @see XTSC_FATAL
 * @see XTSC_ERROR
 * @see XTSC_WARN
 * @see XTSC_NOTE
 * @see XTSC_INFO
 * @see XTSC_VERBOSE
 * @see XTSC_DEBUG
 * @see XTSC_TRACE
 *
 */
extern Readme xtsc_text_logging_macros;



#ifndef DOXYGEN_SKIP
/*
 * Summary of macros to disable or to do binary logging.
 *
 * The following macros may be passed to the compiler to disable binary logging at
 * compile time.
 *
 * \verbatim
   XTSC_DISABLE_LOGGING             Disable all text and binary logging (FATAL, ERROR
                                    and WARN text messages will be sent to stderr).
   XTSC_DISABLE_BIN_LOGGING         Disable all binary logging.
   XTSC_DISABLE_NOTE_BIN_LOGGING    Disable all binary logging at NOTE level and below.
   XTSC_DISABLE_INFO_BIN_LOGGING    Disable all binary logging at INFO level and below.
   XTSC_DISABLE_VERBOSE_BIN_LOGGING Disable all binary logging at VERBOSE level and
                                    below.
   \endverbatim
 *
 *
 * Unless compiled out using the above macros, the following macros may be used in
 * source code to do binary logging at the specified level.
 *
 * \verbatim
   XTSC_NOTE_BIN(binaryLogger, type, logEvent)
   XTSC_INFO_BIN(binaryLogger, type, logEvent)
   XTSC_VERBOSE_BIN(binaryLogger, type, logEvent)
   \endverbatim
 *
 * @see XTSC_NOTE_BIN
 * @see XTSC_INFO_BIN
 * @see XTSC_VERBOSE_BIN
 */
//extern Readme xtsc_binary_logging_macros;
#endif // DOXYGEN_SKIP



#if defined(XTSC_DISABLE_LOGGING) && !defined(XTSC_DISABLE_FATAL_LOGGING)
#define XTSC_DISABLE_FATAL_LOGGING
#endif

#if defined(XTSC_DISABLE_FATAL_LOGGING) && !defined(XTSC_DISABLE_ERROR_LOGGING)
#define XTSC_DISABLE_ERROR_LOGGING
#endif

#if defined(XTSC_DISABLE_ERROR_LOGGING) && !defined(XTSC_DISABLE_WARN_LOGGING)
#define XTSC_DISABLE_WARN_LOGGING
#endif

#if defined(XTSC_DISABLE_WARN_LOGGING) && !defined(XTSC_DISABLE_NOTE_LOGGING)
#define XTSC_DISABLE_NOTE_LOGGING
#endif

#if defined(XTSC_DISABLE_NOTE_LOGGING) && !defined(XTSC_DISABLE_INFO_LOGGING)
#define XTSC_DISABLE_INFO_LOGGING
#endif

#if defined(XTSC_DISABLE_INFO_LOGGING) && !defined(XTSC_DISABLE_VERBOSE_LOGGING)
#define XTSC_DISABLE_VERBOSE_LOGGING
#endif

#if defined(XTSC_DISABLE_VERBOSE_LOGGING) && !defined(XTSC_DISABLE_DEBUG_LOGGING)
#define XTSC_DISABLE_DEBUG_LOGGING
#endif

#if defined(XTSC_DISABLE_DEBUG_LOGGING) && !defined(XTSC_DISABLE_TRACE_LOGGING)
#define XTSC_DISABLE_TRACE_LOGGING
#endif


#if defined(XTSC_DISABLE_LOGGING) && !defined(XTSC_DISABLE_BIN_LOGGING)
#define XTSC_DISABLE_BIN_LOGGING
#endif

#if defined(XTSC_DISABLE_NOTE_BIN_LOGGING) && !defined(XTSC_DISABLE_INFO_BIN_LOGGING)
#define XTSC_DISABLE_INFO_BIN_LOGGING
#endif

#if defined(XTSC_DISABLE_INFO_BIN_LOGGING) && !defined(XTSC_DISABLE_VERBOSE_BIN_LOGGING)
#define XTSC_DISABLE_VERBOSE_BIN_LOGGING
#endif

#if defined(XTSC_DISABLE_VERBOSE_BIN_LOGGING) && !defined(XTSC_DISABLE_DEBUG_BIN_LOGGING)
#define XTSC_DISABLE_DEBUG_BIN_LOGGING
#endif



/**
 * Macro to ensure a standard prefix to all XTSC log messages.
 *
 *  Note: This macro was changed with the RD-2011.1 release to require the user
 *        to provide a non-const string buffer.  See xtsc_log_delta_cycle().
 *
 * @see xtsc_set_text_logging_time_precision()
 * @see xtsc_set_text_logging_time_width()
 * @see xtsc_set_system_clock_factor()
 * @see xtsc_set_text_logging_delta_cycle_digits()
 */
#define XTSC_LOG_PREFIX(buf) std::setprecision(xtsc::xtsc_get_text_logging_time_precision()) \
                          << std::fixed \
                          << std::setw(xtsc::xtsc_get_text_logging_time_width()) \
                          << (sc_core::sc_time_stamp() / xtsc::xtsc_get_system_clock_period()) \
                          << xtsc::xtsc_log_delta_cycle(buf) \
                          << ": "



/**
 * Macro for logging at the FATAL_LOG_LEVEL.
 *
 * @Note calling this macro does not cause your program to terminate.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_FATAL_LOGGING)
#define   XTSC_FATAL(logger, msg)       do { \
                                          std::string buf; \
                                          if (xtsc::xtsc_is_logging_configured()) { \
                                            LOG4XTENSA_FATAL  (logger, XTSC_LOG_PREFIX(buf) << msg); \
                                          } \
                                          else { \
                                            std::cerr << "FATAL: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                          } \
                                        } while (false)

#else
#define   XTSC_FATAL(logger, msg)       do { \
                                          std::string buf; \
                                          std::cerr << "FATAL: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                        } while (false)
#endif


/**
 * Macro for logging at the ERROR_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_ERROR_LOGGING)
#define   XTSC_ERROR(logger, msg)       do { \
                                          std::string buf; \
                                          if (xtsc::xtsc_is_logging_configured()) { \
                                            LOG4XTENSA_ERROR  (logger, XTSC_LOG_PREFIX(buf) << msg); \
                                          } \
                                          else { \
                                            std::cerr << "ERROR: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                          } \
                                        } while (false)

#else
#define   XTSC_ERROR(logger, msg)       do { \
                                          std::string buf; \
                                          std::cerr << "ERROR: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                        } while (false)
#endif


/**
 * Macro for logging at the WARN_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_WARN_LOGGING)
#define    XTSC_WARN(logger, msg)       do { \
                                          std::string buf; \
                                          if (xtsc::xtsc_is_logging_configured()) { \
                                            LOG4XTENSA_WARN   (logger, XTSC_LOG_PREFIX(buf) << msg); \
                                          } \
                                          else { \
                                            std::cerr << "WARN: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                          } \
                                        } while (false)

#else
#define    XTSC_WARN(logger, msg)       do { \
                                          std::string buf; \
                                          std::cerr << "WARN: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                        } while (false)
#endif


/**
 * Macro for logging at the NOTE_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_NOTE_LOGGING)
#define    XTSC_NOTE(logger, msg)       do { \
                                          std::string buf; \
                                          if (xtsc::xtsc_is_logging_configured()) { \
                                            LOG4XTENSA_NOTE   (logger, XTSC_LOG_PREFIX(buf) << msg); \
                                          } \
                                          else { \
                                            std::cout << "NOTE: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                          } \
                                        } while (false)

#else
#define    XTSC_NOTE(logger, msg)       do { \
                                          std::string buf; \
                                          std::cout << "NOTE: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
                                        } while (false)
#endif


/**
 * Macro for logging at the INFO_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_INFO_LOGGING)
#define    XTSC_INFO(logger, msg) do { if (xtsc::xtsc_is_text_logging_enabled()) \
                                        { std::string buf; LOG4XTENSA_INFO   (logger, XTSC_LOG_PREFIX(buf) << msg); } \
                                     } while (false)
#else
#define    XTSC_INFO(logger, msg)
#endif


/**
 * Macro for logging at the VERBOSE_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_VERBOSE_LOGGING)
#define XTSC_VERBOSE(logger, msg) do { if (xtsc::xtsc_is_text_logging_enabled()) \
                                        { std::string buf; LOG4XTENSA_VERBOSE(logger, XTSC_LOG_PREFIX(buf) << msg); } \
                                     } while (false)
#else
#define XTSC_VERBOSE(logger, msg)
#endif


/**
 * Macro for logging at the DEBUG_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_DEBUG_LOGGING)
#define   XTSC_DEBUG(logger, msg) do { if (xtsc::xtsc_is_text_logging_enabled()) \
                                        { std::string buf; LOG4XTENSA_DEBUG  (logger, XTSC_LOG_PREFIX(buf) << msg); } \
                                     } while (false)
#else
#define   XTSC_DEBUG(logger, msg)
#endif


/**
 * Macro for logging at the TRACE_LOG_LEVEL.
 *
 * @param logger        A reference to a TextLogger object.
 *
 * @param msg           The message to log.  msg can be any text acceptable
 *                      to the ostringstream left-shift operator.
 */
#if !defined(XTSC_DISABLE_TRACE_LOGGING)
#define   XTSC_TRACE(logger, msg) do { if (xtsc::xtsc_is_text_logging_enabled()) \
                                        { std::string buf; LOG4XTENSA_TRACE  (logger, XTSC_LOG_PREFIX(buf) << msg); } \
                                     } while (false)
#else
#define   XTSC_TRACE(logger, msg)
#endif


/**
 * This macro is used to log at a programmatic log level.
 */
#define XTSC_LOG(logger, level, msg) do { \
  if ((xtsc::xtsc_is_text_logging_enabled() || (level >= log4xtensa::NOTE_LOG_LEVEL)) && logger.isEnabledFor(level)) { \
    std::string buf; \
    log4xtensa::tostringstream _xtsc_buf; \
    _xtsc_buf << XTSC_LOG_PREFIX(buf) << msg; \
    logger.forcedLog(level, _xtsc_buf.str(), __FILE__, __LINE__); \
  } \
  else if (level == log4xtensa::NOTE_LOG_LEVEL) { \
    std::string buf; \
    std::cout << "NOTE: " << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
  } \
  else if (level > log4xtensa::NOTE_LOG_LEVEL) { \
    std::string buf; \
    std::cerr << ((level >= log4xtensa::FATAL_LOG_LEVEL) ? "FATAL: " : (level >= log4xtensa::ERROR_LOG_LEVEL) ? "ERROR: " : "WARN: ");\
    std::cerr << XTSC_LOG_PREFIX(buf) << msg << std::endl; \
  } \
} while (false)





#ifndef DOXYGEN_SKIP
#if !defined(XTSC_DISABLE_NOTE_BIN_LOGGING)
#define XTSC_NOTE_BIN(binaryLogger, type, logEvent) do { if (xtsc::xtsc_is_binary_logging_enabled()) \
                                        { LOG4XTENSA_BIN(binaryLogger, log4xtensa::NOTE_LOG_LEVEL, type, logEvent); } } while (false)
#else
#define XTSC_NOTE_BIN(binaryLogger, type, logEvent)
#endif

#if !defined(XTSC_DISABLE_INFO_BIN_LOGGING)
#define XTSC_INFO_BIN(binaryLogger, type, logEvent) do { if (xtsc::xtsc_is_binary_logging_enabled()) \
                                        { LOG4XTENSA_BIN(binaryLogger, log4xtensa::INFO_LOG_LEVEL, type, logEvent); } } while (false)
#else
#define XTSC_INFO_BIN(binaryLogger, type, logEvent)
#endif

#if !defined(XTSC_DISABLE_VERBOSE_BIN_LOGGING)
#define XTSC_VERBOSE_BIN(binaryLogger, type, logEvent) do { if (xtsc::xtsc_is_binary_logging_enabled()) \
                                        { LOG4XTENSA_BIN(binaryLogger, log4xtensa::VERBOSE_LOG_LEVEL, type, logEvent); } } \
                                        while (false)
#else
#define XTSC_VERBOSE_BIN(binaryLogger, type, logEvent)
#endif
#endif // DOXYGEN_SKIP



/// Return true if the logging facility (log4xtensa) has been configured
XTSC_API bool xtsc_is_logging_configured();





#if !defined(SCP)
#define SCP
#undef SCP
#endif

/**
 * Method to set the XTSC system clock period (SCP).
 *
 * Although XTSC does not instantiate any sc_clock objects, it still has the concept of
 * a system clock period.  This method is used to set the XTSC system clock period as a
 * multiple of the SystemC time resolution (which can be accessed using the SystemC
 * methods sc_set_time_resolution() and sc_get_time_resolution()).  By default (i.e. if
 * this method is never called), XTSC uses a factor of 1000.  So, if neither the 
 * sc_set_time_resolution() method nor the xtsc_set_system_clock_factor() method are
 * called then the XTSC system clock period will be 1 nanosecond (which is 1000 times
 * the default SystemC time resolution of 1 picosecond).
 *
 * XTSC also has the concept of clock phase.  By default, the first posedge clock
 * conceptually occurs at time 0, the second one occurs one system clock period later,
 * and so on.  If desired, the posedge_factor argument can be set to a value other than
 * 0 (but strictly less than the clock_factor argument) to delay the first conceptual
 * posedge clock (and all subsequent ones, too).  Many of the XTSC components have
 * parameters that make reference to such things as "clock phase", "posedge clock", and
 * "negedge clock".  All these parameters are in reference to the clock phase concept
 * explained here and their effect is adjusted in accordance with any change to the
 * posedge_factor argument.
 *
 * When this method is called the the clock phase delta factors are also recalibrated.
 * If you don't want the recalibrated values then you may subsequently changed them
 * using the xtsc_core::set_clock_phase_delta_factors() method.
 *
 * This method must not be called after xtsc_get_system_clock_period() has been called.
 * The xtsc_initialize() method internally calls xtsc_get_system_clock_period, so
 * xtsc_set_system_clock_factor() must not be called after xtsc_initialize() is called.
 *
 * Technique #1: Using the logging configuration file form of the xtsc_initialize()
 * function.  The general sequence (most likely in sc_main) is:
 * \verbatim
       sc_set_time_resolution(...);                     // Optional
       xtsc_set_system_clock_factor(...);               // Optional
       xtsc_initialize(logging_config_file);
       // Now construct any desired XTSC modules (xtsc_core, xtsc_memory, etc)
   \endverbatim
 *
 * Technique #2: Using the xtsc_initialize_parms form of the xtsc_initialize() function.
 * The general sequence (most likely in sc_main) is:
 * \verbatim
       sc_set_time_resolution(...);                     // Optional
       xtsc_initialize_parms init_parms(...);
       init_parms.set("system_clock_factor", ...);      // Optional
       init_parms.set("posedge_offset_factor", ...);    // Optional
       init_parms.extract_parms(argc, argv, "xtsc");    // Optional
       xtsc_initialize(init_parms);
       // Now construct any desired XTSC modules (xtsc_core, xtsc_memory, etc)
   \endverbatim
 *
 * @see xtsc_initialize_parms
 * @see xtsc_get_system_clock_period.
 * @see xtsc_wait.
 * @see xtsc_core::set_clock_phase_delta_factors
 */
XTSC_API void xtsc_set_system_clock_factor(u32 clock_factor, u32 posedge_factor = 0);




/**
 * This method returns the factor by which the SystemC time resolution is multiplied to
 * determine the system clock period.
 * @see xtsc_set_system_clock_factor.
 */
XTSC_API u32 xtsc_get_system_clock_factor();



/**
 * Configuration parameters for the call to xtsc_initialize().
 *
 *  \verbatim
   Name                            Type         Description
   -----------------------------   -----------  ----------------------------------------
  
   "systemc_time_resolution"       char*        This read-only parameter shows the value
                                                returned from sc_get_time_resolution().

   "system_clock_factor"           u32          If this value is changed, then the
                                                xtsc_set_system_clock_factor() method
                                                is called during xtsc_initialize() with
                                                the new value supplied by this parameter.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                1000).

   "posedge_offset_factor"         u32          This specifies the time at which the
                                                first posedge of the system clock
                                                conceptually occurs.  It is expressed in
                                                units of the SystemC time resolution and
                                                must be strictly less than
                                                "system_clock_factor".
                                                Default = 0.  The default can be
                                                overridden by setting the environment
                                                variable XTSC_POSEDGE_OFFSET_FACTOR
                                                which, in turn, can be overridden by
                                                calling xtsc_set_system_clock_factor().

   "clock_phase_delta_factors"     vector<u32>  If any of these values are changed, then
                                                xtsc_core::set_clock_phase_delta_factors()
                                                is called during xtsc_initialize() with
                                                the 3 values supplied by this parameter.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                [200, 100, 600]).

   "call_sc_stop_on_finalize"      bool         The xtsc_set_call_sc_stop() method is
                                                called during xtsc_initialize() with the
                                                value supplied by this parameter.  If 
                                                this parameter is set to true, then the
                                                sc_stop() method will be called by the
                                                xtsc_finalize() method.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                true).

   "stop_after_all_cores_exit"     bool         xtsc_core::set_stop_after_all_cores_exit()
                                                is called during xtsc_initialize() with the
                                                value supplied by this parameter.  If 
                                                this parameter is set to true, then the
                                                sc_stop() method will be called when the
                                                last running core exits.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                true).

   "constructor_log_level"         char*        The xtsc_set_constructor_log_level()
                                                method is called during xtsc_initialize()
                                                with the value supplied by this parameter.
                                                Case-insensitive valid values are:
                                           FATAL|ERROR|WARN|NOTE|INFO|VERBOSE|DEBUG|TRACE
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                INFO).

   "hex_dump_left_to_right"        bool         The xtsc_set_hex_dump_left_to_right()
                                                method is called during xtsc_initialize()
                                                with the value supplied by this parameter.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                true).

   "breakpoint_csv_file"           char*        Optional name of a comma-separated value
                                                file in which to record breakpoint hits.
                                                If desired, you can specify a single
                                                hyphen for "breakpoint_csv_file" value
                                                and the output will be sent to STDOUT
                                                instead of to a disk file.
                                                The xtsc_core::set_breakpoint_callback()
                                                method provides a means for user code
                                                to get a callback each time a breakpoint
                                                is hit.  If desired, you can use this
                                                mechanism to record the breakpoint
                                                information to a CSV file using a
                                                built-in callback function instead of
                                                having to write your own.  To do this
                                                simply set this parameter to the desired
                                                name ofthe CSV file to generated.  At
                                                the start of simulation, this file will
                                                be created and for each user breakpoint
                                                hit in target code, a line with 9 CSV
                                                values will be added to this file in the
                                                following format:
                                        SimulationTime,"CoreName",PC,CycleCount,CCOUNT
                                                Default = NULL

   "simcall_csv_file"              char*        Optional name of a comma-separated value
                                                file in which to record user simcalls.
                                                If desired, you can specify a single
                                                hyphen for "simcall_csv_file" value and
                                                the output will be sent to STDOUT
                                                instead of to a disk file.
                                                Xtensa ISS provides a means for target
                                                code to call a user-provided function in
                                                the simulator and pass it up to 6 values
                                                (see xtsc_core::set_simcall_callback).
                                                If desired, you can use this mechanism
                                                to record the 6 simcall arguments to a
                                                CSV file using a built-in callback
                                                function instead of having to write your
                                                own.  To do this simply set this
                                                parameter to the desired name ofthe CSV
                                                file to generated.  At the start of
                                                simulation, this file will be created
                                                and for each user simcall executed in
                                                target code, a line with 10 CSV values
                                                will be added to this file in the
                                                following format:
         SimulationTime,"CoreName",CycleCount,CCOUNT,arg1,arg2,arg3,arg4,arg5,arg6
                                                By default, each argN value is printed
                                                as a decimal number but this can be
                                                changed using the "simcall_csv_format" 
                                                parameter.
                                                Default = NULL

   "simcall_csv_format"            vector<u32>  Optional vector of from 0 to 6 values
                                                used to specify the output format of the
                                                6 simcall arguments.  The first value
                                                specifies the format for arg1, the
                                                second for arg2, and so on (see
                                                "simcall_csv_file").  A value of 0 means
                                                to print the corresponding argument as a
                                                decimal number in the CSV file.  A value
                                                of 1 means to print it as hexadecimal with
                                                "0x" prefix.  A value of 2 means to
                                                interrpret the value as a pointer to a
                                                null-terminated C-string (char*) in the
                                                core's address space and to print the
                                                string.  A value of 3 means to not print
                                                the corresponding argument.  If less
                                                than 6 values are specified the missing
                                                values default to 0.
                                                Default (unset).

   "target_memory_limit"           u32          The total limit in megabytes of target
                                                memory space that the ISS will allocate
                                                for local memories modelled internally.
                                                This limit does not apply to XTSC
                                                memory components such as xtsc_memory.
                                                Default = 512 megabytes (0x20000000).

   "text_logging_config_file"      char*        The configuration file for text logging.
                                                If NULL (the default) or empty, and the
                                                XTSC_TEXTLOGGER_CONFIG_FILE environment
                                                variable is defined and NOT equal to off
                                                (case-insensitive) then the contents of
                                                that environment variable will be taken
                                                as the optional path and name of the
                                                configuration file to be used.  If the
                                                configuration file named by the
                                                XTSC_TEXTLOGGER_CONFIG_FILE environment
                                                variable does not exists, it will be
                                                created and initialized to contain
                                                configuration information to send
                                                messages at INFO_LOG_LEVEL and higher to
                                                a file in the current working directory
                                                called xtsc.log (or whatever is
                                                specified by the XTSC_LOG_FILE_NAME
                                                environment variable) and to also send
                                                messages at NOTE_LOG_LEVEL and higher to
                                                the console.  If
                                                "text_logging_config_file" is NULL or
                                                empty and the
                                                XTSC_TEXTLOGGER_CONFIG_FILE environment
                                                variable is not defined, or is defined
                                                and equal to off, then text logging will
                                                be configured so that messages at
                                                NOTE_LOG_LEVEL and higher will be sent
                                                to the console and other messages will
                                                be discarded.  In this case if the
                                                XTSC_TEXTLOGGER_CONFIG_FILE is equal to
                                                off, even though the log4xtensa library
                                                will be configured as just specified,
                                                the xtsc_enable_text_logging() method
                                                will be called with an argument of false
                                                to disable all text logging at
                                                INFO_LOG_LEVEL and below.

   "text_logging_delta_cycle_digits" u32        xtsc_set_text_logging_delta_cycle_digits()
                                                is called during xtsc_initialize() with the
                                                value supplied by this parameter.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                1).

   "text_logging_disable"          bool         The xtsc_enable_text_logging() method is
                                                called during xtsc_initialize() with the
                                                value supplied by this parameter unless
                                                "text_logging_config_file" is NULL or
                                                empty and the
                                                XTSC_TEXTLOGGER_CONFIG_FILE environment
                                                is defined and equal to off.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                false).

   "text_logging_time_precision"   u32          The xtsc_set_text_logging_time_precision()
                                                method is called during xtsc_initialize()
                                                with the value supplied by this parameter.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                1).

   "text_logging_time_width"       u32          The xtsc_set_text_logging_time_width()
                                                method is called during xtsc_initialize()
                                                with the value supplied by this parameter.
                                                Default = as set at xtsc_initialize_parms
                                                construction time (which, by default, is
                                                10).

   "turbo"                         bool         This parameter controls the default
                                                setting of the xtsc_core_parms parameter
                                                "SimTurbo".  
                                                Default = false.

   "turbo_max_relaxed_cycles"      u32          This specifies the maximum total amount
                                                that a device may run ahead of actual
                                                simulation time without yielding to the
                                                SystemC kernel when operating in the
                                                functional mode of TurboXim.  This
                                                amount is expressed in terms of 
                                                system clock periods.
                                                See xtsc_get_system_clock_period().
                                                Default = 10000000.

   "turbo_min_sync"                bool         By default, after TurboXim has run ahead
                                                and returns control back to the SystemC
                                                kernel, it synchronizes the SystemC time
                                                by waiting for a number of clock periods
                                                approximately corresponding to the
                                                number of instructions executed.  If
                                                desired, you can limit this wait to a
                                                single cycle by setting this parameter
                                                to true.
                                                Note: Setting this parameter to true
                                                when using xtsc_dma_engine can cause
                                                problems.  See the "turbo" parameter in
                                                xtsc_component::xtsc_dma_engine_parms.
                                                Default = false.

   "xtsc_finalize_unwind"          bool         If this parameter is set to false, then
                                                the first call to xtsc_finalize() will
                                                finalize simulation and subsequent calls
                                                will be ignored.  If this parameter is
                                                true, then xtsc_finalize() must be called
                                                as many times as xtsc_initialize() has
                                                been called before simulation will
                                                actually be finalized.
                                                Note:  Simulation is always initialized
                                                upon the first call to xtsc_initialize()
                                                and subsequent calls are tallied but
                                                otherwise ignored.
                                                Default = false.

    \endverbatim
 *
 * @see xtsc_initialize
 * @see xtsc_parms
 * @see xtsc_core_parms
 */
class XTSC_API xtsc_initialize_parms : public xtsc_parms {
public:

  /**
   * Constructor for an xtsc_initialize_parms object.
   *
   * @param text_logging_config_file    The value for the "text_logging_config_file"
   *                                    parameter.
   */
  xtsc_initialize_parms(const char *text_logging_config_file = NULL);

  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_initialize_parms"; }
};



/**
 * Initialize XTSC simulation.
 *
 * This function should be called before constructing any XTSC modules
 * and before generating any logging messages.  Generally this function
 * should be called from sc_main before constructing any XTSC modules.
 * If you have any global objects then those objects should not generate
 * logging messages in their constructor or they should call this method
 * prior to generating any logging messages.
 *
 * @param init_parms    The parameters used to configure XTSC.
 *
 * @see xtsc_initialize_parms
 */
XTSC_API void xtsc_initialize(const xtsc_initialize_parms& init_parms);



/**
 * Initialize XTSC simulation.
 *
 * This function should be called before constructing any XTSC modules and before
 * generating any logging messages.  This function constructs a default
 * xtsc_initialize_parms object.  Sets the "text_logging_config_file" parameter
 * according to the text_logging_config_file value passed in to this function, and then
 * calls xtsc_initialize with the xtsc_initialize_parms object.
 *
 * @param text_logging_config_file      Value to set the "text_logging_config_file" parameter
 *                                      of the xtsc_initialize_parms object to.
 *
 * @param binary_logging_config_file    The configuration file for binary
 *                                      logging.  If NULL (the default), then
 *                                      binary loggers are disabled for this
 *                                      simulation run.
 *
 * NOTE:  Binary logging is not supported in Xtensa Tools 7.0.
 *
 * @see xtsc_initialize_parms
 */
XTSC_API void xtsc_initialize(const char *text_logging_config_file = NULL, const char *binary_logging_config_file = NULL);


/// Return a copy of the xtsc_initialize_parms used to initialize XTSC
XTSC_API xtsc_initialize_parms xtsc_get_xtsc_initialize_parms();



/// Return true if xtsc_initialize has been called, else return false.
XTSC_API bool xtsc_is_initialized();



/**
 * This function should be called when simulation is over to
 * ensure all resources are properly released and all clients
 * are properly finalized.  Not calling this method can result
 * in, for example, no profile client output being generated.
 */
XTSC_API void xtsc_finalize();



/**
 * Set the flag that determines whether sc_stop() will be called when xtsc_finalize() is
 * called.  The default value of the flag is true.  That is, if this method is never
 * called then sc_stop() will be called when xtsc_finalize() is called.
 *
 * Note: Calling sc_stop causes failure during the elaboration phase for some Cadence 
 *       versions (for example, IUS 10.2 s010/s012; but not s017).  Message is:
 *       ncverilog: *E,ELBERR: Error during elaboration (status 250), exiting
 *       
 * @param       call_sc_stop    If true, sc_stop() will be called from xtsc_finalize().
 *
 * @return the previous value of the flag
 */
XTSC_API bool xtsc_set_call_sc_stop(bool call_sc_stop);



/**
 * This method returns the period of the conceptual system clock.
 * @see xtsc_set_system_clock_factor.
 */
XTSC_API sc_core::sc_time xtsc_get_system_clock_period();



/**
 * Get the posedge offset of the conceptual system clock.
 *
 * This methods returns the amount of time by which the first posedge of the conceptual
 * system clock is offset from time 0.  It is computed by multiplying the value of the
 * "posedge_offset_factor" parameter of xtsc_initialize_parms by the SystemC time
 * resolution.  This method must not be called before xtsc_initialize() is called.
 *
 * @see xtsc_initialize_parms.
 */
XTSC_API sc_core::sc_time xtsc_get_system_clock_posedge_offset();



/**
 * This method waits the specified number of system clock periods.
 *
 * This is just a convenience method that calls sc_core::wait() with an sc_time object
 * equal to num_periods times the system clock period.
 *
 * @param       num_periods     The number of system clock periods to wait.
 *
 * @see xtsc_set_system_clock_factor.
 * @see xtsc_get_system_clock_period.
 */
XTSC_API void xtsc_wait(u32 num_periods = 1);



/// This method returns true if host processor is big endian, otherwise returns false
XTSC_API bool xtsc_is_big_endian_host();



/**
 * This function returns a unique 64-bit number that can be associate with a new element
 * when it is added to a queue.  This function is meant for use by queue implementations.
 * It is the queue implementation's job to maintain the ticket-to-element association and
 * to return and/or log the ticket with its associated element is popped from the queue.
 *
 * @return a unique 64-bit number.
 *
 * @see xtsc_queue_push_if
 * @see xtsc_queue_pop_if
 */
XTSC_API u64 xtsc_create_queue_ticket();



/**
 * Fire (that is, signal or notify) the specified TurboXim event id.  This function has no
 * effect if called when operating in cycle-accurate (non-TurboXim) mode.
 *
 * When you use the TurboXim simulation engine, you can significantly improve
 * performance by allowing each core to run a large number of instructions at a time.
 * In this relaxed simulation mode, you may want to programmatically control when a core
 * should yield control to allow other cores and other SystemC processes to execute.  To
 * force a core to wait for an event when running in the fast functional simulation
 * mode (TurboXim), you should call the following function from your Xtensa target
 * program:
 *
 *    void xt_iss_event_wait(unsigned eventId);
 *
 * You can fire the event on which a core is waiting by calling
 * xtsc_fire_turboxim_event_id() from the host program and passing the same event ID as
 * was passed in the xt_iss_event_wait() call.  This might be done, for example, from
 * sc_main() or, more typically, from a thread or method process of a SystemC module.
 *
 * Alternatively, the event on which a core is waiting can be fired by calling
 *
 *    void xt_iss_event_fire(unsigned eventId);
 *
 * from the target program running on another core and passing the same eventId number.
 *
 * Prototypes for both target functions are provided in the <xtensa/sim.h> header file.
 *
 */
XTSC_API void xtsc_fire_turboxim_event_id(u32 turboxim_event_id);



/**
 * This method is used to set an absolute time barrier for use by modules operating in
 * fast functional mode.  When the current SystemC simulation time equals or exceeds
 * this barrier, then modules should not run ahead of the current SystemC simulation
 * time.
 *
 * If this method is not called then there is effectively no absolute time barrier.
 *
 * This function is commonly called before calling sc_start() when sampled simulation is
 * to be used.
 *
 * @param delta         This time is added to the current SystemC time to compute the
 *                      absolute time barrier.
 */
XTSC_API void xtsc_set_relaxed_simulation_barrier(const sc_core::sc_time& delta);



/**
 * This method returns the absolute simulation time barrier beyond which modules should
 * cease running ahead of the current SystemC simulation time when in fast functional
 * mode.  This is the absolute simulation time when relaxed simulation (i.e. "running
 * ahead") should cease.
 *
 * If the xtsc_set_relaxed_simulation_barrier() method is never called, then this method
 * will return the maximum possible SystemC simulation time less one SystemC time
 * resolution, that is ((2^64) - 1) times the SystemC time resolution.
 */
XTSC_API sc_core::sc_time xtsc_get_relaxed_simulation_barrier();



/**
 * Set the amount of "equivalent time" that a module is allowed to run ahead of the
 * actual SystemC simulation time when operating in fast functional mode.
 *
 * This method should be called before simulation begins or should be left at its
 * default value of SC_ZERO_TIME.
 *
 * When this interval is SC_ZERO_TIME, then cores and other modules should not run ahead
 * of the current SystemC simulation time.
 *
 * @param       interval        Maximum amount of "equivalent time" that a module may
 *                              run ahead of the current SystemC simulation time.
 */
XTSC_API void xtsc_set_relaxed_simulation_interval(const sc_core::sc_time &interval);



/**
 * Get the amount of "equivalent time" that a module is allowed to run ahead of the
 * actual SystemC simulation time when operating in fast functional mode as set by
 * the xtsc_set_relaxed_simulation_interval() method.
 *
 * This method does not take the absolute simulation time barrier into account.
 * Use xtsc_get_remaining_relaxed_simulation_time() to get the max
 * duration that a module should use.
 */
XTSC_API sc_core::sc_time xtsc_get_relaxed_simulation_interval(void);



/**
 * This method returns the maximum amount of time that a module may run ahead of the
 * current SystemC simulation time.
 *
 * When a system is executing in fast functional mode, cores and other modules are
 * allowed to run ahead of the current simulation time.  The amount of "equivalent time"
 * that a module is allowed to run ahead of the actual SystemC simulation time is
 * limited by two things:
 *
 * First, a module must not run ahead of the current simulation time by an amount
 * greater then the interval specified by the xtsc_set_relaxed_simulation_interval()
 * method.
 *
 * Second, a module should not run ahead when the current SystemC simulation time
 * equals or exceeds the absolute simulation time barrier as set by the
 * xtsc_set_relaxed_simulation_barrier() method.
 *
 * If the xtsc_set_relaxed_simulation_interval() method has not been called (or if it
 * was called with an argument of SC_ZERO_TIME) or if the absolute simulation time
 * barrier as set by the xtsc_set_relaxed_simulation_barrier() method has already
 * arrived or passed, then this method returns SC_ZERO_TIME.  Otherwise, this method
 * returns the smaller of the interval set by the xtsc_set_relaxed_simulation_interval()
 * method and the difference between the current SystemC simulation time and the
 * absolute simulation time barrier as set by the xtsc_set_relaxed_simulation_barrier()
 * method.
 *
 */
XTSC_API sc_core::sc_time xtsc_get_remaining_relaxed_simulation_time(void);



#ifndef DOXYGEN_SKIP
// Deprecated function names being maintained for backward compatibility

inline void xtsc_set_max_relaxed_duration(const sc_core::sc_time &interval) { xtsc_set_relaxed_simulation_interval(interval); }

inline sc_core::sc_time xtsc_get_max_relaxed_duration(void) { return xtsc_get_relaxed_simulation_interval(); }

inline void xtsc_set_relaxed_simulation_duration(const sc_core::sc_time& delta) { xtsc_set_relaxed_simulation_barrier(delta); }

inline sc_core::sc_time xtsc_get_relaxed_simulation_time_limit() { return xtsc_get_relaxed_simulation_barrier(); }

inline sc_core::sc_time xtsc_get_remaining_relaxed_duration(void) { return xtsc_get_remaining_relaxed_simulation_time(); }
#endif



/**
 * This is needed for when XTSC is being used with some commercial SystemC simulators.
 */
XTSC_API bool xtsc_trap_port_binding_failures(bool trap);



/**
 * This method sets the log level for constructor logging.  Normally, the XTSC module
 * constructors (xtsc_core, xtsc_memory, xtsc_queue, etc) will log certain construction
 * information and parameters at INFO_LOG_LEVEL.  This method can be called to change
 * that to a different log level.
 *
 * @param       log_level       The new log level for XTSC module constructor logging.
 *
 * @return previous value
 */
XTSC_API log4xtensa::LogLevel xtsc_set_constructor_log_level(log4xtensa::LogLevel log_level);



/// Get the current log level for constructor logging
XTSC_API log4xtensa::LogLevel xtsc_get_constructor_log_level();



/// Determine if text logging is enabled
XTSC_API bool xtsc_is_text_logging_enabled();



/**
 * Turn text logging on or off for XTSC_INFO and lower.
 *
 * This method enables or disables text logging for XTSC_INFO and lower.  If text
 * logging is disabled, the XTSC_INFO and lower macros do not generate logging messages,
 * but XTSC_NOTE and higher macros still work as usual.  Calling this method with
 * enable_logging set to false results in near zero logging facility overhead within the
 * XTSC libraries.
 *
 * @param       enable_logging  If true, the default, text logging is enabled as normal.
 *                              Otherwise, text logging is disabled for XTSC_INFO,
 *                              XTSC_VERBOSE, XTSC_DEBUG, and XTSC_TRACE macros.
 *
 * @return previous value
 */
XTSC_API bool xtsc_enable_text_logging(bool enable_logging = true);



/// Set number of digits after decimal point used when logging simulation time
XTSC_API void xtsc_set_text_logging_time_precision(u32 num_decimal_digits);



/// Get number of digits after decimal point used when logging simulation time
XTSC_API u32  xtsc_get_text_logging_time_precision();



/// Set total number of characters used when logging simulation time
XTSC_API void xtsc_set_text_logging_time_width(u32 num_characters);



/// Get total number of characters used when logging simulation time
XTSC_API u32  xtsc_get_text_logging_time_width();



/**
 * Set the number of least-significant decimal digits of the delta cycle
 * count to be displayed when logging.
 */
XTSC_API void xtsc_set_text_logging_delta_cycle_digits(u32 num_digits);



/**
 * Get the number of least-significant decimal digits of the delta cycle
 * count to be displayed when logging.
 */
XTSC_API u32 xtsc_get_text_logging_delta_cycle_digits();



/**
 *  Put the formatted delta cycle count into string reference buf and return it.
 *
 *  Note: This function was changed with the RD-2011.1 release to require the user
 *        to provide a non-const string buffer.  The caller should create a string
 *        on the stack and pass it to this function.  This is needed for multi-thread
 *        safety.  Although SystemC is single-thread, XTSC will create other OS-level
 *        threads if an xtsc_core has debugging enabled for its target program.  For 
 *        an example usage, see the XTSC_INFO macro.
 */
XTSC_API std::string& xtsc_log_delta_cycle(std::string &buf);



#ifndef DOXYGEN_SKIP
/// Determine if binary logging is enabled
XTSC_API bool xtsc_is_binary_logging_enabled();



/**
 * Turn binary logging on or off.
 *
 * This method enables or disables binary logging.
 *
 * @param       enable_logging  If true, the default, binary logging is enabled.
 *                              Otherwise, binary logging is disabled.
 *
 * @return previous value
 */
XTSC_API bool xtsc_enable_binary_logging(bool enable_logging = true);



/// Dump currently registered binary loggers
XTSC_API void xtsc_dump_binary_loggers(std::ostream& os = std::cout);
#endif // DOXYGEN_SKIP



/// Dump currently registered text loggers
XTSC_API void xtsc_dump_text_loggers(std::ostream& os = std::cout);



/**
 * This method sets the flag that determines the order in which data is
 * dumped by the xtsc_hex_dump(u32, const u8 *, ostream&) method.  The
 * initial (default) value of this flag is true.
 *
 * @param       left_to_right   If true, data is dumped in the order:
 *                              buffer[0], buffer[1], ..., buffer[size8-1].
 *                              If false, data is dumped in the order:
 *                              buffer[size8-1], buffer[size8-2], ..., buffer[0].
 *
 * @return the old (previous) value of the flag.
 */
XTSC_API bool xtsc_set_hex_dump_left_to_right(bool left_to_right);



/**
 * This method returns the flag that determines the order in which data is
 * dumped by the xtsc_hex_dump(u32, const u8 *, ostream&) method.
 *
 * @return If true, data is dumped in the order:
 *            buffer[0], buffer[1], ..., buffer[size8-1].
 *         If false, data is dumped in the order:
 *            buffer[size8-1], buffer[size8-2], ..., buffer[0].
 */
XTSC_API bool xtsc_get_hex_dump_left_to_right();



/**
 * This method dumps the specified number of bytes from the data buffer in hex
 * format (two hex nibbles and a space for each byte in the buffer).  The data
 * is dumped in the order specified by the xtsc_get_hex_dump_left_to_right()
 * method.
 *
 * @param       size8           The number of bytes of data to dump.
 *
 * @param       buffer          The buffer of data.
 *
 * @param       os              The ostream object to which the data is to be
 *                              dumped.
 */
XTSC_API void xtsc_hex_dump(u32 size8, const u8 *buffer, std::ostream& os = std::cout);



/**
 * This method dumps the specified number of bytes from the data buffer.  Each
 * line of output is divided into three columnar sections, each of which is
 * optional.  The first section contains an address.  The second section contains
 * a hex dump of some (possibly all) of the data (two hex nibbles and a space for
 * each byte from the buffer).  The third section contains an ASCII dump of the
 * same data.
 *
 * @param       left_to_right           If true, the data is dumped in the order:
 *                                      buffer[0], buffer[1], ..., buffer[bytes_per_line-1].
 *                                      If false, the data is dumped in the order:
 *                                      buffer[bytes_per_line-1], buffer[bytes_per_line-2],
 *                                      ..., buffer[0].
 *
 * @param       size8                   The number of bytes of data to dump.
 *
 * @param       buffer                  The buffer of data.
 *
 * @param       os                      The ostream object to which the data is to be
 *                                      dumped.
 *
 * @param       bytes_per_line          The number of bytes to dump on each line of output.
 *                                      If bytes_per_line is 0 (the default) then all size8
 *                                      bytes are dumped on a single line with no newline at
 *                                      the end.  If bytes_per_line is non-zero, then all
 *                                      lines of output end in newline.
 *
 * @param       show_address            If true, the first columnar section contains an
 *                                      address printed as an 8-hex-digit number with a 0x
 *                                      prefix.  If false, the first columnar section is null
 *                                      and takes no space in the output.
 *
 * @param       start_byte_address      If show_address is true, the first line of output
 *                                      starts with start_byte_address, the second line of
 *                                      output starts with start_byte_address+bytes_per_line,
 *                                      and so on.  If show_address is false, this parameter
 *                                      is ignored.
 *
 * @param       show_hex_values         If true, the second (middle) columnar section of
 *                                      hex data values is printed.  If false, the second
 *                                      columnar section is null and takes no space in the
 *                                      output.
 *
 * @param       do_column_heading       If true, print byte position column headings over
 *                                      the hex values section.  If false, no column headings
 *                                      are printed.  If show_hex_values is false, then the
 *                                      do_column_heading value is ignored and no column
 *                                      headings are printed.
 *
 * @param       show_ascii_values       If true, the third (last) columnar section of ASCII
 *                                      data values is printed (if an ASCII value is a
 *                                      non-printable character a period is printed).  If
 *                                      show_ascii_values is false, the third columnar
 *                                      section is null and takes no space in the output.
 *
 * @param       initial_skipped_bytes   Skip initial_skipped_bytes bytes on the first line
 *                                      in the second (hex) and third (ASCII) columnar
 *                                      sections.
 */
XTSC_API
void xtsc_hex_dump(bool                 left_to_right,
                   u32                  size8,
                   const u8            *buffer,
                   std::ostream&        os                      = std::cout,
                   u32                  bytes_per_line          = 0,
                   bool                 show_address            = false,
                   u32                  start_byte_address      = 0x0,
                   bool                 show_hex_values         = true,
                   bool                 do_column_heading       = false,
                   bool                 show_ascii_values       = false,
                   u32                  initial_skipped_bytes   = 0);



/// Utility method to convert a string of comma-separated values to a vector<u32> (or throw xtsc_exception)
XTSC_API void xtsc_strtou32vector(const std::string& str, std::vector<u32>& vec);



/// Utility method to convert a string to a u32 (or throw xtsc_exception)
XTSC_API u32 xtsc_strtou32(const std::string& str);



/// Utility method to convert a string to a u64 (or throw xtsc_exception)
XTSC_API u64 xtsc_strtou64(const std::string& str);



/// Utility method to convert a string to a i32 (or throw xtsc_exception)
XTSC_API i32 xtsc_strtoi32(const std::string& str);



/// Utility method to convert a string to a double (or throw xtsc_exception)
XTSC_API double xtsc_strtod(const std::string& str);





/// Maximum size of an Xtensa memory bus in bytes
static const u32 xtsc_max_bus_width8 = 512/8;

/// Maximum size of a TIE port or queue in bytes
static const u32 xtsc_max_tie_port_width8 = 1024/8;

/// Maximum size of a register in bytes
static const u32 xtsc_max_register_width8 = 2048/8;




/**
 * Class for registering TurboXim simulation mode switching interfaces.
 */
class XTSC_API xtsc_switch_registration {
 public:
  sc_core::sc_object        *m_object;
  xtsc::xtsc_mode_switch_if *m_switch_if;
  std::string                m_switch_group;

  /**
   * constructor for a module_name, switching interface, group triplet
   *
   * @param module_name     The module that is registering the switch
   * @param switch_in       The interface class that implements mode switching
   * @param switch_group    The name of a group of modules that are
   *                        generally switched together
   */
  xtsc_switch_registration(sc_core::sc_object &obj,
                           xtsc::xtsc_mode_switch_if &switch_if,
                           std::string switch_group);
};



/**
 *  Registration function for registering TurboXim simulation mode switching interfaces.
 *
 *  @param registration    a module name, interface, switching group name
 *                         triplet to register.
 */
XTSC_API void xtsc_register_mode_switch_if(const xtsc::xtsc_switch_registration &registration);



/**
 *  Fill a vector will all of the registered switch groups.
 *
 */
XTSC_API void xtsc_get_registered_mode_switch_ifs(std::vector<xtsc::xtsc_switch_registration> &ifs);



/**
 *  Switch all modules in all switch groups that have registered a
 *  simulation mode switching interface.  This function will throw an
 *  exception if one or more of the modules was unable to switch.  It
 *  should always be valid to switch mode before simulation starts.
 *  After simulation starts, use the mode-switching protocol to relax
 *  the transactions in the system before invoking this function.
 *  Throws an exception if any of the registered modules cannot switch.
 *
 * @param mode      If mode is XTSC_CYCLE_ACCURATE, then switch to
 *                  cycle-accurate (non-TurboXim for cores) mode.  If mode is
 *                  XTSC_FUNCTIONAL, then switch to functional mode
 *                  (TurboXim for cores).
 */
XTSC_API void xtsc_switch_sim_mode(xtsc::xtsc_sim_mode mode);



/**
 *  Polling-based dynamic simulation switching preparation.
 *
 *  This function should be used prior to switching the simulation
 *  mode as part of the mode-switching protocol to remove or drain
 *  transactions from the system.
 *
 *  Prepare to switch all modules in all switch groups that have
 *  registered a simulation mode switching interface.  This function
 *  will return true if all modules are ready to switch.  Otherwise,
 *  it will return false.  If it returns false, the switcher should
 *  wait at least a cycle before trying again.  Once all modules are
 *  ready, there may still be transactions in passive modules.  The
 *  user should wait enough cycles for all passive modules to
 *  propagate transactions through the system, try one more time, and
 *  switch if all modules are still ready.
 *
 * @param mode      If mode is XTSC_CYCLE_ACCURATE, then switch to
 *                  cycle-accurate (non-TurboXim for cores) mode.  If mode is
 *                  XTSC_FUNCTIONAL, then switch to functional mode
 *                  (TurboXim for cores).
 */
XTSC_API bool xtsc_prepare_to_switch_sim_mode(xtsc::xtsc_sim_mode mode);



/**
 * Compute the swizzle value from memory storage.
 *
 * @param buf  the pointer to memory storage
 *
 * @return the swizzle value to use in xtsc_fast_access_request::allow_raw_access()
 *         or 0xffffffff if no swizzle matches.
 */
XTSC_API u32 xtsc_compute_fast_access_swizzle(const u32 *buf);



/**
 * Get the portion of the path before the last path separator.  On Linux this method
 * just returns dirname(path).
 */
XTSC_API char *xtsc_dirname(char *path);



/**
 * Get the portion of the path after the last path separator.  On Linux this method 
 * just returns basename(path).
 */
XTSC_API char *xtsc_basename(char *path);



/**
 * Utility function to get the absolute path of an existing directory.
 *
 * @param directory  The directory whose absolute path is desired.  directory may be an
 *                   absolute or relative path.
 *
 * @return the absolute path of the directory containing directory
 */
XTSC_API std::string xtsc_get_absolute_directory(const std::string& directory);



/**
 * Utility function to get the absolute path of the directory containing an existing file.
 *
 * @param file_name  The file whose absolute path is desired.  file_name may include an
 *                   absolute or relative path.
 *
 * @return the absolute path of the directory containing file_name
 */
XTSC_API std::string xtsc_get_absolute_path(const std::string& file_name);



/**
 * Utility function to get the absolute path and file name of an existing file.
 *
 * @param file_name  The file whose absolute path is desired.  file_name may include an
 *                   absolute or relative path.
 *
 * @return the absolute path to file_name
 */
XTSC_API std::string xtsc_get_absolute_path_and_name(const std::string& file_name);



/**
 * This function splits up a multi-line message into multiple calls
 * to the TextLogger::log() method (one call per line).
 *
 * @param       logger          The TextLogger object.
 *
 * @param       log_level       The log level of this message.
 *
 * @param       msg             The message to log.
 *
 * @param       indent          The number of spaces to indent all
 *                              lines except the first one.
 */
XTSC_API 
void xtsc_log_multiline(log4xtensa::TextLogger& logger,
                        log4xtensa::LogLevel    log_level,
                        const std::string&      msg,
                        u32                     indent = 0);



#ifndef DOXYGEN_SKIP
// This class should not be used except by the friend classes
class XTSC_API xtsc_length_context {
private:
  xtsc_length_context(u32 length);
  void end();
  sc_dt::sc_length_context length_context;
  friend class xtsc_signal_sc_bv_base;
};
#endif // DOXYGEN_SKIP



/**
 * Pin-level signal for connecting to a TIE export state, TIE import wire, or
 * system-level I/O of xtsc_core.
 *
 * This convenience class implements a pin-level signal for connecting to a TIE export
 * state, TIE import wire, or system-level I/O of an xtsc_core.  The advantage of using
 * this class instead of sc_signal<sc_bv_base> (from which it inherits) is that you can
 * directly specify the bit-width of the underlying sc_bv_base using the width1
 * constructor parameter instead of having to indirectly specify it through a separate
 * sc_length_context object.
 *
 * This is a convenience class.  If desired you may use sc_signal<sc_bv_base> in lieu of
 * xtsc_signal_sc_bv_base for connecting to a pin-level input or output of xtsc_core.
 * In either case, however, the TIE export state, TIE import wire, or system-level I/O
 * interfaces must be named in the xtsc_core object's "SimPinLevelInterfaces" parameter
 * in order for it to be available as a pin-level input or output.
 *
 * @see xtsc_core::How_to_do_output_pin_binding;
 * @see xtsc_core::How_to_do_input_pin_binding;
 */
class XTSC_API xtsc_signal_sc_bv_base : public xtsc_length_context, public sc_core::sc_signal<sc_dt::sc_bv_base> {
public:
  xtsc_signal_sc_bv_base(u32 width1);
  xtsc_signal_sc_bv_base(const char *name, u32 width1);

  /// Get the width in bits of this signal.
  u32 get_bit_width() const { return m_width1; }

  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_signal_sc_bv_base"; }

private:
  u32   m_width1;
};



/**
 * Floating signal for a capped (unused) TIE export state or import wire.
 *
 * This class implements the signal use to cap an unconnected TIE export state or TIE
 * import wire.
 */
class XTSC_API xtsc_signal_sc_bv_base_floating: public xtsc_signal_sc_bv_base {
public:
  xtsc_signal_sc_bv_base_floating(u32 width1, log4xtensa::TextLogger& text);
  xtsc_signal_sc_bv_base_floating(const char *name, u32 width1, log4xtensa::TextLogger& text);

  const sc_dt::sc_bv_base& read() const;
  void write(const sc_dt::sc_bv_base& value);

  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_signal_sc_bv_base_floating"; }

private:
  log4xtensa::TextLogger&       m_text;
};



#ifndef DOXYGEN_SKIP
struct XTSC_API is_true  { static const bool value = true;  };
struct XTSC_API is_false { static const bool value = false; };

template <typename T> struct is_integral                        : is_false {};
template <>           struct is_integral  <              bool > : is_true  {};
template <>           struct is_integral  <              char > : is_true  {};
template <>           struct is_integral  <  signed      char > : is_true  {};
template <>           struct is_integral  <  signed      short> : is_true  {};
template <>           struct is_integral  <  signed      int  > : is_true  {};
template <>           struct is_integral  <  signed      long > : is_true  {};
template <>           struct is_integral  <  signed long long > : is_true  {};
template <>           struct is_integral  <unsigned      char > : is_true  {};
template <>           struct is_integral  <unsigned      short> : is_true  {};
template <>           struct is_integral  <unsigned      int  > : is_true  {};
template <>           struct is_integral  <unsigned      long > : is_true  {};
template <>           struct is_integral  <unsigned long long > : is_true  {};

template <typename T> struct is_integral64                      : is_false {};
template <>           struct is_integral64<  signed long long > : is_true  {};
template <>           struct is_integral64<unsigned long long > : is_true  {};
#endif // DOXYGEN_SKIP



/**
 * Base class for converting an sc_out<sc_bv_base> to sc_out<T>.
 *
 * @Note This class is not used directly, instead use
 *       xtsc_sc_out_sc_bv_base_adapter.
 */
template <int W, typename T>
class xtsc_sc_out_sc_bv_base_adapter_base : public sc_core::sc_module, public sc_core::sc_signal_inout_if<sc_dt::sc_bv_base> {
public:

  /// Bind the sc_out<sc_bv_base> to here
  sc_core::sc_export<sc_core::sc_signal_inout_if<sc_dt::sc_bv_base> >   m_sc_export;

  /// Bind this to an sc_signal<T> or to a higher-level (outer) sc_out<T>
  sc_core::sc_out<T>                                                    m_sc_out;

  xtsc_sc_out_sc_bv_base_adapter_base(sc_core::sc_module_name module_name) :
    sc_module   (module_name),
    m_sc_export ("m_sc_export"),
    m_sc_out    ("m_sc_out"),
    m_value     (W)
  {
    m_value = 0;
    m_sc_export(*this);
  }

  // get the default event
  virtual const sc_core::sc_event& default_event() const {
    return m_sc_out.default_event();
  }

  // get the value changed event
  virtual const sc_core::sc_event& value_changed_event() const {
    return m_sc_out.value_changed_event();
  }

  // was there an event?
  virtual bool event() const {
    return m_sc_out.event();
  }

  // get a reference to the current value (for tracing)
  virtual const sc_dt::sc_bv_base& get_data_ref() const {
    return read();
  }

  // read the current value
  virtual const sc_dt::sc_bv_base& read() const  = 0;

  // write the new value
  virtual void write(const sc_dt::sc_bv_base& value)  = 0;


protected:

  mutable sc_dt::sc_bv_base     m_value;

};



#ifndef DOXYGEN_SKIP
// Derived template for converting vector to/from integral types
template <int W, typename T, bool integral = true>
class xtsc_sc_out_sc_bv_base_adapter_derived : public xtsc_sc_out_sc_bv_base_adapter_base<W, T> {
public:
  xtsc_sc_out_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_out_sc_bv_base_adapter_base<W, T>
                                                                                   (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_out.read(); return this->m_value; }
  virtual void write(const sc_dt::sc_bv_base& value) { this->m_sc_out.write((T) value.to_uint64()); }
};



// Specialization for converting vector to/from non-integral types:  T must have a conversion to/from sc_bv_base
template <int W, typename T>
class xtsc_sc_out_sc_bv_base_adapter_derived<W, T, false> : public xtsc_sc_out_sc_bv_base_adapter_base<W, T> {
public:
  xtsc_sc_out_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_out_sc_bv_base_adapter_base<W, T>
                                                                                   (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_out.read(); return this->m_value; }
  virtual void write(const sc_dt::sc_bv_base& value) { this->m_sc_out.write(value); }
};



// Specialization for converting scalar to/from integral types
template <typename T>
class xtsc_sc_out_sc_bv_base_adapter_derived<1, T, true> : public xtsc_sc_out_sc_bv_base_adapter_base<1, T> {
public:
  xtsc_sc_out_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_out_sc_bv_base_adapter_base<1, T>
                                                                                   (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_out.read(); return this->m_value; }
  virtual void write(const sc_dt::sc_bv_base& value) { this->m_sc_out.write((T) value.to_uint()); }
};



// Specialization for converting scalar to/from non-integral types:  T must have bit_select ([0]) which has member function to_bool().
template <typename T>
class xtsc_sc_out_sc_bv_base_adapter_derived<1, T, false> : public xtsc_sc_out_sc_bv_base_adapter_base<1, T> {
public:
  xtsc_sc_out_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_out_sc_bv_base_adapter_base<1, T>
                                                                                   (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_out.read()[0].to_bool(); return this->m_value; }
  virtual void write(const sc_dt::sc_bv_base& value) { T v(value[0].to_bool()); this->m_sc_out.write(v); }
};



// Explicit specialization for converting scalar to/from non-integral sc_logic.
template <>
class xtsc_sc_out_sc_bv_base_adapter_derived<1, sc_dt::sc_logic, false> :
      public xtsc_sc_out_sc_bv_base_adapter_base<1, sc_dt::sc_logic>
{
public:
  xtsc_sc_out_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_out_sc_bv_base_adapter_base<1, sc_dt::sc_logic>
                                                                                   (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_out.read().to_bool(); return this->m_value; }
  virtual void write(const sc_dt::sc_bv_base& value) { sc_dt::sc_logic v(value[0].to_bool()); this->m_sc_out.write(v); }
};

#endif // DOXYGEN_SKIP



/**
 * User interface class for converting an sc_out<sc_bv_base> to sc_out<T>.
 *
 * An instance of this class can be used to convert an sc_out<sc_bv_base> to an
 * sc_out<T> where T is some common integral or SystemC type.  For example, the
 * xtsc_core class uses sc_out<sc_bv_base> for pin-level TIE output ports.  If you need
 * output ports of a different type then sc_bv_base, then this adapter template can
 * be used to create an adapter of the appropriate type.  A typical use-case for this
 * adapter is when cosimulating XTSC with Verilog using a commercial simulator.
 *
 * Some possible types for T are:
 * \verbatim
      bool
      sc_logic
      sc_lv<W>
      sc_bv<W>
      sc_uint<W>
      sc_biguint<W>
      unsigned long long
      unsigned long
      unsigned int
      unsigned short
      unsigned char
   \endverbatim
 *
 * As an example, assume you have two xtsc_core objects.  The first is called core0 and
 * it has a 50-bit TIE export state called "TIE_status" which has been enabled for
 * pin-level access.  The second is called core1 and it has a 50-bit TIE import wire
 * called "TIE_control" which has been enabled for pin-level access.  The following code
 * snippet can be use to connect these two TIE ports together using two adapters,
 * called status and control, and an sc_signal<sc_uint<50>>, called core0_to_core1:
 *
 * @Note  These core interfaces can (and typically should) be connected directly without
 * the use of adapters and an sc_signal.  This example is contrived just to
 * illustrate constructing and connecting these adapters.  Please consult the
 * xtsc-run documentation and the cosim sub-directories in the XTSC examples directory
 * for realistic uses of these adapters to cosimulate XTSC with Verilog.
 *
 * \verbatim
    xtsc_sc_out_sc_bv_base_adapter<50, sc_uint<50> > status("status");
    xtsc_sc_in_sc_bv_base_adapter<50, sc_uint<50> > control("control");
    sc_signal<sc_uint<50> >  core0_to_core1;

    core0.get_output_pin("TIE_status")(status.m_sc_export);
    status.m_sc_out(core0_to_core1);

    core1.get_input_pin("TIE_control")(control.m_sc_export);
    control.m_sc_in(core0_to_core1);
   \endverbatim
 * @see xtsc_sc_out_sc_bv_base_adapter_base::m_sc_export
 * @see xtsc_sc_out_sc_bv_base_adapter_base::m_sc_out
 * @see xtsc_sc_in_sc_bv_base_adapter
 * @see xtsc_core::get_output_pin()
 */
template <int W, typename T>
class xtsc_sc_out_sc_bv_base_adapter : public xtsc_sc_out_sc_bv_base_adapter_derived<W, T, is_integral<T>::value > {
public:

  xtsc_sc_out_sc_bv_base_adapter(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_bv_base_adapter_derived<W, T, is_integral<T>::value> (module_name)
  {
  }


  virtual const char* kind() const { return "xtsc_sc_out_sc_bv_base_adapter"; }

};






/**
 * Base class for converting an sc_in<T> to an sc_in<sc_bv_base>.
 *
 * @Note This class is not used directly, instead use
 *       xtsc_sc_in_sc_bv_base_adapter.
 */
template <int W, typename T>
class xtsc_sc_in_sc_bv_base_adapter_base : public sc_core::sc_module, public sc_core::sc_signal_in_if<sc_dt::sc_bv_base> {
public:

  /// Bind the sc_in<sc_bv_base> to here
  sc_core::sc_export<sc_core::sc_signal_in_if<sc_dt::sc_bv_base> >   m_sc_export;

  /// Bind this to an sc_signal<T> or to a higher-level (outer) sc_in<T>
  sc_core::sc_in<T>                                                  m_sc_in;

  xtsc_sc_in_sc_bv_base_adapter_base(sc_core::sc_module_name module_name) :
    sc_module   (module_name),
    m_sc_export ("m_sc_export"),
    m_sc_in     ("m_sc_in"),
    m_value     (W)
  {
    m_value = 0;
    m_sc_export(*this);
  }

  // get the default event
  virtual const sc_core::sc_event& default_event() const {
    return m_sc_in.default_event();
  }

  // get the value changed event
  virtual const sc_core::sc_event& value_changed_event() const {
    return m_sc_in.value_changed_event();
  }

  // was there an event?
  virtual bool event() const {
    return m_sc_in.event();
  }

  // get a reference to the current value (for tracing)
  virtual const sc_dt::sc_bv_base& get_data_ref() const {
    return read();
  }

  // read the current value
  virtual const sc_dt::sc_bv_base& read() const  = 0;


protected:

  mutable sc_dt::sc_bv_base    m_value;

};



#ifndef DOXYGEN_SKIP
// Derived template for converting vector from integral types
template <int W, typename T, bool integral = true>
class xtsc_sc_in_sc_bv_base_adapter_derived : public xtsc_sc_in_sc_bv_base_adapter_base<W, T> {
public:
  xtsc_sc_in_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_in_sc_bv_base_adapter_base<W, T>
                                                                                  (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_in.read(); return this->m_value; }
};



// Specialization for converting vector from non-integral types:  T must have a conversion to sc_bv_base.
template <int W, typename T>
class xtsc_sc_in_sc_bv_base_adapter_derived<W, T, false> : public xtsc_sc_in_sc_bv_base_adapter_base<W, T> {
public:
  xtsc_sc_in_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_in_sc_bv_base_adapter_base<W, T>
                                                                                  (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_in.read(); return this->m_value; }
};



// Specialization for converting scalar from integral types
template <typename T>
class xtsc_sc_in_sc_bv_base_adapter_derived<1, T, true> : public xtsc_sc_in_sc_bv_base_adapter_base<1, T> {
public:
  xtsc_sc_in_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_in_sc_bv_base_adapter_base<1, T>
                                                                                  (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_in.read(); return this->m_value; }
};



// Specialization for converting scalar to/from non-integral types:  T must have bit_select ([0]) which has member function to_bool().
template <typename T>
class xtsc_sc_in_sc_bv_base_adapter_derived<1, T, false> : public xtsc_sc_in_sc_bv_base_adapter_base<1, T> {
public:
  xtsc_sc_in_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_in_sc_bv_base_adapter_base<1, T>
                                                                                  (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_in.read()[0].to_bool(); return this->m_value; }
};



// Explicit specialization for converting scalar to/from non-integral sc_logic.
template <>
class xtsc_sc_in_sc_bv_base_adapter_derived<1, sc_dt::sc_logic, false> :
      public xtsc_sc_in_sc_bv_base_adapter_base<1, sc_dt::sc_logic>
{
public:
  xtsc_sc_in_sc_bv_base_adapter_derived(sc_core::sc_module_name module_name) : xtsc_sc_in_sc_bv_base_adapter_base<1, sc_dt::sc_logic>
                                                                                  (module_name) { }
  virtual const sc_dt::sc_bv_base& read() const { this->m_value = this->m_sc_in.read().to_bool(); return this->m_value; }
};

#endif // DOXYGEN_SKIP






/**
 * User interface class for converting an sc_in<T> to an sc_in<sc_bv_base>.
 *
 * An instance of this class can be used to convert an sc_in<T> to an sc_in<sc_bv_base>
 * where T is some common integral or SystemC type.  For example, the xtsc_core class
 * uses sc_in<sc_bv_base> for pin-level TIE input ports.  If you need input ports of a
 * different type then sc_bv_base, then this adapter template can be used to create
 * an adapter of the appropriate type.  A typical use-case for this adapter is
 * when cosimulating XTSC with Verilog using a commercial simulator.
 *
 * For more information about when this adapter might be used and how to use it,
 * see xtsc_sc_out_sc_bv_base_adapter.
 *
 * @see xtsc_sc_out_sc_bv_base_adapter
 * @see xtsc_sc_in_sc_bv_base_adapter_base::m_sc_export
 * @see xtsc_sc_in_sc_bv_base_adapter_base::m_sc_in
 * @see xtsc_core::get_input_pin()
 */
template <int W, typename T>
class xtsc_sc_in_sc_bv_base_adapter : public xtsc_sc_in_sc_bv_base_adapter_derived<W, T, is_integral<T>::value > {
public:

  xtsc_sc_in_sc_bv_base_adapter(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_bv_base_adapter_derived<W, T, is_integral<T>::value> (module_name)
  {
  }

  virtual const char* kind() const { return "xtsc_sc_in_sc_bv_base_adapter"; }


};



/**
 * Base class for converting an sc_out<sc_uint_base> to sc_out<T>.
 *
 * @Note This class is not used directly, instead use
 *       xtsc_sc_out_sc_uint_base_adapter.
 */
template <int W, typename T>
class xtsc_sc_out_sc_uint_base_adapter_base : public sc_core::sc_module, public sc_core::sc_signal_inout_if<sc_dt::sc_uint_base> {
public:

  /// Bind the sc_out<sc_uint_base> to here
  sc_core::sc_export<sc_core::sc_signal_inout_if<sc_dt::sc_uint_base> > m_sc_export;

  /// Bind this to sc_signal<T> or an outer sc_out<T>
  sc_core::sc_out<T>                                                    m_sc_out;

  xtsc_sc_out_sc_uint_base_adapter_base(sc_core::sc_module_name module_name) :
    sc_module   (module_name),
    m_sc_export ("m_sc_export"),
    m_sc_out    ("m_sc_out"),
    m_value     (W)
  {
    m_value = 0;
    m_sc_export(*this);
  }

  // get the default event
  virtual const sc_core::sc_event& default_event() const {
    return m_sc_out.default_event();
  }

  // get the value changed event
  virtual const sc_core::sc_event& value_changed_event() const {
    return m_sc_out.value_changed_event();
  }

  // was there an event?
  virtual bool event() const {
    return m_sc_out.event();
  }

  // get a reference to the current value (for tracing)
  virtual const sc_dt::sc_uint_base& get_data_ref() const {
    return read();
  }

  // read the current value
  virtual const sc_dt::sc_uint_base& read() const  = 0;

  // write the new value
  virtual void write(const sc_dt::sc_uint_base& value)  = 0;


protected:

  mutable sc_dt::sc_uint_base    m_value;

};



#ifndef DOXYGEN_SKIP
// Derived template for converting vector to/from integral types
template <int W, typename T, bool integral = true>
class xtsc_sc_out_sc_uint_base_adapter_derived : public xtsc_sc_out_sc_uint_base_adapter_base<W, T> {
public:
  xtsc_sc_out_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_uint_base_adapter_base<W, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_out.read(); return this->m_value; }
  virtual void write(const sc_dt::sc_uint_base& value) { this->m_sc_out.write((T) value.to_uint64()); }
};



// Specialization for converting vector to/from non-integral types:  T must have a conversion to/from sc_uint_base
template <int W, typename T>
class xtsc_sc_out_sc_uint_base_adapter_derived<W, T, false> : public xtsc_sc_out_sc_uint_base_adapter_base<W, T> {
public:
  xtsc_sc_out_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_uint_base_adapter_base<W, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_out.read(); return this->m_value; }
  virtual void write(const sc_dt::sc_uint_base& value) { this->m_sc_out.write(value); }
};



// Specialization for converting scalar to/from integral types
template <typename T>
class xtsc_sc_out_sc_uint_base_adapter_derived<1, T, true> : public xtsc_sc_out_sc_uint_base_adapter_base<1, T> {
public:
  xtsc_sc_out_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_uint_base_adapter_base<1, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_out.read(); return this->m_value; }
  virtual void write(const sc_dt::sc_uint_base& value) { this->m_sc_out.write((T) value.to_uint()); }
};



// Specialization for converting scalar to/from non-integral types:  T must have bit_select ([0]) which has member function to_bool().
template <typename T>
class xtsc_sc_out_sc_uint_base_adapter_derived<1, T, false> : public xtsc_sc_out_sc_uint_base_adapter_base<1, T> {
public:
  xtsc_sc_out_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_uint_base_adapter_base<1, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_out.read()[0].to_bool(); return this->m_value; }
  virtual void write(const sc_dt::sc_uint_base& value) { T v(value[0].to_bool()); this->m_sc_out.write(v); }
};



// Explicit specialization for converting scalar to/from non-integral sc_logic.
template <>
class xtsc_sc_out_sc_uint_base_adapter_derived<1, sc_dt::sc_logic, false> :
      public xtsc_sc_out_sc_uint_base_adapter_base<1, sc_dt::sc_logic>
{
public:
  xtsc_sc_out_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_uint_base_adapter_base<1, sc_dt::sc_logic>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_out.read().to_bool(); return this->m_value; }
  virtual void write(const sc_dt::sc_uint_base& value) { sc_dt::sc_logic v(value[0].to_bool()); this->m_sc_out.write(v); }
};

#endif // DOXYGEN_SKIP



/**
 * User interface class for converting an sc_out<sc_uint_base> to sc_out<T>.
 *
 * An instance of this class can be used to convert an sc_out<sc_uint_base> to an
 * sc_out<T> where T is some common integral or SystemC type.  For example, the
 * xtsc_tlm2pin_memory_transactor class uses sc_out<sc_uint_base> for output port
 * vectors of 32 bits or less (except for a data bus).  If you need output ports of a
 * different type then sc_uint_base, then this adapter template can be used to create
 * an adapter of the appropriate type.  A typical use-case for this adapter is
 * when cosimulating XTSC with Verilog using a commercial simulator.
 *
 * Some possible types for T are:
 * \verbatim
      bool
      sc_logic
      sc_lv<W>
      sc_bv<W>
      sc_uint<W>
      sc_biguint<W>
      unsigned long long
      unsigned long
      unsigned int
      unsigned short
      unsigned char
   \endverbatim
 *
 * Note:  Please consult the xtsc-run documentation and the cosim sub-directories in the
 *        XTSC examples directory for uses of this adapter to cosimulate XTSC with
 *        Verilog.
 *
 * @see xtsc_sc_out_sc_uint_base_adapter_base::m_sc_export
 * @see xtsc_sc_out_sc_uint_base_adapter_base::m_sc_out
 * @see xtsc_sc_in_sc_uint_base_adapter
 */
template <int W, typename T>
class xtsc_sc_out_sc_uint_base_adapter : public xtsc_sc_out_sc_uint_base_adapter_derived<W, T, is_integral<T>::value > {
public:

  xtsc_sc_out_sc_uint_base_adapter(sc_core::sc_module_name module_name) :
    xtsc_sc_out_sc_uint_base_adapter_derived<W, T, is_integral<T>::value> (module_name)
  {
  }


  virtual const char* kind() const { return "xtsc_sc_out_sc_uint_base_adapter"; }

};






/**
 * Base class for converting an sc_in<T> to an sc_in<sc_uint_base>.
 *
 * @Note This class is not used directly, instead use
 *       xtsc_sc_in_sc_uint_base_adapter.
 */
template <int W, typename T>
class xtsc_sc_in_sc_uint_base_adapter_base : public sc_core::sc_module, public sc_core::sc_signal_in_if<sc_dt::sc_uint_base> {
public:

  /// Bind the sc_in<sc_uint_base> to here
  sc_core::sc_export<sc_core::sc_signal_in_if<sc_dt::sc_uint_base> >  m_sc_export;

  /// Bind this to sc_signal<T> or an outer sc_in<T>
  sc_core::sc_in<T>                                                   m_sc_in;

  xtsc_sc_in_sc_uint_base_adapter_base(sc_core::sc_module_name module_name) :
    sc_module   (module_name),
    m_sc_export ("m_sc_export"),
    m_sc_in     ("m_sc_in"),
    m_value     (W)
  {
    m_value = 0;
    m_sc_export(*this);
  }

  // get the default event
  virtual const sc_core::sc_event& default_event() const {
    return m_sc_in.default_event();
  }

  // get the value changed event
  virtual const sc_core::sc_event& value_changed_event() const {
    return m_sc_in.value_changed_event();
  }

  // was there an event?
  virtual bool event() const {
    return m_sc_in.event();
  }

  // get a reference to the current value (for tracing)
  virtual const sc_dt::sc_uint_base& get_data_ref() const {
    return read();
  }

  // read the current value
  virtual const sc_dt::sc_uint_base& read() const  = 0;


protected:

  mutable sc_dt::sc_uint_base    m_value;

};



#ifndef DOXYGEN_SKIP
// Derived template for converting vector from integral types
template <int W, typename T, bool integral = true>
class xtsc_sc_in_sc_uint_base_adapter_derived : public xtsc_sc_in_sc_uint_base_adapter_base<W, T> {
public:
  xtsc_sc_in_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_uint_base_adapter_base<W, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_in.read(); return this->m_value; }
};



// Specialization for converting vector from non-integral types:  T must have a conversion to sc_uint_base.
template <int W, typename T>
class xtsc_sc_in_sc_uint_base_adapter_derived<W, T, false> : public xtsc_sc_in_sc_uint_base_adapter_base<W, T> {
public:
  xtsc_sc_in_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_uint_base_adapter_base<W, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_in.read(); return this->m_value; }
};



// Specialization for converting scalar from integral types
template <typename T>
class xtsc_sc_in_sc_uint_base_adapter_derived<1, T, true> : public xtsc_sc_in_sc_uint_base_adapter_base<1, T> {
public:
  xtsc_sc_in_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_uint_base_adapter_base<1, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_in.read(); return this->m_value; }
};



// Specialization for converting scalar to/from non-integral types:  T must have bit_select ([0]) which has member function to_bool().
template <typename T>
class xtsc_sc_in_sc_uint_base_adapter_derived<1, T, false> : public xtsc_sc_in_sc_uint_base_adapter_base<1, T> {
public:
  xtsc_sc_in_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_uint_base_adapter_base<1, T>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_in.read()[0].to_bool(); return this->m_value; }
};



// Explicit specialization for converting scalar to/from non-integral sc_logic.
template <>
class xtsc_sc_in_sc_uint_base_adapter_derived<1, sc_dt::sc_logic, false> :
      public xtsc_sc_in_sc_uint_base_adapter_base<1, sc_dt::sc_logic>
{
public:
  xtsc_sc_in_sc_uint_base_adapter_derived(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_uint_base_adapter_base<1, sc_dt::sc_logic>(module_name) { }
  virtual const sc_dt::sc_uint_base& read() const { this->m_value = this->m_sc_in.read().to_bool(); return this->m_value; }
};

#endif // DOXYGEN_SKIP






/**
 * User interface class for converting an sc_in<T> to an sc_in<sc_uint_base>.
 *
 * An instance of this class can be used to convert an sc_in<T> to an
 * sc_in<sc_uint_base> where T is some common integral or SystemC type.  For example,
 * the xtsc_tlm2pin_memory_transactor class uses sc_in<sc_uint_base> for input port
 * vectors of 32 bits or less (except for a data bus).  If you need input ports of a
 * different type then sc_uint_base, then this adapter template can be used to create
 * an adapter of the appropriate type.  A typical use-case for this adapter is
 * when cosimulating XTSC with Verilog using a commercial simulator.
 *
 * For more information about when this adapter might be used and how to use it,
 * see xtsc_sc_out_sc_uint_base_adapter.
 *
 * @see xtsc_sc_out_sc_uint_base_adapter
 * @see xtsc_sc_in_sc_uint_base_adapter_base::m_sc_export
 * @see xtsc_sc_in_sc_uint_base_adapter_base::m_sc_in
 */
template <int W, typename T>
class xtsc_sc_in_sc_uint_base_adapter : public xtsc_sc_in_sc_uint_base_adapter_derived<W, T, is_integral<T>::value > {
public:

  xtsc_sc_in_sc_uint_base_adapter(sc_core::sc_module_name module_name) :
    xtsc_sc_in_sc_uint_base_adapter_derived<W, T, is_integral<T>::value> (module_name)
  {
  }

  virtual const char* kind() const { return "xtsc_sc_in_sc_uint_base_adapter"; }


};



/**
 * Base class for converting an sc_out<xtsc_wire_write_if> to sc_out<T>.
 *
 * @Note This class is not used directly, instead use
 *       xtsc_tlm2pin_wire_transactor.
 */
template <int W, typename T>
class xtsc_tlm2pin_wire_transactor_base : public sc_core::sc_module, public xtsc_wire_write_if {
public:

  /// Bind the sc_port<xtsc_wire_write_if> to here
  sc_core::sc_export<xtsc_wire_write_if>        m_sc_export;

  /// Bind this to an sc_signal<T> or to a higher-level (outer) sc_out<T>
  sc_core::sc_out<T>                            m_sc_out;

  xtsc_tlm2pin_wire_transactor_base(sc_core::sc_module_name module_name) :
    sc_module   (module_name),
    m_sc_out    ("m_sc_out"),
    m_bit_width (W)
  {
    m_sc_export(*this);
  }

  // get the default event
  virtual const sc_core::sc_event& default_event() const {
    return m_sc_out.default_event();
  }

  // get the value changed event
  virtual const sc_core::sc_event& value_changed_event() const {
    return m_sc_out.value_changed_event();
  }

  // was there an event?
  virtual bool event() const {
    return m_sc_out.event();
  }

  virtual void nb_write(const sc_dt::sc_unsigned& value) = 0;

  virtual u32 nb_get_bit_width() { return m_bit_width; }

private:
  u32                           m_bit_width;
};



#ifndef DOXYGEN_SKIP
// Derived template for converting to integral types of 32 bits or less
template <int W, typename T, bool integral = true, bool integral64 = false>
class xtsc_tlm2pin_wire_transactor_derived : public xtsc_tlm2pin_wire_transactor_base<W, T> {
public:
  xtsc_tlm2pin_wire_transactor_derived(sc_core::sc_module_name module_name) : xtsc_tlm2pin_wire_transactor_base<W, T> (module_name) { }
  virtual void nb_write(const sc_dt::sc_unsigned& value) { this->m_sc_out.write((T) value.to_uint()); }
};



// Specialization for converting to integral64 types
template <int W, typename T>
class xtsc_tlm2pin_wire_transactor_derived<W, T, true, true>: public xtsc_tlm2pin_wire_transactor_base<W, T> {
public:
  xtsc_tlm2pin_wire_transactor_derived(sc_core::sc_module_name module_name) : xtsc_tlm2pin_wire_transactor_base<W, T> (module_name) { }
  virtual void nb_write(const sc_dt::sc_unsigned& value) { this->m_sc_out.write((T) value.to_uint64()); }
};



// Specialization for converting to non-integral types:  T must have a conversion from sc_unsigned
template <int W, typename T>
class xtsc_tlm2pin_wire_transactor_derived<W, T, false> : public xtsc_tlm2pin_wire_transactor_base<W, T> {
public:
  xtsc_tlm2pin_wire_transactor_derived(sc_core::sc_module_name module_name) : xtsc_tlm2pin_wire_transactor_base<W, T> (module_name) { }
  virtual void nb_write(const sc_dt::sc_unsigned& value) { this->m_sc_out.write(value); }
};
#endif // DOXYGEN_SKIP







/**
 * User interface class for connecting an sc_port<xtsc_wire_write_if> to an sc_out<T> or
 * an sc_signal<T>.
 *
 * An instance of this class can be used to connect an sc_port<xtsc_wire_write_if> to an
 * sc_out<T> or an sc_signal<T> where T is some common integral or SystemC type.  For
 * example, the xtsc_core class uses sc_port<xtsc_wire_write_if> for TLM-level TIE
 * export states and system-level output ports.  If you need output ports of a different
 * type then xtsc_wire_write_if, then this transactor template can be used to create an
 * transactor of the appropriate type.  A typical use-case for this transactor is when
 * cosimulating XTSC with Verilog using a commercial simulator.
 *
 * @Note see xtsc_sc_out_sc_bv_base_adapter for a list of some possible types for T.
 *
 * As an example, assume you have two xtsc_core objects.  The first is called core0 and
 * it has a 1-bit TIE export state called "onebit".  The second is called core1 and it
 * has a 1-bit system-level input called "BReset".  The following code snippet can be
 * use to connect these two ports together using two transactors, called onebit and
 * BReset, and an sc_signal<bool>, called onebit_to_BReset:
 *
 * Note:  These core interfaces can (and typically should) be connected directly without
 * the use of transactors and an sc_signal.  This example is contrived just to
 * illustrate constructing and connecting these transactors.  Please consult the
 * xtsc-run documentation and the cosim sub-directories in the XTSC examples directory
 * for realistic uses of these transactors to cosimulate XTSC with Verilog.
 *
 * \verbatim
    xtsc_tlm2pin_wire_transactor<1, bool>  onebit("onebit");
    xtsc_pin2tlm_wire_transactor<1, bool>  BReset("BReset");
    sc_signal<bool>                        onebit_to_BReset;

    core0.get_export_state("onebit")(onebit.m_sc_export);
    onebit.m_sc_out(onebit_to_BReset);

    BReset.m_sc_port(core1.get_input_wire("BReset"));
    BReset.m_sc_in(onebit_to_BReset);
   \endverbatim
 *
 *
 *
 * @see xtsc_tlm2pin_wire_transactor_base::m_sc_export
 * @see xtsc_tlm2pin_wire_transactor_base::m_sc_out
 * @see xtsc_core::get_export_state()
 * @see xtsc_core::get_output_wire()
 * @see xtsc_core::get_input_wire()
 */
template <int W, typename T>
class xtsc_tlm2pin_wire_transactor :
  public xtsc_tlm2pin_wire_transactor_derived<W, T, is_integral<T>::value, is_integral64<T>::value> {
public:

  xtsc_tlm2pin_wire_transactor(sc_core::sc_module_name module_name) :
    xtsc_tlm2pin_wire_transactor_derived<W, T, is_integral<T>::value, is_integral64<T>::value> (module_name)
  {
  }


  virtual const char* kind() const { return "xtsc_tlm2pin_wire_transactor"; }

};



/**
 * User interface class for connecting an sc_in<T> or an sc_signal<T> to an
 * sc_export<xtsc_wire_write_if>.
 *
 * An instance of this class can be used to connect an sc_in<T> or an sc_signal<T> to an
 * sc_export<xtsc_wire_write_if> where T is some common integral or SystemC type.  For
 * example, the xtsc_core class uses sc_export<xtsc_wire_write_if> for system-level
 * input ports.  If you need input ports of a different type then xtsc_wire_write_if,
 * then this transactor template can be used to create an transactor of the appropriate
 * type.  A typical use-case for this transactor is when cosimulating XTSC with Verilog
 * using a commercial simulator.
 *
 * For more information about when this transactor might be used and how to use it,
 * see xtsc_tlm2pin_wire_transactor.
 *
 *
 * @see xtsc_tlm2pin_wire_transactor
 * @see xtsc_core::get_input_wire()
 */
template <int W, typename T>
class xtsc_pin2tlm_wire_transactor : public sc_core::sc_module {
public:

  /// Bind this to the sc_export<xtsc_wire_write_if>
  sc_core::sc_port<xtsc_wire_write_if>          m_sc_port;

  /// Bind this to the sc_signal<T> or to a higher-level (outer) sc_in<T>
  sc_core::sc_in<T>                             m_sc_in;

  SC_HAS_PROCESS(xtsc_pin2tlm_wire_transactor);

  xtsc_pin2tlm_wire_transactor(sc_core::sc_module_name module_name) :
    sc_module   (module_name),
    m_sc_port   ("m_sc_port"),
    m_sc_in     ("m_sc_in"),
    m_value     (W)
  {
    SC_METHOD(relay_method);
    sensitive << m_sc_in;
    dont_initialize();
  }

  virtual const char* kind() const { return "xtsc_pin2tlm_wire_transactor"; }

  // get the default event
  virtual const sc_core::sc_event& default_event() const {
    return m_sc_in.default_event();
  }

  // get the value changed event
  virtual const sc_core::sc_event& value_changed_event() const {
    return m_sc_in.value_changed_event();
  }

  // was there an event?
  virtual bool event() const {
    return m_sc_in.event();
  }

  void relay_method() {
    this->m_value = this->m_sc_in.read();
    m_sc_port->nb_write(m_value);
  }

private:
  sc_dt::sc_unsigned            m_value;
};






/**
 * Utility class for handling a script-style file.
 *
 * This utility class provides a wrapper for handling a script-style file.
 *
 * The get_words() method can be used to get the next non-blank, non-comment line from
 * the file tokenized into words (as a vector of strings).
 *
 * The getline() method can be used to get the next non-blank, non-comment line from the
 * file as a string.
 *
 * The handling of an xtsc_script_file includes a pseudo-preprocessor:
 *
 * - C and C++ style comments are never returned by the get_words() and getline()
 *   methods.
 *
 * - A "#define <Identifier>" line defines a simple macro (a simple macro is a macro
 *   that is defined but that doesn't contain a value).  <Identifier> must be a legal
 *   C/C++ identifier.  The line itself is never returned by the get_words() and
 *   getline() methods.
 *
 * - A "#define <Identifier>=<String>" line defines an alias macro.  <String> can be any
 *   string (including the empty string).  Before a line from the script file is
 *   processed in any other way (other then being skipped because it is a comment or is
 *   between a false "#if/#endif" pair), it is first scanned for occurrences of any
 *   defined simple or alias macro preceeded by a dollar sign and opening parenthesis
 *   and followed by a closing parenthesis.  Each occurrence is replaced verbatim by the
 *   corresponding <String> if it is an alias macro or by an empty string if it is a
 *   simple macro.  For example, a script file consisting only of the following two
 *   lines will result in the single line "Hello World" being returned by the getline()
 *   method:
 *   \verbatim
        #define entity=World
        Hello $(entity)
     \endverbatim
 *
 *   Note: On preprocessor lines (lines beginning with #), a sub-string in the format
 *         "$(<Identifier>)" where <Identifier> is not a defined macro will be removed
 *         (i.e. treated like <Identifier> is a macro aliased to the empty string).
 *         On non-preprocessor lines, no substitution takes place if the <Identifier> is
 *         not a defined macro.  If substitution is desired, then place code similiar to
 *         the following earlier in the script file:
 *   \verbatim
              #ifndef MY_MACRO
              #define MY_MACRO
              #endif
     \endverbatim
 *
 * - A "#define <Identifier> <Text>" line also defines an alias macro.  <Text> can be
 *   any non-blank text that does not contain an equal sign (=).  Leading and trailing
 *   spaces are stripped from <Text>, but embedded spaces are left in place.  For
 *   example, a script file consisting only of the following two lines will result in
 *   the single line "Hello World" being returned by the getline() method:
 *   \verbatim
        #define message     Hello World
        $(message)
     \endverbatim
 *
 *   Note:  The alias macro XTSC_SCRIPT_FILE_PATH is always defined and is equal to the
 *          path to the current script file being processed.  It can be used to specify
 *          locations in the file system relative to the xtsc_script_file.
 *
 * - A "#environment" line causes all the environment variables that are valid C/C++
 *   identifiers to be read in and treated as macros.  All environment variables that
 *   contain a value are defined as alias macros.  All environment variables that don't
 *   contain a value are defined as simple macros.  The "#environment" line itself is
 *   never returned by the get_words() and getline() methods.
 *
 * - A "#undef <Identifier>" line removes <Identifier> from the list of defined macros
 *   if such a macro exists (no error is generated if the macro is not currently
 *   defined).  The "#undef" line itself is never returned by the get_words() and
 *   getline() methods.
 *
 * - All lines between and including a line starting with "#if 0" and a line starting with
 *   "#endif" are ignored and are never returned by the get_words() and getline() methods.
 *
 * - Lines starting with "#if 1" and "#endif" are never returned by the get_words() and
 *   getline() methods; however, non-comment, non-blank, non-pseudo-preprocessor lines
 *   between them are.
 *
 * - Lines starting with "#ifdef <Identifier>" are handled according to one of the
 *   previous two rules depending upon whether or not <Identifier> is a defined macro
 *   (either a simple macro or an alias macro).  "#ifndef <Identifier>" is also
 *   supported.
 *
 * - A "#ifeq (<TextA>,<TextB>)" line is handled like a "#if 1" line if <TextA>
 *   is equal to <TextB>, otherwise, the line is handled like a "#if 0" line.  Although
 *   it is not a requirement, it is common for at least one of <TextA> and <TextB> to be
 *   a macro.  For example, "#ifeq ($(GCC_VERSION),3.4.1)".  The #ifneq directive is
 *   also supported.
 *
 * - The "#else" construct is supported between a "#if/#ifdef/#ifndef/#ifeq/#ifneq" line
 *   and its matching "#endif" line.  The "#else" line itself is never returned by the
 *   get_words() and getline() methods.
 *
 * - Nesting of "#if/#ifdef/#ifndef/#ifeq/#ifneq" and "#endif" pairs is NOT supported.
 *
 * - A "#error" line causes the line to be thrown as an exception.
 *
 * - A "#warn" line causes the line to be logged by logger xtsc_script_file at
 *   WARN_LOG_LEVEL if logging has been configured, otherwise the line is sent to
 *   STDERR.
 *
 * - A "#include <FileName>" line causes processing of the current file to be
 *   temporarily suspended while file FileName is opened and processed.  The "#include"
 *   line itself is never returned by the get_words() and getline() methods.  FileName
 *   must be enclosed in quotation marks or angle brackets.  The XTSC_SCRIPT_FILE_PATH
 *   macro can be used as part of FileName to specify a path relative to the current
 *   script file being processed.  For example:
 *     #include "$(XTSC_SCRIPT_FILE_PATH)/../common.inc"
 *
 * - In the formats described above the pound sign ("#") must always be in the first
 *   column.
 *
 * - Other then the formats defined above, script files should not contain any lines
 *   that have a pound sign ("#") in the first column.
 *
 * Here is an example script to illustrate most of the supported preprocessor constructs:
 * \verbatim
    // You can include other files
    #include "common.inc"

    // You can define macros
    #define OPTION FAST

    // You can test macros
    #ifndef OPTION
    // You can cause a fatal error
    #error macro OPTION must be defined
    #endif

    // You can compare macros with text strings
    #ifeq ($(OPTION),SLOW)
    #define DELAY=1000
    #else
    #define DELAY=1
    #endif

    // You can print warnings
    #warn DELAY is $(DELAY)

    // Other then what might be in "common.inc", these 3 lines are
    // all that the module reading this script file will see
    $(DELAY) 123
    $(DELAY) 456
    $(DELAY) 789

    // You can disable/enable blocks of "code"
    #if 0
    #warn Not doing this
    #else
    #warn Doing this instead
    #endif

    // Pull in environment variables (for example, you could
    // have all the scripts used in a system simulaton conditioned
    // on a single environment variable)
    #environment
    #warn CWD is $(PWD)

    // This predefined macro is were the script file is located
    // even when it is not in the current directory
    #warn This script file is located in $(XTSC_SCRIPT_FILE_PATH)

   \endverbatim
 */
class XTSC_API xtsc_script_file {
public:

  /**
   * Constructor for an xtsc_script_file.
   *
   * @param     file_name       The name of the script file.  If this file does not
   *                            exist an exception is thrown.
   *
   * @param     m_parm_name     Optional parameter name that file_name came from.  This
   *                            is used for exception messages.
   *
   * @param     name            Optional name of the component using the script file
   *                            (for example, "ColorLUT").  This is used for exception
   *                            messages.
   *
   * @param     type            Optional type of component using the script file (for
   *                            example, "xtsc_lookup").  This is used for exception
   *                            messages.
   *
   * @param     wraparound      If false (the default), the file is only processed one
   *                            time.  If true, the file pointer will be reset to the
   *                            beginning of the file each time the end of file is
   *                            reached.  If wraparound is true, the file must contain
   *                            at least one non-blank, non-comment line or an exception
   *                            will be thrown.
   */
  xtsc_script_file(const char  *file_name,
                   const char  *m_parm_name     = "",
                   const char  *name            = "",
                   const char  *type            = "",
                   bool         wraparound      = false);

  /// Destructor.
  ~xtsc_script_file();

  /// Reset script file to the beginning and start over
  void reset();

  /**
   * Get the next line from the file.
   *
   * This method returns the next non-blank, non-comment line from the file.
   *
   * @param     line            A reference to the string object in which to return the
   *                            next non-blank, non-comment line of the file.
   *
   * @param     p_file          An optional pointer to a string in which to return the 
   *                            name of the file from which this line came from (or NULL
   *                            if the name of the file is not desired).
   *
   * @returns the line number.  Returns 0 if wraparound is false and the end of file is
   *          reached.
   */
  u32 getline(std::string& line, std::string *p_file = NULL);

  /**
   * Get the next line from the file broken into words.
   *
   * This method returns the next non-blank, non-comment line from the file broken into
   * words.  Word tokenization is based on whitespace (tab and space characters).
   *
   * @param     words           A reference to a vector<string> object in which to
   *                            return the words.
   *
   * @param     downshift       If true, all the uppercase characters in each string in
   *                            the words vector will be shifted to lowercase.
   *
   * @param     p_file          An optional pointer to a string in which to return the 
   *                            name of the file from which this line came from (or NULL
   *                            if the name of the file is not desired).
   *
   * @returns the line number.  Returns 0 if wraparound is false and the end of file is
   *          reached.
   */
  u32 get_words(std::vector<std::string>& words, bool downshift = false, std::string *p_file = NULL);

  /**
   * Get the next line from the file broken into words.
   *
   * This method returns the next non-blank, non-comment line from the file broken into
   * words.  Word tokenization is based on whitespace (tab and space characters).  In
   * addition, the line read from the file is returned.
   *
   * @param     words           A reference to a vector<string> object in which to
   *                            return the words.
   *
   * @param     line            A reference to the string object in which to return the
   *                            line read from the file and used to form the words
   *                            vector.  Any C-style comment characters will not appear
   *                            in line, but the other characters will be in the same
   *                            case as they came from the file (i.e. they will not be
   *                            downshifted).
   *
   * @param     downshift       If true, all the uppercase characters in each string in
   *                            the words vector will be shifted to lowercase.
   *
   * @param     p_file          An optional pointer to a string in which to return the 
   *                            name of the file from which this line came from (or NULL
   *                            if the name of the file is not desired).
   *
   * @returns the line number.  Returns 0 if wraparound is false and the end of file is
   *          reached.
   */
  u32 get_words(std::vector<std::string>& words, std::string& line, bool downshift = false, std::string *p_file = NULL);

  /**
   * Dump file name and line number info, including #include backtrace.
   *
   * @param     os              The ostream object to which the backtrace is to be
   *                            dumped.
   *
   * @param     single_line     If true, print backtrace on a signle line, otherwise
   *                            use one line per file.
   */
  void dump_last_line_info(std::ostream& os = std::cout, bool single_line = true);

  // Return common information about current situation for use when throwing an exception
  std::string info_for_exception(bool show_line_number = true, bool new_line = true);


private:

  // Fill in info from m_parm_name, m_name, and m_type
  void info_for_exception(std::ostringstream& oss);

  // Do macro expansion
  void do_macro_expansion(std::string& line);

  // Add name/value to m_alias_macros and add name to m_macros
  void add_alias(const std::string& name, const std::string& value);

  // Add name to m_macros
  void add_macro(const std::string& name);

  // Deletes name from m_macros and m_alias_macros (if they have name as an entry)
  void delete_macro(const std::string& name);

  // Open the file and add it to the stack
  void open(const std::string& file_name);

  // Close all files in stack
  void close_all();

  // Close top file in stack
  void close();

  // Trim leading and trailing whitespace
  void trim_whitespace(std::string& str);

  // Throw exception due to missing identifer in line type line_type
  void throw_missing_identifier(const std::string& line_type);


  typedef std::vector<bool>                     vector_bool;
  typedef std::vector<u32>                      vector_u32;
  typedef std::vector<std::string>              vector_string;
  typedef std::vector<std::ifstream*>           vector_ifstream;
  typedef std::set<std::string>                 set_string;
  typedef std::map<std::string, std::string>    map_string_string;

  u32                   m_active_line_count;    // Count of active lines (non-comment, non-blank) in all files
  std::string           m_script_file_name;     // The name of the script file
  std::string           m_parm_name;            // Optional parameter name of m_file_name; used for exception messages
  std::string           m_name;                 // Optional name of component using this file; used for exception messages
  std::string           m_type;                 // Optional type of component using this file; used for exception messages
  bool                  m_wraparound;           // True means to wrap around to start of file when end of top level file is reached
  bool                  m_comment_skip;         // Skipping lines because of multiline comment
  bool                  m_pound_if_else_skip;   // Skipping lines because of #if/#ifdef/#ifndef/#ifeq/#ifneq/#else
  set_string            m_macros;               // The set of simple and alias macros
  map_string_string     m_alias_macros;         // The map of alias macros and their string value

  u32                   m_depth;                // Index into the vectors that keep track of the #include file stack
  vector_bool           m_in_pound_if;          // The stack of flags: true means we're inside a #if/#else/#endif block or any variant
  vector_bool           m_in_pound_else;        // The stack of flags: true means we've encounter a #else
  vector_u32            m_line_number;          // The stack of file line numbers
  vector_string         m_file_name;            // The stack of file names
  vector_string         m_file_path;            // The stack of file paths
  vector_ifstream       m_file;                 // The stack of file stream object pointers

  std::string           m_whitespace;           // whitespace characters
};



/**
 * This utility function can be used to determine if a string is a valid C/C++ identifier.
 *
 * @param       name    The string to be tested.
 *
 * @return true if name is a valid C/C++ identifier, otherwise return false.
 */
XTSC_API bool xtsc_is_valid_identifier(const std::string& name);



/**
 * This utility function safely copies a c-string (char *).  It throws an exception if
 * memory for the new c-string cannot be allocated.
 *
 * @param       str     The c-string to be copied.  It can be NULL.
 *
 * @return a new copy of the str (or NULL if str is NULL).
 */
XTSC_API char *xtsc_copy_c_str(const char *str);



/**
 * This utility function safely copies an array of c-strings (char **).  It throws an
 * exception if memory for the new c-string array or the contained c-strings cannot be
 * allocated.
 *
 * @param       str_array       The c-string array to be copied.  The last entry in the
 *                              array must be NULL.  str_array itself can be NULL.
 *
 * @return a new copy of the str_array (or NULL if str_array is NULL) with each array
 *         entry itself a new copy .
 */
XTSC_API char **xtsc_copy_c_str_array(const char * const *str_array);



/**
 * This utility function deletes a c-string and then sets the pointer to NULL.
 *
 * @param       str     The c-string to be deleted.  It can be NULL.
 */
XTSC_API void xtsc_delete_c_str(char *&str);



/**
 * This utility function deletes an array of c-strings (char **).
 *
 * @param       str_array       The c-string array to be deleted.  The last entry in the
 *                              array must be NULL.  str_array itself can be NULL.
 */
XTSC_API void xtsc_delete_c_str_array(char **&str_array);



/**
 * Dump a list of all SystemC objects.
 *
 * This utility method dumps a list of all existing SystemC objects (sc_object) to the
 * specified ostream object.
 */
XTSC_API void xtsc_dump_systemc_objects(std::ostream& os = std::cout);



/**
 * Interface for objects which can be reset.
 *
 * @Note   In addition to calling this reset() method on a component, if the componnt
 *         has any SystemC thread processes then it may require that those thread
 *         processes be restarted in order to get a clean reset.  In general, restarting
 *         a SystemC thread processes is a job best handled by the SystemC kernel
 *         itself.  Some commercial SystemC simulators automatically restart all SystemC
 *         processes as part of their simulation reset capability.
 */
class XTSC_API xtsc_resettable {
public:
  /// Reset the object
  virtual void reset(bool hard_reset) = 0;
  virtual ~xtsc_resettable() {};
};


} // namespace xtsc




#endif  // _XTSC_H_

#ifndef _XTSC_QUEUE_CONSUMER_H_
#define _XTSC_QUEUE_CONSUMER_H_

// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <string>
#include <vector>
#include <fstream>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_queue_pop_if.h>




namespace xtsc {
class xtsc_tx_loader;
}



namespace xtsc_component {


/**
 * Constructor parameters for a xtsc_queue_consumer object.
 *
 * This class contains the constructor parameters for a xtsc_queue_consumer object.
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------------
  
   "pin_level"          bool    If true, pin-level connections are used.
                                Default = false (TLM connections are used).

   "vcd_handle"         void*   Pointer to SystemC VCD object (sc_trace_file *) or 0 if
                                tracing is not desired.  This parameter is ignored if
                                "pin_level" is false.
                                Default = 0 (NULL).

   "script_file"        char*   The file to read the test vector commands from.  Each
                                command occupies one line in the file.  Valid command
                                formats are shown below (the first format shows a queue
                                pop transaction command):

                                  <delay> [<timeout>]
                                  <delay> STOP
                                  WAIT  <duration>
                                  SYNC  <time>
                                  NOTE  message
                                  INFO  message
                                
                                1.  Integers can appear in decimal or hexadecimal (using
                                    '0x' prefix) format.
                                2.  <delay> specifies how long to wait after completing
                                    the previous command before attempting to pop a
                                    value from the queue or stopping the simulation.
                                    <delay> can be 0 (to mean 1 delta cycle), or "now"
                                    to mean no delta cycle delay, or a positive integer
                                    or floating point number to mean that many clock
                                    periods (see "clock_period").
                                    Note:  For pin-level connections, a pop command
                                    is considered to be complete when the pop request
                                    is deasserted.  See parameter "deassert_delay".
                                3.  The optional <timeout> specifies how long to
                                    continue attempting to pop a value from the queue.
                                    For a TLM-level connection (e.g. to xtsc_queue),
                                    <timeout> specifies a non-zero integer number of
                                    clock periods and the pop request is repeated each
                                    clock period for <timeout> number of clock periods
                                    or until the pop request is granted.  For a pin-
                                    level connection (e.g. to xtsc_queue_pin),
                                    <timeout> is multiplied by the module's clock
                                    period (see "clock_period") to determine the
                                    maximum amount of time the pop request will be
                                    asserted before being abandoned if it has not yet
                                    been not granted.  For a pin-level connection,
                                    <timeout> can be an integer or a floating point
                                    number.  For either a TLM-level or a pin-level
                                    connection, a negative or missing <timeout> value
                                    means to keep trying the request until it is
                                    granted.
                                4.  The "<delay> STOP" command causes simulation to stop
                                    via a call to the sc_stop() method after the
                                    specified delay.
                                5.  The "WAIT <duration>" command can be used to cause a 
                                    wait of the specified duration.  <duration> can be 0
                                    (to mean 1 delta cycle) or a positive integer or
                                    floating point number to mean that many clock
                                    periods.
                                6.  The "SYNC <time>" command with <time> less than 1.0
                                    can be used to cause a wait to the clock phase
                                    (relative to posedge clock) specified by <time>.
                                    Posedge clock is as specified by "posedge_offset".
                                    For example, "sync 0.5" will cause the minimum wait
                                    necessary to sync to negedge clock.
                                    The "SYNC <time>" command with <time> greater than
                                    or equal to 1.0 can be used to cause a wait until
                                    the specified absolute simulation time.
                                7.  The NOTE and INFO commands can be used to cause
                                    the entire line to be logged at NOTE_LOG_LEVEL
                                    or INFO_LOG_LEVEL, respectively.
                                8.  Words are case insensitive.
                                9.  Comments, extra whitespace, blank lines, and lines
                                    between "#if 0" and "#endif" are ignored.  
                                    See xtsc_script_file for a complete list of
                                    pseudo-preprocessor commands.

   "bit_width"          u32     Width of each queue element in bits.

   "clock_period"       u32     This is the length of this module's clock period
                                expressed in terms of the SystemC time resolution
                                (from sc_get_time_resolution()).  A value of 
                                0xFFFFFFFF means to use the XTSC system clock 
                                period (from xtsc_get_system_clock_period()).
                                Default = 0xFFFFFFFF (i.e. use the system clock 
                                period).

   "posedge_offset"     u32     This specifies the time at which the first posedge of
                                this device's clock conceptually occurs.  It is
                                expressed in units of the SystemC time resolution and
                                the value implied by it must be strictly less than the
                                value implied by the "clock_period" parameter.  A value
                                of 0xFFFFFFFF means to use the same posedge offset as
                                the system clock (from
                                xtsc_get_system_clock_posedge_offset()).
                                Default = 0xFFFFFFFF.

   "sample_phase"       u32     This specifies the phase (i.e. the point) in each clock
                                period at which the empty signal is sampled.  It is
                                expressed in terms of the SystemC time resolution (from
                                sc_get_time_resolution()) and must be strictly less
                                than the clock period as specified by the
                                "clock_period" parameter.  A value of 0 means sampling
                                occurs at posedge clock as specified by "posedge_clock".
                                This parameter is for pin-level connection only.
                                Default = 0.

   "deassert_delay"    u32      During a pop attempt, this specifies how long after the
                                empty signal is sampled and found to be false, that the
                                pop signal should be deasserted.  It is expressed in
                                terms of the SystemC time resolution (from
                                sc_get_time_resolution()) and must be strictly less than
                                the clock period as specified by the "clock_period"
                                parameter.  A value of 0 means the pop signal will be
                                deasserted 1 delta cycle after the empty signal is
                                sampled and found to be false.  This parameter is for
                                pin-level connection only.
                                Default = 0.

   "wraparound"        bool     If false (the default), "script_file" is only processed
                                one time.  If true, the file pointer will be reset to
                                the beginning of the file each time the end of file is
                                reached.
                                Default = false.

    \endverbatim
 *
 * @see xtsc_queue_consumer
 * @see xtsc::xtsc_parms
 * @see xtsc::xtsc_script_file
 */
class XTSC_COMP_API xtsc_queue_consumer_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for an xtsc_queue_consumer_parms object.
   *
   * @param     script_file        The file name to read the xtsc_request test 
   *                                    vectors from.
   *
   * @param     width1                  Width of each queue element in bits.
   *
   *
   */
  xtsc_queue_consumer_parms(const char *script_file = 0, xtsc::u32 width1 = 32) {
    add("pin_level",            false);
    add("vcd_handle",           (void*) NULL); 
    add("script_file",          script_file);
    add("bit_width",            width1);
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("sample_phase",         0);
    add("deassert_delay",       0);
    add("wraparound",           false);
  }


  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_queue_consumer_parms"; }

};





/**
 * A scripted consumer to drain a queue.
 *
 * This XTSC module implements a queue consumer that reads an input file ("script_file")
 * to determine when to attempt to read values from (i.e. pop) a queue module.
 *
 * This module provides a simple means to test reading a queue at the TLM-level (such as
 * xtsc_queue) or at the pin-level (such as xtsc_queue_pin).  To use pin-level
 * connections, you must set the "pin_level" parameter to true.
 *
 * Here is a block diagram of an xtsc_queue_consumer as it is used in the queue consumer
 * example:
 * @image html  xtsc_queue_consumer.jpg
 * @image latex xtsc_queue_consumer.eps "xtsc_queue_consumer Example" width=10cm
 *
 * @see xtsc_queue_consumer_parms
 * @see xtsc::xtsc_queue_pop_if
 */
class XTSC_COMP_API xtsc_queue_consumer : public sc_core::sc_module, virtual public xtsc::xtsc_queue_pop_if, public xtsc::xtsc_resettable {
public:

  sc_core::sc_out<sc_dt::sc_bv_base>            m_pop;          ///<  pin-level pop request to queue
  sc_core::sc_in<sc_dt::sc_bv_base>             m_data;         ///<  pin-level data from queue
  sc_core::sc_in<sc_dt::sc_bv_base>             m_empty;        ///<  pin-level empty signal from queue

  sc_core::sc_port<xtsc::xtsc_queue_pop_if>     m_queue;        ///<  TLM port to queue


  // For SystemC
  SC_HAS_PROCESS(xtsc_queue_consumer);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_queue_consumer"; }


  /**
   * Constructor for an xtsc_queue_consumer.
   *
   * @param     module_name     Name of the xtsc_queue_consumer sc_module.
   *
   * @param     consumer_parms  The remaining parameters for construction.
   *
   * @see xtsc_queue_consumer_parms
   */
  xtsc_queue_consumer(sc_core::sc_module_name module_name, const xtsc_queue_consumer_parms& consumer_parms);


  /// Destructor.
  ~xtsc_queue_consumer(void);


  /**
   * Reset the xtsc_queue_consumer.
   */
  void reset(bool hard_reset = false);


  /**
   * Connect to a upstream xtsc_tx_loader.
   *
   * This method connects the m_queue sc_port of this xtsc_queue_consumer to the
   * m_consumer sc_export of an xtsc_tx_loader.
   *
   * @param     loader          The xtsc_tx_loader to connect to.
   */
  void connect(xtsc::xtsc_tx_loader& loader);


  /// Thread to process commands from m_script_file
  void script_thread(void);


  /// Thread to sample the empty signal to see if the pop succeeded
  void sample_thread(void);


  /// Thread to drive the pop request
  void request_thread(void);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


protected:

  log4xtensa::TextLogger&               m_text;
  bool                                  m_wraparound;                   ///<  Should "script_file" wraparound
  xtsc::xtsc_script_file                m_test_vector_stream;
  std::string                           m_script_file;
  std::string                           m_line;
  xtsc::u32                             m_line_count;
  std::vector<std::string>              m_words;
  xtsc::u64                             m_clock_period_value;           ///<  This device's clock period as u64
  sc_core::sc_time                      m_clock_period;
  sc_core::sc_time                      m_time_resolution;              ///<  The SystemC time resolution
  bool                                  m_has_posedge_offset;           ///<  True if m_posedge_offset is non-zero
  sc_core::sc_time                      m_posedge_offset;               ///<  From "posedge_offset" parameter
  xtsc::u64                             m_posedge_offset_value;         ///<  m_posedge_offset as u64
  sc_core::sc_time                      m_sample_phase;
  sc_core::sc_time                      m_deassert_delay;
  bool                                  m_no_timeout;
  xtsc::u32                             m_width1;                       ///<  Bit width of each element
  bool                                  m_pin_level;
  sc_core::sc_trace_file               *m_p_trace_file;
  sc_dt::sc_unsigned                    m_value;
  sc_dt::sc_bv_base                     m_value_bv;
  sc_dt::sc_unsigned                    m_zero;
  sc_dt::sc_bv_base                     m_zero_bv;
  sc_dt::sc_unsigned                    m_one;
  sc_dt::sc_bv_base                     m_one_bv;
  sc_core::sc_event                     m_next_request;
  sc_core::sc_event                     m_assert;
  sc_core::sc_event                     m_deassert;


  /// Extract a u32 value (named argument_name) from the word at m_words[index]
  xtsc::u32 get_u32(xtsc::u32 index, const std::string& argument_name);

  /// Extract a double value (named argument_name) from the word at m_words[index]
  double get_double(xtsc::u32 index, const std::string& argument_name);

  /// Method to check interface width 
  void end_of_elaboration();

  // These 3 methods are to cap off the TLM port (m_queue) when this consumer is 
  // connected at the pin level to, for example, xtsc_queue_pin.  These
  // methods will never be called
  bool nb_can_pop()                                             { return false; }
  bool nb_pop(sc_dt::sc_unsigned& element, xtsc::u64& ticket)   { return false; }
  xtsc::u32 nb_get_bit_width()                                  { return m_width1; }

  // This 3 signals are to cap off the pin-level ports when this consumer is 
  // connected at the TLM level to, for example, xtsc_queue.
  xtsc::xtsc_signal_sc_bv_base_floating m_pop_floating;
  xtsc::xtsc_signal_sc_bv_base_floating m_data_floating;
  xtsc::xtsc_signal_sc_bv_base_floating m_empty_floating;

};



}  // namespace xtsc_component



#endif  // _XTSC_QUEUE_CONSUMER_H_

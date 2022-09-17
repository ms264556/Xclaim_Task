#ifndef _XTSC_QUEUE_PIN_H_
#define _XTSC_QUEUE_PIN_H_

// Copyright (c) 2005-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <vector>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_parms.h>



namespace xtsc_component {



/**
 * Constructor parameters for a xtsc_queue_pin object.
 *
 * This class contains the constructor parameters for a xtsc_queue_pin object.
 * \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------
  
   "bit_width"          u32     Width of each queue element in bits.

   "depth"              u32     Number of elements the queue can hold.
  
   "push_file"          char*   Name of file to write pushed elements to instead
                                of adding them to the queue fifo.  If the
                                "push_file" parameter is non-null and non-empty,
                                then pushes will cause the passed 
                                element to be written to the file and to NOT be
                                added to the queue fifo.  If the file named by the
                                "push_file" parameter value does not exist, it
                                will be created.  If it does exist, it will be 
                                overwritten.  If both "push_file" and "pop_file" 
                                parameters are null or empty, then pushes
                                will cause the passed element to be added to the
                                queue fifo.

   "timestamp"          bool    If true, then each value written to "push_file"
                                will include the SystemC timestamp as an
                                xtsc_script_file comment.  This parameter is ignored
                                unless "push_file" is non-null and non-empty.
                                Default = true.

   "pop_file"           char*   Name of file to read popped elements from instead
                                of getting them from the queue.  If the "pop_file"
                                parameter is non-null and non-empty, then pops will
                                get their element from the file instead of from the
                                queue fifo.  The file named by the "pop_file"
                                parameter must exist.  If both "pop_file" and
                                "push_file" parameters are null or empty, then pops
                                will get their element from the queue fifo.  Element
                                values in the file can be expressed in decimal or
                                hexadecimal (using leading '0x') format.  Element
                                values must be separated by white-space.
                                See xtsc::xtsc_script_file.

   "wraparound"         bool    Specifies what should happen when the end of file
                                (EOF) is reached on "pop_file".  When EOF is reached
                                and "wraparound" is true, "pop_file" will be reset
                                to the beginning of file and pops will return
                                the first element from the file.  When EOF is
                                reached and "wraparound" is false, pops will fail.
                                Default = false.

   Note:  To cause xtsc_queue_pin to function as a normal queue, set both "push_file"
          and "pop_file" parameter values to null (the default) or empty and 
          bind to both push and pop interfaces.

          To cause xtsc_queue_pin to function as an infinite sink of elements pushed 
          into it, specify a valid file name for "push_file" and bind to the 
          push interfaces.

          To cause xtsc_queue_pin to function as a source of elements popped from
          it, specify a valid and existing file name for "pop_file" and bind to
          the pop interfaces.

          To cause xtsc_queue_pin to function as both a sink and a source, specify
          both file names and bind to both push and pop interfaces.
          
          To cause xtsc_queue_pin to function as a sink but not as a source, specify
          "pop_file" as null or empty and do NOT bind to the push interfaces.
          
          To cause xtsc_queue_pin to function as a source but not as a sink, specify
          "push_file" as null or empty and do NOT bind to the pop interfaces.


   "clock_period"               u32     This is the length of this queue's clock period
                                        expressed in terms of the SystemC time
                                        resolution (from sc_get_time_resolution()).  A
                                        value of 0xFFFFFFFF means to use the XTSC system
                                        clock period from xtsc_get_system_clock_period().
                                        Default = 0xFFFFFFFF (i.e. use the system clock
                                        period).

   "posedge_offset"             u32     This specifies the time at which the first
                                        posedge of this device's clock conceptually
                                        occurs.  It is expressed in units of the SystemC
                                        time resolution and the value implied by it must
                                        be strictly less than the value implied by the
                                        "clock_period" parameter.  A value of 0xFFFFFFFF
                                        means to use the same posedge offset as the
                                        system clock (from
                                        xtsc_get_system_clock_posedge_offset()).
                                        Default = 0xFFFFFFFF.

   "sample_phase"               u32     This specifies the phase (i.e. the point) in
                                        each clock period at which the signals are
                                        sampled.  It is expressed in terms of the
                                        SystemC time resolution (from
                                        sc_get_time_resolution()) and must be strictly
                                        less than the clock period as specified by the
                                        "clock_period" parameter.  A value of 0 means
                                        sampling occurs at posedge clock as specified by
                                        "posedge_offset".
                                        Default = 0.

   "vcd_handle"                 void*   Pointer to SystemC VCD object (sc_trace_file *)
                                        or 0 if tracing is not desired.


   \endverbatim
 *
 *
 * @see xtsc_queue_pin
 * @see xtsc::xtsc_parms
 * @see xtsc::xtsc_initialize_parms
 */
class XTSC_COMP_API xtsc_queue_pin_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for an xtsc_queue_pin_parms object.
   *
   * @param     width1          Width of each queue element in bits.
   *                            Default = 32.
   *
   * @param     depth           Number of elements the queue can hold.
   *                            Default = 16.
   *
   * @param     push_file       Name of file to write nb_push elements to instead
   *                            of adding them to the queue fifo.
   *
   * @param     pop_file        Name of file to read nb_pop elements from instead
   *                            of getting them from the queue fifo.
   *
   * @param     wraparound      Indicates if pop_file should wraparound to the
   *                            beginning of the file after the end of file is
   *                            reached.
   *
   * @param     p_trace_file    Pointer to SystemC VCD object or 0 if
   *                            tracing is not desired.
   *                            Default = 0.
   *
   */
  xtsc_queue_pin_parms(xtsc::u32                width1          = 32,
                       xtsc::u32                depth           = 16,
                       const char              *push_file       = 0,
                       const char              *pop_file        = 0,
                       bool                     wraparound      = false,
                       sc_core::sc_trace_file  *p_trace_file    = 0)
  {
    add("bit_width",            width1);
    add("depth",                depth);
    add("push_file",            push_file);
    add("timestamp",            true);
    add("pop_file",             pop_file);
    add("wraparound",           wraparound);
    add("vcd_handle",           (void*) p_trace_file); 
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("sample_phase",         0);
  }

  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_queue_pin_parms"; }
};








/**
 * A TIE queue implementation using the pin-level interface.
 *
 * Example XTSC queue implementation that connects at the pin-level.
 *
 * Here is a block diagram of an xtsc_queue_pin as it is used in the xtsc_queue_pin example:
 * @image html  Example_xtsc_queue_pin.jpg
 * @image latex Example_xtsc_queue_pin.eps "xtsc_queue_pin Example" width=10cm
 *
 * @see xtsc_queue_pin_parms
 */
class XTSC_COMP_API xtsc_queue_pin : public sc_core::sc_module {
public:

  xtsc::u32                             m_width1;               ///<  Bit width of each element

  sc_core::sc_in<sc_dt::sc_bv_base>     m_push;                 ///<  Push request from producer
  sc_core::sc_in<sc_dt::sc_bv_base>     m_data_in;              ///<  Input data from producer
  sc_core::sc_out<sc_dt::sc_bv_base>    m_full;                 ///<  Signal producer that queue is full

  sc_core::sc_in<sc_dt::sc_bv_base>     m_pop;                  ///<  Pop request from consumer
  sc_core::sc_out<sc_dt::sc_bv_base>    m_empty;                ///<  Signal consumer that queue is empty
  sc_core::sc_out<sc_dt::sc_bv_base>    m_data_out;             ///<  Output data to consumer

  xtsc::xtsc_signal_sc_bv_base_floating m_push_floating;        ///<  To cap unused push interface
  xtsc::xtsc_signal_sc_bv_base_floating m_data_in_floating;     ///<  To cap unused push interface
  xtsc::xtsc_signal_sc_bv_base_floating m_full_floating;        ///<  To cap unused push interface

  xtsc::xtsc_signal_sc_bv_base_floating m_pop_floating;         ///<  To cap unused pop interface
  xtsc::xtsc_signal_sc_bv_base_floating m_empty_floating;       ///<  To cap unused pop interface
  xtsc::xtsc_signal_sc_bv_base_floating m_data_out_floating;    ///<  To cap unused pop interface

  sc_core::sc_signal<xtsc::u32>         m_quantity;             ///<  Number of elements in queue


  // For SystemC
  SC_HAS_PROCESS(xtsc_queue_pin);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_queue_pin"; }


  /**
   * Constructor for an xtsc_queue_pin.
   *
   * @param     module_name     Name of the xtsc_queue_pin sc_module.
   *
   * @param     queue_parms     The remaining parameters for construction.
   *
   * @see xtsc_queue_pin_parms
   */
  xtsc_queue_pin(sc_core::sc_module_name module_name, const xtsc_queue_pin_parms& queue_parms);


  /// Destructor.
  ~xtsc_queue_pin();


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


protected:

  /// Cap unused interfaces and do checks
  void before_end_of_elaboration();

  /// Set m_has_pop_file_element and m_pop_file_element
  void get_next_pop_file_element();

  /// For regular queue using a fifo
  void worker_thread();

  /// For push to file and/or pop from file
  void file_worker_thread();

  sc_core::sc_time                      m_time_resolution;
  sc_core::sc_time                      m_clock_period;
  xtsc::u64                             m_clock_period_value;
  bool                                  m_has_posedge_offset;      ///< True if m_posedge_offset is non-zero
  sc_core::sc_time                      m_posedge_offset;          ///< From "posedge_offset" parameter
  xtsc::u64                             m_posedge_offset_value;    ///< m_posedge_offset as u64
  sc_core::sc_time                      m_sample_phase;
  xtsc::u64                             m_sample_phase_value;
  sc_dt::sc_bv_base                     m_zero;
  sc_dt::sc_bv_base                     m_one;
  int                                   m_rp;
  int                                   m_wp;

  sc_dt::sc_bv_base                   **m_element_ptrs;            ///<  To store the elements
  xtsc::u64                            *m_tickets;                 ///<  To store the ticket associated with each element

  xtsc::u32                             m_depth;                   ///<  Capacity - number of elements
  log4xtensa::TextLogger&               m_text;                    ///<  Text logger
  log4xtensa::BinaryLogger&             m_binary;                  ///<  Binary logger
  bool                                  m_log_data_binary;         ///<  True if transaction data should be logged by m_binary
  sc_core::sc_trace_file               *m_p_trace_file;

  bool                                  m_use_fifo;                ///<  False if pushing to a file and/or popping from a file
  const char                           *m_push_file_name;          ///<  Name of file to write elements to instead of using the fifo
  const char                           *m_pop_file_name;           ///<  Name of file to read elements from instead of using the fifo
  std::ofstream                        *m_push_file;               ///<  File to write elements to instead of using the fifo
  bool                                  m_timestamp;               ///<  From "timestamp" parameter
  xtsc::xtsc_script_file               *m_pop_file;                ///<  File to read elements from instead of using the fifo
  bool                                  m_wraparound;              ///<  False if m_pop_file should only be read through one time
  bool                                  m_has_pop_file_element;    ///<  For use by nb_can_pop() when using m_pop_file
  sc_dt::sc_bv_base                     m_pop_file_element;        ///<  For use by nb_pop() when using m_pop_file
  std::vector<std::string>              m_words;                   ///<  The tokenized words of the current line from m_pop_file
  std::string                           m_line;                    ///<  The current line from m_pop_file
  xtsc::u32                             m_next_word_index;         ///<  Index into m_words.
  xtsc::u32                             m_pop_file_line_number;    ///<  The line number of m_words in m_pop_file
};



}  // namespace xtsc_component

#endif  // _XTSC_QUEUE_PIN_H_

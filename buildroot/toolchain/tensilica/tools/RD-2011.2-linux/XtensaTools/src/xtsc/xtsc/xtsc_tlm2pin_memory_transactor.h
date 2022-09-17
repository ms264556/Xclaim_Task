#ifndef _XTSC_TLM2PIN_MEMORY_TRANSACTOR_H_
#define _XTSC_TLM2PIN_MEMORY_TRANSACTOR_H_

// Copyright (c) 2007-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

/**
 * @file 
 */


#include <deque>
#include <vector>
#include <string>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_respond_if.h>
#include <xtsc/xtsc_module_pin_base.h>
#include <xtsc/xtsc_memory_base.h>
#include <xtsc/xtsc_core.h>




namespace xtsc {
// Forward references
class xtsc_cohctrl;
};



namespace xtsc_component {


// Forward references
class xtsc_arbiter;
class xtsc_master;
class xtsc_memory_trace;
class xtsc_router;


/**
 * Constructor parameters for a xtsc_tlm2pin_memory_transactor transactor object.
 *
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------------

   "memory_interface"   char*   The memory interface type.  Valid values are "DRAM0",
                                "DRAM1", "DROM0", "IRAM0", "IRAM1", "IROM0", "URAM0",
                                "XLMI0", and "PIF" (case-insensitive).
                                Note: For inbound PIF, set this parameter to "PIF" and
                                set the "inbound_pif" parameter to true.
                                Note: For snoop port, set this parameter to "PIF" and
                                set the "snoop" parameter to true.
    
   "num_ports"          u32     The number of memory ports this transactor has.  A value
                                of 1 means this transactor is single-ported, a value of
                                2 means this transactor is dual-ported, etc.
                                Default = 1.
                                Minimum = 1.

   "port_name_suffix"   char*   Optional constant suffix to be appended to every input
                                and output port name.
                                Default = "".

   "byte_width"         u32     Memory data interface width in bytes.  Valid values for
                                "DRAM0", "DRAM1", "DROM0", "URAM0", and "XLMI0" are 4,
                                8, 16, 32, and 64.  Valid values for "IRAM0", "IRAM1",
                                "IROM0", and "PIF" are 4, 8, and 16.

   "start_byte_address" u32     The number to be subtracted from the address received in
                                the TLM request.  For non-XLMI local memories, this
                                corresponds to the starting byte address of the memory
                                in the 4GB address space.  For PIF and XLMI this should
                                be 0x00000000.
                                Default = 0x00000000.
  
   "big_endian"         bool    True if the memory interface master is big endian.
                                Default = false.

   "dso_name"           char*   Optional name of a DSO (Linux shared object or MS Windows
                                DLL) that contains peek and poke functions declared with
                                the signature and with extern "C" (and __declspec if 
                                MS Windows) as shown here:
#ifdef _WIN32
#define DSO_EXPORT extern "C" __declspec(dllexport) 
#else
#define DSO_EXPORT extern "C" 
#endif
typedef unsigned char u8;
typedef unsigned int  u32;
DSO_EXPORT void peek(u32 address8, u32 size8,       u8 *buffer, const char *dso_cookie, u32 port);
DSO_EXPORT void poke(u32 address8, u32 size8, const u8 *buffer, const char *dso_cookie, u32 port);
                                Typically this parameter is only needed when connecting
                                this module to a Verilog/SystemVerilog module for
                                cosimulation when peek/poke access is desired to the
                                Verilog memory (for example, for loading a target
                                program using xtsc_core, for TurboXim or xt-gdb access,
                                or for hostlink support for argv[], clock(), etc).  The
                                idea is that the named DSO, provided by the user, is
                                able to use some non-blocking mechanism (i.e. a
                                mechanism that does not require any simulation time to
                                elapse) to communicate the peeks/pokes to the Verilog/
                                SystemVerilog module (for example, using a SystemVerilog
                                export "DPI-C" function).  The dso_cookie argument will
                                come from the "dso_cookie" parameter.  
                                Caution: When a DSO is used during SystemC-Verilog
                                         cosimulation, be sure to use the xtsc_core_parm
                                         parameter SimTargetProgram to name the core
                                         program (if any) that will be loaded using the
                                         DSO.  The xtsc-run command --core_program=
                                         should not be used in this situation because it
                                         can result in the core program being loaded
                                         prior to the Verilog memory being constructed.
                                         This is vendor specific but results in a
                                         failure that is hard to decipher.
                                Default = NULL.

   "dso_cookie"         char*   Optional C-string to pass to the peek and poke methods
                                of the DSO named by the "dso_name" parameter.  This
                                model does not use this parameter in any way other then
                                to pass it to the DSO methods.
                                Default = NULL.

   "cosim"              bool    This parameter is for when the model is connected to a
                                Verilog model and "dso_name" is NULL.  Because Verilog
                                is not automatically compatible with the xtsc_debug_if
                                methods (nb_peek, nb_poke, and nb_fast_access) a way is
                                needed to prevent a run-time exception if these methods
                                are called.  If "cosim" is false and the user does not
                                connect the m_debug_ports (either directly or by passing
                                a DSO name in using the "dso_name" parameter) then a
                                call to one of these methods coming in on
                                m_request_exports will result in an exception being
                                thrown.  If "cosim" is true and the user does not
                                connect the m_debug_ports then a call to one of these
                                methods will be handled according to the "shadow_memory"
                                parameter.
                                Default = false.

   "shadow_memory"      bool    If "cosim" is false or if the user connects the
                                m_debug_ports (either directly or by passing a DSO name
                                in using the "dso_name" parameter) then this parameter
                                is ignored.  Otherwise, if "shadow_memory" is false,
                                calls to nb_peek() and nb_poke() will be ignored and
                                calls to nb_fast_access() will return false.  If
                                "shadow_memory" is true, then all writes (calls to
                                nb_request for WRITE, BLOCK_WRITE, BURST_WRITE, and the
                                write beat of RCW) and all nb_poke() calls will update a
                                locally-maintained shadow memory (all writes will also
                                be driven out the pin-level interface).  All nb_peek()
                                calls will return data from the shadow memory.  Reads
                                (calls to nb_request for READ, BLOCK_READ, BURST_READ,
                                and the read beat of RCW) will never use the shadow
                                memory.  Calls to nb_fast_access will return false.
                                Note: The shadow memory mechanism is far from perfect
                                and should NOT be relied upon for accurate simulation
                                and debugging.  Here are some ways in which the shadow
                                memory could be inaccurate:
                                1. Write requests always update the shadow memory even 
                                   if the pin-level write request is not accepted or is
                                   accepted and then rejected due to, for example, an
                                   address or data error.
                                2. The write beat of RCW always updates the shadow memory
                                   regardless of the result of the conditional read part.
                                3. The Verilog memory, as opposed to the shadow memory,
                                   might be modified by other means then through this
                                   module.
                                4. If the debugger (xt-gdb/Xplorer) is used to change
                                   memory contents, only the shadow memory will be
                                   changed.  The Verilog memory, which is what the
                                   Xtensa processor will see when it does read requests,
                                   will not be changed.
                                5. The hostlink support for certain features such as
                                   argv[] and the clock() function use pokes to put data
                                   in memory which is then retrieved using read requests
                                   by the Xtensa processor.  The poke will go to the
                                   shadow memory; however, the Xtensa will read the
                                   Verilog memory.
                                Despite the above caveat and issues, in cases where a
                                peek/poke DSO is not available, a shadow memory can
                                sometimes prove useful (for example, to support printf
                                in target code and/or partial debugger visibility).
                                Default = false.

   "initial_value_file" char*   If not NULL or empty, this names a text file from which
                                to read the initial memory contents as byte values.
                                This parameter is ignored unless a shadow memory is
                                used.
                                Default = NULL.
                                The text file format is:

                                ([@<Offset>] <Value>*)*

                                1.  Any number (<Offset> or <Value>) can be in decimal
                                    or hexadecimal (using '0x' prefix) format.
                                2.  @<Offset> is added to "start_byte_address".
                                3.  <Value> cannot exceed 255 (0xFF).
                                4.  If a <Value> entry is not immediately preceeded in
                                    the file by an @<Offset> entry, then its offset is
                                    one greater than the preceeding <Value> entry.
                                5.  If the first <Value> entry in the file is not 
                                    preceeded by an @<Offset> entry, then its offset
                                    is zero.
                                6.  Comments, extra whitespace, and blank lines are
                                    ignored.  See xtsc::xtsc_script_file.

                                Example text file contents:

                                   0x01 0x02 0x3    // First three bytes of the memory,
                                                    // 0x01 is at "start_byte_address"
                                   @0x1000 50       // The byte at offset 0x1000 is 50
                                   51 52            // The byte at offset 0x1001 is 51
                                                    // The byte at offset 0x1002 is 52

   "memory_fill_byte"   u32     The low byte specifies the value used to initialize 
                                memory contents at address locations not initialize
                                from "initial_value_file".
                                This parameter is ignored unless a shadow memory is
                                used.
                                Default = 0.

   "clock_period"       u32     This is the length of this device's clock period
                                expressed in terms of the SystemC time resolution
                                (from sc_get_time_resolution()).  A value of 
                                0xFFFFFFFF means to use the XTSC system clock 
                                period (from xtsc_get_system_clock_period()).  A value
                                of 0 means one delta cycle.
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

   "sample_phase"       u32     This specifies the phase (i.e. the point) in a clock
                                period at which input pins are sampled.  PIF output pins
                                which are used for handshaking (POReqValid/PORespRdy)
                                are also sampled at this time.  This value is expressed
                                in terms of the SystemC time resolution (from
                                sc_get_time_resolution()) and must be less than the
                                clock period as specified by the "clock_period"
                                parameter.  A value of 0 means pins are sampled on
                                posedge clock as specified by "posedge_offset".  A value
                                of 0xFFFFFFFF, the default, means that, if
                                "memory_interface" is PIF, then pins will be sampled at
                                posedge clock, and if "memory_interface" is not PIF
                                (i.e. a local memory), then pins will be sampled at 1
                                SystemC time resolution prior to phase A.  This later
                                value is used to enable meeting the TLM timing
                                requirements of the local memory interfaces of
                                xtsc_core.  See the discussion under
                                xtsc_core::set_clock_phase_delta_factors().
                                Default = 0xFFFFFFFF

   "output_delay"       u32     This specifies how long to delay before output pins are
                                driven.  The output pins will remain driven for one
                                clock period (see the "clock_period" parameter).  For
                                request output pins, the delay timing starts when the
                                nb_request() call is received.  For DPortLoadRetired,
                                DPortRetireFlush, and DRamLock, the delay timing starts
                                when the nb_load_retire(), nb_retire_flush(), or
                                nb_lock() call (respectively) is received.  For
                                PORespRdy, the delay timing starts at the sample phase
                                (see "sample_phase") when the nb_respond() call
                                returns false.  This value is expressed in terms of the
                                SystemC time resolution (from sc_get_time_resolution())
                                and must be less than the clock period.  A value of 0
                                means one delta cycle.
                                Default = 1 (i.e. 1 time resolution).

   "vcd_handle"         void*   Pointer to SystemC VCD object (sc_trace_file *) or
                                0 if tracing is not desired.

   "request_fifo_depth" u32     The request fifo depth.  
                                Default = 1.
                                Minimum = 1.


   Parameters which apply to PIF only:

   "inbound_pif"        bool    Set to true for inbound PIF.  Set to false for outbound
                                PIF or snoop.  This parameter is ignored if
                                "memory_interface" is other then "PIF".
                                Default = false (outbound PIF or snoop).

   "snoop"              bool    Set to true for snoop port.  Set to false for outbound
                                or inbound PIF.  This parameter is ignored if
                                "memory_interface" is other then "PIF".
                                Default = false (outbound or inbound PIF).

   "has_coherence"      bool    True if the "POReqCohCntl", "POReqCohVAdrsIndex", and
                                "PIRespCohCntl" ports should be present.  This parameter
                                is ignored unless "memory_interface" is "PIF" and
                                "inbound_pif" and "snoop" are both false.
                                Default = false.

   "has_pif_attribute"  bool    True if the "POReqAttribute" or "PIReqAttribute" port
                                should be present.  This parameter is ignored unless
                                "memory_interface" is "PIF" and "snoop" is false.
                                Default = false.

   "has_request_id"     bool    True if the "POReqId" and "PIRespId" ports should be
                                present.  This parameter is ignored unless
                                "memory_interface" is "PIF".

   "write_responses"    bool    True if write responses should be generated by the
                                xtsc_tlm2pin_memory_transactor.  False if write responses will
                                be generated by the downstream slave.  This parameter is
                                ignored unless "memory_interface" is "PIF".
                                Default = false.
                                Note for xtsc-run users:  When the --cosim command is
                                present, xtsc-run will initialize "write_responses" with
                                a value appropriate for the current config.

   "route_id_bits"      u32     Number of bits in the route ID.  Valid values are 0-32.
                                If "route_id_bits" is 0, then the "POReqRouteId" and
                                "PIRespRouteId" output ports will not be present.  This
                                parameter is ignored unless "memory_interface" is "PIF".


   Parameters which apply to local memories only (that is, non-PIF memories):

   "address_bits"       u32     Number of bits in the address.  This parameter is
                                ignored if "memory_interface" is "PIF".

   "check_bits"         u32     Number of bits in the parity/ecc signals.  This
                                parameter is ignored unless "memory_interface" is
                                "IRAM0", "IRAM1", "DRAM0", or "DRAM1".
                                Default = 0.
  
   "has_busy"           bool    True if the memory interface has a busy pin.  This
                                parameter is ignored if "memory_interface" is "PIF".
                                Default = true.
  
   "has_lock"           bool    True if the memory interface has a lock pin.  This
                                parameter is ignored unless "memory_interface" is
                                "DRAM0" or "DRAM1".
                                Default = false.
  
   "has_xfer_en"        bool    True if the memory interface has an xfer enable pin.
                                This parameter is ignored if "memory_interface" is
                                "DROM0", "XLMI0", or "PIF".
                                Default = false.
  
   "read_delay"         u32     Number of clock periods to delay before read data is
                                sampled.  This should be 0 for a 5-stage pipeline and 1
                                for a 7-stage pipeline.  This parameter is ignored if
                                "memory_interface" is "PIF".
                                Default = 0.
  
   "cbox"               bool    True if this memory interface is driven from an Xtensa
                                CBOX.  This parameter is ignored if "memory_interface"
                                is other then "DRAM0"|"DRAM1"|"DROM0".
    \endverbatim
 *
 * @see xtsc_tlm2pin_memory_transactor
 * @see xtsc::xtsc_parms
 * @see xtsc::xtsc_initialize_parms
 */
class XTSC_COMP_API xtsc_tlm2pin_memory_transactor_parms : public xtsc::xtsc_parms {
public:


  /**
   * Constructor for an xtsc_tlm2pin_memory_transactor_parms transactor object.
   *
   * @param     memory_interface        The memory interface type.  Valid values are
   *                                    "DRAM0", "DRAM1", "DROM0", "IRAM0", "IRAM1",
   *                                    "IROM0", "URAM0", "XLMI0", and "PIF"
   *                                    (case-insensitive).
   *
   * @param     byte_width              Memory data interface width in bytes.
   *
   * @param     address_bits            Number of bits in address.  Ignored for "PIF".
   *
   * @param     num_ports               The number of memory ports this transactor has.
   */
  xtsc_tlm2pin_memory_transactor_parms(const char      *memory_interface        = "PIF",
                                       xtsc::u32        byte_width              = 4,
                                       xtsc::u32        address_bits            = 32,
                                       xtsc::u32        num_ports               = 1)
  {
    init(memory_interface, byte_width, address_bits, num_ports);
  }


  /**
   * Constructor for an xtsc_tlm2pin_memory_transactor_parms transactor object based
   * upon an xtsc_core object and a named memory interface. 
   *
   * This constructor will determine "clock_period", "byte_width", "big_endian",
   * "read_delay", "address_bits", "start_byte_address", "has_busy", "has_lock", "cbox",
   * "check_bits", and "has_pif_attribute" by querying the core object.  
   *
   * If desired, after the xtsc_tlm2pin_memory_transactor_parms object is constructed,
   * its data members can be changed using the appropriate xtsc_parms::set() method
   * before passing it to the xtsc_tlm2pin_memory_transactor constructor.
   *
   * @param     core                    A reference to the xtsc_core object upon which
   *                                    to base the xtsc_tlm2pin_memory_transactor_parms.
   *
   * @param     memory_interface        The memory interface type.  Valid values are
   *                                    "DRAM0", "DRAM1", "DROM0", "IRAM0", "IRAM1",
   *                                    "IROM0", "URAM0", "XLMI0", and "PIF"
   *                                    (case-insensitive).
   *
   * @param     num_ports               The number of ports this memory has.  If 0, the
   *                                    default, the number of ports (1 or 2) will be
   *                                    inferred thusly: If memory_name is a LD/ST unit
   *                                    0 port of a dual-ported core interface, and the
   *                                    core is dual-ported, and if the 2nd port of the
   *                                    core has not been bound, then "num_ports" will
   *                                    be 2; otherwise, "num_ports" will be 1.
   *
   */
  xtsc_tlm2pin_memory_transactor_parms(const xtsc::xtsc_core& core, const char *memory_interface, xtsc::u32 num_ports = 0);


  // Perform initialization common to both constructors
  void init(const char *memory_interface, xtsc::u32 byte_width, xtsc::u32 address_bits, xtsc::u32 num_ports) {
    add("memory_interface",     memory_interface);
    add("inbound_pif",          false);
    add("snoop",                false);
    add("has_coherence",        false);
    add("has_pif_attribute",    false);
    add("num_ports",            num_ports);
    add("port_name_suffix",     "");
    add("byte_width",           byte_width);
    add("start_byte_address",   0x00000000);
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("sample_phase",         0xFFFFFFFF);
    add("output_delay",         1);
    add("big_endian",           false);
    add("dso_name",             (char*)NULL);
    add("dso_cookie",           (char*)NULL);
    add("cosim",                false);
    add("shadow_memory",        false);
    add("test_shadow_memory",   false);         // Do not use - for test purposes only
    add("initial_value_file",   (char*)NULL);
    add("memory_fill_byte",     0);
    add("has_request_id",       true);
    add("write_responses",      false);
    add("address_bits",         address_bits);
    add("route_id_bits",        0);
    add("has_busy",             true);
    add("has_lock",             false);
    add("has_xfer_en",          false);
    add("read_delay",           0);
    add("vcd_handle",           (void*)NULL);
    add("request_fifo_depth",   1);
    add("cbox",                 false);
    add("check_bits",           0);
  }


  /// Our C++ type (the xtsc_parms base class uses this for error messages)
  virtual const char* kind() const { return "xtsc_tlm2pin_memory_transactor_parms"; }

};




/**
 * This transactor converts memory transactions from transaction level (TLM) to pin level.
 *
 * This module is a transactor which converts TLM memory requests (xtsc_request_if) to
 * pin-level requests and the corresponding pin-level responses into TLM responses
 * (xtsc_respond_if).
 *
 * On the TLM side, this module can be connected with any XTSC memory interface master
 * (e.g. xtsc_core, xtsc_arbiter, xtsc_master, xtsc_router, etc).  However, it is always
 * configured for the specified memory interface of xtsc_core (such as DRAM0, DRAM1,
 * IRAM0, PIF, etc).  See the xtsc_tlm2pin_memory_transactor_parms "memory_interface"
 * parameter.
 *
 * Although there is a pin-level XTSC memory model (xtsc_memory_pin) that this
 * transactor can be connected to on the pin-level side, the main use for this
 * transactor is assumed to be for connecting to RTL (Verilog) models of, for instance,
 * XLMI or PIF devices.  When connecting to RTL, there is no guarranteed support for the
 * TLM debug interface (see the m_debug_ports data member) because there is no way to
 * guarrantee support for non-blocking access across the SystemC-Verilog boundary for
 * peek and poke to arbitrary Verilog modules.  See the discussion of the "dso_name",
 * "dso_cookie", "cosim", and "shadow_memory" parameters in
 * xtsc_tlm2pin_memory_transactor_parms for more information.
 *
 * This module inherits from the xtsc_module_pin_base class which is responsible for
 * maintaining the pin-level sc_in<> and sc_out<> ports.  The pin-level port names
 * exactly match the Xtensa RTL.  These names, as well as their SystemC type and bit
 * width, are log at info log-level when the module is constructed.
 *
 * This module supports driving multi-ported memories.  See the "num_ports" parameter of
 * xtsc_tlm2pin_memory_transactor_parms.
 *
 * Note: The parity/ECC signals (DRamNCheckDataM, DRamNCheckWrDataM, IRamNCheckData, and
 * IRamNCheckWrData) are present for IRAM and DRAM interfaces when "check_bits" is
 * non-zero; however, the input signal is ignored and the output signal is driven with
 * constant 0.
 *
 * Here is a block diagram of an xtsc_tlm2pin_memory_transactor as it is used in the
 * xtsc_tlm2pin_memory_transactor example:
 * @image html  Example_xtsc_tlm2pin_memory_transactor.jpg
 * @image latex Example_xtsc_tlm2pin_memory_transactor.eps "xtsc_tlm2pin_memory_transactor Example" width=13cm
 *
 * @see xtsc_module_pin_base
 * @see xtsc_memory_pin
 * @see xtsc_tlm2pin_memory_transactor_parms
 * @see xtsc::xtsc_core
 * @see xtsc::xtsc_cohctrl
 * @see xtsc_arbiter
 * @see xtsc_master
 * @see xtsc_router
 *
 */
class XTSC_COMP_API xtsc_tlm2pin_memory_transactor : public sc_core::sc_module, public xtsc_module_pin_base, public xtsc::xtsc_resettable {
public:

  sc_core::sc_export<xtsc::xtsc_request_if>           **m_request_exports;      /// From master to us (per mem port)
  sc_core::sc_port  <xtsc::xtsc_respond_if>           **m_respond_ports;        /// From us to master (per mem port)
  sc_core::sc_port  <xtsc::xtsc_debug_if, NSPP>       **m_debug_ports;          /// From us to slave  (per mem port)

  /**
   * @see xtsc_module_pin_base::get_bool_input()
   * @see xtsc_module_pin_base::get_uint_input()
   * @see xtsc_module_pin_base::get_wide_input()
   * @see xtsc_module_pin_base::get_bool_output()
   * @see xtsc_module_pin_base::get_uint_output()
   * @see xtsc_module_pin_base::get_wide_output()
   */
  xtsc::Readme How_to_get_input_and_output_ports;


  // Shorthand aliases
  typedef xtsc::xtsc_request                            xtsc_request;
  typedef std::pair<sc_core::sc_time, sc_core::sc_time> start_stop_time;
  typedef std::deque<start_stop_time>                   start_stop_deque;
  typedef sc_core::sc_signal<bool>                      bool_signal;
  typedef sc_core::sc_fifo<bool>                        bool_fifo;

  /// This SystemC macro inserts some code required for SC_THREAD's to work
  SC_HAS_PROCESS(xtsc_tlm2pin_memory_transactor);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_tlm2pin_memory_transactor"; }


  /**
   * Constructor for a xtsc_tlm2pin_memory_transactor.
   *
   * @param     module_name             Name of the xtsc_tlm2pin_memory_transactor
   *                                    sc_module.
   * @param     tlm2pin_parms           The remaining parameters for construction.
   *
   * @see xtsc_tlm2pin_memory_transactor_parms
   */
  xtsc_tlm2pin_memory_transactor(sc_core::sc_module_name module_name, const xtsc_tlm2pin_memory_transactor_parms& tlm2pin_parms);


  /// Destructor.
  ~xtsc_tlm2pin_memory_transactor(void);


  /// Return true if "dso_name" was provided
  bool has_dso() const;


  /**
   * Connect an xtsc_arbiter with this xtsc_tlm2pin_memory_transactor.
   *
   * This method connects the master port pair of the xtsc_arbiter with the specified
   * TLM slave port pair of this xtsc_tlm2pin_memory_transactor.
   *
   * @param     arbiter         The xtsc_arbiter to connect with this
   *                            xtsc_tlm2pin_memory_transactor.
   *
   * @param     tran_port       The TLM slave port pair of this xtsc_tlm2pin_memory_transactor to
   *                            connect with the xtsc_arbiter.
   */
  void connect(xtsc_arbiter& arbiter, xtsc::u32 tran_port = 0);


  /**
   * Connect an xtsc_cohctrl with this xtsc_tlm2pin_memory_transactor.
   *
   * Depending upon the "snoop" parameter value, this method connects a master port pair
   * of either the memory interface ("snoop" is false) or the snoop interface ("snoop"
   * is true) of the xtsc_cohctrl with a TLM slave port pair of this
   * xtsc_tlm2pin_memory_transactor.
   *
   * @param     cohctrl         The xtsc_cohctrl to connect with this
   *                            xtsc_tlm2pin_memory_transactor.
   *
   * @param     port            If "snoop" is false, this specifies which slave port
   *                            pair of this xtsc_tlm2pin_memory_transactor to connect
   *                            the memory interface master port pair of the
   *                            xtsc_cohctrl with.  If "snoop" is true, this specifies
   *                            which snoop master port pair of the xtsc_cohctrl to
   *                            connect with this xtsc_tlm2pin_memory_transactor.
   */
  void connect(xtsc::xtsc_cohctrl& cohctrl, xtsc::u32 port = 0);


  /**
   * Connect an xtsc_core with this xtsc_tlm2pin_memory_transactor.
   *
   * This method connects the specified memory interface master port pair of the
   * specified xtsc_core with the specified TLM slave port pair of this
   * xtsc_tlm2pin_memory_transactor.
   *
   * @param     core                    The xtsc_core to connect with this
   *                                    xtsc_tlm2pin_memory_transactor.
   *
   * @param     memory_port_name        The name of the memory interface master port
   *                                    pair of the xtsc_core to connect with this
   *                                    xtsc_tlm2pin_memory_transactor. 
   *                                    Case-insensitive.
   *
   * @param     tran_port               The slave port pair of this
   *                                    xtsc_tlm2pin_memory_transactor to connect the
   *                                    xtsc_core with.
   *
   * @param     single_connect          If true only one port pair of the xtsc_core and
   *                                    this xtsc_tlm2pin_memory_transactor will be
   *                                    connected.  If false, the default, and if
   *                                    memory_port_name names a LD/ST unit 0
   *                                    dual-ported interface of core and if tran_port+1
   *                                    exists and has not yet been connected, then
   *                                    tran_port and tran_port+1 will be connected with
   *                                    core.
   *
   * @see xtsc::xtsc_core::How_to_do_memory_port_binding for a list of valid
   *      memory_port_name values.
   *
   * @returns number of ports that were connected by this call (1 or 2)
   */
  xtsc::u32 connect(xtsc::xtsc_core& core, const char *memory_port_name, xtsc::u32 tran_port = 0, bool single_connect = false);


  /**
   * Connect an xtsc_master with this xtsc_tlm2pin_memory_transactor.
   *
   * This method connects the master port pair of the xtsc_master with the TLM slave
   * port pair of this xtsc_tlm2pin_memory_transactor.
   *
   * @param     master          The xtsc_master to connect with this
   *                            xtsc_tlm2pin_memory_transactor.
   *
   * @param     tran_port       The slave port pair of this
   *                            xtsc_tlm2pin_memory_transactor to connect the
   *                            xtsc_master with.
   */
  void connect(xtsc_master& master, xtsc::u32 tran_port = 0);


  /**
   * Connect an xtsc_memory_trace with this xtsc_tlm2pin_memory_transactor.
   *
   * This method connects the specified master port pair of the specified upstream
   * xtsc_memory_trace with the specified TLM slave port pair of this
   * xtsc_tlm2pin_memory_transactor.  
   *
   * @param     memory_trace    The xtsc_memory_trace to connect with this
   *                            xtsc_tlm2pin_memory_transactor.
   *
   * @param     trace_port      The master port pair of the xtsc_memory_trace to connect
   *                            with this xtsc_tlm2pin_memory_transactor.
   *
   * @param     tran_port       The TLM slave port pair of this
   *                            xtsc_tlm2pin_memory_transactor to connect the
   *                            xtsc_memory_trace with.
   *
   * @param     single_connect  If true only one TLM slave port pair of this
   *                            xtsc_tlm2pin_memory_transactor will be connected.  If
   *                            false, the default, then all contiguous, unconnected
   *                            slave port pairs of this xtsc_tlm2pin_memory_transactor
   *                            starting at tran_port that have a corresponding existing
   *                            master port pair in memory_trace (starting at
   *                            trace_port) will be connected with that corresponding
   *                            memory_trace master port pair.
   *
   * @returns number of ports that were connected by this call (1 or more)
   */
  xtsc::u32 connect(xtsc_memory_trace& memory_trace, xtsc::u32 trace_port = 0, xtsc::u32 tran_port = 0, bool single_connect = false);


  /**
   * Connect an xtsc_router with this xtsc_tlm2pin_memory_transactor.
   *
   * This method connects the specified master port pair of the specified xtsc_router
   * with the specified TLM slave port pair of this xtsc_tlm2pin_memory_transactor.
   *
   * @param     router          The xtsc_router to connect with this
   *                            xtsc_tlm2pin_memory_transactor.
   *
   * @param     router_port     The master port pair of the xtsc_router to connect with
   *                            this xtsc_tlm2pin_memory_transactor.  router_port must
   *                            be in the range of 0 to the xtsc_router's "num_slaves"
   *                            parameter minus 1.
   *
   * @param     tran_port       The TLM slave port pair of this
   *                            xtsc_tlm2pin_memory_transactor to connect the
   *                            xtsc_router with.
   */
  void connect(xtsc_router& router, xtsc::u32 router_port, xtsc::u32 tran_port = 0);


  virtual void reset(bool hard_reset = false);


  /**
   *  Handle nb_peek() calls.
   *
   *  This method can be overriden by a derived class to provide custom nb_peek handling
   *  when this device is used to connect to a Verilog module.
   */
  virtual void peek(xtsc::u32 port_num, xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);


  /**
   *  Handle nb_poke() calls.
   *
   *  This method can be overriden by a derived class to provide custom nb_poke handling
   *  when this device is used to connect to a Verilog module.
   */
  virtual void poke(xtsc::u32 port_num, xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);


  /**
   *  Handle nb_fast_access() calls.
   *
   *  This method can be overriden by a derived class to provide custom nb_fast_access
   *  handling when this device is used to connect to a Verilog module.
   */
  virtual bool fast_access(xtsc::u32 port_num, xtsc::xtsc_fast_access_request &request);


  /// Return the interface type
  memory_interface_type get_interface_type() const { return m_interface_type; }


  /// Return the interface name string
  const char *get_interface_name() const { return xtsc_module_pin_base::get_interface_name(m_interface_type); }


  /// Return the number of memory ports this transactor has
  xtsc::u32 get_num_ports() const { return m_num_ports; }


  /// Return true if pin port names include the set_id as a suffix
  bool get_append_id() const { return m_append_id; }


protected:


  /// Information about each response
  class response_info {
  public:
    /// Constructor
    response_info(xtsc::xtsc_response          *p_response,
                  xtsc::u32                     bus_addr_bits,
                  xtsc::u32                     size,
                  bool                          is_read,
                  xtsc::u32                     id,
                  xtsc::u32                     route_id) :
      m_p_response      (p_response),
      m_bus_addr_bits   (bus_addr_bits),
      m_size            (size),
      m_id              (id),
      m_route_id        (route_id),
      m_is_read         (is_read),
      m_copy_data       (false),
      m_status          (0),
      m_last            (true)
    {}
    xtsc::xtsc_response        *m_p_response;                           ///< The response
    xtsc::u32                   m_bus_addr_bits;                        ///< Bits of xtsc_request address that identify byte lanes
    xtsc::u32                   m_size;                                 ///< Size from xtsc_request
    xtsc::u32                   m_id;                                   ///< Transfer ID from xtsc_request
    xtsc::u32                   m_route_id;                             ///< Route ID from xtsc_request
    bool                        m_is_read;                              ///< Expect read data in response
    bool                        m_copy_data;                            ///< True if data needs to be copied from m_buffer to response
    xtsc::u32                   m_status;                               ///< Response status
    bool                        m_last;                                 ///< True if actual response has last_transfer bit set
    xtsc::u8                    m_buffer[xtsc::xtsc_max_bus_width8];    ///< Buffer for non-last BLOCK_READ read data
  };


  /// SystemC callback
  virtual void before_end_of_elaboration(void);


  /// SystemC callback
  virtual void end_of_elaboration(void);


  /// SystemC callback
  virtual void start_of_simulation(void);


  /// Handle PIF requests
  void pif_request_thread(void);


  /// Handle PIF responses
  void pif_response_thread(void);


  /// PIF: Drive PORespRdy
  void pif_drive_resp_rdy_thread(void);


  /// Handle local memory requests
  void lcl_request_thread(void);


  /// Thread to sample local busy and/or send write response
  void lcl_busy_write_rsp_thread(void);


  /// Thread to sample local memory read data and respond
  void lcl_sample_read_data_thread(void);


  /// XLMI: Handle DPortLoadRetired pin
  void xlmi_retire_thread(void);


  /// XLMI: Handle DPortRetireFlush pin
  void xlmi_flush_thread(void);


  /// XLMI: Handle DPortLoad pin
  void xlmi_load_thread(void);


  /// XLMI: Help handle DPortLoad pin when interface has a busy pin
  void xlmi_load_method(void);


  /// DRAM: Handle DRamLock
  void dram_lock_thread(void);


  /// Log and send a local-memory or RSP_NACC response; also delete it if it is a last transfer
  void send_unchecked_response(xtsc::xtsc_response*& p_response, xtsc::u32 port);


  /// Get a new response_info (from the pool)
  response_info *new_response_info(xtsc::xtsc_response         *p_response,
                                   xtsc::u32                    bus_addr_bits,
                                   xtsc::u32                    size,
                                   bool                         is_read = false,
                                   xtsc::u32                    id = 0,
                                   xtsc::u32                    route_id = 0);


  /// Get a new response_info (from the pool) and initialize it by copying
  response_info *new_response_info(const response_info& info);


  /// Delete an response_info (return it to the pool)
  void delete_response_info(response_info*& p_response_info);


  /// Get a new xtsc_request (from the pool) that is a copy of the specified request
  xtsc::xtsc_request *new_request(const xtsc::xtsc_request& request);


  /// Delete an xtsc_request (return it to the pool)
  void delete_request(xtsc::xtsc_request*& p_request);


  /// Swizzle byte enables 
  void swizzle_byte_enables(xtsc::xtsc_byte_enables& byte_enables) const;


  /// Implementation of xtsc_request_if.
  class xtsc_request_if_impl : public xtsc::xtsc_request_if, public sc_core::sc_object {
  public:

    /// Constructor
    xtsc_request_if_impl(const char *object_name, xtsc_tlm2pin_memory_transactor& transactor, xtsc::u32 port_num) :
      sc_object         (object_name),
      m_transactor      (transactor),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// The kind of sc_object we are
    const char* kind() const { return "xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl"; }

    /**
     *  Receive peeks from the memory interface master
     *  @see xtsc::xtsc_request_if
     */
    void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /**
     *  Receive pokes from the memory interface master
     *  @see xtsc::xtsc_request_if
     */
    void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /**
     *  Receive requests for information about how to do fast access from the memory
     *  interface master
     *  @see xtsc::xtsc_request_if
     */
    bool nb_fast_access(xtsc::xtsc_fast_access_request &request);

    /**
     *  Receive requests from the memory interface master
     *  @see xtsc::xtsc_request_if
     */
    void nb_request(const xtsc::xtsc_request& request);

    /**
     *  For XLMI: DPortLoadRetired.
     *  @see xtsc::xtsc_request_if
     */
    void nb_load_retired(xtsc::xtsc_address address8);

    /**
     *  For XLMI: DPortRetireFlush.
     *  @see xtsc::xtsc_request_if
     */
    void nb_retire_flush();

    /**
     *  For DRamnLockm.
     *  @see xtsc::xtsc_request_if
     */
    void nb_lock(bool lock);

    /// Return true if a port has been bound to this implementation
    bool is_connected() { return (m_p_port != 0); }


  protected:

    /// SystemC callback when something binds to us
    void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_tlm2pin_memory_transactor&     m_transactor;   ///< Our xtsc_tlm2pin_memory_transactor object
    sc_core::sc_port_base              *m_p_port;       ///< Port that is bound to us
    xtsc::u32                           m_port_num;     ///< Our port number
  };



  /**
   * To cap an unconnected m_debug_ports port when the user can't bind anything to it.
   * For example, when connecting to RTL and a DSO cannot be provided (see "dso_name").
   */
  class xtsc_debug_if_cap : public xtsc::xtsc_debug_if {
  public:

    /// Constructor
    xtsc_debug_if_cap(xtsc_tlm2pin_memory_transactor& transactor, xtsc::u32 port_num) :
      m_transactor      (transactor),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /**
     *  Receive peeks from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /**
     *  Receive pokes from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /**
     *  Receive fast access requests from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    bool nb_fast_access(xtsc::xtsc_fast_access_request &request);


  protected:

    /// SystemC callback when something binds to us
    void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_tlm2pin_memory_transactor&     m_transactor;   ///< Our xtsc_tlm2pin_memory_transactor object
    sc_core::sc_port_base              *m_p_port;       ///< Port that is bound to us
    xtsc::u32                           m_port_num;     ///< Our port number
  };


#if !defined(_WIN32)
  typedef void *HMODULE;
#endif

  typedef void (*peek_t)(xtsc::u32 address8, xtsc::u32 size8,       xtsc::u8 *buffer, const char *dso_cookie, xtsc::u32 port);
  typedef void (*poke_t)(xtsc::u32 address8, xtsc::u32 size8, const xtsc::u8 *buffer, const char *dso_cookie, xtsc::u32 port);

  xtsc_request_if_impl        **m_request_impl;                 ///< m_request_exports binds to this (per mem port)
  xtsc_debug_if_cap           **m_debug_cap;                    ///< m_debug_ports binds to this if user can't (per mem port)
  xtsc::u32                     m_num_ports;                    ///< The number of memory ports this transactor has

  xtsc_memory_base             *m_p_memory;                     ///< Optional shadow memory
  HMODULE                       m_dso;                          ///< from dlopen/LoadLibrary of m_dso_name
  peek_t                        m_peek;                         ///< Function pointer to the peek symbol in m_dso
  poke_t                        m_poke;                         ///< Function pointer to the poke symbol in m_dso

  std::deque<xtsc_request*>    *m_request_fifo;                 ///< The fifo of incoming requests (per mem port)
  xtsc::u32                     m_request_fifo_depth;           ///< From "request_fifo_depth" parameter
  std::string                   m_interface_uc;                 ///< Uppercase version of "memory_interface" parameter
  memory_interface_type         m_interface_type;               ///< The memory interface type
  xtsc::u32                     m_width8;                       ///< Data width in bytes of the memory interface
  xtsc::xtsc_address            m_start_byte_address;           ///< Number to be subtracted from the TLM request address
  xtsc::u64                     m_clock_period_value;           ///< This device's clock period as u64
  sc_core::sc_time              m_clock_period;                 ///< This device's clock period as sc_time
  sc_core::sc_time              m_time_resolution;              ///< The SystemC time resolution
  sc_core::sc_time              m_posedge_offset;               ///< From "posedge_offset" parameter
  sc_core::sc_time              m_sample_phase;                 ///< Clock phase for sampling inputs and deasserting outputs
  sc_core::sc_time              m_sample_phase_plus_one;        ///< m_sample_phase plus one clock period
  sc_core::sc_time              m_output_delay;                 ///< See "output_delay" parameter
  sc_core::sc_time             *m_retire_deassert;              ///< Time at which XLMI retire should be deasserted (per mem port)
  sc_core::sc_time             *m_flush_deassert;               ///< Time at which XLMI flush should be deasserted (per mem port)
  sc_core::sc_time              m_read_delay_time;              ///< See "read_delay" parameter
  xtsc::u32                     m_read_delay;                   ///< See "read_delay" parameter
  xtsc::u64                     m_posedge_offset_value;         ///< m_posedge_offset as u64
  bool                          m_has_posedge_offset;           ///< True if m_posedge_offset is non-zero
  bool                          m_cosim;                        ///< See "cosim" parameter
  bool                          m_shadow_memory;                ///< See "shadow_memory" parameter
  bool                          m_test_shadow_memory;
  bool                          m_cbox;                         ///< See "cbox" parameter
  bool                          m_append_id;                    ///< True if pin port names should include the set_id.
  bool                          m_inbound_pif;                  ///< True if interface is inbound PIF
  bool                          m_snoop;                        ///< True if interface is snoop port
  bool                          m_has_coherence;                ///< See "has_coherence" parameter
  bool                          m_has_pif_attribute;            ///< See "has_pif_attribute" parameter
  bool                          m_big_endian;                   ///< True if the memory interface master is big endian
  bool                          m_write_responses;              ///< True if pin-level write responses will be received (PIF only)
  bool                          m_has_request_id;               ///< True if the "POReqId" and "PIRespId" ports should be present
  std::string                   m_dso_name;                     ///< See "dso_name"
  const char                   *m_dso_cookie;                   ///< See "dso_cookie"
  const char                   *m_initial_value_file;           ///< See "initial_value_file"
  xtsc::u8                      m_memory_fill_byte;             ///< See "memory_fill_byte" parameter
  xtsc::u32                    *m_current_id;                   ///< Used when m_has_request_id is false (per mem port)
  xtsc::u32                     m_address_bits;                 ///< Number of bits in the address (non-PIF only)
  xtsc::u32                     m_check_bits;                   ///< Number of bits in ECC/parity signals (from "check_bits")
  xtsc::xtsc_address            m_address_mask;                 ///< Address mask
  xtsc::xtsc_address            m_bus_addr_bits_mask;           ///< Address mask to get bits which indicate which byte lane
  xtsc::u32                     m_address_shift;                ///< Number of bits to right-shift the address
  xtsc::u32                     m_route_id_bits;                ///< Number of bits in the route ID (PIF only)
  xtsc::u32                     m_next_port_pif_request_thread; ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_response_thread;///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_drive_resp_rdy_thread;  ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_request_thread; ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_xlmi_retire_thread; ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_xlmi_flush_thread;  ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_xlmi_load_thread;   ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_dram_lock_thread;   ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_busy_write_rsp_thread;  ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_sample_read_data_thread;///< To give each thread instance a port number
  bool                          m_has_busy;                     ///< True if memory interface has a busy pin (non-PIF only)
  bool                          m_has_lock;                     ///< True if memory interface has a lock pin (DRAM0|DRAM1 only)
  bool                          m_has_xfer_en;                  ///< True if memory interface has Xfer enable pin (NA PIF|DROM0|XLMI0)
  sc_core::sc_event            *m_write_response_event;         ///< Event used to notify pif_response_thread (per mem port)
  sc_core::sc_event            *m_request_event;                ///< Event used to notify request_thread (per mem port)
  sc_core::sc_event            *m_retire_event;                 ///< Event used to notify xlmi_retire_thread (per mem port)
  sc_core::sc_event            *m_flush_event;                  ///< Event used to notify xlmi_flush_thread (per mem port)
  sc_dt::sc_uint_base           m_address;                      ///< The address after any required shifting and masking
  sc_dt::sc_uint_base           m_vadrs;                        ///< SnoopReqCohVadrsIndex
  sc_dt::sc_uint_base           m_req_coh_cntl;                 ///< POReqCohCntl;
  sc_dt::sc_uint_base           m_lane;                         ///< Byte/Word enables
  sc_dt::sc_uint_base           m_id;                           ///< POReqId/PIRespId
  sc_dt::sc_uint_base           m_priority;                     ///< POReqPriority/PIRespPriority
  sc_dt::sc_uint_base           m_route_id;                     ///< POReqRouteId/PIRespRouteId
  sc_dt::sc_uint_base           m_req_attribute;                ///< POReqAttribute/PIReqAttribute
  sc_dt::sc_bv_base             m_data;                         ///< Read/write data
  req_cntl                      m_req_cntl;                     ///< Value for POReqCntrl
  bool_fifo                   **m_resp_rdy_fifo;                ///< sc_fifo to keep track of PORespRdy pin (per mem port)
  sc_core::sc_event            *m_drive_resp_rdy_event;         ///< Notify when PORespRdy should be driven (per mem port)
  std::vector<xtsc_request*>    m_request_pool;                 ///< Maintain a pool of requests to improve performance
  std::vector<response_info*>   m_response_pool;                ///< Maintain a pool of responses to improve performance
  std::deque<response_info*>   *m_busy_write_rsp_deque;         ///< pending responses: check busy and/or send write rsp (per mem port)
  std::deque<response_info*>   *m_read_data_rsp_deque;          ///< deque of pending read responses (per mem port)
  std::deque<response_info*>   *m_pif_response_deque;           ///< deque of pending PIF responses (per mem port)
  std::deque<response_info*>   *m_write_response_deque;         ///< deque of pending generated PIF write responses (per mem port)
  std::deque<bool>             *m_lock_deque;                   ///< deque of pending DRamLock values (per mem port)
  std::deque<bool>             *m_load_deque;                   ///< deque of pending DPortLoad/m_p_preload values (per mem port)
  bool                         *m_previous_response_last;       ///< true if previous response was a last transfer (per mem port)
  sc_core::sc_event_queue      *m_busy_write_rsp_event_queue;   ///< When busy should be sampled and/or write rsp sent (per mem port)
  sc_core::sc_event_queue      *m_read_data_event_queue;        ///< When read data should be sampled (per mem port)
  sc_core::sc_event_queue      *m_lock_event_queue;             ///< When DRamLock should be driven (per mem port)
  sc_core::sc_event_queue      *m_load_event_queue;             ///< When DPortLoad/m_p_preload should be driven (per mem port)
  log4xtensa::TextLogger&       m_text;                         ///< Used for logging 
  bool_signal                 **m_p_preload;                    ///< DPortLoad prior to qualification with the busy pin

  // Local Memory pins (per mem port)
  bool_output                 **m_p_en;                         ///< DPortEn, DRamEn, DRomEn, IRamEn, IRomEn
  uint_output                 **m_p_addr;                       ///< DPortAddr, DRamAddr, DRomAddr, IRamAddr, IRomAddr
  uint_output                 **m_p_lane;                       ///< DPortByteEn, DRamByteEn, DRomByteEn, IRamWordEn, IRomWordEn
  wide_output                 **m_p_wrdata;                     ///< DPortWrData, DRamWrData, IRamWrData
  bool_output                 **m_p_wr;                         ///< DPortWr, DRamWr, IRamWr
  bool_output                 **m_p_load;                       ///< DPortLoad, IRamLoadStore, IRomLoad
  bool_output                 **m_p_retire;                     ///< DPortLoadRetired
  bool_output                 **m_p_flush;                      ///< DPortRetireFlush
  bool_output                 **m_p_lock;                       ///< DRamLock
  wide_output                 **m_p_check_wr;                   ///< DRamCheckWrData, IRamCheckWrData
  wide_input                  **m_p_check;                      ///< DRamCheckData, IRamCheckData
  bool_output                 **m_p_xfer_en;                    ///< DRamXferEn, IRamXferEn, IRomXferEn, URamXferEn
  bool_input                  **m_p_busy;                       ///< DPortBusy, DRamBusy, DRomBusy, IRamBusy, IRomBusy
  wide_input                  **m_p_data;                       ///< DPortData, DRamData, DRomData, IRamData, IRomData
  
  // PIF/inbound PIF/Snoop request channel pins (per mem port)
  bool_output                 **m_p_req_valid;                  ///< POReqValid
  uint_output                 **m_p_req_cntl;                   ///< POReqCntl
  uint_output                 **m_p_req_adrs;                   ///< POReqAdrs
  wide_output                 **m_p_req_data;                   ///< POReqData
  uint_output                 **m_p_req_data_be;                ///< POReqDataBE
  uint_output                 **m_p_req_id;                     ///< POReqId
  uint_output                 **m_p_req_priority;               ///< POReqPriority
  uint_output                 **m_p_req_route_id;               ///< POReqRouteId
  uint_output                 **m_p_req_attribute;              ///< POReqAttribute
  uint_output                 **m_p_req_coh_vadrs;              ///< POReqCohVAdrsIndex/SnoopReqCohVAdrsIndex
  uint_output                 **m_p_req_coh_cntl;               ///< POReqCohCntl
  bool_input                  **m_p_req_rdy;                    ///< PIReqRdy
  
  // PIF/inbound PIF/Snoop response channel pins (per mem port)
  bool_input                  **m_p_resp_valid;                 ///< PIRespValid
  uint_input                  **m_p_resp_cntl;                  ///< PIRespCntl
  wide_input                  **m_p_resp_data;                  ///< PORespData
  uint_input                  **m_p_resp_id;                    ///< PIRespId
  uint_input                  **m_p_resp_priority;              ///< PIRespPriority
  uint_input                  **m_p_resp_route_id;              ///< PIRespRouteId
  uint_input                  **m_p_resp_coh_cntl;              ///< PIRespCohCntl
  bool_output                 **m_p_resp_rdy;                   ///< PORespRdy

  response_info*              (*m_response_tab)[m_num_ids];     ///< Storage for outstanding responses (per mem port)

};  // class xtsc_tlm2pin_memory_transactor 






}  // namespace xtsc_component



#endif // _XTSC_TLM2PIN_MEMORY_TRANSACTOR_H_

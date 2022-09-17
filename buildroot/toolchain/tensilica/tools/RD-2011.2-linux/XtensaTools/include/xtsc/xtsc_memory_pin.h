#ifndef _XTSC_MEMORY_PIN_H_
#define _XTSC_MEMORY_PIN_H_

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


#include <map>
#include <vector>
#include <deque>
#include <string>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_module_pin_base.h>
#include <xtsc/xtsc_memory_base.h>
#include <xtsc/xtsc_request_if.h>



namespace xtsc {
// Forward references
class xtsc_core;
};



namespace xtsc_component {

// Forward references
class xtsc_tlm2pin_memory_transactor;

/**
 * Constructor parameters for a xtsc_memory_pin object.
 *
 * This class contains the constructor parameters for an xtsc_memory_pin object.
 *
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------------

   "memory_interface"   char*   The memory interface type.  Valid values are "DRAM0",
                                "DRAM1", "DROM0", "IRAM0", "IRAM1", "IROM0", "URAM0",
                                "XLMI0", and "PIF" (case-insensitive).
                                Note: For inbound PIF set this parameter to "PIF" and
                                set the "inbound_pif" parameter to true.
    
   "inbound_pif"        bool    Set to true for inbound PIF.  Set to false for outbound
                                PIF.  This parameter is ignored if "memory_interface"
                                is other then "PIF".
                                Default = false (outbound PIF).

   "num_ports"          u32     The number of memory ports this memory has.  A value of
                                1 means this memory is single-ported, a value of 2 means
                                this memory is dual-ported, etc.
                                Default = 1.
                                Minimum = 1.

   "port_name_suffix"   char*   Optional constant suffix to be appended to every input
                                and output port name.
                                Default = "".

   "byte_width"         u32     Memory data interface width in bytes.  Valid values for
                                "DRAM0", "DRAM1", "DROM0", "URAM0", and "XLMI0" are 4,
                                8, 16, 32, and 64.  Valid values for "IRAM0", "IRAM1",
                                "IROM0", and "PIF" are 4, 8, and 16.

   "start_byte_address" u32     The starting byte address of this memory in the 4GB
                                address space.
  
   "memory_byte_size"   u32     The byte size of this memory.  0 means the memory
                                occupies all of the 4GB address space at and above
                                "start_byte_address".
  
   "page_byte_size"     u32     The byte size of a page of memory.  In the model,
                                memory is not allocated until it is accessed.  This
                                parameter specifies the allocation size.
                                Default is 16 Kilobytes (1024*16=16384=0x4000).
                                Minimum page size is 16*byte_width.

   "initial_value_file" char*   If not NULL or empty, this names a text file from which
                                to read the initial memory contents as byte values.
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
                                Default = 0.

   "use_fast_access"    bool    If true, this memory will support fast access for the
                                turboxim simulation engine.
                                Default = true.

   "big_endian"         bool    True if the master is big endian.
                                Default = false.

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
                                period at which input pins are sampled.  Outputs which
                                are used for handshaking (PIReqRdy, PIRespValid,
                                IRamBusy, DRamBusy, etc.) are also sampled at this time.
                                This value is expressed in terms of the SystemC time
                                resolution (from sc_get_time_resolution()) and must be
                                strictly less than the clock period as specified by the
                                "clock_period" parameter.  A value of 0 means input pins
                                are sampled on posedge clock as specified by 
                                "posedge_offset".
                                Default = 0 (sample at posedge clock).

   "drive_phase"        u32     This specifies the phase (i.e. the point) in a clock
                                period at which output pins are driven.  It is expressed
                                in terms of the SystemC time resolution (from
                                sc_get_time_resolution()) and must be strictly less than
                                the clock period as specified by the "clock_period"
                                parameter.  A value of 0 means output pins are driven on
                                posedge clock as specified by "posedge_offset".
                                Default = 1 (1 time resolution after posedge clock).

   "vcd_handle"         void*   Pointer to SystemC VCD object (sc_trace_file *) or
                                0 if tracing is not desired.
                                Default = 0.
                                
   "busy_percentage"    u32     This specifies the percentage of requests that will
                                receive a busy/not-ready response.  This is for testing
                                purposes.  Valid values are 0-100.  Not applicable for
                                local memories with "has_busy" of false.
                                Default = 0.


   Parameters which apply to PIF only:

   "has_pif_attribute"  bool    True if the "POReqAttribute" or "PIReqAttribute" port
                                should be present.  This parameter is ignored unless
                                "memory_interface" is "PIF".
                                Default = false.

   "has_request_id"     bool    True if the "POReqId" and "PIRespId" ports should be
                                present.  This parameter is ignored unless
                                "memory_interface" is "PIF".
                                Default = true.

   "write_responses"    bool    True if write responses should be sent to the master.  
                                If false, write responses will not be sent.  This
                                parameter is ignored unless "memory_interface" is "PIF".
                                Default = true.

   "route_id_bits"      u32     Number of bits in the route ID.  Valid values are 0-32.
                                If "route_id_bits" is 0, then the "POReqRouteId" and
                                "PIRespRouteId" output ports will not be present.  This
                                parameter is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "void_resp_cntl"     u32     The low byte specifies the value to be driven out the
                                "PIRespCntl" port when "PIRespValid" is low (that is
                                when no response is occurring).  A default value of 0xFF
                                is used (instead of 0x00 which is the encoding for
                                "Response, not last transfer") to make it easier to
                                visually distinguish when a response is occuring when
                                using a waveform viewer.  This parameter is ignored
                                unless "memory_interface" is "PIF".
                                Default = 0xFF.

   "request_fifo_depth" u32     The request fifo depth.  This parameter is ignored
                                unless "memory_interface" is "PIF".
                                Default = 2.
                                Minimum = 1.

   "read_delay"         u32     The number of clock periods between starting to process
                                a READ request and sending the response.  See below for
                                the meaning of this parameter when "memory_interface" is
                                not "PIF".
                                Default = 0.

   "block_read_delay"   u32     The number of clock periods between starting to process
                                a BLOCK_READ request and sending the first response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 0.

   "block_read_repeat"  u32     Number of clock periods between each BLOCK_READ response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 1.

   "burst_read_delay"   u32     The number of clock periods between starting to process
                                a BURST_READ request and sending the first response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 0.

   "burst_read_repeat"  u32     Number of clock periods between each BURST_READ response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 1.

   "rcw_repeat"         u32     The minimum number of clock periods between starting to
                                process the first RCW request and starting to process
                                the second RCW request.  This parameter is ignored
                                unless "memory_interface" is "PIF".
                                Default = 1.

   "rcw_response"       u32     The number of clock periods between starting to process
                                the second RCW request and sending the response.  This
                                parameter is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "write_delay"        u32     The number of clock periods between starting to process
                                a WRITE request and sending the response.  This
                                parameter is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "block_write_delay"  u32     The minimum number of clock periods between starting to
                                process the first BLOCK_WRITE request and starting to
                                process the second BLOCK_WRITE request.  This parameter
                                is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "block_write_repeat" u32     The minimum number of clock periods between starting to
                                process a BLOCK_WRITE request (except the first or last
                                one) and starting to process the following BLOCK_WRITE
                                request.  This parameter is ignored unless
                                "memory_interface" is "PIF".
                                Default = 0.

   "block_write_response" u32   The number of clock periods between starting to process
                                the last BLOCK_WRITE request and sending the response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 0.

   "burst_write_delay"  u32     The minimum number of clock periods between starting to
                                process the first BURST_WRITE request and starting to
                                process the second BURST_WRITE request.  This parameter
                                is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "burst_write_repeat" u32     The minimum number of clock periods between starting to
                                process a BURST_WRITE request (except the first or last
                                one) and starting to process the following BURST_WRITE
                                request.  This parameter is ignored unless
                                "memory_interface" is "PIF".
                                Default = 0.

   "burst_write_response" u32   The number of clock periods between starting to process
                                the last BURST_WRITE request and sending the response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 0.


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
                                driven.  This should be 0 for a 5-stage pipeline and 1
                                for a 7-stage pipeline.  See above for the meaning of
                                this parameter when "memory_interface" is "PIF".
                                Default = 0.

   "cbox"               bool    True if this memory interface is driven from an Xtensa
                                CBOX.  This parameter is ignored if "memory_interface"
                                is other then "DRAM0"|"DRAM1"|"DROM0".
                                Default = false.
    \endverbatim
 *
 * @see xtsc_memory_pin
 * @see xtsc::xtsc_parms
 * @see xtsc::xtsc_initialize_parms
 */
class XTSC_COMP_API xtsc_memory_pin_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for an xtsc_memory_pin_parms object.
   *
   * @param memory_interface    The memory interface type.  Valid values are "DRAM0",
   *                            "DRAM1", "DROM0", "IRAM0", "IRAM1", "IROM0", "URAM0",
   *                            "XLMI0", and "PIF" (case-insensitive).
   *
   * @param byte_width          Memory data interface width in bytes.
   *
   * @param address_bits        Number of bits in address.  Ignored for "PIF".
   *
   * @param delay               Default delay for PIF read and write in terms of this
   *                            memory's clock period (see "clock_period").  For non-PIF
   *                            devices, this parameter defines the value of the
   *                            "read_delay" parameter.  Non-PIF memory devices should
   *                            use a delay of 0 for a 5-stage pipeline and a delay of 1
   *                            for a 7-stage pipeline.  
   *
   * @param num_ports           The number of ports this memory has.
   */
  xtsc_memory_pin_parms(const char     *memory_interface = "PIF",
                        xtsc::u32       byte_width       = 4,
                        xtsc::u32       address_bits     = 32,
                        xtsc::u32       delay            = 1,
                        xtsc::u32       num_ports        = 1)
  {
    init(memory_interface, byte_width, address_bits, delay, num_ports);
  }


  /**
   * Constructor for an xtsc_memory_pin_parms object based upon an xtsc_core
   * object and a named memory interface. 
   *
   * This constructor will determine "clock_period", "has_busy", "has_lock", "cbox",
   * "start_byte_address", "memory_byte_size", "address_bits", "byte_width", 
   * "big_endian", "check_bits", and "has_pif_attribute" by querying the core object.
   *
   * If desired, after the xtsc_memory_pin_parms object is constructed, its data
   * members can be changed using the appropriate xtsc_parms::set() method before
   * passing it to the xtsc_memory_pin constructor.
   *
   * @param     core                    A reference to the xtsc_core object upon which
   *                                    to base the xtsc_memory_pin_parms.
   *
   * @param     memory_interface        The memory interface name.
   *
   * @param     delay                   Default delay for PIF read and write in terms of
   *                                    this memory's clock period (see "clock_period").
   *                                    For non-PIF interfaces this parameter specifies
   *                                    the value of "read_delay".  A value of
   *                                    0xFFFFFFFF (the default) means to use a delay of
   *                                    0 if the core has a 5-stage pipeline and a delay
   *                                    of 1 if the core has a 7-stage pipeline.
   *
   * @param     num_ports               The number of ports this memory has.  If 0, the
   *                                    default, the number of ports (1 or 2) will be
   *                                    inferred thusly: If memory_interface is a LD/ST
   *                                    unit 0 port of a dual-ported core interface, and
   *                                    the core is dual-ported and has no CBox, and if
   *                                    the 2nd port of the core has not been bound,
   *                                    then "num_ports" will be 2; otherwise,
   *                                    "num_ports" will be 1.
   *
   */
  xtsc_memory_pin_parms(const xtsc::xtsc_core&  core,
                        const char             *memory_interface,
                        xtsc::u32               delay            = 0xFFFFFFFF,
                        xtsc::u32               num_ports        = 0);


  // Perform initialization common to both constructors
  void init(const char *memory_interface, xtsc::u32 byte_width, xtsc::u32 address_bits, xtsc::u32 delay, xtsc::u32 num_ports) {
    add("memory_interface",     memory_interface);
    add("inbound_pif",          false);
    add("has_pif_attribute",    false);
    add("byte_width",           byte_width);
    add("start_byte_address",   0);
    add("memory_byte_size",     0);
    add("page_byte_size",       1024*16);
    add("initial_value_file",   (char*)NULL);
    add("memory_fill_byte",     0);
    add("use_fast_access",      true);
    add("num_ports",            num_ports);
    add("port_name_suffix",     "");
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("read_delay",           delay);
    add("block_read_delay",     delay);
    add("block_read_repeat",    1);
    add("burst_read_delay",     delay);
    add("burst_read_repeat",    1);
    add("rcw_repeat",           1);
    add("rcw_response",         delay);
    add("write_delay",          delay);
    add("block_write_delay",    delay);
    add("block_write_repeat",   delay);
    add("block_write_response", delay);
    add("burst_write_delay",    delay);
    add("burst_write_repeat",   delay);
    add("burst_write_response", delay);
    add("sample_phase",         0);
    add("drive_phase",          1);
    add("big_endian",           false);
    add("has_request_id",       true);
    add("write_responses",      true);
    add("address_bits",         address_bits);
    add("route_id_bits",        0);
    add("void_resp_cntl",       0xFF);
    add("request_fifo_depth",   2);
    add("has_busy",             true);
    add("has_lock",             false);
    add("has_xfer_en",          false);
    add("busy_percentage",      0);
    add("vcd_handle",           (void*)NULL);
    add("cbox",                 false);
    add("check_bits",           0);
  }


  /// Our C++ type (the xtsc_parms base class uses this for error messages)
  virtual const char* kind() const { return "xtsc_memory_pin_parms"; }

};




/**
 * This device implements a pin-level memory model.  It can be driven by any of the XTSC
 * TLM memory interface master modules by using an xtsc_tlm2pin_memory_transactor to
 * convert the TLM requests/responses into pin-level requests/responses.  In addition,
 * when doing SystemC-Verilog cosimulation, this model can be driven by any Verilog
 * module which supports the configured Xtensa memory interface and protocol.
 *
 * This model supports any of the Xtensa memory interfaces (except caches) and it
 * supports operating as a multi-ported memory.  It also supports generating a random
 * busy/not-ready signal for testing purposes.
 *
 * The SystemC names of the pin-level ports exactly match the pin names of the Xtensa
 * RTL.  For example, an Xtensa core with a PIF has an output pin called "POReqValid"
 * and, when it is serving as a PIF memory, this module has a matching SystemC input
 * port which is also called "POReqValid" (see data member m_p_req_valid).
 *
 * Note: The parity/ECC signals (DRamNCheckDataM, DRamNCheckWrDataM, IRamNCheckData, and
 * IRamNCheckWrData) are present for IRAM and DRAM interfaces when "check_bits" is
 * non-zero; however, the input signal is ignored and the output signal is driven with
 * constant 0.
 *
 * Here is a block diagram of an xtsc_memory_pin as it is used in the
 * xtsc_tlm2pin_memory_transactor example:
 * @image html  Example_xtsc_tlm2pin_memory_transactor.jpg
 * @image latex Example_xtsc_tlm2pin_memory_transactor.eps "xtsc_tlm2pin_memory_transactor Example" width=13cm
 *
 * @see xtsc_memory_pin_parms
 * @see xtsc_memory_base
 * @see xtsc_tlm2pin_memory_transactor
 * @see xtsc::xtsc_core
 *
 */
class XTSC_COMP_API xtsc_memory_pin : public sc_core::sc_module, public xtsc_module_pin_base, public xtsc::xtsc_resettable {
protected:
  class pif_req_info;
  friend class pif_req_info;

public:


  sc_core::sc_export<xtsc::xtsc_debug_if>     **m_debug_exports;        ///< From master to us (per mem port)


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
  typedef sc_core::sc_fifo<bool>                        bool_fifo;
  typedef sc_core::sc_fifo<sc_dt::sc_bv_base>           wide_fifo;
  typedef sc_core::sc_signal<bool>                      bool_signal;
  typedef sc_core::sc_signal<sc_dt::sc_uint_base>       uint_signal;
  typedef sc_core::sc_signal<sc_dt::sc_bv_base>         wide_signal;
  typedef std::map<std::string, bool_signal*>           map_bool_signal;
  typedef std::map<std::string, uint_signal*>           map_uint_signal;
  typedef std::map<std::string, wide_signal*>           map_wide_signal;


  /// This SystemC macro inserts some code required for SC_THREAD's to work
  SC_HAS_PROCESS(xtsc_memory_pin);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_memory_pin"; }


  /**
   * Constructor for a xtsc_memory_pin.
   *
   * @param     module_name     Name of the xtsc_memory_pin sc_module.
   * @param     memory_parms    The remaining parameters for construction.
   *
   * @see xtsc_memory_pin_parms
   */
  xtsc_memory_pin(sc_core::sc_module_name module_name, const xtsc_memory_pin_parms& memory_parms);


  /// Destructor.
  ~xtsc_memory_pin(void);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


  /**
   * Non-hardware reads (for example, reads by the debugger).
   * @see xtsc::xtsc_request_if::nb_peek
   */
  void peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer) { m_p_memory->peek(address8, size8, buffer); }


  /**
   * Non-hardware writes (for example, writes from the debugger).
   * @see xtsc::xtsc_request_if::nb_poke
   */
  void poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer) { m_p_memory->poke(address8, size8, buffer); }


  /**
   * This method dumps the specified number of bytes from the memory.  Each
   * line of output is divided into three columnar sections, each of which is
   * optional.  The first section contains an address.  The second section contains
   * a hex dump of some (possibly all) of the data (two hex nibbles and a space for
   * each byte from the memory).  The third section contains an ASCII dump of the 
   * same data.
   *
   * @param       address8                The starting byte address in memory.
   *                                      
   * @param       size8                   The number of bytes of data to dump.
   *
   * @param       os                      The ostream object to which the data is to be
   *                                      dumped.
   *
   * @param       left_to_right           If true, the data is dumped in the order:
   *                                      memory[0], memory[1], ..., memory[bytes_per_line-1].
   *                                      If false, the data is dumped in the order:
   *                                      memory[bytes_per_line-1], memory[bytes_per_line-2], ..., memory[0].
   *
   * @param       bytes_per_line          The number of bytes to dump on each line of output.
   *                                      If bytes_per_line is 0 then all size8 bytes are dumped 
   *                                      on a single line with no newline at the end.  If 
   *                                      bytes_per_line is non-zero, then all lines of output
   *                                      end in newline.
   *
   * @param       show_address            If true, the first columnar section contains an 
   *                                      address printed as an 8-hex-digit number with a 0x 
   *                                      prefix.  If false, the first columnar section is null
   *                                      and takes no space in the output.
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
   * @param       adjust_address          If adjust_address is true and address8 modulo 
   *                                      bytes_per_line is not 0, then offset the
   *                                      printed values on the first line of the hex and 
   *                                      ASCII columnar sections and adjust the printed 
   *                                      address so that the printed address modulo 
   *                                      bytes_per_line is always zero.  Otherwize, do not
   *                                      offset the first printed data values and do not
   *                                      adjust the printed address.
   */
  void byte_dump(xtsc::xtsc_address     address8,
                 xtsc::u32              size8,
                 std::ostream&          os                      = std::cout,
                 bool                   left_to_right           = true,
                 xtsc::u32              bytes_per_line          = 16,
                 bool                   show_address            = true,
                 bool                   show_hex_values         = true,
                 bool                   do_column_heading       = true,
                 bool                   show_ascii_values       = true,
                 bool                   adjust_address          = true)
  {
    m_p_memory->byte_dump(address8, size8, os, left_to_right, bytes_per_line, show_address, show_hex_values, do_column_heading,
                           show_ascii_values, adjust_address);
  }


  /**
   * Connect an xtsc_tlm2pin_memory_transactor transactor to this xtsc_memory_pin.
   *
   * This method connects the pin-level ports of the xtsc_tlm2pin_memory_transactor to
   * this xtsc_memory_pin.  It also connects the debug interface unless the transactor
   * was configured with a "dso_name".  In the process of connecting the pin-level
   * ports, it creates the necessary signals of type  sc_signal<bool>,
   * sc_signal<sc_uint_base>, and sc_signal<sc_bv_base>.  The name of each signal is
   * formed by the concatenation of the SystemC name of the xtsc_memory_pin object, the
   * 2 characters "__", and the SystemC name of the xtsc_memory_pin port (for example,
   * "pif__POReqValid").
   *
   * @param     tlm2pin         The xtsc_tlm2pin_memory_transactor to connect to this
   *                            xtsc_memory_pin.
   *
   * @param     tlm2pin_port    The tlm2pin master port to connect to.
   *
   * @param     mem_port        The port of this memory to connect tlm2pin to.
   *
   * @param     single_connect  If true only one port of this memory will be connected.
   *                            If false, the default, then all contiguous, unconnected
   *                            port numbers of this memory starting at mem_port that
   *                            have a corresponding existing port in tlm2pin (starting
   *                            at tlm2pin_port) will be connected to that corresponding
   *                            port in tlm2pin.
   *
   * @returns number of ports that were connected by this call (1 or more)
   */
  xtsc::u32 connect(xtsc_tlm2pin_memory_transactor&     tlm2pin,
                    xtsc::u32                           tlm2pin_port = 0,
                    xtsc::u32                           mem_port = 0,
                    bool                                single_connect = false);


  virtual void reset(bool hard_reset = false);


  /// Return true if pin port names include the set_id as a suffix
  bool get_append_id() const { return m_append_id; }


protected:


  /// Helper method to get the tlm2pin port name and confirm pin sizes match
  std::string adjust_name_and_check_size(const std::string&                     port_name,
                                         const xtsc_tlm2pin_memory_transactor&  tlm2pin,
                                         xtsc::u32                              tlm2pin_port,
                                         const set_string&                      transactor_set) const;

  /// Dump a set of strings one string per line with the specified indent
  void dump_set_string(std::ostringstream& oss, const set_string& strings, const std::string& indent);


  /// SystemC callback
  virtual void end_of_elaboration(void);


  /// SystemC callback
  virtual void start_of_simulation(void);


  /// Sync to m_sample_phase (always waits)
  void sync_to_sample_phase(void);


  /// Sync to m_drive_phase (only waits if not already at m_drive_phase)
  void sync_to_drive_phase(void);


  /// Capture PIF request information
  void pif_request_thread(void);


  /// Drive POReqRdy at "drive_phase"
  void pif_drive_req_rdy_thread(void);


  /// Process PIF requests and drive response signals at "drive_phase"
  void pif_respond_thread(void);


  /// Handle local memory-interface requests
  void lcl_request_thread(void);


  /// Drive local memory read data
  void lcl_drive_read_data_thread(void);


  /// Drive local memory busy signal
  void lcl_drive_busy_thread(void);


  /// Helper method to handle READ.
  void do_read(const pif_req_info& info);


  /// Helper method to handle BLOCK_READ.
  void do_block_read(const pif_req_info& info);


  /// Helper method to handle BURST_READ.
  void do_burst_read(const pif_req_info& info);


  /// Helper method to handle RCW.
  void do_rcw(const pif_req_info& info);


  /// Helper method to handle WRITE.
  void do_write(const pif_req_info& info);


  /// Helper method to handle BLOCK_WRITE.
  void do_block_write(const pif_req_info& info);


  /// Helper method to handle BURST_WRITE.
  void do_burst_write(const pif_req_info& info);


  /// Drive response for durations of one clock cycle until accepted, then deassert
  void pif_drive_response(const pif_req_info& info, const resp_cntl& response, const sc_dt::sc_bv_base& data);


  /// Create an sc_signal<bool> with the specified name
  bool_signal& create_bool_signal(const std::string& signal_name);


  /// Create an sc_signal<sc_uint_base> with the specified name and size
  uint_signal& create_uint_signal(const std::string& signal_name, xtsc::u32 num_bits);


  /// Create an sc_signal<sc_bv_base> with the specified name and size
  wide_signal& create_wide_signal(const std::string& signal_name, xtsc::u32 num_bits);


  /// Swizzle byte enables 
  void swizzle_byte_enables(xtsc::xtsc_byte_enables& byte_enables) const;



  /// Implementation of xtsc_debug_if.
  class xtsc_debug_if_impl : public xtsc::xtsc_debug_if, public sc_core::sc_object {
  public:

    /// Constructor
    xtsc_debug_if_impl(const char *object_name, xtsc_memory_pin& memory_pin, xtsc::u32 port_num) :
      sc_object         (object_name),
      m_memory_pin      (memory_pin),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// The kind of sc_object we are
    const char* kind() const { return "xtsc_memory_pin::xtsc_debug_if_impl"; }

    /**
     *  Receive peeks from the master
     *  @see xtsc::xtsc_debug_if
     */
    void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /**
     *  Receive pokes from the master
     *  @see xtsc::xtsc_debug_if
     */
    void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /**
     *  Receive requests for fast access information from the master
     *  @see xtsc::xtsc_debug_if
     */
    bool nb_fast_access(xtsc::xtsc_fast_access_request &request);

    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }


  protected:

    /// SystemC callback when something binds to us
    void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_memory_pin&            m_memory_pin;   ///< Our xtsc_memory_pin object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
    xtsc::u32                   m_port_num;     ///< Our port number
  };


  /**
   * Information about each request.
   * Constructor and init() populate data members by reading the input pin values.
   */
  class pif_req_info {
  public:

    /// Constructor for a new pif_req_info
    pif_req_info(const xtsc_memory_pin& memory, xtsc::u32 port);

    /// Initialize an already existing pif_req_info object
    void init(xtsc::u32 port);

    /// Dump a pif_req_info object
    void dump(std::ostream& os) const;

    const xtsc_memory_pin&      m_memory;               ///< A reference to the owning xtsc_memory_pin
    xtsc::u32                   m_port;                 ///< The memory port this request was received on
    sc_core::sc_time            m_time_stamp;           ///< Time when request was received
    req_cntl                    m_req_cntl;             ///< POReqCntl
    xtsc::xtsc_address          m_address;              ///< POReqAdrs
    sc_dt::sc_bv_base           m_data;                 ///< POReqData
    xtsc::xtsc_byte_enables     m_byte_enables;         ///< POReqDataBE
    sc_dt::sc_uint_base         m_id;                   ///< POReqId
    sc_dt::sc_uint_base         m_priority;             ///< POReqPriority
    sc_dt::sc_uint_base         m_route_id;             ///< POReqRouteId
    xtsc::xtsc_byte_enables     m_fixed_byte_enables;   ///< POReqDataBE swizzled if m_big_endian
  };
  friend std::ostream& operator<<(std::ostream& os, const pif_req_info& info);


  /// Get a new pif_req_info (from the pool)
  pif_req_info *new_pif_req_info(xtsc::u32 port);


  /// Delete an pif_req_info (return it to the pool)
  void delete_pif_req_info(pif_req_info*& p_pif_req_info);



  xtsc_debug_if_impl          **m_debug_impl;                   ///< m_debug_exports binds to these (per mem port)
  xtsc_memory_base             *m_p_memory;                     ///< The memory itself

  std::deque<pif_req_info*>    *m_pif_req_fifo;                 ///< The fifo of incoming PIF requests (per mem port)
  xtsc::u32                     m_request_fifo_depth;           ///< From "request_fifo_depth" parameter
  std::vector<pif_req_info*>    m_pif_req_pool;                 ///< Pool of pif_req_info objects
  xtsc::u32                     m_num_ports;                    ///< The number of ports this memory has
  std::string                   m_interface_uc;                 ///< Uppercase version of "memory_interface" parameter
  memory_interface_type         m_interface_type;               ///< The memory interface type
  xtsc::u64                     m_clock_period_value;           ///< This device's clock period as u64
  sc_core::sc_time              m_clock_period;                 ///< This device's clock period as sc_time

  sc_core::sc_time              m_time_resolution;              ///< The SystemC time resolution
  sc_core::sc_time              m_posedge_offset;               ///< From "posedge_offset" parameter
  sc_core::sc_time              m_sample_phase;                 ///< Clock phase at which inputs are sampled (from "sample_phase")
  sc_core::sc_time              m_sample_phase_plus_one;        ///< m_sample_phase plus one clock period
  sc_core::sc_time              m_drive_phase;                  ///< Clock phase at which outputs are driven (from "drive_phase")
  sc_core::sc_time              m_drive_phase_plus_one;         ///< m_drive_phase plus one clock period
  sc_core::sc_time              m_drive_to_sample_time;         ///< Time from m_drive_phase to next sample phase (PIF)
  sc_core::sc_time              m_sample_to_drive_time;         ///< Time from next sample phase to next drive phase (PIF)
  sc_core::sc_time              m_sample_to_drive_data_delay;   ///< Time to wait from sampling inputs to driving data output (lcl mem)
  sc_core::sc_time              m_sample_to_drive_busy_delay;   ///< Time to wait from sampling inputs to driving busy output (lcl mem)
  bool                          m_has_posedge_offset;           ///< True if m_posedge_offset is non-zero
  xtsc::u64                     m_posedge_offset_value;         ///< m_posedge_offset as u64

  xtsc::u32                     m_read_delay_value;             ///< See "read_delay" parameter

  xtsc::u32                     m_next_port_lcl_request_thread; ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_drive_read_data_thread; ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_drive_busy_thread;      ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_request_thread; ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_drive_req_rdy_thread;   ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_respond_thread; ///< To give each thread instance a port number

  sc_core::sc_time              m_read_delay;                   ///< See "read_delay" parameter
  sc_core::sc_time              m_block_read_delay;             ///< See "block_read_delay" parameter
  sc_core::sc_time              m_block_read_repeat;            ///< See "block_read_repeat" parameter
  sc_core::sc_time              m_burst_read_delay;             ///< See "burst_read_delay" parameter
  sc_core::sc_time              m_burst_read_repeat;            ///< See "burst_read_repeat" parameter
  sc_core::sc_time              m_rcw_repeat;                   ///< See "rcw_repeat" parameter
  sc_core::sc_time              m_rcw_response;                 ///< See "rcw_response" parameter
  sc_core::sc_time              m_write_delay;                  ///< See "write_delay" parameter
  sc_core::sc_time              m_block_write_delay;            ///< See "block_write_delay" parameter
  sc_core::sc_time              m_block_write_repeat;           ///< See "block_write_repeat" parameter
  sc_core::sc_time              m_block_write_response;         ///< See "block_write_response" parameter
  sc_core::sc_time              m_burst_write_delay;            ///< See "burst_write_delay" parameter
  sc_core::sc_time              m_burst_write_repeat;           ///< See "burst_write_repeat" parameter
  sc_core::sc_time              m_burst_write_response;         ///< See "burst_write_response" parameter
  sc_core::sc_time             *m_last_action_time_stamp;       ///< Time of last action: recovery time starts from here (per mem port)

  bool                          m_cbox;                         ///< See "cbox" parameter
  bool                          m_append_id;                    ///< True if pin port names should include the set_id.
  bool                          m_inbound_pif;                  ///< True if interface is inbound PIF
  bool                          m_has_pif_attribute;            ///< See "has_pif_attribute" parameter
  bool                          m_use_fast_access;              ///< See "use_fast_access" parameter
  bool                          m_big_endian;                   ///< True if master is big endian
  bool                          m_has_request_id;               ///< True if the "POReqId" and "PIRespId" ports should be present
  bool                          m_write_responses;              ///< See "write_responses" parameter
  xtsc::u32                     m_address_bits;                 ///< Number of bits in the address (non-PIF only)
  xtsc::u32                     m_check_bits;                   ///< Number of bits in ECC/parity signals (from "check_bits")
  xtsc::u32                     m_setw;                         ///< Number of nibbles to use when displaying an address
  xtsc::u32                     m_address_shift;                ///< Number of bits to left-shift the address
  xtsc::u32                     m_address_mask;                 ///< To mask out unused bits of address
  xtsc::u32                     m_route_id_bits;                ///< Number of bits in the route ID (PIF only)
  bool                          m_has_busy;                     ///< True if memory interface has a busy pin (non-PIF only)
  bool                          m_has_lock;                     ///< True if memory interface has a lock pin (DRAM0|DRAM1 only)
  bool                          m_has_xfer_en;                  ///< True if memory interface has Xfer enable pin (NA PIF|DROM0|XLMI0)
  bool                         *m_testing_busy;                 ///< We're asserting PIReqRdy because of "busy_percentage"
  xtsc::i32                     m_busy_percentage;              ///< Percent of requests that will get a busy response
  sc_core::sc_event            *m_pif_req_event;                ///< Event used to notify pif_respond_thread (per mem port)
  sc_core::sc_event            *m_pif_req_rdy_event;            ///< Event used to notify pif_drive_req_rdy_thread (per mem port)
  sc_core::sc_event            *m_respond_event;                ///< Event used to notify pif_request_thread (per mem port)

  bool                         *m_first_block_write;            ///< True if next BLOCK_WRITE will be first in the block (per mem port)
  bool                         *m_first_burst_write;            ///< True if next BURST_WRITE will be first in the block (per mem port)

  xtsc::xtsc_address           *m_block_write_address;          ///< Address to be used for next BLOCK_WRITE (per mem port)
  xtsc::xtsc_address           *m_burst_write_address;          ///< Address to be used for next BURST_WRITE (per mem port)

  sc_dt::sc_uint_base           m_address;                      ///< The address after any required shifting and masking
  sc_dt::sc_bv_base           **m_data;                         ///< POReqData/PIRespData, also temp buffer for lcl memory reads
  sc_dt::sc_bv_base           **m_data_to_be_written;           ///< For delayed handling of lcl memory write data
  sc_dt::sc_bv_base           **m_rcw_compare_data;             ///< RCW compare data from 1st transfer of RCW (per mem port)

  sc_dt::sc_uint_base           m_id_zero;                      ///< For deasserting PIRespId
  sc_dt::sc_uint_base           m_priority_zero;                ///< For deasserting PIRespPriority
  sc_dt::sc_uint_base           m_route_id_zero;                ///< For deasserting PIRespRouteId
  sc_dt::sc_bv_base             m_data_zero;                    ///< For deasserting PIRespData
  resp_cntl                     m_resp_cntl_zero;               ///< For deasserting PIRespCntl

  req_cntl                      m_req_cntl;                     ///< Value from POReqCntl
  resp_cntl                     m_resp_cntl;                    ///< Value for PIRespCntl
  wide_fifo                   **m_read_data_fifo;               ///< sc_fifo of sc_bv_base read data values (lcl mem) (per mem port)
  bool_fifo                   **m_busy_fifo;                    ///< sc_fifo of busy values (lcl mem) (per mem port)
  sc_core::sc_event_queue     **m_read_event_queue;             ///< When read data should be driven (lcl mem) (per mem port)
  sc_core::sc_event_queue     **m_busy_event_queue;             ///< When busy should be driven (lcl mem) (per mem port)
  map_bool_signal               m_map_bool_signal;              ///< The optional map of all sc_signal<bool> signals
  map_uint_signal               m_map_uint_signal;              ///< The optional map of all sc_signal<sc_uint_base> signals
  map_wide_signal               m_map_wide_signal;              ///< The optional map of all sc_signal<sc_bv_base> signals

  // Local Memory pins
  bool_input                  **m_p_en;                         ///< DPortEn, DRamEn, DRomEn, IRamEn, IRomEn (per mem port)
  uint_input                  **m_p_addr;                       ///< DPortAddr, DRamAddr, DRomAddr, IRamAddr, IRomAddr (per mem port)
  uint_input                  **m_p_lane;                       ///< DPortByteEn, DRamByteEn, DRomByteEn, IRamWordEn, IRomWordEn (per mem port)
  wide_input                  **m_p_wrdata;                     ///< DPortWrData, DRamWrData, IRamWrData (per mem port)
  bool_input                  **m_p_wr;                         ///< DPortWr, DRamWr, IRamWr (per mem port)
  bool_input                  **m_p_load;                       ///< DPortLoad, IRamLoadStore, IRomLoad - Trace only (per mem port)
  bool_input                  **m_p_retire;                     ///< DPortLoadRetired - Trace only (per mem port)
  bool_input                  **m_p_flush;                      ///< DPortRetireFlush - Trace only (per mem port)
  bool_input                  **m_p_lock;                       ///< DRamLock (per mem port)
  wide_input                  **m_p_check_wr;                   ///< DRamCheckWrData, IRamCheckWrData
  wide_output                 **m_p_check;                      ///< DRamCheckData, IRamCheckData
  bool_input                  **m_p_xfer_en;                    ///< DRamXferEn, IRamXferEn, IRomXferEn, URamXferEn
  bool_output                 **m_p_busy;                       ///< DPortBusy, DRamBusy, DRomBusy, IRamBusy, IRomBusy (per mem port)
  wide_output                 **m_p_data;                       ///< DPortData, DRamData, DRomData, IRamData, IRomData (per mem port)
  
  // PIF request channel pins
  bool_input                  **m_p_req_valid;                  ///< POReqValid (per mem port)
  uint_input                  **m_p_req_cntl;                   ///< POReqCntl (per mem port)
  uint_input                  **m_p_req_adrs;                   ///< POReqAdrs (per mem port)
  wide_input                  **m_p_req_data;                   ///< POReqData (per mem port)
  uint_input                  **m_p_req_data_be;                ///< POReqDataBE (per mem port)
  uint_input                  **m_p_req_id;                     ///< POReqId (per mem port)
  uint_input                  **m_p_req_priority;               ///< POReqPriority (per mem port)
  uint_input                  **m_p_req_route_id;               ///< POReqRouteId (per mem port)
  uint_input                  **m_p_req_attribute;              ///< POReqAttribute
  bool_output                 **m_p_req_rdy;                    ///< PIReqRdy (per mem port)
  
  // PIF response channel pins
  bool_output                 **m_p_resp_valid;                 ///< PIRespValid (per mem port)
  uint_output                 **m_p_resp_cntl;                  ///< PIRespCntl (per mem port)
  wide_output                 **m_p_resp_data;                  ///< PORespData (per mem port)
  uint_output                 **m_p_resp_id;                    ///< PIRespId (per mem port)
  uint_output                 **m_p_resp_priority;              ///< PIRespPriority (per mem port)
  uint_output                 **m_p_resp_route_id;              ///< PIRespRouteId (per mem port)
  bool_input                  **m_p_resp_rdy;                   ///< PORespRdy (per mem port)

  xtsc::xtsc_address            m_start_address8;               ///< The starting byte address of this memory
  xtsc::u32                     m_size8;                        ///< The byte size of this memory
  xtsc::u32                     m_width8;                       ///< The byte width of this memories data interface
  xtsc::u32                     m_enable_bits;                  ///< Number of explicit/implied byte/word enable bits
  xtsc::xtsc_address            m_end_address8;                 ///< The ending byte address of this memory

  log4xtensa::TextLogger&       m_text;                         ///< Text logger
  log4xtensa::BinaryLogger&     m_binary;                       ///< Binary logger

};  // class xtsc_memory_pin 



}  // namespace xtsc_component



#endif // _XTSC_MEMORY_PIN_H_

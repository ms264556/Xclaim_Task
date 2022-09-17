#ifndef _XTSC_LOOKUP_H_
#define _XTSC_LOOKUP_H_

// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <xtsc/xtsc.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_lookup_if.h>
#include <string>
#include <vector>
#include <map>
#include <deque>




namespace xtsc {
class xtsc_core;
}


namespace xtsc_component {


class xtsc_lookup_driver;


/**
 * Constructor parameters for a xtsc_lookup object.
 *
 * This class contains the constructor parameters for a xtsc_lookup object.
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    -------------------------------------------------------
  
   "ram"                bool    If false, the default, then this module functions as a
                                lookup table and the entire TIE address signal (i.e.
                                the address argument of the nb_send_address() method)
                                functions as an index into the lookup table.  If true,
                                then this module functions as a RAM and the RAM address,
                                write data, and write strobe are bit-fields within the
                                TIE address signal.  In the latter case, the RAM address
                                size is defined to be the following (which must be
                                greater than 0):
                                     ("address_bit_width" - "data_bit_width" - 1)
                                Default = false (function as a lookup table).

   "address_bit_width"  u32     The width of the TIE address signal (i.e. request
                                address) in bits.  Maximum is 1024.

   "data_bit_width"     u32     The width of the response data in bits.  Maximum is
                                1024.

   "write_data_lsb"     u32     If "ram" is true, this parameter specifies the bit
                                location in the TIE address signal of the least-
                                significant bit of the write data field.  There are
                                exactly four legal values for this parameter:
                                  0
                                  1
                                  "address_bit_width" - "data_bit_width" - 1
                                  "address_bit_width" - "data_bit_width"
                                This parameter is ignored if "ram" is false.
                                Default = 0.

   "write_strobe_bit"   u32     If "ram" is true, this parameter specifies the bit
                                location in the TIE address signal of the write strobe.
                                A value of 0xFFFFFFFF means to use the highest bit
                                location within the TIE address signal that is
                                consistent with the "write_data_lsb" value.  This
                                parameter is ignored if "ram" is false.
                                Default = 0xFFFFFFFF (use highest possible location).

   "active_high_strobe" bool    If "ram" is true, this parameter specifies whether or
                                not the write strobe is active high.  A value of true,
                                the default, means the strobe is active high so that
                                when the value of the write strobe bit is 1 then the
                                lookup is a write and when the value is 0 the lookup is
                                a read.

   "ram_write_enables" vector<u32> If "ram" is true, this parameter may be used to
                                designate one or more bits of the RAM address as enable
                                bits for designated sub-fields of the write data bit
                                field.  Each enable bit can correspond to any arbitrary
                                sub-field of the write data bit field; however, any
                                given bit in the write data bit field can be controlled
                                by at most one enable bit.  If this parameter is used,
                                then it must contain a multiple of four u32 values.  The
                                first value of the quartet specifies the zero-based
                                index into the full RAM address of the enable bit being
                                specified by the quartet.  The second and third values
                                of the quartet specify the low and high (respectively)
                                bits of the sub-field in the write data bit field that
                                the enable bit controls.  The last value of the quartet
                                must be either 0 or 1 to specify the enable as active
                                low or high (respectively).  If desired, a given enable
                                bit may be given multiple times to allow it to control a
                                non-contiguous sub-field of the write data bit field.
                                Note: It is best if the enable bits can be placed at the
                                      high end of the RAM address.  This facilitates
                                      specifying the addresses in the lookup_table
                                      (initial value file) and reading the log file.
                                As an example, let's say we want a TIE lookup RAM that
                                is 16-bits wide and 256 entries deep.  Furthermore, lets
                                say we wish to be able to divide each 16-bit entry into
                                two byte lanes that can be individually enabled for
                                writing so that a given lookup event with the write
                                strobe enabled can do any one of the following four
                                actions depending on two enable bits within the full RAM
                                address:
                                1) write both byte lanes
                                2) write the first byte lane; leave the second unchanged
                                3) write the second byte lane; leave the first unchanged
                                4) leave both bytes unchanged
                                Because we want to address 256 entries we need 8 bits
                                and when we add the 2 enable bits, we get a full RAM
                                address of 10 bits.  Let's choose to place the write
                                data at the low end of the TIE lookup address, to use
                                the high two bits of the full RAM address as the
                                active-high enable bits, and to use the high bit of the
                                TIE lookup address as the active-high write strobe.
                                These choices can be realized by defining a TIE lookup
                                with a 27 bit output-width and a 16 bit input-width and
                                modelling the attached lookup RAM using an xtsc_lookup
                                with the following parameters and values:
                                "ram"                = true
                                "address_bit_width"  = 27
                                "data_bit_width"     = 16
                                "write_data_lsb"     = 0                (default)
                                "write_strobe_bit"   = 0xFFFFFFFF       (default)
                                "active_high_strobe" = true             (default)
                                "ram_write_enables"  = 8,0,7,1,9,8,15,1
                                The "ram_write_enables" parameter contains 8 values
                                which make two quartets.  The first quartet (8,0,7,1)
                                specifies that bit 8 of the full RAM address is an
                                active high-enable which controls whether or not bits
                                0-7 of the write data bit field are written.  The second
                                quartet (9,8,15,1) specifies that bit 9 of the full RAM
                                address is an active high-enable which controls whether
                                or not bits 8-15 of the write data bit field are
                                written.  So the layout of the 27-bit TIE lookup address
                                is:
                                  [15: 0] - write data
                                  [25:16] - full RAM address
                                  [24:24] - enable for write data[ 7: 0]
                                  [25:25] - enable for write data[15: 8]
                                  [26:26] - write strobe
                                Default = empty

   "has_ready"          bool    Specifies whether the xtsc_lookup has a ready signal.

   "pipeline_depth"     u32     This parameter specifies the depth of the pipeline.
                                When the pipeline is full and "has_ready" is true,
                                nb_is_ready() will return false.  If "has_ready" is
                                false and nb_send_address() is called when the pipeline
                                is full, an xtsc_exception will be thrown.  A value of
                                0 means to be fully pipelined (i.e. have a pipeline
                                depth equal to the latency+1).  See "latency" parameter.
                                Default = 0 (i.e. fully pipelined).

   "enforce_latency"    bool    If "enforce_latency" is true, checks will be performed
                                to ensure nb_get_data() is called during the correct
                                clock cycle.  See "latency" parameter.
                                If "enforce_latency" is false, the checks will not be
                                performed, and nb_get_data() will simply return the 
                                next data (from m_data_fifo).
                                Note: Use "enforce_latency" of false if operating in
                                turbo mode (regardless of the "has_ready" setting).
                                Default = true.

   "latency"            u32     The latency is defined by the <use_stage> and
                                <def_stage> values in the user TIE code defining the
                                TIE lookup interface according to the following formula:
                                  latency = <use_stage> - <def_stage>
                                If "enforce_latency" is true and "has_ready" is true,
                                "latency" specifies the clock cycle (relative to the
                                nb_is_ready() call that returned true) in which
                                nb_get_data() can be called without an exception being
                                thrown.
                                If "enforce_latency" is true and "has_ready" is false,
                                "latency" specifies the clock cycle (relative to the
                                nb_send_address() call) in which nb_get_data() can be
                                called without an exception being thrown.
                                If "enforce_latency" is false then the latency check in
                                nb_get_data() will not be performed and nb_get_data()
                                will simply return the next data (from m_data_fifo).
                                "latency" must be greater than 0.
                                Default = 1.

   "delay"              u32     If "has_ready" is true, "delay" specifies how many clock
                                cycles that the lookup device will not be ready for a
                                subsequent lookup after it is ready for the current
                                lookup.  If "delay" is 0, then nb_is_ready() will always
                                return true (unless the pipeline is full).  The value
                                specified by "delay" can be overriden on a per-address
                                basis by using the optional @<Delay> field in the lookup
                                table specified by the "lookup_table" parameter.  The
                                purpose of this parameter is to allow a limited ability
                                to model the lookup device being busy (for example,
                                because it is shared with another Xtensa or because it
                                is having its values reloaded).  If "has_ready" is
                                false, "delay" must be 0.
                                Default = 0.
   
   Note:  The interpretation of the "delay" parameter changed after (not including) RC-2010.1
          to better model recommended TIE lookup implementation behavior.

   "lookup_table"       char*   The name of the file containing the address-data pairs
                                and/or just the data (with an implied address).  Each
                                address-data pair can also have an optional delay
                                specified (which is only used if "ram" is false and
                                "has_ready" is true).  If "ram" is false and this
                                parameter is NULL, then all lookups will return the
                                "default_data" value (unless a sub-class overrides the
                                xtsc_lookup::get_data_from_address() method).  If "ram"
                                is true, the file acts as an initial value file.  If
                                "ram" is true and this parameter is NULL, then all reads
                                to RAM addresses that have not been written will return
                                the "default_data" value. This file must contain lines
                                in the following format:

                                [<Address>] <Value> [@<Delay>]
                                
                                1.  Each line of the text file contains one or two
                                    numbers in decimal or hexadecimal (using '0x'
                                    prefix) format followed by an optional delay
                                    value which must start with an @.
                                    For example,

                                       0x12345678        // Implied address of 0x0
                                       0xbabeface        // Implied address of 0x1
                                       0x99  0x11111111  // Explicit address of 0x99
                                       16    0x11111111  // Explicit address of 0x10
                                       0x22222222        // Implied address of 0x11
                                       18 0x33333333 @3  // Delay m_ready 3 clock cycles
                             
                                2.  If a line contains two numbers (excluding the
                                    optional delay), the first number is interpreted as
                                    an address and the second number is interpreted as
                                    the data corresponding to the address.
                                3.  If a line only contains one number (excluding the
                                    optional delay),then it is interpreted as the data
                                    corresponding to an address which is one greater
                                    than the address of the previous line.  If the first
                                    line of the file contains one number, then the
                                    implied address is 0.
                                4.  Numbers can contain up to 1024 bits (256 hex 
                                    nibbles).
                                5.  An address can occur only once in the file (either 
                                    explicitly or implied).  A data value can occur as
                                    often as desired.
                                6.  An optional integer delay value can be specified to
                                    override the "delay" parameter for this lookup
                                    address only.  The @<Delay> entries only take effect
                                    if "has_ready" is true.
                                7.  Comments, extra whitespace, and blank lines are
                                    ignored.  See xtsc::xtsc_script_file.

   "default_data"       char*   C-string containing the default data.  If "ram" is
                                false, this value will be returned for any lookup
                                address not specified (explicitly or implicitly) in
                                "lookup_table".  If "ram" is true, this value will be
                                returned for any RAM address not specified in
                                "lookup_table" that has not been written.
                                Default = "0x0".

   "override_lookup"    bool    If true, then the get_data_from_address() virtual method
                                will be called to do the lookup.  If "override_lookup"
                                is true then the get_data_from_address() virtual method
                                must be overriden by a subclass.
                                Default = false.

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

   "vcd_handle"         void*   Pointer to SystemC VCD object (sc_trace_file *) or 0 if
                                tracing is not desired.
                                Default = 0 (no tracing).

    \endverbatim
 *
 * @see xtsc_lookup
 * @see xtsc::xtsc_parms
 */
class XTSC_COMP_API xtsc_lookup_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for an xtsc_lookup_parms object.
   *
   * @param     address_bit_width       The width of the request address in bits.
   *
   * @param     data_bit_width          The width of the response data in bits.
   *
   * @param     has_ready               Specifies whether or not the xtsc_lookup has a
   *                                    ready signal (corresponds to the rdy keyword in
   *                                    the user's TIE code for the lookup).
   *
   * @param     lookup_table            The name of the file containing the address-data
   *                                    pairs and/or just the data (with an implied
   *                                    address).
   *
   * @param     default_data            The data to use for addresses which aren't in
   *                                    lookup_table.
   *
   * @param     ram                     If false, the default, this module functions as
   *                                    a lookup table.  If true, this module functions
   *                                    as a RAM with the RAM address, write data, and
   *                                    write strobe being bit fields within the TIE
   *                                    address signal (i.e. the address argument of
   *                                    the nb_send_address() method).
   *
   */
  xtsc_lookup_parms(xtsc::u32   address_bit_width,
                    xtsc::u32   data_bit_width,
                    bool        has_ready,
                    const char *lookup_table            = NULL,
                    const char *default_data            = "0x0",
                    bool        ram                     = false)
  {
    init(address_bit_width, data_bit_width, has_ready, lookup_table, default_data, ram);
  }


  /**
   * Constructor for an xtsc_lookup_parms object based upon an xtsc_core object and a
   * named TIE lookup. 
   *
   * This constructor will determine latency, address_bit_width, data_bit_width,
   * has_ready, and clock_period by querying the core object.  If desired, after the
   * xtsc_lookup_parms object is constructed, its data members can be changed using the
   * appropriate xtsc::xtsc_parms::set() method before passing it to the xtsc_lookup
   * constructor.
   *
   * @param     core                    A reference to the xtsc_core object upon which
   *                                    to base the xtsc_lookup_parms.
   *
   * @param     lookup_name             The name of the TIE lookup as it appears in the
   *                                    user's TIE code after the lookup keyword.
   *
   * @param     lookup_table            The name of the file containing the address-data
   *                                    pairs and/or just the data (with an implied
   *                                    address).
   *
   * @param     default_data            The data to use for addresses which aren't in
   *                                    lookup_table.
   *
   * @param     ram                     If false, the default, this module functions as
   *                                    a lookup table.  If true, this module functions
   *                                    as a RAM with the RAM address, write data, and
   *                                    write strobe being bit fields within the TIE
   *                                    address signal (i.e. the address argument of
   *                                    the nb_send_address() method).
   *
   */
  xtsc_lookup_parms(const xtsc::xtsc_core&      core, 
                    const char                 *lookup_name,
                    const char                 *lookup_table    = NULL,
                    const char                 *default_data    = "0x0",
                    bool                        ram             = false);


  /**
   * Do initialization common to both constructors.
   */
  void init(xtsc::u32   address_bit_width,
            xtsc::u32   data_bit_width,
            bool        has_ready,
            const char *lookup_table            = NULL,
            const char *default_data            = "0x0",
            bool        ram                     = false)
  {
    std::vector<xtsc::u32> ram_write_enables;
    add("ram",                  ram);
    add("override_lookup",      false);
    add("address_bit_width",    address_bit_width);
    add("data_bit_width",       data_bit_width);
    add("write_data_lsb",       0);
    add("write_strobe_bit",     0xFFFFFFFF);
    add("active_high_strobe",   true);
    add("ram_write_enables",    ram_write_enables);
    add("has_ready",            has_ready);
    add("pipeline_depth",       0);
    add("enforce_latency",      true);
    add("latency",              1);
    add("delay",                0);
    add("lookup_table",         lookup_table);
    add("default_data",         default_data);
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("vcd_handle",           (void*) NULL); 
  }


  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_lookup_parms"; }

};





/**
 * An TIE lookup implementation that connects using TLM-level ports.
 *
 * Example XTSC module implementing the xtsc::xtsc_lookup_if interface.  This module can
 * be configured to function as a ROM-based lookup table which is initialized from a
 * file or to function as a custom RAM.  In the latter case the TIE address signal is
 * comprised of three bit fields: the RAM address, the write data, and a write strobe.
 *
 * This module can also be used to model a non-ROM lookup (for example, a computed
 * lookup) as long as all possible lookups can be enumerated individually without
 * exceeding the host memory system.   When modeling a lookup device with a ready
 * signal, each possible lookup address can have a custom delay associated with it that
 * specifies how long the ready signal is deasserted after a lookup to that particular
 * address (so the delay potentially affects the next lookup, not the current lookup).
 * Note:  This delay behavior is a change from the RC-2010.1 and earlier behavior.
 *
 * If desired, this class can be sub-classes to provide an arbitrary lookup function
 * (i.e. not ROM based) by overriding the get_data_from_address() virtual method.
 *
 * Here is a block diagram of an xtsc_lookup as it is used in the xtsc_lookup
 * example:
 * @image html  Example_xtsc_lookup.jpg
 * @image latex Example_xtsc_lookup.eps "xtsc_lookup Example" width=10cm
 *
 * @see xtsc_lookup_parms
 * @see xtsc::xtsc_lookup_if
 */
class XTSC_COMP_API xtsc_lookup : public sc_core::sc_module, public xtsc::xtsc_resettable {
public:

  sc_core::sc_export<xtsc::xtsc_lookup_if>      m_lookup;       ///<  Driver binds to this


  // SystemC needs this.
  SC_HAS_PROCESS(xtsc_lookup);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_lookup"; }


  /**
   * Constructor for an xtsc_lookup.
   * @param     module_name     Name of the xtsc_lookup sc_module.
   * @param     lookup_parms    The remaining parameters for construction.
   * @see xtsc_lookup_parms
   */
  xtsc_lookup(sc_core::sc_module_name module_name, const xtsc_lookup_parms& lookup_parms);


  // Destructor.
  ~xtsc_lookup(void);


  /**
   * Reset the xtsc_lookup.
   */
  void reset(bool hard_reset = false);


  /**
   * Connect to a xtsc_core.
   *
   * This method connects this xtsc_lookup to the named lookup interface of the specified xtsc_core.
   *
   * @param     core            The xtsc_core to connect to.
   *
   * @param     lookup_name     Lookup name as it appears in the user's TIE code after
   *                            the lookup keyword.  This name must NOT begin with the
   *                            "TIE_" prefix.
   *
   */
  void connect(xtsc::xtsc_core& core, const char *lookup_name);


  /**
   * Connect to a xtsc_lookup_driver.
   *
   * This method connects this xtsc_lookup to the specified xtsc_lookup_driver.
   *
   * @param     driver          The xtsc_lookup_driver to connect to.
   *
   */
  void connect(xtsc_lookup_driver& driver);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


protected:

  /**
   * Get the data given the address.
   *
   * Sub-classes can override this virtual method to provide their own way of
   * determining the data (m_data) given a address (m_address) when using an exhaustive
   * mapping of addresses-to-data is undesirable.  
   *
   * Note:  The input to this method is the m_address member variable and the output of
   * this method is the m_data member variable.
   *
   * For example, the following code implements an xtsc_lookup sub-class that computes
   * a data value equal to double the address:
   * \verbatim
    class double_lookup : public xtsc_lookup {
    public:
      double_lookup(sc_module_name module_name, xtsc_lookup_parms parms) :
        xtsc_lookup (module_name, parms)
      {}
    protected:
      virtual void get_data_from_address() {
        m_data = 2*m_address;
      }
    };
    \endverbatim
   */
  virtual void get_data_from_address();


  /// Convert m_words[index] to sc_unsigned value
  void get_sc_unsigned(xtsc::u32 index, sc_dt::sc_unsigned& value);


  /// Return true if the pipeline is full
  bool pipeline_full();


  /// Do the lookup
  void do_lookup();


  /// Get an sc_unsigned from the pool
  sc_dt::sc_unsigned *new_sc_unsigned(const sc_dt::sc_unsigned& value);


  /// Return an sc_unsigned to the pool
  void delete_sc_unsigned(sc_dt::sc_unsigned*& p_sc_unsigned);


  /// Implementation of xtsc_lookup_if.
  class xtsc_lookup_if_impl : public xtsc::xtsc_lookup_if, public sc_core::sc_object  {
  public:

    /// Constructor
    xtsc_lookup_if_impl(const char *object_name, xtsc_lookup& lookup) :
      sc_object (object_name),
      m_lookup  (lookup),
      m_p_port  (0)
    {}

    /// @see xtsc::xtsc_lookup_if
    void nb_send_address(const sc_dt::sc_unsigned& address);

    /// @see xtsc::xtsc_lookup_if
    bool nb_is_ready();

    /// @see xtsc::xtsc_lookup_if
    sc_dt::sc_unsigned nb_get_data();

    /// @see xtsc::xtsc_lookup_if
    xtsc::u32 nb_get_address_bit_width() { return m_lookup.m_address_bit_width; }

    /// @see xtsc::xtsc_lookup_if
    xtsc::u32 nb_get_data_bit_width() {return m_lookup.m_data_bit_width; }

    /**
     * Get the event that will be notified when the lookup data is available.
     */
    virtual const sc_core::sc_event& default_event() const { return m_lookup.m_lookup_ready_event; }

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_lookup&                m_lookup;       ///< Our xtsc_lookup object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };



  xtsc_lookup_if_impl                   m_lookup_impl;          ///<  m_lookup binds to this

  bool                                  m_ram;                  ///<  True if lookup is a RAM, otherwise false
  bool                                  m_override_lookup;      ///<  See the "override_lookup" parameter
  xtsc::u32                             m_address_bit_width;    ///<  The lookup address bit width
  xtsc::u32                             m_ram_address_bits;     ///<  The RAM address bit width (if m_ram)
  xtsc::u32                             m_data_bit_width;       ///<  The lookup data bit width
  bool                                  m_active_high_strobe;   ///<  The write strobe is active high
  xtsc::u32                             m_write_strobe_bit;     ///<  The bit position in the lookup address of the write strobe
  xtsc::u32                             m_ram_address_lsb;      ///<  The starting bit position in the lookup address of the RAM address
  xtsc::u32                             m_ram_address_msb;      ///<  The ending bit position in the lookup address of the RAM address
  xtsc::u32                             m_write_data_lsb;       ///<  The starting bit position in the lookup address of the write data
  xtsc::u32                             m_write_data_msb;       ///<  The ending bit position in the lookup address of the write data
  std::vector<xtsc::u32>                m_ram_write_enables;    ///<  From "ram_write_enables" parameter
  bool                                  m_has_ram_write_enables;///<  true if "ram_write_enables" is specified
  bool                                  m_has_ready;            ///<  True if lookup has a ready signal
  xtsc::u32                             m_latency;              ///<  Latency from request/ready to valid data
  xtsc::u32                             m_pipeline_depth;       ///<  Pipeline depth
  bool                                  m_enforce_latency;      ///<  Check timing of nb_get_data() call
  xtsc::u32                             m_delay;                ///<  Default delay from request to ready as cycles
  xtsc::u32                             m_delay_next;           ///<  Delay to apply after this lookup
  bool                                  m_ready;                ///<  The ready signal
  sc_core::sc_time                      m_ready_net_time;       ///<  Ready no-earlier-than (net) time
  sc_dt::sc_unsigned                    m_default_data;         ///<  Data to use if address is not in lookup table
  std::string                           m_lookup_table;         ///<  Name of lookup_table file
  bool                                  m_file_logged;          ///<  True if contents of m_file have been logged
  xtsc::xtsc_script_file               *m_file;                 ///<  The lookup_file
  std::string                           m_line;                 ///<  Current line from m_file
  xtsc::u32                             m_line_count;           ///<  Current line number from m_file
  std::vector<std::string>              m_words;                ///<  Current line tokenized into words
  std::map<std::string, sc_dt::sc_unsigned*>
                                        m_data_map;             ///<  The lookup table
  std::map<std::string, xtsc::u32>      m_delay_map;            ///<  Delay associated with each address
  sc_dt::sc_unsigned                    m_data;                 ///<  Current request data
  sc_dt::sc_unsigned                    m_data_temp;            ///<  To hold data temporarily
  sc_dt::sc_unsigned                    m_old_data;             ///<  Data previously at current request address
  sc_dt::sc_unsigned                    m_address;              ///<  Current request address
  bool                                  m_write;                ///<  True if lookup is a write, false if lookup is a read
  sc_core::sc_event                     m_lookup_ready_event;   ///<  Notified when lookup might be ready (i.e. "ask me again")
  // Note: We use a deque instead of a sc_fifo so that this component can be used in turbo mode
  std::deque<sc_dt::sc_unsigned*>       m_data_fifo;            ///<  FIFO of lookup data values
  std::deque<xtsc::u64>                 m_cycle_fifo;           ///<  FIFO of clock cycle to get lookup data for latency checking
  sc_dt::sc_unsigned                    m_zero;                 ///<  Constant 0
  sc_dt::sc_unsigned                    m_one;                  ///<  Constant 1
  sc_core::sc_time                      m_time_resolution;      ///<  SystemC time resolution
  sc_core::sc_time                      m_clock_period;         ///<  This module's clock period
  xtsc::u64                             m_clock_period_value;   ///<  Clock period as u64
  bool                                  m_has_posedge_offset;   ///<  True if m_posedge_offset is non-zero
  sc_core::sc_time                      m_posedge_offset;       ///<  From "posedge_offset" parameter
  xtsc::u64                             m_posedge_offset_value; ///<  m_posedge_offset as u64
  std::vector<sc_dt::sc_unsigned*>      m_sc_unsigned_pool;     ///<  Maintain a pool of sc_unsigned to improve performance

  sc_core::sc_trace_file               *m_p_trace_file;         ///<  From "vcd_handle" parameter
  xtsc::u32                             m_nb_send_address_cnt;  ///<  Count nb_send_address() calls for vcd tracing
  xtsc::u32                             m_nb_is_ready_cnt;      ///<  Count nb_is_ready()     calls for vcd tracing
  xtsc::u32                             m_nb_get_data_cnt;      ///<  Count nb_get_data()     calls for vcd tracing
  sc_dt::sc_unsigned                    m_address_trace;        ///<  For vcd tracing
  sc_dt::sc_unsigned                    m_data_trace;           ///<  For vcd tracing
  sc_dt::sc_unsigned                   *m_p_ram_addr;           ///<  RAM address unmodified
  sc_dt::sc_unsigned                   *m_p_effective_ram_addr; ///<  RAM address with enable bits (if any) forced to 0

  log4xtensa::TextLogger&               m_text;                 ///<  The logger for text messages
  log4xtensa::BinaryLogger&             m_binary;               ///<  The logger for binary logging

};



}  // namespace xtsc_component




#endif  // _XTSC_LOOKUP_H_

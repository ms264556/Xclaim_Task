#ifndef _XTSC_PIN2TLM_MEMORY_TRANSACTOR_H_
#define _XTSC_PIN2TLM_MEMORY_TRANSACTOR_H_

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




namespace xtsc {
// Forward references
class xtsc_core;
class xtsc_cohctrl;
};



namespace xtsc_component {


// Forward references
class xtsc_tlm2pin_memory_transactor;


/**
 * Constructor parameters for a xtsc_pin2tlm_memory_transactor transactor object.
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
                                "IROM0", and "PIF" are 4, 8, 16, and 32.

   "start_byte_address" u32     The number to be added to the pin-level address to form
                                the TLM request address.  This corresponds to the
                                starting byte address of the memory in the 4GB address
                                space.
                                Default = 0x00000000.
  
   "big_endian"         bool    True if the memory interface master is big endian.
                                Default = false.

   "clock_period"       u32     This is the length of this device's clock period
                                expressed in terms of the SystemC time resolution
                                (from sc_get_time_resolution()).  A value of 
                                0xFFFFFFFF means to use the XTSC system clock 
                                period (from xtsc_get_system_clock_period()).  A value
                                of 0 means one delta cycle.
                                Default = 0xFFFFFFFF (i.e. use the system clock period).

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
                                period at which input pins are sampled.  Output pins
                                which are used for handshaking (PIReqRdy, PIRespValid,
                                IRamBusy, DRamBusy, etc.) are also sampled at this time.
                                This value is expressed in terms of the SystemC time
                                resolution (from sc_get_time_resolution()) and must be
                                strictly less than the clock period as specified by the
                                "clock_period" parameter.  A value of 0 means pins are
                                sampled on posedge clock as specified by the 
                                "posedge_offset" parameter.
                                Default = 0 (sample at posedge clock).

   "output_delay"       u32     This specifies how long to delay after the nb_respond()
                                call before starting to drive the output pins.  The
                                output pins will remain driven for one clock period
                                (see the "clock_period" parameter).  This value is
                                expressed in terms of the SystemC time resolution (from
                                sc_get_time_resolution()) and must be strictly less than
                                the clock period.  A value of 0 means one delta cycle.
                                Default = 1 (i.e. 1 time resolution).

   "vcd_handle"         void*   Pointer to SystemC VCD object (sc_trace_file *) or
                                0 if tracing is not desired.


   Parameters which apply to PIF only:

   "inbound_pif"        bool    Set to true for inbound PIF.  Set to false for outbound
                                PIF.  This parameter is ignored if "memory_interface"
                                is other then "PIF".
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

   "route_id_bits"      u32     Number of bits in the route ID.  Valid values are 0-32.
                                If "route_id_bits" is 0, then the "POReqRouteId" and
                                "PIRespRouteId" output ports will not be present.  This
                                parameter is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "prereject_responses" bool   If true, calls to nb_respond() will return false if
                                PORespRdy is low when the call to nb_respond() is made.
                                From the pin-level perspective, a response is not 
                                rejected unless PORespRdy is low at the sample time;
                                however, the XTSC TLM model of the response channel only
                                allows a response to be rejected at the time of the
                                nb_respond() call (by returning false).  The default
                                behavior of this model when PIRespRdy is low at the time
                                of the nb_respond() call is to simply repeat the
                                response the next cycle (if any other responses arrive
                                in the mean time, they are saved and handled in order).
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = false (don't return false to nb_respond).


   Parameters which apply to local memories only (that is, non-PIF memories):

   "memory_byte_size"   u32     The byte size of this memory.  This parameter is
                                ignored unless "memory_interface" is "XLMI0".
  
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
  
   "cbox"               bool    True if this memory interface is driven from an Xtensa
                                CBOX.  This parameter is ignored if "memory_interface"
                                is other then "DRAM0"|"DRAM1"|"DROM0".
    \endverbatim
 *
 * @see xtsc_pin2tlm_memory_transactor
 * @see xtsc::xtsc_parms
 */
class XTSC_COMP_API xtsc_pin2tlm_memory_transactor_parms : public xtsc::xtsc_parms {
public:


  /**
   * Constructor for an xtsc_pin2tlm_memory_transactor_parms transactor object.
   *
   * @param     memory_interface        The memory interface type.  Valid values are
                                        "DRAM0", "DRAM1", "DROM0", "IRAM0", "IRAM1",
                                        "IROM0", "URAM0", "XLMI0", and "PIF"
                                        (case-insensitive).
   *
   * @param     byte_width              Memory data interface width in bytes.
   *
   * @param     address_bits            Number of bits in address.  Ignored for "PIF".
   *
   * @param     num_ports               The number of memory ports this transactor has.
   */
  xtsc_pin2tlm_memory_transactor_parms(const char      *memory_interface        = "PIF",
                                       xtsc::u32        byte_width              = 4,
                                       xtsc::u32        address_bits            = 32,
                                       xtsc::u32        num_ports               = 1)
  {
    init(memory_interface, byte_width, address_bits, num_ports);
  }


  /**
   * Constructor for an xtsc_pin2tlm_memory_transactor_parms transactor object based
   * upon an xtsc_core object and a named memory interface. 
   *
   * This constructor will determine "clock_period", "byte_width", "big_endian",
   * "address_bits", "start_byte_address", "has_busy", "has_lock", "cbox", "check_bits",
   * and "has_pif_attribute" by querying the core object.  
   *
   * If desired, after the xtsc_pin2tlm_memory_transactor_parms object is constructed,
   * its data members can be changed using the appropriate xtsc_parms::set() method
   * before passing it to the xtsc_pin2tlm_memory_transactor constructor.
   *
   * @param     core                    A reference to the xtsc_core object upon which
   *                                    to base the xtsc_pin2tlm_memory_transactor_parms.
   *
   * @param     memory_interface        The memory interface type.  Valid values are
   *                                    "DRAM0", "DRAM1", "DROM0", "IRAM0", "IRAM1",
   *                                    "IROM0", "URAM0", "XLMI0", and "PIF"
   *                                    (case-insensitive).
   *
   * @param     num_ports               The number of ports this memory has.  If 0, the
   *                                    default, the number of ports (1 or 2) will be
   *                                    inferred thusly:  If memory_name is a LD/ST unit
   *                                    0 port of a dual-ported core interface, and the
   *                                    core is dual-ported and if the 2nd port of the
   *                                    core has not been bound, then "num_ports" will
   *                                    be 2; otherwise, "num_ports" will be 1.
   *
   * @Note If memory_interface is "PIF", then "start_byte_address" will be set to 0.
   */
  xtsc_pin2tlm_memory_transactor_parms(const xtsc::xtsc_core& core, const char *memory_interface, xtsc::u32 num_ports = 0);


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
    add("memory_byte_size",     0);
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("sample_phase",         0);
    add("output_delay",         1);
    add("big_endian",           false);
    add("has_request_id",       true);
    add("address_bits",         address_bits);
    add("route_id_bits",        0);
    add("prereject_responses",  false);
    add("has_busy",             true);
    add("has_lock",             false);
    add("has_xfer_en",          false);
    add("vcd_handle",           (void*)NULL);
    add("cbox",                 false);
    add("check_bits",           0);
  }


  /// Our C++ type (the xtsc_parms base class uses this for error messages)
  virtual const char* kind() const { return "xtsc_pin2tlm_memory_transactor_parms"; }

};




/**
 * This device converts memory transactions from pin level to transaction level.
 *
 * This device converts pin-level memory requests to TLM memory requests
 * (xtsc_request_if) and it converts the corresponding TLM responses (xtsc_respond_if)
 * to pin-level responses.
 *
 * When configured for the PIF, this module introduces some timing artifacts that might
 * not be present in a pure pin-level system.  This is because of the PIF protocol and
 * the way it is modeled in XTSC TLM.  Specifically, this module does not know that
 * there is a pin-level request until it is too late to reject the request.  Also, the
 * only way to reject a TLM response is to return false to the nb_response() call;
 * however, during the nb_response() call (which is non-blocking) this module does not
 * yet know that the upstream pin-level master will eventually reject the response (but
 * see the "prereject_responses" parameter).  To overcome these issues, the PIReqRdy
 * signal is deasserted for one clock period each time the nb_respond() call is
 * RSP_NACC.  For the case of back-to-back PIF requests, the effect of this is to
 * reject the next request after the request that would have been rejected in a pure
 * pin-level simulation.  By default, this model does not reject TLM PIF responses which
 * come from the memory interface slave and, if the memory interface master rejects a
 * pin-level response, then this module will simply repeat the response next cycle.
 *
 * When configured for a local memory, these timing artifacts don't exist because, for a
 * request, the busy is not due until the cycle after the request and, for a response,
 * there is no concept of rejecting it.
 *
 * Note: The parity/ECC signals (DRamNCheckDataM, DRamNCheckWrDataM, IRamNCheckData, and
 * IRamNCheckWrData) are present for IRAM and DRAM interfaces when "check_bits" is
 * non-zero; however, the input signal is ignored and the output signal is driven with
 * constant 0.
 *
 * Here is a block diagram of an xtsc_pin2tlm_memory_transactor as it is used in the
 * xtsc_pin2tlm_memory_transactor example:
 * @image html  Example_xtsc_pin2tlm_memory_transactor.jpg
 * @image latex Example_xtsc_pin2tlm_memory_transactor.eps "xtsc_pin2tlm_memory_transactor Example" width=13cm
 *
 * @see xtsc_pin2tlm_memory_transactor_parms
 * @see xtsc::xtsc_core
 * @see xtsc_tlm2pin_memory_transactor
 *
 */
class XTSC_COMP_API xtsc_pin2tlm_memory_transactor : public sc_core::sc_module, public xtsc_module_pin_base, public xtsc::xtsc_resettable {
protected:
  class request_info;
  friend class request_info;
public:

  sc_core::sc_port<xtsc::xtsc_request_if>     **m_request_ports;        /// From us to slave (per mem port)

  sc_core::sc_export<xtsc::xtsc_respond_if>   **m_respond_exports;      /// From slave to us (per mem port)

  sc_core::sc_export<xtsc::xtsc_debug_if>     **m_debug_exports;        /// From master to us (per mem port)

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
  typedef xtsc::xtsc_response                           xtsc_response;
  typedef sc_core::sc_fifo<sc_dt::sc_bv_base>           wide_fifo;
  typedef sc_core::sc_fifo<bool>                        bool_fifo;
  typedef sc_core::sc_signal<bool>                      bool_signal;
  typedef sc_core::sc_signal<sc_dt::sc_uint_base>       uint_signal;
  typedef sc_core::sc_signal<sc_dt::sc_bv_base>         wide_signal;
  typedef std::map<std::string, bool_signal*>           map_bool_signal;
  typedef std::map<std::string, uint_signal*>           map_uint_signal;
  typedef std::map<std::string, wide_signal*>           map_wide_signal;
  typedef std::deque<xtsc::xtsc_address>                address_deque;


  /// This SystemC macro inserts some code required for SC_THREAD's to work
  SC_HAS_PROCESS(xtsc_pin2tlm_memory_transactor);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_pin2tlm_memory_transactor"; }


  /**
   * Constructor for a xtsc_pin2tlm_memory_transactor.
   *
   * @param     module_name             Name of the xtsc_pin2tlm_memory_transactor
   *                                    sc_module.
   * @param     pin2tlm_parms           The remaining parameters for construction.
   *
   * @see xtsc_pin2tlm_memory_transactor_parms
   */
  xtsc_pin2tlm_memory_transactor(sc_core::sc_module_name module_name, const xtsc_pin2tlm_memory_transactor_parms& pin2tlm_parms);


  /// Destructor.
  ~xtsc_pin2tlm_memory_transactor(void);


  /**
   * Connect this xtsc_pin2tlm_memory_transactor to an xtsc_core.
   *
   * Depending upon the "snoop" parameter value, this method connects the specified TLM
   * master port pair of this xtsc_pin2tlm_memory_transactor to either the inbound PIF
   * ("snoop" is false) or the snoop ("snoop" is true) slave port pair of the specified
   * xtsc_core.
   *
   * @param     core                    The xtsc_core to connect with this
   *                                    xtsc_pin2tlm_memory_transactor.
   *
   * @param     pin2tlm_port            The master port pair of this
   *                                    xtsc_pin2tlm_memory_transactor to connect with
   *                                    the inbound PIF or snoop interface of core.
   *
   */
  void connect(xtsc::xtsc_core& core, xtsc::u32 pin2tlm_port = 0);


  /**
   * Connect with a client slave port pair of an xtsc_cohctrl.
   *
   * This method connects the specified TLM master port pair of this
   * xtsc_pin2tlm_memory_transactor to the specified client slave port pair of the
   * specified xtsc_cohctrl.
   *
   * @param     cohctrl         The xtsc_cohctrl to connect with.
   *
   * @param     cohctrl_port    This specifies which client slave port pair of the
   *                            xtsc_cohctrl to connect this transactor with.
   *
   * @param     pin2tlm_port    This specifies which TLM master port pair of this
   *                            xtsc_pin2tlm_memory_transactor to connect with the
   *                            xtsc_cohctrl.
   */
  void connect(xtsc::xtsc_cohctrl& cohctrl, xtsc::u32 cohctrl_port, xtsc::u32 pin2tlm_port);


  /**
   * Connect an xtsc_tlm2pin_memory_transactor transactor to this
   * xtsc_pin2tlm_memory_transactor transactor.
   *
   * This method connects the pin-level ports of an upstream
   * xtsc_tlm2pin_memory_transactor to the pin-level ports of this
   * xtsc_pin2tlm_memory_transactor.  In the process, it creates the necessary signals
   * of type sc_signal<bool>, sc_signal<sc_uint_base>, and sc_signal<sc_bv_base>.  The
   * name of each signal is formed by the concatenation of the SystemC name of the
   * xtsc_pin2tlm_memory_transactor object, the 2 characters "__", and the SystemC name
   * of the xtsc_pin2tlm_memory_transactor port (for example, "pin2tlm__POReqValid").
   *
   * @param     tlm2pin         The xtsc_tlm2pin_memory_transactor to connect to this
   *                            xtsc_pin2tlm_memory_transactor.
   *
   * @param     tlm2pin_port    The tlm2pin port to connect to.
   *
   * @param     pin2tlm_port    The port of this transactor to connect the tlm2pin to.
   *
   * @param     single_connect  If true only one port of this transactor will be connected.
   *                            If false, the default, then all contiguous, unconnected
   *                            port numbers of this transactor starting at pin2tlm_port
   *                            that have a corresponding existing port in tlm2pin
   *                            (starting at tlm2pin_port) will be connected to that
   *                            corresponding port in tlm2pin.
   *
   * NOTE:  This method is just for special testing purposes.  In general, connecting a
   *        xtsc_tlm2pin_memory_transactor to a xtsc_pin2tlm_memory_transactor is not
   *        guarranteed to meet timing requirements.
   *
   * @returns number of ports that were connected by this call (1 or more)
   */
  xtsc::u32 connect(xtsc_tlm2pin_memory_transactor&     tlm2pin,
                    xtsc::u32                           tlm2pin_port = 0,
                    xtsc::u32                           pin2tlm_port = 0,
                    bool                                single_connect = false);


  virtual void reset(bool hard_reset = false);


  /// Return the interface type
  memory_interface_type get_interface_type() const { return m_interface_type; }


  /// Return the interface name string
  const char *get_interface_name() const { return xtsc_module_pin_base::get_interface_name(m_interface_type); }


  /// Return the number of memory ports this transactor has
  xtsc::u32 get_num_ports() const { return m_num_ports; }


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


  /// Move response buffer data to m_data
  void get_read_data_from_response(const xtsc::xtsc_response& response);


  /// Sync to sample phase (m_sample_phase).  If already at sample phase, sync to next one.
  void sync_to_sample_phase();


  /// Handle local memory-interface requests
  void lcl_request_thread(void);


  /// Handle local memory-interface requests
  void lcl_drive_read_data_thread(void);


  /// Handle local memory-interface busy
  void lcl_drive_busy_thread(void);


  /// DPort0LoadRetiredm
  void xlmi_load_retired_thread();


  /// DPort0RetireFlushm
  void xlmi_retire_flush_thread();


  /// DRamnLockm
  void dram_lock_method();


  /// PIF: Capture a pin request from upstream
  void pif_sample_pin_request_thread(void);


  /// PIF: Drive PIReqRdy
  void pif_drive_req_rdy_thread(void);


  /// PIF: Send a TLM request downstream
  void pif_send_tlm_request_thread(void);


  /// PIF: Send a pin response upstream
  void pif_drive_pin_response_thread(void);


  /// Create an sc_signal<bool> with the specified name
  bool_signal& create_bool_signal(const std::string& signal_name);


  /// Create an sc_signal<sc_uint_base> with the specified name and size
  uint_signal& create_uint_signal(const std::string& signal_name, xtsc::u32 num_bits);


  /// Create an sc_signal<sc_bv_base> with the specified name and size
  wide_signal& create_wide_signal(const std::string& signal_name, xtsc::u32 num_bits);


  /// Swizzle byte enables 
  void swizzle_byte_enables(xtsc::xtsc_byte_enables& byte_enables) const;


  /// Implementation of xtsc_respond_if.
  class xtsc_respond_if_impl : public xtsc::xtsc_respond_if, public sc_core::sc_object {
  public:

    /// Constructor
    xtsc_respond_if_impl(const char *object_name, xtsc_pin2tlm_memory_transactor& pin2tlm, xtsc::u32 port_num) :
      sc_object         (object_name),
      m_pin2tlm         (pin2tlm),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// The kind of sc_object we are
    const char* kind() const { return "xtsc_pin2tlm_memory_transactor::xtsc_respond_if_impl"; }

    /// @see xtsc::xtsc_respond_if
    bool nb_respond(const xtsc::xtsc_response& response);

    /// Return true if a port has been bound to this implementation
    bool is_connected() { return (m_p_port != 0); }

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_pin2tlm_memory_transactor&     m_pin2tlm;      ///< Our xtsc_pin2tlm_memory_transactor object
    sc_core::sc_port_base              *m_p_port;       ///< Port that is bound to us
    xtsc::u32                           m_port_num;     ///< Our port number
  };



  /// Implementation of xtsc_debug_if.
  class xtsc_debug_if_impl : public xtsc::xtsc_debug_if, public sc_core::sc_object {
  public:

    /// Constructor
    xtsc_debug_if_impl(const char *object_name, xtsc_pin2tlm_memory_transactor& pin2tlm, xtsc::u32 port_num) :
      sc_object         (object_name),
      m_pin2tlm         (pin2tlm),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// The kind of sc_object we are
    const char* kind() const { return "xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl"; }

    /**
     *  Receive peeks from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    virtual void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /**
     *  Receive pokes from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    virtual void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /**
     *  Receive coherent peeks from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    virtual bool nb_peek_coherent(xtsc::xtsc_address    virtual_address8,
                                  xtsc::xtsc_address    physical_address8,
                                  xtsc::u32             size8,
                                  xtsc::u8             *buffer);

    /**
     *  Receive coherent pokes from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    virtual bool nb_poke_coherent(xtsc::xtsc_address    virtual_address8,
                                  xtsc::xtsc_address    physical_address8,
                                  xtsc::u32             size8,
                                  const xtsc::u8       *buffer);

    /**
     *  Receive requests for fast access information from the memory interface master
     *  @see xtsc::xtsc_debug_if
     */
    virtual bool nb_fast_access(xtsc::xtsc_fast_access_request &request);

    /// Return true if a port has been bound to this implementation
    bool is_connected() { return (m_p_port != 0); }


  protected:

    /// SystemC callback when something binds to us
    void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_pin2tlm_memory_transactor&     m_pin2tlm;      ///< Our xtsc_pin2tlm_memory_transactor object
    sc_core::sc_port_base              *m_p_port;       ///< Port that is bound to us
    xtsc::u32                           m_port_num;     ///< Our port number
  };



  /**
   * Information about each request.
   * Constructor and init() populate data members by reading the input pin values.
   */
  class request_info {
  public:

    /// Constructor for a new request_info
    request_info(const xtsc_pin2tlm_memory_transactor& pin2tlm, xtsc::u32 port);

    /// Initialize an already existing request_info object
    void init(xtsc::u32 port);

    /// Dump a request_info object
    void dump(std::ostream& os) const;

    const xtsc_pin2tlm_memory_transactor&       m_pin2tlm;              ///< A reference to the owning xtsc_pin2tlm_memory_transactor
    req_cntl                                    m_req_cntl;             ///< POReqCntl
    xtsc::xtsc_address                          m_address;              ///< POReqAdrs
    sc_dt::sc_bv_base                           m_data;                 ///< POReqData
    xtsc::xtsc_byte_enables                     m_byte_enables;         ///< POReqDataBE
    sc_dt::sc_uint_base                         m_id;                   ///< POReqId
    sc_dt::sc_uint_base                         m_priority;             ///< POReqPriority
    sc_dt::sc_uint_base                         m_route_id;             ///< POReqRouteId
    sc_dt::sc_uint_base                         m_req_attribute;        ///< POReqAttribute
    sc_dt::sc_uint_base                         m_vadrs;                ///< POReqCohVAdrsIndex
    sc_dt::sc_uint_base                         m_coherence;            ///< POReqCohCntl
    xtsc::xtsc_byte_enables                     m_fixed_byte_enables;   ///< POReqDataBE swizzled if m_big_endian
    xtsc::xtsc_address                          m_fixed_address;        ///< POReqAdrs fixed for xtsc_request
    xtsc::xtsc_request                          m_request;              ///< The TLM request
  };
  friend std::ostream& operator<<(std::ostream& os, const request_info& info);


  /// Get a new request_info (from the pool)
  request_info *new_request_info(xtsc::u32 port);


  /// Delete an request_info (return it to the pool)
  void delete_request_info(request_info*& p_request_info);


  /// Get a new xtsc_request (from the pool)
  xtsc_response *new_response(const xtsc_response& response);


  /// Delete an xtsc_request (return it to the pool)
  void delete_response(xtsc_response*& p_response);



  xtsc_respond_if_impl        **m_respond_impl;                 ///< m_respond_exports binds to this (per mem port)
  xtsc_debug_if_impl          **m_debug_impl;                   ///< m_debug_exports binds to this (per mem port)
  xtsc::u32                     m_num_ports;                    ///< The number of memory ports this transactor has
  std::deque<request_info*>    *m_request_fifo;                 ///< The fifo of incoming requests (per mem port)
  std::deque<xtsc_response*>   *m_pif_resp_fifo;                ///< The fifo of incoming PIF responses (per mem port)
  std::string                   m_interface_uc;                 ///< Uppercase version of "memory_interface" parameter
  memory_interface_type         m_interface_type;               ///< The memory interface type
  xtsc::u32                     m_size8;                        ///< Byte size of the attached XLMI0 from "memory_byte_size" parameter
  xtsc::u32                     m_width8;                       ///< Data width in bytes of the memory interface
  xtsc::xtsc_address            m_start_byte_address;           ///< Number to be add to the pin address to form the TLM request address
  xtsc::u64                     m_clock_period_value;           ///< This device's clock period as u64
  sc_core::sc_time              m_clock_period;                 ///< This device's clock period as sc_time
  sc_core::sc_time              m_time_resolution;              ///< The SystemC time resolution
  sc_core::sc_time              m_posedge_offset;               ///< From "posedge_offset" parameter
  sc_core::sc_time              m_sample_phase;                 ///< Clock phase for sampling certain signals (see "sample_phase")
  sc_core::sc_time              m_sample_phase_plus_one;        ///< m_sample_phase plus one clock period
  sc_core::sc_time              m_output_delay;                 ///< See "output_delay" parameter
  xtsc::u64                     m_posedge_offset_value;         ///< m_posedge_offset as u64
  bool                          m_has_posedge_offset;           ///< True if m_posedge_offset is non-zero
  bool                         *m_waiting_for_nacc;             ///< True if waiting for RSP_NACC from PIF slave (per mem port)
  bool                         *m_request_got_nacc;             ///< True if active request got RSP_NACC from PIF slave (per mem port)
  bool                          m_cbox;                         ///< See "cbox" parameter
  bool                          m_append_id;                    ///< True if pin port names should include the set_id.
  bool                          m_inbound_pif;                  ///< True if interface is inbound PIF
  bool                          m_snoop;                        ///< True if interface is snoop port
  bool                          m_has_coherence;                ///< See "has_coherence" parameter
  bool                          m_has_pif_attribute;            ///< See "has_pif_attribute" parameter
  bool                          m_big_endian;                   ///< True if master is big endian
  bool                          m_has_request_id;               ///< True if the "POReqId" and "PIRespId" ports should be present
  bool                          m_prereject_responses;          ///< See "prereject_responses" parameter
  xtsc::u32                    *m_current_id;                   ///< Used when m_has_request_id is false (per mem port)
  xtsc::u32                     m_address_bits;                 ///< Number of bits in the address (non-PIF only)
  xtsc::u32                     m_check_bits;                   ///< Number of bits in ECC/parity signals (from "check_bits")
  xtsc::xtsc_address            m_address_mask;                 ///< Address mask
  xtsc::xtsc_address            m_bus_addr_bits_mask;           ///< Address mask to get bits which indicate which byte lane
  xtsc::u32                     m_address_shift;                ///< Number of bits to right-shift the address
  xtsc::u32                     m_route_id_bits;                ///< Number of bits in the route ID (PIF only)

  xtsc::u32                     m_next_port_lcl_request_thread;           ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_drive_read_data_thread;   ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_lcl_drive_busy_thread;        ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_xlmi_load_retired_thread;     ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_xlmi_retire_flush_thread;     ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_sample_pin_request_thread;///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_drive_req_rdy_thread;     ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_send_tlm_request_thread;  ///< To give each thread instance a port number
  xtsc::u32                     m_next_port_pif_drive_pin_response_thread;///< To give each thread instance a port number

  bool                          m_has_busy;                     ///< True if memory interface has a busy pin (non-PIF only)
  bool                          m_has_lock;                     ///< True if memory interface has a lock pin (DRAM0|DRAM1 only)
  bool                          m_has_xfer_en;                  ///< True if memory interface has Xfer enable pin (NA PIF|DROM0|XLMI0)
  sc_core::sc_event            *m_pif_req_event;                ///< Notify pif_send_tlm_request_thread (per mem port)
  sc_core::sc_event            *m_pif_resp_event;               ///< Notify pif_drive_pin_response_thread (per mem port)
  bool                         *m_first_block_write;            ///< True if next BLOCK_WRITE will be first in the block (per mem port)
  xtsc::u32                    *m_burst_write_transfer_num;     ///< Transfer number for BURST_WRITE
  bool                         *m_first_rcw;                    ///< True if next RCW will be first in the block (per mem port)
  bool                         *m_dram_lock;                    ///< State of DRAM lock pin
  bool                          m_dram_lock_reset;              ///< State of DRAM lock pin needs to be reset
  xtsc::u64                    *m_tag;                          ///< Tag from first BLOCK_WRITE and RCW
  xtsc::xtsc_address           *m_last_address;                 ///< Keep track of BLOCK_WRITE/BURST_WRITE addresses
  sc_dt::sc_uint_base           m_address;                      ///< The address after any required shifting and masking
  sc_dt::sc_uint_base           m_id;                           ///< POReqId/PIRespId
  sc_dt::sc_uint_base           m_priority;                     ///< POReqPriority/PIRespPriority
  sc_dt::sc_uint_base           m_route_id;                     ///< POReqRouteId/PIRespRouteId
  sc_dt::sc_uint_base           m_coh_cntl;                     ///< POReqRouteId/PIRespRouteId
  sc_dt::sc_bv_base             m_data;                         ///< Read/write data
  req_cntl                      m_req_cntl;                     ///< Value for POReqCntrl
  resp_cntl                     m_resp_cntl;                    ///< Value for PIRespCntrl
  wide_fifo                   **m_read_data_fifo;               ///< sc_fifo of sc_bv_base read data values (per mem port)
  bool_fifo                   **m_busy_fifo;                    ///< sc_fifo to keep track of busy pin (per mem port)
  bool_fifo                   **m_req_rdy_fifo;                 ///< sc_fifo to keep track of PIReqRdy pin (per mem port)
  sc_core::sc_event            *m_drive_read_data_event;        ///< Notify when read data should be driven (per mem port)
  sc_core::sc_event            *m_drive_busy_event;             ///< Notify when busy should be driven (per mem port)
  sc_core::sc_event            *m_drive_req_rdy_event;          ///< Notify when PIReqRdy should be driven (per mem port)
  std::vector<request_info*>    m_request_pool;                 ///< Maintain a pool of requests to improve performance
  std::vector<xtsc_response*>   m_response_pool;                ///< Maintain a pool of responses to improve performance
  address_deque                *m_load_address_deque;           ///< deque of XLMI load addresses (per mem port)
  bool                         *m_previous_response_last;       ///< true if previous response was a last transfer (per mem port)
  map_bool_signal               m_map_bool_signal;              ///< The optional map of all sc_signal<bool> signals
  map_uint_signal               m_map_uint_signal;              ///< The optional map of all sc_signal<sc_uint_base> signals
  map_wide_signal               m_map_wide_signal;              ///< The optional map of all sc_signal<sc_bv_base> signals
  sc_dt::sc_bv_base             m_zero_bv;                      ///< For initialization
  sc_dt::sc_uint_base           m_zero_uint;                    ///< For initialization
  log4xtensa::TextLogger&       m_text;                         ///< Used for logging 

  // Local Memory pins (per mem port)
  bool_input                  **m_p_en;                         ///< DPortEn, DRamEn, DRomEn, IRamEn, IRomEn
  uint_input                  **m_p_addr;                       ///< DPortAddr, DRamAddr, DRomAddr, IRamAddr, IRomAddr
  uint_input                  **m_p_lane;                       ///< DPortByteEn, DRamByteEn, DRomByteEn, IRamWordEn, IRomWordEn
  wide_input                  **m_p_wrdata;                     ///< DPortWrData, DRamWrData, IRamWrData
  bool_input                  **m_p_wr;                         ///< DPortWr, DRamWr, IRamWr
  bool_input                  **m_p_load;                       ///< DPortLoad, IRamLoadStore, IRomLoad
  bool_input                  **m_p_retire;                     ///< DPortLoadRetired
  bool_input                  **m_p_flush;                      ///< DPortRetireFlush
  bool_input                  **m_p_lock;                       ///< DRamLock
  wide_input                  **m_p_check_wr;                   ///< DRamCheckWrData, IRamCheckWrData
  wide_output                 **m_p_check;                      ///< DRamCheckData, IRamCheckData
  bool_input                  **m_p_xfer_en;                    ///< DRamXferEn, IRamXferEn, IRomXferEn, URamXferEn
  bool_output                 **m_p_busy;                       ///< DPortBusy, DRamBusy, DRomBusy, IRamBusy, IRomBusy
  wide_output                 **m_p_data;                       ///< DPortData, DRamData, DRomData, IRamData, IRomData
  
  // PIF request channel pins (per mem port)
  bool_input                  **m_p_req_valid;                  ///< POReqValid
  uint_input                  **m_p_req_cntl;                   ///< POReqCntl
  uint_input                  **m_p_req_adrs;                   ///< POReqAdrs
  wide_input                  **m_p_req_data;                   ///< POReqData
  uint_input                  **m_p_req_data_be;                ///< POReqDataBE
  uint_input                  **m_p_req_id;                     ///< POReqId
  uint_input                  **m_p_req_priority;               ///< POReqPriority
  uint_input                  **m_p_req_route_id;               ///< POReqRouteId
  uint_input                  **m_p_req_attribute;              ///< POReqAttribute
  uint_input                  **m_p_req_coh_vadrs;              ///< POReqCohVAdrsIndex/SnoopReqCohVAdrsIndex
  uint_input                  **m_p_req_coh_cntl;               ///< POReqCohCntl
  bool_output                 **m_p_req_rdy;                    ///< PIReqRdy
  
  // PIF response channel pins (per mem port)
  bool_output                 **m_p_resp_valid;                 ///< PIRespValid
  uint_output                 **m_p_resp_cntl;                  ///< PIRespCntl
  wide_output                 **m_p_resp_data;                  ///< PORespData
  uint_output                 **m_p_resp_id;                    ///< PIRespId
  uint_output                 **m_p_resp_priority;              ///< PIRespPriority
  uint_output                 **m_p_resp_route_id;              ///< PIRespRouteId
  uint_output                 **m_p_resp_coh_cntl;              ///< PIRespCohCntl
  bool_input                  **m_p_resp_rdy;                   ///< PORespRdy


};  // class xtsc_pin2tlm_memory_transactor 



}  // namespace xtsc_component



#endif // _XTSC_PIN2TLM_MEMORY_TRANSACTOR_H_

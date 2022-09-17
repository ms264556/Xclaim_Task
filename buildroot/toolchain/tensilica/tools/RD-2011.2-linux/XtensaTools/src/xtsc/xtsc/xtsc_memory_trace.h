#ifndef _XTSC_MEMORY_TRACE_H_
#define _XTSC_MEMORY_TRACE_H_

// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_respond_if.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_address_range_entry.h>
#include <xtsc/xtsc_fast_access.h>
#include <vector>
#include <cstring>
#include <xtsc/xtsc_cohctrl.h>



namespace xtsc {
class xtsc_core;
}


namespace xtsc_component {

class xtsc_arbiter;
class xtsc_dma_engine;
class xtsc_master;
class xtsc_pin2tlm_memory_transactor;
class xtsc_router;


/**
 * Constructor parameters for a xtsc_memory_trace object.
 *
 *  \verbatim
   Name                  Type   Description
   ------------------    ----   -------------------------------------------------------
  
   "byte_width"          u32    Memory data interface width in bytes.  Valid values are
                                4, 8, 16, 32, and 64.  
                                Default = 64.
  
   "big_endian"          bool   True if the master is big endian.
                                Default = false.

   "vcd_handle"          void*  Pointer to a SystemC VCD object (sc_trace_file *) or 0
                                if the memory trace device should create its own trace 
                                file (which will be called "waveforms.vcd").
                                Default = 0.

   "num_ports"           u32    The number of memory ports attached to the memory trace
                                device.  This number of master devices and this number
                                of slave devices must be connected to the memory trace
                                device.
                                Default = 1.

   "enable_tracing"      bool   True if trace is to be done.  False if trace is not to
                                be done.  Setting "enable_tracing" to false allows
                                leaving the model in place in sc_main but with tracing
                                disabled so there is near zero impact on simulation
                                time.
                                Default = true.

    \endverbatim
 *
 * @see xtsc_memory_trace
 * @see xtsc::xtsc_parms
 */
class XTSC_COMP_API xtsc_memory_trace_parms : public xtsc::xtsc_parms {
public:


  /**
   * Constructor for an xtsc_memory_trace_parms object.
   *
   * @param     width8          Memory data interface width in bytes.
   *                            Default = 64.
   *
   * @param     big_endian      True if master is big_endian.
   *                            Default = false.
   *
   * @param     p_trace_file    Pointer to SystemC VCD object or 0 if the memory trace
   *                            device should create its own VCD object (which will be
   *                            called "waveforms.vcd").
   *                            Default = 0.
   *
   * @param     num_ports       The number of memory ports the memory trace device has.
   *                            Default = 1.
   *
   */
  xtsc_memory_trace_parms(xtsc::u32               width8        = 64,
                          bool                    big_endian    = false,
                          sc_core::sc_trace_file *p_trace_file  = 0,
                          xtsc::u32               num_ports     = 1)
  {
    init(width8, big_endian, p_trace_file, num_ports);
  }


  /**
   * Constructor for an xtsc_memory_trace_parms object based upon an xtsc_core object
   * and a named memory interface. 
   *
   * This constructor will determine width8, big_endian, and, optionally, num_ports by
   * querying the core object and then pass the values to the init() method.  
   *
   * If desired, after the xtsc_memory_trace_parms object is constructed, its data
   * members can be changed using the appropriate xtsc_parms::set() method before
   * passing it to the xtsc_memory constructor.
   *
   * @param     core            A reference to the xtsc_core object upon which to base
   *                            the xtsc_memory_trace_parms.
   *
   * @param     memory_name     The name of the memory interface. 
   *                            Note:  The core configuration must have the named memory
   *                            interface.
   *
   * @param     p_trace_file    Pointer to SystemC VCD object or 0 if the memory trace
   *                            device should create its own trace file (which will be
   *                            called "waveforms.vcd").
   *                            Default = 0.
   *
   * @param     num_ports       The number of ports this memory has.  If 0, the default,
   *                            the number of ports (1 or 2) will be inferred thusly: If 
   *                            memory_name is a LD/ST unit 0 port of a dual-ported core
   *                            interface, and the core is dual-ported and has no CBox,
   *                            and if the 2nd port of the core has not been bound, then
   *                            "num_ports" will be 2; otherwise, "num_ports" will be 1.
   *
   * @see xtsc::xtsc_core::How_to_do_memory_port_binding for a list of legal memory_name
   *      values.
   */
  xtsc_memory_trace_parms(const xtsc::xtsc_core&        core,
                          const char                   *memory_name, 
                          sc_core::sc_trace_file       *p_trace_file    = 0,
                          xtsc::u32                     num_ports       = 0);


  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_memory_trace_parms"; }

protected:

  /// Do initialization common to all constructors
  void init(xtsc::u32 width8, bool big_endian, sc_core::sc_trace_file *p_trace_file, xtsc::u32 num_ports) {
    add("byte_width",           width8);
    add("big_endian",           big_endian);
    add("vcd_handle",           (void*) p_trace_file);
    add("num_ports",            num_ports);
    add("enable_tracing",       true);
  }
};





/**
 * Example XTSC module which generates a value-change dump (VCD) file of the data
 * members of each xtsc_request and xtsc_response that passes through it.  Also, each
 * request and each response is counted and the counts are traced.  In addition, each
 * RSP_OK response and each RSP_NACC response are separately counted and traced to make
 * it easier to detect when an RSP_NACC response happens in the same cycle as an RSP_OK
 * response.
 *
 * This module is designed to be inserted between a memory interface master (for
 * example, an xtsc_core) and a memory interface slave (for example, an xtsc_memory) to 
 * generate a VCD file trace of the data members of each xtsc_request that the master
 * sends the slave and each xtsc_response that the slave sends the master.
 *
 * Here is a block diagram of an xtsc_memory_trace as it is used in the
 * xtsc_memory_trace example:
 * @image html  Example_xtsc_memory_trace.jpg
 * @image latex Example_xtsc_memory_trace.eps "xtsc_memory_trace Example" width=10cm
 *
 * @see xtsc_memory_trace_parms
 * @see xtsc::xtsc_request_if
 * @see xtsc::xtsc_respond_if
 *
 */
class XTSC_COMP_API xtsc_memory_trace : public sc_core::sc_module, public xtsc::xtsc_resettable {
public:

  sc_core::sc_export<xtsc::xtsc_request_if>   **m_request_exports;      ///<  From multi-ported master or multiple masters to us
  sc_core::sc_port  <xtsc::xtsc_request_if>   **m_request_ports;        ///<  From us to multi-ported slave or multiple slaves
  sc_core::sc_export<xtsc::xtsc_respond_if>   **m_respond_exports;      ///<  From multi-port slave or multiple slaves to us
  sc_core::sc_port  <xtsc::xtsc_respond_if>   **m_respond_ports;        ///<  From us to multi-ported master or multiple masters


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_memory_trace"; }


  /**
   * Constructor for an xtsc_memory_trace.
   * @param     module_name     Name of the xtsc_memory_trace sc_module.
   * @param     trace_parms     The remaining parameters for construction.
   * @see xtsc_memory_trace_parms
   */
  xtsc_memory_trace(sc_core::sc_module_name module_name, const xtsc_memory_trace_parms& trace_parms);


  // Destructor.
  ~xtsc_memory_trace(void);


  /// Get the number of memory ports this memory trace device has
  xtsc::u32 get_num_ports() { return m_num_ports; }


  /// Get a pointer to the VCD object that this memory trace device is using
  sc_core::sc_trace_file *get_trace_file() { return m_p_trace_file; }


  /**
   * Reset the xtsc_memory_trace.
   */
  void reset(bool hard_reset = false);


  /// Return whether or not tracing is enabled
  bool is_tracing_enabled() { return m_enable_tracing; }


  /// Set whether or not tracing is enabled
  void enable_tracing(bool enable) { m_enable_tracing = enable; }


  /**
   * Connect an upstream xtsc_arbiter with this xtsc_memory_trace.
   *
   * This method connects the master port pair of the specified xtsc_arbiter to the
   * specified slave port pair of this xtsc_memory_trace.
   *
   * @param     arbiter         The xtsc_arbiter to connect with.
   *
   * @param     trace_port      The slave port pair of this xtsc_memory_trace to connect
   *                            the xtsc_arbiter with.  trace_port must be in the
   *                            range of 0 to this xtsc_memory_trace's "num_ports"
   *                            parameter minus 1.
   */
  void connect(xtsc_arbiter& arbiter, xtsc::u32 trace_port = 0);


  /**
   * Connect with an upstream or downstream xtsc_cohctrl.
   *
   * This method is used to make one of the following three types of connections:
   * #- Connect the memory interface master port pair of the upstream xtsc_cohctrl with
   *    the specified slave port pair of this xtsc_memory_trace (type is PT_MEMORY).
   * #- Connect the specified snoop master port pair of the upstream xtsc_cohctrl with
   *    the specified slave port pair of this xtsc_memory_trace (type is PT_SNOOP).
   * #- Connect the specified master port pair of this xtsc_memory_trace with the
   *    specified client slave port pair of the downstream xtsc_cohctrl (type is
   *    PT_CLIENT).
   *
   * @param     cohctrl         The xtsc_cohctrl to connect with.
   *
   * @param     type            Which type of xtsc_cohctrl port to connect with:
   *                            PT_MEMORY, PT_SNOOP, or PT_CLIENT.
   *
   * @param     cohctrl_port    If type is PT_MEMORY, then this parameter is ignored.
   *                            If type is PT_SNOOP, then this specifies which snoop
   *                            master port pair of the xtsc_cohctrl to connect with the
   *                            slave port pair of this xtsc_memory_trace specified by
   *                            trace_port.
   *                            If type is PT_CLIENT, then this specifies the
   *                            xtsc_cohctrl client slave port pair that the single
   *                            master port pair of this xtsc_memory_trace is to be
   *                            connected with.
   *
   * @param     trace_port      If type is PT_MEMORY or PT_SNOOP, then this specifies
   *                            the slave port pair of this xtsc_memory_trace that is to
   *                            be connected with the xtsc_cohctrl.  If type is
   *                            PT_CLIENT, then this parameter is ignored.
   */
  void connect(xtsc::xtsc_cohctrl& cohctrl, xtsc::xtsc_cohctrl::port_type type, xtsc::u32 cohctrl_port, xtsc::u32 trace_port);


  /**
   * Connect with an upstream or downstream (inbound pif) xtsc_core.
   *
   * This method connects this xtsc_memory_trace with the memory interface specified by
   * memory_port_name of the xtsc_core specified by core.  If memory_port_name is
   * "inbound_pif" or "snoop", then a master port pair of this xtsc_memory_trace is
   * connected with the inbound pif or snoop slave port pair (respectively) of core.  If
   * memory_port_name is neither "inbound_pif" nor "snoop", then the master port pair of
   * core specified by memory_port_name is connected with a slave port pair of this
   * xtsc_memory_trace
   *
   * @param     core                    The xtsc_core to connect with.
   *
   * @param     memory_port_name        The name of the xtsc_core memory interface to
   *                                    connect with.  Case-insensitive.
   *
   * @param     port_num                If memory_port_name is "inbound_pif" or "snoop"
   *                                    then this specifies the master port pair of this
   *                                    xtsc_memory_trace to connect with core.  If
   *                                    memory_port_name is neither "inbound_pif" nor
   *                                    "snoop" then this specifies the slave port pair
   *                                    of this xtsc_memory_trace to connect with core.
   *
   * @param     single_connect          If true only one port pair of this
   *                                    xtsc_memory_trace will be connected.  If false,
   *                                    the default, and if memory_port_name names a
   *                                    LD/ST unit 0 dual-ported interface of core and
   *                                    if port_num+1 exists and has not yet been
   *                                    connected, then port_num and port_num+1 will be
   *                                    connected to core.  This parameter is ignored if
   *                                    memory_port_name is "inbound_pif" or "snoop".
   *
   * @returns number of ports that were connected by this call (1 or 2)
   *
   * @see xtsc::xtsc_core::How_to_do_memory_port_binding for a list of valid
   *      memory_port_name values.
   */
  xtsc::u32 connect(xtsc::xtsc_core& core, const char *memory_port_name, xtsc::u32 port_num = 0, bool single_connect = false);


  /**
   * Connect with an xtsc_dma_engine.
   *
   * This method connects the master port pair of the specified xtsc_dma_engine with the
   * specified slave port pair of this xtsc_memory_trace.
   *
   * @param     dma_engine      The xtsc_dma_engine to connect with.
   *
   * @param     trace_port      The slave port pair of this xtsc_memory_trace to connect
   *                            the xtsc_dma_engine with.
   */
  void connect(xtsc_dma_engine& dma_engine, xtsc::u32 trace_port = 0);


  /**
   * Connect with an xtsc_master.
   *
   * This method connects the master port pair of the specified xtsc_master with the
   * specified slave port pair of this xtsc_memory_trace.
   *
   * @param     master          The xtsc_master to connect with.
   *
   * @param     trace_port      The slave port pair of this xtsc_memory_trace to connect
   *                            the xtsc_master with.
   */
  void connect(xtsc_master& master, xtsc::u32 trace_port = 0);


  /**
   * Connect with an xtsc_pin2tlm_memory_transactor.
   *
   * This method connects this xtsc_memory_trace with the specified TLM master port pair
   * of the specified xtsc_pin2tlm_memory_transactor.
   *
   * @param     pin2tlm         The xtsc_pin2tlm_memory_transactor to connect with this
   *                            xtsc_memory_trace.
   *
   * @param     tran_port       The xtsc_pin2tlm_memory_transactor TLM master port pair
   *                            to connect with this xtsc_memory_trace.
   *
   * @param     trace_port      The slave port pair of this xtsc_memory_trace to connect
   *                            the xtsc_pin2tlm_memory_transactor with.
   *
   * @param     single_connect  If true only one slave port pair of this
   *                            xtsc_memory_trace will be connected.  If false, the
   *                            default, then all contiguous, unconnected slave port
   *                            pairs of this xtsc_memory_trace starting at trace_port
   *                            that have a corresponding existing master port pair in
   *                            pin2tlm (starting at tran_port) will be connected with
   *                            that corresponding pin2tlm master port pair.
   */
  xtsc::u32 connect(xtsc_pin2tlm_memory_transactor&     pin2tlm,
                    xtsc::u32                           tran_port       = 0,
                    xtsc::u32                           trace_port      = 0,
                    bool                                single_connect  = false);


  /**
   * Connect with an upstream xtsc_router.
   *
   * This method connects the specified master port pair of the specified upstream xtsc_router
   * with the specified slave port pair of this xtsc_memory_trace.
   *
   * @param     router          The upstream xtsc_router to connect with.
   *
   * @param     router_port     The master port pair of the upstream xtsc_router to connect
   *                            with.  router_port must be in the range of 0 to the upstream
   *                            xtsc_router's "num_slaves" parameter minus 1.
   *
   * @param     trace_port      The slave port pair of this xtsc_memory_trace to connect
   *                            with.  trace_port must be in the range of 0 to this
   *                            xtsc_memory_trace's "num_ports" parameter minus 1.
   */
  void connect(xtsc_router& router, xtsc::u32 router_port, xtsc::u32 trace_port);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


protected:

  /// Implementation of xtsc_request_if.
  class xtsc_request_if_impl : public xtsc::xtsc_request_if, public sc_core::sc_object  {
  public:

    /**
     * Constructor.
     * @param   object_name     The name of this SystemC channel (aka implementation)
     * @param   trace           A reference to the owning xtsc_memory_trace object.
     * @param   port_num        The port number that this object represents.
     */
    xtsc_request_if_impl(const char *object_name, xtsc_memory_trace& trace, xtsc::u32 port_num);

    /// @see xtsc::xtsc_debug_if
    virtual void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /// @see xtsc::xtsc_debug_if
    virtual void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /// @see xtsc::xtsc_debug_if
    virtual bool nb_peek_coherent(xtsc::xtsc_address    virtual_address8,
                                  xtsc::xtsc_address    physical_address8,
                                  xtsc::u32             size8,
                                  xtsc::u8             *buffer);

    /// @see xtsc::xtsc_debug_if
    virtual bool nb_poke_coherent(xtsc::xtsc_address    virtual_address8,
                                  xtsc::xtsc_address    physical_address8,
                                  xtsc::u32             size8,
                                  const xtsc::u8       *buffer);

    /// @see xtsc::xtsc_debug_if
    virtual bool nb_fast_access(xtsc::xtsc_fast_access_request &request);

    /// @see xtsc::xtsc_request_if
    virtual void nb_request(const xtsc::xtsc_request& request);

    /// @see xtsc::xtsc_request_if
    virtual void nb_load_retired(xtsc::xtsc_address address8);

    /// @see xtsc::xtsc_request_if
    virtual void nb_retire_flush();

    /// @see xtsc::xtsc_request_if
    virtual void nb_lock(bool lock);


    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_memory_trace&          m_trace;                ///<  Our xtsc_memory_trace object
    sc_core::sc_port_base      *m_p_port;               ///<  Port that is bound to us
    xtsc::u32                   m_port_num;             ///<  Our port number

    // The following are to allow tracing of each nb_request call
    xtsc::u64                   m_nb_request_count;     ///<  Count the nb_request calls on each port
    xtsc::u32                   m_address8;             ///<  Byte address 
    sc_dt::sc_unsigned          m_data;                 ///<  Data 
    xtsc::u32                   m_size8;                ///<  Byte size of each transfer
    xtsc::u32                   m_pif_attribute;        ///<  PIF request attribute of each transfer
    xtsc::u32                   m_route_id;             ///<  Route ID for arbiters
    xtsc::u8                    m_type;                 ///<  Request type (READ, BLOCK_READ, etc)
    xtsc::u32                   m_num_transfers;        ///<  Number of transfers
    xtsc::u16                   m_byte_enables;         ///<  Byte enables
    xtsc::u8                    m_id;                   ///<  Transaction ID
    xtsc::u8                    m_priority;             ///<  Transaction priority
    bool                        m_last_transfer;        ///<  True if last transfer of request
    xtsc::u32                   m_pc;                   ///<  Program counter associated with request 
    xtsc::u64                   m_tag;                  ///<  Unique tag per request-response set
    bool                        m_instruction_fetch;    ///<  True if request is for an instruction fetch, otherwise false
    xtsc::u8                    m_coherence;            ///<  Cache Coherence information.  @see xtsc::xtsc_request::coherence_t
    xtsc::u32                   m_snoop_virtual_address;///<  Virtual address for snoop controller
    xtsc::u32                   m_hw_address8;          ///<  Address that would appear in hardware.
    xtsc::u32                   m_transfer_num;         ///<  Number of this transfer.
  };


  /// Implementation of xtsc_respond_if
  class xtsc_respond_if_impl : virtual public xtsc::xtsc_respond_if, public sc_core::sc_object  {
  public:

    /**
     * Constructor.
     * @param   object_name     The name of this SystemC channel (aka implementation)
     * @param   trace           A reference to the owning xtsc_memory_trace object.
     * @param   port_num        The port number that this object represents.
     */
    xtsc_respond_if_impl(const char *object_name, xtsc_memory_trace& trace, xtsc::u32 port_num);

    /// @see xtsc::xtsc_respond_if
    bool nb_respond(const xtsc::xtsc_response& response);

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_memory_trace&          m_trace;                ///<  Our xtsc_memory_trace object
    sc_core::sc_port_base      *m_p_port;               ///<  Port that is bound to us
    xtsc::u32                   m_port_num;             ///<  Our port number

    // The following are to allow tracing of each nb_respond call
    xtsc::u64                   m_nb_respond_count;     ///<  Count the nb_respond calls on each port
    xtsc::u64                   m_rsp_ok_count;         ///<  Count the nb_respond calls on each port that are RSP_OK
    xtsc::u64                   m_rsp_nacc_count;       ///<  Count the nb_respond calls on each port that are RSP_NACC
    xtsc::u32                   m_address8;             ///<  Starting byte address
    sc_dt::sc_unsigned          m_data;                 ///<  Data
    xtsc::u32                   m_size8;                ///<  Byte size of each transfer
    xtsc::u32                   m_route_id;             ///<  Route ID for arbiters
    xtsc::u8                    m_status;               ///<  Response status
    xtsc::u8                    m_id;                   ///<  Transaction ID
    xtsc::u8                    m_priority;             ///<  Transaction priority
    bool                        m_last_transfer;        ///<  True if last transfer of response
    xtsc::u32                   m_pc;                   ///<  Program counter associated with request
    xtsc::u64                   m_tag;                  ///<  Unique tag per request-response set
    bool                        m_snoop;                ///<  True if this is a snoop response, otherwise false
    bool                        m_snoop_data;           ///<  True if this is a snoop response with data, otherwise false
    xtsc::u8                    m_coherence;            ///<  Cache Coherence information.  @see xtsc::xtsc_response::coherence_t
  };



  xtsc_request_if_impl                **m_request_impl;                 ///<  m_request_exports bind to these
  xtsc_respond_if_impl                **m_respond_impl;                 ///<  m_respond_exports bind to these

  xtsc::u32                             m_width8;                       ///<  The byte width of the memory data interface
  bool                                  m_big_endian;                   ///<  Swizzle the bytes in the request/response data bufer
  sc_core::sc_trace_file               *m_p_trace_file;                 ///<  The VCD object
  xtsc::u32                             m_num_ports;                    ///<  The number of memory ports
  bool                                  m_enable_tracing;               ///<  See "enable_tracing" parameter

  log4xtensa::TextLogger&               m_text;                         ///<  Text logger


};



}  // namespace xtsc_component




#endif  // _XTSC_MEMORY_TRACE_H_

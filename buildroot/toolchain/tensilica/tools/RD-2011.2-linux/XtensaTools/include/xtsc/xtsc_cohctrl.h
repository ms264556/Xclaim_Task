#ifndef _XTSC_COHCTRL_H_
#define _XTSC_COHCTRL_H_

// Copyright (c) 2007-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <vector>
#include <deque>
#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_respond_if.h>
#include <xtsc/xtsc_fast_access.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_address_range_entry.h>




/**
 * Most XTSC component library objects are in the xtsc_component namespace.
 *
 * @Note this does not include xtsc_core, xtsc_cohctrl, and xtsc_interrupt_distributor
 * which are in the xtsc namespace.
 */
namespace xtsc {


/**
 * Constructor parameters for an xtsc_cohctrl (coherence controller) object.
 *
 * This class contains the constructor parameters for a xtsc_cohctrl object.
 *  \verbatim
   Name                  Type   Description
   ------------------    ----   --------------------------------------------------------
  
   "num_transfers"       u32    The number of BLOCK_READ/BLOCK_WRITE transfers to do a
                                cache line fill/castout.  "num_transfers" multiplied by
                                "byte_width" must equal the cache line size.  Valid 
                                values are 2, 4, 8, and 16.

   "byte_width"          u32    PIF data interface width in bytes.  Valid values are
                                4, 8, and 16. 
                                Default = 4.
  
   "num_clients"         u32    The number of cache coherence clients in the cluster.
                                Maximum = 32.
                                Default = 4.

   "ignore_extra_snoop"  bool   This flag controls what happens if multiple snoop
                                clients respond with data.  If false, a exception is
                                thrown.  If true, the first snoop response with data is
                                accepted and all subsequent snoop responses with data to
                                the same snoop request are logged at INFO_LOG_LEVEL and
                                discarded.  
                                Default = false.

   "route_id_lsb"        u32    This parameter specifies the least significant bit of
                                this coherence controller's route_id bit field.  Each
                                xtsc_request and xtsc_response object contains a
                                route_id data member that can be accessed using the
                                get_route_id() and set_route_id() methods.  Each device
                                in a communication path must be assigned a bit field in
                                route_id by the system designer.  When a request is
                                received, the coherence controller fills in its assigned
                                bit field in route_id with the port number that the
                                request arrived on.  When a response comes back, the
                                coherence controller uses its bit field in route_id to
                                determine which port to forward the reply out on.
                                Note:  The coherence controller will use the following
                                       number of bits in route_id:
                                              ceil(log2(num_clients))+1
                                Warning:  The system designer must ensure that all
                                          devices in a communication path use non-
                                          overlapping bit fields in route_id.  The
                                          simulator is not able to detect overlapping
                                          bit fields in route_id and they will probably
                                          result in communication failure.  XTSC devices
                                          which use the route_id field include
                                          xtsc_cohctrl and xtsc_arbiter.

   "request_fifo_depth"  u32    The depth of the request fifo.
                                Default = 2.

   "response_fifo_depth" u32    The depth of the response fifo for responses from the
                                PIF memory.
                                Default = 2.

   "clock_period"        u32    This is the length of this coherence controller's clock
                                period expressed in terms of the SystemC time resolution
                                (from sc_get_time_resolution()).  A value of 0xFFFFFFFF
                                means to use the XTSC system clock period (from
                                xtsc_get_system_clock_period()).  A value of 0 means one
                                delta cycle.
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

   "arbitration_phase"   u32    The phase of the clock at which arbitration is performed
                                expressed in terms of the SystemC time resolution (from
                                sc_get_time_resolution()).  A value of 0 means to
                                arbitrate at posedge clock as specified by
                                "posedge_offset".  A value of 0xFFFFFFFF means to use a
                                phase of one-half of this arbiter's clock period which
                                corresponds to arbitrating at negedge clock.  The
                                arbitration phase must be strictly less than the
                                arbiter's clock period.
                                Default = 0xFFFFFFFF (arbitrate at negedge clock).

   "nacc_wait_time"      u32    This parameter, expressed in terms of the SystemC time
                                resolution, specifies how long to wait after sending a
                                request downstream to see if it was rejected by
                                RSP_NACC.  This value must not exceed this device's
                                clock period.  A value of 0 means one delta cycle.  A
                                value of 0xFFFFFFFF means to wait for a period equal to
                                this device's clock period.
                                CAUTION:  A value of 0 can cause an infinite loop in the
                                          simulation if the downstream module requires a
                                          non-zero time to become available.
                                Default = 0xFFFFFFFF (use this device's clock period).

   "snoop_nacc_wait_time" u32   This parameter, expressed in terms of the SystemC time
                                resolution, specifies how long to wait after sending a
                                snoop request to see if it was rejected by RSP_NACC.
                                This value must not exceed this device's clock period.
                                A value of 0 means one delta cycle.  A value of
                                0xFFFFFFFF means to wait for a period equal to this
                                device's clock period.
                                CAUTION:  A value of 0 can cause an infinite loop in the
                                          simulation if the module being snooped
                                          requires a non-zero time to become available.
                                Default = 0xFFFFFFFF (use this device's clock period).

   "response_repeat"     u32    The number of clock periods after a response is sent and
                                rejected before the response will be resent.  A value of 
                                0 means one delta cycle.
                                Default = 1.

   "recovery_time"       u32    If "delay_from_receipt" is true, this specifies two
                                things.  First, the minimum number of clock periods 
                                after a request is forwarded before the next request 
                                will be forwarded.  Second, the minimum number of clock
                                periods after a response is forwarded before the next
                                response will be forwarded.  If "delay_from_receipt" is
                                false, this parameter is ignored.  
                                Default = 1.

   "delay_from_receipt"  bool   If false, the following delay parameters apply from 
                                the start of processing of the request or response (i.e.
                                after all previous requests or all previous responses,
                                as appropriate, have been forwarded).  This models a 
                                coherence controller that can only service one request
                                at a time and one response at a time.  If true, the
                                following delay parameters apply from the time of
                                receipt of the request or response.  This models a
                                coherence controller with pipelining.
                                Default = true.

   "request_delay"       u32    The minimum number of clock periods it takes to forward
                                a request.  If "delay_from_receipt" is true, timing 
                                starts when the request is received by the coherence
                                controller.  If "delay_from_receipt" is false, timing
                                starts at the later of when the request is received and
                                when the previous request was forwarded.  A value of 0
                                means one delta cycle.  
                                Default = 1.

   "response_delay"      u32    The minimum number of clock periods it takes to forward
                                a response.  If "delay_from_receipt" is true, timing 
                                starts when the response is received by the coherence
                                controller.  If "delay_from_receipt" is false, timing
                                starts at the later of when the response is received and
                                when the previous response was forwarded.  A value of 0
                                means one delta cycle.  
                                Default = 1.

    \endverbatim
 *
 * @see xtsc_cohctrl
 * @see xtsc_parms
 */
class XTSC_API xtsc_cohctrl_parms : public xtsc_parms {
public:

  /**
   * Constructor for an xtsc_cohctrl_parms object.
   *
   * @param     num_transfers   The number of BLOCK_READ/BLOCK_WRITE transfers in a
   *                            cache line fill/castout.  Valid values are 2, 4, 8, and
   *                            16.
   *
   * @param     width8          The number bytes in the PIF data bus.  Valid values are
   *                            4, 8, and 16.
   *
   * @param     num_clients     The number of memory interface masters (usually
   *                            xtsc_core) competing for the memory interface slave
   *                            (usually xtsc_memory).  Must be greater then 0.
   *
   * @param     route_id_lsb    The least significant bit of this device's route_id bit
   *                            field.
   *
   * @Note  This device will use ceil(log2(num_clients))+1 bits in the route_id field.
   */
  xtsc_cohctrl_parms(u32  num_transfers,
                     u32  width8          = 4,
                     u32  num_clients     = 4,
                     u32  route_id_lsb    = 0)
  {
    add("num_transfers",        num_transfers);
    add("byte_width",           width8);
    add("num_clients",          num_clients);
    add("ignore_extra_snoop",   false);
    add("route_id_lsb",         route_id_lsb);
    add("request_fifo_depth",   2);
    add("response_fifo_depth",  2);
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("arbitration_phase",    0xFFFFFFFF);
    add("nacc_wait_time",       0xFFFFFFFF);
    add("snoop_nacc_wait_time", 0xFFFFFFFF);
    add("response_repeat",      1);
    add("recovery_time",        1);
    add("delay_from_receipt",   true);
    add("request_delay",        1);
    add("response_delay",       1);
  }

  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_cohctrl_parms"; }
};




/**
 * A TLM model of Tensilica's cache coherence controller.
 *
 * An XTSC module implementing a cache coherence controller that allows a memory
 * interface slave module (e.g. an xtsc_memory) to be accessed by multiple cache
 * coherence clients (usually xtsc_core modules).  The modules communicate mainly via
 * xtsc_request_if and xtsc_respond_if interfaces.  In addition, the xtsc_wire_write_if
 * interface is used to allow a core to opt in or out of coherence.
 *
 * Here is a block diagram of an xtsc_cohctrl as it is used in the coherence controller
 * example:
 * @image html  Example_xtsc_cohctrl.jpg
 * @image latex Example_xtsc_cohctrl.eps "xtsc_cohctrl Example" width=10cm
 *
 * @see xtsc_request_if
 * @see xtsc_respond_if
 * @see xtsc_wire_write_if
 * @see xtsc_cohctrl_parms
 */
// TODO: block diagram
class XTSC_API xtsc_cohctrl : public sc_core::sc_module, public xtsc_resettable {
public:

  sc_core::sc_export<xtsc_request_if>         **m_client_exports;       ///< Bind xtsc_core outbound PIF ports to these
  sc_core::sc_port  <xtsc_respond_if>         **m_client_ports;         ///< Bind these to xtsc_core outbound PIF exports

  sc_core::sc_port  <xtsc_request_if>         **m_snoop_ports;          ///< Bind these to xtsc_core snoop exports
  sc_core::sc_export<xtsc_respond_if>         **m_snoop_exports;        ///< Bind xtsc_core snoop ports to these

  sc_core::sc_export<xtsc_wire_write_if>      **m_ccon_exports;         ///< Bind xtsc_core CCON export states to these (SnoopOptInN)

  sc_core::sc_port  <xtsc_request_if>           m_request_port;         ///< Bind to single PIF slave export
  sc_core::sc_export<xtsc_respond_if>           m_respond_export;       ///< Bind single PIF slave port to this


  // For SystemC
  SC_HAS_PROCESS(xtsc_cohctrl);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_cohctrl"; }


  /**
   * Constructor for an xtsc_cohctrl.
   * @param     module_name     Name of the xtsc_cohctrl sc_module.
   * @param     parms           The remaining parameters for construction.
   * @see xtsc_cohctrl_parms
   */
  xtsc_cohctrl(sc_core::sc_module_name module_name, const xtsc_cohctrl_parms& parms);


  // Destructor.
  ~xtsc_cohctrl(void);


  /// Get the number of cache coherence client ports this coherence controller has
  u32 get_num_clients() { return m_num_clients; }


  /**
   * Reset the xtsc_cohctrl.
   */
  void reset(bool hard_reset = false);


  /**
   * Dump the MHTE table for debugging.
   *
   * Dump the MHTE table showing the following information for each table entry:
   * \verbatim
     Column Heading     Contents
     --------------     ------------------------------------
     MHTE               MHTE table index
     Idle               m_idle
     QWait              m_qwait
     Share              m_shared
     URDB               m_use_response_data_buffer
     Cast               m_castout
     CD                 m_castout_done
     LCRR               m_last_coherent_response_recd
     CRS                m_coherent_response_sent
     CwSD               m_client_with_snoop_data
     WRR                m_write_response_recd
     UMR                m_use_memory_response
     nSLT               m_num_snoop_last_transfers
     QPtr               m_qptr
     OriginalRequest    m_p_coherent_request_info->m_request
     \endverbatim
   */
  void dump_mht(log4xtensa::LogLevel ll);


  /**
   * Connect with an xtsc_core.
   *
   * If outbound is true, then this method connects the outbound PIF master port pair of
   * the specified xtsc_core with the specified client slave port pair of this
   * xtsc_cohctrl.
   * If snoop is true, then this method connects the specified snoop master port pair of
   * this xtsc_cohctrl with the inbound snoop slave port pair of the specified
   * xtsc_core.
   * If ccon is true, then this method connects the TIE_CCON port of the specified
   * xtsc_core to the ccon port of this xtsc_cohctrl.
   *
   * @param     core            The xtsc_core to connect with.
   *
   * @param     port_num        This specifies both the client slave port pair of this
   *                            xtsc_cohctrl that the outbound PIF master port pair of the
   *                            xtsc_core will be connect with (if outbound is true) as
   *                            well as the snoop master port pair of this xtsc_cohctrl that
   *                            will be connected with the snoop slave port pair of the
   *                            xtsc_core (if snoop is true).  This parameter must be in
   *                            the range of 0 to this xtsc_cohctrl's "num_clients"
   *                            parameter minus 1.
   *
   * @param     outbound        If true, then the outbound PIF master port pair of core
   *                            will be connected with the client slave port pair of
   *                            this xtsc_cohctrl specified by port_num.
   *
   * @param     snoop           If true, then the snoop master port pair specified by
   *                            port_num of this xtsc_cohctrl will be connected with the
   *                            inbound snoop slave port pair of core.
   *
   * @param     ccon            If true, then the TIE_CCON port of core will be
   *                            connected to the ccon port of this xtsc_cohctrl.
   */
  void connect(xtsc_core& core, u32 port_num, bool outbound = true, bool snoop = true, bool ccon = true);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


  enum port_type {
    PT_CLIENT = 0,      ///<  PIF from cache coherence clients:        m_client_exports/m_client_ports
    PT_SNOOP  = 1,      ///<  Snoop ports to cache coherence clients:  m_snoop_ports/m_snoop_exports
    PT_MEMORY = 2,      ///<  Single port to the shared memory:        m_request_port/m_respond_export
  };

protected:

  /// Implementation of xtsc_request_if for requests from the cache coherence clients.
  class xtsc_request_if_impl : virtual public xtsc_request_if, public sc_core::sc_object {
  public:
    friend class xtsc_cohctrl;

    /**
     * Constructor.
     * @param   cohctrl         A reference to the owning xtsc_cohctrl object.
     * @param   port_num        The port number that this object serves.
     */
    xtsc_request_if_impl(const char *object_name, xtsc_cohctrl& cohctrl, u32 port_num) :
      sc_object         (object_name),
      m_cohctrl         (cohctrl),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// @see xtsc_request_if
    virtual void nb_request(const xtsc_request& request);

    /// @see xtsc_debug_if
    virtual void nb_peek(xtsc_address address8, u32 size8, u8 *buffer);

    /// @see xtsc_debug_if
    virtual void nb_poke(xtsc_address address8, u32 size8, const u8 *buffer);

    /// @see xtsc_debug_if
    virtual bool nb_peek_coherent(xtsc_address virtual_address8, xtsc_address physical_address8, u32 size8, u8 *buffer);

    /// @see xtsc_debug_if
    virtual bool nb_poke_coherent(xtsc_address virtual_address8, xtsc_address physical_address8, u32 size8, const u8 *buffer);

    /// @see xtsc_debug_if
    virtual bool nb_fast_access(xtsc_fast_access_request &request);

  private:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_cohctrl&               m_cohctrl;      ///< Our xtsc_cohctrl object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
    u32                         m_port_num;     ///< Our port number
  };



  /// Implementation of xtsc_respond_if for the downstream memory.
  class xtsc_respond_if_impl : public xtsc_respond_if, public sc_core::sc_object {
  public:
    friend class xtsc_cohctrl;

    /// Constructor
    xtsc_respond_if_impl(const char *object_name, xtsc_cohctrl& cohctrl) :
      sc_object         (object_name),
      m_cohctrl         (cohctrl),
      m_p_port          (0)
    {}

    /// @see xtsc_respond_if
    bool nb_respond(const xtsc_response& response);

  private:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_cohctrl&               m_cohctrl;      ///< Our xtsc_cohctrl object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };


  /// Implementation of xtsc_respond_if for the snoop responses from the cache coherence clients
  class xtsc_respond_if_impl_snoop : public xtsc_respond_if, public sc_core::sc_object {
  public:
    friend class xtsc_cohctrl;

    /// Constructor
    xtsc_respond_if_impl_snoop(const char *object_name, xtsc_cohctrl& cohctrl, u32 port_num) :
      sc_object         (object_name),
      m_cohctrl         (cohctrl),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// @see xtsc_respond_if
    bool nb_respond(const xtsc_response& response);

  private:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_cohctrl&               m_cohctrl;      ///< Our xtsc_cohctrl object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
    u32                         m_port_num;     ///< Our port number
  };


  /// Implementation of xtsc_wire_write_if.
  class xtsc_wire_write_if_impl : public xtsc_wire_write_if, public sc_core::sc_object {
  public:

    /// Constructor
    xtsc_wire_write_if_impl(const char *object_name, xtsc_cohctrl& cohctrl, u32 port_num) :
      sc_object         (object_name),
      m_cohctrl         (cohctrl),
      m_p_port          (0),
      m_port_num        (port_num),
      m_text            (m_cohctrl.m_text)
    {}

    /// The kind of sc_object we are
    const char* kind() const { return "xtsc_cohctrl::xtsc_wire_write_if_impl"; }

    /**
     *  Receive writes from the master
     *  @see xtsc_wire_write_if
     */
    virtual void nb_write(const sc_dt::sc_unsigned& value);

    /**
     *  Get the wire width in bits.
     *  @see xtsc_wire_write_if
     */
    virtual u32 nb_get_bit_width() { return 1; }


    /// SystemC callback when something binds to us
    void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_cohctrl&               m_cohctrl;      ///< Our xtsc_cohctrl object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
    u32                         m_port_num;     ///< Our port number
    log4xtensa::TextLogger&     m_text;         ///< Used for logging 
  };


  /// Information about each request
  class request_info {
  public:
    /// Constructor
    request_info(const xtsc_request&    request,
                 bool                   can_delete,
                 u32                    port_num,
                 u32                    route_id_bits   = 0xFFFFFFFF,
                 u32                    snoop_client    = 0xFFFFFFFF) :
      m_request                 (request),
      m_can_delete              (can_delete),
      m_port_num                (port_num),
      m_route_id_bits           (route_id_bits == 0xFFFFFFFF ? m_port_num : route_id_bits),
      m_snoop_client            (snoop_client),
      m_reread                  (false),
      m_from_request_fifo       (false),
      m_time_stamp              (sc_core::sc_time_stamp())
    {}
    xtsc_request        m_request;              ///< Our copy of the request
    bool                m_can_delete;           ///< request_thread can delete this after it is accepted by downstream memory
    u32                 m_port_num;             ///< Port original request (NOT snoop response) came in on
    u32                 m_route_id_bits;        ///< Bits to insert into route ID
    u32                 m_snoop_client;         ///< Client which had snoop-with-data response (else 0xFFFFFFFF)
    bool                m_reread;               ///< True if repeat due to castout
    bool                m_from_request_fifo;    ///< true if from m_request_fifo, false if from m_block_write_deque/m_mht_deque
    sc_core::sc_time    m_time_stamp;           ///< Timestamp when received
  };


  /// Information about each response from snoop port or from downstream memory
  class response_info {
  public:
    /// Constructor
    response_info(const xtsc_response& response, u32 client, bool snoop, bool block_write, bool reread, bool coherent) :
      m_response        (response)
    {
      common_init(client, snoop, block_write, reread, coherent);
    }
    /// Initializer for recycling response_info
    void init(const xtsc_response& response, u32 client, bool snoop, bool block_write, bool reread, bool coherent) {
      m_response = response;
      common_init(client, snoop, block_write, reread, coherent);
    }
    xtsc_response       m_response;             ///< Our copy of the response
    u32                 m_client;               ///< Client who sent the original request to xtsc_cohctrl
    bool                m_snoop;                ///< True if response is from snoop; false if response if from memory
    bool                m_block_write;          ///< True if response is for memory BLOCK_WRITE due to snoop response with data
    bool                m_reread;               ///< True if response is from coherent memory request repeat due to castout
    bool                m_coherent;             ///< False if this is a memory response to a non-coherent request, otherwise true
    sc_core::sc_time    m_time_stamp;           ///< Timestamp when received
  protected:
    /// Common initializer routine
    void common_init(u32 client, bool snoop, bool block_write, bool reread, bool coherent);
  };


  /**
   * Miss Handling Table Entry.
   *
   * RDB = Response Data Buffer
   */
  class MHTE {
  public:
    friend class xtsc_cohctrl;
    /// Constructor
    MHTE(xtsc_cohctrl &cohctrl, u32 client);
    void init(request_info *p_info);
    xtsc_cohctrl&                       m_cohctrl;                      ///< Our xtsc_cohctrl
    u32                                 m_client;                       ///< Client which uses this MHTE
    bool                                m_idle;                         ///< True if this entry is not in use
    bool                                m_qwait;                        ///< True if this entry is queued behind another MHTE
    bool                                m_shared;                       ///< True if any snoop response is SHARED
    bool                                m_use_response_data_buffer;     ///< True if first coherent response had to go to RDB
    bool                                m_castout;                      ///< A castout of this cache line has been detected
    bool                                m_castout_done;                 ///< The castout of this cache line has been completed
    bool                                m_last_coherent_response_recd;  ///< True when last transfer coherent response rec'd from memory
    bool                                m_coherent_response_sent;       ///< True when last_transfer coherent response sent to client
    bool                                m_snoop_with_data;              ///< Snoop response with data has occurred
    bool                                m_write_response_recd;          ///< True when write response for snoop data is rec'd from mem
    bool                                m_use_memory_response;          ///< True if all snoop responses are rec'd and none had data
    u32                                 m_client_with_snoop_data;       ///< The client which has snoop data (or 0xFFFFFFFF)
    u32                                 m_num_snoop_last_transfers;     ///< How many last_transfer snoop responses have been rec'd
    u32                                 m_qptr;                         ///< Index of next queued MHTE with same cache line
    u32                                 m_transfer_num;                 ///< Count of snoop response with data (for req address)
    request_info                       *m_p_coherent_request_info;      ///< Our copy of the original coherent request_info
    request_info                       *m_p_snoop_request_info;         ///< Our copy of the snoop request_info
    request_info                       *m_p_mem_read_request_info;      ///< Our copy of the memory read request_info
    std::deque<response_info*>          m_response_info_deque;          ///< Deque of coherent request responses from memory
  };


  // Return true if addresses are within the same cache line
  bool same_cache_line(xtsc_address address1, xtsc_address address2);


  /// SystemC thread to arbitrate incoming requests from multiple clients during the arbitration_phase
  void arbitrate_thread(void);


  /// SystemC thread to handle each accepted request at the correct time
  void request_thread(void);


  /// SystemC thread to send snoop requests
  void snoop_thread(void);


  /// SystemC thread to handle responses 
  void response_thread(void);


  /**
   * This method updates the route ID in the request with the bits of the specified port
   * number so the the response derived from the request will be able to get back to the
   * upstream module that sent xtsc_cohctrl this request.
   */
  void insert_route_id_bits(xtsc_request& request, u32 route_id_bits);


  /**
   * This method updates the route ID in the response with the bits of the specified
   * port number so the the response will be able to get back to the upstream module
   * that sent xtsc_cohctrl the original request.
   */
  void insert_route_id_bits(xtsc_response& response, u32 route_id_bits);


  /// Extract and return our route ID bitfield from the full route ID
  u32 extract_route_id_bits(u32 route_id);


  /// Free the resources in an MHTE
  void free_mhte_resources(MHTE& mhte);


  /**
   * Release the MHTE and return true if the following conditions are met:
   * -# All snoop responses have been received from the non-requesting clients.
   * -# All coherent responses (including repeat-due-to-castout) have been received from
   *    memory.
   * -# All coherent responses have been sent to the client (regardless of whether they
   *    originated from memory or from snoop response with data).
   * -# If there was snoop-with-data, then the data has been written to the downstream
   *    memory and the write response has been received.
   */
  bool test_and_release_mhte(u32 mhte_index);


  /// Get a new request_info (from the pool)
  request_info *new_request_info(const xtsc_request&    request,
                                 bool                   can_delete,
                                 u32                    port_num,
                                 u32                    route_id_bits   = 0xFFFFFFFF,
                                 u32                    snoop_client    = 0xFFFFFFFF);
        

  /// Delete a request_info (return it to the pool)
  void delete_request_info(request_info*& p_request_info);


  /// Get a new response_info (from the pool)
  response_info *new_response_info(const xtsc_response& response, bool snoop);


  /// Delete a response_info (return it to the pool)
  void delete_response_info(response_info*& p_response_info);


  /// Lock codes for processor outbound PIF response port
  enum response_port_lock {
    RPL_NONE = 0,       ///< Unlocked
    RPL_MEM  = 1,       ///< Locked to main memory
    RPL_MHTE = 2        ///< Locked to MHTE
  };


  xtsc_request_if_impl                **m_request_impl;                 ///<  m_client_exports bind to these (1 per client)
  xtsc_respond_if_impl                  m_respond_impl;                 ///<  m_respond_export binds to this
  xtsc_respond_if_impl_snoop          **m_snoop_respond_impl;           ///<  m_snoop_exports bind to this (1 per client)
  xtsc_wire_write_if_impl             **m_wire_write_impl;              ///<  m_ccon_exports bind to this (1 per client)

  std::vector<bool>                     m_ccon;                         ///<  Value of the m_ccon_exports input

  u32                                   m_request_fifo_depth;           ///<  The depth of the request fifos 
  u32                                   m_response_fifo_depth;          ///<  The depth of the single memory response fifo 

  sc_core::sc_fifo<request_info*>     **m_request_fifos;                ///<  Buffer requests pre-arbitration (1 per client)
  sc_core::sc_fifo<request_info*>       m_request_fifo;                 ///<  Buffer requests post-arbitration
  sc_core::sc_fifo<u32>                 m_response_fifo;                ///<  Buffer memory responses tokens to ensure determinancy
  std::deque<request_info*>             m_mht_deque;                    ///<  Buffer coherent req from MHT (castout repeats or queued)
  std::deque<request_info*>             m_block_write_deque;            ///<  Buffer BLOCK_WRITE requests due to snoop response w/ data
  std::deque<u32>                     **m_snoop_deques;                 ///<  Buffer MHTE indexes of snoop requests (1 per client)
  std::deque<response_info*>            m_response_deque;               ///<  Buffer actual memory responses in deque so we can peek
  std::deque<response_info*>          **m_response_deques;              ///<  Buffer snoop and coherent memory responses (1 per client)

  u32                                   m_num_request_info;             ///<  Number of request_info objects created
  u32                                   m_num_response_info;            ///<  Number of response_info objects created
  u32                                   m_num_transfers;                ///<  Number of transfers to do a cache line fill/castout
  u32                                   m_width8;                       ///<  Byte width of PIF data interface
  u32                                   m_cache_line_size;              ///<  The number of bytes in a cache line
  xtsc_address                          m_cache_line_mask;              ///<  Mask out address bits not used to determine the cache line
  u32                                   m_num_clients;                  ///<  The number of cache coherence clients
  u32                                   m_next_port_num;                ///<  Used by response_thread to get its port number
  std::vector<MHTE*>                    m_miss_handling_table;          ///<  The Miss Handling Table
  u32                                   m_route_id_bits_mask;           ///<  Our bit-field in the request route ID
  u32                                   m_route_id_bits_shift;          ///<  Offset to our bit-field in request route ID
  bool                                  m_waiting_for_nacc;             ///<  True if waiting for RSP_NACC from slave
  bool                                  m_request_got_nacc;             ///<  True if active request got RSP_NACC from slave
  u32                                   m_token;                        ///<  The port number which has the token
  u32                                   m_snoop_data_buffer_client;     ///<  Client using the snoop data buffer
  u32                                   m_lock_request_snoop_client;    ///<  Snoop client owning m_lock_request or 0xFFFFFFFF
  bool                                  m_lock_request;                 ///<  Lock outbound PIF request port
  response_port_lock                   *m_lock_response;                ///<  Lock PIF response port (1 per client)
  bool                                 *m_snoop_request_got_nacc;       ///<  True if snoop request got RSP_NACC (1 per client)
  bool                                 *m_waiting_for_snoop_nacc;       ///<  True if snoop port is waiting for RSP_NACC (1 per client)

  sc_core::sc_time                      m_clock_period;                 ///<  This coherence controller's clock period
  sc_core::sc_time                      m_arbitration_phase;            ///<  Clock phase arbitration occurs
  sc_core::sc_time                      m_arbitration_phase_plus_one;   ///<  Clock phase arbitration occurs plus one clock period
  sc_core::sc_time                      m_time_resolution;              ///<  SystemC time resolution
  u64                                   m_clock_period_value;           ///<  Clock period as u64
  bool                                  m_has_posedge_offset;           ///<  True if m_posedge_offset is non-zero
  sc_core::sc_time                      m_posedge_offset;               ///<  From "posedge_offset" parameter
  xtsc::u64                             m_posedge_offset_value;         ///<  m_posedge_offset as u64

  bool                                  m_ignore_extra_snoop;           ///<  See "ignore_extra_snoop" in xtsc_cohctrl_parms
  bool                                  m_delay_from_receipt;           ///<  See "delay_from_receipt" in xtsc_cohctrl_parms
  sc_core::sc_time                      m_last_request_time_stamp;      ///<  Time last request was sent out
  sc_core::sc_time                     *m_last_response_time_stamp;     ///<  Time last response was sent out (1 per client)

  sc_core::sc_time                      m_recovery_time;                ///<  See "recovery_time" in xtsc_cohctrl_parms
  sc_core::sc_time                      m_request_delay;                ///<  See "request_delay" in xtsc_cohctrl_parms
  sc_core::sc_time                      m_nacc_wait_time;               ///<  See "nacc_wait_time" in xtsc_cohctrl_parms
  sc_core::sc_time                      m_snoop_nacc_wait_time;         ///<  See "snoop_nacc_wait_time" 
  sc_core::sc_time                      m_response_delay;               ///<  See "response_delay" in xtsc_cohctrl_parms
  sc_core::sc_time                      m_response_repeat;              ///<  See "response_repeat" in xtsc_cohctrl_parms

  sc_core::sc_event                     m_arbitrate_thread_event;       ///<  To notify arbitrate_thread
  sc_core::sc_event                     m_request_thread_event;         ///<  To notify request_thread
  sc_core::sc_event                     m_snoop_thread_event;           ///<  To notify snoop_thread
  sc_core::sc_event                    *m_response_thread_event;        ///<  To notify response_thread (1 per client)

  std::vector<request_info*>            m_request_pool;                 ///<  Maintain a pool of requests to improve performance
  std::vector<response_info*>           m_response_pool;                ///<  Maintain a pool of responses to improve performance

  log4xtensa::TextLogger&               m_text;                         ///<  Text logger
  log4xtensa::BinaryLogger&             m_binary;                       ///<  Binary logger
  bool                                  m_log_data_binary;              ///<  True if transaction data should be logged by m_binary

};



}  // namespace xtsc



#endif  // _XTSC_COHCTRL_H_

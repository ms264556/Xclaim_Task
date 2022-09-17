#ifndef XTSC_AHB2PIF_BRIDGE_H_
#define XTSC_AHB2PIF_BRIDGE_H_

// Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <map>
#include <deque>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_respond_if.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_response.h>
#include <maxsim.h>
#include <AHB_Transaction.h>
#include <sc_mx_import_module.h>



#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }




/**
 * An AHB-to-PIF bridge.
 *
 * This module implements an AHB-to-PIF bridge.  It is an AHB slave and a PIF master.
 * The PIF side can be connected to the inbound PIF interface of an xtsc_core or to the
 * master interface of an xtsc_router or an xtsc_arbiter.
 *
 * Note: See xtsc_pif2ahb_bridge_sd for a PIF-to-AHB bridge.
 *
 * Here are the parameters for this module:
 *
 *  \verbatim
   Name                   Type  Description
   ------------------     ----  -------------------------------------------------------

   "pif_byte_width"     u32     PIF read/write data interface width in bytes.  Valid
                                values are 4|8|16.

   "ahb_byte_width"     u32     AHB read/write data interface width in bytes.  A value
                                of 0 means to match "pif_byte_width".  Valid values are
                                0|4|8|16.

   "big_endian"         bool    This specifies the data layout on the AHB bus.  
                                Default = false.

   "start_byte_address" u32     The starting byte address of this bridge in the 4GB
                                address space.  This parameter is ignored if the
                                "address_range_file" parameter is used.
  
   "bridge_byte_size"   u32     The byte size of this bridge.  0 means the bridge
                                occupies all of the address space at and above
                                "start_byte_address" (up to 4GB).  This parameter is
                                ignored if the "address_range_file" parameter is used.
  
   "max_burst_size"     u32     The maximum AHB burst size supported.  If an AHB burst
                                request is received whose total size exceeds this value,
                                then an exception will be thrown.  A value of 0xFFFFFFFF
                                means that all AHB burst sizes are accepted.
                                Default = 0xFFFFFFFF (all burst sizes are accepted).

   "request_priority"   u32     The priority of inbound PIF requests.
                                Default = 3 (highest priority).

   "wait_write_response" bool   AHB data phase write beats that complete a PIF write or
                                block write request always get AHB wait states at least
                                until the request is accepted on the PIF.  If
                                "wait_write_response" is true and if the AHB data phase
                                write beat completes a PIF write or block write request
                                with the last_transfer flag set to true, then the AHB
                                data phase write beat will get wait states until the PIF
                                request gets a non-RSP_NACC response.
                                Default = true.

   "address_range_file" char*   Optional name of file containing address ranges (aka
                                address regions) that this bridge serves (that is,
                                address ranges that the AHB bus should route to this
                                bridge).  If this parameter is NULL or empty, then
                                "start_byte_address" and "bridge_byte_size" define the
                                single address range served by this bridge.  If this
                                parameter is neither NULL nor empty, then it must name a
                                file containing lines which specify address ranges that
                                this bridge serves.  The line format is:
                                        <LowAddr> <HighAddr> [<RangeName>]
                                1.  Each line of the text file contains two numbers in
                                    decimal or hexadecimal (with '0x' prefix) format and
                                    an optional address range name.  For example,
                                    // <LowAddr>   <HighAddr>  [<RangeName>]
                                       0x40000000  0x4FFFFFFF
                                       0x70000000  0x7FFFFFFF  SysRAM
                                2.  The first number is the low address and the second
                                    number is the high address in a range of addresses
                                    that this bridge serves.
                                3.  Comments, extra whitespace, blank lines, and lines
                                    between "#if 0" and "#endif" are ignored.  
                                    See xtsc_script_file.

   "pif_clock_period"   u32     This is the length of the PIF's clock period expressed
                                in terms of the SystemC time resolution (from
                                sc_get_time_resolution()).  A value of 0xFFFFFFFF means
                                to use the XTSC system clock period (from
                                xtsc_get_system_clock_period()).  A value of 0 means one
                                delta cycle.
                                Default = 0xFFFFFFFF (i.e. use the system clock period).

   "request_phase"      u32     This is the phase of the PIF clock period at which 
                                requests will be submitted to the PIF.  It is expressed
                                in terms of the SystemC time resolution (from
                                sc_get_time_resolution()) and must be less than the 
                                PIF's clock period.  A value of 0 means to submit
                                PIF requests at the positive edge of the PIF clock.
                                A value of 0xFFFFFFFF means to submit requests to the
                                PIF as soon as they are available from the AHB without
                                synchronizing to any PIF clock phase. 
                                Default = 400.  In a default system this corresponds to
                                a time in between phase B and phase C.  See xtsc_core::
                                set_clock_phase_delta_factors().


   "nacc_wait_time"      u32    This parameter, expressed in terms of the SystemC time
                                resolution, specifies how long to wait after sending a
                                PIF request to see if it was rejected by RSP_NACC.  This
                                value must not exceed this bridge's PIF clock period.  A
                                value of 0 means one delta cycle.  A value of 0xFFFFFFFF
                                means to wait for a period equal to the PIF clock period
                                as specified by "pif_clock_period".
                                CAUTION:  A value of 0 can cause an infinite loop in the
                                simulation if the downstream PIF module requires a 
                                non-zero time to become available.
                                Default = 0xFFFFFFFF (bridge's PIF clock period).


    \endverbatim
 *
 *
 *                          Bridge Theory of Operation
 *
 * During run time the responsibilities of the bridge are distributed across 5 member
 * methods:  read(), write(), update(), request_thead(), and nb_respond().
 *
 * The bridge receives AHB transactions via calls to the read() and write() methods.
 *
 * A good portion of the work for the AHB side of a READ request is handled in the
 * update() method instead of the read() method.  This is because one doesn't know for
 * sure if a wait state occured until the update() callback.
 *
 * The bridge translates the AHB transactions into PIF transactions and delivers them
 * to the PIF via calls to the nb_request() method of m_inbound_pif_request_port.  This
 * occurs in the request_thread() method.
 *
 * The bridge receives responses back from the PIF via calls to nb_respond().
 *
 * Reads:
 * - During the address phase call to the read() method the bridge just records the
 *   necessary information (in m_addr_phase_read and other data members--most of which
 *   have a "_apr" suffix) and waits for the SystemC kernel callback to the updata()
 *   method to act upon the read.  This is because we don't know until the update()
 *   callback whether the data phase of the current clock cycle was a wait state.
 * - During the update() callback if there was a non-sequential (HTRANS==NONSEQ) address
 *   phase read and no wait state, then the sequence of PIF transactions that will be
 *   necessary to satisfy the AHB single read or burst read sequence is computed and the
 *   corresponding request_info objects are created and added to the deque of pending
 *   requests (m_request_info_deque).  Note: Each AHB beat of an unspecified-length
 *   burst (HBURST==INCR) is treated like a single read request.  When any request_info
 *   object is added to the deque of pending requests, the request thread event
 *   (m_requests_pending_event) is notified.  The responsibility for driving the PIF
 *   belongs to the request thread (see Request Thread below).
 * - During the data phase call to the read() method the bridge checks to see if it has
 *   enough response data for this beat (PIF responses are stored in response_info
 *   objects which are accumulated in an sc_fifo called m_response_fifo--see nb_respond()
 *   below).  If it has enough data then the appropriate amount of data is delivered to
 *   the AHB (and "consumed" from the pool of available response data).  If not enough
 *   data has been returned from the PIF then this beat gets a wait state.
 *
 * Writes:
 * - Address phase calls to the write() method are essentially ignored.
 * - During the data phase call to the write() method for new writes (HTRANS==NONSEQ)
 *   the bridge figures out most of the parameters for the sequence of PIF requests
 *   that will be necessary to handle this write or burst write sequence.
 * - During the data phase call to the write() method, data from the AHB bus is
 *   captured and stored in the xtsc_request member of an request_info object.
 *   When an request_info object has the right amount of data, it is added to the
 *   deque of pending requests (m_request_info_deque) and the request thread event
 *   (m_requests_pending_event) is notified.  If this request has its "last transfer"
 *   flag set, then wait states are inserted on the AHB bus until the PIF accepts the
 *   request (m_wait_write_response is false) or until the request receives an RSP_OK
 *   write response from the PIF (m_wait_write_response is true).
 *
 * Request Thread (request_thread()):
 * - The request thread waits upon the event called m_requests_pending_event.  When this
 *   event is notified, the request thread wakes up and services the deque of pending
 *   requests until it is empty.  Servicing the deque of pending requests is done by:
 *     1)  Read next request_info object from the deque.
 *     2)  Optionally, align to a user-specified clock phase (as specified by the
 *         "request_phase" parameter).
 *     3)  Send the request to the PIF by calling nb_request().
 *     4)  Determine when we are done with this request.  For writes when
 *         m_wait_write_response is false or when no last_transfer flag has been set, we
 *         wait for the amount of time specified by the "nacc_wait_time" parameter.  For
 *         writes when m_wait_write_response is true and a last_transfer flag has been set,
 *         we wait until we get the final write response (there can be multiple
 *         outstanding write requests when dealing with width conversion).  For reads,
 *         we wait until we get a response with the "last transfer" flag set.  For both
 *         reads and writes if a NACC is received then we go back to step 3 and resend
 *         the request.  If a NACC is not received then go to step 1.
 *
 * nb_respond():
 * - If an RSP_NACC is received then the request thread is told about it by setting
 *   m_request_got_nacc to true.
 * - When an RSP_OK read response is received, a response_info object is created and 
 *   added to m_response_fifo where it will be consumed by the read() method during a
 *   data phase.
 * - When an RSP_OK write response, then the request thread is told about it by setting
 *   m_write_response_event to true.
 *
 */
class xtsc_ahb2pif_bridge_sd : public sc_mx_import_module {
protected: class sc_mx_transaction_if_impl;
public:

  sc_mx_transaction_if_impl            *m_p_ahb_slave_port;             ///<  Connect AHB master to this
  sc_port  <xtsc::xtsc_request_if>      m_inbound_pif_request_port;     ///<  Bind to PIF slave
  sc_export<xtsc::xtsc_respond_if>      m_inbound_pif_respond_export;   ///<  Bind PIF slave to this

  SC_HAS_PROCESS(xtsc_ahb2pif_bridge_sd);

  /**
   * Constructor for an xtsc_ahb2pif_bridge_sd.
   */
  xtsc_ahb2pif_bridge_sd(sc_mx_m_base* c, const sc_module_name &module_name);


  // The destructor.
  ~xtsc_ahb2pif_bridge_sd(void);


  /// memcpy with optional swizzle (if m_big_endian is true)
  void xmemcpy(void *dst, const void *src, xtsc::u32 size);


  virtual std::string getName() { return "xtsc_ahb2pif_bridge_sd"; }


  /// Register with MxSI kernel
  void interconnect();


  // Overloaded sc_mx_module methods
  string getProperty(MxPropertyType property);
  void setParameter(const string &name, const string &value);
  void init();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);
  void terminate();


  /**
   * Non-hardware reads (for example, reads by the debugger).
   */
  void peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer, bool log = false);


  /**
   * Non-hardware writes (for example, writes from the debugger).
   */
  void poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer, bool log = false);


protected:

  /// Called from MxSI kernel
  void update();

  /// Handle PIF side requests
  void request_thread();

  /// Implementation of sc_mx_transaction_if.
  class sc_mx_transaction_if_impl : public sc_mx_transaction_slave, public sc_object {
  public:

    /**
     * Constructor.
     * @param   port_name   This port's name.
     * @param   bridge      A reference to the owning xtsc_ahb2pif_bridge_sd object.
     */
    sc_mx_transaction_if_impl(const char *port_name, xtsc_ahb2pif_bridge_sd& bridge) :
      sc_mx_transaction_slave   (&bridge, port_name),
      sc_object                 (port_name),
      m_bridge                  (bridge),
      m_p_port                  (0)
    {}

    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }

    // Arbitration functions
    virtual MxGrant requestAccess(MxU64 /*addr*/) { return MX_GRANT_OK; }
    virtual MxGrant checkForGrant(MxU64 /*addr*/) { return MX_GRANT_OK; }

    // Synchronous access functions
    virtual MxStatus read    (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus write   (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus readDbg (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus writeDbg(MxU64 addr, MxU32* value, MxU32* ctrl);

    // Asynchronous access functions
    virtual MxStatus readReq (MxU64 addr, MxU32* value, MxU32* ctrl, MxTransactionCallbackIF* callback);

    // Memory map functions
    virtual int getNumRegions();
    virtual void getAddressRegions(MxU64* start, MxU64* size, std::string* name);

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_ahb2pif_bridge_sd&     m_bridge;       ///< Our xtsc_ahb2pif_bridge_sd object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us


  };


  /// Implementation of xtsc_respond_if.
  class xtsc_respond_if_impl : public xtsc::xtsc_respond_if, public sc_object {
  public:

    /// Constructor
    xtsc_respond_if_impl(const char *object_name, xtsc_ahb2pif_bridge_sd& bridge) :
      sc_object (object_name),
      m_bridge  (bridge),
      m_p_port  (0)
    {}

    /// @see xtsc::xtsc_respond_if
    bool nb_respond(const xtsc::xtsc_response& response);

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_ahb2pif_bridge_sd&     m_bridge;       ///< Our xtsc_ahb2pif_bridge_sd object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };


  /// Class to track address ranges
  class address_range {
  public:
    address_range(xtsc::xtsc_address low_address, xtsc::xtsc_address high_address, const string& name) :
      m_start_address   (low_address),
      m_byte_size       (high_address - low_address + 1ULL),
      m_name            (name)
    {}
    xtsc::xtsc_address  m_start_address;
    xtsc::u64           m_byte_size;
    std::string         m_name;
  };



  /// Information about each request
  class request_info {
  public:
    /// Constructor
    request_info() :
      m_buffer_offset   (0)
    {
      m_num_created += 1;
    }
    void initialize() {
      m_buffer_offset   = 0;
    }
    xtsc::xtsc_request  m_request;              ///< The request
    xtsc::u32           m_buffer_offset;        ///< Advances as data in xtsc_request buffer is added
    static xtsc::u32    m_num_created;          ///< Number of request_info objects created
  };


  /// Get a new xtsc_request object (from the pool)
  request_info *new_request_info();

  /// Delete an xtsc_request (return it to the pool)
  void delete_request_info(request_info*& p_request_info);



  /// Information about each response
  class response_info {
  public:
    /// Constructor
    response_info(const xtsc::xtsc_response& response, request_info *p_request_info) :
      m_response        (response),
      m_p_request_info  (p_request_info),
      m_buffer_offset   (0)
    {
      m_num_created += 1;
    }
    void initialize(const xtsc::xtsc_response& response, request_info *p_request_info) {
      m_response        = response;
      m_p_request_info  = p_request_info;
      m_buffer_offset   = 0;
    }
    xtsc::xtsc_response m_response;             ///< Our copy of the response
    request_info       *m_p_request_info;       ///< Corresponding request
    xtsc::u32           m_buffer_offset;        ///< Advances as data in xtsc_response buffer is used
    static xtsc::u32    m_num_created;          ///< Number of response_info objects created
  };

  /// Get a new response_info (from the pool)
  response_info *new_response_info(const xtsc::xtsc_response& response);

  /// Delete an response_info (return it to the pool)
  void delete_response_info(response_info*& p_response_info);

  // Notes on data member name suffix conventions:
  // _sct  = data of type sc_time
  // _apr  = AHB address phase read
  // _dpw  = AHB data phase write

  xtsc_respond_if_impl                  m_respond_impl;                 ///< m_inbound_pif_respond_export binds to this

  bool                                  m_init_complete;

  xtsc::u32                             m_pif_byte_width;               ///< The byte width of the PIF data interface
  xtsc::u32                             m_ahb_byte_width;               ///< The byte width of the AHB data interface
  bool                                  m_big_endian;                   ///< True if data layout on the bus is big endian
  xtsc::xtsc_address                    m_start_byte_address;           ///< The starting byte address of this bridge in 4GB space
  xtsc::u32                             m_bridge_byte_size;             ///< The byte size of this bridge
  xtsc::u32                             m_max_burst_size;               ///< The maximum total AHB burst size allowed
  xtsc::u8                              m_priority;                     ///< See "priority" parameter
  string                                m_address_range_file;           ///< The name of the optional address range file
  bool                                  m_wait_write_response;          ///< True if bridge waits for write responses

  log4xtensa::TextLogger&               m_text;                         ///< Text logger

  std::vector<address_range*>           m_address_ranges;               ///< Vector of address ranges served by this bridge
  xtsc::xtsc_script_file               *m_p_address_range_stream;       ///< Pointer to the address range file stream
  std::string                           m_line;                         ///< The current script file line
  xtsc::u32                             m_line_count;                   ///< The current script file line number
  std::vector<std::string>              m_words;                        ///< Tokenized words from m_line

  sc_time                               m_time_resolution_sct;          ///< SystemC time resolution
  unsigned int                          m_pif_clock_period;             ///< "pif_clock_period" parameter
  sc_time                               m_pif_clock_period_sct;         ///< "pif_clock_period" as sc_time
  xtsc::u64                             m_pif_clock_period_value;       ///< "pif_clock_period" as u64
  unsigned int                          m_request_phase;                ///< "request_phase" parameter
  sc_time                               m_request_phase_sct;            ///< "request_phase" as sc_time
  sc_time                               m_request_phase_plus_one_sct;   ///< request phase plus one PIF clock period
  unsigned int                          m_nacc_wait_time;               ///< "nacc_wait_time" parameter
  sc_time                               m_nacc_wait_time_sct;           ///< "nacc_wait_time" as sc_time

  xtsc::u64                             m_tag;                          ///< tag for PIF requests associated with the same AHB burst
  request_info                         *m_p_request_info;               ///< A nascent request_info object
  response_info                        *m_p_response_info;              ///< The current response_info object 
  xtsc::u32                             m_current_pif_read_size8;       ///< The byte size of the current/most recent PIF read

  bool                                  m_addr_phase_read;              ///< True if address phase was read, false otherwise
  xtsc::u32                             m_htrans_apr;                   ///< HTRANS for address phase read
  xtsc::u32                             m_hsize_apr;                    ///< HSIZE for address phase read
  xtsc::u32                             m_factor_apr;                   ///< HBURST factor for address phase read (# AHB beats INCR=1)
  bool                                  m_wrapx_apr;                    ///< True if HBURST is WRAP4|WRAP8|WRAP16
  bool                                  m_incr_apr;                     ///< True if address phase read was INCR (unspecified length)
  bool                                  m_single_apr;                   ///< True if address phase read was SINGLE
  xtsc::xtsc_address                    m_haddr_apr;                    ///< HADDR for address phase read
  xtsc::u8                              m_read_buffer[16];              ///< Store PIF read data when AHB is wider then PIF
  xtsc::u32                             m_read_buffer_offset;           ///< Pointer to next available byte in m_read_buffer

  xtsc::u32                             m_factor_dpw;                   ///< HBURST factor for data phase write (# AHB beats INCR=1)
  xtsc::xtsc_request::type_t            m_write_type;                   ///< WRITE or BLOCK_WRITE for current PIF write
  xtsc::u32                             m_write_total_size;             ///< Total size of AHB beats (all beats in burst except INCR=1)
  xtsc::u32                             m_write_num_transfers;          ///< Number of PIF transfers for this BLOCK_WRITE
  xtsc::u32                             m_write_num_blocks;             ///< Number of PIF blocks for this AHB burst write
  xtsc::u32                             m_write_transfer_count;         ///< Number of PIF transfers so far for this BLOCK_WRITE
  xtsc::u32                             m_write_transfer_size;          ///< Number of bytes in each PIF transfer for this AHB burst
  bool                                  m_write_pending;                ///< Previous write got a wait state
  bool                                  m_final_write_got_rsp_ok;       ///< Final outstanding write got RSP_OK response

  bool                                  m_waiting_for_nacc;             ///< True if waiting for RSP_NACC from PIF
  bool                                  m_request_got_nacc;             ///< True if request got RSP_NACC from PIF
  sc_event                              m_last_read_response_event;     ///< When last transfer of read response is received
  sc_event                              m_write_response_event;         ///< Notified when a RSP_OK is received to a write request

  std::deque<request_info*>             m_request_info_deque;           ///< To hold request_info objects in before delivery to PIF
  xtsc::u32                             m_num_outstanding_requests;     ///< Number of requests generated but not yet accepted on PIF

  sc_fifo<response_info*>               m_response_fifo;                ///< Buffer responses from PIF
  sc_event                              m_requests_pending_event;       ///< To notify request_thread
  std::vector<request_info*>            m_request_info_pool;            ///< Maintain a pool of requests to improve performance
  std::vector<response_info*>           m_response_pool;                ///< Maintain a pool of responses to improve performance


  /// Get the next vector of words from the script file
  int get_words();

  /// Extract a u32 value (named argument_name) from the word at m_words[index]
  xtsc::u32 get_u32(xtsc::u32 index, const std::string& argument_name);


public:

  TAHBSignals                          *m_p_signals;                    ///< AHB signals when bridge is used with an AHB bus

};



#endif // XTSC_AHB2PIF_BRIDGE_H_

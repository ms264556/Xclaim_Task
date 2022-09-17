#ifndef XTSC__PIF2AHB_BRIDGE_H_
#define XTSC__PIF2AHB_BRIDGE_H_

// Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
#include <deque>
#include <fstream>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_respond_if.h>
#include <xtsc/xtsc_wire_write_if.h>
#include <xtsc/xtsc_fast_access.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_parms.h>
#include <maxsim.h>
#include <MxMaster.h>
#include <AHB_Transaction.h>
#include <sc_mx_import_module.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


/**
 * A PIF-to-AHB bridge.
 *
 * This module implements a PIF-to-AHB bridge.  It is a PIF slave and an AHB master.
 *
 * Note: See xtsc_ahb2pif_bridge_sd for an AHB-to-PIF bridge.
 *
 * Constructor parameters for an xtsc_pif2ahb_bridge_sd object.
 *
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------------
  
   "pif_byte_width"     u32     PIF read/write data interface width in bytes. Valid 
                                values are 4|8|16.

   "ahb_byte_width"     u32     AHB read/write data interface width in bytes.  A value
                                of 0 means to match "pif_byte_width".  Valid values are
                                0|4|8|16.

   "big_endian"         bool    This specifies the data layout on the AHB bus.  
                                Default = false.

   "immediate_write_response"
                        bool    If true, WRITE and BLOCK_WRITE transactions with the
                                last_transfer flag set get an immediate response without
                                waiting for the write to complete on the AHB bus.  If
                                false the response is not sent until the write completes
                                on the AHB bus.
                                Default = false.

   "ignore_write_errors"
                        bool    This parameter is ignored if "immediate_write_response"
                                is false.  If "immediate_write_response" is true and
                                "ignore_write_errors" is true, then AHB errors from
                                WRITE and BLOCK_WRITE transactions are logged and
                                ignored.  If "immediate_write_response" is true and
                                "ignore_write_errors" is false, then AHB errors from
                                WRITE and BLOCK_WRITE transactions cause an exception to
                                be thrown.
                                Default = false.

   "wrap"               bool    If true, AHB transactions, when appropriate, have an
                                HBURST of WRAP4, WRAP8, or WRAP16.
                                If false, AHB transactions, when appropriate, have an
                                HBURST of INCR4, INCR8, or INCR16.
                                Default = true.

   "hprot"              u32     This specifies the constant value used for the HPROT 
                                signal on the AHB bus.
                                Default = 0x3.

   "lock"               bool    If true, AHB transactions that result from a single
                                PIF READ, BLOCK_READ, or WRITE transfer or from a
                                sequence of related RCW or BLOCK_WRITE transfers will be
                                locked on the AHB.  If false, AHB locking will only be
                                done as specified by "lock_block" and "lock_block_2".
                                Default = false.

   "lock_block"         bool    If true (and "lock" is false), AHB transactions that
                                result from BLOCK_READ and BLOCK_WRITE requests will be
                                locked.  If false (and "lock" is false), AHB locking
                                will only be done as specified by "lock_block_2".  This
                                parameter is ignored if "lock" is true.
                                Default = false.

   "lock_block_2"       bool    BLOCK_READ and BLOCK_WRITE requests with a block size of
                                2 get translated into two single AHB transactions.  If
                                this parameter is true, then the AHB bus will be locked
                                during those two AHB transactions.  If this parameter is
                                false, then the AHB bus will not be locked during those
                                two transations.  This parameter is ignored if "lock" or
                                "lock_block" is true.
                                Default = false.

   "pif_clock_period"   u32     This is the length of the PIF's clock period expressed
                                in terms of the SystemC time resolution (from
                                sc_get_time_resolution()).  A value of 0xFFFFFFFF means
                                to use the XTSC system clock period (from
                                xtsc_get_system_clock_period()).  A value of 0 means one
                                delta cycle.
                                Default = 0xFFFFFFFF (i.e. use the system clock period).

   "delay_from_receipt" bool    If false, the following delay parameters apply from 
                                the start of processing of the request or response
                                (i.e. after all previous requests or all previous
                                responses, as appropriate, have been forwarded).  This
                                models a bridge that can only service one request at a
                                time and one response at a time.  If true, the following
                                delay parameters apply from the time of receipt of the
                                request or response.  This models a bridge with
                                pipelining.
                                Default = true.

   "recovery_time"      u32     If "delay_from_receipt" is true, this specifies two
                                things.  First, the minimum number of clock periods 
                                after a request is forwarded before the next request 
                                will be forwarded.  Second, the minimum number of 
                                clock periods after a response is forwarded before the
                                next response will be forwarded.
                                If "delay_from_receipt" is false, this parameter is
                                ignored.  
                                Default = 1.

   "read_response_delay" u32    The minimum number of clock periods it takes to forward
                                a read response.  If "delay_from_receipt" is true,
                                timing starts when the response is received by the
                                bridge.  If "delay_from_receipt" is false, timing starts
                                at the later of when the response is received and when
                                the previous response was forwarded.  A value of 0 means
                                one delta cycle.  
                                Default = 1.

   "write_response_delay" u32   The minimum number of clock periods it takes to forward
                                a write response.  If "delay_from_receipt" is true,
                                timing starts when the response is received by the
                                bridge.  If "delay_from_receipt" is false, timing starts
                                at the later of when the response is received and when
                                the previous response was forwarded.  A value of 0 means
                                one delta cycle.  
                                Default = 1.

   "response_repeat"    u32     The number of clock periods after a response is sent
                                and rejected before the response will be resent.  A
                                value of 0 means one delta cycle.
                                Default = 1.

   "request_fifo_depth" u32     The request fifo depth.  
                                Default = 2.

   "response_fifo_depth" u32    The response fifo depth.  
                                Default = 2.

    \endverbatim
 *
 *
 *                          Bridge Theory of Operation
 *
 * Each xtsc_request object gets translated into a sequence of one or more ahb_beat_info
 * objects which are stored in m_ahb_beat_info_deque.  Each ahb_beat_info object contains
 * the information needed to drive the two phases (addr and data) of an AHB beat as well
 * as information about when an xtsc_response should be generated.  The deque of AHB
 * beat information serves as a live "script" to drive the communicate() method.
 *
 *
 *
 */
class xtsc_pif2ahb_bridge_sd : public sc_mx_import_module {
public:
  sc_export<xtsc::xtsc_request_if>      m_pif_request_export;   ///<  PIF master binds to this
  sc_port  <xtsc::xtsc_respond_if>      m_pif_respond_port;     ///<  Bind this to PIF master
  sc_port  <xtsc::xtsc_wire_write_if>   m_write_bus_error;      ///<  Signal WriteBusErrorEdgeInt
  MxTransactionMaster                   m_ahb_master_port;      ///<  AHB master port

  SC_HAS_PROCESS(xtsc_pif2ahb_bridge_sd);
      

  /**
   * Constructor for an xtsc_pif2ahb_bridge_sd.
   */
  xtsc_pif2ahb_bridge_sd(sc_mx_m_base* c, const sc_module_name &module_name);

  /// Destructor.
  ~xtsc_pif2ahb_bridge_sd(void);


  /// memcpy with optional swizzle (if m_big_endian is true)
  void xmemcpy(void *dst, const void *src, xtsc::u32 size);


  // overloaded sc_mx_import_module methods
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);


  virtual std::string getName() { return "xtsc_pif2ahb_bridge_sd"; }


  /// Register with MxSI kernel
  void interconnect();


  /// Module debugging method
  void dump_container_info(std::ostream& os = std::cout) const;


protected:

  /// Called from MxSI kernel
  void communicate();


  /// Called from MxSI kernel
  void update();


  /// Thread to handle responses
  void response_thread();


  /// Thread to driver m_write_bus_error
  void write_bus_error_thread();



  /// Implementation of xtsc_request_if.
  class xtsc_request_if_impl : public xtsc::xtsc_request_if, public sc_object {
  public:

    /**
     * Constructor.
     * @param   object_name     Name of this object instance.
     * @param   bridge          A reference to the owning xtsc_pif2ahb_bridge_sd object.
     */
    xtsc_request_if_impl(const char *object_name, xtsc_pif2ahb_bridge_sd& bridge) :
      sc_object         (object_name),
      m_bridge          (bridge),
      m_p_port          (0)
    {}

    /// @see xtsc::xtsc_request_if
    virtual void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /// @see xtsc::xtsc_request_if
    virtual void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /// @see xtsc::xtsc_request_if
    virtual bool nb_fast_access(xtsc::xtsc_fast_access_request  &request);

    /// @see xtsc::xtsc_request_if
    void nb_request(const xtsc::xtsc_request& request);

    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_pif2ahb_bridge_sd&     m_bridge;       ///< Our xtsc_pif2ahb_bridge_sd object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };



  /// Information about each AHB transfer
  class ahb_beat_info {
  public:

    /// Constructor
    ahb_beat_info(bool                          hwrite,
                  MxU32                         haddr,
                  MxU32                         htrans,
                  MxU32                         hlock,
                  MxU32                         hsize,
                  MxU32                         hburst,
                  MxU32                         hprot,
                  MxU32                         hdata[4],
                  bool                          unlock,
                  bool                          is_rcw1,
                  bool                          first_beat,
                  bool                          completes_request,
                  bool                          send_response,
                  bool                          last_transfer,
                  bool                          end_of_burst,
                  xtsc::u32                     offset,
                  const xtsc::xtsc_request     *p_request)
    {
      init(hwrite, haddr, htrans, hlock, hsize, hburst, hprot, hdata, unlock, is_rcw1,
           first_beat, completes_request, send_response, last_transfer, end_of_burst, offset, p_request);
    }

    void init(bool                      hwrite,
              MxU32                     haddr,
              MxU32                     htrans,
              MxU32                     hlock,
              MxU32                     hsize,
              MxU32                     hburst,
              MxU32                     hprot,
              MxU32                     hdata[4],
              bool                      unlock,
              bool                      is_rcw1,
              bool                      first_beat,
              bool                      completes_request,
              bool                      send_response,
              bool                      last_transfer,
              bool                      end_of_burst,
              xtsc::u32                 offset,
              const xtsc::xtsc_request *p_request);

    void dump(std::ostream& os = std::cout, bool dump_data = true) const;

    bool                        m_hwrite;
    MxU32                       m_haddr;
    MxU32                       m_htrans;
    MxU32                       m_do_hlock;                     ///< This beat is part of a locked set of beats
    MxU32                       m_hsize;
    MxU32                       m_hburst;
    MxU32                       m_hprot;
    MxU32                       m_hdata[4];

    AHB_ACCESS_TYPE             m_type;
    bool                        m_do_unlock;                    ///< This beat is last beat of a locked set of beats
    bool                        m_is_rcw1;                      ///< This is 1st beat of an RCW xtsc_request
    bool                        m_first_beat;                   ///< First beat of xtsc_request
    bool                        m_completes_request;            ///< This is last beat corresponding to the xtsc_request 
    bool                        m_send_response;                ///< Send an xtsc_response after this beat completes its data phase
    bool                        m_last_transfer;                ///< The xtsc_response should have its last_transfer flag set
    bool                        m_end_of_burst;                 ///< This beat is the last beat of a burst or SINGLE transfer
    xtsc::u32                   m_offset;                       ///< Offset in xtsc_response buffer for this beat's data
    const xtsc::xtsc_request   *m_p_request;                    ///< Pointer to the xtsc_request object that this beat is part of
  };


  /// Get a new ahb_beat_info (from the pool)
  ahb_beat_info *new_ahb_beat_info(bool                         hwrite,
                                   MxU32                        haddr,
                                   MxU32                        htrans,
                                   MxU32                        hlock,
                                   MxU32                        hsize,
                                   MxU32                        hburst,
                                   MxU32                        hprot,
                                   MxU32                        hdata[4],
                                   bool                         unlock,
                                   bool                         is_rcw1,
                                   bool                         m_first_beat,
                                   bool                         completes_request,
                                   bool                         send_response,
                                   bool                         last_transfer,
                                   bool                         end_of_burst,
                                   xtsc::u32                    offset,
                                   const xtsc::xtsc_request    *p_request);


  /// Delete an ahb_beat_info (return it to the pool)
  void delete_ahb_beat_info(ahb_beat_info*& p_ahb_beat_info);



  /// Information about each request
  class request_info {
  public:
    /// Constructor
    request_info(const xtsc::xtsc_request& request) :
      m_request         (request),
      m_time_stamp      (sc_time_stamp())
    {}
    xtsc::xtsc_request  m_request;              ///< Our copy of the request
    sc_time             m_time_stamp;           ///< Timestamp when received
  };


  /// Get a new request_info (from the pool)
  request_info *new_request_info(const xtsc::xtsc_request& request);


  /// Delete an request_info (return it to the pool)
  void delete_request_info(request_info*& p_request_info);



  /// Information about each response
  class response_info {
  public:
    /// Constructor
    response_info(const xtsc::xtsc_request& request);
    /// Initializer
    void init(const xtsc::xtsc_request& request);

    xtsc::xtsc_response        *m_p_response;           ///< Pointer to our response
    sc_time                     m_time_stamp;           ///< Timestamp when received
    bool                        m_is_read;              ///< Request was READ|BLOCK_READ
    bool                        m_is_write;             ///< Request was WRITE|BLOCK_WRITE
  };


  /// Get a new response_info (from the pool)
  response_info *new_response_info(const xtsc::xtsc_request& request);


  /// Delete an response_info (return it to the pool)
  void delete_response_info(response_info*& p_response_info);



  bool                                  m_init_complete; 
  bool                                  m_reset_called;                         ///<  True if reset() has been called

  unsigned int                          m_pif_clock_period;                     ///<  "pif_clock_period" parameter
  sc_time                               m_pif_clock_period_sct;                 ///<  "pif_clock_period" as sc_time
  unsigned int                          m_recovery_time;                        ///<  "recovery_time" parameter
  sc_time                               m_recovery_time_sct;                    ///<  "recovery_time" as sc_time
  unsigned int                          m_read_response_delay;                  ///<  "read_response_delay" parameter
  sc_time                               m_read_response_delay_sct;              ///<  "read_response_delay" as sc_time
  unsigned int                          m_write_response_delay;                 ///<  "write_response_delay" parameter
  sc_time                               m_write_response_delay_sct;             ///<  "write_response_delay" as sc_time
  unsigned int                          m_response_repeat;                      ///<  "response_repeat" parameter
  sc_time                               m_response_repeat_sct;                  ///<  "response_repeat" as sc_time

  bool                                  m_lock;                                 ///<  Use AHB locking (see "lock" parameter)
  bool                                  m_lock_block;                           ///<  Lock AHB for PIF BLOCK_READ and BLOCK_WRITE 
  bool                                  m_lock_block_2;                         ///<  Lock AHB for PIF blocks with num_transfers of 2
  unsigned int                          m_request_fifo_depth;                   ///<  Depth of m_p_request_fifo
  unsigned int                          m_response_fifo_depth;                  ///<  Depth of m_p_response_fifo


  xtsc::xtsc_response::status_t         m_status;                               ///<  Next response status
  xtsc_request_if_impl                  m_request_impl;                         ///<  The m_pif_request_export binds to this
  log4xtensa::TextLogger&               m_text;                                 ///<  TextLogger
  xtsc::u32                             m_pif_byte_width;                       ///<  The byte width of the PIF data interface
  xtsc::u32                             m_ahb_byte_width;                       ///<  The byte width of the AHB data interface
  bool                                  m_big_endian;                           ///<  True if data layout on the bus is big endian
  xtsc::u32                             m_ahb_width8_mask;                      ///<  m_ahb_byte_width - 1
  xtsc::u32                             m_max_beat_width8;                      ///<  min(pif_width, ahb_width)
  AHB_ACCESS_TYPE                       m_ahb_native_access_type;               ///<  AHB_ACCESS_TYPE corresponding to AHB bus width
  MxU32                                 m_ctrl_peek_poke[AHB_IDX_END];          ///<  For CASI API's peek/poke
  MxU32                                 m_ctrl_default[AHB_IDX_END];            ///<  For default grant and UNLOCK
  MxU32                                 m_ctrl_ap[AHB_IDX_END];                 ///<  For CASI API's address phase
  MxU32                                 m_ctrl_dp[AHB_IDX_END];                 ///<  For CASI API's data phase
  MxU32                                 m_unused_value[4];                      ///<  For CASI API's
  xtsc::u32                             m_hprot;                                ///<  The constant hprot value to use
  xtsc::u32                             m_htrans_next;                          ///<  The next value of htrans
  bool                                  m_wrap;                                 ///<  Use AHB HBURST of WRAPn instead of INCRn
  MxU32                                 m_hlock;                                ///<  set by requestAccess(), clear by ADDR phase
  bool                                  m_do_hlock;                             ///<  This beat set should have hlock
  TAHBSignals                          *m_p_signals;                            ///<  Published pointer to common AHB signal struct
  bool                                  m_immediate_write_response;             ///<  WRITE and BLOCK_WRITE get immediate response
  bool                                  m_ignore_write_errors;                  ///<  Ignore AHB write errors
  bool                                  m_is_rcw1;                              ///<  Current/last generated AHB beat info was RCW #1
  bool                                  m_do_data_phase;                        ///<  This master should read/write the data bus
  bool                                  m_own_addr_bus;                         ///<  This master owned the addr/ctrl bus this cycle
  bool                                  m_own_addr_bus_last;                    ///<  This master owned the addr/ctrl bus last cycle
  bool                                  m_own_addr_bus_next;                    ///<  This master will own the addr/ctrl bus next cycle
  bool                                  m_did_addr_phase;                       ///<  We drove ADDR phase this cycle
  bool                                  m_did_unlock;                           ///<  We drove ADDR phase this cycle for an unlock
  bool                                  m_did_lock_cycle;                       ///<  We drove ADDR phase this cycle for a LOCK
  bool                                  m_arb_to_addr;                          ///<  State machine transition from ARB to ADDR 
  bool                                  m_do_unlock;                            ///<  Do an UNLOCK cycle
  bool                                  m_hwrite;                               ///<  Value from most recent addr phase
  bool                                  m_end_of_burst;                         ///<  Previous ADDR phase was an end of BURST beat
  bool                                  m_early_burst_termination;              ///<  Doing burst which got interrupted (EBT)
  MxU32                                 m_hreq;                                 ///<  hreq bit in previous/current requestAccess() call
  MxU32                                 m_haddr;                                ///<  Value from most recent addr phase
  MxU32                                 m_hsize;                                ///<  Value from most recent addr phase
  bool                                  m_addr_phase_was_rcw1;                  ///<  Address phase was 1st beat of RCW
  bool                                  m_rcw1_got_rsp_address_error;           ///<  1st beat of RCW got an AHB error
  bool                                  m_rcw1_got_rsp_address_error_z1;        ///<  1st beat of RCW got an AHB error prev cycle
  bool                                  m_rcw_read_completed;                   ///<  RCW data phase read completed
  bool                                  m_rcw_read_completed_z1;                ///<  RCW data phase read completed prev cycle
  bool                                  m_rcw_read_data_matched;                ///<  RCW read data matched
  sc_event                              m_response_thread_event;                ///<  To notify response_thread
  sc_event                              m_write_bus_error_event;                ///<  To notify write_bus_error_thread
  sc_dt::sc_unsigned                    m_zero;                                 ///<  Constant 0
  sc_dt::sc_unsigned                    m_one;                                  ///<  Constant 1
  sc_fifo<request_info*>               *m_p_request_fifo;                       ///<  To hold outstanding PIF requests in
  std::deque<ahb_beat_info*>            m_ahb_beat_info_deque;                  ///<  To hold outstanding AHB beats in
  sc_fifo<response_info*>              *m_p_response_fifo;                      ///<  To hold outstanding PIF responses in
  std::vector<request_info*>            m_request_pool;                         ///<  Maintain a pool to improve performance
  response_info                        *m_p_nascent_response_info;              ///<  Holds the response being formulated
  xtsc::u8                             *m_p_nascent_response_buffer;            ///<  The buffer of the nascent response
  std::vector<ahb_beat_info*>           m_ahb_beat_info_pool;                   ///<  Maintain a pool to improve performance
  std::vector<response_info*>           m_response_pool;                        ///<  Maintain a pool to improve performance
  ahb_beat_info                        *m_p_ahb_beat_info_ap;                   ///<  The active (current) ahb_beat_info for addr phase
  ahb_beat_info                        *m_p_ahb_beat_info_dp;                   ///<  The active (current) ahb_beat_info for data phase


  bool                                  m_delay_from_receipt;                   ///<  True if delay starts upon request receipt
  sc_time                               m_last_response_time_stamp;             ///<  Time last response was sent out


  enum {
    IDLE,       ///<  Do nothing
    ARB,        ///<  Arbitration for READ|WRITE
    LOCK,       ///<  Insert IDLE cycle so HLOCK will be high on the cycle prior to the first ADDR phase of a locked set of beats
    ADDR,       ///<  Address phase of READ|WRITE command
    WAIT_RCW,   ///<  Waiting for 1st RCW xtsc_request to complete its data phase (read) and for 2nd RCW xtsc_request to arrive
    UNLOCK,     ///<  IDLE cycle with HLOCK low
  } m_state;    ///<  Processing state 


  /// Get the text string of m_state
  char *get_state_string();

private:
  /// Logic needed in different places in the communicate() method
  void communicate_helper(ahb_beat_info*& p_ahb_beat_info, xtsc::xtsc_response::status_t status, bool immediate_write_response);

  /// Generate the AHB beat information corresponding to this request
  void generate_ahb_beat_info(const xtsc::xtsc_request& request);


friend std::ostream& operator<<(std::ostream& os, const xtsc_pif2ahb_bridge_sd::ahb_beat_info& info);
};




/**
 * Dump an ahb_beat_info object.
 *
 * This operator dumps an ahb_beat_info object using the ahb_beat_info::dump() method.
 *
 */
std::ostream& operator<<(std::ostream& os, const xtsc_pif2ahb_bridge_sd::ahb_beat_info& info);




#endif  // XTSC__PIF2AHB_BRIDGE_H_

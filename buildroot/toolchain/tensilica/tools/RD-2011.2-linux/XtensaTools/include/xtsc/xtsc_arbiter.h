#ifndef _XTSC_ARBITER_H_
#define _XTSC_ARBITER_H_

// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
#include <xtsc/xtsc_cohctrl.h>



namespace xtsc {
class xtsc_core;
}



/**
 * All XTSC component library objects are in the xtsc_component namespace.
 *
 * @Note this does not include xtsc_core which is in the xtsc namespace.
 */
namespace xtsc_component {

class xtsc_dma_engine;
class xtsc_master;
class xtsc_memory_trace;
class xtsc_router;
class xtsc_pin2tlm_memory_transactor;

/**
 * Constructor parameters for a xtsc_arbiter object.
 *
 * This class contains the constructor parameters for a xtsc_arbiter object.
 *  \verbatim
   Name                  Type   Description
   ------------------    ----   --------------------------------------------------
  
   "num_masters"         u32    The number of memory interface masters competing
                                for the memory interface slave.  The arbiter will
                                have this number of memory interface slave port pairs
                                (one for each master to connect with).

   "master_byte_widths" vector<u32>  The byte width of the data interface of each PIF
                                master.  Typically, this and the "slave_byte_width"
                                parameters are left unset and xtsc_arbiter does not
                                concern itself with the byte width of the data interface
                                (it just forwards requests and responses and leaves it
                                to the upstream masters and downstream slave to have
                                matching data interface byte widths).  If desired when
                                modeling a PIF interface, this parameter can be set to
                                indicate the byte widths of each PIF master (in this
                                case the "slave_byte_width" parameter must also be set
                                to indicate the byte width of the downstream PIF slave)
                                and the xtsc_arbiter will act as a PIF width convertor
                                (PWC) to ensure that each request sent out on the
                                request port has the byte width to match the downstream
                                slave and that each response sent out on a response port
                                has a byte width to match the upstream master.  If this
                                parameter is set then "immediate_timing" must be false.
                                If this parameter is set then it must contain exactly
                                "num_masters" entries.
                                Valid entry values are 4|8|16.
                                Default (unset).

   "slave_byte_width"    u32    The PIF data interface byte width of the downstream
                                slave.  Typically, this parameter should be left at
                                its default value of 0; however, if the 
                                "master_byte_widths" parameter is set then this
                                parameter must be set to a non-zero value to indicate
                                the byte width of the downstream PIF slave.
                                Value non-default values are 4|8|16.
                                Default = 0.

   "use_block_requests"  bool   This parameter is only used when acting as a PIF width
                                converter (i.e. when the "master_byte_widths" parameter
                                is set).  By default, the downstream request type is the
                                same as the upstream request type.  If this parameter is
                                set to true when acting as a PIF width converter, then
                                an upstream WRITE|READ request which has all byte lanes
                                enabled and which is larger then the downstream PIF
                                width will be converted into BLOCK_WRITE|BLOCK_READ
                                request(s).
                                Default = false.

   "route_id_lsb"        u32    This parameter specifies the least significant bit of
                                this arbiter's route_id bit field.  Each xtsc_request
                                and xtsc_response object contains a route_id data member
                                that can be accessed using the get_route_id() and
                                set_route_id() methods.  Each arbiter in a communication
                                path must be assigned a bit field in route_id by the
                                system designer. When a request is received, the arbiter
                                fills in its assigned bit field in route_id with the
                                port number that the request arrived on.  When a
                                response comes back, the arbiter uses its bit field in
                                route_id to determine which port to forward the reply
                                out on.
                                Note:  The arbiter will use ceil(log2(num_masters)) 
                                       bits in route_id.
                                Warning:  The system designer must ensure that all
                                          arbiter-like devices in a communication path
                                          (such as xtsc_arbiter and xtsc_cohctrl) use
                                          non-overlapping bit fields in route_id.  The
                                          simulator is not able to detect overlapping
                                          bit fields in route_id and they will probably
                                          result in communication failure.

   "translation_file"    char*  The name of a script file providing an address
                                translation table for each of the memory interface slave
                                port pairs.  If "translation_file" is NULL or empty,
                                then no address translation is performed.
                                Default = NULL.

                                If "translation_file" is neither NULL nor empty, then
                                it must name a script file containing lines in the
                                following format:

                                <PortNum> <LowAddr> [<HighAddr>] <NewBaseAddr>

                                1.  The numbers can be in decimal or hexadecimal (with
                                    '0x' prefix) format.
                                2.  <PortNum> is the memory interface slave port pair
                                    number.
                                3.  <LowAddr> is the low address of an address
                                    range that is to be translated.
                                4.  The optional <HighAddr> is the high address of 
                                    the address range that is to be translated.  If
                                    <HighAddr> is not present, it defaults to
                                    0xFFFFFFFF.
                                5.  <NewBaseAddr> specifies a new base address for
                                    address translation using the formula:
        
                                       NewAddr = OldAddr + <NewBaseAddr> - <LowAddr>

                                6.  The same <PortNum> can appear more than once;
                                    however, address ranges for a given <PortNum>
                                    cannot overlap.
                                7.  Comments, extra whitespace, and blank lines are
                                    ignored.   See xtsc_script_file for a complete list
                                    of pseudo-preprocessor commands.

   "dram_lock"           bool   If true, then use the xtsc_request_if::nb_lock() to
                                lock the arbiter.  This is used to support the
                                DRamNLock functionality.  If false, then pass the
                                nb_lock() call downstream.  If "dram_lock" is true then
                                "immediate_timing" must be false and
                                "master_byte_widths" must be unset.
                                Default = false.

   "external_cbox"       bool   By default, xtsc_arbiter uses a fair, round-robin
                                arbitration policy.  If you want to use xtsc_arbiter as
                                a type of external CBox to connect dual load/store units
                                of an Xtensa to a single local memory, then this
                                parameter can be set to true to cause xtsc_arbiter to
                                modify the arbitration policy such that if there is a
                                pending READ request on one port and a pending WRITE
                                request to the same address on the other port then the
                                READ will always get priority (this is to support the
                                dual load/store unit requirement that a simultaneous
                                read and write to the same address return old data for
                                the read).  If "external_cbox" is true then
                                "immediate_timing" must be false, "master_byte_widths"
                                must be unset, and "num_masters" must be 2.
                                Default = false.

   "xfer_en_port"        u32    By default, xtsc_arbiter uses a fair, round-robin
                                arbitration policy.  If you want to use xtsc_arbiter on
                                a local memory interface with busy of a TX Xtensa
                                core which also has a boot loader interface, then set
                                this parameter to the master port connected to the TX to
                                cause xtsc_arbiter to modify the arbitration policy such
                                that a request received on this port whose get_xfer_en()
                                method returns true will get priority over all other
                                requests.  If "xfer_en_port" is set then "dram_lock",
                                "external_cbox", and "immediate_timing" must be false
                                and "master_byte_widths" must be unset.
                                Default = 0xFFFFFFFF (no special xfer_en handling).

   "immediate_timing"    bool   If true, the following timing parameters are ignored and
                                the arbiter module forwards all requests and responses
                                immediately (without any delay--not even a delta cycle).
                                In this case, there is no arbitration, because the 
                                arbiter forwards all requests immediately.  If false, 
                                the following parameters are used to determine arbiter
                                timing.  This parameter must be false when the arbiter
                                is being used as a PIF width converter.
                                Default = false.

   "request_fifo_depth"  u32    The depth of the request fifos (each memory interface
                                master has its own request fifo).  
                                Default = 2.

   "request_fifo_depths" vector<u32> The depth of each request fifo.  Each memory
                                interface master has its own request fifo.  If this
                                parameter is set it must contain "num_masters" number
                                of values (all non-zero) which will be used to define 
                                the individual request fifo depths in port number order.
                                If this parameter is not set then "request_fifo_depth"
                                (without the trailing s) will define the depth of all
                                the request fifos.  If this parameter is set then
                                "one_at_a_time" should typically be changed to false.
                                Default:  no entries.

   "response_fifo_depth" u32    The depth of the single response fifo.  
                                Default = 2.

   "read_only"           bool   By default, this arbiter supports all transaction types.
                                Set this parameter to true to model a modified PIF
                                interconnect that does not have ReqData pins.  If this
                                parameter is true an exception will be thrown if any of
                                the following types of requests are received (nb_poke
                                calls will still be supported):
                                    WRITE, BLOCK_WRITE, RCW, BURST_WRITE
                                Default:  false

   "write_only"          bool   By default, this arbiter supports all transaction types.
                                Set this parameter to true to model a modified PIF
                                interconnect that does not have RespData pins.  If this
                                parameter is true an exception will be thrown if any of
                                the following types of requests are received (nb_peek
                                calls will still be supported):
                                    READ, BLOCK_READ, RCW, BURST_READ
                                Default:  false

   "clock_period"        u32    This is the length of this arbiter's clock period
                                expressed in terms of the SystemC time resolution
                                (from sc_get_time_resolution()).  A value of 
                                0xFFFFFFFF means to use the XTSC system clock 
                                period (from xtsc_get_system_clock_period()).
                                A value of 0 means one delta cycle.
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
                                RSP_NACC.  This value must not exceed this arbiter's
                                clock period.  A value of 0 means one delta cycle.  A
                                value of 0xFFFFFFFF means to wait for a period equal to
                                this arbiter's clock period.  CAUTION:  A value of 0 can
                                cause an infinite loop in the simulation if the
                                downstream module requires a non-zero time to become
                                available.
                                Default = 0xFFFFFFFF (arbiter's clock period).

   "one_at_a_time"       bool   If true only one request will be accepted by the arbiter
                                at a time (i.e. one for all memory interface masters put
                                together).  If false, each master can have one or more
                                requests pending at one time with the limit on the
                                number of pending requests from each master being
                                determined by the "request_fifo_depth" or
                                "request_fifo_depths" parameters.  If this parameter is
                                true, then "request_delay" and "recovery_time" as it
                                applies to requests are ignored.
                                Default = true.

   "delay_from_receipt"  bool   If false, the following delay parameters apply from 
                                the start of processing of the request or response (i.e.
                                after all previous requests or all previous responses,
                                as appropriate, have been forwarded).  This models a 
                                arbiter that can only service one request at a time 
                                and one response at a time.  If true, the following 
                                delay parameters apply from the time of receipt of 
                                the request or response.  This models an arbiter with
                                pipelining.
                                Default = true.

   "request_delay"       u32    The minimum number of clock periods it takes to forward
                                a request.  If "delay_from_receipt" is true, timing 
                                starts when the request is received by the arbiter.  If 
                                "delay_from_receipt" is false, timing starts at the 
                                later of when the request is received and when the
                                previous request was forwarded.  A value of 0 means one
                                delta cycle.  
                                Default = 1.

   "response_delay"      u32    The minimum number of clock periods it takes to forward
                                a response.  If "delay_from_receipt" is true, timing 
                                starts when the response is received by the arbiter.  If 
                                "delay_from_receipt" is false, timing starts at the 
                                later of when the response is received and when the
                                previous response was forwarded.  A value of 0 means one
                                delta cycle.  
                                Default = 1.

   "response_repeat"     u32    The number of clock periods after a response is sent and
                                rejected before the response will be resent.  A value of 
                                0 means one delta cycle.
                                Default = 1.

   "recovery_time"       u32    If "delay_from_receipt" is true, this specifies two
                                things.  First, the minimum number of clock periods 
                                after a request is forwarded before the next request 
                                will be forwarded (this doesn't apply if "one_at_a_time"
                                is true).  Second, the minimum number of clock periods
                                after a response is forwarded before the next response
                                will be forwarded. If "delay_from_receipt" is false,
                                this parameter is ignored.  
                                Default = 1.

    \endverbatim
 *
 * @see xtsc_arbiter
 * @see xtsc::xtsc_parms
 * @see xtsc::xtsc_script_file
 */
class XTSC_COMP_API xtsc_arbiter_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for an xtsc_arbiter_parms object.
   *
   * @param     num_masters     The number of memory interface masters competing for the
   *                            memory interface slave.  A value of 1 (the default) can
   *                            be used to cause the arbiter to act like a simple
   *                            pass-through delay and/or address-translation device.
   *                            The arbiter will have this number of memory interface
   *                            slave port pairs (one for each master to connect with).
   *
   * @param     route_id_lsb    The least significant bit of this arbiters route_id bit
   *                            field.
   *
   * @param     one_at_a_time   If true, the default, only one request will be accepted
   *                            by the arbiter at a time (i.e. one for all memory
   *                            interface masters put together).  If false, each master
   *                            can have one or more requests pending at one time with
   *                            the number of pending requests for each master being
   *                            determined by the "request_fifo_depth" parameter.
   *
   * @Note  The arbiter will use ceil(log2(num_masters)) bits in the route_id.
   */
  xtsc_arbiter_parms(xtsc::u32 num_masters = 1, xtsc::u32 route_id_lsb = 0, bool one_at_a_time = true) {
    std::vector<xtsc::u32> widths;
    std::vector<xtsc::u32> depths;
    add("num_masters",          num_masters);
    add("master_byte_widths",   widths);
    add("slave_byte_width",     0);
    add("use_block_requests",   false);
    add("route_id_lsb",         route_id_lsb);
    add("translation_file",     (char*) NULL);
    add("dram_lock",            false);
    add("external_cbox",        false);
    add("xfer_en_port",         0xFFFFFFFF);
    add("immediate_timing",     false);
    add("request_fifo_depth",   2);
    add("request_fifo_depths",  depths);
    add("response_fifo_depth",  2);
    add("read_only",            false);
    add("write_only",           false);
    add("clock_period",         0xFFFFFFFF);
    add("posedge_offset",       0xFFFFFFFF);
    add("arbitration_phase",    0xFFFFFFFF);
    add("nacc_wait_time",       0xFFFFFFFF);
    add("one_at_a_time",        one_at_a_time);
    add("delay_from_receipt",   true);
    add("request_delay",        1);
    add("response_delay",       1);
    add("response_repeat",      1);
    add("recovery_time",        1);
  }

  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_arbiter_parms"; }
};




/**
 * A memory interface arbiter and/or address translator.
 *
 * Example XTSC module implementing an arbiter that allows a memory interface slave
 * module (e.g. xtsc_memory or xtsc_mmio) to be accessed by multiple memory interface
 * master modules (e.g. xtsc_core and/or xtsc_master).  All modules involved communicate
 * via the xtsc::xtsc_request_if and xtsc::xtsc_respond_if interfaces.
 *
 * This module does arbitration on a fair, round-robin basis and ignores the priority
 * field in the xtsc::xtsc_request and xtsc::xtsc_response objects (other than to
 * forward them on).  Note: This policy is modified if the "external_cbox" parameter is
 * true or if the "xfer_en_port" parameter is set.
 *
 * This module can also be used to provided address translations.  Each memory interface
 * master can have a different set of address translations applied.  See
 * "translation_file" in xtsc_arbiter_parms.
 *
 * If desired this arbiter can be used as a PIF width converter (PWC) by setting the
 * "master_byte_widths" parameter to indicated the byte width of each upstream PIF
 * master and by setting the "slave_byte_width" parameter to indicate the byte width of
 * the downstream PIF slave.  
 *
 * Limitations of PIF Width Convertor:
 *
 * - Critical word first BLOCK_READ transactions are not supported (i.e. for BLOCK_READ
 *   requests, the start address must be aligned to the total transfer size, not just
 *   the bus width).
 *
 * - When going from a wide master to a narrow slave if an incoming BLOCK_READ request
 *   requires multiple outgoing BLOCK_READ requests then the downstream system must
 *   return all the BLOCK_READ responses in the order the requests were sent out and
 *   without any intervening responses to other requests.
 *
 * When not configured as a PWC, this module supports all memory interface data bus
 * widths and so does not need to be configured for any particular data bus width.
 *
 * Here is a block diagram of an xtsc_arbiter as it is used in the arbiter example:
 * @image html  Example_xtsc_arbiter.jpg
 * @image latex Example_xtsc_arbiter.eps "xtsc_arbiter Example" width=10cm
 *
 * @see xtsc::xtsc_request_if
 * @see xtsc::xtsc_respond_if
 * @see xtsc_arbiter_parms
 */
class XTSC_COMP_API xtsc_arbiter : public sc_core::sc_module, public xtsc::xtsc_resettable {
public:

  sc_core::sc_export<xtsc::xtsc_request_if>   **m_request_exports;      ///< Masters bind to these
  sc_core::sc_port  <xtsc::xtsc_request_if>     m_request_port;         ///< Bind to single slave
  sc_core::sc_export<xtsc::xtsc_respond_if>     m_respond_export;       ///< Single slave binds to this
  sc_core::sc_port  <xtsc::xtsc_respond_if>   **m_respond_ports;        ///< Bind to masters


  // For SystemC
  SC_HAS_PROCESS(xtsc_arbiter);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_arbiter"; }


  /**
   * Constructor for an xtsc_arbiter.
   * @param     module_name     Name of the xtsc_arbiter sc_module.
   * @param     arbiter_parms   The remaining parameters for construction.
   * @see xtsc_arbiter_parms
   */
  xtsc_arbiter(sc_core::sc_module_name module_name, const xtsc_arbiter_parms& arbiter_parms);

  // Destructor.
  ~xtsc_arbiter(void);


  /**
   * Get the number of memory interface masters that can be connected with this
   * xtsc_arbiter (this is the number of memory interface slave port pairs that this
   * xtsc_arbiter has).
   */
  xtsc::u32 get_num_masters() { return m_num_masters; }


  /**
   * Reset the xtsc_arbiter.
   */
  void reset(bool hard_reset = false);


  /**
   * Connect an upstream xtsc_arbiter with this xtsc_arbiter.
   *
   * This method connects the single master port pair of the specified upstream
   * xtsc_arbiter with the specified slave port pair of this xtsc_arbiter.
   *
   * @param     arbiter         The upstream xtsc_arbiter to be connected with this
   *                            xtsc_arbiter.
   *
   * @param     port_num        This specifies the slave port pair of this xtsc_arbiter
   *                            that the single master port pair of the upstream
   *                            xtsc_arbiter is to be connected with.  port_num must
   *                            be in the range of 0 to this xtsc_arbiter's
   *                            "num_masters" parameter minus 1.
   */
  void connect(xtsc_arbiter& arbiter, xtsc::u32 port_num);


  /**
   * Connect with an upstream or downstream xtsc_cohctrl.
   *
   * This method is used to make one of the following three types of connections:
   * #- Connect the memory interface master port pair of the xtsc_cohctrl with the
   *    specified slave port pair of this xtsc_arbiter (type = PT_MEMORY).
   * #- Connect the specified snoop master port pair of the xtsc_cohctrl with the
   *    specified slave port pair of this xtsc_arbiter (type = PT_SNOOP).
   * #- Connect the single master port pair of this xtsc_arbiter with the specified
   *    client slave port pair of the xtsc_cohctrl (type = PT_CLIENT).
   *
   * @param     cohctrl         The xtsc_cohctrl to connect with this xtsc_arbiter.
   *
   * @param     type            Which type of xtsc_cohctrl port to connect with:
   *                            PT_MEMORY, PT_SNOOP, or PT_CLIENT.
   *
   * @param     cohctrl_port    If type is PT_MEMORY, then this parameter is ignored.
   *                            If type is PT_SNOOP, then this specifies which snoop
   *                            master port pair of the xtsc_cohctrl to connect with the
   *                            slave port pair of this xtsc_arbiter specified by
   *                            arbiter_port.
   *                            If type is PT_CLIENT, then this specifies the
   *                            xtsc_cohctrl client slave port pair that the single
   *                            master port pair of this xtsc_arbiter is to be connected
   *                            with.
   *
   * @param     arbiter_port    If type is PT_MEMORY or PT_SNOOP, then this specifies
   *                            the slave port pair of this xtsc_arbiter that is to be
   *                            connected with the xtsc_cohctrl.  If type is PT_CLIENT,
   *                            then this parameter is ignored.
   */
  void connect(xtsc::xtsc_cohctrl& cohctrl, xtsc::xtsc_cohctrl::port_type type, xtsc::u32 cohctrl_port, xtsc::u32 arbiter_port);


  /**
   * Connect with an upstream or downstream (inbound pif) xtsc_core.
   *
   * This method connects this xtsc_arbiter with the memory interface specified by
   * memory_port_name of the xtsc_core specified by core.  If memory_port_name is
   * "inbound_pif" or "snoop" then the master port pair of this xtsc_arbiter is
   * connected with the inbound pif or snoop slave port pair of core.  If
   * memory_port_name is neither "inbound_pif" nor "snoop" then the memory interface
   * master port pair specified by memory_port_name of core is connected with the slave
   * port pair specified by port_num of this xtsc_arbiter.
   *
   * @param     core                    The xtsc_core to connect with.
   *
   * @param     memory_port_name        The memory interface name to connect with.
   *                                    Case-insensitive.
   *
   * @param     port_num                If memory_port_name is neither "inbound_pif" nor
   *                                    "snoop", then the memory interface of core
   *                                    specified by memory_port_name will be connected
   *                                    with the slave port pair of this xtsc_arbiter
   *                                    specified by this parameter.  In this case, this
   *                                    parameter must be explicitly set and must be in
   *                                    the range of 0 to this xtsc_arbiter's
   *                                    "num_masters" parameter minus 1.
   *                                    This parameter is ignored if memory_port_name is
   *                                    "inbound_pif" or "snoop".
   *
   * @see xtsc::xtsc_core::How_to_do_memory_port_binding for a list of valid
   *      memory_port_name values.
   */
  void connect(xtsc::xtsc_core& core, const char *memory_port_name, xtsc::u32 port_num = 0xFFFFFFFF);


  /**
   * Connect an upstream xtsc_dma_engine with this xtsc_arbiter.
   *
   * This method connects the master port pair of the specified xtsc_dma_engine with the
   * specified slave port pair of this xtsc_arbiter.
   *
   * @param     dma_engine      The xtsc_dma_engine to connect with this xtsc_arbiter.
   *
   * @param     port_num        This specifies the slave port pair of this xtsc_arbiter
   *                            that the specified xtsc_dma_engine will be connected with. 
   *                            port_num must be in the range of 0 to this
   *                            xtsc_arbiter's "num_masters" parameter minus 1.
   */
  void connect(xtsc_dma_engine& dma_engine, xtsc::u32 port_num);


  /**
   * Connect an upstream xtsc_master with this xtsc_arbiter.
   *
   * This method connects the master port pair of the specified xtsc_master with the
   * specified slave port pair of this xtsc_arbiter.
   *
   * @param     master          The xtsc_master to connect with this xtsc_arbiter.
   *
   * @param     port_num        This specifies the slave port pair of this xtsc_arbiter
   *                            that the specified xtsc_master will be connected with. 
   *                            port_num must be in the range of 0 to this
   *                            xtsc_arbiter's "num_masters" parameter minus 1.
   */
  void connect(xtsc_master& master, xtsc::u32 port_num);


  /**
   * Connect an upstream xtsc_memory_trace with this xtsc_arbiter.
   *
   * This method connects the specified master port pair of the specified upstream
   * xtsc_memory_trace with the specified slave port pair of this xtsc_arbiter.
   *
   * @param     memory_trace    The upstream xtsc_memory_trace to connect with.
   *
   * @param     trace_port      The master port pair of the upstream xtsc_memory_trace
   *                            to connect with this xtsc_arbiter.  trace_port must be
   *                            in the range of 0 to the upstream xtsc_memory_trace's
   *                            "num_ports" parameter minus 1.
   *
   * @param     arbiter_port    The slave port pair of this xtsc_arbiter to connect the
   *                            xtsc_memory_trace with.  arbiter_port must be in the
   *                            range of 0 to this xtsc_arbiter's "num_masters"
   *                            parameter minus 1.
   */
  void connect(xtsc_memory_trace& memory_trace, xtsc::u32 trace_port, xtsc::u32 arbiter_port);


  /**
   * Connect an upstream xtsc_pin2tlm_memory_transactor with this xtsc_arbiter.
   *
   * This method connects the specified master port pair of the specified upstream 
   * xtsc_pin2tlm_memory_transactor with the specified slave port pair of this
   * xtsc_arbiter.
   *
   * @param     pin2tlm         The upstream xtsc_pin2tlm_memory_transactor to connect
   *                            with this xtsc_arbiter.
   *
   * @param     tran_port       The xtsc_pin2tlm_memory_transactor master port pair to
   *                            connect with this xtsc_arbiter.  tran_port must be in
   *                            the range of 0 to the xtsc_pin2tlm_memory_transactor's
   *                            "num_ports" parameter minus 1.
   *
   * @param     arbiter_port    The slave port pair of this xtsc_arbiter to connect with
   *                            the xtsc_pin2tlm_memory_transactor.  arbiter_port must be
   *                            in the range of 0 to this xtsc_arbiter's "num_masters"
   *                            parameter minus 1.
   */
  void connect(xtsc_pin2tlm_memory_transactor& pin2tlm, xtsc::u32 tran_port, xtsc::u32 arbiter_port);


  /**
   * Connect an upstream xtsc_router with this xtsc_arbiter.
   *
   * This method connects the specified master port pair of the specified upstream
   * xtsc_router with the specified slave port pair of this xtsc_arbiter.
   *
   * @param     router          The upstream xtsc_router to connect with this
   *                            xtsc_arbiter.
   *
   * @param     router_port     The master port pair of the upstream xtsc_router to
   *                            connect with this xtsc_arbiter.  router_port must be in
   *                            the range of 0 to the upstream xtsc_router's
   *                            "num_slaves" parameter minus 1.
   *
   * @param     arbiter_port    The slave port pair of this xtsc_arbiter to connect the
   *                            xtsc_router with.  arbiter_port must be in the range of
   *                            0 to this xtsc_arbiter's "num_masters" parameter minus 1.
   */
  void connect(xtsc_router& router, xtsc::u32 router_port, xtsc::u32 arbiter_port);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


protected:

  class req_rsp_info;
  class response_info;
  class request_info;


  /// Get the next empty Request ID slot 
  xtsc::u8 get_empty_slot();


  /// PWC: Convert a request as required for a narrow/wider downstream PIF
  void convert_request(request_info*& p_request_info, xtsc::u32 master_byte_width, xtsc::u32 port_num);


  /**
   * PWC: Convert a response as required for a narrow/wider upstream PIF.
   * @return true if this was final last transfer.
   */
  bool convert_response(response_info*& p_response_info, xtsc::u32 master_byte_width, req_rsp_info*& p_req_rsp_info);


  /// Implementation of xtsc_request_if.
  class xtsc_request_if_impl : virtual public xtsc::xtsc_request_if, public sc_core::sc_object {
  public:

    /**
     * Constructor.
     * @param   arbiter     A reference to the owning xtsc_arbiter object.
     * @param   port_num    The port number that this object serves.
     */
    xtsc_request_if_impl(const char *object_name, xtsc_arbiter& arbiter, xtsc::u32 port_num) :
      sc_object         (object_name),
      m_arbiter         (arbiter),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// @see xtsc::xtsc_request_if
    virtual void nb_request(const xtsc::xtsc_request& request);

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
    virtual void nb_load_retired(xtsc::xtsc_address address8);

    /// @see xtsc::xtsc_request_if
    virtual void nb_retire_flush();

    /// @see xtsc::xtsc_request_if
    virtual void nb_lock(bool lock);


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_arbiter&               m_arbiter;      ///< Our xtsc_arbiter object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
    xtsc::u32                   m_port_num;     ///< Our port number
  };



  /// Implementation of xtsc_respond_if.
  class xtsc_respond_if_impl : public xtsc::xtsc_respond_if, public sc_core::sc_object {
  public:

    /// Constructor
    xtsc_respond_if_impl(const char *object_name, xtsc_arbiter& arbiter) :
      sc_object (object_name),
      m_arbiter (arbiter),
      m_p_port  (0)
    {}

    /// @see xtsc::xtsc_respond_if
    bool nb_respond(const xtsc::xtsc_response& response);

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_arbiter&               m_arbiter;      ///< Our xtsc_arbiter object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };


  /// Information about each request
  class request_info {
  public:
    /// Constructor
    request_info(const xtsc::xtsc_request& request) :
      m_request         (request),
      m_time_stamp      (sc_core::sc_time_stamp())
    {}
    xtsc::xtsc_request  m_request;              ///< Our copy of the request
    sc_core::sc_time    m_time_stamp;           ///< Timestamp when received
  };


  /// Information about each response
  class response_info {
  public:
    /// Constructor
    response_info(const xtsc::xtsc_response& response) :
      m_response        (response),
      m_time_stamp      (sc_core::sc_time_stamp())
    {}
    /// Constructor for PWC
    response_info(const xtsc::xtsc_request& request) :
      m_response        (request),
      m_time_stamp      (sc_core::sc_time_stamp())
    {}
    xtsc::xtsc_response m_response;             ///< Our copy of the response
    sc_core::sc_time    m_time_stamp;           ///< Timestamp when received
  };

  /// Information for PIF width converter (PWC) mode 
  class req_rsp_info {
  public:
    req_rsp_info() { memset(this, 0, sizeof(req_rsp_info)); }
    request_info       *m_p_first_request_info;         ///< To create the responses for sending upstream
    xtsc::u32           m_num_rsp_received;             ///< To place the data in the upstream response buffer
    xtsc::u32           m_num_last_xfer_rsp_expected;   ///< To determine when the final downstream response has been received
    xtsc::u32           m_num_last_xfer_rsp_received;   ///< To determine when the final downstream response has been received
    xtsc::u32           m_num_block_write_requests;     ///< To determine last_transfer flag of multi-sets and also data offset
    xtsc::xtsc_address  m_block_write_address;          ///< Keep track of next address to be used for downstream requests
    xtsc::u8            m_slot;                         ///< Entry in m_req_rsp_table
    bool                m_responses_sent;               ///< To detect conflicting error responses from multi-sets
    bool                m_single_rsp_error_received;    ///< True if RSP_ADDRESS_ERROR|RSP_ADDRESS_DATA_ERROR received
    request_info       *m_p_nascent_request;            ///< Hold the downstream request being built from multiple BLOCK_WRITE
    response_info      *m_p_nascent_response;           ///< Hold the upstrearm response being built
  };


  /// nb_request helper when m_immediate_timing is true
  void do_request_immediate_timing(xtsc::u32 port_num, const xtsc::xtsc_request& request);

  /// nb_request helper when m_immediate_timing is false
  void do_request(xtsc::u32 port_num, const xtsc::xtsc_request& request);

  /// Handle request
  void handle_request(request_info*& p_active_request_info, xtsc::u32 port_num);

  /// Common routine to nacc all remaining requests when operating with m_one_at_a_time true
  void nacc_remaining_requests();

  /// Handle incoming requests from multiple masters at the correct time
  void arbiter_thread(void);

  /// PWC: Handle incoming requests from multiple masters at the correct time
  virtual void arbiter_pwc_thread(void);

  /// Handle responses from single slave at the correct time
  void response_thread(void);

  /// PWC: Handle responses from single slave at the correct time
  void response_pwc_thread(void);

  /**
   * Given a response transaction, this method determines which port to use to
   * forward the response back to the upstream module that sent the original request.
   */
  xtsc::u32 get_port_from_response(const xtsc::xtsc_response& response);

  /**
   * This method updates the route ID in the request with the bits of the 
   * specified port number so the the response derived from the request
   * will be able to get back to the upstream module that sent xtsc_arbiter
   * this request.
   */
  void add_route_id_bits(xtsc::xtsc_request& request, xtsc::u32 port_num);

  /// Get a new request_info (from the pool)
  request_info *new_request_info(xtsc::u32 port_num, const xtsc::xtsc_request& request);

  /// Copy a new request_info (using the pool)
  request_info *new_request_info(const request_info& info);

  /// Delete an request_info (return it to the pool)
  void delete_request_info(request_info*& p_request_info);

  /// Get a new response_info (from the pool)
  response_info *new_response_info(const xtsc::xtsc_response& response);

  /// Get a new response_info (from the pool)
  response_info *new_response_info(const xtsc::xtsc_request& request);

  /// Delete an response_info (return it to the pool)
  void delete_response_info(response_info*& p_response_info);

  /// Apply address translation if applicable
  xtsc::xtsc_address translate(xtsc::u32 port_num, xtsc::xtsc_address address8);

  /// Get a new req_rsp_info (from the pool)
  req_rsp_info *new_req_rsp_info(request_info *first_request_info);

  /// Delete an req_rsp_info (return it to the pool)
  void delete_req_rsp_info(req_rsp_info*& p_req_rsp_info);


  xtsc_request_if_impl                  **m_request_impl;               ///<  m_request_exports bind to these
  xtsc_respond_if_impl                    m_respond_impl;               ///<  m_respond_export binds to this

  std::deque<request_info*>             **m_request_deques;             ///<  Buffer requests from multiple masters in peekable deque's
  sc_core::sc_fifo<int>                 **m_request_fifos;              ///<  Use sc_fifo to ensure determinancy
  sc_core::sc_fifo<response_info*>        m_response_fifo;              ///<  Buffer responses from single slave

  bool                                    m_is_pwc;                     ///<  True if acting as a PIF width converter
  bool                                    m_use_block_requests;         ///<  PWC: From "use_block_requests" parameter
  std::vector<xtsc::u32>                  m_master_byte_widths;         ///<  PWC: From "master_byte_widths" parameter
  xtsc::u32                               m_slave_byte_width;           ///<  PWC: From "slave_byte_width" parameter
  static const xtsc::u8                   m_num_slots = 64;             ///<  PWC: Number of Request ID's (2^6=64)
  xtsc::u8                                m_next_slot;                  ///<  PWC: Next slot to test for availability
  xtsc::u8                                m_pending_request_id;         ///<  PWC: New ID of pending multi-request (when != m_num_slots)
  xtsc::u8                                m_active_block_read_id;       ///<  PWC: ID of BLOCK_READ request while responses are active
  req_rsp_info                           *m_req_rsp_table[m_num_slots]; ///<  PWC: Table of outstanding requests indexed by request ID
  request_info                           *m_requests[4];                ///<  PWC: List of requests to be sent downstream
  response_info                          *m_responses[4];               ///<  PWC: List of responses to be sent back upstream

  xtsc::u32                               m_num_masters;                ///<  The number of master ports
  bool                                    m_one_at_a_time;              ///<  True if arbiter will only accept one request at a time
  xtsc::u32                               m_route_id_bits_mask;         ///<  Our bit-field in the request route ID
  xtsc::u32                               m_route_id_bits_shift;        ///<  Offset to our bit-field in request route ID
  bool                                    m_do_translation;             ///<  Indicates address translations may apply
  bool                                    m_waiting_for_nacc;           ///<  True if waiting for RSP_NACC from slave
  bool                                    m_request_got_nacc;           ///<  True if active request got RSP_NACC from slave
  xtsc::u32                               m_token;                      ///<  The port number which has the token
  bool                                    m_lock;                       ///<  Lock if non-last_transfer 

  bool                                    m_read_only;                  ///<  From "read_only" parameter
  bool                                    m_write_only;                 ///<  From "write_only" parameter

  sc_core::sc_time                        m_clock_period;               ///<  This arbiter's clock period
  sc_core::sc_time                        m_arbitration_phase;          ///<  Clock phase arbitration occurs
  sc_core::sc_time                        m_arbitration_phase_plus_one; ///<  Clock phase arbitration occurs plus one clock period
  sc_core::sc_time                        m_time_resolution;            ///<  SystemC time resolution
  xtsc::u64                               m_clock_period_value;         ///<  Clock period as u64
  bool                                    m_has_posedge_offset;         ///<  True if m_posedge_offset is non-zero
  sc_core::sc_time                        m_posedge_offset;             ///<  From "posedge_offset" parameter
  xtsc::u64                               m_posedge_offset_value;       ///<  m_posedge_offset as u64

  bool                                    m_dram_lock;                  ///<  See "dram_lock" in xtsc_arbiter_parms
  std::vector<bool>                       m_dram_locks;                 ///<  Current value from nb_lock().  Reset to false.
  bool                                    m_external_cbox;              ///<  See "external_cbox" in xtsc_arbiter_parms
  xtsc::u32                               m_xfer_en_port;               ///<  See "xfer_en_port" in xtsc_arbiter_parms

  bool                                    m_delay_from_receipt;         ///<  True if delay starts upon request receipt
  bool                                    m_immediate_timing;           ///<  True if no delay (not even a delta cycle)
  sc_core::sc_time                        m_last_request_time_stamp;    ///<  Time last request was sent out
  sc_core::sc_time                        m_last_response_time_stamp;   ///<  Time last response was sent out

  sc_core::sc_time                        m_recovery_time;              ///<  See "recovery_time" in xtsc_arbiter_parms
  sc_core::sc_time                        m_request_delay;              ///<  See "request_delay" in xtsc_arbiter_parms
  sc_core::sc_time                        m_nacc_wait_time;             ///<  See "nacc_wait_time" in xtsc_arbiter_parms
  sc_core::sc_time                        m_response_delay;             ///<  See "response_delay" in xtsc_arbiter_parms
  sc_core::sc_time                        m_response_repeat;            ///<  See "response_repeat" in xtsc_arbiter_parms

  sc_core::sc_event                       m_arbiter_thread_event;       ///<  To notify arbiter_thread
  sc_core::sc_event                       m_response_thread_event;      ///<  To notify response_thread

  std::vector<req_rsp_info*>              m_req_rsp_info_pool;          ///<  Maintain a pool of req_rsp_info to improve performance
  std::vector<request_info*>              m_request_pool;               ///<  Maintain a pool of requests to improve performance
  std::vector<response_info*>             m_response_pool;              ///<  Maintain a pool of responses to improve performance

  std::vector<std::vector<xtsc::xtsc_address_range_entry*>*>
                                          m_translation_tables;         ///<  One table of address translations for each master

  log4xtensa::TextLogger&                 m_text;                       ///<  Text logger
  log4xtensa::BinaryLogger&               m_binary;                     ///<  Binary logger
  bool                                    m_log_data_binary;            ///<  True if transaction data should be logged by m_binary

};



}  // namespace xtsc_component 



#endif  // _XTSC_ARBITER_H_

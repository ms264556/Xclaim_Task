// Copyright (c) 2007-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include <iostream>
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_fast_access.h>
#include <xtsc/xtsc_pin2tlm_memory_transactor.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_cohctrl.h>
#include <xtsc/xtsc_arbiter.h>
#include <xtsc/xtsc_slave.h>
#include <xtsc/xtsc_mmio.h>
#include <xtsc/xtsc_router.h>
#include <xtsc/xtsc_tlm2pin_memory_transactor.h>


/**
   Theory of operation:
  
   This module converts a pin-level PIF, inbound PIF, snoop, or local memory interface into a XTSC TLM memory
   interface.  The upstream module is a pin-level memory interface master module and the downstream module is
   a TLM memory interface slave module.  Requests come in on the pins and get converted to xtsc_request objects
   that are passed downstream via a call to nb_request().  Responses come in by means of the downstream TLM
   module calling this modules' nb_respond() method.  This module then converts the xtsc_response to the
   appropriate pin-level response signals.
  
   For local memory interface pins that share a common function, the same data member is used, but it is
   given a SystemC name that exactly matches the Xtensa RTL.  For example, the m_p_en data member is given
   the SystemC name of "DPort0En0" when it is used for XLMI0 load/store unit 0 and it is given the SystemC
   name of "DRam0En0" when it is used for DRAM0 load/store unit 0.  An example data member for a PIF pin is
   m_p_req_adrs which is given the SystemC name of "POReqAdrs".  The comments in this file and the
   corresponding header file generally omit the digits in the signal names (so "DRam0En0", "DRam0En1",
   "DRam1En0", and "DRam1En1" are all referred to as DRamEn)
  
   Multi-ported memories are supported for all memory interfaces.  This is accomplished by having each port
   data member be an array with one entry per memory port.  In addition, one instance of each applicable
   SystemC thread process is created per memory port.  Each thread process knows which memory port it is
   associated with by the local port variable which is created on the stack when the thread process is
   first entered.  This technique does not work for SystemC method processes because they always run to
   completion, so there is only one instance of the dram_lock_method spawned (DRAM only) and it is made
   sensitive to the necessary events of all the memory ports.
  
   When configured for the PIF, inbound PIF, or snoop, the following SystemC threads are used:
     - pif_sample_pin_request_thread
           This thread waits for m_p_req_valid or m_p_req_rdy (representing the POReqValid and PIReqRdy
           signals, respectively) to go high, synchronizes to the sample phase (specified by the
           "sample_phase" parameter), then once each clock period while both m_p_req_valid and m_p_req_rdy
           remain high it reads the pin values, converts them to an xtsc_request, puts the request in a queue
           (m_request_fifo), and notifies pif_send_tlm_request_thread (with a delay of m_delay_output).
     - pif_drive_req_rdy_thread
           This thread drives m_p_req_rdy (representing the PIReqRdy signal) low for one clock period each
           time an nb_respond() call is received with RSP_NACC.  The nb_respond() method notifies this thread
           using m_drive_req_rdy_event.notify(m_delay_output).  To ensure all RSP_NACC calls are accounted
           for, the nb_respond() method adds a dummy value to m_req_rdy_fifo.
     - pif_send_tlm_request_thread
           This thread waits to be notified by pif_sample_pin_request_thread that a request has been
           received and then repeatedly sends the TLM request downstream until it is accepted.  Acceptance
           is defined as no RSP_NACC being received for one clock period after the nb_request() call.
     - pif_drive_pin_response_thread
           This thread waits to be notified by the nb_respond() method that a TLM response has been received
           and then repeatedly drives the pin-level response until it has been accepted.  The nb_respond()
           method notifies this thread using m_pif_resp_event.notify(m_output_delay).  The response
           information is stored in m_pif_resp_fifo.  See the "prereject_responses" parameter.
  
   When configured for a local memory, the following SystemC processes (threads or methods) are used:
     - lcl_request_thread
           This thread waits for m_p_en (representing the DPortEn, DRamEn, DRomEn, IRamEn, or IRomEn signal)
           to go high, synchronizes to the sample phase (specified by the "sample_phase" parameter), then
           once each clock period while m_p_en remains high it reads the pin values, converts them to an
           xtsc_request, and sends it to the downstream TLM model by calling nb_request().
     - lcl_drive_read_data_thread
           This thread drives m_p_data (representing the DPortData, DRamData, DRomData, IRamData, or IRomData 
           signal) for one clock period each time this module's nb_respond() method is called with an
           xtsc_response of RSP_OK.  This thread is activated by the nb_respond() method notifying 
           m_drive_read_data_event.  The m_p_data signal takes its new value at the time after the nb_respond()
           call specified by the "output_delay" parameter.
     - lcl_drive_busy_thread (only if the interface has a busy signal)
           This thread drives m_p_busy (representing the DPortBusy, DRamBusy, DRomBusy, IRamBusy, or IRomBusy
           signal) high for one clock period each time this module's nb_respond() method is called with an
           xtsc_response of RSP_NACC.  This thread is activated by the nb_respond() method notifying 
           m_drive_busy_event. The m_p_busy signal goes high at the time after the nb_respond() call defined by
           the "output_delay" parameter.
     - xlmi_load_retired_thread  (xlmi only)
           This thread waits for m_p_retire (representing the DPortLoadRetired signal) to go high, synchronizes
           to the sample phase (specified by the "sample_phase" parameter), then once each clock period while
           m_p_retire remains high it calls the nb_load_retired() method in the downstream TLM module.
     - xlmi_retire_flush_thread  (xlmi only)
           This thread waits for m_p_flush (representing the DPortRetireFlush signal) to go high, synchronizes
           to the sample phase (specified by the "sample_phase" parameter), then once each clock period while
           m_p_flush remains high it calls the nb_retire_flush() method in the downstream TLM module.
     - dram_lock_method  (dram only)
           Each time the m_p_lock signal changes (representing the DRamLock signal) this method calls the
           nb_lock() method in the downstream TLM module.  There is no attempt to synchronize with the sample
           phase.
  
 */


using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace log4xtensa;
using namespace xtsc;




// Shorthand aliases
typedef xtsc::xtsc_request::type_t              type_t;
typedef sc_core::sc_signal<bool>                bool_signal;
typedef sc_core::sc_signal<sc_dt::sc_uint_base> uint_signal;
typedef sc_core::sc_signal<sc_dt::sc_bv_base>   wide_signal;








// ctor helper
static u32 get_num_ports(const xtsc_component::xtsc_pin2tlm_memory_transactor_parms& pin2tlm_parms) {
  return pin2tlm_parms.get_non_zero_u32("num_ports");
}



// ctor helper
static sc_trace_file *get_trace_file(const xtsc_component::xtsc_pin2tlm_memory_transactor_parms& pin2tlm_parms) {
  return static_cast<sc_trace_file*>(const_cast<void*>(pin2tlm_parms.get_void_pointer("vcd_handle")));
}



// ctor helper
static const char *get_suffix(const xtsc_component::xtsc_pin2tlm_memory_transactor_parms& pin2tlm_parms) {
  const char *port_name_suffix = pin2tlm_parms.get_c_str("port_name_suffix");
  static const char *blank = "";
  return port_name_suffix ? port_name_suffix : blank;
}



xtsc_component::xtsc_pin2tlm_memory_transactor_parms::xtsc_pin2tlm_memory_transactor_parms(const xtsc_core&     core,
                                                                                           const char          *memory_name,
                                                                                           u32                  num_ports)
{
  xtsc_core::memory_port mem_port = xtsc_core::get_memory_port(memory_name);
  if (!core.has_memory_port(mem_port)) {
    ostringstream oss;
    oss << "xtsc_pin2tlm_memory_transactor_parms: xtsc_core '" << core.name() << "' doesn't have a \"" << memory_name
        << "\" memory port.";
    throw xtsc_exception(oss.str());
  }
  xtsc_module_pin_base::memory_interface_type interface_type = xtsc_module_pin_base::get_interface_type(mem_port);


  // Infer number of ports if num_ports is 0
  if (!num_ports) {
    // Normally be single-ported
    num_ports = 1;
    // ; however, if core is dual-ported (has 2 LD/ST units) AND this is port 0 of a dual LD/ST interface AND . . .
    if (core.is_dual_ported(xtsc_core::is_xlmi(mem_port)) && xtsc_core::is_ls_dual_port(mem_port, 0)) {
      // If that 2nd interface appears to not be connected (this is not reliable--it might be delay bound), then
      if (!core.get_request_port(xtsc_core::get_memory_port_name(mem_port+1)).get_interface()) {
        // infer dual ported
        num_ports = 2;
      }
    }
  }


  const xtsc_core_parms& core_parms = core.get_parms();

  xtsc_address  start_address8  = 0;    // PIF|XLMI0
  u32           width8          = core.get_memory_byte_width(mem_port);
  u32           address_bits    = 32;  // PIF|XLMI0

  if (interface_type == xtsc_module_pin_base::PIF) {
    ; // Nothing
  }
  else {
    if (interface_type != xtsc_module_pin_base::XLMI0) {
      core.get_local_memory_starting_byte_address(mem_port, start_address8);
      // Compute address_bits  (size8 must be a power of 2)
      u32 size8 = 0;
      core.get_local_memory_byte_size(mem_port, size8);
      u32 shift_value = size8 / width8;
      for (u32 i=0; i<32; ++i) {
        if (shift_value & 0x1) address_bits = i;
        shift_value >>= 1;
      }
    }
  }

  init(xtsc_module_pin_base::get_interface_name(interface_type), width8, address_bits, num_ports);

  set("clock_period", core_parms.get_u32("SimClockFactor")*xtsc_get_system_clock_factor());
  set("big_endian", core.is_big_endian());
  set("has_busy", core.has_busy(mem_port));
  set("start_byte_address", start_address8);

  if (((interface_type == xtsc_module_pin_base::DRAM0) && core_parms.get_bool("DataRAM0HasRCW")) ||
      ((interface_type == xtsc_module_pin_base::DRAM1) && core_parms.get_bool("DataRAM1HasRCW"))) {
    set("has_lock", true);
  }

  if ((interface_type == xtsc_module_pin_base::DRAM0) ||
      (interface_type == xtsc_module_pin_base::DRAM1) ||
      (interface_type == xtsc_module_pin_base::DROM0))
  {
    if (num_ports == 1) {
      set("cbox", core_parms.get_bool("HasCBox"));
    }
  }

  if (((interface_type == xtsc_module_pin_base::IRAM0) && core_parms.get_bool("InstRAM0HasParity")) ||
      ((interface_type == xtsc_module_pin_base::IRAM1) && core_parms.get_bool("InstRAM1HasParity")) ||
      ((interface_type == xtsc_module_pin_base::DRAM0) && core_parms.get_bool("DataRAM0HasParity")) ||
      ((interface_type == xtsc_module_pin_base::DRAM1) && core_parms.get_bool("DataRAM1HasParity"))) {
    set("check_bits", 1);
  }
  else if (((interface_type == xtsc_module_pin_base::IRAM0) && core_parms.get_bool("InstRAM0HasECC")) ||
           ((interface_type == xtsc_module_pin_base::IRAM1) && core_parms.get_bool("InstRAM1HasECC"))) {
    set("check_bits", width8*7/4);
  }
  else if (((interface_type == xtsc_module_pin_base::DRAM0) && core_parms.get_bool("DataRAM0HasECC")) ||
           ((interface_type == xtsc_module_pin_base::DRAM1) && core_parms.get_bool("DataRAM1HasECC"))) {
    set("check_bits", width8*5);
  }

  if (interface_type == xtsc_module_pin_base::PIF) {
    set("has_pif_attribute", core_parms.get_u32("PIFVersion") >= 32);
  }

}



xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_pin2tlm_memory_transactor(
    sc_module_name                              module_name,
    const xtsc_pin2tlm_memory_transactor_parms& pin2tlm_parms
  ) :
  sc_module             (module_name),
  xtsc_module_pin_base  (*this, ::get_num_ports(pin2tlm_parms), get_trace_file(pin2tlm_parms), get_suffix(pin2tlm_parms)),
  m_num_ports           (pin2tlm_parms.get_u32("num_ports")),
  m_interface_uc        (get_interface_uc(pin2tlm_parms.get_c_str("memory_interface"))),
  m_interface_type      (xtsc_module_pin_base::get_interface_type(m_interface_uc)),
  m_size8               (pin2tlm_parms.get_u32("memory_byte_size")),
  m_width8              (pin2tlm_parms.get_non_zero_u32("byte_width")),
  m_start_byte_address  (pin2tlm_parms.get_u32("start_byte_address")),
  m_inbound_pif         (false),
  m_snoop               (false),
  m_has_coherence       (false),
  m_has_pif_attribute   (false),
  m_address_bits        ((m_interface_type == PIF) ? 32 : pin2tlm_parms.get_non_zero_u32("address_bits")),
  m_check_bits          (pin2tlm_parms.get_u32("check_bits")),
  m_route_id_bits       (pin2tlm_parms.get_u32("route_id_bits")),
  m_address             (m_address_bits),
  m_id                  (m_id_bits),
  m_priority            (2),
  m_route_id            (m_route_id_bits ? m_route_id_bits : 1),
  m_coh_cntl            (2),
  m_data                ((int)m_width8*8),
  m_req_cntl            (0),
  m_resp_cntl           (0),
  m_zero_bv             (1),
  m_zero_uint           (1),
  m_text                (TextLogger::getInstance(name()))
{

  m_zero_bv   = 0;
  m_zero_uint = 0;
  if (m_interface_type == PIF) {
    m_inbound_pif       = pin2tlm_parms.get_bool("inbound_pif");
    m_snoop             = pin2tlm_parms.get_bool("snoop");
    if (m_inbound_pif && m_snoop) {
      ostringstream oss;
      oss << kind() << " '" << name() << "':  You cannot have both \"inbound_pif\" and \"snoop\" set to true";
      throw xtsc_exception(oss.str());
    }
    if (!m_inbound_pif && !m_snoop) {
      m_has_coherence = pin2tlm_parms.get_bool("has_coherence");
    }
    if (!m_snoop) {
      m_has_pif_attribute = pin2tlm_parms.get_bool("has_pif_attribute");
    }
  }
  m_big_endian          = pin2tlm_parms.get_bool("big_endian");
  m_has_request_id      = pin2tlm_parms.get_bool("has_request_id");
  m_has_busy            = pin2tlm_parms.get_bool("has_busy");
  m_has_lock            = pin2tlm_parms.get_bool("has_lock");
  m_has_xfer_en         = pin2tlm_parms.get_bool("has_xfer_en");
  m_bus_addr_bits_mask  = ((m_width8 ==  4) ? 0x03 :
                           (m_width8 ==  8) ? 0x07 : 
                           (m_width8 == 16) ? 0x0F :
                           (m_width8 == 32) ? 0x1F :
                                              0x3F);
  m_cbox                = pin2tlm_parms.get_bool("cbox");
  m_prereject_responses = pin2tlm_parms.get_bool("prereject_responses");
  m_dram_lock_reset     = true;

  m_append_id = true;
  if (m_num_ports == 1) {
    if ((m_interface_type == PIF) || (m_interface_type == IRAM0) || (m_interface_type == IRAM1) || (m_interface_type == IROM0)) {
      m_append_id = false;
    }
    else if (m_cbox) {
      if ((m_interface_type == DRAM0) || (m_interface_type == DRAM1) || (m_interface_type == DROM0)) {
        m_append_id = false;
      }
    }
  }

  if ((m_interface_type == IRAM0) || (m_interface_type == IRAM1) || (m_interface_type == IROM0) || (m_interface_type == PIF)) {
    if ((m_width8 != 4) && (m_width8 != 8) && (m_width8 != 16)) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': Invalid \"byte_width\"= " << m_width8
          << " (legal values for IRAM0|IRAM1|IROM0|PIF are 4, 8, and 16)";
      throw xtsc_exception(oss.str());
    }
  }
  else if ((m_width8 != 4) && (m_width8 != 8) && (m_width8 != 16) && (m_width8 != 32) && (m_width8 != 64)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': Invalid \"byte_width\"= " << m_width8 << " (legal values are 4|8|16|32|64)";
    throw xtsc_exception(oss.str());
  }

  if (m_route_id_bits > 32) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': Invalid \"route_id_bits\"= " << m_route_id_bits << " (max is 32)";
    throw xtsc_exception(oss.str());
  }

  if (m_interface_type == PIF) {
    m_has_busy          = false;
    m_address_shift     = 0;
    m_address_mask      = 0xFFFFFFFF;
  }
  else if (m_interface_type == XLMI0) {
    if (m_size8 == 0) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': \"memory_byte_size\" cannot be 0 for XLMI0";
      throw xtsc_exception(oss.str());
    }
    m_address_shift     = 0;
    u32 phy_addr_bits   = 32;
    u32 shift_value     = m_size8;
    for (u32 i=0; i<32; ++i) {
      if (shift_value & 0x1) phy_addr_bits = i;
      shift_value >>= 1;
    }
    m_address_mask      = (phy_addr_bits >= 32) ? 0xFFFFFFFF : (1 << phy_addr_bits) - 1;
    m_address_mask     &= ((m_width8 ==  4) ? 0xFFFFFFFC :
                           (m_width8 ==  8) ? 0xFFFFFFF8 :
                           (m_width8 == 16) ? 0xFFFFFFF0 :
                           (m_width8 == 32) ? 0xFFFFFFE0 :
                                              0xFFFFFFC0);
  }
  else {
    m_address_shift     = ((m_width8 ==  4) ? 2 :
                           (m_width8 ==  8) ? 3 :
                           (m_width8 == 16) ? 4 :
                           (m_width8 == 32) ? 5 :
                                              6);
    m_address_mask      = ((m_address_bits + m_address_shift) >= 32) ? 0xFFFFFFFF : (1 << (m_address_bits + m_address_shift)) - 1;
  }

  // Get clock period 
  m_time_resolution = sc_get_time_resolution();
  u32 clock_period = pin2tlm_parms.get_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = m_time_resolution * clock_period;
  }
  m_clock_period_value = m_clock_period.value();
  u32 posedge_offset = pin2tlm_parms.get_u32("posedge_offset");
  if (posedge_offset == 0xFFFFFFFF) {
    m_posedge_offset = xtsc_get_system_clock_posedge_offset();
  }
  else {
    m_posedge_offset = posedge_offset * m_time_resolution;
  }
  if (m_posedge_offset >= m_clock_period) {
    ostringstream oss;
    oss << kind() << " '" << name() << "' parameter error:" << endl;
    oss << "\"posedge_offset\" (0x" << hex << posedge_offset << "=>" << m_posedge_offset
        << ") must be strictly less than \"clock_period\" (0x" << clock_period << "=>" << m_clock_period << ")";
    throw xtsc_exception(oss.str());
  }
  m_posedge_offset_value = m_posedge_offset.value();
  m_has_posedge_offset = (m_posedge_offset != SC_ZERO_TIME);

  // Get sample phase
  u32 sample_phase = pin2tlm_parms.get_u32("sample_phase");
  m_sample_phase = sample_phase * m_time_resolution;
  m_sample_phase_plus_one = m_sample_phase + m_clock_period;

  m_output_delay = pin2tlm_parms.get_u32("output_delay") * m_time_resolution;


  // Create all the "per mem port" stuff

  m_request_ports               = new sc_port<xtsc_request_if>*         [m_num_ports];
  m_respond_exports             = new sc_export<xtsc_respond_if>*       [m_num_ports];
  m_respond_impl                = new xtsc_respond_if_impl*             [m_num_ports];
  m_debug_exports               = new sc_export<xtsc_debug_if>*         [m_num_ports];
  m_debug_impl                  = new xtsc_debug_if_impl*               [m_num_ports];
  m_request_fifo                = new deque<request_info*>              [m_num_ports];
  m_pif_resp_fifo               = new deque<xtsc_response*>             [m_num_ports];

  m_current_id                  = 0;
  m_drive_busy_event            = 0;
  m_previous_response_last      = 0;
  m_first_block_write           = 0;
  m_burst_write_transfer_num    = 0;
  m_first_rcw                   = 0;
  m_tag                         = 0;
  m_waiting_for_nacc            = 0;
  m_request_got_nacc            = 0;
  m_last_address                = 0;
  m_read_data_fifo              = 0;
  m_drive_read_data_event       = 0;
  m_busy_fifo                   = 0;
  m_req_rdy_fifo                = 0;
  m_drive_req_rdy_event         = 0;
  m_dram_lock                   = 0;
  m_load_address_deque          = 0;

  if (m_interface_type == PIF) {
    m_current_id                = new u32                               [m_num_ports];
    m_drive_req_rdy_event       = new sc_event                          [m_num_ports];
    m_req_rdy_fifo              = new bool_fifo*                        [m_num_ports];
    m_previous_response_last    = new bool                              [m_num_ports];
    m_first_block_write         = new bool                              [m_num_ports];
    m_burst_write_transfer_num  = new u32                               [m_num_ports];
    m_first_rcw                 = new bool                              [m_num_ports];
    m_tag                       = new u64                               [m_num_ports];
    m_last_address              = new xtsc_address                      [m_num_ports];
    m_waiting_for_nacc          = new bool                              [m_num_ports];
    m_request_got_nacc          = new bool                              [m_num_ports];
  }
  else {
    m_read_data_fifo            = new wide_fifo*                        [m_num_ports];
    m_drive_read_data_event     = new sc_event                          [m_num_ports];
    if (((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) && m_has_lock) {
      m_dram_lock               = new bool                              [m_num_ports];
    }
    if (m_interface_type == XLMI0) {
      m_load_address_deque      = new address_deque                     [m_num_ports];
    }
    if (m_has_busy) {
      m_busy_fifo               = new bool_fifo*                        [m_num_ports];
      m_drive_busy_event        = new sc_event                          [m_num_ports];
    }
  }

  m_pif_req_event               = new sc_event                          [m_num_ports];
  m_pif_resp_event              = new sc_event                          [m_num_ports];


  m_p_req_valid         = NULL;
  m_p_req_cntl          = NULL;
  m_p_req_adrs          = NULL;
  m_p_req_data          = NULL;
  m_p_req_data_be       = NULL;
  m_p_req_id            = NULL;
  m_p_req_priority      = NULL;
  m_p_req_route_id      = NULL;
  m_p_req_attribute     = NULL;
  m_p_req_coh_vadrs     = NULL;
  m_p_req_coh_cntl      = NULL;
  m_p_req_rdy           = NULL;
  m_p_resp_valid        = NULL;
  m_p_resp_cntl         = NULL;
  m_p_resp_data         = NULL;
  m_p_resp_id           = NULL;
  m_p_resp_priority     = NULL;
  m_p_resp_route_id     = NULL;
  m_p_resp_coh_cntl     = NULL;
  m_p_resp_rdy          = NULL;
  m_p_en                = NULL;
  m_p_addr              = NULL;
  m_p_lane              = NULL;
  m_p_wrdata            = NULL;
  m_p_wr                = NULL;
  m_p_load              = NULL;
  m_p_retire            = NULL;
  m_p_flush             = NULL;
  m_p_lock              = NULL;
  m_p_check_wr          = NULL;
  m_p_check             = NULL;
  m_p_xfer_en           = NULL;
  m_p_busy              = NULL;
  m_p_data              = NULL;

  if (m_interface_type == PIF) {
    m_p_req_valid               = new bool_input*                       [m_num_ports];
    m_p_req_cntl                = new uint_input*                       [m_num_ports];
    m_p_req_adrs                = new uint_input*                       [m_num_ports];
    m_p_req_data                = new wide_input*                       [m_num_ports];
    m_p_req_data_be             = new uint_input*                       [m_num_ports];
    m_p_req_id                  = new uint_input*                       [m_num_ports];
    m_p_req_priority            = new uint_input*                       [m_num_ports];
    m_p_req_route_id            = new uint_input*                       [m_num_ports];
    m_p_req_attribute           = new uint_input*                       [m_num_ports];
    m_p_req_coh_vadrs           = new uint_input*                       [m_num_ports];
    m_p_req_coh_cntl            = new uint_input*                       [m_num_ports];
    m_p_req_rdy                 = new bool_output*                      [m_num_ports];
    m_p_resp_valid              = new bool_output*                      [m_num_ports];
    m_p_resp_cntl               = new uint_output*                      [m_num_ports];
    m_p_resp_data               = new wide_output*                      [m_num_ports];
    m_p_resp_id                 = new uint_output*                      [m_num_ports];
    m_p_resp_priority           = new uint_output*                      [m_num_ports];
    m_p_resp_route_id           = new uint_output*                      [m_num_ports];
    m_p_resp_coh_cntl           = new uint_output*                      [m_num_ports];
    m_p_resp_rdy                = new bool_input*                       [m_num_ports];
  }
  else {
    m_p_en                      = new bool_input*                       [m_num_ports];
    m_p_addr                    = new uint_input*                       [m_num_ports];
    m_p_lane                    = new uint_input*                       [m_num_ports];
    m_p_wrdata                  = new wide_input*                       [m_num_ports];
    m_p_wr                      = new bool_input*                       [m_num_ports];
    m_p_load                    = new bool_input*                       [m_num_ports];
    m_p_retire                  = new bool_input*                       [m_num_ports];
    m_p_flush                   = new bool_input*                       [m_num_ports];
    m_p_lock                    = new bool_input*                       [m_num_ports];
    m_p_check_wr                = new wide_input*                       [m_num_ports];
    m_p_check                   = new wide_output*                      [m_num_ports];
    m_p_xfer_en                 = new bool_input*                       [m_num_ports];
    m_p_busy                    = new bool_output*                      [m_num_ports];
    m_p_data                    = new wide_output*                      [m_num_ports];
  }


  for (u32 port=0; port<m_num_ports; ++port) {

    ostringstream oss0;
    oss0 << "m_request_ports[" << port << "]";
    m_request_ports[port]     = new sc_port<xtsc_request_if>(oss0.str().c_str());

    ostringstream oss1;
    oss1 << "m_respond_exports[" << port << "]";
    m_respond_exports[port]     = new sc_export<xtsc_respond_if>(oss1.str().c_str());

    ostringstream oss2;
    oss2 << "m_respond_impl[" << port << "]";
    m_respond_impl[port]        = new xtsc_respond_if_impl(oss2.str().c_str(), *this, port);

    (*m_respond_exports[port])(*m_respond_impl[port]);

    ostringstream oss3;
    oss3 << "m_debug_exports[" << port << "]";
    m_debug_exports[port]       = new sc_export<xtsc_debug_if>(oss3.str().c_str());

    ostringstream oss4;
    oss4 << "m_debug_impl[" << port << "]";
    m_debug_impl[port]          = new xtsc_debug_if_impl(oss4.str().c_str(), *this, port);

    (*m_debug_exports[port])(*m_debug_impl[port]);

    if (m_interface_type == PIF) {
      ostringstream oss2;
      oss2 << "m_req_rdy_fifo[" << port << "]";
      m_req_rdy_fifo[port]      = new bool_fifo(oss2.str().c_str());
    }
    else {
      sc_length_context length(m_width8*8);
      ostringstream oss1;
      oss1 << "m_read_data_fifo[" << port << "]";
      m_read_data_fifo[port]    = new wide_fifo(oss1.str().c_str());
      if (m_has_busy) {
        ostringstream oss2;
        oss2 << "m_busy_fifo[" << port << "]";
        m_busy_fifo[port]       = new bool_fifo(oss2.str().c_str());
      }
    }

    if (m_interface_type == PIF) {
      m_p_req_valid         [port] = NULL;
      m_p_req_cntl          [port] = NULL;
      m_p_req_adrs          [port] = NULL;
      m_p_req_data          [port] = NULL;
      m_p_req_data_be       [port] = NULL;
      m_p_req_id            [port] = NULL;
      m_p_req_priority      [port] = NULL;
      m_p_req_route_id      [port] = NULL;
      m_p_req_attribute     [port] = NULL;
      m_p_req_coh_vadrs     [port] = NULL;
      m_p_req_coh_cntl      [port] = NULL;
      m_p_req_rdy           [port] = NULL;
      m_p_resp_valid        [port] = NULL;
      m_p_resp_cntl         [port] = NULL;
      m_p_resp_data         [port] = NULL;
      m_p_resp_id           [port] = NULL;
      m_p_resp_priority     [port] = NULL;
      m_p_resp_route_id     [port] = NULL;
      m_p_resp_coh_cntl     [port] = NULL;
      m_p_resp_rdy          [port] = NULL;
    }
    else {
      m_p_en                [port] = NULL;
      m_p_addr              [port] = NULL;
      m_p_lane              [port] = NULL;
      m_p_wrdata            [port] = NULL;
      m_p_wr                [port] = NULL;
      m_p_load              [port] = NULL;
      m_p_retire            [port] = NULL;
      m_p_flush             [port] = NULL;
      m_p_lock              [port] = NULL;
      m_p_check_wr          [port] = NULL;
      m_p_check             [port] = NULL;
      m_p_xfer_en           [port] = NULL;
      m_p_busy              [port] = NULL;
      m_p_data              [port] = NULL;
    }

    switch (m_interface_type) {
      case DRAM0: {
        m_p_addr            [port] = &add_uint_input ("DRam0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_input ("DRam0ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_output("DRam0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("DRam0En", m_append_id, port);
        m_p_wr              [port] = &add_bool_input ("DRam0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_input ("DRam0WrData", m_width8*8, m_append_id, port);
        if (m_has_lock) {
          m_p_lock          [port] = &add_bool_input ("DRam0Lock", m_append_id, port);
        }
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_input ("XferDRam0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("DRam0Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_input ("DRam0CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_output("DRam0CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case DRAM1: {
        m_p_addr            [port] = &add_uint_input ("DRam1Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_input ("DRam1ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_output("DRam1Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("DRam1En", m_append_id, port);
        m_p_wr              [port] = &add_bool_input ("DRam1Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_input ("DRam1WrData", m_width8*8, m_append_id, port);
        if (m_has_lock) {
          m_p_lock          [port] = &add_bool_input ("DRam1Lock", m_append_id, port);
        }
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_input ("XferDRam1En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("DRam1Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_input ("DRam1CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_output("DRam1CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case DROM0: {
        m_p_addr            [port] = &add_uint_input ("DRom0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_input ("DRom0ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_output("DRom0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("DRom0En", m_append_id, port);
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("DRom0Busy", m_append_id, port);
        }
        break;
      }
      case IRAM0: {
        m_p_addr            [port] = &add_uint_input ("IRam0Addr", m_address_bits, m_append_id, port);
        m_p_data            [port] = &add_wide_output("IRam0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("IRam0En", m_append_id, port);
        if (m_width8 >= 8) {
          m_p_lane          [port] = &add_uint_input ("IRam0WordEn", (m_width8/4), m_append_id, port);
        }
        m_p_load            [port] = &add_bool_input ("IRam0LoadStore", m_append_id, port);
        m_p_wr              [port] = &add_bool_input ("IRam0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_input ("IRam0WrData", m_width8*8, m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_input ("XferIRam0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("IRam0Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_input ("IRam0CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_output("IRam0CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case IRAM1: {
        m_p_addr            [port] = &add_uint_input ("IRam1Addr", m_address_bits, m_append_id, port);
        m_p_data            [port] = &add_wide_output("IRam1Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("IRam1En", m_append_id, port);
        if (m_width8 >= 8) {
          m_p_lane          [port] = &add_uint_input ("IRam1WordEn", (m_width8/4), m_append_id, port);
        }
        m_p_load            [port] = &add_bool_input ("IRam1LoadStore", m_append_id, port);
        m_p_wr              [port] = &add_bool_input ("IRam1Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_input ("IRam1WrData", m_width8*8, m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_input ("XferIRam1En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("IRam1Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_input ("IRam1CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_output("IRam1CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case IROM0: {
        m_p_addr            [port] = &add_uint_input ("IRom0Addr", m_address_bits, m_append_id, port);
        m_p_data            [port] = &add_wide_output("IRom0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("IRom0En", m_append_id, port);
        if (m_width8 >= 8) {
          m_p_lane          [port] = &add_uint_input ("IRom0WordEn", (m_width8/4), m_append_id, port);
        }
        m_p_load            [port] = &add_bool_input ("IRom0Load", m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_input ("XferIRom0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("IRom0Busy", m_append_id, port);
        }
        break;
      }
      case URAM0: {
        m_p_addr            [port] = &add_uint_input ("URam0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_input ("URam0ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_output("URam0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_input ("URam0En", m_append_id, port);
        m_p_load            [port] = &add_bool_input ("URam0LoadStore", m_append_id, port);
        m_p_wr              [port] = &add_bool_input ("URam0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_input ("URam0WrData", m_width8*8, m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_input ("XferURam0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("URam0Busy", m_append_id, port);
        }
        break;
      }
      case XLMI0: {
        m_p_en              [port] = &add_bool_input ("DPort0En", m_append_id, port);
        m_p_addr            [port] = &add_uint_input ("DPort0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_input ("DPort0ByteEn", m_width8, m_append_id, port);
        m_p_wr              [port] = &add_bool_input ("DPort0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_input ("DPort0WrData", m_width8*8, m_append_id, port);
        m_p_load            [port] = &add_bool_input ("DPort0Load", m_append_id, port);
        m_p_data            [port] = &add_wide_output("DPort0Data", m_width8*8, m_append_id, port);
        m_p_retire          [port] = &add_bool_input ("DPort0LoadRetired", m_append_id, port);
        m_p_flush           [port] = &add_bool_input ("DPort0RetireFlush", m_append_id, port);
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_output("DPort0Busy", m_append_id, port);
        }
        break;
      }
      case PIF: {
        if (m_inbound_pif) {
        m_p_req_valid       [port] = &add_bool_input ("PIReqValid", m_append_id, port);
        m_p_req_cntl        [port] = &add_uint_input ("PIReqCntl", 8, m_append_id, port);
        m_p_req_adrs        [port] = &add_uint_input ("PIReqAdrs", m_address_bits, m_append_id, port);
        m_p_req_data        [port] = &add_wide_input ("PIReqData", m_width8*8, m_append_id, port);
        m_p_req_data_be     [port] = &add_uint_input ("PIReqDataBE", m_width8, m_append_id, port);
        m_p_req_priority    [port] = &add_uint_input ("PIReqPriority", 2, m_append_id, port);
        m_p_req_rdy         [port] = &add_bool_output("POReqRdy", m_append_id, port);
        m_p_resp_valid      [port] = &add_bool_output("PORespValid", m_append_id, port);
        m_p_resp_cntl       [port] = &add_uint_output("PORespCntl", 8, m_append_id, port);
        m_p_resp_data       [port] = &add_wide_output("PORespData", m_width8*8, m_append_id, port);
        m_p_resp_priority   [port] = &add_uint_output("PORespPriority", 2, m_append_id, port);
        m_p_resp_rdy        [port] = &add_bool_input ("PIRespRdy", m_append_id, port);
        if (m_has_request_id) {
          m_p_req_id        [port] = &add_uint_input ("PIReqId", m_id_bits, m_append_id, port);
          m_p_resp_id       [port] = &add_uint_output("PORespId", m_id_bits, m_append_id, port);
        }
        if (m_route_id_bits) {
          m_p_req_route_id  [port] = &add_uint_input ("PIReqRouteId", m_route_id_bits, m_append_id, port);
          m_p_resp_route_id [port] = &add_uint_output("PORespRouteId", m_route_id_bits, m_append_id, port);
        }
        if (m_has_pif_attribute) {
          m_p_req_attribute [port] = &add_uint_input ("PIReqAttribute", 12, m_append_id, port);
        }
        }
        else if (m_snoop) {
        m_p_req_valid       [port] = &add_bool_input ("SnoopReqValid", m_append_id, port);
        m_p_req_cntl        [port] = &add_uint_input ("SnoopReqCntl", 8, m_append_id, port);
        m_p_req_adrs        [port] = &add_uint_input ("SnoopReqAdrs", m_address_bits, m_append_id, port);
        m_p_req_priority    [port] = &add_uint_input ("SnoopReqPriority", 2, m_append_id, port);
        m_p_req_coh_vadrs   [port] = &add_uint_input ("SnoopReqCohVAdrsIndex", 6, m_append_id, port);
        m_p_req_rdy         [port] = &add_bool_output("SnoopReqRdy", m_append_id, port);
        m_p_resp_valid      [port] = &add_bool_output("SnoopRespValid", m_append_id, port);
        m_p_resp_cntl       [port] = &add_uint_output("SnoopRespCntl", 8, m_append_id, port);
        m_p_resp_data       [port] = &add_wide_output("SnoopRespData", m_width8*8, m_append_id, port);
        m_p_resp_rdy        [port] = &add_bool_input ("SnoopRespRdy", m_append_id, port);
        if (m_has_request_id) {
          m_p_req_id        [port] = &add_uint_input ("SnoopReqId", m_id_bits, m_append_id, port);
          m_p_resp_id       [port] = &add_uint_output("SnoopRespId", m_id_bits, m_append_id, port);
        }
        }
        else {
        // outbound PIF
        m_p_req_valid       [port] = &add_bool_input ("POReqValid", m_append_id, port);
        m_p_req_cntl        [port] = &add_uint_input ("POReqCntl", 8, m_append_id, port);
        m_p_req_adrs        [port] = &add_uint_input ("POReqAdrs", m_address_bits, m_append_id, port);
        m_p_req_data        [port] = &add_wide_input ("POReqData", m_width8*8, m_append_id, port);
        m_p_req_data_be     [port] = &add_uint_input ("POReqDataBE", m_width8, m_append_id, port);
        m_p_req_priority    [port] = &add_uint_input ("POReqPriority", 2, m_append_id, port);
        m_p_req_rdy         [port] = &add_bool_output("PIReqRdy", m_append_id, port);
        m_p_resp_valid      [port] = &add_bool_output("PIRespValid", m_append_id, port);
        m_p_resp_cntl       [port] = &add_uint_output("PIRespCntl", 8, m_append_id, port);
        m_p_resp_data       [port] = &add_wide_output("PIRespData", m_width8*8, m_append_id, port);
        m_p_resp_priority   [port] = &add_uint_output("PIRespPriority", 2, m_append_id, port);
        m_p_resp_rdy        [port] = &add_bool_input ("PORespRdy", m_append_id, port);
        if (m_has_request_id) {
          m_p_req_id        [port] = &add_uint_input ("POReqId", m_id_bits, m_append_id, port);
          m_p_resp_id       [port] = &add_uint_output("PIRespId", m_id_bits, m_append_id, port);
        }
        if (m_route_id_bits) {
          m_p_req_route_id  [port] = &add_uint_input ("POReqRouteId", m_route_id_bits, m_append_id, port);
          m_p_resp_route_id [port] = &add_uint_output("PIRespRouteId", m_route_id_bits, m_append_id, port);
        }
        if (m_has_coherence) {
          m_p_req_coh_vadrs [port] = &add_uint_input ("POReqCohVAdrsIndex", 6, m_append_id, port);
          m_p_req_coh_cntl  [port] = &add_uint_input ("POReqCohCntl", 2, m_append_id, port);
          m_p_resp_coh_cntl [port] = &add_uint_output("PIRespCohCntl", 2, m_append_id, port);
        }
        if (m_has_pif_attribute) {
          m_p_req_attribute [port] = &add_uint_input ("POReqAttribute", 12, m_append_id, port);
        }
        }
        break;
      }
    }

  }

  // Squelch SystemC's complaint about multiple thread "objects"
  sc_actions original_action = sc_report_handler::set_actions("object already exists", SC_WARNING, SC_DO_NOTHING);
  for (u32 port=0; port<m_num_ports; ++port) {
    if (m_interface_type == PIF) {
      ostringstream oss1;
      oss1 << "pif_sample_pin_request_thread[" << port << "]";
      declare_thread_process(requestuest_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, pif_sample_pin_request_thread);
      sensitive << m_p_req_valid[port]->pos() << m_p_req_rdy[port]->pos();
      ostringstream oss2;
      oss2 << "pif_drive_req_rdy_thread[" << port << "]";
      declare_thread_process(pif_respond_thread_handle, oss2.str().c_str(), SC_CURRENT_USER_MODULE, pif_drive_req_rdy_thread);
      ostringstream oss3;
      oss3 << "pif_send_tlm_request_thread[" << port << "]";
      declare_thread_process(pif_respond_thread_handle, oss3.str().c_str(), SC_CURRENT_USER_MODULE, pif_send_tlm_request_thread);
      ostringstream oss4;
      oss4 << "pif_drive_pin_response_thread[" << port << "]";
      declare_thread_process(pif_respond_thread_handle, oss4.str().c_str(), SC_CURRENT_USER_MODULE, pif_drive_pin_response_thread);
    }
    else {
      ostringstream oss1;
      oss1 << "lcl_request_thread[" << port << "]";
      declare_thread_process(lcl_request_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, lcl_request_thread);
      sensitive << m_p_en[port]->pos();
      ostringstream oss2;
      oss2 << "lcl_drive_read_data_thread[" << port << "]";
      declare_thread_process(lcl_drive_read_data_thread_handle, oss2.str().c_str(), SC_CURRENT_USER_MODULE, lcl_drive_read_data_thread);
      if (m_interface_type == XLMI0) {
        ostringstream oss3;
        oss3 << "xlmi_load_retired_thread[" << port << "]";
        declare_thread_process(xlmi_load_retired_thread_handle, oss3.str().c_str(), SC_CURRENT_USER_MODULE, xlmi_load_retired_thread);
        sensitive << m_p_retire[port]->pos();
        ostringstream oss4;
        oss4 << "xlmi_retire_flush_thread[" << port << "]";
        declare_thread_process(xlmi_retire_flush_thread_handle, oss4.str().c_str(), SC_CURRENT_USER_MODULE, xlmi_retire_flush_thread);
        sensitive << m_p_flush[port]->pos();
      }
      if (m_has_busy) {
        ostringstream oss5;
        oss5 << "lcl_drive_busy_thread[" << port << "]";
        declare_thread_process(lcl_drive_busy_thread_handle, oss5.str().c_str(), SC_CURRENT_USER_MODULE, lcl_drive_busy_thread);
      }
    }
  }
  // Restore SystemC
  sc_report_handler::set_actions("object already exists", SC_WARNING, original_action);

  if (((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) && m_has_lock) {
    SC_METHOD(dram_lock_method);
    for (u32 port=0; port<m_num_ports; ++port) {
      sensitive << m_p_lock[port]->pos() << m_p_lock[port]->neg();
    }
  }


  // Log our construction
  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, "Constructed " << kind() << " '" << name() << "':");
  XTSC_LOG(m_text, ll, " memory_interface       = "                 << m_interface_uc);
  if (m_interface_type == PIF) {
  XTSC_LOG(m_text, ll, " inbound_pif            = "                 << boolalpha << m_inbound_pif);
  XTSC_LOG(m_text, ll, " snoop                  = "                 << boolalpha << m_snoop);
  if (!m_snoop) {
  XTSC_LOG(m_text, ll, " has_pif_attribute      = "                 << boolalpha << m_has_pif_attribute);
  if (!m_inbound_pif) {
  XTSC_LOG(m_text, ll, " has_coherence          = "                 << boolalpha << m_has_coherence);
  }
  }
  }
  XTSC_LOG(m_text, ll, " num_ports              = "   << dec        << m_num_ports);
  XTSC_LOG(m_text, ll, " port_name_suffix       = "                 << m_suffix);
  XTSC_LOG(m_text, ll, " start_byte_address     = 0x" << hex        << setfill('0') << setw(8) << m_start_byte_address);
  if (m_interface_type == XLMI0) {
  XTSC_LOG(m_text, ll, " memory_byte_size       = 0x" << hex        << m_size8);
  }
  XTSC_LOG(m_text, ll, " byte_width             = "                 << m_width8);
  XTSC_LOG(m_text, ll, " big_endian             = "                 << boolalpha << m_big_endian);
  XTSC_LOG(m_text, ll, " vcd_handle             = "                 << m_p_trace_file);
  if (clock_period == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, " clock_period           = 0xFFFFFFFF => "   << m_clock_period.value() << " (" << m_clock_period << ")");
  } else {
  XTSC_LOG(m_text, ll, " clock_period           = "                 << clock_period << " (" << m_clock_period << ")");
  }
  if (posedge_offset == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, " posedge_offset         = 0xFFFFFFFF => "   << m_posedge_offset.value() << " (" << m_posedge_offset << ")");
  } else {
  XTSC_LOG(m_text, ll, " posedge_offset         = "                 << posedge_offset << " (" << m_posedge_offset << ")");
  }
  XTSC_LOG(m_text, ll, " sample_phase           = "                 << sample_phase << " (" << m_sample_phase << ")");
  if (m_interface_type == PIF) {
  XTSC_LOG(m_text, ll, " has_request_id         = "                 << boolalpha << m_has_request_id);
  XTSC_LOG(m_text, ll, " route_id_bits          = "  << dec         << m_route_id_bits);
  XTSC_LOG(m_text, ll, " prereject_responses    = "                 << boolalpha << m_prereject_responses);
  } else {
  XTSC_LOG(m_text, ll, " address_bits           = "  << dec         << m_address_bits);
  XTSC_LOG(m_text, ll, " has_busy               = "                 << boolalpha << m_has_busy);
  }
  XTSC_LOG(m_text, ll, " output_delay           = "                 << m_output_delay);
  if ((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) {
  XTSC_LOG(m_text, ll, " has_lock               = "                 << boolalpha << m_has_lock);
  }
  if ((m_interface_type == DRAM0) || (m_interface_type == DRAM1) ||
      (m_interface_type == IRAM0) || (m_interface_type == IRAM1)) {
  XTSC_LOG(m_text, ll, " check_bits             = "                 << m_check_bits);
  }
  if ((m_interface_type != DROM0) && (m_interface_type != XLMI0) && (m_interface_type != PIF)) {
  XTSC_LOG(m_text, ll, " has_xfer_en               = "              << boolalpha << m_has_xfer_en);
  }
  XTSC_LOG(m_text, ll, " cbox                   = "                 << boolalpha << m_cbox);
  XTSC_LOG(m_text, ll, " m_address_mask         = 0x" << hex        << m_address_mask);
  XTSC_LOG(m_text, ll, " m_address_shift        = "   << dec        << m_address_shift);
  ostringstream oss;
  oss << "Port List:" << endl;
  dump_ports(oss);
  xtsc_log_multiline(m_text, ll, oss.str(), 2);
}



xtsc_component::xtsc_pin2tlm_memory_transactor::~xtsc_pin2tlm_memory_transactor(void) {
  // Do any required clean-up here
}



string xtsc_component::xtsc_pin2tlm_memory_transactor::adjust_name_and_check_size(
 const string&                               port_name,
 const xtsc_tlm2pin_memory_transactor&       tlm2pin,
 u32                                         tlm2pin_port,
 const set_string&                           transactor_set
) const
{
  string tlm2pin_port_name(port_name);
  if (m_append_id) {
    tlm2pin_port_name.erase(tlm2pin_port_name.size()-1);
  }
  if (tlm2pin.get_append_id()) {
    ostringstream oss;
    oss << tlm2pin_port;
    tlm2pin_port_name += oss.str();
  }
  u32 pin2tlm_width = get_bit_width(port_name);
  u32 tlm2pin_width = tlm2pin.get_bit_width(tlm2pin_port_name);
  if (pin2tlm_width != tlm2pin_width) {
    ostringstream oss;
    oss << "Signal mismatch in connect():  " << kind() << " '" << name() << "' has a port named \"" << port_name
        << "\" with a width of " << pin2tlm_width << " bits, but port \"" << tlm2pin_port_name << "\" in " << tlm2pin.kind()
        << " '" << tlm2pin.name() << "' has " << tlm2pin_width << " bits.";
    throw xtsc_exception(oss.str());
  }
  return tlm2pin_port_name;
} 



void xtsc_component::xtsc_pin2tlm_memory_transactor::dump_set_string(ostringstream&     oss,
                                                                     const set_string&  strings,
                                                                     const string&      indent)
{
  for (set_string::const_iterator is = strings.begin(); is != strings.end(); ++is) {
    oss << indent << *is << endl;
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::connect(xtsc_core& core, u32 pin2tlm_port) {
  if (m_interface_type != PIF) {
    ostringstream oss;
    oss << kind() << " '" << name() << "' has a " << get_interface_name()
        << " interface which cannot be connected to the inbound PIF/snoop interface of an xtsc_core";
    throw xtsc_exception(oss.str());
  }
  if (!m_inbound_pif && !m_snoop) {
    ostringstream oss;
    oss << kind() << " '" << name()
        << "' has a outbound PIF interface which cannot be connected to the inbound PIF/snoop interface of an xtsc_core";
    throw xtsc_exception(oss.str());
  }
  if (pin2tlm_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid pin2tlm_port=" << pin2tlm_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  u32 width8 = core.get_memory_byte_width(xtsc_core::get_memory_port("PIF"));
  if (width8 != m_width8) {
    ostringstream oss;
    oss << "Memory interface data width mismatch: " << endl;
    oss << kind() << " '" << name() << "' is " << m_width8 << " bytes wide, but the inbound PIF/snoop interface of" << endl;
    oss << core.kind() << " '" << core.name() << "' is " << width8 << " bytes wide.";
    throw xtsc_exception(oss.str());
  }
  if (m_snoop) {
    core.get_respond_port("snoop")(*m_respond_exports[pin2tlm_port]);
    (*m_request_ports[pin2tlm_port])(core.get_request_export("snoop"));
  }
  else {
    core.get_respond_port("inbound_pif")(*m_respond_exports[pin2tlm_port]);
    (*m_request_ports[pin2tlm_port])(core.get_request_export("inbound_pif"));
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::connect(xtsc_cohctrl& cohctrl, u32 cohctrl_port, u32 pin2tlm_port) {
  u32 cohctrl_ports = cohctrl.get_num_clients();
  if (cohctrl_port >= cohctrl_ports) {
    ostringstream oss;
    oss << "Invalid cohctrl_port=" << cohctrl_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << cohctrl_ports << " client ports numbered from 0 to " << cohctrl_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (pin2tlm_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid pin2tlm_port=" << pin2tlm_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  (*m_request_ports[pin2tlm_port])(*cohctrl.m_client_exports[cohctrl_port]);
  (*cohctrl.m_client_ports[cohctrl_port])(*m_respond_exports[pin2tlm_port]);
}



u32 xtsc_component::xtsc_pin2tlm_memory_transactor::connect(xtsc_tlm2pin_memory_transactor&     tlm2pin,
                                                            u32                                 tlm2pin_port,
                                                            u32                                 pin2tlm_port,
                                                            bool                                single_connect)
{
  u32 tlm2pin_ports = tlm2pin.get_num_ports();
  if (tlm2pin_port >= tlm2pin_ports) {
    ostringstream oss;
    oss << "Invalid tlm2pin_port=" << tlm2pin_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << tlm2pin_ports << " ports numbered from 0 to " << tlm2pin_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (pin2tlm_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid pin2tlm_port=" << pin2tlm_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (m_interface_type != tlm2pin.get_interface_type()) {
    ostringstream oss;
    oss << "Interface type mismatch in connect(): " << kind() << " '" << name() << "' has a " << get_interface_name() << endl;
    oss << " interface but the upstream device, " << tlm2pin.kind() << " '" << tlm2pin.name() << "', has a "
        << tlm2pin.get_interface_name() << " interface.";
    throw xtsc_exception(oss.str());
  }

  u32 num_connected = 0;

  while ((tlm2pin_port < tlm2pin_ports) && (pin2tlm_port < m_num_ports)) {

    {
      // Connect sc_in<bool> ports of xtsc_pin2tlm_memory_transactor to sc_out<bool> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_bool_input_set(pin2tlm_port);
      set_string tran_set = tlm2pin.get_bool_output_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_pin2tlm_memory_transactor::connect():" << endl;
        oss << kind() << " '" << name() << "':" << endl;
        dump_set_string(oss, mem_set, " ");
        oss << tlm2pin.kind() << " '" << tlm2pin.name() << "':" << endl;
        dump_set_string(oss, tran_set, " ");
        throw xtsc_exception(oss.str());
      }
      for (set_string::const_iterator is = mem_set.begin(); is != mem_set.end(); ++is) {
        string their_name = adjust_name_and_check_size(*is, tlm2pin, tlm2pin_port, tran_set);
        string signal_name = name() + ("__" + *is);
        bool_signal& signal = create_bool_signal(signal_name);
        get_bool_input(*is)(signal);
        tlm2pin.get_bool_output(their_name)(signal);
      }
    }


    {
      // Connect sc_in<sc_uint_base> ports of xtsc_pin2tlm_memory_transactor to sc_out<sc_uint_base> ports of
      // xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_uint_input_set(pin2tlm_port);
      set_string tran_set = tlm2pin.get_uint_output_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_pin2tlm_memory_transactor::connect():" << endl;
        oss << kind() << " '" << name() << "':" << endl;
        dump_set_string(oss, mem_set, " ");
        oss << tlm2pin.kind() << " '" << tlm2pin.name() << "':" << endl;
        dump_set_string(oss, tran_set, " ");
        throw xtsc_exception(oss.str());
      }
      for (set_string::const_iterator is = mem_set.begin(); is != mem_set.end(); ++is) {
        string their_name = adjust_name_and_check_size(*is, tlm2pin, tlm2pin_port, tran_set);
        string signal_name = name() + ("__" + *is);
        uint_signal& signal = create_uint_signal(signal_name, get_bit_width(*is));
        get_uint_input(*is)(signal);
        tlm2pin.get_uint_output(their_name)(signal);
      }
    }


    {
      // Connect sc_in<sc_bv_base> ports of xtsc_pin2tlm_memory_transactor to sc_out<sc_bv_base> ports of
      // xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_wide_input_set(pin2tlm_port);
      set_string tran_set = tlm2pin.get_wide_output_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_pin2tlm_memory_transactor::connect():" << endl;
        oss << kind() << " '" << name() << "':" << endl;
        dump_set_string(oss, mem_set, " ");
        oss << tlm2pin.kind() << " '" << tlm2pin.name() << "':" << endl;
        dump_set_string(oss, tran_set, " ");
        throw xtsc_exception(oss.str());
      }
      for (set_string::const_iterator is = mem_set.begin(); is != mem_set.end(); ++is) {
        string their_name = adjust_name_and_check_size(*is, tlm2pin, tlm2pin_port, tran_set);
        string signal_name = name() + ("__" + *is);
        wide_signal& signal = create_wide_signal(signal_name, get_bit_width(*is));
        get_wide_input(*is)(signal);
        tlm2pin.get_wide_output(their_name)(signal);
      }
    }


    {
      // Connect sc_out<bool> ports of xtsc_pin2tlm_memory_transactor to sc_in<bool> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_bool_output_set(pin2tlm_port);
      set_string tran_set = tlm2pin.get_bool_input_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_pin2tlm_memory_transactor::connect():" << endl;
        oss << kind() << " '" << name() << "':" << endl;
        dump_set_string(oss, mem_set, " ");
        oss << tlm2pin.kind() << " '" << tlm2pin.name() << "':" << endl;
        dump_set_string(oss, tran_set, " ");
        throw xtsc_exception(oss.str());
      }
      for (set_string::const_iterator is = mem_set.begin(); is != mem_set.end(); ++is) {
        string their_name = adjust_name_and_check_size(*is, tlm2pin, tlm2pin_port, tran_set);
        string signal_name = name() + ("__" + *is);
        bool_signal& signal = create_bool_signal(signal_name);
        get_bool_output(*is)(signal);
        tlm2pin.get_bool_input(their_name)(signal);
      }
    }


    {
      // Connect sc_out<sc_uint_base> ports of xtsc_pin2tlm_memory_transactor to sc_in<sc_uint_base> ports of
      // xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_uint_output_set(pin2tlm_port);
      set_string tran_set = tlm2pin.get_uint_input_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_pin2tlm_memory_transactor::connect():" << endl;
        oss << kind() << " '" << name() << "':" << endl;
        dump_set_string(oss, mem_set, " ");
        oss << tlm2pin.kind() << " '" << tlm2pin.name() << "':" << endl;
        dump_set_string(oss, tran_set, " ");
        throw xtsc_exception(oss.str());
      }
      for (set_string::const_iterator is = mem_set.begin(); is != mem_set.end(); ++is) {
        string their_name = adjust_name_and_check_size(*is, tlm2pin, tlm2pin_port, tran_set);
        string signal_name = name() + ("__" + *is);
        uint_signal& signal = create_uint_signal(signal_name, get_bit_width(*is));
        get_uint_output(*is)(signal);
        tlm2pin.get_uint_input(their_name)(signal);
      }
    }


    {
      // Connect sc_out<sc_bv_base> ports of xtsc_pin2tlm_memory_transactor to sc_in<sc_bv_base> ports of
      // xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_wide_output_set(pin2tlm_port);
      set_string tran_set = tlm2pin.get_wide_input_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_pin2tlm_memory_transactor::connect():" << endl;
        oss << kind() << " '" << name() << "':" << endl;
        dump_set_string(oss, mem_set, " ");
        oss << tlm2pin.kind() << " '" << tlm2pin.name() << "':" << endl;
        dump_set_string(oss, tran_set, " ");
        throw xtsc_exception(oss.str());
      }
      for (set_string::const_iterator is = mem_set.begin(); is != mem_set.end(); ++is) {
        string their_name = adjust_name_and_check_size(*is, tlm2pin, tlm2pin_port, tran_set);
        string signal_name = name() + ("__" + *is);
        wide_signal& signal = create_wide_signal(signal_name, get_bit_width(*is));
        get_wide_output(*is)(signal);
        tlm2pin.get_wide_input(their_name)(signal);
      }
    }

    // Connect the debug interface
    (*tlm2pin.m_debug_ports[tlm2pin_port])(*m_debug_exports[pin2tlm_port]);

    pin2tlm_port += 1;
    tlm2pin_port += 1;
    num_connected += 1;

    if (single_connect) break;
    if (tlm2pin_port >= tlm2pin_ports) break;
    if (pin2tlm_port >= m_num_ports) break;
    if (m_debug_impl[pin2tlm_port]->is_connected()) break;
  }

  return num_connected;
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::end_of_elaboration(void) {
} 



void xtsc_component::xtsc_pin2tlm_memory_transactor::start_of_simulation(void) {
  reset();
} 



void xtsc_component::xtsc_pin2tlm_memory_transactor::reset(bool /* hard_reset */) {
  XTSC_INFO(m_text, kind() << "::reset()");

  m_next_port_lcl_request_thread                = 0;
  m_next_port_pif_sample_pin_request_thread     = 0;
  m_next_port_pif_drive_req_rdy_thread          = 0;
  m_next_port_pif_send_tlm_request_thread       = 0;
  m_next_port_pif_drive_pin_response_thread     = 0;
  m_next_port_lcl_drive_read_data_thread        = 0;
  m_next_port_lcl_drive_busy_thread             = 0;
  m_next_port_xlmi_load_retired_thread          = 0;
  m_next_port_xlmi_retire_flush_thread          = 0;

  for (u32 port=0; port<m_num_ports; ++port) {
    m_request_fifo[port].clear();
    if (m_interface_type == PIF) {
      m_p_req_rdy[port]->write(true);
      m_p_resp_valid[port]->write(false);
      m_first_block_write[port]         = true;
      m_burst_write_transfer_num[port]  = 1;
      m_first_rcw[port]                 = true;
      m_tag[port]                       = 0;
      m_waiting_for_nacc[port]          = false;
      m_request_got_nacc[port]          = false;
    }
    else if (((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) && m_has_lock) {
      m_dram_lock[port] = false;
    }
    else if (m_interface_type == XLMI0) {
      m_load_address_deque[port].clear();
    }
    if (m_p_busy && m_p_busy[port]) {
      m_p_busy[port]->write(false);
    }
  }

  m_dram_lock_reset     = true;

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::get_read_data_from_response(const xtsc_response& response) {
  m_data = 0;
  u32 size = response.get_byte_size();
  if (size) {
    xtsc_address  address8        = response.get_byte_address();
    u32           bus_addr_bytes  = address8 & (m_width8 - 1);
    u32           bit_offset      = (m_big_endian ? (m_width8 - 1 - bus_addr_bytes) : bus_addr_bytes) * 8;
    u32           delta           = (m_big_endian ? -8 : 8);
    const u8     *buffer          = response.get_buffer();
    u32           max_offset      = (m_width8 - 1) * 8; // Prevent SystemC "E5 out of bounds" - (should be an Aerr response)
    for (u32 i=0; (i<size) && (bit_offset<=max_offset); ++i) {
      m_data.range(bit_offset+7, bit_offset) = buffer[i];
      bit_offset += delta;
    }
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::sync_to_sample_phase(void) {
  sc_time now = sc_time_stamp();
  sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
  if (m_has_posedge_offset) {
    if (phase_now < m_posedge_offset) {
      phase_now += m_clock_period;
    }
    phase_now -= m_posedge_offset;
  }
  if (phase_now < m_sample_phase) {
    wait(m_sample_phase - phase_now);
  }
  else  {
    wait(m_sample_phase_plus_one - phase_now);
  }
} 



void xtsc_component::xtsc_pin2tlm_memory_transactor::lcl_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_request_thread++;

  // A try/catch block in sc_main will not catch an exception thrown from
  // an SC_THREAD, so we'll catch them here, log them, then rethrow them.
  try {

    XTSC_INFO(m_text, "in lcl_request_thread[" << port << "]");

    // If present, drive 0 on parity/ECC signal
    if (m_p_check[port]) {
      sc_bv_base dummy((int)m_check_bits);
      dummy = 0;
      m_p_check[port]->write(dummy);
    }

    // Loop forever
    while (true) {

      // Wait for enable to go high: m_p_en[port]->pos()
      wait();

      XTSC_DEBUG(m_text, "lcl_request_thread(): awoke from wait()");

      // Sync to sample phase
      sync_to_sample_phase();

      XTSC_DEBUG(m_text, "lcl_request_thread(): completed sync_to_sample_phase()");

      // Capture a request once each clock period that enable is high
      while (m_p_en[port]->read()) {

        request_info *p_info = new_request_info(port);

        XTSC_INFO(m_text, p_info->m_request);

        m_request_fifo[port].push_back(p_info);
        (*m_request_ports[port])->nb_request(p_info->m_request);

        wait(m_clock_period);

      }

      XTSC_DEBUG(m_text, "lcl_request_thread(): enable/valid is low");

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in lcl_request_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::lcl_drive_read_data_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_drive_read_data_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_drive_read_data_thread[" << port << "]");

    // Loop forever
    while (true) {
      // Wait to be notified
      wait(m_drive_read_data_event[port]);
      while (m_read_data_fifo[port]->num_available()) {
        m_read_data_fifo[port]->nb_read(m_data);
        m_p_data[port]->write(m_data);
        XTSC_DEBUG(m_text, "lcl_drive_read_data_thread[" << port << "] driving read data 0x" << hex << m_data);
        wait(m_clock_period);
        if (!m_read_data_fifo[port]->num_available()) {
          m_data = 0;
          m_p_data[port]->write(m_data);
        }
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in lcl_drive_read_data_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::lcl_drive_busy_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_drive_busy_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_drive_busy_thread[" << port << "]");

    // Loop forever
    while (true) {
      // Wait to be notified
      wait(m_drive_busy_event[port]);
      while (m_busy_fifo[port]->num_available()) {
        bool dummy;
        m_busy_fifo[port]->nb_read(dummy);
        m_p_busy[port]->write(true);
        XTSC_DEBUG(m_text, "lcl_drive_busy_thread[" << port << "] asserting busy");
        wait(m_clock_period);
        if (!m_busy_fifo[port]->num_available()) {
          m_p_busy[port]->write(false);
          XTSC_DEBUG(m_text, "lcl_drive_busy_thread[" << port << "] deasserting busy");
        }
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in lcl_drive_busy_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::xlmi_load_retired_thread() {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_xlmi_load_retired_thread++;

  try {

    XTSC_INFO(m_text, "in xlmi_load_retired_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for DPortLoadRetired to go high: m_p_retire[port]->pos()
      wait();

      // Sync to sample phase
      sync_to_sample_phase();

      // Call nb_load_retired() once each clock period while DPort0LoadRetiredm is high
      while (m_p_retire[port]->read()) {
        if (m_load_address_deque[port].empty()) {
          ostringstream oss;
          oss << "XLMI Port #" << port << ": the interface signaled load retired, but there are no outstanding loads";
          throw xtsc_exception(oss.str());
        }
        xtsc_address address = m_load_address_deque[port].front();
        m_load_address_deque[port].pop_front();
        (*m_request_ports[port])->nb_load_retired(address);
        wait(m_clock_period);
      }

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in xlmi_load_retired_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::xlmi_retire_flush_thread() {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_xlmi_retire_flush_thread++;

  try {

    XTSC_INFO(m_text, "in xlmi_retire_flush_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for DPortRetireFlush to go high: m_p_flush[port]->pos()
      wait();

      // Sync to sample phase
      sync_to_sample_phase();

      // Call nb_retire_flush() once each clock period while DPort0RetireFlushm is high
      while (m_p_flush[port]->read()) {
        if (m_load_address_deque[port].empty()) {
          ostringstream oss;
          oss << "XLMI Port #" << port << ": the interface signaled retire flush, but there are no outstanding loads";
          throw xtsc_exception(oss.str());
        }
        m_load_address_deque[port].clear();
        (*m_request_ports[port])->nb_retire_flush();
        wait(m_clock_period);
      }

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in xlmi_retire_flush_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::dram_lock_method() {
  try {
    for (u32 port=0; port<m_num_ports; ++port) {
      bool lock = (*m_p_lock[port])->read();
      if (m_dram_lock_reset || (lock != m_dram_lock[port])) {
        (*m_request_ports[port])->nb_lock(lock);
        m_dram_lock[port] = lock;
      }
    }
    m_dram_lock_reset = false;
  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in dram_lock_method() of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::pif_sample_pin_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_sample_pin_request_thread++;

  try {

    XTSC_INFO(m_text, "in pif_sample_pin_request_thread[" << port << "]");

    // Loop forever
    while (true) {

      do {
        XTSC_DEBUG(m_text, "pif_sample_pin_request_thread(): Waiting for POReqValid|PIReqRdy");
        wait();  // m_p_req_valid[port]->pos()  /  m_p_req_rdy[port]->pos()
        XTSC_DEBUG(m_text, "pif_sample_pin_request_thread(): Got POReqValid|PIReqRdy");
      } while (!m_p_req_valid[port]->read() || !m_p_req_rdy[port]->read());

      // Sync to sample phase
      sync_to_sample_phase();
      XTSC_DEBUG(m_text, "pif_sample_pin_request_thread(): completed sync_to_sample_phase()");

      // Capture one request per clock period while both POReqValid and PIReqRdy are asserted
      while (m_p_req_valid[port]->read() && m_p_req_rdy[port]->read()) {
        request_info *p_info = new_request_info(port);
        m_request_fifo[port].push_back(p_info);
        m_pif_req_event[port].notify(SC_ZERO_TIME);
        XTSC_INFO(m_text, *p_info);
        wait(m_clock_period);
      }

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_sample_pin_request_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::pif_drive_req_rdy_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_drive_req_rdy_thread++;

  try {

    XTSC_INFO(m_text, "in pif_drive_req_rdy_thread[" << port << "]");

#if defined(MTI_SYSTEMC)
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 0)
    // Work around Mentor Graphics bug.  Emails of 22 && 23 Sep 2008.  Should be fixed in Modelsim 6.3i.
    m_p_req_rdy[port]->write(false);
    m_p_req_rdy[port]->write(true);
#endif
#endif

    // Loop forever
    while (true) {
      // Wait to be notified
      wait(m_drive_req_rdy_event[port]);
      while (m_req_rdy_fifo[port]->num_available()) {
        bool dummy;
        m_req_rdy_fifo[port]->nb_read(dummy);
        m_p_req_rdy[port]->write(false);
        XTSC_DEBUG(m_text, "pif_drive_req_rdy_thread[" << port << "] deasserting PIReqRdy");
        wait(m_clock_period);
        m_p_req_rdy[port]->write(true);
        XTSC_DEBUG(m_text, "pif_drive_req_rdy_thread[" << port << "] asserting PIReqRdy");
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_drive_req_rdy_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::pif_send_tlm_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_send_tlm_request_thread++;

  try {

    XTSC_INFO(m_text, "in pif_send_tlm_request_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for a request
      wait(m_pif_req_event[port]);
      XTSC_DEBUG(m_text, "pif_send_tlm_request_thread(): awoke from wait()");

      // Process up to one request at a time while there are requests available.
      while (!m_request_fifo[port].empty()) {
        request_info *p_info = m_request_fifo[port].front();
        m_request_fifo[port].pop_front();
        u32 tries = 0;
        do {
          m_request_got_nacc[port] = false;
          tries += 1;
          if (tries > 1) {
          }
          XTSC_VERBOSE(m_text, p_info->m_request << " Port #" << port << " Try #" << tries);
          m_waiting_for_nacc[port] = true;
          (*m_request_ports[port])->nb_request(p_info->m_request);
          wait(m_clock_period);
          m_waiting_for_nacc[port] = false;
        } while (m_request_got_nacc[port]);
        delete_request_info(p_info);
      }

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_send_tlm_request_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_pin2tlm_memory_transactor::pif_drive_pin_response_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_drive_pin_response_thread++;

  try {

    XTSC_INFO(m_text, "in pif_drive_pin_response_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for a response
      wait(m_pif_resp_event[port]);
      XTSC_DEBUG(m_text, "pif_drive_pin_response_thread(): awoke from wait()");

      // Process up to one response per clock period while there are responses avaiable.
      while (!m_pif_resp_fifo[port].empty()) {

        xtsc_response *p_response = m_pif_resp_fifo[port].front();
        m_pif_resp_fifo[port].pop_front();

        m_id       = p_response->get_id();
        m_priority = p_response->get_priority();
        m_route_id = p_response->get_route_id();
        m_coh_cntl = p_response->get_coherence();
        get_read_data_from_response(*p_response);

        bool last = p_response->get_last_transfer();
        if (m_snoop) {
          bool data   = p_response->has_snoop_data();
          bool shared = (m_coh_cntl.to_uint() != 0);
          m_resp_cntl.init(shared, data, last);
        }
        else {
          m_resp_cntl.init((u32)p_response->get_status(), last);
        }

        // drive response
        m_p_resp_valid      [port]->write(true);
        m_p_resp_cntl       [port]->write(m_resp_cntl.get_value());
        m_p_resp_data       [port]->write(m_data);
        if (m_p_resp_priority [port]) {
          m_p_resp_priority [port]->write(m_priority);
        }
        if (m_has_request_id) {
          m_p_resp_id       [port]->write(m_id);
        }
        if (m_route_id_bits) {
          m_p_resp_route_id [port]->write(m_route_id);
        }
        if (m_p_resp_coh_cntl[port]) {
          m_p_resp_coh_cntl [port]->write(m_coh_cntl);
        }

        // We continue driving this response until it is accepted because at this point it
        // is too late to reject the TLM response because we have already returned true
        // to the nb_respond() call (the only way to reject a TLM response)
        bool ready = false;
        while (!ready) {
          sc_time now = sc_time_stamp();
          sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
          if (m_has_posedge_offset) {
            if (phase_now < m_posedge_offset) {
              phase_now += m_clock_period;
            }
            phase_now -= m_posedge_offset;
          }
          sc_time wait_before(m_clock_period);
          if (phase_now < m_sample_phase) {
            wait_before = (m_sample_phase - phase_now);
          }
          else if (phase_now > m_sample_phase) {
            wait_before = (m_sample_phase_plus_one - phase_now);
          }
          wait(wait_before);
          // Sample at "sample_phase" time
          ready = m_p_resp_rdy[port]->read();
          sc_time wait_after(m_clock_period - wait_before);
          if (wait_after != SC_ZERO_TIME) {
            wait(wait_after);
          }
        }

        delete_response(p_response);
      }

      m_resp_cntl.init(0);
      m_id       = 0;
      m_priority = 0;
      m_route_id = 0;
      m_coh_cntl = 0;
      m_data     = 0;

      // deassert response
      m_p_resp_valid    [port]->write(false);
      m_p_resp_cntl     [port]->write(m_resp_cntl.get_value());
      m_p_resp_data     [port]->write(m_data);
      if (m_p_resp_priority [port]) {
        m_p_resp_priority [port]->write(m_priority);
      }
      if (m_has_request_id) {
        m_p_resp_id       [port]->write(m_id);
      }
      if (m_route_id_bits) {
        m_p_resp_route_id [port]->write(m_route_id);
      }
      if (m_p_resp_coh_cntl[port]) {
        m_p_resp_coh_cntl [port]->write(m_coh_cntl);
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_drive_pin_response_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



bool_signal& xtsc_component::xtsc_pin2tlm_memory_transactor::create_bool_signal(const string& signal_name) {
  bool_signal *p_signal = new bool_signal(signal_name.c_str());
  m_map_bool_signal[signal_name] = p_signal;
  return *p_signal;
}



uint_signal& xtsc_component::xtsc_pin2tlm_memory_transactor::create_uint_signal(const string& signal_name, u32 num_bits) {
  sc_length_context length(num_bits);
  uint_signal *p_signal = new uint_signal(signal_name.c_str());
  m_map_uint_signal[signal_name] = p_signal;
  return *p_signal;
}



wide_signal& xtsc_component::xtsc_pin2tlm_memory_transactor::create_wide_signal(const string& signal_name, u32 num_bits) {
  sc_length_context length(num_bits);
  wide_signal *p_signal = new wide_signal(signal_name.c_str());
  m_map_wide_signal[signal_name] = p_signal;
  return *p_signal;
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::swizzle_byte_enables(xtsc_byte_enables& byte_enables) const {
  xtsc_byte_enables swizzled = 0;
  for (u32 i=0; i<m_width8; i++) {
    swizzled <<= 1;
    swizzled |= byte_enables & 1;
    byte_enables >>= 1;
  }
  byte_enables = swizzled;
}



xtsc_component::xtsc_pin2tlm_memory_transactor::request_info::request_info(const xtsc_pin2tlm_memory_transactor& pin2tlm, u32 port) :
  m_pin2tlm     (pin2tlm),
  m_req_cntl    (0),
  m_data        ((int)m_pin2tlm.m_width8*8),
  m_id          (m_id_bits),
  m_priority    (2),
  m_route_id    (m_pin2tlm.m_route_id_bits ? m_pin2tlm.m_route_id_bits : 1),
  m_vadrs       (6),
  m_coherence   (2)
{
  init(port);
}



xtsc_component::xtsc_pin2tlm_memory_transactor::request_info *
xtsc_component::xtsc_pin2tlm_memory_transactor::new_request_info(u32 port) {
  if (m_request_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new request_info");
    return new request_info(*this, port);
  }
  else {
    request_info *p_request_info = m_request_pool.back();
    m_request_pool.pop_back();
    p_request_info->init(port);
    return p_request_info;
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::delete_request_info(request_info*& p_request_info) {
  m_request_pool.push_back(p_request_info);
  p_request_info = 0;
}



xtsc_response *xtsc_component::xtsc_pin2tlm_memory_transactor::new_response(const xtsc_response& response) {
  if (m_response_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new xtsc_response");
    return new xtsc_response(response);
  }
  else {
    xtsc_response *p_response = m_response_pool.back();
    m_response_pool.pop_back();
    *p_response = response;
    return p_response;
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::delete_response(xtsc_response*& p_response) {
  m_response_pool.push_back(p_response);
  p_response = 0;
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::request_info::init(u32 port) {
  bool                  has_data = false;
  u32                   size     = m_pin2tlm.m_width8;
  xtsc_request::type_t  type     = xtsc_request::READ;

  if (m_pin2tlm.m_interface_type == xtsc_module_pin_base::PIF) {
    m_address           = m_pin2tlm.m_p_req_adrs     [port]->read().to_uint();
    m_fixed_address     = m_address;
    m_req_cntl          = m_pin2tlm.m_p_req_cntl     [port]->read().to_uint();
    m_priority          = m_pin2tlm.m_p_req_priority [port]->read();
    m_byte_enables      = m_pin2tlm.m_p_req_data_be  [port] ? m_pin2tlm.m_p_req_data_be  [port]->read().to_uint() : 0;
    m_data              = m_pin2tlm.m_p_req_data     [port] ? m_pin2tlm.m_p_req_data     [port]->read() : m_pin2tlm.m_zero_bv;
    m_id                = m_pin2tlm.m_p_req_id       [port] ? m_pin2tlm.m_p_req_id       [port]->read() : m_pin2tlm.m_zero_uint;
    m_route_id          = m_pin2tlm.m_p_req_route_id [port] ? m_pin2tlm.m_p_req_route_id [port]->read() : m_pin2tlm.m_zero_uint;
    m_req_attribute     = m_pin2tlm.m_p_req_attribute[port] ? m_pin2tlm.m_p_req_attribute[port]->read() : m_pin2tlm.m_zero_uint;
    m_vadrs             = m_pin2tlm.m_p_req_coh_vadrs[port] ? m_pin2tlm.m_p_req_coh_vadrs[port]->read() : m_pin2tlm.m_zero_uint;
    m_coherence         = m_pin2tlm.m_p_req_coh_cntl [port] ? m_pin2tlm.m_p_req_coh_cntl [port]->read() : m_pin2tlm.m_zero_uint;
    m_fixed_byte_enables = m_byte_enables;
    if (m_pin2tlm.m_big_endian) {
      m_pin2tlm.swizzle_byte_enables(m_fixed_byte_enables);
    }
  }
  else {
    m_address = m_pin2tlm.m_p_addr[port]->read().to_uint();
    if (m_pin2tlm.m_p_wr[port] && m_pin2tlm.m_p_wr[port]->read()) {
      m_data = m_pin2tlm.m_p_wrdata[port]->read();
      type = xtsc_request::WRITE;
      has_data = true;
    }
    m_fixed_address = ((m_address << m_pin2tlm.m_address_shift) & m_pin2tlm.m_address_mask) + m_pin2tlm.m_start_byte_address;
    m_byte_enables = (0xFFFFFFFFFFFFFFFFull >> (64 - (size)));
    m_fixed_byte_enables = m_byte_enables;
    if (m_pin2tlm.m_p_lane[port]) {
      m_byte_enables = m_pin2tlm.m_p_lane[port]->read();
      if ((m_pin2tlm.m_interface_type == IRAM0) || (m_pin2tlm.m_interface_type == IRAM1) || (m_pin2tlm.m_interface_type == IROM0)) {
        // Convert word enables (1 "word" is 32 bits) to byte enables (64-bit IRAM0|IRAM1|IROM0 only)
        xtsc_byte_enables be = 0x0000;
        if (m_byte_enables & 0x1) be |= 0x000F;
        if (m_byte_enables & 0x2) be |= 0x00F0;
        if (m_byte_enables & 0x4) be |= 0x0F00;
        if (m_byte_enables & 0x8) be |= 0xF000;
        m_byte_enables = be;
      }
      m_fixed_byte_enables = m_byte_enables;
      if (m_pin2tlm.m_big_endian) {
        m_pin2tlm.swizzle_byte_enables(m_fixed_byte_enables);
      }
    }
  }

  if ((m_pin2tlm.m_interface_type == xtsc_module_pin_base::PIF) && !m_pin2tlm.m_snoop) {
    // Adjust address based on byte enables
    xtsc_byte_enables mask = 0x1;
    for (u32 i=0; ((i<m_pin2tlm.m_width8) && !(m_fixed_byte_enables & mask)); ++i) {
      m_fixed_address -= 1;
      mask <<= 1;
    }
  }

  XTSC_DEBUG(m_pin2tlm.m_text, "In init(): m_fixed_address=0x" << hex << setw(8) << m_fixed_address <<
                               " m_fixed_byte_enables=0x" << m_fixed_byte_enables);

  if (m_pin2tlm.m_interface_type == xtsc_module_pin_base::PIF) {
    u64 tag = 0;
    type = m_req_cntl.get_request_type();
    u32  num_transfers = m_req_cntl.get_num_transfers();
    bool last_transfer = m_req_cntl.get_last_transfer();
    u8 id = (u8) m_id.to_uint();
    if (type == xtsc_request::BLOCK_WRITE) {
      has_data = true;
      if (m_pin2tlm.m_first_block_write[port]) {
        m_request.initialize(type, m_fixed_address, size, tag, num_transfers, m_fixed_byte_enables,
                             last_transfer, m_route_id, id, m_priority);
      }
      else {
        tag = m_pin2tlm.m_tag[port];
        m_fixed_address = m_pin2tlm.m_last_address[port] + m_pin2tlm.m_width8;
        m_request.initialize(tag, m_fixed_address, size, num_transfers, last_transfer, m_route_id, id, m_priority);
      }
      m_pin2tlm.m_tag[port] = m_request.get_tag();
      m_pin2tlm.m_last_address[port] = m_fixed_address;
      m_pin2tlm.m_first_block_write[port] = last_transfer;
    }
    else if (type == xtsc_request::BURST_WRITE) {
      has_data = true;
      if (m_pin2tlm.m_burst_write_transfer_num[port] == 1) {
        m_request.initialize(type, m_fixed_address, size, tag, num_transfers, m_fixed_byte_enables,
                             last_transfer, m_route_id, id, m_priority);
      }
      else {
        tag = m_pin2tlm.m_tag[port];
        m_fixed_address = m_pin2tlm.m_last_address[port] + m_pin2tlm.m_width8;
        m_request.initialize(m_address, tag, m_fixed_address, size, num_transfers, m_pin2tlm.m_burst_write_transfer_num[port],
                             m_fixed_byte_enables, m_route_id, id, m_priority);
      }
      m_pin2tlm.m_last_address[port] = m_fixed_address;
      m_pin2tlm.m_tag[port] = m_request.get_tag();
      if (last_transfer) {
        m_pin2tlm.m_burst_write_transfer_num[port] = 1;
      }
      else {
        m_pin2tlm.m_burst_write_transfer_num[port] += 1;
      }
    }
    else if (type == xtsc_request::RCW) {
      has_data = true;
      if (m_pin2tlm.m_first_rcw[port]) {
        m_request.initialize(type, m_fixed_address, size, tag, num_transfers, m_fixed_byte_enables,
                             last_transfer, m_route_id, id, m_priority);
      }
      else {
        tag = m_pin2tlm.m_tag[port];
        m_request.initialize(tag, m_fixed_address, m_route_id, id, m_priority);
        m_request.set_byte_size(size);
        m_request.set_byte_enables(m_fixed_byte_enables);
      }
      m_pin2tlm.m_tag[port] = m_request.get_tag();
      m_pin2tlm.m_first_rcw[port] = last_transfer;
    }
    else {
      if (type == xtsc_request::WRITE) {
        has_data = true;
      }
      else if (type == xtsc_request::SNOOP) {
        m_coherence = (((m_req_cntl.get_value().to_uint() & req_cntl::m_type_mask) == req_cntl::SNOOP_EXCLUSIVE) ? 2 : 1);
      }
      m_request.initialize(type, m_fixed_address, size, tag, num_transfers, m_fixed_byte_enables,
                           last_transfer, m_route_id, id, m_priority);
    }
    if (m_pin2tlm.m_p_req_coh_vadrs[port]) {
      xtsc_address vaddr = (m_vadrs.to_uint() << m_vadrs_shift) | (m_request.get_byte_address() & m_vaddr_lo_mask);
      m_request.set_snoop_virtual_address(vaddr);
    }
    if (m_pin2tlm.m_p_req_attribute[port]) {
      m_request.set_pif_attribute(m_req_attribute.to_uint());
    }
    m_request.set_coherence((xtsc_request::coherence_t)m_coherence.to_uint());
  }
  else {
    m_request.initialize(type, m_fixed_address, size, 0, 1, m_fixed_byte_enables);
    if (m_pin2tlm.m_p_xfer_en && m_pin2tlm.m_p_xfer_en[port]) {
      m_request.set_xfer_en(m_pin2tlm.m_p_xfer_en[port]->read());
    }
    if (!has_data && m_pin2tlm.m_interface_type == xtsc_module_pin_base::XLMI0) {
      // If busy gets asserted, then this will need to be removed from the deque
      m_pin2tlm.m_load_address_deque[port].push_back(m_fixed_address);
    }
  }

  // We don't check for valid byte enables here - the terminal device is responsible for enforcing legal byte enables.
  if (has_data) {
    u8 *buffer = m_request.get_buffer();
    u32 r = (m_pin2tlm.m_big_endian ? (size-1) : 0) * 8;
    u32 delta = (m_pin2tlm.m_big_endian ? -8 : 8);
    for (u32 i=0; i<size; ++i) {
      XTSC_DEBUG(m_pin2tlm.m_text, "init() data movement: buffer[" << i << "] = m_data.range(" << (r+7) << ", " << r << ")");
      buffer[i] = m_data.range(r+7, r).to_uint();
      r += delta;
    }
  }

  XTSC_INFO(m_pin2tlm.m_text, m_request << ": In init()");
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::request_info::dump(ostream& os) const {
  // Save state of stream
  char c = os.fill('0');
  ios::fmtflags old_flags = os.flags();

  os << m_req_cntl << " [0x" << hex << setw(8) << m_address << "/0x" << setw(m_pin2tlm.m_width8/4) << m_byte_enables << "]";

  u32 type = m_req_cntl.get_type();
  if (type == req_cntl::WRITE || type == req_cntl::BLOCK_WRITE || type == req_cntl::BURST_WRITE || type == req_cntl::RCW) {
    os << "=0x" << hex << m_data;
  }

  // Restore state of stream
  os.fill(c);
  os.flags(old_flags);
}



namespace xtsc_component {
ostream& operator<<(ostream& os, const xtsc_pin2tlm_memory_transactor::request_info& info) {
  info.dump(os);
  return os;
}
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  (*m_pin2tlm.m_request_ports[m_port_num])->nb_peek(address8, size8, buffer);
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  (*m_pin2tlm.m_request_ports[m_port_num])->nb_poke(address8, size8, buffer);
}



bool xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl::nb_peek_coherent(xtsc_address  virtual_address8,
                                                                                          xtsc_address  physical_address8,
                                                                                          u32           size8,
                                                                                          u8           *buffer)
{
  return (*m_pin2tlm.m_request_ports[m_port_num])->nb_peek_coherent(virtual_address8, physical_address8, size8, buffer);
}



bool xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl::nb_poke_coherent(xtsc_address  virtual_address8,
                                                                                          xtsc_address  physical_address8,
                                                                                          u32           size8,
                                                                                          const u8     *buffer)
{
  return (*m_pin2tlm.m_request_ports[m_port_num])->nb_poke_coherent(virtual_address8, physical_address8, size8, buffer);
}



bool xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl::nb_fast_access(xtsc_fast_access_request &request) {
  return (*m_pin2tlm.m_request_ports[m_port_num])->nb_fast_access(request);
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_debug_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_pin2tlm_memory_transactor '" << m_pin2tlm.name() << "' m_debug_exports["
        << m_port_num << "]: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_pin2tlm.m_text, "Binding '" << port.name() << "' to '" << m_pin2tlm.name() << ".m_debug_exports[" << m_port_num <<
                              "]'");
  m_p_port = &port;
}



bool xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_respond_if_impl::nb_respond(const xtsc_response& response) {
  XTSC_INFO(m_pin2tlm.m_text, response);
  if (m_pin2tlm.m_interface_type == xtsc_module_pin_base::PIF) {
    if (response.get_status() == xtsc_response::RSP_NACC) {
      if (m_pin2tlm.m_waiting_for_nacc[m_port_num]) {
        m_pin2tlm.m_request_got_nacc[m_port_num] = true;
        m_pin2tlm.m_req_rdy_fifo[m_port_num]->write(true);
        m_pin2tlm.m_drive_req_rdy_event[m_port_num].notify(m_pin2tlm.m_output_delay);
        XTSC_DEBUG(m_pin2tlm.m_text, "scheduled PIReqRdy");
        return true;
      }
      else {
        ostringstream oss;
        oss << "xtsc_pin2tlm_memory_transactor '" << m_pin2tlm.name() << "' received nacc too late: " << response << endl;
        oss << " - Possibly something is wrong with the downstream device" << endl;
        throw xtsc_exception(oss.str());
      }
    }
    else {
      if (m_pin2tlm.m_prereject_responses && !m_pin2tlm.m_p_resp_rdy[m_port_num]->read()) {
        XTSC_INFO(m_pin2tlm.m_text, response << " <== Rejected on port #" << m_port_num);
        return false;
      }
      xtsc_response *p_response = m_pin2tlm.new_response(response);
      m_pin2tlm.m_pif_resp_fifo[m_port_num].push_back(p_response);
      m_pin2tlm.m_pif_resp_event[m_port_num].notify(m_pin2tlm.m_output_delay);
      return true;
    }
  }
  else {
    if (m_pin2tlm.m_request_fifo[m_port_num].empty()) {
      ostringstream oss;
      oss << kind() << " '" << name() << "' received a response with no outstanding request";
      throw xtsc_exception(oss.str());
    }
    request_info *p_info = m_pin2tlm.m_request_fifo[m_port_num].front();
    m_pin2tlm.m_request_fifo[m_port_num].pop_front();

    if (response.get_status() == xtsc_response::RSP_NACC) {
      if (m_pin2tlm.m_p_busy[m_port_num]) {
        m_pin2tlm.m_busy_fifo[m_port_num]->write(true);
        m_pin2tlm.m_drive_busy_event[m_port_num].notify(m_pin2tlm.m_output_delay);
        XTSC_DEBUG(m_pin2tlm.m_text, "scheduled busy");
      }
      else {
        ostringstream oss;
        oss << kind() << " '" << name() << "' received RSP_NACC but this " << m_pin2tlm.m_interface_uc
            << " interface has no busy signal";
        throw xtsc_exception(oss.str());
      }
      // If the request getting busy (RSP_NACC) was a READ, then the last address added to the deque needs to be removed
      if ((m_pin2tlm.m_interface_type == xtsc_module_pin_base::XLMI0) && (p_info->m_request.get_type() == xtsc_request::READ)) {
        m_pin2tlm.m_load_address_deque[m_port_num].pop_back();
      }
    }
    else if (p_info->m_request.get_type() == xtsc_request::READ) {
      // Schedule read data to be driven
      m_pin2tlm.get_read_data_from_response(response);
      m_pin2tlm.m_read_data_fifo[m_port_num]->write(m_pin2tlm.m_data);
      m_pin2tlm.m_drive_read_data_event[m_port_num].notify(m_pin2tlm.m_output_delay);
      XTSC_DEBUG(m_pin2tlm.m_text, "schedule drive_read_data of 0x" << hex << m_pin2tlm.m_data);
    }
    m_pin2tlm.delete_request_info(p_info);
    return true;
  }
}



void xtsc_component::xtsc_pin2tlm_memory_transactor::xtsc_respond_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_pin2tlm_memory_transactor '" << m_pin2tlm.name() << "' m_respond_exports["
        << m_port_num << "]: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_pin2tlm.m_text, "Binding '" << port.name() << "' to '" << m_pin2tlm.name() << ".m_respond_exports[" << m_port_num <<
                              "]'");
  m_p_port = &port;
}




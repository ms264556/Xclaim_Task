// Copyright (c) 2007-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include <iostream>
#include <algorithm>
#include <xtsc/xtsc_fast_access.h>
#include <xtsc/xtsc_memory_pin.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_tlm2pin_memory_transactor.h>


/**
 * Theory of operation:
 *
 * This model supports any of the Xtensa memory interfaces (except the caches themselves) and
 * it supports operating as a multi-ported memory.  It also supports generating a random busy/
 * not-ready signal for testing purposes.
 *
 * The "sample_phase" parameter defines the clock phase at which signals are sampled and
 * the "drive_phase" parameter defines the clock phase when outputs are driven.  Typically
 * you would sample at the clock boundary and drive a fraction of a clock cycle later.
 *
 * When this model is operating with a PIF interface, it uses the following threads:
 *    pif_request_thread (to sample signals except PORespRdy)
 *    pif_drive_req_rdy_thread  (to drive PIReqRdy)
 *    pif_respond_thread (to drive response outputs and sample PORespRdy)
 *
 * When this model is operating with a local memory interface, it uses the following threads:
 *    lcl_request_thread (to sample signals)
 *    lcl_drive_read_data_thread (to drive the read data output)
 *    lcl_drive_busy_thread (to drive the busy output)
 *
 *
 */



using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace log4xtensa;
using namespace xtsc;



// Shorthand aliases
typedef sc_core::sc_signal<bool>                bool_signal;
typedef sc_core::sc_signal<sc_dt::sc_uint_base> uint_signal;
typedef sc_core::sc_signal<sc_dt::sc_bv_base>   wide_signal;



// ctor helper
static u32 get_num_ports(const xtsc_component::xtsc_memory_pin_parms& memory_parms) {
  return memory_parms.get_non_zero_u32("num_ports");
}



// ctor helper
static sc_trace_file *get_trace_file(const xtsc_component::xtsc_memory_pin_parms& memory_parms) {
  return static_cast<sc_trace_file*>(const_cast<void*>(memory_parms.get_void_pointer("vcd_handle")));
}



// ctor helper
static const char *get_suffix(const xtsc_component::xtsc_memory_pin_parms& memory_parms) {
  const char *port_name_suffix = memory_parms.get_c_str("port_name_suffix");
  static const char *blank = "";
  return port_name_suffix ? port_name_suffix : blank;
}



xtsc_component::xtsc_memory_pin_parms::xtsc_memory_pin_parms(const xtsc_core&   core,
                                                             const char        *memory_interface,
                                                             u32                delay,
                                                             u32                num_ports)
{
  xtsc_core::memory_port mem_port = xtsc_core::get_memory_port(memory_interface);
  if (!core.has_memory_port(mem_port)) {
    ostringstream oss;
    oss << core.kind() << " '" << core.name() << "' doesn't have a \"" << memory_interface << "\" memory port.";
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

  u32   start_address8  = 0;
  u32   width8          = core.get_memory_byte_width(mem_port);
  u32   address_bits    = 32;   // PIF|XLMI0
  u32   size8           = 0;    // 4GB for PIF

  if (interface_type == xtsc_module_pin_base::PIF) {
    if (delay == 0xFFFFFFFF) {
      delay = core_parms.get_u32("LocalMemoryLatency") - 1;
    }
  }
  else {
    delay = core_parms.get_u32("LocalMemoryLatency") - 1;
    if ((interface_type == xtsc_module_pin_base::XLMI0) || core_parms.get_bool("SimFullLocalMemAddress")) {
      core.get_local_memory_starting_byte_address(mem_port, start_address8);
    }
    core.get_local_memory_byte_size(mem_port, size8);
    if (interface_type != xtsc_module_pin_base::XLMI0) {
      // Compute address_bits  (size8 must be a power of 2)
      address_bits = 0;
      u32 shift_value = size8 / width8;
      for (u32 i=0; i<32; ++i) {
        if (shift_value & 0x1) address_bits = i;
        shift_value >>= 1;
      }
    }
  }

  init(xtsc_module_pin_base::get_interface_name(interface_type), width8, address_bits, delay, num_ports);

  set("clock_period", core_parms.get_u32("SimClockFactor")*xtsc_get_system_clock_factor());
  set("start_byte_address", start_address8);
  set("memory_byte_size", size8);
  set("big_endian", core.is_big_endian());
  set("has_busy", core.has_busy(mem_port));

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



xtsc_component::xtsc_memory_pin::xtsc_memory_pin(sc_module_name module_name, const xtsc_memory_pin_parms& memory_parms) :
  sc_module             (module_name),
  xtsc_module_pin_base  (*this, get_num_ports(memory_parms), get_trace_file(memory_parms), get_suffix(memory_parms)),
  m_num_ports           (memory_parms.get_u32("num_ports")),
  m_interface_uc        (get_interface_uc(memory_parms.get_c_str("memory_interface"))),
  m_interface_type      (get_interface_type(m_interface_uc)),
  m_read_delay_value    (memory_parms.get_u32("read_delay")),
  m_inbound_pif         (false),
  m_has_pif_attribute   (false),
  m_use_fast_access     (memory_parms.get_bool("use_fast_access")),
  m_address_bits        ((m_interface_type == PIF) ? 32 : memory_parms.get_non_zero_u32("address_bits")),
  m_check_bits          (memory_parms.get_u32("check_bits")),
  m_setw                ((m_address_bits+3)/4),
  m_route_id_bits       (memory_parms.get_u32("route_id_bits")),
  m_address             (m_address_bits),
  m_id_zero             (m_id_bits),
  m_priority_zero       (2),
  m_route_id_zero       (m_route_id_bits ? m_route_id_bits : 1),
  m_data_zero           ((int)memory_parms.get_non_zero_u32("byte_width")),
  m_resp_cntl_zero      (memory_parms.get_u32("void_resp_cntl")),
  m_req_cntl            (0),
  m_resp_cntl           (0),
  m_text                (log4xtensa::TextLogger::getInstance(name())),
  m_binary              (log4xtensa::BinaryLogger::getInstance(name()))
{

  u32           byte_width              = memory_parms.get_u32("byte_width");
  u32           start_byte_address      = memory_parms.get_u32("start_byte_address");
  u32           memory_byte_size        = memory_parms.get_u32("memory_byte_size");
  u32           page_byte_size          = memory_parms.get_u32("page_byte_size");
  const char   *initial_value_file      = memory_parms.get_c_str("initial_value_file");
  u8            memory_fill_byte        = (u8) memory_parms.get_u32("memory_fill_byte");

  m_p_memory = new xtsc_memory_base(name(), kind(), byte_width, start_byte_address, memory_byte_size, page_byte_size,
                                    initial_value_file, memory_fill_byte);

  m_start_address8      = m_p_memory->m_start_address8;
  m_size8               = m_p_memory->m_size8;
  m_width8              = m_p_memory->m_width8;
  m_end_address8        = m_p_memory->m_end_address8;
  m_enable_bits         = m_width8;
  if ((m_interface_type == IRAM0) || (m_interface_type == IRAM1) || (m_interface_type == IROM0)) {
    m_enable_bits       = m_width8 / 4;
  }

  m_id_zero                     = 0;
  m_priority_zero               = 0;
  m_route_id_zero               = 0;
  m_data_zero                   = 0;
  m_request_fifo_depth          = 0;

  if (m_interface_type == PIF) {
    m_inbound_pif       = memory_parms.get_bool("inbound_pif");
    m_has_pif_attribute = memory_parms.get_bool("has_pif_attribute");
  }
  m_big_endian          = memory_parms.get_bool("big_endian");
  m_has_request_id      = memory_parms.get_bool("has_request_id");
  m_write_responses     = memory_parms.get_bool("write_responses");
  m_has_busy            = memory_parms.get_bool("has_busy");
  m_has_lock            = memory_parms.get_bool("has_lock");
  m_has_xfer_en         = memory_parms.get_bool("has_xfer_en");
  m_busy_percentage     = (i32) memory_parms.get_u32 ("busy_percentage");
  m_cbox                = memory_parms.get_bool("cbox");

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

  // Warn if "memory_byte_size" is non-zero AND "address_bits" is not 32 (i.e. both explicitly set by user) AND they are inconsistent.
  u32 address_unit = (((m_interface_type == PIF) || (m_interface_type == XLMI0)) ?  1 : m_width8);
  if ((m_size8 != 0) && (m_address_bits != 32) && (m_size8 != (address_unit << m_address_bits))) {
    XTSC_WARN(m_text, "\"memory_byte_size\" of 0x" << hex << m_size8 <<
                      " is inconsistent with \"address_bits\" of " << dec << m_address_bits);
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
    m_address_shift     = 0;
    m_address_mask      = ((m_width8 == 4) ? 0xFFFFFFFC : (m_width8 == 8) ? 0xFFFFFFF8 : 0xFFFFFFF0);
    m_request_fifo_depth= memory_parms.get_non_zero_u32("request_fifo_depth");
  }
  else if (m_interface_type == XLMI0) {
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
    m_address_mask      = (m_address_bits >= 32) ? 0xFFFFFFFF : (1 << m_address_bits) - 1;
  }

  // Get clock period 
  m_time_resolution = sc_get_time_resolution();
  u32 clock_period = memory_parms.get_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = sc_get_time_resolution() * clock_period;
  }
  m_clock_period_value = m_clock_period.value();
  u32 posedge_offset = memory_parms.get_u32("posedge_offset");
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

  // Convert delays from an integer number of periods to sc_time values
  m_read_delay          = m_clock_period * memory_parms.get_u32("read_delay");
  m_block_read_delay    = m_clock_period * memory_parms.get_u32("block_read_delay");
  m_block_read_repeat   = m_clock_period * memory_parms.get_u32("block_read_repeat");
  m_burst_read_delay    = m_clock_period * memory_parms.get_u32("burst_read_delay");
  m_burst_read_repeat   = m_clock_period * memory_parms.get_u32("burst_read_repeat");
  m_rcw_repeat          = m_clock_period * memory_parms.get_u32("rcw_repeat");
  m_rcw_response        = m_clock_period * memory_parms.get_u32("rcw_response");
  m_write_delay         = m_clock_period * memory_parms.get_u32("write_delay");
  m_block_write_delay   = m_clock_period * memory_parms.get_u32("block_write_delay");
  m_block_write_repeat  = m_clock_period * memory_parms.get_u32("block_write_repeat");
  m_block_write_response= m_clock_period * memory_parms.get_u32("block_write_response");
  m_burst_write_delay   = m_clock_period * memory_parms.get_u32("burst_write_delay");
  m_burst_write_repeat  = m_clock_period * memory_parms.get_u32("burst_write_repeat");
  m_burst_write_response= m_clock_period * memory_parms.get_u32("burst_write_response");

  // Get phase at which inputs are to be sampled
  u32 sample_phase = memory_parms.get_u32("sample_phase");
  m_sample_phase = sample_phase * m_time_resolution;
  m_sample_phase_plus_one = m_sample_phase + m_clock_period;

  // Get phase at which outputs are to be driven
  u32 drive_phase = memory_parms.get_u32("drive_phase");
  m_drive_phase = drive_phase * m_time_resolution;
  m_drive_phase_plus_one = m_drive_phase + m_clock_period;
  m_drive_to_sample_time = ((m_drive_phase < m_sample_phase) ? m_sample_phase : m_sample_phase_plus_one) - m_drive_phase;
  m_sample_to_drive_time = m_clock_period - m_drive_to_sample_time;
  m_sample_to_drive_busy_delay  = ((m_drive_phase >= m_sample_phase) ? m_drive_phase : m_drive_phase_plus_one) - m_sample_phase;
  m_sample_to_drive_data_delay  = m_sample_to_drive_busy_delay + m_read_delay;


  // Create all the "per mem port" stuff

  m_last_action_time_stamp      = new sc_time                   [m_num_ports];
  m_data                        = new sc_bv_base*               [m_num_ports];
  m_data_to_be_written          = new sc_bv_base*               [m_num_ports];

  m_debug_exports               = new sc_export<xtsc_debug_if>* [m_num_ports];
  m_debug_impl                  = new xtsc_debug_if_impl*       [m_num_ports];

  if (m_interface_type == PIF) {
    m_pif_req_fifo              = new deque<pif_req_info*>      [m_num_ports];
    m_first_block_write         = new bool                      [m_num_ports];
    m_first_burst_write         = new bool                      [m_num_ports];
    m_block_write_address       = new xtsc_address              [m_num_ports];
    m_burst_write_address       = new xtsc_address              [m_num_ports];
    m_rcw_compare_data          = new sc_bv_base*               [m_num_ports];
    m_pif_req_event             = new sc_event                  [m_num_ports];
    m_pif_req_rdy_event         = new sc_event                  [m_num_ports];
    m_respond_event             = new sc_event                  [m_num_ports];
    m_testing_busy              = new bool                      [m_num_ports];
  }
  else {
    m_read_data_fifo            = new wide_fifo*                [m_num_ports];
    m_busy_fifo                 = new bool_fifo*                [m_num_ports];
    m_read_event_queue          = new sc_event_queue*           [m_num_ports];
    m_busy_event_queue          = new sc_event_queue*           [m_num_ports];
  }

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
  m_p_req_valid         = NULL;
  m_p_req_cntl          = NULL;
  m_p_req_adrs          = NULL;
  m_p_req_data          = NULL;
  m_p_req_data_be       = NULL;
  m_p_req_id            = NULL;
  m_p_req_priority      = NULL;
  m_p_req_route_id      = NULL;
  m_p_req_attribute     = NULL;
  m_p_req_rdy           = NULL;
  m_p_resp_valid        = NULL;
  m_p_resp_cntl         = NULL;
  m_p_resp_data         = NULL;
  m_p_resp_id           = NULL;
  m_p_resp_priority     = NULL;
  m_p_resp_route_id     = NULL;
  m_p_resp_rdy          = NULL;


  if (m_interface_type == PIF) {
    m_p_req_valid               = new bool_input*               [m_num_ports];
    m_p_req_cntl                = new uint_input*               [m_num_ports];
    m_p_req_adrs                = new uint_input*               [m_num_ports];
    m_p_req_data                = new wide_input*               [m_num_ports];
    m_p_req_data_be             = new uint_input*               [m_num_ports];
    m_p_req_id                  = new uint_input*               [m_num_ports];
    m_p_req_priority            = new uint_input*               [m_num_ports];
    m_p_req_route_id            = new uint_input*               [m_num_ports];
    m_p_req_attribute           = new uint_input*               [m_num_ports];
    m_p_req_rdy                 = new bool_output*              [m_num_ports];
    m_p_resp_valid              = new bool_output*              [m_num_ports];
    m_p_resp_cntl               = new uint_output*              [m_num_ports];
    m_p_resp_data               = new wide_output*              [m_num_ports];
    m_p_resp_id                 = new uint_output*              [m_num_ports];
    m_p_resp_priority           = new uint_output*              [m_num_ports];
    m_p_resp_route_id           = new uint_output*              [m_num_ports];
    m_p_resp_rdy                = new bool_input*               [m_num_ports];
  }
  else {
    m_p_en                      = new bool_input*               [m_num_ports];
    m_p_addr                    = new uint_input*               [m_num_ports];
    m_p_lane                    = new uint_input*               [m_num_ports];
    m_p_wrdata                  = new wide_input*               [m_num_ports];
    m_p_wr                      = new bool_input*               [m_num_ports];
    m_p_load                    = new bool_input*               [m_num_ports];
    m_p_retire                  = new bool_input*               [m_num_ports];
    m_p_flush                   = new bool_input*               [m_num_ports];
    m_p_lock                    = new bool_input*               [m_num_ports];
    m_p_check_wr                = new wide_input*               [m_num_ports];
    m_p_check                   = new wide_output*              [m_num_ports];
    m_p_xfer_en                 = new bool_input*               [m_num_ports];
    m_p_busy                    = new bool_output*              [m_num_ports];
    m_p_data                    = new wide_output*              [m_num_ports];
  }

  for (u32 port=0; port<m_num_ports; ++port) {

    ostringstream oss1;
    oss1 << "m_debug_exports[" << port << "]";
    m_debug_exports[port]       = new sc_export<xtsc_debug_if>(oss1.str().c_str());

    ostringstream oss2;
    oss2 << "m_debug_impl[" << port << "]";
    m_debug_impl[port]          = new xtsc_debug_if_impl(oss2.str().c_str(), *this, port);

    (*m_debug_exports[port])(*m_debug_impl[port]);

    m_data[port] = new sc_bv_base((int)m_width8*8);
    *m_data[port] = 0;

    m_data_to_be_written[port] = new sc_bv_base((int)m_width8*8);
    *m_data_to_be_written[port] = 0;

    if (m_interface_type == PIF) {
      m_rcw_compare_data[port]  = new sc_bv_base((int)m_width8*8);
      *m_rcw_compare_data[port] = 0;
    }
    else {
      sc_length_context length(m_width8*8);
      ostringstream oss1;
      oss1 << "m_read_data_fifo[" << port << "]";
      m_read_data_fifo[port]    = new wide_fifo(oss1.str().c_str(), (m_read_delay_value + 1) * 2);
      ostringstream oss2;
      oss2 << "m_busy_fifo[" << port << "]";
      m_busy_fifo[port]         = new bool_fifo(oss2.str().c_str(), (m_read_delay_value + 1) * 2);
      ostringstream oss3;
      oss3 << "m_read_event_queue[" << port << "]";
      m_read_event_queue[port]  = new sc_event_queue(oss3.str().c_str());
      if (m_has_busy) {
        ostringstream oss4;
        oss4 << "m_busy_event_queue[" << port << "]";
        m_busy_event_queue[port]  = new sc_event_queue(oss4.str().c_str());
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
      m_p_req_rdy           [port] = NULL;
      m_p_resp_valid        [port] = NULL;
      m_p_resp_cntl         [port] = NULL;
      m_p_resp_data         [port] = NULL;
      m_p_resp_id           [port] = NULL;
      m_p_resp_priority     [port] = NULL;
      m_p_resp_route_id     [port] = NULL;
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
        m_p_lock            [port] = &add_bool_input ("DRam0Lock", m_append_id, port);
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
        m_p_lock            [port] = &add_bool_input ("DRam1Lock", m_append_id, port);
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
          m_p_lane          [port] = &add_uint_input ("IRam0WordEn", m_enable_bits, m_append_id, port);
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
          m_p_lane          [port] = &add_uint_input ("IRam1WordEn", m_enable_bits, m_append_id, port);
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
          m_p_lane          [port] = &add_uint_input ("IRom0WordEn", m_enable_bits, m_append_id, port);
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
        else {
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
      oss1 << "pif_request_thread_" << port;
      declare_thread_process(pif_request_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, pif_request_thread);
      sensitive << m_p_req_valid[port]->pos() << m_p_req_rdy[port]->pos();
      ostringstream oss2;
      oss2 << "pif_drive_req_rdy_thread_" << port;
      declare_thread_process(pif_drive_req_rdy_thread_handle, oss2.str().c_str(), SC_CURRENT_USER_MODULE, pif_drive_req_rdy_thread);
      ostringstream oss3;
      oss3 << "pif_respond_thread_" << port;
      declare_thread_process(pif_respond_thread_handle, oss3.str().c_str(), SC_CURRENT_USER_MODULE, pif_respond_thread);
    }
    else {
      ostringstream oss1;
      oss1 << "lcl_request_thread_" << port;
      declare_thread_process(lcl_request_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, lcl_request_thread);
      sensitive << m_p_en[port]->pos();
      ostringstream oss2;
      oss2 << "lcl_drive_read_data_thread_" << port;
      declare_thread_process(lcl_drive_read_data_thread_handle, oss2.str().c_str(), SC_CURRENT_USER_MODULE, lcl_drive_read_data_thread);
      sensitive << *m_read_event_queue[port];
      if (m_has_busy) {
        ostringstream oss3;
        oss3 << "lcl_drive_busy_thread_" << port;
        declare_thread_process(lcl_drive_busy_thread_handle, oss3.str().c_str(), SC_CURRENT_USER_MODULE, lcl_drive_busy_thread);
        sensitive << *m_busy_event_queue[port];
      }
    }
  }
  // Restore SystemC
  sc_report_handler::set_actions("object already exists", SC_WARNING, original_action);

  m_p_memory->load_initial_values();

  // Log our construction
  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, "Constructed " << kind() << " '" << name() << "':");
  XTSC_LOG(m_text, ll, " memory_interface       = "                 << m_interface_uc);
  if (m_interface_type == PIF) {
  XTSC_LOG(m_text, ll, " inbound_pif            = "                 << boolalpha << m_inbound_pif);
  }
  XTSC_LOG(m_text, ll, " num_ports              = "   << dec        << m_num_ports);
  XTSC_LOG(m_text, ll, " port_name_suffix       = "                 << m_suffix);
  XTSC_LOG(m_text, ll, " start_byte_address     = 0x" << hex        << m_start_address8);
  XTSC_LOG(m_text, ll, " memory_byte_size       = 0x" << hex        << m_size8 << (m_size8 ? " " : " (4GB)"));
  XTSC_LOG(m_text, ll, " End byte address       = 0x" << hex        << m_end_address8);
  XTSC_LOG(m_text, ll, " byte_width             = "   << dec        << m_width8);
  XTSC_LOG(m_text, ll, " use_fast_access        = "   << boolalpha << m_use_fast_access);
  XTSC_LOG(m_text, ll, " big_endian             = "   << boolalpha  << m_big_endian);
  XTSC_LOG(m_text, ll, " page_byte_size         = 0x" << hex        << m_p_memory->m_page_size8);
  XTSC_LOG(m_text, ll, " initial_value_file     = "   << hex        << m_p_memory->m_initial_value_file);
  XTSC_LOG(m_text, ll, " memory_fill_byte       = 0x" << hex        << (u32) m_p_memory->m_memory_fill_byte);
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
  XTSC_LOG(m_text, ll, " sample_phase           = "   << dec        << sample_phase << " (" << m_sample_phase << ")");
  XTSC_LOG(m_text, ll, " drive_phase            = "   << dec        << drive_phase << " (" << m_drive_phase << ")");
  if (m_interface_type == PIF) {
  XTSC_LOG(m_text, ll, " request_fifo_depth     = "   << dec        << m_request_fifo_depth);
  XTSC_LOG(m_text, ll, " has_request_id         = "   << boolalpha  << m_has_request_id);
  XTSC_LOG(m_text, ll, " write_responses        = "   << boolalpha  << m_write_responses);
  XTSC_LOG(m_text, ll, " route_id_bits          = "   << dec        << m_route_id_bits);
  XTSC_LOG(m_text, ll, " void_resp_cntl         = 0x" << hex        << m_resp_cntl_zero.get_value());
  XTSC_LOG(m_text, ll, " read_delay             = "   << dec        << memory_parms.get_u32("read_delay"));
  XTSC_LOG(m_text, ll, " write_delay            = "   << dec        << memory_parms.get_u32("write_delay"));
  XTSC_LOG(m_text, ll, " block_read_delay       = "   << dec        << memory_parms.get_u32("block_read_delay"));
  XTSC_LOG(m_text, ll, " block_read_repeat      = "   << dec        << memory_parms.get_u32("block_read_repeat"));
  XTSC_LOG(m_text, ll, " burst_read_delay       = "   << dec        << memory_parms.get_u32("burst_read_delay"));
  XTSC_LOG(m_text, ll, " burst_read_repeat      = "   << dec        << memory_parms.get_u32("burst_read_repeat"));
  XTSC_LOG(m_text, ll, " block_write_delay      = "   << dec        << memory_parms.get_u32("block_write_delay"));
  XTSC_LOG(m_text, ll, " block_write_repeat     = "   << dec        << memory_parms.get_u32("block_write_repeat"));
  XTSC_LOG(m_text, ll, " block_write_response   = "   << dec        << memory_parms.get_u32("block_write_response"));
  XTSC_LOG(m_text, ll, " burst_write_delay      = "   << dec        << memory_parms.get_u32("burst_write_delay"));
  XTSC_LOG(m_text, ll, " burst_write_repeat     = "   << dec        << memory_parms.get_u32("burst_write_repeat"));
  XTSC_LOG(m_text, ll, " burst_write_response   = "   << dec        << memory_parms.get_u32("burst_write_response"));
  XTSC_LOG(m_text, ll, " rcw_repeat             = "   << dec        << memory_parms.get_u32("rcw_repeat"));
  XTSC_LOG(m_text, ll, " rcw_response           = "   << dec        << memory_parms.get_u32("rcw_response"));
  } else {
  XTSC_LOG(m_text, ll, " read_delay             = "   << dec        << memory_parms.get_u32("read_delay"));
  XTSC_LOG(m_text, ll, " address_bits           = "   << dec        << m_address_bits);
  XTSC_LOG(m_text, ll, " has_busy               = "   << boolalpha  << m_has_busy);
  }
  XTSC_LOG(m_text, ll, " busy_percentage        = "   << dec        << m_busy_percentage << "%");
  if ((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) {
  XTSC_LOG(m_text, ll, " has_lock               = "   << boolalpha  << m_has_lock);
  }
  if ((m_interface_type == DRAM0) || (m_interface_type == DRAM1) ||
      (m_interface_type == IRAM0) || (m_interface_type == IRAM1)) {
  XTSC_LOG(m_text, ll, " check_bits             = "   << dec        << m_check_bits);
  }
  if ((m_interface_type != DROM0) && (m_interface_type != XLMI0) && (m_interface_type != PIF)) {
  XTSC_LOG(m_text, ll, " has_xfer_en               = "              << boolalpha << m_has_xfer_en);
  }
  XTSC_LOG(m_text, ll, " cbox                   = "   << boolalpha  << m_cbox);
  XTSC_LOG(m_text, ll, " m_address_mask         = 0x" << hex        << m_address_mask);
  XTSC_LOG(m_text, ll, " m_address_shift        = "   << dec        << m_address_shift);
  ostringstream oss;
  oss << "Port List:" << endl;
  dump_ports(oss);
  xtsc_log_multiline(m_text, ll, oss.str(), 2);

}



xtsc_component::xtsc_memory_pin::~xtsc_memory_pin(void) {
  // Do any required clean-up here
}



string xtsc_component::xtsc_memory_pin::adjust_name_and_check_size(const string&                                port_name,
                                                                   const xtsc_tlm2pin_memory_transactor&        tlm2pin,
                                                                   u32                                          tlm2pin_port,
                                                                   const set_string&                            transactor_set) const
{
  string transactor_port_name(port_name);
  if (m_append_id) {
    transactor_port_name.erase(transactor_port_name.size()-1);
  }
  if (tlm2pin.get_append_id()) {
    ostringstream oss;
    oss << tlm2pin_port;
    transactor_port_name += oss.str();
  }
  u32 memory_width = get_bit_width(port_name);
  u32 transactor_width = tlm2pin.get_bit_width(transactor_port_name);
  if (memory_width != transactor_width) {
    ostringstream oss;
    oss << "Signal mismatch in connect():  " << kind() << " '" << name() << "' has a port named \"" << port_name
        << "\" with a width of " << memory_width << " bits, but port \"" << transactor_port_name << "\" in " << tlm2pin.kind()
        << " '" << tlm2pin.name() << "' has " << transactor_width << " bits.";
    throw xtsc_exception(oss.str());
  }
  return transactor_port_name;
} 



void xtsc_component::xtsc_memory_pin::dump_set_string(ostringstream& oss, const set_string& strings, const string& indent) {
  for (set_string::const_iterator is = strings.begin(); is != strings.end(); ++is) {
    oss << indent << *is << endl;
  }
}



xtsc::u32 xtsc_component::xtsc_memory_pin::connect(xtsc_tlm2pin_memory_transactor&      tlm2pin,
                                                   u32                                  tlm2pin_port,
                                                   u32                                  mem_port,
                                                   bool                                 single_connect)
{
  u32 tran_ports = tlm2pin.get_num_ports();
  if (tlm2pin_port >= tran_ports) {
    ostringstream oss;
    oss << "Invalid tlm2pin_port=" << tlm2pin_port << " in connect(): " << endl;
    oss << tlm2pin.kind() << " '" << tlm2pin.name() << "' has " << tran_ports << " ports numbered from 0 to " << tran_ports-1
        << endl;
    throw xtsc_exception(oss.str());
  }
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }

  u32 num_connected = 0;

  while ((tlm2pin_port < tran_ports) && (mem_port < m_num_ports)) {

    {
      // Connect sc_in<bool> ports of xtsc_memory_pin to sc_out<bool> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_bool_input_set(mem_port);
      set_string tran_set = tlm2pin.get_bool_output_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_memory_pin::connect():" << endl;
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
      // Connect sc_in<sc_uint_base> ports of xtsc_memory_pin to sc_out<sc_uint_base> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_uint_input_set(mem_port);
      set_string tran_set = tlm2pin.get_uint_output_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_memory_pin::connect():" << endl;
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
      // Connect sc_in<sc_bv_base> ports of xtsc_memory_pin to sc_out<sc_bv_base> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_wide_input_set(mem_port);
      set_string tran_set = tlm2pin.get_wide_output_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_memory_pin::connect():" << endl;
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
      // Connect sc_out<bool> ports of xtsc_memory_pin to sc_in<bool> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_bool_output_set(mem_port);
      set_string tran_set = tlm2pin.get_bool_input_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_memory_pin::connect():" << endl;
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
      // Connect sc_out<sc_uint_base> ports of xtsc_memory_pin to sc_in<sc_uint_base> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_uint_output_set(mem_port);
      set_string tran_set = tlm2pin.get_uint_input_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_memory_pin::connect():" << endl;
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
      // Connect sc_out<sc_bv_base> ports of xtsc_memory_pin to sc_in<sc_bv_base> ports of xtsc_tlm2pin_memory_transactor 
      set_string mem_set = get_wide_output_set(mem_port);
      set_string tran_set = tlm2pin.get_wide_input_set(tlm2pin_port);
      if (mem_set.size() != tran_set.size()) {
        ostringstream oss;
        oss << "Signal set sizes don't match in xtsc_memory_pin::connect():" << endl;
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
    if (!tlm2pin.has_dso()) {
      (*tlm2pin.m_debug_ports[tlm2pin_port])(*m_debug_exports[mem_port]);
    }

    mem_port += 1;
    tlm2pin_port += 1;
    num_connected += 1;

    if (single_connect) break;
    if (tlm2pin_port >= tran_ports) break;
    if (mem_port >= m_num_ports) break;
    if (m_debug_impl[mem_port]->is_connected()) break;
  }

  return num_connected;
}



void xtsc_component::xtsc_memory_pin::end_of_elaboration(void) {
} 



void xtsc_component::xtsc_memory_pin::start_of_simulation(void) {
  reset();
} 



void xtsc_component::xtsc_memory_pin::reset(bool hard_reset) {
  XTSC_INFO(m_text, "xtsc_memory_pin::reset()");

  m_next_port_lcl_request_thread         = 0;
  m_next_port_pif_request_thread         = 0;
  m_next_port_pif_drive_req_rdy_thread   = 0;
  m_next_port_pif_respond_thread         = 0;
  m_next_port_lcl_drive_read_data_thread = 0;
  m_next_port_lcl_drive_busy_thread      = 0;

  for (u32 port=0; port<m_num_ports; ++port) {
    if (m_interface_type == PIF) {
      m_p_req_rdy[port]->write(true);
      m_block_write_address[port]       = 0;
      m_burst_write_address[port]       = 0;
      m_first_block_write[port]         = true;
      m_first_burst_write[port]         = true;
      *m_rcw_compare_data[port]         = 0;
      m_last_action_time_stamp[port]    = sc_time_stamp() - sc_get_time_resolution();
      m_testing_busy[port]              = false;
    }
  }

  if (hard_reset) {
    m_p_memory->load_initial_values();
  }
}



void xtsc_component::xtsc_memory_pin::sync_to_sample_phase(void) {
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
  else {
    wait(m_sample_phase_plus_one - phase_now);
  }
} 



void xtsc_component::xtsc_memory_pin::sync_to_drive_phase(void) {
  sc_time now = sc_time_stamp();
  sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
  if (m_has_posedge_offset) {
    if (phase_now < m_posedge_offset) {
      phase_now += m_clock_period;
    }
    phase_now -= m_posedge_offset;
  }
  if (phase_now < m_drive_phase) {
    wait(m_drive_phase - phase_now);
  }
  else if (phase_now > m_drive_phase) {
    wait(m_drive_phase_plus_one - phase_now);
  }
} 



void xtsc_component::xtsc_memory_pin::lcl_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_request_thread++;

  // For dual-ported case, get other port
  u32 other = ((port == 0) ? 1 : 0);

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

      bool need_to_schedule_read_data_deassert = false;

      // Wait for a request: m_p_en[port]->pos()
      wait();

      XTSC_DEBUG(m_text, "lcl_request_thread(): awoke from wait()");

      // Sync to sample phase
      sync_to_sample_phase();

      XTSC_DEBUG(m_text, "lcl_request_thread(): completed sync_to_sample_phase()");

      bool busy = false;

      while (m_p_en[port]->read()) {
        xtsc_address addr8 = ((m_p_addr[port]->read().to_uint() & m_address_mask) << m_address_shift) + m_start_address8;
        xtsc_byte_enables byte_enables = 0xFFFF;
        xtsc_byte_enables lane         = 0xFFFF;
        if (m_p_lane[port]) {
          lane = (xtsc_byte_enables) m_p_lane[port]->read().to_uint64();
          byte_enables = lane;
          if ((m_interface_type == IRAM0) || (m_interface_type == IRAM1) || (m_interface_type == IROM0)) {
            // Convert word enables (1 "word" is 32 bits) to byte enables (64-bit IRAM0|IRAM1|IROM0 only)
            xtsc_byte_enables be = 0x0000;
            if (byte_enables & 0x1) be |= 0x000F;
            if (byte_enables & 0x2) be |= 0x00F0;
            if (byte_enables & 0x4) be |= 0x0F00;
            if (byte_enables & 0x8) be |= 0xF000;
            byte_enables = be;
          }
          if (m_big_endian) {
            swizzle_byte_enables(byte_enables);
          }
        }
        if (m_has_busy && m_busy_percentage && ((rand() % 100) < m_busy_percentage)) {
          XTSC_INFO(m_text, "Busy 0x" << hex << addr8);
          m_busy_event_queue[port]->notify(m_sample_to_drive_busy_delay);
          m_busy_fifo[port]->write(true);
          busy = true;
        }
        else if (m_p_wr[port] && m_p_wr[port]->read()) {
          // Handle a write request
          if (need_to_schedule_read_data_deassert) {
            *m_data[port] = 0;
            m_read_event_queue[port]->notify(m_sample_to_drive_data_delay);
            m_read_data_fifo[port]->write(*m_data[port]);
            need_to_schedule_read_data_deassert = false;
          }
          *m_data_to_be_written[port] = m_p_wrdata[port]->read();
          // If we're dual-ported and the other port has a read this cycle (or if we have more then 2 ports),
          // then we delay handling of write requests by one delta cycle so that if there is also a read this
          // clock cycle to the same address it will return the original (old) data regardless of SystemC
          // thread scheduling indeterminancy.
          if (((m_num_ports == 2) && m_p_en[other]->read() && !m_p_wr[other]->read()) || (m_num_ports > 2)) {
            wait(SC_ZERO_TIME);
          }
          u32 delta      = (m_big_endian ? -8 : +8);
          u32 bit_offset = (m_big_endian ? (8 * (m_width8 - 1)) : 0);
          u32 mem_offset = m_p_memory->get_page_offset(addr8);
          u32 page       = m_p_memory->get_page(addr8);
          u64 bytes      = byte_enables;
          for (u32 i = 0; i<m_width8; ++i) {
            if (bytes & 0x1) {
              *(m_p_memory->m_page_table[page]+mem_offset) = (u8) m_data_to_be_written[port]->range(bit_offset+7, bit_offset).to_uint();
            }
            bytes >>= 1;
            mem_offset += 1;
            bit_offset += delta;
          }
          XTSC_INFO(m_text, "Write  [0x" << hex << setfill('0') << setw(8) << addr8 << "/0x" << setw(m_enable_bits/4) << lane <<
                            "]: 0x" << *m_data_to_be_written[port]);
        }
        else {
          // Handle a read request
          *m_data[port]  = 0;
          u32 delta      = (m_big_endian ? -8 : +8);
          u32 bit_offset = (m_big_endian ? (8 * (m_width8 - 1)) : 0);
          u32 mem_offset = m_p_memory->get_page_offset(addr8);
          u32 page       = m_p_memory->get_page(addr8);
          u64 bytes      = byte_enables;
          for (u32 i = 0; i<m_width8; ++i) {
            if (bytes & 0x1) {
              m_data[port]->range(bit_offset+7, bit_offset) = *(m_p_memory->m_page_table[page]+mem_offset);
            }
            bytes >>= 1;
            mem_offset += 1;
            bit_offset += delta;
          }
          m_read_event_queue[port]->notify(m_sample_to_drive_data_delay);
          m_read_data_fifo[port]->write(*m_data[port]);
          XTSC_INFO(m_text, "Read   [0x" << hex << setfill('0') << setw(8) << addr8 << "/0x" << setw(m_enable_bits/4) << lane <<
                            "]: 0x" << *m_data[port]);

          need_to_schedule_read_data_deassert = true;
        }

        // Wait one clock period
        wait(m_clock_period);

        // Schedule busy to be deasserted, if applicable
        if (busy) {
          m_busy_fifo[port]->write(false);
          m_busy_event_queue[port]->notify(m_sample_to_drive_busy_delay);
        }
      }

      if (need_to_schedule_read_data_deassert) {
        *m_data[port] = 0;
        m_read_event_queue[port]->notify(m_sample_to_drive_data_delay);
        m_read_data_fifo[port]->write(*m_data[port]);
        need_to_schedule_read_data_deassert = false;
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



void xtsc_component::xtsc_memory_pin::pif_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_request_thread++;

  try {

    XTSC_INFO(m_text, "in pif_request_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for posedge POReqValid or PIReqRdy
      XTSC_DEBUG(m_text, "pif_request_thread(): waiting for posedge POReqValid/PIReqRdy");
      wait();
      XTSC_DEBUG(m_text, "pif_request_thread(): got posedge POReqValid/PIReqRdy");

      // Sync to sample phase
      sync_to_sample_phase();
      XTSC_DEBUG(m_text, "pif_request_thread(): completed sync_to_sample_phase()");

      // While valid is asserted
      while (m_p_req_valid[port]->read()) {
        // Capture one request per clock period while both POReqValid and PIReqRdy are asserted
        if (m_p_req_rdy[port]->read()) {
          pif_req_info *p_info = new_pif_req_info(port);
          m_pif_req_fifo[port].push_back(p_info);
          m_pif_req_event[port].notify(SC_ZERO_TIME);
          m_pif_req_rdy_event[port].notify(SC_ZERO_TIME);
          XTSC_VERBOSE(m_text, *p_info << " (pif_request_thread)");
        }
        else {
          XTSC_INFO(m_text, " Busy [" << port << "] 0x" << hex << setfill('0') << setw(8) << m_p_req_adrs[port]->read() <<
                            (m_testing_busy[port] ? " (test)" : ""));
          m_pif_req_rdy_event[port].notify(SC_ZERO_TIME);
          m_testing_busy[port] = false;
        }
        wait(m_clock_period);
      }

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_request_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_memory_pin::pif_drive_req_rdy_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_drive_req_rdy_thread++;

  try {

    XTSC_INFO(m_text, "in pif_drive_req_rdy_thread[" << port << "]");

    // Work around Mentor Graphics bug.  Emails of 22 && 23 Sep 2008.  Should be fixed in Modelsim 6.3i.
    m_p_req_rdy[port]->write(false);
    m_p_req_rdy[port]->write(true);

    // Loop forever
    while (true) {

      XTSC_DEBUG(m_text, "pif_drive_req_rdy_thread(): calling wait()");
      wait(m_pif_req_rdy_event[port]);
      XTSC_DEBUG(m_text, "pif_drive_req_rdy_thread(): awoke from wait()");
      
      // Sync to m_drive_phase
      sync_to_drive_phase();
      XTSC_DEBUG(m_text, "pif_drive_req_rdy_thread(): returned from sync_to_drive_phase()");

      if (m_pif_req_fifo[port].size() >= m_request_fifo_depth) {
        m_p_req_rdy[port]->write(false);
        m_testing_busy[port] = false;
      }
      else if (m_busy_percentage && (m_testing_busy[port] || ((rand() % 100) < m_busy_percentage))) {
        m_p_req_rdy[port]->write(false);
        m_testing_busy[port] = true;
      }
      else {
        m_p_req_rdy[port]->write(true);
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



void xtsc_component::xtsc_memory_pin::pif_respond_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_respond_thread++;

  try {

    XTSC_INFO(m_text, "in pif_respond_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for a request
      wait(m_pif_req_event[port]);
      XTSC_DEBUG(m_text, "pif_respond_thread(): awoke from wait(m_pif_req_event)");

      // Sync to m_drive_phase
      sync_to_drive_phase();
      XTSC_DEBUG(m_text, "pif_respond_thread(): returned from sync_to_drive_phase()");

      // Process up to one request per clock period while there are requests avaiable.
      while (!m_pif_req_fifo[port].empty()) {
        pif_req_info *p_info = m_pif_req_fifo[port].front();
        m_pif_req_fifo[port].pop_front();
        m_pif_req_rdy_event[port].notify(SC_ZERO_TIME);
        m_respond_event[port].notify(SC_ZERO_TIME);

        // Determine delay
        sc_time delay = SC_ZERO_TIME;
        switch (p_info->m_req_cntl.get_type()) {
          case req_cntl::READ: {
            delay  = m_read_delay;
            break;
          }
          case req_cntl::BLOCK_READ: {
            delay  = m_block_read_delay;
            break;
          }
          case req_cntl::BURST_READ: {
            delay  = m_burst_read_delay;
            break;
          }
          case req_cntl::RCW: {
            delay  = (p_info->m_req_cntl.get_last_transfer() ?  m_rcw_response : m_rcw_repeat);
            break;
          }
          case req_cntl::WRITE: {
            delay  = m_write_delay;
            break;
          }
          case req_cntl::BLOCK_WRITE:  {
            bool last = p_info->m_req_cntl.get_last_transfer();
            delay  = (m_first_block_write[port] ?  m_block_write_delay : (last ? m_block_write_response : m_block_write_repeat));
            break;
          }
          case req_cntl::BURST_WRITE:  {
            bool last = p_info->m_req_cntl.get_last_transfer();
            delay  = (m_first_burst_write[port] ?  m_burst_write_delay : (last ? m_burst_write_response : m_burst_write_repeat));
            break;
          }
          default: {
            // Unrecognized request type: If last transfer, then respond with AErr, else drop on floor
            if (p_info->m_req_cntl.get_last_transfer()) {
              XTSC_INFO(m_text, *p_info << " (Rsp=AErr)");
              m_resp_cntl.init(resp_cntl::AErr, true);
              pif_drive_response(*p_info, m_resp_cntl, *m_data[port]);
            }
            else {
              XTSC_INFO(m_text, *p_info << " (Dropping on floor)");
              wait(m_clock_period);
            }
            continue;
          }
        }
        if (delay != SC_ZERO_TIME) {
          XTSC_DEBUG(m_text, "pif_respond_thread() doing wait for " << delay);
          wait(delay);
        }

        // Initialize some common values
        m_resp_cntl.init(resp_cntl::OK, true);

        switch (p_info->m_req_cntl.get_type()) {
          case req_cntl::READ: {
            do_read(*p_info);
            break;
          }
          case req_cntl::BLOCK_READ: {
            do_block_read(*p_info);
            break;
          }
          case req_cntl::BURST_READ: {
            do_burst_read(*p_info);
            break;
          }
          case req_cntl::RCW: {
            do_rcw(*p_info);
            break;
          }
          case req_cntl::WRITE: {
            do_write(*p_info);
            break;
          }
          case req_cntl::BLOCK_WRITE: {
            do_block_write(*p_info);
            break;
          }
          case req_cntl::BURST_WRITE: {
            do_burst_write(*p_info);
            break;
          }
          default: {
            ostringstream oss;
            oss << kind() << " '" << name() << "': ";
            oss << "Unrecognized request type = 0x" << hex << p_info->m_req_cntl.get_type();
            throw xtsc_exception(oss.str());
          }
        }

        delete_pif_req_info(p_info);

      }

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_respond_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_memory_pin::lcl_drive_read_data_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_drive_read_data_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_drive_read_data_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait to be notified: m_read_event_queue
      wait();

      if (!m_read_data_fifo[port]->nb_read(*m_data[port])) {
        ostringstream oss;
        oss << "Program bug in xtsc_memory_pin::lcl_drive_read_data_thread: m_read_data_fifo[" << port << "] is empty";
        throw xtsc_exception(oss.str());
      }

      m_p_data[port]->write(*m_data[port]);

      XTSC_VERBOSE(m_text, "lcl_drive_read_data_thread() driving read data 0x" << hex << *m_data[port]);

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



void xtsc_component::xtsc_memory_pin::lcl_drive_busy_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_drive_busy_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_drive_busy_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait to be notified: m_busy_event_queue
      wait();

      bool busy;
      if (!m_busy_fifo[port]->nb_read(busy)) {
        ostringstream oss;
        oss << "Program bug in xtsc_memory_pin::lcl_drive_busy_thread: m_busy_fifo[" << port << "] is empty";
        throw xtsc_exception(oss.str());
      }

      m_p_busy[port]->write(busy);

      XTSC_VERBOSE(m_text, "lcl_drive_busy_thread() driving busy=" << busy);

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



void xtsc_component::xtsc_memory_pin::do_read(const pif_req_info& info) {
  u32           port            = info.m_port;
  xtsc_address  address8        = info.m_address;
  xtsc_address  addr8           = address8 & m_address_mask;
  u32           page            = m_p_memory->get_page(addr8);
  u32           mem_offset      = m_p_memory->get_page_offset(addr8);
  u32           delta           = (m_big_endian ? -8 : +8);
  u32           bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);
  u64           bytes           = info.m_fixed_byte_enables;

  XTSC_DEBUG(m_text, "do_read: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                     " page=" << dec << page << " mem_offset=" << mem_offset << " bit_offset=" << bit_offset <<
                     " bytes=0x" << hex << bytes);

  for (u32 i = 0; i<m_width8; ++i) {
    if (bytes & 0x1) {
      m_data[port]->range(bit_offset+7, bit_offset) = *(m_p_memory->m_page_table[page]+mem_offset);
    }
    bytes >>= 1;
    mem_offset += 1;
    bit_offset += delta;
  }

  XTSC_INFO(m_text, info << "=0x" << hex << *m_data[port]);

  pif_drive_response(info, m_resp_cntl, *m_data[port]);
}



void xtsc_component::xtsc_memory_pin::do_block_read(const pif_req_info& info) {
  u32           port            = info.m_port;
  xtsc_address  address8        = info.m_address;
  xtsc_address  addr8           = address8 & m_address_mask;
  u32           page            = m_p_memory->get_page(addr8);
  u32           mem_offset      = m_p_memory->get_page_offset(addr8);
  u32           delta           = (m_big_endian ? -8 : +8);
  u32           num_transfers   = info.m_req_cntl.get_num_transfers();
  u32           block_size      = num_transfers * m_width8;
  xtsc_address  addr8_limit     = addr8 | (block_size - 1);

  XTSC_DEBUG(m_text, "do_block_read: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                     " page=" << dec << page << " mem_offset=" << mem_offset << " num_transfers=" << num_transfers << 
                     " block_size=" << block_size << " addr8_limit=0x" << hex << setw(8) << addr8_limit);

  for (u32 j=0; j<num_transfers; ++j) {
    u32         bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);
    bool        last            = ((j+1)==num_transfers);
    for (u32 i = 0; i<m_width8; ++i) {
      m_data[port]->range(bit_offset+7, bit_offset) = *(m_p_memory->m_page_table[page]+mem_offset);
      mem_offset += 1;
      addr8 += 1;
      bit_offset += delta;
    }
    m_resp_cntl.set_last_transfer(last);

    XTSC_INFO(m_text, "BlR    0x" << hex << (address8 + m_width8*j) << "=0x" << *m_data[port] << " " << m_resp_cntl);

    pif_drive_response(info, m_resp_cntl, *m_data[port]);
    if (!last && (m_block_read_repeat > m_clock_period)) {
      wait(m_block_read_repeat - m_clock_period);
    }
    if (addr8 > addr8_limit) {
      xtsc_address old_addr8 = addr8;
      addr8 = old_addr8 - block_size;
      mem_offset = m_p_memory->get_page_offset(addr8);
      XTSC_DEBUG(m_text, "do_block_read: after transfer #" << (j+1) << ", wrapping address from 0x" << hex << setfill('0') <<
                         setw(8) << old_addr8 << " to 0x" << setw(8) << addr8 << " mem_offset=" << mem_offset);
    }
  }
}



void xtsc_component::xtsc_memory_pin::do_burst_read(const pif_req_info& info) {
  u32           port            = info.m_port;
  xtsc_address  address8        = info.m_address;
  xtsc_address  addr8           = address8 & m_address_mask;
  u32           page            = m_p_memory->get_page(addr8);
  u32           mem_offset      = m_p_memory->get_page_offset(addr8);
  u32           delta           = (m_big_endian ? -8 : +8);
  u32           num_transfers   = info.m_req_cntl.get_num_transfers();

  XTSC_DEBUG(m_text, "do_burst_read: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                     " page=" << dec << page << " mem_offset=" << mem_offset << " num_transfers=" << num_transfers);

  for (u32 j=0; j<num_transfers; ++j) {
    u32         bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);
    bool        last            = ((j+1)==num_transfers);
    for (u32 i = 0; i<m_width8; ++i) {
      m_data[port]->range(bit_offset+7, bit_offset) = *(m_p_memory->m_page_table[page]+mem_offset);
      mem_offset += 1;
      addr8 += 1;
      bit_offset += delta;
    }
    m_resp_cntl.set_last_transfer(last);

    XTSC_INFO(m_text, "BuR    0x" << hex << (address8 + m_width8*j) << "=0x" << *m_data[port] << " " << m_resp_cntl);

    pif_drive_response(info, m_resp_cntl, *m_data[port]);
    if (!last && (m_burst_read_repeat > m_clock_period)) {
      wait(m_burst_read_repeat - m_clock_period);
    }
  }
}



void xtsc_component::xtsc_memory_pin::do_rcw(const pif_req_info& info) {
  u32           port            = info.m_port;
  if (!info.m_req_cntl.get_last_transfer()) {
    *m_rcw_compare_data[port] = info.m_data;
    XTSC_INFO(m_text, info);
    wait(m_clock_period);
  }
  else {
    xtsc_address  address8        = info.m_address;
    xtsc_address  addr8           = address8 & m_address_mask;
    u32           page            = m_p_memory->get_page(addr8);
    u32           mem_offset      = m_p_memory->get_page_offset(addr8);
    u32           delta           = (m_big_endian ? -8 : +8);
    u32           bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);
    u64           bytes           = info.m_fixed_byte_enables;
    bool          data_matches    = true;

    XTSC_DEBUG(m_text, "do_rcw: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                       " page=" << dec << page << " mem_offset=" << mem_offset << " bit_offset=" << bit_offset <<
                       " bytes=0x" << hex << bytes);

    // Compare data and capture old memory value in m_data
    for (u32 i = 0; i<m_width8; ++i) {
      if (bytes & 0x1) {
        m_data[port]->range(bit_offset+7, bit_offset) = *(m_p_memory->m_page_table[page]+mem_offset);
        if (m_rcw_compare_data[port]->range(bit_offset+7, bit_offset) != m_data[port]->range(bit_offset+7, bit_offset)) {
          data_matches = false;
        }
      }
      bytes >>= 1;
      mem_offset += 1;
      bit_offset += delta;
    }

    if (data_matches) {
      mem_offset  = m_p_memory->get_page_offset(addr8);
      bit_offset  = 8 * (m_big_endian ? (m_width8 - 1) : 0);
      bytes       = info.m_fixed_byte_enables;
      for (u32 i = 0; i<m_width8; ++i) {
        if (bytes & 0x1) {
          *(m_p_memory->m_page_table[page]+mem_offset) = (u8) info.m_data.range(bit_offset+7, bit_offset).to_uint();
        }
        bytes >>= 1;
        mem_offset += 1;
        bit_offset += delta;
      }
    }

    XTSC_INFO(m_text, info << (data_matches ? " Match" : " NoMatch"));

    pif_drive_response(info, m_resp_cntl, *m_data[port]);
  }
}



void xtsc_component::xtsc_memory_pin::do_write(const pif_req_info& info) {
  u32           port            = info.m_port;
  xtsc_address  address8        = info.m_address;
  xtsc_address  addr8           = address8 & m_address_mask;
  u32           page            = m_p_memory->get_page(addr8);
  u32           mem_offset      = m_p_memory->get_page_offset(addr8);
  u32           delta           = (m_big_endian ? -8 : +8);
  u32           bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);
  u64           bytes           = info.m_fixed_byte_enables;

  XTSC_DEBUG(m_text, "do_write: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                     " page=" << dec << page << " mem_offset=" << mem_offset << " bit_offset=" << bit_offset <<
                     " bytes=0x" << hex << bytes);

  for (u32 i = 0; i<m_width8; ++i) {
    if (bytes & 0x1) {
      *(m_p_memory->m_page_table[page]+mem_offset) = (u8) info.m_data.range(bit_offset+7, bit_offset).to_uint();
    }
    bytes >>= 1;
    mem_offset += 1;
    bit_offset += delta;
  }

  XTSC_INFO(m_text, info);

  if (m_write_responses) {
    *m_data[port] = 0;
    pif_drive_response(info, m_resp_cntl, *m_data[port]);
  }
  else {
    wait(m_clock_period);
  }
}



void xtsc_component::xtsc_memory_pin::do_block_write(const pif_req_info& info) {
  u32           port            = info.m_port;
  xtsc_address  address8        = m_first_block_write[port] ? info.m_address : m_block_write_address[port];
  xtsc_address  addr8           = address8 & m_address_mask;
  u32           page            = m_p_memory->get_page(addr8);
  u32           mem_offset      = m_p_memory->get_page_offset(addr8);
  u32           delta           = (m_big_endian ? -8 : +8);
  u32           bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);

  XTSC_DEBUG(m_text, "do_block_write: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                     " page=" << dec << page << " mem_offset=" << mem_offset << " bit_offset=" << bit_offset);

  for (u32 i = 0; i<m_width8; ++i) {
    *(m_p_memory->m_page_table[page]+mem_offset) = (u8) info.m_data.range(bit_offset+7, bit_offset).to_uint();
    mem_offset += 1;
    bit_offset += delta;
  }

  XTSC_INFO(m_text, info);

  if (!info.m_req_cntl.get_last_transfer()) {
    m_first_block_write[port] = false;
    m_block_write_address[port] = addr8 + m_width8;
    wait(m_clock_period);
  }
  else {
    m_first_block_write[port] = true;
    if (m_write_responses) {
      *m_data[port] = 0;
      pif_drive_response(info, m_resp_cntl, *m_data[port]);
    }
    else {
      wait(m_clock_period);
    }
  }
}



void xtsc_component::xtsc_memory_pin::do_burst_write(const pif_req_info& info) {
  u32           port            = info.m_port;
  xtsc_address  address8        = m_first_burst_write[port] ? info.m_address : m_burst_write_address[port];
  xtsc_address  addr8           = address8 & m_address_mask;
  u32           page            = m_p_memory->get_page(addr8);
  u32           mem_offset      = m_p_memory->get_page_offset(addr8);
  u32           delta           = (m_big_endian ? -8 : +8);
  u32           bit_offset      = 8 * (m_big_endian ? (m_width8 - 1) : 0);
  u64           bytes           = info.m_fixed_byte_enables;

  XTSC_DEBUG(m_text, "do_burst_write: address8=0x" << hex << setfill('0') << setw(8) << address8 << " addr8=0x" << setw(8) << addr8 <<
                     " page=" << dec << page << " mem_offset=" << mem_offset << " bit_offset=" << bit_offset <<
                     " bytes=0x" << hex << bytes);

  for (u32 i = 0; i<m_width8; ++i) {
    if (bytes & 0x1) {
      *(m_p_memory->m_page_table[page]+mem_offset) = (u8) info.m_data.range(bit_offset+7, bit_offset).to_uint();
    }
    mem_offset += 1;
    bit_offset += delta;
    bytes >>= 1;
  }

  XTSC_INFO(m_text, info);

  if (!info.m_req_cntl.get_last_transfer()) {
    m_first_burst_write[port] = false;
    m_burst_write_address[port] = addr8 + m_width8;
    wait(m_clock_period);
  }
  else {
    m_first_burst_write[port] = true;
    if (m_write_responses) {
      *m_data[port] = 0;
      pif_drive_response(info, m_resp_cntl, *m_data[port]);
    }
    else {
      wait(m_clock_period);
    }
  }
}



void xtsc_component::xtsc_memory_pin::pif_drive_response(const pif_req_info&  info,
                                                                   const resp_cntl&     response,
                                                                   const sc_bv_base&    data)
{
  u32 port = info.m_port;

  // Assert signals 
  m_p_resp_valid[port]->write(true);
  m_p_resp_cntl[port]->write(response.get_value());
  m_p_resp_data[port]->write(data);
  XTSC_VERBOSE(m_text, "Drive data=0x" << hex << data);
  if (m_p_resp_id[port]) m_p_resp_id[port]->write(info.m_id);
  m_p_resp_priority[port]->write(info.m_priority);
  if (m_p_resp_route_id[port]) m_p_resp_route_id[port]->write(info.m_route_id);

  // Drive in time chunks of one clock cycle until accepted
  bool ready = true;
  do {
    if (m_sample_phase == m_drive_phase) {
      wait(m_clock_period);
      ready = m_p_resp_rdy[port]->read();
    }
    else {
      wait(m_drive_to_sample_time);
      ready = m_p_resp_rdy[port]->read();
      XTSC_DEBUG(m_text, "pif_drive_response() at sample_phase: ready=" << ready);
      wait(m_sample_to_drive_time);
      XTSC_DEBUG(m_text, "pif_drive_response() at drive_phase");
    }
  } while (!ready);

  // Deassert signals
  m_p_resp_valid[port]->write(false);
  m_p_resp_cntl[port]->write(m_resp_cntl_zero.get_value());
  m_p_resp_data[port]->write(m_data_zero);
  if (m_p_resp_id[port]) m_p_resp_id[port]->write(m_id_zero);
  m_p_resp_priority[port]->write(m_priority_zero);
  if (m_p_resp_route_id[port]) m_p_resp_route_id[port]->write(m_route_id_zero);
}



bool_signal& xtsc_component::xtsc_memory_pin::create_bool_signal(const string& signal_name) {
  bool_signal *p_signal = new bool_signal(signal_name.c_str());
  m_map_bool_signal[signal_name] = p_signal;
  return *p_signal;
}



uint_signal& xtsc_component::xtsc_memory_pin::create_uint_signal(const string& signal_name, u32 num_bits) {
  sc_length_context length(num_bits);
  uint_signal *p_signal = new uint_signal(signal_name.c_str());
  m_map_uint_signal[signal_name] = p_signal;
  return *p_signal;
}



wide_signal& xtsc_component::xtsc_memory_pin::create_wide_signal(const string& signal_name, u32 num_bits) {
  sc_length_context length(num_bits);
  wide_signal *p_signal = new wide_signal(signal_name.c_str());
  m_map_wide_signal[signal_name] = p_signal;
  return *p_signal;
}



void xtsc_component::xtsc_memory_pin::swizzle_byte_enables(xtsc_byte_enables& byte_enables) const {
  xtsc_byte_enables swizzled = 0;
  for (u32 i=0; i<m_width8; i++) {
    swizzled <<= 1;
    swizzled |= byte_enables & 1;
    byte_enables >>= 1;
  }
  byte_enables = swizzled;
}



xtsc_component::xtsc_memory_pin::pif_req_info::pif_req_info(const xtsc_memory_pin& memory, u32 port) :
  m_memory      (memory),
  m_req_cntl    (0),
  m_data        ((int)m_memory.m_width8*8),
  m_id          (m_id_bits),
  m_priority    (2),
  m_route_id    (m_memory.m_route_id_bits ? m_memory.m_route_id_bits : 1)
{
  init(port);
}



xtsc_component::xtsc_memory_pin::pif_req_info *xtsc_component::xtsc_memory_pin::new_pif_req_info(u32 port) {
  if (m_pif_req_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new pif_req_info");
    return new pif_req_info(*this, port);
  }
  else {
    pif_req_info *p_pif_req_info = m_pif_req_pool.back();
    m_pif_req_pool.pop_back();
    p_pif_req_info->init(port);
    return p_pif_req_info;
  }
}



void xtsc_component::xtsc_memory_pin::delete_pif_req_info(pif_req_info*& p_pif_req_info) {
  m_pif_req_pool.push_back(p_pif_req_info);
  p_pif_req_info = 0;
}






void xtsc_component::xtsc_memory_pin::pif_req_info::init(u32 port) {
  m_port                = port;
  m_time_stamp          = sc_time_stamp();
  m_req_cntl            = m_memory.m_p_req_cntl[port]->read().to_uint();
  m_address             = m_memory.m_p_req_adrs[port]->read().to_uint();
  m_data                = m_memory.m_p_req_data[port]->read();
  m_byte_enables        = m_memory.m_p_req_data_be[port]->read().to_uint();
  m_id                  = m_memory.m_p_req_id[port] ? m_memory.m_p_req_id[port]->read() : 0;
  m_priority            = m_memory.m_p_req_priority[port]->read();
  m_route_id            = m_memory.m_p_req_route_id[port] ? m_memory.m_p_req_route_id[port]->read() : 0;
  m_fixed_byte_enables  = m_byte_enables;
  if (m_memory.m_big_endian) {
    m_memory.swizzle_byte_enables(m_fixed_byte_enables);
  }
}



void xtsc_component::xtsc_memory_pin::pif_req_info::dump(ostream& os) const {
  // Save state of stream
  char c = os.fill('0');
  ios::fmtflags old_flags = os.flags();

  os << m_req_cntl << " [0x" << hex << setw(8) << m_address << "/0x" << setw(m_memory.m_width8/4) << m_byte_enables << "]";

  u32 type = m_req_cntl.get_type();
  if (type == req_cntl::WRITE || type == req_cntl::BLOCK_WRITE || type == req_cntl::BURST_WRITE || type == req_cntl::RCW) {
    os << "=0x" << hex << m_data;
  }

  // Restore state of stream
  os.fill(c);
  os.flags(old_flags);
}



namespace xtsc_component {
ostream& operator<<(ostream& os, const xtsc_memory_pin::pif_req_info& info) {
  info.dump(os);
  return os;
}
}



void xtsc_component::xtsc_memory_pin::xtsc_debug_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  m_memory_pin.peek(address8, size8, buffer);
}



void xtsc_component::xtsc_memory_pin::xtsc_debug_if_impl::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  m_memory_pin.poke(address8, size8, buffer);
}



bool xtsc_component::xtsc_memory_pin::xtsc_debug_if_impl::nb_fast_access(xtsc_fast_access_request &request) {
  xtsc_address address8    = request.get_translated_request_address();
  xtsc_address page_start8 = address8 & ~(m_memory_pin.m_p_memory->m_page_size8 - 1);
  xtsc_address page_end8   = page_start8 + m_memory_pin.m_p_memory->m_page_size8 - 1;

  // Allow any fast access?
  if (!m_memory_pin.m_use_fast_access) {
    request.deny_access();
    XTSC_INFO(m_memory_pin.m_text, hex << setfill('0') << "nb_fast_access: address8=0x" << address8 <<
                               " page=[0x" << page_start8 << "-0x" << page_end8 << "] deny_access");
    return true;
  }
  
  XTSC_VERBOSE(m_memory_pin.m_text, "nb_fast_access: using peek/poke");
  request.allow_peek_poke_access();
  return true;
}



void xtsc_component::xtsc_memory_pin::xtsc_debug_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_memory_pin '" << m_memory_pin.name() << "' m_debug_exports[" << m_port_num
        << "]: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_memory_pin.m_text, "Binding '" << port.name() << "' to '" << m_memory_pin.name() << ".m_debug_exports[" << m_port_num <<
                                 "]");
  m_p_port = &port;
}




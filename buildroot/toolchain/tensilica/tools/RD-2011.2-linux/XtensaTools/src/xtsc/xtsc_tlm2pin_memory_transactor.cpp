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
#include <xtsc/xtsc_tlm2pin_memory_transactor.h>
#include <xtsc/xtsc_arbiter.h>
#include <xtsc/xtsc_cohctrl.h>
#include <xtsc/xtsc_master.h>
#include <xtsc/xtsc_memory_trace.h>
#include <xtsc/xtsc_router.h>
#if defined(_WIN32)
#else
#include <dlfcn.h>
#endif



/*
   Theory of operation
  
   This module can be used as a transactor for the PIF, inbound PIF, snoop, or for any
   of the local memory interfaces.  Which interface it is being used for determines which
   pin-level ports and what SystemC processes get instantiated.  The main difference is
   between the PIF and the local memory interfaces.
   
   Port Data-Member Naming Scheme:
   
   PIF/Inbound PIF/snoop        Local Memory Interface
   ---------------------        -------------------------------------------
   m_p_req*                     m_p_*  (* does not start with req/resp)
   m_p_resp*                    
    

   SystemC Processes:

   PIF/Inbound PIF/snoop        Local Memory Interface
   ---------------------        -------------------------------------------
   pif_request_thread           lcl_request_thread
   pif_response_thread          lcl_busy_write_rsp_thread
   pif_drive_resp_rdy_thread    lcl_sample_read_data_thread
                                xlmi_retire_thread              (XLMI only)
                                xlmi_flush_thread               (XLMI only)
                                xlmi_load_thread                (XLMI only)
                                xlmi_load_method                (XLMI only)
                                dram_lock_thread                (DRAM only)
 
   For local memory interface pins that share a common function, the same data member is
   used, but it is given a SystemC name that exactly matches the Xtensa RTL.  For example,
   the m_p_en data member is given the SystemC name of "DPort0En0" when it is used for
   XLMI0 load/store unit 0 and it is given the SystemC name of "DRam0En0" when it is used
   for DRAM0 load/store unit 0.  An example data member for a PIF pin is m_p_req_adrs which
   is given the SystemC name of "POReqAdrs".  The comments in this file and the corresponding
   header file generally omit the digits in the signal names (so "DRam0En0", "DRam0En1",
   "DRam1En0", and "DRam1En1" are all referred to as DRamEn)
  
   Multi-ported memories are supported for all memory interfaces.  This is accomplished
   by having each port data member be an array with one entry per memory port.  In
   addition, one instance of each applicable SystemC thread process is created per
   memory port.  Each thread process knows which memory port it is associated with by
   the local port variable which is created on the stack when the thread process is
   first entered.  This technique does not work for SystemC method processes because
   they always run to completion, so there is only one instance of the xlmi_load_method
   spawned (XLMI with busy only) and it is made sensitive to the necessary events of all
   the memory ports.

   The "sample_phase" and "output_delay" parameters play key roles in the timing of
   the model.  See their discussion in the xtsc_tlm2pin_memory_transactor_parms
   documentation.

   The request-response life cycle is like this:

   To generated a request, the upstream TLM master calls nb_request() in this module.
   The nb_request() implementation in this module makes a copy of the request, adds the
   copy to m_request_fifo, and notifies the appropriate thread (pif_request_thread or
   lcl_request_thread) by notifying m_request_event.  From this point on, the behavior
   is different for the PIF then for the local memory interfaces:


   PIF, Inbound PIF, snoop:

   When pif_request_thread wakes up because m_request_event has been notified, it
   translates the TLM request to a pin-level request and drives the pin-level signals at
   that time (which is delayed from the TLM nb_request call by m_output_delay).  The
   thread then syncs to the sample phase (as specified by the "sample_phase" parameter)
   and samples the "PIReqRdy" input to determine if the request was accepted by the
   downstream pin-level slave.
   
   If the request was not accepted then an RSP_NACC is sent to the upstream TLM master
   (this occurs at the "sample_phase" clock phase).  If the request is accepted and it
   is a write request with its last-transfer flag set and if this module is responsible
   for generating write responses (the "write_responses" parameter is true) then a TLM
   write response of RSP_OK is added to m_write_response_deque and pif_response_thread
   is notify via m_write_response_event.  If necessary, a second wait is performed at
   this point so that the output signals will be driven for one full clock period.
   After being driven for one full clock period, the output signals are deasserted
   (driven to 0).

   When a TLM request is first received a TLM response is pre-formed and stored in
   m_response_tab.  If the pin-level request gets rejected or if it is not a
   last-transfer, then the pre-formed response gets removed from m_response_tab and
   discarded.  Otherwise, it stays in m_response_tab until a non-RSP_NACC, last-transfer
   response is received.  When the non-RSP_NACC pin-level response is received it is
   matched up with the pre-formed TLM response in m_response_tab.  When a last-transfer
   TLM response is accepted by the upstream TLM master, the response is removed from
   m_response_tab.

   Each clock period that there is something to do, the pif_response_thread wakes up at
   the "sample_phase" phase and does the following (in order):
   1. Checks to see if a new response is available ("PIRespValid" and "PORespRdy" are
      both high) and, if it is, captures it and stores it in m_pif_response_deque.
   2. If the previous response was accepted and if it was a last transfer and if there
      are locally generated write responses in m_write_response_deque (see the
      "write_responses" parameter), then they are moved to the front of
      m_pif_response_deque.
   3. If there is a response in m_pif_response_deque, it is sent to the upstream TLM
      master.  If the upstream TLM master, rejects the response, then "PORespRdy" is
      driven low for one clock period so that no new response will be accepted by this
      transactor (this is handled in pif_drive_resp_rdy_thread which is notified by
      m_drive_resp_rdy_event).  The original response will be sent to the upstream TLM
      master once per clock period until it is accepted (it is too late for this module
      to reject the response at the pin-level interface).


   Local Memory Interface:

   When lcl_request_thread() wakes up (which is delayed from the TLM nb_request call by
   m_output_delay, see the "output_delay" parameter) because m_request_event has been
   notified, it pre-forms a TLM response and then translates the TLM request to a
   pin-level request and drives the pin-level signals.  It then schedules various events
   as appropriate, waits one clock period, and then deasserts the outputs by driving
   them with all 0's.  The events that may get scheduled are:

   m_load_event_queue:  (XLMI read only)
   This event queue is notified twice: first with a delay of m_clock_period and second 
   with a delay of 2*m_clock_period.  The first notification is to assert DPortLoad and
   it is notified with a delay of m_clock_period so that DPortLoad is asserted one clock
   cycle later then the other signals.  The second notification is to deassert
   DPortLoad.  The assert/deassert values are stored in m_load_deque.  The
   xlmi_load_thread waits for this event queue.

   m_busy_write_rsp_event_queue:
   This event is notified if the interface has a busy signal or if the request is a
   write.  The event is notified with a delay calculated to aligned with the sample
   phase.  The lcl_busy_write_rsp_thread waits for this event and then executes the
   following algorithm at the sample phas:
     - If interface has busy:
         - If busy is asserted:
             - Send RSP_NACC TLM response
         - If busy is not asserted and request is a read:
             - Schedule read data to be sample now (5-stage) or after 1 clock period
               (7-stage).  This scheduling is done by m_read_data_event_queue.notify().
         - If busy is not asserted and request is a write:
             - Send RSP_OK TLM write response
     - If interface has no busy (then this must be a write):
         - Send RSP_OK TLM write response

   m_read_data_event_queue:
   This event only applies to read requests.  This event is notified by lcl_request_thread
   if the interface has no busy signal and it is notified by lcl_busy_write_rsp_thread if
   the interface has a busy signal and busy is not asserted.  The event is notified with a
   delay calculated to aligned with the sample phase of the appropriate cycle for sampling
   the read data.  The lcl_sample_read_data_thread handles this event.

   When the lcl_sample_read_data_thread() thread wakes up at the sample phase, it gets
   the pre-formed response from m_read_data_rsp_deque, copies the read data from the
   pin-level read data bus into the TLM response buffer, and sends the TLM response to
   the upstream TLM master.

   The xlmi_retire_thread drives m_p_retire (DPortLoadRetired) based on m_retire_event
   and m_retire_deassert which are controlled by nb_load_retired().

   The xlmi_flush_thread drives m_p_flush (DPortRetireFlush) based on m_flush_event and
   m_flush_deassert which are controlled by nb_retire_flush().

   The xlmi_load_thread drives m_p_load (DPortLoad) for an XLMI without a busy and it
   drives m_p_preload for an XLMI with a busy.  It is controlled by m_load_event_queue
   which gets notified by lcl_request_thread.

   The xlmi_load_method drives m_p_load (DPortLoad) for an XLMI with a busy.  Its job is
   to qualify the load signal (m_p_preload) with the busy signal (m_p_busy/DPortBusy).

   The dram_lock_thread drives m_p_lock (DRamLock) based upon m_lock_event_queue and
   m_lock_deque which are controlled by nb_lock().

 */



using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace log4xtensa;
using namespace xtsc;



typedef xtsc::xtsc_request::type_t type_t;



#if defined(_WIN32)
static const char *win_error() {
  static char buffer[256];
  DWORD error = GetLastError();
  if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, sizeof(buffer), NULL)) {
    ostringstream oss;
    oss << " GetLastError()=" << error;
    strcpy(buffer, oss.str().c_str());
  }
  return buffer;
}
#endif



// ctor helper
static u32 get_num_ports(const xtsc_component::xtsc_tlm2pin_memory_transactor_parms& tlm2pin_parms) {
  return tlm2pin_parms.get_non_zero_u32("num_ports");
}



// ctor helper
static sc_trace_file *get_trace_file(const xtsc_component::xtsc_tlm2pin_memory_transactor_parms& tlm2pin_parms) {
  return static_cast<sc_trace_file*>(const_cast<void*>(tlm2pin_parms.get_void_pointer("vcd_handle")));
}



// ctor helper
static const char *get_suffix(const xtsc_component::xtsc_tlm2pin_memory_transactor_parms& tlm2pin_parms) {
  const char *port_name_suffix = tlm2pin_parms.get_c_str("port_name_suffix");
  static const char *blank = "";
  return port_name_suffix ? port_name_suffix : blank;
}



xtsc_component::xtsc_tlm2pin_memory_transactor_parms::xtsc_tlm2pin_memory_transactor_parms(const xtsc_core&     core,
                                                                                           const char          *memory_name,
                                                                                           u32                  num_ports)
{
  xtsc_core::memory_port mem_port = xtsc_core::get_memory_port(memory_name);
  if (!core.has_memory_port(mem_port)) {
    ostringstream oss;
    oss << "xtsc_tlm2pin_memory_transactor_parms: xtsc_core '" << core.name() << "' doesn't have a \"" << memory_name
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
  u32           address_bits    = 32;   // PIF|XLMI0

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
  set("read_delay", core_parms.get_u32("LocalMemoryLatency") - 1);

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



xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_tlm2pin_memory_transactor(
    sc_module_name                              module_name,
    const xtsc_tlm2pin_memory_transactor_parms& tlm2pin_parms
  ) :
  sc_module             (module_name),
  xtsc_module_pin_base  (*this, ::get_num_ports(tlm2pin_parms), get_trace_file(tlm2pin_parms), get_suffix(tlm2pin_parms)),
  m_num_ports           (tlm2pin_parms.get_u32("num_ports")),
  m_request_fifo_depth  (tlm2pin_parms.get_non_zero_u32("request_fifo_depth")),
  m_interface_uc        (get_interface_uc(tlm2pin_parms.get_c_str("memory_interface"))),
  m_interface_type      (xtsc_module_pin_base::get_interface_type(m_interface_uc)),
  m_width8              (tlm2pin_parms.get_non_zero_u32("byte_width")),
  m_start_byte_address  (tlm2pin_parms.get_u32("start_byte_address")),
  m_read_delay          (tlm2pin_parms.get_u32("read_delay")),
  m_inbound_pif         (false),
  m_snoop               (false),
  m_has_coherence       (false),
  m_has_pif_attribute   (false),
  m_address_bits        ((m_interface_type == PIF) ? 32 : tlm2pin_parms.get_non_zero_u32("address_bits")),
  m_check_bits          (tlm2pin_parms.get_u32("check_bits")),
  m_route_id_bits       (tlm2pin_parms.get_u32("route_id_bits")),
  m_address             (m_address_bits),
  m_vadrs               (6),
  m_req_coh_cntl        (2),
  m_lane                (get_number_of_enables(m_interface_type, m_width8)),
  m_id                  (m_id_bits),
  m_priority            (2),
  m_route_id            (m_route_id_bits ? m_route_id_bits : 1),
  m_data                ((int)m_width8*8),
  m_req_cntl            (0),
  m_text                (TextLogger::getInstance(name()))
{

  if (m_interface_type == PIF) {
    m_inbound_pif       = tlm2pin_parms.get_bool("inbound_pif");
    m_snoop             = tlm2pin_parms.get_bool("snoop");
    if (m_inbound_pif && m_snoop) {
      ostringstream oss;
      oss << kind() << " '" << name() << "':  You cannot have both \"inbound_pif\" and \"snoop\" set to true";
      throw xtsc_exception(oss.str());
    }
    if (!m_inbound_pif && !m_snoop) {
      m_has_coherence = tlm2pin_parms.get_bool("has_coherence");
    }
    if (!m_snoop) {
      m_has_pif_attribute = tlm2pin_parms.get_bool("has_pif_attribute");
    }
  }
  m_big_endian          = tlm2pin_parms.get_bool("big_endian");
  m_has_request_id      = tlm2pin_parms.get_bool("has_request_id");
  m_write_responses     = tlm2pin_parms.get_bool("write_responses");
  m_has_busy            = tlm2pin_parms.get_bool("has_busy");
  m_has_lock            = tlm2pin_parms.get_bool("has_lock");
  m_has_xfer_en         = tlm2pin_parms.get_bool("has_xfer_en");
  m_bus_addr_bits_mask  = ((m_width8 ==  4) ? 0x03 :
                           (m_width8 ==  8) ? 0x07 : 
                           (m_width8 == 16) ? 0x0F :
                           (m_width8 == 32) ? 0x1F :
                                              0x3F);
  m_cbox                = tlm2pin_parms.get_bool("cbox");
  m_cosim               = tlm2pin_parms.get_bool("cosim");
  m_shadow_memory       = tlm2pin_parms.get_bool("shadow_memory");
  m_test_shadow_memory  = tlm2pin_parms.get_bool("test_shadow_memory");
  m_dso_name            = tlm2pin_parms.get_c_str("dso_name") ? tlm2pin_parms.get_c_str("dso_name") : "";
  m_dso_cookie          = xtsc_copy_c_str(tlm2pin_parms.get_c_str("dso_cookie"));
  m_initial_value_file  = tlm2pin_parms.get_c_str("initial_value_file") ? tlm2pin_parms.get_c_str("initial_value_file") : "";
  m_memory_fill_byte    = (u8) tlm2pin_parms.get_u32("memory_fill_byte");
  m_p_memory            = NULL;
  m_dso                 = NULL;
  m_peek                = NULL;
  m_poke                = NULL;

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
    m_address_shift     = 0;
    m_address_mask      = 0xFFFFFFFF;
  }
  else if (m_interface_type == XLMI0) {
    m_address_shift     = 0;
    m_address_mask      = (m_address_bits >= 32) ? 0xFFFFFFFF : (1 << m_address_bits) - 1;
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
  u32 clock_period = tlm2pin_parms.get_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = m_time_resolution * clock_period;
  }
  m_clock_period_value = m_clock_period.value();
  u32 posedge_offset = tlm2pin_parms.get_u32("posedge_offset");
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

  m_read_delay_time = m_clock_period * m_read_delay;

  // Get sample phase
  u32 sample_phase = tlm2pin_parms.get_u32("sample_phase");
  if (sample_phase == 0xFFFFFFFF) {
    if (m_interface_type == PIF) {
      m_sample_phase = 0 * m_time_resolution;
    }
    else {
      u32 pdf_a, pdf_b, pdf_c;
      xtsc_core::get_clock_phase_delta_factors(pdf_a, pdf_b, pdf_c);
      if (pdf_a == 0) {
        pdf_a = xtsc_get_system_clock_factor();
      }
      m_sample_phase = (pdf_a - 1) * m_time_resolution;
    }
  }
  else {
    m_sample_phase = sample_phase * m_time_resolution;
  }
  m_sample_phase_plus_one = m_sample_phase + m_clock_period;
  if (m_sample_phase >= m_clock_period) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': Sample phase of " << m_sample_phase << " (from \"sample_phase\"= " << sample_phase
        << ") must be less than clock period of " << m_clock_period;
    throw xtsc_exception(oss.str());
  }


  m_output_delay = tlm2pin_parms.get_u32("output_delay") * m_time_resolution;

  // Create all the "per mem port" stuff

  m_request_exports             = new sc_export<xtsc_request_if>*       [m_num_ports];
  m_request_impl                = new xtsc_request_if_impl*             [m_num_ports];
  m_debug_cap                   = new xtsc_debug_if_cap*                [m_num_ports];
  m_request_fifo                = new deque<xtsc_request*>              [m_num_ports];
  m_respond_ports               = new sc_port<xtsc_respond_if>*         [m_num_ports];
  m_debug_ports                 = new sc_port<xtsc_debug_if, NSPP>*     [m_num_ports];
  m_response_tab                = new response_info*                    [m_num_ports][m_num_ids];
  m_request_event               = new sc_event                          [m_num_ports];


  m_current_id                  = 0;
  m_write_response_event        = 0;
  m_drive_resp_rdy_event        = 0;
  m_resp_rdy_fifo               = 0;
  m_pif_response_deque          = 0;
  m_write_response_deque        = 0;
  m_previous_response_last      = 0;
  m_busy_write_rsp_deque        = 0;
  m_busy_write_rsp_event_queue  = 0;
  m_read_data_rsp_deque         = 0;
  m_read_data_event_queue       = 0;
  m_retire_event                = 0;
  m_flush_event                 = 0;
  m_retire_deassert             = 0;
  m_flush_deassert              = 0;
  m_lock_event_queue            = 0;
  m_load_event_queue            = 0;
  m_lock_deque                  = 0;
  m_load_deque                  = 0;

  if (m_interface_type == PIF) {
    m_current_id                = new u32                               [m_num_ports];
    m_write_response_event      = new sc_event                          [m_num_ports];
    m_drive_resp_rdy_event      = new sc_event                          [m_num_ports];
    m_resp_rdy_fifo             = new bool_fifo*                        [m_num_ports];
    m_pif_response_deque        = new deque<response_info*>             [m_num_ports];
    m_write_response_deque      = new deque<response_info*>             [m_num_ports];
    m_previous_response_last    = new bool                              [m_num_ports];
  }
  else {
    m_busy_write_rsp_deque      = new deque<response_info*>             [m_num_ports];
    m_busy_write_rsp_event_queue= new sc_event_queue                    [m_num_ports];
    m_read_data_rsp_deque       = new deque<response_info*>             [m_num_ports];
    m_read_data_event_queue     = new sc_event_queue                    [m_num_ports];
  }

  if (m_interface_type == XLMI0) {
    m_retire_event              = new sc_event                          [m_num_ports];
    m_flush_event               = new sc_event                          [m_num_ports];
    m_retire_deassert           = new sc_time                           [m_num_ports];
    m_flush_deassert            = new sc_time                           [m_num_ports];
    m_load_event_queue          = new sc_event_queue                    [m_num_ports];
    m_load_deque                = new deque<bool>                       [m_num_ports];
  }

  if (((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) && m_has_lock) {
    m_lock_event_queue          = new sc_event_queue                    [m_num_ports];
    m_lock_deque                = new deque<bool>                       [m_num_ports];
  }

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
  m_p_preload           = NULL;
  m_p_retire            = NULL;
  m_p_flush             = NULL;
  m_p_lock              = NULL;
  m_p_check_wr          = NULL;
  m_p_check             = NULL;
  m_p_xfer_en           = NULL;
  m_p_busy              = NULL;
  m_p_data              = NULL;

  if (m_interface_type == PIF) {
    m_p_req_valid               = new bool_output*                      [m_num_ports];
    m_p_req_cntl                = new uint_output*                      [m_num_ports];
    m_p_req_adrs                = new uint_output*                      [m_num_ports];
    m_p_req_data                = new wide_output*                      [m_num_ports];
    m_p_req_data_be             = new uint_output*                      [m_num_ports];
    m_p_req_id                  = new uint_output*                      [m_num_ports];
    m_p_req_priority            = new uint_output*                      [m_num_ports];
    m_p_req_route_id            = new uint_output*                      [m_num_ports];
    m_p_req_attribute           = new uint_output*                      [m_num_ports];
    m_p_req_coh_vadrs           = new uint_output*                      [m_num_ports];
    m_p_req_coh_cntl            = new uint_output*                      [m_num_ports];
    m_p_req_rdy                 = new bool_input*                       [m_num_ports];
    m_p_resp_valid              = new bool_input*                       [m_num_ports];
    m_p_resp_cntl               = new uint_input*                       [m_num_ports];
    m_p_resp_data               = new wide_input*                       [m_num_ports];
    m_p_resp_id                 = new uint_input*                       [m_num_ports];
    m_p_resp_priority           = new uint_input*                       [m_num_ports];
    m_p_resp_route_id           = new uint_input*                       [m_num_ports];
    m_p_resp_coh_cntl           = new uint_input*                       [m_num_ports];
    m_p_resp_rdy                = new bool_output*                      [m_num_ports];
  }
  else {
    m_p_en                      = new bool_output*                      [m_num_ports];
    m_p_addr                    = new uint_output*                      [m_num_ports];
    m_p_lane                    = new uint_output*                      [m_num_ports];
    m_p_wrdata                  = new wide_output*                      [m_num_ports];
    m_p_wr                      = new bool_output*                      [m_num_ports];
    m_p_load                    = new bool_output*                      [m_num_ports];
    m_p_preload                 = new bool_signal*                      [m_num_ports];
    m_p_retire                  = new bool_output*                      [m_num_ports];
    m_p_flush                   = new bool_output*                      [m_num_ports];
    m_p_lock                    = new bool_output*                      [m_num_ports];
    m_p_check_wr                = new wide_output*                      [m_num_ports];
    m_p_check                   = new wide_input*                       [m_num_ports];
    m_p_xfer_en                 = new bool_output*                      [m_num_ports];
    m_p_busy                    = new bool_input*                       [m_num_ports];
    m_p_data                    = new wide_input*                       [m_num_ports];
  }


  for (u32 port=0; port<m_num_ports; ++port) {

    ostringstream oss1;
    oss1 << "m_request_exports[" << port << "]";
    m_request_exports[port]     = new sc_export<xtsc_request_if>(oss1.str().c_str());

    ostringstream oss2;
    oss2 << "m_request_impl[" << port << "]";
    m_request_impl[port]        = new xtsc_request_if_impl(oss2.str().c_str(), *this, port);

    (*m_request_exports[port])(*m_request_impl[port]);

    ostringstream oss3;
    oss3 << "m_respond_ports[" << port << "]";
    m_respond_ports[port]       = new sc_port<xtsc_respond_if>(oss3.str().c_str());

    ostringstream oss4;
    oss4 << "m_debug_ports[" << port << "]";
    m_debug_ports[port]         = new sc_port<xtsc_debug_if, NSPP>(oss4.str().c_str());

    if (m_interface_type == PIF) {
      ostringstream oss2;
      oss2 << "m_resp_rdy_fifo[" << port << "]";
      m_resp_rdy_fifo[port]      = new bool_fifo(oss2.str().c_str());
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
      m_p_preload           [port] = NULL;
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
        m_p_addr            [port] = &add_uint_output("DRam0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_output("DRam0ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("DRam0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("DRam0En", m_append_id, port);
        m_p_wr              [port] = &add_bool_output("DRam0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_output("DRam0WrData", m_width8*8, m_append_id, port);
        if (m_has_lock) {
        m_p_lock            [port] = &add_bool_output("DRam0Lock", m_append_id, port);
        }
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_output("XferDRam0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("DRam0Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_output("DRam0CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_input ("DRam0CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case DRAM1: {
        m_p_addr            [port] = &add_uint_output("DRam1Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_output("DRam1ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("DRam1Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("DRam1En", m_append_id, port);
        m_p_wr              [port] = &add_bool_output("DRam1Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_output("DRam1WrData", m_width8*8, m_append_id, port);
        if (m_has_lock) {
        m_p_lock            [port] = &add_bool_output("DRam1Lock", m_append_id, port);
        }
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_output("XferDRam1En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("DRam1Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_output("DRam1CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_input ("DRam1CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case DROM0: {
        m_p_addr            [port] = &add_uint_output("DRom0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_output("DRom0ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("DRom0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("DRom0En", m_append_id, port);
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("DRom0Busy", m_append_id, port);
        }
        break;
      }
      case IRAM0: {
        m_p_addr            [port] = &add_uint_output("IRam0Addr", m_address_bits, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("IRam0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("IRam0En", m_append_id, port);
        if (m_width8 >= 8) {
          m_p_lane          [port] = &add_uint_output("IRam0WordEn", (m_width8/4), m_append_id, port);
        }
        m_p_load            [port] = &add_bool_output("IRam0LoadStore", m_append_id, port);
        m_p_wr              [port] = &add_bool_output("IRam0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_output("IRam0WrData", m_width8*8, m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_output("XferIRam0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("IRam0Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_output("IRam0CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_input ("IRam0CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case IRAM1: {
        m_p_addr            [port] = &add_uint_output("IRam1Addr", m_address_bits, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("IRam1Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("IRam1En", m_append_id, port);
        if (m_width8 >= 8) {
          m_p_lane          [port] = &add_uint_output("IRam1WordEn", (m_width8/4), m_append_id, port);
        }
        m_p_load            [port] = &add_bool_output("IRam1LoadStore", m_append_id, port);
        m_p_wr              [port] = &add_bool_output("IRam1Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_output("IRam1WrData", m_width8*8, m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_output("XferIRam1En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("IRam1Busy", m_append_id, port);
        }
        if (m_check_bits) {
        m_p_check_wr        [port] = &add_wide_output("IRam1CheckWrData", m_check_bits, m_append_id, port);
        m_p_check           [port] = &add_wide_input ("IRam1CheckData",   m_check_bits, m_append_id, port);
        }
        break;
      }
      case IROM0: {
        m_p_addr            [port] = &add_uint_output("IRom0Addr", m_address_bits, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("IRom0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("IRom0En", m_append_id, port);
        if (m_width8 >= 8) {
          m_p_lane          [port] = &add_uint_output("IRom0WordEn", (m_width8/4), m_append_id, port);
        }
        m_p_load            [port] = &add_bool_output("IRom0Load", m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_output("XferIRom0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("IRom0Busy", m_append_id, port);
        }
        break;
      }
      case URAM0: {
        m_p_addr            [port] = &add_uint_output("URam0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_output("URam0ByteEn", m_width8, m_append_id, port);
        m_p_data            [port] = &add_wide_input ("URam0Data", m_width8*8, m_append_id, port);
        m_p_en              [port] = &add_bool_output("URam0En", m_append_id, port);
        m_p_load            [port] = &add_bool_output("URam0LoadStore", m_append_id, port);
        m_p_wr              [port] = &add_bool_output("URam0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_output("URam0WrData", m_width8*8, m_append_id, port);
        if (m_has_xfer_en) {
        m_p_xfer_en         [port] = &add_bool_output("XferURam0En", m_append_id, port);
        }
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("URam0Busy", m_append_id, port);
        }
        break;
      }
      case XLMI0: {
        m_p_en              [port] = &add_bool_output("DPort0En", m_append_id, port);
        m_p_addr            [port] = &add_uint_output("DPort0Addr", m_address_bits, m_append_id, port);
        m_p_lane            [port] = &add_uint_output("DPort0ByteEn", m_width8, m_append_id, port);
        m_p_wr              [port] = &add_bool_output("DPort0Wr", m_append_id, port);
        m_p_wrdata          [port] = &add_wide_output("DPort0WrData", m_width8*8, m_append_id, port);
        m_p_load            [port] = &add_bool_output("DPort0Load", m_append_id, port);
        m_p_data            [port] = &add_wide_input ("DPort0Data", m_width8*8, m_append_id, port);
        m_p_retire          [port] = &add_bool_output("DPort0LoadRetired", m_append_id, port);
        m_p_flush           [port] = &add_bool_output("DPort0RetireFlush", m_append_id, port);
        if (m_has_busy) {
          m_p_busy          [port] = &add_bool_input ("DPort0Busy", m_append_id, port);
          ostringstream preload;
          preload << "preload" << port;
          m_p_preload       [port] = new bool_signal(preload.str().c_str());
        }
        break;
      }
      case PIF: {
        if (m_inbound_pif) {
        m_p_req_valid       [port] = &add_bool_output("PIReqValid", m_append_id, port);
        m_p_req_cntl        [port] = &add_uint_output("PIReqCntl", 8, m_append_id, port);
        m_p_req_adrs        [port] = &add_uint_output("PIReqAdrs", m_address_bits, m_append_id, port);
        m_p_req_data        [port] = &add_wide_output("PIReqData", m_width8*8, m_append_id, port);
        m_p_req_data_be     [port] = &add_uint_output("PIReqDataBE", m_width8, m_append_id, port);
        m_p_req_priority    [port] = &add_uint_output("PIReqPriority", 2, m_append_id, port);
        m_p_req_rdy         [port] = &add_bool_input ("POReqRdy", m_append_id, port);
        m_p_resp_valid      [port] = &add_bool_input ("PORespValid", m_append_id, port);
        m_p_resp_cntl       [port] = &add_uint_input ("PORespCntl", 8, m_append_id, port);
        m_p_resp_data       [port] = &add_wide_input ("PORespData", m_width8*8, m_append_id, port);
        m_p_resp_priority   [port] = &add_uint_input ("PORespPriority", 2, m_append_id, port);
        m_p_resp_rdy        [port] = &add_bool_output("PIRespRdy", m_append_id, port);
        if (m_has_request_id) {
          m_p_req_id        [port] = &add_uint_output("PIReqId", m_id_bits, m_append_id, port);
          m_p_resp_id       [port] = &add_uint_input ("PORespId", m_id_bits, m_append_id, port);
        }
        if (m_route_id_bits) {
          m_p_req_route_id  [port] = &add_uint_output("PIReqRouteId", m_route_id_bits, m_append_id, port);
          m_p_resp_route_id [port] = &add_uint_input ("PORespRouteId", m_route_id_bits, m_append_id, port);
        }
        if (m_has_pif_attribute) {
          m_p_req_attribute [port] = &add_uint_output("PIReqAttribute", 12, m_append_id, port);
        }
        }
        else if (m_snoop) {
        m_p_req_valid       [port] = &add_bool_output("SnoopReqValid", m_append_id, port);
        m_p_req_cntl        [port] = &add_uint_output("SnoopReqCntl", 8, m_append_id, port);
        m_p_req_adrs        [port] = &add_uint_output("SnoopReqAdrs", m_address_bits, m_append_id, port);
        m_p_req_priority    [port] = &add_uint_output("SnoopReqPriority", 2, m_append_id, port);
        m_p_req_coh_vadrs   [port] = &add_uint_output("SnoopReqCohVAdrsIndex", 6, m_append_id, port);
        m_p_req_rdy         [port] = &add_bool_input ("SnoopReqRdy", m_append_id, port);
        m_p_resp_valid      [port] = &add_bool_input ("SnoopRespValid", m_append_id, port);
        m_p_resp_cntl       [port] = &add_uint_input ("SnoopRespCntl", 8, m_append_id, port);
        m_p_resp_data       [port] = &add_wide_input ("SnoopRespData", m_width8*8, m_append_id, port);
        m_p_resp_rdy        [port] = &add_bool_output("SnoopRespRdy", m_append_id, port);
        if (m_has_request_id) {
          m_p_req_id        [port] = &add_uint_output("SnoopReqId", m_id_bits, m_append_id, port);
          m_p_resp_id       [port] = &add_uint_input ("SnoopRespId", m_id_bits, m_append_id, port);
        }
        }
        else {
        m_p_req_valid       [port] = &add_bool_output("POReqValid", m_append_id, port);
        m_p_req_cntl        [port] = &add_uint_output("POReqCntl", 8, m_append_id, port);
        m_p_req_adrs        [port] = &add_uint_output("POReqAdrs", m_address_bits, m_append_id, port);
        m_p_req_data        [port] = &add_wide_output("POReqData", m_width8*8, m_append_id, port);
        m_p_req_data_be     [port] = &add_uint_output("POReqDataBE", m_width8, m_append_id, port);
        m_p_req_priority    [port] = &add_uint_output("POReqPriority", 2, m_append_id, port);
        m_p_req_rdy         [port] = &add_bool_input ("PIReqRdy", m_append_id, port);
        m_p_resp_valid      [port] = &add_bool_input ("PIRespValid", m_append_id, port);
        m_p_resp_cntl       [port] = &add_uint_input ("PIRespCntl", 8, m_append_id, port);
        m_p_resp_data       [port] = &add_wide_input ("PIRespData", m_width8*8, m_append_id, port);
        m_p_resp_priority   [port] = &add_uint_input ("PIRespPriority", 2, m_append_id, port);
        m_p_resp_rdy        [port] = &add_bool_output("PORespRdy", m_append_id, port);
        if (m_has_request_id) {
          m_p_req_id        [port] = &add_uint_output("POReqId", m_id_bits, m_append_id, port);
          m_p_resp_id       [port] = &add_uint_input ("PIRespId", m_id_bits, m_append_id, port);
        }
        if (m_route_id_bits) {
          m_p_req_route_id  [port] = &add_uint_output("POReqRouteId", m_route_id_bits, m_append_id, port);
          m_p_resp_route_id [port] = &add_uint_input ("PIRespRouteId", m_route_id_bits, m_append_id, port);
        }
        if (m_has_coherence) {
          m_p_req_coh_vadrs [port] = &add_uint_output("POReqCohVAdrsIndex", 6, m_append_id, port);
          m_p_req_coh_cntl  [port] = &add_uint_output("POReqCohCntl", 2, m_append_id, port);
          m_p_resp_coh_cntl [port] = &add_uint_input ("PIRespCohCntl", 2, m_append_id, port);
        }
        if (m_has_pif_attribute) {
          m_p_req_attribute [port] = &add_uint_output("POReqAttribute", 12, m_append_id, port);
        }
        }
        break;
      }
    }

    for (u32 i=0; i<m_num_ids; ++i) {
      m_response_tab[port][i] = 0;
    }

  }

  // Squelch SystemC's complaint about multiple thread "objects"
  sc_actions original_action = sc_report_handler::set_actions("object already exists", SC_WARNING, SC_DO_NOTHING);
  for (u32 port=0; port<m_num_ports; ++port) {
    // If this doesn't work for you, change "#if 1" to "#if 0"
    if (m_interface_type == PIF) {
      ostringstream oss1;
      oss1 << "pif_request_thread[" << port << "]";
      declare_thread_process(pif_request_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, pif_request_thread);
      ostringstream oss2;
      oss2 << "pif_response_thread[" << port << "]";
      declare_thread_process(pif_response_thread_handle, oss2.str().c_str(), SC_CURRENT_USER_MODULE, pif_response_thread);
      sensitive << m_p_resp_valid[port]->pos() << m_p_resp_rdy[port]->pos() << m_write_response_event[port];
      ostringstream oss3;
      oss3 << "pif_drive_resp_rdy_thread[" << port << "]";
      declare_thread_process(pif_drive_resp_rdy_thread_handle, oss3.str().c_str(), SC_CURRENT_USER_MODULE, pif_drive_resp_rdy_thread);
    }
    else {
      ostringstream oss1;
      oss1 << "lcl_request_thread[" << port << "]";
      declare_thread_process(lcl_request_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, lcl_request_thread);
      ostringstream oss2;
      oss2 << "lcl_busy_write_rsp_thread[" << port << "]";
      declare_thread_process(lcl_busy_write_rsp_thread_handle, oss2.str().c_str(),
                             SC_CURRENT_USER_MODULE, lcl_busy_write_rsp_thread);
      sensitive << m_busy_write_rsp_event_queue[port];
      ostringstream oss3;
      oss3 << "lcl_sample_read_data_thread[" << port << "]";
      declare_thread_process(lcl_sample_read_data_thread_handle, oss3.str().c_str(),
                             SC_CURRENT_USER_MODULE, lcl_sample_read_data_thread);
      sensitive << m_read_data_event_queue[port];
      if (m_interface_type == XLMI0) {
        ostringstream oss1;
        oss1 << "xlmi_retire_thread[" << port << "]";
        declare_thread_process(xlmi_retire_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, xlmi_retire_thread);
        ostringstream oss2;
        oss2 << "xlmi_flush_thread[" << port << "]";
        declare_thread_process(xlmi_flush_thread_handle, oss2.str().c_str(), SC_CURRENT_USER_MODULE, xlmi_flush_thread);
        ostringstream oss3;
        oss3 << "xlmi_load_thread[" << port << "]";
        declare_thread_process(xlmi_load_thread_handle, oss3.str().c_str(), SC_CURRENT_USER_MODULE, xlmi_load_thread);
        sensitive << m_load_event_queue[port];
      }
      if (((m_interface_type == DRAM0) || (m_interface_type == DRAM1)) && m_has_lock) {
        ostringstream oss1;
        oss1 << "dram_lock_thread[" << port << "]";
        declare_thread_process(dram_lock_thread_handle, oss1.str().c_str(), SC_CURRENT_USER_MODULE, dram_lock_thread);
        sensitive << m_lock_event_queue[port];
      }
    }
  }
  if ((m_interface_type == XLMI0) && m_has_busy) {
    SC_METHOD(xlmi_load_method);
    for (u32 port=0; port<m_num_ports; ++port) {
      sensitive << *m_p_preload[port] << *m_p_busy[port];
    }
  }
  // Restore SystemC
  sc_report_handler::set_actions("object already exists", SC_WARNING, original_action);


  // Log our construction
  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, "Constructed " << kind() << " '" << name() << "':");
  XTSC_LOG(m_text, ll, " memory_interface       = "                 << m_interface_uc);
  if (m_interface_type == PIF) {
  XTSC_LOG(m_text, ll, " inbound_pif            = "                 << boolalpha << m_inbound_pif);
  XTSC_LOG(m_text, ll, " snoop                  = "                 << boolalpha << m_snoop);
  if (!m_inbound_pif && !m_snoop) {
  XTSC_LOG(m_text, ll, " has_coherence          = "                 << boolalpha << m_has_coherence);
  }
  }
  XTSC_LOG(m_text, ll, " num_ports              = "   << dec        << m_num_ports);
  XTSC_LOG(m_text, ll, " port_name_suffix       = "                 << m_suffix);
  XTSC_LOG(m_text, ll, " byte_width             = "                 << m_width8);
  XTSC_LOG(m_text, ll, " start_byte_address     = 0x" << hex        << setfill('0') << setw(8) << m_start_byte_address);
  XTSC_LOG(m_text, ll, " big_endian             = "                 << boolalpha << m_big_endian);
  XTSC_LOG(m_text, ll, " vcd_handle             = "                 << m_p_trace_file);
  XTSC_LOG(m_text, ll, " request_fifo_depth     = "                 << m_request_fifo_depth);
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
  if (sample_phase == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, " sample_phase           = 0x" << hex        << sample_phase << " (" << m_sample_phase << ")");
  } else {
  XTSC_LOG(m_text, ll, " sample_phase           = "                 << sample_phase << " (" << m_sample_phase << ")");
  }
  XTSC_LOG(m_text, ll, " read_delay             = "                 << m_read_delay);
  if (m_interface_type == PIF) {
  XTSC_LOG(m_text, ll, " has_request_id         = "                 << boolalpha << m_has_request_id);
  XTSC_LOG(m_text, ll, " write_responses        = "                 << boolalpha << m_write_responses);
  XTSC_LOG(m_text, ll, " route_id_bits          = "   << dec        << m_route_id_bits);
  } else {
  XTSC_LOG(m_text, ll, " address_bits           = "   << dec        << m_address_bits);
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
  XTSC_LOG(m_text, ll, " dso_name               = "                 << m_dso_name);
  XTSC_LOG(m_text, ll, " dso_cookie             = "                 << (m_dso_cookie ? m_dso_cookie : ""));
  XTSC_LOG(m_text, ll, " cosim                  = "                 << boolalpha << m_cosim);
  XTSC_LOG(m_text, ll, " shadow_memory          = "                 << boolalpha << m_shadow_memory);
  XTSC_LOG(m_text, ll, " test_shadow_memory     = "                 << boolalpha << m_test_shadow_memory);
  if (m_test_shadow_memory) {
    XTSC_WARN(m_text, "The \"test_shadow_memory\" flag is set -- Operating in shadow memory test mode.");
  }
  XTSC_LOG(m_text, ll, " initial_value_file     = "                 << m_initial_value_file);
  XTSC_LOG(m_text, ll, " memory_fill_byte       = 0x" << hex        << (u32) m_memory_fill_byte);
  XTSC_LOG(m_text, ll, " m_address_mask         = 0x" << hex        << m_address_mask);
  XTSC_LOG(m_text, ll, " m_address_shift        = "   << dec        << m_address_shift);
  ostringstream oss;
  oss << "Port List:" << endl;
  dump_ports(oss);
  xtsc_log_multiline(m_text, ll, oss.str(), 2);

}



xtsc_component::xtsc_tlm2pin_memory_transactor::~xtsc_tlm2pin_memory_transactor(void) {
  // Do any required clean-up here
  XTSC_DEBUG(m_text, "m_request_pool.size()=" << m_request_pool.size());
  XTSC_DEBUG(m_text, "m_response_pool.size()=" << m_response_pool.size());
}



bool xtsc_component::xtsc_tlm2pin_memory_transactor::has_dso() const {
  return (m_peek != NULL);
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::connect(xtsc_arbiter& arbiter, u32 tran_port) {
  if (tran_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  arbiter.m_request_port(*m_request_exports[tran_port]);
  (*m_respond_ports[tran_port])(arbiter.m_respond_export);
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::connect(xtsc_cohctrl& cohctrl, u32 port) {
  if (m_snoop) {
    u32 num_ports = cohctrl.get_num_clients();
    if (port >= num_ports) {
      ostringstream oss;
      oss << "Invalid port=" << port << " in connect(): " << endl;
      oss << cohctrl.kind() << " '" << cohctrl.name() << "' has " << num_ports << " client ports numbered from 0 to " << num_ports-1
          << endl;
      throw xtsc_exception(oss.str());
    }
    (*cohctrl.m_snoop_ports[port])(*m_request_exports[0]);
    (*m_respond_ports[0])(*cohctrl.m_snoop_exports[port]);
  }
  else {
    if (port >= m_num_ports) {
      ostringstream oss;
      oss << "Invalid port=" << port << " in connect(): " << endl;
      oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
      throw xtsc_exception(oss.str());
    }
    cohctrl.m_request_port(*m_request_exports[port]);
    (*m_respond_ports[port])(cohctrl.m_respond_export);
  }
}



xtsc::u32 xtsc_component::xtsc_tlm2pin_memory_transactor::connect(xtsc_core&    core,
                                                                  const char   *memory_port_name,
                                                                  u32           tran_port,
                                                                  bool          single_connect)
{
  if (tran_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  xtsc_core::memory_port mem_port = xtsc_core::get_memory_port(memory_port_name);
  bool consistent = false;
  switch (mem_port) {
    case xtsc_core::MEM_DRAM0LS0:
    case xtsc_core::MEM_DRAM0LS1:  consistent = (m_interface_type == DRAM0); break;

    case xtsc_core::MEM_DRAM1LS0:
    case xtsc_core::MEM_DRAM1LS1:  consistent = (m_interface_type == DRAM1); break;

    case xtsc_core::MEM_DROM0LS0:
    case xtsc_core::MEM_DROM0LS1:  consistent = (m_interface_type == DROM0); break;

    case xtsc_core::MEM_IRAM0:     consistent = (m_interface_type == IRAM0); break;
    case xtsc_core::MEM_IRAM1:     consistent = (m_interface_type == IRAM1); break;

    case xtsc_core::MEM_IROM0:     consistent = (m_interface_type == IROM0); break;

    case xtsc_core::MEM_URAM0:     consistent = (m_interface_type == URAM0); break;

    case xtsc_core::MEM_XLMI0LS0:
    case xtsc_core::MEM_XLMI0LS1:  consistent = (m_interface_type == XLMI0); break;

    case xtsc_core::MEM_PIF:       consistent = (m_interface_type == PIF ); break;

    default: {
      ostringstream oss;
      oss << "Program bug (unsupported xtsc_core::mem_port) in line " << __LINE__ << " of " << __FILE__;
      throw xtsc_exception(oss.str());
    }
  }
  if (!consistent) {
    ostringstream oss;
    oss << "Cannot connect " << kind() << " '" << name() << "' with interface type of \"" << m_interface_uc
        << "\" to memory port \"" << memory_port_name << "\" of xtsc_core '" << core.name() << "'";
    throw xtsc_exception(oss.str());
  }
  u32 width8 = core.get_memory_byte_width(xtsc_core::get_memory_port(memory_port_name));
  if (m_width8 && (width8 != m_width8)) {
    ostringstream oss;
    oss << "Memory interface data width mismatch: " << endl;
    oss << kind() << " '" << name() << "' is " << m_width8 << " bytes wide, but '" << memory_port_name << "' interface of" << endl;
    oss << "xtsc_core '" << core.name() << "' is " << width8 << " bytes wide.";
    throw xtsc_exception(oss.str());
  }
  core.get_request_port(memory_port_name)(*m_request_exports[tran_port]);
  (*m_respond_ports[tran_port])(core.get_respond_export(memory_port_name));
  u32 num_connected = 1;
  // Should we connect a 2nd port?
  if (!single_connect && (tran_port+1 < m_num_ports) && core.is_dual_ported(xtsc_core::is_xlmi(mem_port))) {
    xtsc_core::memory_port memory_port = xtsc_core::get_memory_port(memory_port_name);
    if (xtsc_core::is_ls_dual_port(memory_port, 0)) {
      // Don't connect if 2nd port of xtsc_tlm2pin_memory_transactor has already been connected.
      if (!m_request_impl[tran_port+1]->is_connected()) {
        const char *dual_name = xtsc_core::get_memory_port_name(memory_port+1);
        sc_port<xtsc_request_if, NSPP>& dual_port = core.get_request_port(dual_name);
        // Don't connect if 2nd port of xtsc_core has already been connected.
        // This test is not reliable so we'll catch any errors and carry on (which
        // works for the OSCI simulator--but maybe not for others).
        if (!dual_port.get_interface()) {
          try {
            dual_port(*m_request_exports[tran_port+1]);
            (*m_respond_ports[tran_port+1])(core.get_respond_export(dual_name));
            num_connected += 1;
          } catch (...) {
            XTSC_INFO(m_text, "Core '" << core.name() << "' 2nd LD/ST memory port '" << dual_name << "' is already bound.");
          }
        }
      }
    }
  }
  return num_connected;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::connect(xtsc_master& master, u32 tran_port) {
  if (tran_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  master.m_request_port(*m_request_exports[tran_port]);
  (*m_respond_ports[tran_port])(master.m_respond_export);
}



u32 xtsc_component::xtsc_tlm2pin_memory_transactor::connect(xtsc_memory_trace&  memory_trace,
                                                            u32                 trace_port,
                                                            u32                 tran_port,
                                                            bool                single_connect)
{
  u32 trace_ports = memory_trace.get_num_ports();
  if (trace_port >= trace_ports) {
    ostringstream oss;
    oss << "Invalid trace_port=" << trace_port << " in connect(): " << endl;
    oss << memory_trace.kind() << " '" << memory_trace.name() << "' has " << trace_ports << " ports numbered from 0 to "
        << trace_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (tran_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }

  u32 num_connected = 0;

  while ((trace_port < trace_ports) && (tran_port < m_num_ports)) {

    (*memory_trace.m_request_ports[trace_port])(*m_request_exports[tran_port]);
    (*m_respond_ports[tran_port])(*memory_trace.m_respond_exports[trace_port]);

    tran_port += 1;
    trace_port += 1;
    num_connected += 1;

    if (single_connect) break;
    if (trace_port >= trace_ports) break;
    if (tran_port >= m_num_ports) break;
  }

  return num_connected;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::connect(xtsc_router& router, u32 router_port, u32 tran_port) {
  if (tran_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  u32 num_slaves = router.get_num_slaves();
  if (router_port >= num_slaves) {
    ostringstream oss;
    oss << "Invalid router_port=" << router_port << " in xtsc_tlm2pin_memory_transactor::connect(): " << endl;
    oss << router.kind() << " '" << router.name() << "' has " << num_slaves << " ports numbered from 0 to " << num_slaves-1 << endl;
    throw xtsc_exception(oss.str());
  }
  (*router.m_request_ports[router_port])(*m_request_exports[tran_port]);
  (*m_respond_ports[tran_port])(*router.m_respond_exports[router_port]);
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::before_end_of_elaboration(void) {
  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();

  if (m_dso_name != "") {
#if defined(_WIN32)
    m_dso = LoadLibrary(m_dso_name.c_str());
    if (!m_dso) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': LoadLibrary() call failed for \"dso_name\" = " << m_dso_name << ": " << win_error();
      throw xtsc_exception(oss.str());
    }

    m_peek = (peek_t) GetProcAddress(m_dso, "peek");
    if (!m_peek) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': GetProcAddress() call failed for symbol \"peek\" in DSO " << m_dso_name << ": "
          << win_error();
      throw xtsc_exception(oss.str());
    }

    m_poke = (poke_t) GetProcAddress(m_dso, "poke");
    if (!m_poke) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': GetProcAddress() call failed for symbol \"poke\" in DSO " << m_dso_name << ": "
          << win_error();
      throw xtsc_exception(oss.str());
    }
#else
    m_dso = dlopen(m_dso_name.c_str(), RTLD_LAZY);
    if (!m_dso) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': dlopen() call failed for \"dso_name\" = " << m_dso_name << ": " << dlerror();
      throw xtsc_exception(oss.str());
    }

    m_peek = (peek_t) dlsym(m_dso, "peek");
    if (!m_peek) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': dlsym() call failed for symbol \"peek\" in DSO " << m_dso_name << ": " << dlerror();
      throw xtsc_exception(oss.str());
    }

    m_poke = (poke_t) dlsym(m_dso, "poke");
    if (!m_poke) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': dlsym() call failed for symbol \"poke\" in DSO " << m_dso_name << ": " << dlerror();
      throw xtsc_exception(oss.str());
    }
#endif
  }
  bool found_one_unconnected = false;
  for (u32 port=0; port<m_num_ports; ++port) {
    if (m_debug_ports[port]->get_interface() == NULL) {
      ostringstream oss;
      oss << ( (m_dso_name != "") ? "m_debug_dso" : "m_debug_cap") << "[" << port << "]";
      m_debug_cap[port] = new xtsc_debug_if_cap(*this, port);
      xtsc_trap_port_binding_failures(true);
      try {
        (*m_debug_ports[port])(*m_debug_cap[port]);
        XTSC_LOG(m_text, ll, "Connected port \"" << m_debug_ports[port]->name() << "\") to " << oss.str());
        found_one_unconnected = true;
      }
      catch (...) {
        XTSC_LOG(m_text, ll, "Detected delay-bound port " << m_debug_ports[port]->name());
      }
      xtsc_trap_port_binding_failures(false);
    }
  }
  if (m_test_shadow_memory || (found_one_unconnected && m_cosim && m_shadow_memory)) {
    XTSC_LOG(m_text, ll, "Creating shadow memory");
    m_p_memory = new xtsc_memory_base(name(), kind(), m_width8, m_start_byte_address, 0, 1024*16, m_initial_value_file,
                                      m_memory_fill_byte);
  }

}



void xtsc_component::xtsc_tlm2pin_memory_transactor::end_of_elaboration(void) {
} 



void xtsc_component::xtsc_tlm2pin_memory_transactor::start_of_simulation(void) {
  reset();
} 



void xtsc_component::xtsc_tlm2pin_memory_transactor::reset(bool /* hard_reset */) {
  XTSC_INFO(m_text, kind() << "::reset()");

  m_next_port_pif_request_thread                        = 0;
  m_next_port_pif_response_thread                       = 0;
  m_next_port_pif_drive_resp_rdy_thread                 = 0;
  m_next_port_lcl_request_thread                        = 0;
  m_next_port_xlmi_retire_thread                        = 0;
  m_next_port_xlmi_flush_thread                         = 0;
  m_next_port_xlmi_load_thread                          = 0;
  m_next_port_dram_lock_thread                          = 0;
  m_next_port_lcl_busy_write_rsp_thread                 = 0;
  m_next_port_lcl_sample_read_data_thread               = 0;

  for (u32 port=0; port<m_num_ports; ++port) {

    for (u32 i=0; i<m_num_ids; ++i) {
      if (m_response_tab[port][i]) {
        delete_response_info(m_response_tab[port][i]);
      }
    }

    for (deque<xtsc_request*>::iterator ir = m_request_fifo[port].begin(); ir != m_request_fifo[port].end(); ++ir) {
      xtsc_request *p_request = *ir;
      delete_request(p_request);
    }
    m_request_fifo[port].clear();

    if (m_interface_type == PIF) {
      deque<response_info*>::iterator ir;

      for (ir = m_write_response_deque[port].begin(); ir != m_write_response_deque[port].end(); ++ir) {
        response_info *p_response_info = *ir;
        delete_response_info(p_response_info);
      }
      m_write_response_deque[port].clear();

      for (ir = m_pif_response_deque[port].begin(); ir != m_pif_response_deque[port].end(); ++ir) {
        response_info *p_response_info = *ir;
        delete_response_info(p_response_info);
      }
      m_pif_response_deque[port].clear();

      m_p_resp_rdy[port]->write(true);
      m_current_id[port] = 0;
      m_previous_response_last[port] = true;

    }

  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::peek(u32 port_num, xtsc_address address8, u32 size8, u8 *buffer) {
  if (m_test_shadow_memory) {
    m_p_memory->peek(address8, size8, buffer);
  }
  else {
    (*m_debug_ports[port_num])->nb_peek(address8, size8, buffer);
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::poke(u32 port_num, xtsc_address address8, u32 size8, const u8 *buffer) {
  if (m_test_shadow_memory) {
    m_p_memory->poke(address8, size8, buffer);
  }
  else {
    (*m_debug_ports[port_num])->nb_poke(address8, size8, buffer);
  }
}



bool xtsc_component::xtsc_tlm2pin_memory_transactor::fast_access(u32 port_num, xtsc_fast_access_request &request) {
  return (*m_debug_ports[port_num])->nb_fast_access(request);
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::pif_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_request_thread++;

  // A try/catch block in sc_main will not catch an exception thrown from
  // an SC_THREAD, so we'll catch them here, log them, then rethrow them.
  try {

    XTSC_INFO(m_text, "in pif_request_thread[" << port << "]");

    // Loop forever
    while (true) {

      // Wait for nb_request to tell us there's something to do 
      wait(m_request_event[port]);

      while (!m_request_fifo[port].empty()) {

        xtsc_request *p_request = m_request_fifo[port].front();
        m_request_fifo[port].pop_front();

        // Create response from request
        xtsc_response *p_response = new xtsc_response(*p_request, xtsc_response::RSP_OK);

        // Pick out some useful information about the request
        type_t            type            = p_request->get_type();
        xtsc_address      addr8           = p_request->get_byte_address();
        u32               size            = p_request->get_byte_size();
        xtsc_byte_enables byte_enables    = p_request->get_byte_enables();
        u32               bus_addr_bits   = (addr8 & m_bus_addr_bits_mask);

        // Must be a power of 2  and may not exceed bus width
        if (((size != 1) && (size != 2) && (size != 4) && (size != 8) && (size != 16)) ||
            (size > m_width8))
        {
          ostringstream oss;
          oss << "xtsc_tlm2pin_memory_transactor::pif_request_thread():  Request with invalid size=" << size << " " << *p_request;
          throw xtsc_exception(oss.str());
        }

        xtsc_byte_enables fixed_byte_enables = (byte_enables & (0xFFFFFFFFFFFFFFFFull >> (64 - size))) << bus_addr_bits;
        if ((type == xtsc_request::BLOCK_READ ) ||
            (type == xtsc_request::BLOCK_WRITE) ||
            (type == xtsc_request::BURST_READ ) ||
            (type == xtsc_request::SNOOP      ))
        {
          byte_enables = fixed_byte_enables = (0xFFFFFFFFFFFFFFFFull >> (64 - size));
        }
        if (m_big_endian) {
          swizzle_byte_enables(fixed_byte_enables);
        }
        bool is_read  = ((type == xtsc_request::READ)        ||
                         (type == xtsc_request::RCW)         ||
                         (type == xtsc_request::BLOCK_READ)  ||
                         (type == xtsc_request::BURST_READ)  ||
                         (type == xtsc_request::SNOOP));
        bool is_write = ((type == xtsc_request::WRITE)       ||
                         (type == xtsc_request::RCW)         ||
                         (type == xtsc_request::BLOCK_WRITE) ||
                         (type == xtsc_request::BURST_WRITE));
        m_address       = p_request->get_hardware_address();
        m_vadrs         = (p_request->get_snoop_virtual_address() & m_vadrs_mask) >> m_vadrs_shift;
        m_req_coh_cntl  = p_request->get_coherence();
        m_lane          = fixed_byte_enables;
        m_id            = p_request->get_id() & m_id_mask;
        m_priority      = p_request->get_priority();
        m_route_id      = p_request->get_route_id();
        m_req_attribute = p_request->get_pif_attribute();
        // Don't rely on the xtsc_request::m_id (POReqId) being unique; however, return it unchanged in xtsc_response::m_id (PIRespId)
        if (m_response_tab[port][m_id]) {
          u32 id = 0;
          for (u32 i = m_id + 1; i <= m_id + m_num_ids; ++i) {
            id = i % m_num_ids;
            if (!m_response_tab[port][id]) break;
          }
          if (m_response_tab[port][id]) {
            ostringstream oss;
            oss << kind() << " '" << name() << "' all " << m_num_ids << " POReqId's are taken.";
            throw xtsc_exception(oss.str());
          }
          XTSC_DEBUG(m_text, *p_request << ": Changing POReqId from " << m_id << " to " << id);
          m_id = id;
        }
        if (is_write && m_p_req_data[port]) {
          m_data = 0;
          const u8     *buffer          = p_request->get_buffer();
          u32           delta           = (m_big_endian ? -8 : +8);
          u32           bit_offset      = 8 * (m_big_endian ? (m_width8 - bus_addr_bits - 1) : bus_addr_bits);
          u64           bytes           = byte_enables;
          u32           max_offset      = (m_width8 - 1) * 8; // Prevent SystemC "E5 out of bounds" - slave should give an Aerr rsp
          for (u32 i = 0; (i<size) && (bit_offset <= max_offset); ++i) {
            if (bytes & 0x1) {
              m_data.range(bit_offset+7, bit_offset) = buffer[i];
              if (m_p_memory) {
                // Skip unaligned - they should get an Aerr
                if ((addr8 % size) == 0) {
                  m_p_memory->write_u8(addr8+i, buffer[i]);
                }
              }
            }
            bytes >>= 1;
            bit_offset += delta;
          }
          m_p_req_data[port]->write(m_data);
        }
        m_p_req_valid[port]->write(true);
        m_p_req_cntl[port]->write(m_req_cntl.init(*p_request));
        m_p_req_adrs[port]->write(m_address);
        m_p_req_priority[port]->write(m_priority);
        if (m_p_req_data_be  [port]) m_p_req_data_be  [port]->write(m_lane);
        if (m_p_req_id       [port]) m_p_req_id       [port]->write(m_id);
        if (m_p_req_route_id [port]) m_p_req_route_id [port]->write(m_route_id);
        if (m_p_req_coh_vadrs[port]) m_p_req_coh_vadrs[port]->write(m_vadrs);
        if (m_p_req_coh_cntl [port]) m_p_req_coh_cntl [port]->write(m_req_coh_cntl);
        if (m_p_req_attribute[port]) m_p_req_attribute[port]->write(m_req_attribute);
        response_info *p_info = new_response_info(p_response, bus_addr_bits, size, is_read, m_id, m_route_id);
        XTSC_DEBUG(m_text, "Setting m_response_tab[" << port << "][" << m_id << "]: " << *p_info->m_p_response);
        m_response_tab[port][m_id] = p_info;
        m_current_id[port] = m_id;

        // Sync to sample phase
        sc_time now = sc_time_stamp();
        sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
        if (m_has_posedge_offset) {
          if (phase_now < m_posedge_offset) {
            phase_now += m_clock_period;
          }
          phase_now -= m_posedge_offset;
        }
        sc_time time_to_sample_phase = (((phase_now < m_sample_phase) ?  m_sample_phase : m_sample_phase_plus_one) - phase_now);
        wait(time_to_sample_phase);
        // Handle according to whether or not the request was accepted
        if (m_p_req_rdy[port]->read()) {
          // Request was accepted
          // Are we responsible for write responses and is this a last-data write?
          if (m_write_responses && !is_read && p_request->get_last_transfer()) {
            p_info->m_status = (u32) xtsc_response::RSP_OK;
            p_info->m_last = true;
            m_write_response_deque[port].push_back(p_info);
            m_write_response_event[port].notify(SC_ZERO_TIME);
          }
          else if (!p_request->get_last_transfer()) {
            m_response_tab[port][m_id] = 0;
            XTSC_DEBUG(m_text, "Setting m_response_tab[" << port << "][" << m_id << "] to 0 because request was not last data");
            delete_response_info(p_info);
          }
        }
        else {
          // Request was not accepted
          m_response_tab[port][m_id] = 0;
          XTSC_DEBUG(m_text, "Setting m_response_tab[" << port << "][" << m_id << "] to 0 because request was not accepted");
          delete_response_info(p_info);
          p_response->set_status(xtsc_response::RSP_NACC);
          send_unchecked_response(p_response, port);
        }
        delete_request(p_request);
        // Continue driving the request for the balance of 1 clock period
        if (time_to_sample_phase != m_clock_period) {
          wait(m_clock_period - time_to_sample_phase);
        }
        // Deassert the request and drive all-zero values
        m_address       = 0;
        m_lane          = 0;
        m_id            = 0;
        m_priority      = 0;
        m_route_id      = 0;
        m_req_attribute = 0;
        m_vadrs         = 0;
        m_req_coh_cntl  = 0;
        m_data          = 0;
        if (m_p_req_data[port]) m_p_req_data[port]->write(m_data);
        m_p_req_valid[port]->write(false);
        m_p_req_cntl[port]->write(m_req_cntl.init(0));
        m_p_req_adrs[port]->write(m_address);
        if (m_p_req_data_be[port]) m_p_req_data_be[port]->write(m_lane);
        if (m_p_req_id[port]) m_p_req_id[port]->write(m_id);
        m_p_req_priority[port]->write(m_priority);
        if (m_p_req_route_id [port]) m_p_req_route_id [port]->write(m_route_id);
        if (m_p_req_coh_vadrs[port]) m_p_req_coh_vadrs[port]->write(m_vadrs);
        if (m_p_req_coh_cntl [port]) m_p_req_coh_cntl [port]->write(m_req_coh_cntl);
        if (m_p_req_attribute[port]) m_p_req_attribute[port]->write(m_req_attribute);
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



void xtsc_component::xtsc_tlm2pin_memory_transactor::pif_response_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_response_thread++;

  try {

    XTSC_INFO(m_text, "in pif_response_thread[" << port << "]");

    // Work around Red X bug in ModelSim versions < 6.3i.  Emails of 22 && 23 Sep 2008.  Fixed in Modelsim 6.3i and 6.4a.
    // With OSCI can get: Error: (E115) sc_signal<T> cannot have more than one drive
#if defined(MTI_SYSTEMC)
    m_p_resp_rdy[port]->write(false);
    m_p_resp_rdy[port]->write(true);
#endif

    // Loop forever
    while (true) {

      do {
        // Wait for either PIRespValid or PORespRdy to go high or for locally-generated write response
        XTSC_DEBUG(m_text, "pif_response_thread[" << port << "]: calling wait()");
        wait(); // m_p_resp_valid[port]->pos()  m_p_resp_rdy[port]->pos()  m_write_response_event[port];
        XTSC_DEBUG(m_text, "pif_response_thread[" << port << "]: awoke from wait()");
      } while ((!m_p_resp_valid[port]->read() || !m_p_resp_rdy[port]->read()) &&
               m_pif_response_deque[port].empty() &&
               m_write_response_deque[port].empty());

      // Sync to sample phase.  If already at the sample phase, wait 1 full clock period to ensure
      // PIRespValid and PORespRdy are high a non-zero amount of time.
      sc_time now = sc_time_stamp();
      sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
      if (m_has_posedge_offset) {
        if (phase_now < m_posedge_offset) {
          phase_now += m_clock_period;
        }
        phase_now -= m_posedge_offset;
      }
      wait(((phase_now >= m_sample_phase) ? m_sample_phase_plus_one : m_sample_phase) - phase_now);
      XTSC_DEBUG(m_text, "pif_response_thread[" << port << "]: sync'd to sample phase");

      // Once each clock period:
      //  1) Capture one pin-level response from downstream slave if both PIRespValid and PORespRdy are asserted
      //  2) Attempt to send current TLM response to upstream master
      while ((m_p_resp_valid[port]->read() && m_p_resp_rdy[port]->read()) ||
             !m_pif_response_deque[port].empty() ||
             !m_write_response_deque[port].empty())
      {

        // Capture and save pin-level response (if there is one this clock cycle)
        if (m_p_resp_valid[port]->read() && m_p_resp_rdy[port]->read()) {
          u32 id   = m_p_resp_id[port] ? m_p_resp_id[port]->read().to_uint() : m_current_id[port];
          u32 cntl = m_p_resp_cntl[port]->read().to_uint();
          response_info *p_info = m_response_tab[port][id];
          if (!p_info) {
            ostringstream oss;
            oss << kind() << " '" << name() << "' got response ID=" << id << " with no corresponding request.";
            if (m_write_responses) {
              oss << endl
                  << "(A possible cause of this problem is having both this transactor AND the downstream slave "
                  << "configured to send write responses)";
            }
            throw xtsc_exception(oss.str());
          }
          p_info->m_last = ((cntl & resp_cntl::m_last_mask) == 1);
          p_info->m_status = (m_snoop ? 0 : ((cntl & resp_cntl::m_status_mask) >> resp_cntl::m_status_shift));
          if (!p_info->m_last) {
            // This should only happen for a BLOCK_READ|BURST_READ|SNOOP
            p_info = new_response_info(*p_info);
            XTSC_DEBUG(m_text, "Created duplicated response_info " << p_info);
          }
          if (m_p_resp_priority[port]) {
            p_info->m_p_response->set_priority((u8) m_p_resp_priority[port]->read().to_uint());
          }
          if (m_p_resp_route_id[port]) {
            p_info->m_p_response->set_route_id(m_p_resp_route_id[port]->read().to_uint());
          }
          if (m_snoop) {
            p_info->m_p_response->set_coherence((cntl & resp_cntl::m_shared_mask) ? xtsc_response::SHARED : xtsc_response::INVALID);
          }
          else if (m_p_resp_coh_cntl[port]) {
            p_info->m_p_response->set_coherence((xtsc_response::coherence_t)m_p_resp_coh_cntl[port]->read().to_uint());
          }
          bool has_data = (m_snoop ? (cntl & resp_cntl::m_data_mask) : (!p_info->m_status && p_info->m_is_read));
          if (has_data) {
            p_info->m_copy_data = true;
            m_data = m_p_resp_data[port]->read();
            u32 bus_addr_bits = p_info->m_bus_addr_bits;
            u32 size          = p_info->m_size;
            u8 *buffer        = p_info->m_buffer;
            u32 delta         = (m_big_endian ? -8 : +8);
            u32 bit_offset    = 8 * (m_big_endian ? (m_width8 - bus_addr_bits - 1) : bus_addr_bits);
            u32 max_offset    = (m_width8 - 1) * 8; // Prevent SystemC "E5 out of bounds" - slave should have given an Aerr response
            for (u32 i = 0; (i<size) && (bit_offset <= max_offset); ++i) {
              buffer[i] = (u8) m_data.range(bit_offset+7, bit_offset).to_uint();
              bit_offset += delta;
            }
          }
          if (m_snoop) {
            p_info->m_p_response->set_snoop_data(has_data);
          }
          m_pif_response_deque[port].push_back(p_info);
          XTSC_VERBOSE(m_text, "pif_response_thread[" << port << "]: Got response: id=" << id << " status=" << p_info->m_status <<
                               " tag=" << p_info->m_p_response->get_tag() << " address=0x" << setfill('0') << hex <<
                               setw(8) << p_info->m_p_response->get_byte_address());
        }
        
        bool accepted = true;

        // If previous response was accepted and was a last-transfer then push all outstanding locally-generated
        // write responses to the front of the line (this is to avoid interrupting a BLOCK_READ|BURST_READ sequence).
        // We use a reverse iterator so this set will be in time order; however, it is possible for a write that
        // happens in the near future to get its response inserted in front of this set.
        if (m_previous_response_last[port] && !m_write_response_deque[port].empty()) {
          deque<response_info*>::reverse_iterator ir;
          for (ir = m_write_response_deque[port].rbegin(); ir != m_write_response_deque[port].rend(); ++ir) {
            XTSC_DEBUG(m_text, "Moving response_info " << (*ir) << " from m_write_response_deque to m_pif_response_deque");
            m_pif_response_deque[port].push_front(*ir);
          }
          m_write_response_deque[port].clear();
        }


        if (!m_pif_response_deque[port].empty()) {

          // Now handle current TLM response
          response_info *p_info = m_pif_response_deque[port].front();

          if (p_info->m_copy_data) {
            memcpy(p_info->m_p_response->get_buffer(), p_info->m_buffer, p_info->m_size);
            p_info->m_copy_data = false;
          }
          p_info->m_p_response->set_last_transfer(p_info->m_last);
          p_info->m_p_response->set_status((xtsc_response::status_t) p_info->m_status);

          // Log the response
          XTSC_INFO(m_text, *p_info->m_p_response << " Port #" << port);
          
          accepted = (*m_respond_ports[port])->nb_respond(*p_info->m_p_response);

          if (accepted) {
            m_pif_response_deque[port].pop_front();
            m_previous_response_last[port] = p_info->m_last;
            // Delete it?
            if (p_info->m_last) {
              u32 id = p_info->m_id;
              delete p_info->m_p_response;
              delete_response_info(p_info);
              m_response_tab[port][id] = 0;
              XTSC_DEBUG(m_text, "Setting m_response_tab[" << port << "][" << id << "] to 0 because last was true");
            }
            else {
              XTSC_DEBUG(m_text, "Deleting duplicated response_info " << p_info);
              delete_response_info(p_info);
            }
          }
          else {
            m_previous_response_last[port] = false;   // Treat as non-last so write responses can't cut in
            m_resp_rdy_fifo[port]->write(true);
            m_drive_resp_rdy_event[port].notify(m_output_delay);
            XTSC_INFO(m_text, *p_info->m_p_response << " Port #" << port << " <-- REJECTED");
          }
        }
        wait(m_clock_period);
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_response_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_tlm2pin_memory_transactor::pif_drive_resp_rdy_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_pif_drive_resp_rdy_thread++;

  try {

    XTSC_INFO(m_text, "in pif_drive_resp_rdy_thread[" << port << "]");

    // Loop forever
    while (true) {
      // Wait to be notified
      wait(m_drive_resp_rdy_event[port]);
      while (m_resp_rdy_fifo[port]->num_available()) {
        bool dummy;
        m_resp_rdy_fifo[port]->nb_read(dummy);
        m_p_resp_rdy[port]->write(false);
        XTSC_DEBUG(m_text, "pif_drive_resp_rdy_thread[" << port << "] deasserting PIRespRdy");
        wait(m_clock_period);
        m_p_resp_rdy[port]->write(true);
        XTSC_DEBUG(m_text, "pif_drive_resp_rdy_thread[" << port << "] asserting PIRespRdy");
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in pif_drive_resp_rdy_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_tlm2pin_memory_transactor::lcl_request_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_request_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_request_thread[" << port << "]");

    // If present, drive 0 on parity/ECC signal
    if (m_p_check_wr[port]) {
      sc_bv_base dummy((int)m_check_bits);
      dummy = 0;
      m_p_check_wr[port]->write(dummy);
    }

    // Loop forever
    while (true) {

      // Wait for nb_request to tell us there's something to do 
      wait(m_request_event[port]);

      while (!m_request_fifo[port].empty()) {

        xtsc_request *p_request = m_request_fifo[port].front();
        m_request_fifo[port].pop_front();

        // Create response from request
        xtsc_response *p_response = new xtsc_response(*p_request, xtsc_response::RSP_OK);

        // Pick out some useful information about the request
        xtsc_address      addr8           = p_request->get_byte_address() - m_start_byte_address;
        u32               size            = p_request->get_byte_size();
        xtsc_byte_enables byte_enables    = p_request->get_byte_enables();
        u32               bus_addr_bits   = (addr8 & m_bus_addr_bits_mask);
        bool              inst_fetch      = p_request->get_instruction_fetch();

        // Must be a power of 2  and may not exceed bus width
        if (((size != 1) && (size != 2) && (size != 4) && (size != 8) && (size != 16) && (size != 32) && (size != 64)) ||
            (size > m_width8))
        {
          ostringstream oss;
          oss << "xtsc_tlm2pin_memory_transactor::lcl_request_thread():  Request with invalid size=" << size << " " << *p_request;
          throw xtsc_exception(oss.str());
        }

        m_address = ((addr8 & m_address_mask) >> m_address_shift);

        // Optionally, compute and drive byte/word lanes 
        if (m_p_lane[port]) {
          xtsc_byte_enables fixed_byte_enables = (byte_enables & (0xFFFFFFFFFFFFFFFFull >> (64 - size))) << bus_addr_bits;
#if 0
          XTSC_NOTE(m_text, "addr8=0x" << hex << addr8 << " byte_enables=0x" << byte_enables <<
                            " fixed_byte_enables=0x" << fixed_byte_enables << " size=" << dec << size <<
                            " bus_addr_bits=" << bus_addr_bits);
#endif
          if (m_big_endian) {
            swizzle_byte_enables(fixed_byte_enables);
          }
          if ((m_interface_type == IRAM0) || (m_interface_type == IRAM1) || (m_interface_type == IROM0)) {
            // Convert byte enables to word enables (1 "word" is 32 bits) (64-bit IRAM0|IRAM1|IROM0 only)
            xtsc_byte_enables fixed_word_enables = 0;
            if (fixed_byte_enables & 0x000F) fixed_word_enables |= 0x1;
            if (fixed_byte_enables & 0x00F0) fixed_word_enables |= 0x2;
            if (fixed_byte_enables & 0x0F00) fixed_word_enables |= 0x4;
            if (fixed_byte_enables & 0xF000) fixed_word_enables |= 0x8;
            m_lane = fixed_word_enables;
          }
          else {
            m_lane = fixed_byte_enables;
          }
          m_p_lane[port]->write(m_lane);
        }

        // Optionally, drive IRamNLoadStore|IRom0Load|URam0LoadStore
        if (m_p_load[port] && (m_interface_type != XLMI0)) {
          m_p_load[port]->write(!inst_fetch);
          if (!inst_fetch) {
            XTSC_VERBOSE(m_text, m_p_load[port]->name() << " asserted");
          }
        }

        // Optionally, drive DRamNXferEn0|IRamNXferEn0|IRom0XferEn0|URam0XferEn0
        if (m_has_xfer_en) {
          m_p_xfer_en[port]->write(p_request->get_xfer_en());
        }

        // Handle request according to its type
        xtsc_request::type_t type = p_request->get_type();
        bool is_read = (type == xtsc_request::READ);
        if (is_read) {
          if (m_interface_type == XLMI0) {
            // DPortLoad is asserted 1 cycle after the request
            m_load_event_queue[port].notify(m_clock_period);
            m_load_deque[port].push_back(true);
            // . . . so it needs to be deassert 2 cycles after the request
            m_load_event_queue[port].notify(2*m_clock_period);
            m_load_deque[port].push_back(false);
          }
          m_p_en[port]->write(true);
          if (m_p_wr[port]) m_p_wr[port]->write(false);
          m_p_addr[port]->write(m_address);
          XTSC_DEBUG(m_text, "READ:  m_address=0x" << hex << m_address << " lane=0x" << m_lane);
        }
        else if ((type == xtsc_request::WRITE) && m_p_wrdata[port]) {
          m_data = 0;
          const u8 *buffer = p_request->get_buffer();
          u32 delta      = (m_big_endian ? -8 : +8);
          u32 bit_offset = 8 * (m_big_endian ? (m_width8 - bus_addr_bits - 1) : bus_addr_bits);
          u64 bytes      = byte_enables;
          for (u32 i = 0; i<size; ++i) {
            if (bytes & 0x1) {
              m_data.range(bit_offset+7, bit_offset) = buffer[i];
              if (m_p_memory) {
                m_p_memory->write_u8(m_start_byte_address+addr8+i, buffer[i]);
              }
            }
            bytes >>= 1;
            bit_offset += delta;
          }
          m_p_en[port]->write(true);
          if (m_p_wr[port]) m_p_wr[port]->write(true);
          m_p_addr[port]->write(m_address);
          m_p_wrdata[port]->write(m_data);
          XTSC_DEBUG(m_text, "WRITE: m_address=0x" << hex << m_address << " lane=0x" << m_lane << " data=0x" << m_data);
        }
        else {
          // BLOCK_READ, BLOCK_WRITE, BURST_READ, BURST_WRITE, and RCW aren't supported on local memory interfaces
          // WRITE isn't support on IROM and DROM
          ostringstream oss;
          oss << "Unsupported request type=" << p_request->get_type_name() << " passed to " << m_interface_uc;
          throw xtsc_exception(oss.str());
        }

        sc_time now = sc_time_stamp();
        sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
        if (m_has_posedge_offset) {
          if (phase_now < m_posedge_offset) {
            phase_now += m_clock_period;
          }
          phase_now -= m_posedge_offset;
        }
        sc_time delay_time = (((phase_now < m_sample_phase) ? m_sample_phase : m_sample_phase_plus_one) - phase_now);
        response_info *p_info = new_response_info(p_response, bus_addr_bits, size, is_read);
        if (m_has_busy || !is_read) {
          m_busy_write_rsp_deque[port].push_back(p_info);
          m_busy_write_rsp_event_queue[port].notify(delay_time);
        }
        else {
          m_read_data_rsp_deque[port].push_back(p_info);
          m_read_data_event_queue[port].notify(delay_time + m_read_delay_time);
        }

        wait(m_clock_period);

        // Deassert the request
        if (m_p_load[port] && (m_interface_type != XLMI0)) {
          m_p_load[port]->write(false);
        }
        if (m_p_lane[port]) {
          m_lane = 0;
          m_p_lane[port]->write(m_lane);
        }
        if (m_has_xfer_en) {
          m_p_xfer_en[port]->write(false);
        }
        m_address = 0;
        m_p_en[port]->write(false);
        if (m_p_wr[port]) m_p_wr[port]->write(false);
        m_p_addr[port]->write(m_address);
        if (m_p_wrdata[port]) {
          m_data = 0;
          m_p_wrdata[port]->write(m_data);
        }
        delete_request(p_request);
      }

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



void xtsc_component::xtsc_tlm2pin_memory_transactor::lcl_busy_write_rsp_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_busy_write_rsp_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_busy_write_rsp_thread[" << port << "]");

    // Loop forever
    while (true) {
      wait();   // m_busy_write_rsp_event_queue[port]
      response_info *p_info = m_busy_write_rsp_deque[port].front();
      m_busy_write_rsp_deque[port].pop_front();
      bool send_response = true;
      if (m_has_busy) {
        if (m_p_busy[port]->read()) {
          // It is busy so send RSP_NACC
          p_info->m_p_response->set_status(xtsc_response::RSP_NACC);
        }
        else if (p_info->m_is_read) {
          // Not busy and is a read so schedule read data
          send_response = false;
          m_read_data_rsp_deque[port].push_back(p_info);
          m_read_data_event_queue[port].notify(m_read_delay_time);
        }
      }
      if (send_response) {
        send_unchecked_response(p_info->m_p_response, port);
        delete_response_info(p_info);
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in lcl_busy_write_rsp_thread[" << port << "] of " << kind() << " '" << name() << "'."
        << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_tlm2pin_memory_transactor::lcl_sample_read_data_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_lcl_sample_read_data_thread++;

  try {

    XTSC_INFO(m_text, "in lcl_sample_read_data_thread[" << port << "]");

    // Loop forever
    while (true) {
      wait();   // m_read_data_event_queue[port]
      response_info *p_info = m_read_data_rsp_deque[port].front();
      m_read_data_rsp_deque[port].pop_front();
      m_data = m_p_data[port]->read();
      u32 bus_addr_bits = p_info->m_bus_addr_bits;
      u32 size          = p_info->m_size;
      u8 *buffer        = p_info->m_p_response->get_buffer();
      u32 delta         = (m_big_endian ? -8 : +8);
      u32 bit_offset    = 8 * (m_big_endian ? (m_width8 - bus_addr_bits - 1) : bus_addr_bits);
      for (u32 i = 0; i<size; ++i) {
        buffer[i] = (u8) m_data.range(bit_offset+7, bit_offset).to_uint();
        bit_offset += delta;
      }
      send_unchecked_response(p_info->m_p_response, port);
      delete_response_info(p_info);
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in lcl_sample_read_data_thread[" << port << "] of " << kind() << " '" << name() << "'."
        << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



// Handle DPortLoadRetired
void xtsc_component::xtsc_tlm2pin_memory_transactor::xlmi_retire_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_xlmi_retire_thread++;

  try {

    XTSC_INFO(m_text, "in xlmi_retire_thread[" << port << "]");

    // Loop forever
    while (true) {
      m_p_retire[port]->write(false);
      wait(m_retire_event[port]);
      m_p_retire[port]->write(true);
      sc_time now = sc_time_stamp();
      while (m_retire_deassert[port] > now) {
        wait(m_retire_deassert[port] - now);
        now = sc_time_stamp();
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in xlmi_retire_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xlmi_flush_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_xlmi_flush_thread++;

  try {

    XTSC_INFO(m_text, "in xlmi_flush_thread[" << port << "]");

    // Loop forever
    while (true) {
      m_p_flush[port]->write(false);
      wait(m_flush_event[port]);
      m_p_flush[port]->write(true);
      sc_time now = sc_time_stamp();
      while (m_flush_deassert[port] > now) {
        wait(m_flush_deassert[port] - now);
        now = sc_time_stamp();
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in xlmi_flush_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



// Handle DRamLock
void xtsc_component::xtsc_tlm2pin_memory_transactor::dram_lock_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_dram_lock_thread++;

  try {

    XTSC_INFO(m_text, "in dram_lock_thread[" << port << "]");
    m_p_lock[port]->write(false);

    // Loop forever
    while (true) {
      wait();   // m_lock_event_queue[port]
      bool lock = m_lock_deque[port].front();
      m_lock_deque[port].pop_front();
      m_p_lock[port]->write(lock);
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in dram_lock_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



// Drive m_p_load/DPortLoad (without busy) or m_p_preload (with busy)
void xtsc_component::xtsc_tlm2pin_memory_transactor::xlmi_load_thread(void) {

  // Get the port number for this "instance" of the thread
  u32 port = m_next_port_xlmi_load_thread++;

  try {

    XTSC_INFO(m_text, "in xlmi_load_thread[" << port << "]");

    if (m_has_busy) {
      m_p_preload[port]->write(false);
    }
    else {
      m_p_load[port]->write(false);
    }

    // Loop forever
    while (true) {
      wait();   // m_load_event_queue[port]
      bool load = m_load_deque[port].front();
      m_load_deque[port].pop_front();
      if (m_has_busy) {
        m_p_preload[port]->write(load);
      }
      else {
        m_p_load[port]->write(load);
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in xlmi_load_thread[" << port << "] of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



// Drive m_p_load/DPortLoad in case where XLMI has busy (because DPortLoad is qualified by DPortBusy)
void xtsc_component::xtsc_tlm2pin_memory_transactor::xlmi_load_method(void) {
  for (u32 port=0; port<m_num_ports; ++port) {
    m_p_load[port]->write(m_p_preload[port]->read() && !m_p_busy[port]->read());
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::send_unchecked_response(xtsc_response*& p_response, u32 port) {
  // Log the response
  XTSC_INFO(m_text, *p_response << " Port #" << port);
  
  // Send response
  (*m_respond_ports[port])->nb_respond(*p_response);

  // Delete it?
  if ((m_interface_type != PIF) || (p_response->get_last_transfer())) {
    delete p_response;
    p_response = 0;
  }
}



xtsc_component::xtsc_tlm2pin_memory_transactor::
response_info *xtsc_component::xtsc_tlm2pin_memory_transactor::new_response_info(xtsc_response    *p_response,
                                                                                 u32               bus_addr_bits,
                                                                                 u32               size,
                                                                                 bool              is_read,
                                                                                 u32               id,
                                                                                 u32               route_id)
{
  if (m_response_pool.empty()) {
    response_info *p_response_info = new response_info(p_response, bus_addr_bits, size, is_read, id, route_id);
    XTSC_DEBUG(m_text, "Created new response_info (" << p_response_info << ") from an xtsc_response: " << *p_response);
    return p_response_info;
  }
  else {
    response_info *p_response_info = m_response_pool.back();
    XTSC_DEBUG(m_text, "Recycling response_info (" << p_response_info << ") given an xtsc_response: " << *p_response);
    m_response_pool.pop_back();
    p_response_info->m_p_response    = p_response;
    p_response_info->m_bus_addr_bits = bus_addr_bits;
    p_response_info->m_size          = size;
    p_response_info->m_is_read       = is_read;
    p_response_info->m_id            = id;
    p_response_info->m_route_id      = route_id;
    return p_response_info;
  }
}



xtsc_component::xtsc_tlm2pin_memory_transactor::
response_info *xtsc_component::xtsc_tlm2pin_memory_transactor::new_response_info(const response_info& info) {
  if (m_response_pool.empty()) {
    response_info *p_response_info = new response_info(info);
    XTSC_DEBUG(m_text, "Created new response_info (" << p_response_info << ") from another response_info");
    return p_response_info;
  }
  else {
    response_info *p_response_info = m_response_pool.back();
    XTSC_DEBUG(m_text, "Recycling response_info (" << p_response_info << ") given a response_info");
    m_response_pool.pop_back();
    *p_response_info = info;
    return p_response_info;
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::delete_response_info(response_info*& p_response_info) {
  XTSC_DEBUG(m_text, "Freeing up response_info " << p_response_info);
  m_response_pool.push_back(p_response_info);
  p_response_info = 0;
}



xtsc_request *xtsc_component::xtsc_tlm2pin_memory_transactor::new_request(const xtsc_request& request) {
  if (m_request_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new xtsc_request");
    return new xtsc_request(request);
  }
  else {
    xtsc_request *p_request = m_request_pool.back();
    m_request_pool.pop_back();
    *p_request = request;
    return p_request;
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::delete_request(xtsc_request*& p_request) {
  m_request_pool.push_back(p_request);
  p_request = 0;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::swizzle_byte_enables(xtsc_byte_enables& byte_enables) const {
  xtsc_byte_enables swizzled = 0;
  for (u32 i=0; i<m_width8; i++) {
    swizzled <<= 1;
    swizzled |= byte_enables & 1;
    byte_enables >>= 1;
  }
  byte_enables = swizzled;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  m_transactor.peek(m_port_num, address8, size8, buffer);
  if (xtsc_is_text_logging_enabled() && m_transactor.m_text.isEnabledFor(VERBOSE_LOG_LEVEL)) {
    u32 buf_offset = 0;
    ostringstream oss;
    oss << hex << setfill('0');
    for (u32 i = 0; i<size8; ++i) {
      oss << setw(2) << (u32) buffer[buf_offset] << " ";
      buf_offset += 1;
    }
    XTSC_VERBOSE(m_transactor.m_text, "peek: " << " [0x" << hex << address8 << "/" << size8 << "] = " << oss.str());
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_poke(xtsc_address  address8,
                                                                                   u32           size8,
                                                                                   const u8     *buffer)
{
  m_transactor.poke(m_port_num, address8, size8, buffer);
  if (xtsc_is_text_logging_enabled() && m_transactor.m_text.isEnabledFor(VERBOSE_LOG_LEVEL)) {
    u32 buf_offset = 0;
    ostringstream oss;
    oss << hex << setfill('0');
    for (u32 i = 0; i<size8; ++i) {
      oss << setw(2) << (u32) buffer[buf_offset] << " ";
      buf_offset += 1;
    }
    XTSC_VERBOSE(m_transactor.m_text, "poke: " << " [0x" << hex << address8 << "/" << size8 << "] = " << oss.str());
  }
}



bool xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_fast_access(xtsc_fast_access_request &request) {
  return m_transactor.fast_access(m_port_num, request);
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_request(const xtsc_request& request) {
  XTSC_INFO(m_transactor.m_text, request << " Port #" << m_port_num);

  // Can we accept the request at this time?
  if (m_transactor.m_request_fifo[m_port_num].size() >= m_transactor.m_request_fifo_depth) {
    // No. We're full.  Create an RSP_NACC response.
    xtsc_response response(request, xtsc_response::RSP_NACC, true);
    // Log the response
    XTSC_INFO(m_transactor.m_text, response << " Port #" << m_port_num);
    // Send the response
    (*m_transactor.m_respond_ports[m_port_num])->nb_respond(response);
  }
  else {
    // Create and queue our copy of the request
    m_transactor.m_request_fifo[m_port_num].push_back(m_transactor.new_request(request));
    // Notify the appropriate request thread
    m_transactor.m_request_event[m_port_num].notify(m_transactor.m_output_delay);
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_load_retired(xtsc_address address8) {
  if (m_transactor.m_interface_type != m_transactor.XLMI0) {
    ostringstream oss;
    oss << "'" << m_transactor.name() << "': nb_load_retired() called for interface type '"
        << xtsc_module_pin_base::get_interface_name(m_transactor.m_interface_type) << "' (only allowed for XLMI0)";
    throw xtsc_exception(oss.str());
  }
  XTSC_VERBOSE(m_transactor.m_text, "nb_load_retired(0x" << setfill('0') << hex << setw(8) << address8 <<
                                     ") Port #" << m_port_num);
  m_transactor.m_retire_event[m_port_num].notify(m_transactor.m_output_delay);
  m_transactor.m_retire_deassert[m_port_num] = sc_time_stamp() + m_transactor.m_output_delay + m_transactor.m_clock_period;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_retire_flush() {
  if (m_transactor.m_interface_type != m_transactor.XLMI0) {
    ostringstream oss;
    oss << "'" << m_transactor.name() << "': nb_retire_flush() called for interface type '"
        << xtsc_module_pin_base::get_interface_name(m_transactor.m_interface_type) << "' (only allowed for XLMI0)";
    throw xtsc_exception(oss.str());
  }
  XTSC_VERBOSE(m_transactor.m_text, "nb_retire_flush() Port #" << m_port_num);
  m_transactor.m_flush_event[m_port_num].notify(m_transactor.m_output_delay);
  m_transactor.m_flush_deassert[m_port_num] = sc_time_stamp() + m_transactor.m_output_delay + m_transactor.m_clock_period;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::nb_lock(bool lock) {
  if ((m_transactor.m_interface_type != m_transactor.DRAM0) && (m_transactor.m_interface_type != m_transactor.DRAM1)) {
    ostringstream oss;
    oss << "'" << m_transactor.name() << "': nb_lock() called for interface type '"
        << xtsc_module_pin_base::get_interface_name(m_transactor.m_interface_type) << "' (only allowed for DRAM0|DRAM1)";
    throw xtsc_exception(oss.str());
  }
  if (!m_transactor.m_has_lock) {
    ostringstream oss;
    oss << "'" << m_transactor.name() << "': nb_lock() called but \"has_lock\" is false";
    throw xtsc_exception(oss.str());
  }
  XTSC_VERBOSE(m_transactor.m_text, "nb_lock(" << boolalpha << lock << ") Port #" << m_port_num);
  m_transactor.m_lock_deque[m_port_num].push_back(lock);
  m_transactor.m_lock_event_queue[m_port_num].notify(m_transactor.m_output_delay);
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_request_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_tlm2pin_memory_transactor '" << m_transactor.name() << "' m_request_exports["
        << m_port_num << "]: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_transactor.m_text, "Binding '" << port.name() << "' to '" << m_transactor.name() << ".m_request_exports[" <<
                                 m_port_num << "]'");
  m_p_port = &port;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_debug_if_cap::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  if (m_transactor.m_peek) {
    m_transactor.m_peek(address8, size8, buffer, m_transactor.m_dso_cookie, m_port_num);
  }
  else {
    if (!m_transactor.m_cosim) {
      ostringstream oss;
      oss << m_transactor.name() << ": nb_peek() method called for capped port";
      if (m_p_port) { oss << ": " << m_p_port->name(); }
      oss << " (Try setting \"cosim\" to true)";
      throw xtsc_exception(oss.str());
    }
    if (m_transactor.m_p_memory) {
      m_transactor.m_p_memory->peek(address8, size8, buffer);
    }
  }
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_debug_if_cap::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  if (m_transactor.m_poke) {
    m_transactor.m_poke(address8, size8, buffer, m_transactor.m_dso_cookie, m_port_num);
  }
  else {
    if (!m_transactor.m_cosim) {
      ostringstream oss;
      oss << m_transactor.name() << ": nb_poke() method called for capped port";
      if (m_p_port) { oss << ": " << m_p_port->name(); }
      oss << " (Try setting \"cosim\" to true)";
      throw xtsc_exception(oss.str());
    }
    if (m_transactor.m_p_memory) {
      m_transactor.m_p_memory->poke(address8, size8, buffer);
    }
  }
}



bool xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_debug_if_cap::nb_fast_access(xtsc_fast_access_request &request) {
  if (m_transactor.m_peek) {
    request.allow_peek_poke_access();
    return true;
  }
  if (!m_transactor.m_cosim) {
    ostringstream oss;
    oss << m_transactor.name() << ": nb_fast_access() method called for capped port";
    if (m_p_port) { oss << ": " << m_p_port->name(); }
    oss << " (Try setting \"cosim\" to true)";
    throw xtsc_exception(oss.str());
  }
  return false;
}



void xtsc_component::xtsc_tlm2pin_memory_transactor::xtsc_debug_if_cap::register_port(sc_port_base& port, const char *if_typename) {
  m_p_port = &port;
}




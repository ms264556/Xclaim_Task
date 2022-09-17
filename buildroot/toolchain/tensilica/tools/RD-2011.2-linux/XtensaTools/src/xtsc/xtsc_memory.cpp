// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cstdlib>
#include <ostream>
#include <string>
#include <xtsc/xtsc_memory.h>
#include <xtsc/xtsc_arbiter.h>
#include <xtsc/xtsc_cohctrl.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_dma_engine.h>
#include <xtsc/xtsc_master.h>
#include <xtsc/xtsc_memory_trace.h>
#include <xtsc/xtsc_router.h>
#include <xtsc/xtsc_pin2tlm_memory_transactor.h>
#include <xtsc/xtsc_logging.h>
#include <xtsc/xtsc_fast_access.h>

using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;
using log4xtensa::DEBUG_LOG_LEVEL;
//using xtsc::xtsc_fast_access;
using xtsc::xtsc_raw_block;


namespace xtsc_component {

/* Simple class that just redispatches to the peek poke function */
class xtsc_memory_fast_access : public xtsc_fast_access_if {
public:
  
  xtsc_memory_fast_access(xtsc_memory& mem) : m_mem(mem)
  {
  }

  void nb_fast_access_read(xtsc_address address8, u32 size8, u8 *dst) {
    m_mem.peek(address8, size8, dst);
  }
  
  void nb_fast_access_write(xtsc_address address8, u32 size8, const u8 *src) {
    m_mem.poke(address8, size8, src);
  }

private:

  xtsc_memory&          m_mem;
};

} // namespace xtsc_component


xtsc_component::xtsc_memory_parms::xtsc_memory_parms(const xtsc_core& core, const char *port_name, u32 delay, u32 num_ports) {
  xtsc_core::memory_port mem_port = xtsc_core::get_memory_port(port_name); 
  if (!core.has_memory_port(mem_port)) {
    ostringstream oss;
    oss << "xtsc_memory_parms: core '" << core.name() << "' has no " << port_name << " memory interface.";
    throw xtsc_exception(oss.str());
  }

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

  u32   width8            = core.get_memory_byte_width(mem_port);
  u32   start_address8    = 0;
  u32   size8             = 0;  // 4GB

  if (mem_port == xtsc_core::MEM_PIF) {
    if (delay == 0xFFFFFFFF) {
      delay = core_parms.get_u32("LocalMemoryLatency");
    }
  }
  else {
    if ((mem_port == xtsc_core::MEM_XLMI0LS0) ||
        (mem_port == xtsc_core::MEM_XLMI0LS1) ||
        core_parms.get_bool("SimFullLocalMemAddress"))
    {
      core.get_local_memory_starting_byte_address(mem_port, start_address8);
    }
    core.get_local_memory_byte_size(mem_port, size8);
    delay = core_parms.get_u32("LocalMemoryLatency") - 1;
  }

  init(width8, delay, start_address8, size8, num_ports);

  set("clock_period", core_parms.get_u32("SimClockFactor")*xtsc_get_system_clock_factor());
  if (mem_port == xtsc_core::MEM_PIF) {
    set("check_alignment", true);
  }
  if ((mem_port == xtsc_core::MEM_IROM0) || (mem_port == xtsc_core::MEM_DROM0LS0) || (mem_port == xtsc_core::MEM_DROM0LS1)) {
    set("read_only", true);
  }
}



xtsc_component::xtsc_memory::xtsc_memory(sc_module_name module_name, const xtsc_memory_parms& memory_parms) :
  sc_module             (module_name),
  m_use_fast_access     (memory_parms.get_bool("use_fast_access")),
  m_deny_fast_access    (memory_parms.get_u32_vector("deny_fast_access")),
  m_use_raw_access      (memory_parms.get_bool("use_raw_access")),
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

  memory_parms.get("read_only",          m_read_only);
  memory_parms.get("immediate_timing",   m_immediate_timing);
  memory_parms.get("delay_from_receipt", m_delay_from_receipt);
  memory_parms.get("wraparound",         m_wraparound);
  memory_parms.get("fail_request_mask",  m_fail_request_mask);
  memory_parms.get("fail_percentage",    m_fail_percentage);
  compute_let_through();
  memory_parms.get("fail_seed",          m_fail_seed);
  srand(m_fail_seed);
  u32 status;
  memory_parms.get("fail_status",        status);
  m_fail_status = static_cast<xtsc_response::status_t>(status);

  m_num_ports                   = memory_parms.get_non_zero_u32("num_ports");
  m_check_alignment             = memory_parms.get_bool("check_alignment");

  const char *script_file       = memory_parms.get_c_str("script_file");
  if (m_fail_request_mask && script_file) {
    ostringstream oss;
    oss << "Error in " << kind() << " '" << name()
        << "': It is not legal for both \"fail_request_mask\" and \"script_file\" to be defined (i.e. to be non-zero)";
    throw xtsc_exception(oss.str());
  }
  m_script_file                 = (script_file ? script_file : "");
  m_p_script_stream             = NULL;

  if (m_script_file != "") {
    m_p_script_stream  = new xtsc_script_file(m_script_file.c_str(), "script_file", name(), kind(), m_wraparound);
  }

  // Create all the "per-port" stuff

  m_request_exports = new sc_export<xtsc_request_if>*[m_num_ports];
  m_request_impl    = new xtsc_request_if_impl*      [m_num_ports];
  for (u32 i=0; i<m_num_ports; ++i) {
    ostringstream oss1;
    oss1 << "m_request_exports[" << i << "]";
    m_request_exports[i] = new sc_export<xtsc_request_if>(oss1.str().c_str());
    ostringstream oss2;
    oss2 << "m_request_impl[" << i << "]";
    m_request_impl   [i] = new xtsc_request_if_impl(oss2.str().c_str(), *this, i);
    (*m_request_exports[i])(*m_request_impl[i]);
  }

  m_request_fifo = new sc_fifo<request_info*>*[m_num_ports];
  for (u32 i=0; i<m_num_ports; ++i) {
    ostringstream oss;
    oss << "m_request_fifo[" << i << "]";
    m_request_fifo[i] = new sc_fifo<request_info*>(oss.str().c_str(), memory_parms.get_non_zero_u32("request_fifo_depth"));
  }

  m_respond_ports = new sc_port<xtsc_respond_if>*[m_num_ports];
  for (u32 i=0; i<m_num_ports; ++i) {
    ostringstream oss;
    oss << "m_respond_ports[" << i << "]";
    m_respond_ports[i] = new sc_port<xtsc_respond_if>(oss.str().c_str());
  }

  m_rcw_compare_data            = new u8[m_num_ports*m_width8];
  m_rcw_have_first_transfer     = new bool[m_num_ports];
  m_p_active_request_info       = new request_info*[m_num_ports];
  m_p_active_response           = new xtsc_response*[m_num_ports];
  m_block_write_transfer_count  = new u32[m_num_ports];
  m_burst_write_transfer_count  = new u32[m_num_ports];
  m_first_block_write           = new bool[m_num_ports];
  m_first_burst_write           = new bool[m_num_ports];
  m_first_rcw                   = new bool[m_num_ports];
  m_last_action_time_stamp      = new sc_time[m_num_ports];
  m_worker_thread_event         = new sc_event[m_num_ports];

  // Squelch SystemC's complaint about multiple worker_thread objects
  sc_actions original_action = sc_report_handler::set_actions("object already exists", SC_WARNING, SC_DO_NOTHING);
  for (u32 i=0; i<m_num_ports; ++i) {
    m_p_active_request_info[i]          = NULL;
    m_p_active_response[i]              = NULL;
    // If this doesn't work for you, change "#if 1" to "#if 0"
#if 1
    ostringstream oss;
    oss << "worker_thread_" << i;
    declare_thread_process(worker_thread_handle, oss.str().c_str(), SC_CURRENT_USER_MODULE, worker_thread);
#else
    SC_THREAD(worker_thread);
#endif
  }
  // Restore SystemC
  sc_report_handler::set_actions("object already exists", SC_WARNING, original_action);

  if (m_p_script_stream != NULL) {
    SC_THREAD(script_thread);
  }

  // Get clock period 
  u32 clock_period = memory_parms.get_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = sc_get_time_resolution() * clock_period;
  }

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
  m_recovery_time       = m_clock_period * memory_parms.get_u32("recovery_time");
  m_response_repeat     = m_clock_period * memory_parms.get_u32("response_repeat");

  m_fast_access_object  = new xtsc_memory_fast_access(*this);

  if (m_deny_fast_access.size()) {
    if (m_deny_fast_access.size() & 0x1) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': " << "\"deny_fast_access\" parameter has " << m_deny_fast_access.size()
          << " elements which is not an even number as it must be.";
      throw xtsc_exception(oss.str());
    }
    for (u32 i=0; i<m_deny_fast_access.size(); i+=2) {
      xtsc_address begin = m_deny_fast_access[i];
      xtsc_address end   = m_deny_fast_access[i+1];
      if ((begin < m_start_address8) || (begin > m_end_address8) ||
          (end   < m_start_address8) || (end   > m_end_address8) ||
          (end   < begin)) {
        ostringstream oss;
        oss << kind() << " '" << name() << "': " << "\"deny_fast_access\" range [0x" << hex << setfill('0') << setw(8) << begin
            << "-0x" << setw(8) << end << "] is invalid." << endl;
        oss << "Valid ranges must fit within [0x" << setw(8) << m_start_address8 << "-0x" << m_end_address8
            << "].";
        throw xtsc_exception(oss.str());
      }
    }
  }

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed " << kind() << " '" << name() << "':");
  XTSC_LOG(m_text, ll,        " num_ports               = "   << m_num_ports);
  XTSC_LOG(m_text, ll, hex << " start_byte_address      = 0x" << m_start_address8);
  XTSC_LOG(m_text, ll, hex << " memory_byte_size        = 0x" << m_size8 << (m_size8 ? " " : " (4GB)"));
  XTSC_LOG(m_text, ll, hex << " End byte address        = 0x" << m_end_address8);
  if (m_width8) {
  XTSC_LOG(m_text, ll,        " byte_width              = "   << m_width8);
  } else {
  XTSC_LOG(m_text, ll,        " byte_width              = 0 => 4|8|16|32|64");
  }
  XTSC_LOG(m_text, ll,        " check_alignment         = "   << m_check_alignment);
  XTSC_LOG(m_text, ll, hex << " page_byte_size          = 0x" << m_p_memory->m_page_size8);
  XTSC_LOG(m_text, ll, hex << " initial_value_file      = "   << m_p_memory->m_initial_value_file);
  XTSC_LOG(m_text, ll, hex << " memory_fill_byte        = 0x" << (u32) m_p_memory->m_memory_fill_byte);
  XTSC_LOG(m_text, ll,        " immediate_timing        = "   << (m_immediate_timing ? "true" : "false"));
  if (!m_immediate_timing) {
  if (clock_period == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, hex << " clock_period            = 0x" << clock_period << " (" << m_clock_period << ")");
  } else {
  XTSC_LOG(m_text, ll,        " clock_period            = "   << clock_period << " (" << m_clock_period << ")");
  }
  XTSC_LOG(m_text, ll,        " request_fifo_depth      = "   << memory_parms.get_u32("request_fifo_depth"));
  XTSC_LOG(m_text, ll,        " delay_from_receipt      = "   << (m_delay_from_receipt ? "true" : "false"));
  XTSC_LOG(m_text, ll,        " read_delay              = "   << memory_parms.get_u32("read_delay"));
  XTSC_LOG(m_text, ll,        " write_delay             = "   << memory_parms.get_u32("write_delay"));
  XTSC_LOG(m_text, ll,        " block_read_delay        = "   << memory_parms.get_u32("block_read_delay"));
  XTSC_LOG(m_text, ll,        " block_read_repeat       = "   << memory_parms.get_u32("block_read_repeat"));
  XTSC_LOG(m_text, ll,        " burst_read_delay        = "   << memory_parms.get_u32("burst_read_delay"));
  XTSC_LOG(m_text, ll,        " burst_read_repeat       = "   << memory_parms.get_u32("burst_read_repeat"));
  XTSC_LOG(m_text, ll,        " block_write_delay       = "   << memory_parms.get_u32("block_write_delay"));
  XTSC_LOG(m_text, ll,        " block_write_repeat      = "   << memory_parms.get_u32("block_write_repeat"));
  XTSC_LOG(m_text, ll,        " block_write_response    = "   << memory_parms.get_u32("block_write_response"));
  XTSC_LOG(m_text, ll,        " burst_write_delay       = "   << memory_parms.get_u32("burst_write_delay"));
  XTSC_LOG(m_text, ll,        " burst_write_repeat      = "   << memory_parms.get_u32("burst_write_repeat"));
  XTSC_LOG(m_text, ll,        " burst_write_response    = "   << memory_parms.get_u32("burst_write_response"));
  XTSC_LOG(m_text, ll,        " rcw_repeat              = "   << memory_parms.get_u32("rcw_repeat"));
  XTSC_LOG(m_text, ll,        " rcw_response            = "   << memory_parms.get_u32("rcw_response"));
  XTSC_LOG(m_text, ll,        " recovery_time           = "   << memory_parms.get_u32("recovery_time"));
  XTSC_LOG(m_text, ll,        " response_repeat         = "   << memory_parms.get_u32("response_repeat"));
  }
  XTSC_LOG(m_text, ll,        " use_fast_access         = "   << boolalpha << m_use_fast_access);
  XTSC_LOG(m_text, ll,        " deny_fast_access       :");
  for (u32 i=0; i<m_deny_fast_access.size(); i+=2) {
  XTSC_LOG(m_text, ll, hex << "  [0x" << setfill('0') << setw(8) << m_deny_fast_access[i] << "-0x"
                                                      << setw(8) << m_deny_fast_access[i+1] << "]");
  }
  XTSC_LOG(m_text, ll,        " use_raw_access          = "   << boolalpha << m_use_raw_access);
  XTSC_LOG(m_text, ll,        " script_file             = "   << m_script_file);
  XTSC_LOG(m_text, ll,        " wraparound              = "   << boolalpha << m_wraparound);
  XTSC_LOG(m_text, ll, hex << " fail_request_mask       = 0x" << m_fail_request_mask);
  if (m_fail_request_mask) {
  XTSC_LOG(m_text, ll,        " fail_percentage         = "   << m_fail_percentage);
  XTSC_LOG(m_text, ll,        " fail_seed               = "   << m_fail_seed);
  XTSC_LOG(m_text, ll,        " fail_status             = "   << xtsc_response::get_status_name(m_fail_status));
  XTSC_LOG(m_text, ll, hex << " RAND_MAX                = 0x" << RAND_MAX);
  }

  reset(true);

}



xtsc_component::xtsc_memory::~xtsc_memory(void) {
  XTSC_DEBUG(m_text, "In ~xtsc_memory()");
}



void xtsc_component::xtsc_memory::reset(bool hard_reset) {
  XTSC_INFO(m_text, kind() << "::reset()");

  m_next_port_num       = 0;
  m_line                = "";
  m_line_count          = 0;
  m_prev_type           = 0xFFFFFFFF;
  m_prev_hit            = false;
  m_prev_port           = 0xFFFFFFFF;

  for (u32 i=0; i<m_num_ports; ++i) {
    m_rcw_have_first_transfer[i]        = false;
    m_block_write_transfer_count[i]     = 0;
    m_burst_write_transfer_count[i]     = 0;
    m_first_block_write[i]              = true;
    m_first_burst_write[i]              = true;
    m_first_rcw[i]                      = true;
    m_last_action_time_stamp[i]         = SC_ZERO_TIME - m_recovery_time;
    if (m_p_active_request_info[i]) {
      delete_request_info(m_p_active_request_info[i]);
    }
    if (m_p_active_response[i]) {
      delete m_p_active_response[i];
      m_p_active_response[i] = NULL;
    }
  }

  for (u32 i=0; i<m_num_ports; ++i) {
    while (m_request_fifo[i]->num_available()) {
      request_info *p_info = m_request_fifo[i]->read();
      delete_request_info(p_info);
    }
  }

  if (m_p_script_stream != NULL) {
    m_p_script_stream->reset();
  }

  if (hard_reset) {
    load_initial_values();
  }

}



void xtsc_component::xtsc_memory::connect(xtsc_arbiter& arbiter, u32 mem_port) {
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  arbiter.m_request_port(*m_request_exports[mem_port]);
  (*m_respond_ports[mem_port])(arbiter.m_respond_export);
}



void xtsc_component::xtsc_memory::connect(xtsc_cohctrl& cohctrl, u32 mem_port) {
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  cohctrl.m_request_port(*m_request_exports[mem_port]);
  (*m_respond_ports[mem_port])(cohctrl.m_respond_export);
}



u32 xtsc_component::xtsc_memory::connect(xtsc_core& core, const char *memory_port_name, u32 mem_port, bool single_connect) {
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  u32 width8 = core.get_memory_byte_width(xtsc_core::get_memory_port(memory_port_name));
  if (m_width8 && (width8 != m_width8)) {
    ostringstream oss;
    oss << "Memory interface data width mismatch: " << endl;
    oss << kind() << " '" << name() << "' is " << m_width8 << " bytes wide, but '" << memory_port_name
        << "' interface of" << endl;
    oss << "xtsc_core '" << core.name() << "' is " << width8 << " bytes wide.";
    throw xtsc_exception(oss.str());
  }
  core.get_request_port(memory_port_name)(*m_request_exports[mem_port]);
  (*m_respond_ports[mem_port])(core.get_respond_export(memory_port_name));
  u32 num_connected = 1;
  // Should we connect a 2nd port?
  xtsc_core::memory_port memory_port = xtsc_core::get_memory_port(memory_port_name);
  if (!single_connect && (mem_port+1 < m_num_ports) && core.is_dual_ported(xtsc_core::is_xlmi(memory_port))) {
    if (xtsc_core::is_ls_dual_port(memory_port, 0)) {
      // Don't connect if 2nd port of xtsc_memory has already been connected.
      if (!m_request_impl[mem_port+1]->is_connected()) {
        const char *dual_name = xtsc_core::get_memory_port_name(memory_port+1);
        sc_port<xtsc_request_if, NSPP>& dual_port = core.get_request_port(dual_name);
        // Don't connect if 2nd port of xtsc_core has already been connected.
        // This test is not reliable so we'll catch any errors and carry on (which
        // works for the OSCI simulator--but maybe not for others).
        if (!dual_port.get_interface()) {
          try {
            dual_port(*m_request_exports[mem_port+1]);
            (*m_respond_ports[mem_port+1])(core.get_respond_export(dual_name));
            num_connected += 1;
          } catch (...) {
            XTSC_INFO(m_text, "Core '" << core.name() << "' 2nd LD/ST memory port '" << dual_name << "' is already bond.");
          }
        }
      }
    }
  }
  return num_connected;
}



void xtsc_component::xtsc_memory::connect(xtsc_dma_engine& dma, u32 mem_port) {
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  dma.m_request_port(*m_request_exports[mem_port]);
  (*m_respond_ports[mem_port])(dma.m_respond_export);
}



void xtsc_component::xtsc_memory::connect(xtsc_master& master, u32 mem_port) {
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  master.m_request_port(*m_request_exports[mem_port]);
  (*m_respond_ports[mem_port])(master.m_respond_export);
}



u32 xtsc_component::xtsc_memory::connect(xtsc_memory_trace& memory_trace, u32 trace_port, u32 mem_port, bool single_connect) {
  u32 trace_ports = memory_trace.get_num_ports();
  if (trace_port >= trace_ports) {
    ostringstream oss;
    oss << "Invalid trace_port=" << trace_port << " in connect(): " << endl;
    oss << memory_trace.kind() << " '" << memory_trace.name() << "' has " << trace_ports << " ports numbered from 0 to "
        << trace_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }

  u32 num_connected = 0;

  while ((trace_port < trace_ports) && (mem_port < m_num_ports)) {

    (*memory_trace.m_request_ports[trace_port])(*m_request_exports[mem_port]);
    (*m_respond_ports[mem_port])(*memory_trace.m_respond_exports[trace_port]);

    mem_port += 1;
    trace_port += 1;
    num_connected += 1;

    if (single_connect) break;
    if (trace_port >= trace_ports) break;
    if (mem_port >= m_num_ports) break;
  }

  return num_connected;
}



void xtsc_component::xtsc_memory::connect(xtsc_router& router, u32 router_port, u32 mem_port) {
  u32 num_slaves = router.get_num_slaves();
  if (router_port >= num_slaves) {
    ostringstream oss;
    oss << "Invalid router_port=" << router_port << " in connect(): " << endl;
    oss << router.kind() << " '" << router.name() << "' has " << num_slaves << " ports numbered from 0 to " << num_slaves-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  (*router.m_request_ports[router_port])(*m_request_exports[mem_port]);
  (*m_respond_ports[mem_port])(*router.m_respond_exports[router_port]);
}



u32 xtsc_component::xtsc_memory::connect(xtsc_pin2tlm_memory_transactor& pin2tlm, u32 tran_port, u32 mem_port, bool single_connect) {
  u32 tran_ports = pin2tlm.get_num_ports();
  if (tran_port >= tran_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << pin2tlm.kind() << " '" << pin2tlm.name() << "' has " << tran_ports << " ports numbered from 0 to " << tran_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (mem_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid mem_port=" << mem_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }

  u32 num_connected = 0;

  while ((tran_port < tran_ports) && (mem_port < m_num_ports)) {

    (*pin2tlm.m_request_ports[tran_port])(*m_request_exports[mem_port]);
    (*m_respond_ports[mem_port])(*pin2tlm.m_respond_exports[tran_port]);

    mem_port += 1;
    tran_port += 1;
    num_connected += 1;

    if (single_connect) break;
    if (tran_port >= tran_ports) break;
    if (mem_port >= m_num_ports) break;
  }

  return num_connected;

}



void xtsc_component::xtsc_memory::setup_false_error_responses(xtsc_response::status_t status, u32 request_mask, u32 fail_percentage) {
  m_fail_status         = status;
  m_fail_request_mask   = request_mask;
  m_fail_percentage     = fail_percentage;
  compute_let_through();
}



void xtsc_component::xtsc_memory::compute_let_through() {
  if (m_fail_percentage > 100) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "fail_percentage of " << m_fail_percentage << " is out of range (1-100).";
    throw xtsc_exception(oss.str());
  }
  u64 max = static_cast<u64>(RAND_MAX);
  m_let_through = static_cast<u32>(max - ((max * m_fail_percentage) / 100));
  XTSC_DEBUG(m_text, "m_fail_percentage=" << m_fail_percentage << " => m_let_through=0x" << hex << m_let_through);
}



xtsc_component::xtsc_memory::request_type_t xtsc_component::xtsc_memory::get_request_type(const xtsc_request& request, u32 port_num) {
  switch (request.get_type()) {
    case xtsc_request::READ:        return REQ_READ;
    case xtsc_request::BLOCK_READ:  return REQ_BLOCK_READ;
    case xtsc_request::BURST_READ:  return REQ_BURST_READ;
    case xtsc_request::WRITE:       return REQ_WRITE;
    case xtsc_request::BLOCK_WRITE: return static_cast<request_type_t>(REQ_BLOCK_WRITE_1 << m_block_write_transfer_count[port_num]);
    case xtsc_request::BURST_WRITE: return static_cast<request_type_t>(REQ_BURST_WRITE_1 << m_burst_write_transfer_count[port_num]);
    case xtsc_request::RCW:         return static_cast<request_type_t>(REQ_RCW_1 << (request.get_last_transfer() ? 1 : 0));
    default: {
      ostringstream oss;
      oss << kind() << " '" << name() << "': ";
      oss << "Invalid xtsc_request::type_t in xtsc_memory::get_request_type().";
      throw xtsc_exception(oss.str());
    }
  }
}



bool xtsc_component::xtsc_memory::do_nacc_failure(const xtsc_request& request, u32 port_num) {
  return ((m_fail_request_mask)                                         &&
          (get_request_type(request, port_num) & m_fail_request_mask)   &&
          (m_fail_status == xtsc_response::RSP_NACC)                    &&
          (rand() >= m_let_through));
}



xtsc_response::status_t xtsc_component::xtsc_memory::get_status_for_testing_failures(request_info* p_request_info, u32 port_num) {
  if (p_request_info->m_status != xtsc_response::RSP_OK) {
    return p_request_info->m_status;
  }
  return ((m_fail_request_mask)                                                         &&
          (get_request_type(p_request_info->m_request, port_num) & m_fail_request_mask) &&
          (m_fail_status != xtsc_response::RSP_NACC)                                    &&
          (rand() >= m_let_through)
         ) ? m_fail_status : xtsc_response::RSP_OK;
}



void xtsc_component::xtsc_memory::worker_thread(void) {

  // Get the port number for this "instance" of worker_thread
  u32 port_num = m_next_port_num++;

  try {

    while (true) {
      wait(m_worker_thread_event[port_num]);
      while (m_request_fifo[port_num]->num_available()) {
        bool delta_delay = false;
        // Accept this one as our current transaction
        m_request_fifo[port_num]->nb_read(m_p_active_request_info[port_num]);
        XTSC_DEBUG(m_text, "worker_thread() got: " << m_p_active_request_info[port_num]->m_request);
        // Determine delay
        sc_time delay = SC_ZERO_TIME;
        switch (m_p_active_request_info[port_num]->m_request.get_type()) {
          case xtsc_request::READ:         
            delay  = m_read_delay;
            break;
          case xtsc_request::BLOCK_READ:   
            delay  = m_block_read_delay;
            break;
          case xtsc_request::BURST_READ:   
            delay  = m_burst_read_delay;
            break;
          case xtsc_request::RCW:          
            if (m_read_only) {
              ostringstream oss;
              oss << "Read-only xtsc_memory '" << name() << "' doesn't support transaction: "
                  << m_p_active_request_info[port_num]->m_request;
              throw xtsc_exception(oss.str());
            }
            delay  = (m_first_rcw[port_num] ?  m_rcw_repeat : m_rcw_response);
            m_first_rcw[port_num] = m_p_active_request_info[port_num]->m_request.get_last_transfer();
            break;
          case xtsc_request::WRITE:        
            if (m_read_only) {
              ostringstream oss;
              oss << "Read-only xtsc_memory '" << name() << "' doesn't support transaction: "
                  << m_p_active_request_info[port_num]->m_request;
              throw xtsc_exception(oss.str());
            }
            // To model the dual load/store unit requirement that a simultaneous read and write to
            // the same address must return the old data for the read (NOT the new data being written
            // by the write), we delay WRITE transactions for a delta cycle when we are multi-ported.
            delta_delay = (m_num_ports > 1);
            delay  = m_write_delay;
            break;
          case xtsc_request::BLOCK_WRITE:  {
            if (m_read_only) {
              ostringstream oss;
              oss << "Read-only xtsc_memory '" << name() << "' doesn't support transaction: "
                  << m_p_active_request_info[port_num]->m_request;
              throw xtsc_exception(oss.str());
            }
            bool last = m_p_active_request_info[port_num]->m_request.get_last_transfer();
            delay  = (m_first_block_write[port_num] ?  m_block_write_delay : (last ? m_block_write_response : m_block_write_repeat));
            m_first_block_write[port_num] = last;
            break;
          }
          case xtsc_request::BURST_WRITE:  {
            if (m_read_only) {
              ostringstream oss;
              oss << "Read-only xtsc_memory '" << name() << "' doesn't support transaction: "
                  << m_p_active_request_info[port_num]->m_request;
              throw xtsc_exception(oss.str());
            }
            bool last = m_p_active_request_info[port_num]->m_request.get_last_transfer();
            delay  = (m_first_burst_write[port_num] ?  m_burst_write_delay : (last ? m_burst_write_response : m_burst_write_repeat));
            m_first_burst_write[port_num] = last;
            break;
          }
          default: {
            ostringstream oss;
            oss << kind() << " '" << name() << "': ";
            oss << "Invalid xtsc_request::type_t in nb_request().";
            throw xtsc_exception(oss.str());
          }
        }
        if (m_delay_from_receipt) {
          // net => No Earlier Than time
          sc_time receipt_net     = m_p_active_request_info[port_num]->m_time_stamp + delay;
          sc_time last_action_net = m_last_action_time_stamp[port_num] + m_recovery_time;
          sc_time latest_net      = (receipt_net > last_action_net) ? receipt_net : last_action_net;
          sc_time now             = sc_time_stamp();
          delay = (latest_net <= now) ? SC_ZERO_TIME : (latest_net - now);
        }
        XTSC_DEBUG(m_text, "worker_thread() doing wait for " << delay);
        wait(delay);
        if (delta_delay) {
          wait(SC_ZERO_TIME);
        }
        do_active_request(port_num);
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in SC_THREAD of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, log4xtensa::FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



void xtsc_component::xtsc_memory::do_active_request(u32 port_num) {
  bool drop_on_floor = false;
  m_p_active_response[port_num] = new xtsc_response(m_p_active_request_info[port_num]->m_request);
  xtsc_response::status_t status = get_status_for_testing_failures(m_p_active_request_info[port_num], port_num);
  if (m_check_alignment) {
    xtsc_request *p_request = &m_p_active_request_info[port_num]->m_request;
    u32 size8 = p_request->get_byte_size();
    bool aligned = ((p_request->get_byte_address() % size8) == 0);
    bool powerof2 = (size8==1 || size8==2 || size8==4 || size8==8 || size8==16 || size8==32 || size8==64);
    if (!aligned || !powerof2) {
      if (p_request->get_last_transfer()) {
        status = xtsc_response::RSP_ADDRESS_ERROR;
        XTSC_DEBUG(m_text, *p_request << " (RSP_ADDRESS_ERROR: size8=" << size8 << " powerof2=" << boolalpha <<
                           powerof2 << " aligned=" << aligned << ")");
      }
      else {
        drop_on_floor = true;
      }
    }
  }
  if (drop_on_floor) {
    XTSC_INFO(m_text, "Dropping RSP_ADDRESS_ERROR: " << m_p_active_request_info[port_num]->m_request);
  }
  else if (status != xtsc_response::RSP_OK) {
    m_p_active_response[port_num]->set_status(status);
    send_response(port_num, false);
  }
  else {
    switch (m_p_active_request_info[port_num]->m_request.get_type()) {
      case xtsc_request::READ: {
        do_read(port_num);
        break;
      }
      case xtsc_request::BLOCK_READ: {
        do_block_read(port_num);
        break;
      }
      case xtsc_request::BURST_READ: {
        do_burst_read(port_num);
        break;
      }
      case xtsc_request::RCW: {
        do_rcw(port_num);
        break;
      }
      case xtsc_request::WRITE: {
        do_write(port_num);
        break;
      }
      case xtsc_request::BLOCK_WRITE: {
        do_block_write(port_num);
        break;
      }
      case xtsc_request::BURST_WRITE: {
        do_burst_write(port_num);
        break;
      }
      default: {
        ostringstream oss;
        oss << kind() << " '" << name() << "': ";
        oss << "Program Bug: Unrecognized xtsc_request::type_t in xtsc_memory::worker_thread.";
        throw xtsc_exception(oss.str());
      }
    }
  }
  m_last_action_time_stamp[port_num] = sc_time_stamp();
  delete_request_info(m_p_active_request_info[port_num]);
  delete m_p_active_response[port_num];
  m_p_active_response[port_num] = NULL;
}



void xtsc_component::xtsc_memory::do_read(u32 port_num) {
  xtsc_request       *p_request     = &m_p_active_request_info[port_num]->m_request;
  xtsc_address        address8      = p_request->get_byte_address();
  u32                 size8         = p_request->get_byte_size();
  u8                 *buffer        = m_p_active_response[port_num]->get_buffer();
  if (m_width8 && (size8 > m_width8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received READ tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds \"byte_width\" of "
        << m_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received READ tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds xtsc_max_bus_width8 of "
        << xtsc_max_bus_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  u32 page = get_page(address8);
  u32 mem_offset  = get_page_offset(address8);
  u32 buf_offset  = 0;
  for (u32 i = 0; i<size8; ++i) {
    buffer[buf_offset] = *(m_p_memory->m_page_table[page]+mem_offset);
    mem_offset += 1;
    buf_offset += 1;
  }
  if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
    u32 mem_offset  = get_page_offset(address8);
    ostringstream oss;
    xtsc_hex_dump(size8, (m_p_memory->m_page_table[page]+mem_offset), oss);
    XTSC_DEBUG(m_text, m_p_active_request_info[port_num]->m_request << "= " << oss.str());
  }
  send_response(port_num, m_p_memory->m_log_data_binary);
}



void xtsc_component::xtsc_memory::do_block_read(u32 port_num) {
  xtsc_request *p_request       = &m_p_active_request_info[port_num]->m_request;
  u8           *buffer          = m_p_active_response[port_num]->get_buffer();
  u32           num_transfers   = p_request->get_num_transfers();
  u32           size8           = p_request->get_byte_size();
  u32           total_size8     = size8 * num_transfers;
  if (m_width8 && (size8 != m_width8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BLOCK_READ tag=" << p_request->get_tag() << " with byte size of " << size8
        << " (contradicts \"byte_width\" of " << m_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BLOCK_READ tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds xtsc_max_bus_width8 of "
        << xtsc_max_bus_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  for (u32 transfer_count = 0; transfer_count < num_transfers; ++transfer_count) {
    if (!m_immediate_timing && transfer_count) {
      wait(m_block_read_repeat);
    }
    bool last_transfer   = ((transfer_count+1) == num_transfers);
    m_p_active_response[port_num]->set_last_transfer(last_transfer);
    // Adjust address for this transfer.  Include wrap-around for Critical Word First support.
    xtsc_address address8 = p_request->get_byte_address();
    address8 =  (address8 & ~(total_size8-1)) | (((address8 & (total_size8-1)) + transfer_count * size8) % total_size8);
    XTSC_DEBUG(m_text, "in do_block_read for tag=" << p_request->get_tag() << " xfer #" << 
                       (transfer_count+1) << "/" << num_transfers);
    u32 page = get_page(address8);
    u32 mem_offset  = get_page_offset(address8);
    u32 buf_offset  = 0;
    for (u32 i = 0; i<size8; ++i) {
      buffer[buf_offset] = *(m_p_memory->m_page_table[page]+mem_offset);
      mem_offset += 1;
      buf_offset += 1;
    }
    if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
      u32 mem_offset  = get_page_offset(address8);
      ostringstream oss;
      xtsc_hex_dump(size8, (m_p_memory->m_page_table[page]+mem_offset), oss);
      XTSC_DEBUG(m_text, "BLOCK_READ" << (last_transfer ? "*" : " ") << " tag=" << m_p_active_response[port_num]->get_tag() <<
                         " [0x" << hex << setfill('0') << setw(8) << address8 << setfill(' ') << "]= " << oss.str());
    }
    send_response(port_num, m_p_memory->m_log_data_binary);
  }
}



void xtsc_component::xtsc_memory::do_burst_read(u32 port_num) {
  xtsc_request *p_request       = &m_p_active_request_info[port_num]->m_request;
  u8           *buffer          = m_p_active_response[port_num]->get_buffer();
  u32           num_transfers   = p_request->get_num_transfers();
  u32           size8           = p_request->get_byte_size();
  if (m_width8 && (size8 != m_width8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BURST_READ tag=" << p_request->get_tag() << " with byte size of " << size8
        << " (contradicts \"byte_width\" of " << m_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BURST_READ tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds xtsc_max_bus_width8 of "
        << xtsc_max_bus_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  for (u32 transfer_count = 0; transfer_count < num_transfers; ++transfer_count) {
    if (!m_immediate_timing && transfer_count) {
      wait(m_burst_read_repeat);
    }
    bool last_transfer   = ((transfer_count+1) == num_transfers);
    m_p_active_response[port_num]->set_last_transfer(last_transfer);
    // Adjust address for this transfer.
    xtsc_address address8 = p_request->get_byte_address();
    address8 =  address8 + transfer_count * size8;
    XTSC_DEBUG(m_text, "in do_burst_read for tag=" << p_request->get_tag() << " xfer #" << 
                       (transfer_count+1) << "/" << num_transfers);
    u32 page = get_page(address8);
    u32 mem_offset  = get_page_offset(address8);
    u32 buf_offset  = 0;
    for (u32 i = 0; i<size8; ++i) {
      buffer[buf_offset] = *(m_p_memory->m_page_table[page]+mem_offset);
      mem_offset += 1;
      buf_offset += 1;
    }
    if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
      u32 mem_offset  = get_page_offset(address8);
      ostringstream oss;
      xtsc_hex_dump(size8, (m_p_memory->m_page_table[page]+mem_offset), oss);
      XTSC_DEBUG(m_text, "BURST_READ" << (last_transfer ? "*" : " ") << " tag=" << m_p_active_response[port_num]->get_tag() <<
                         " [0x" << hex << setfill('0') << setw(8) << address8 << setfill(' ') << "]= " << oss.str());
    }
    send_response(port_num, m_p_memory->m_log_data_binary);
  }
}



void xtsc_component::xtsc_memory::do_rcw(u32 port_num) {
  xtsc_request *p_request = &m_p_active_request_info[port_num]->m_request;
  xtsc_address  address8  = p_request->get_byte_address();
  u8           *buffer    = p_request->get_buffer();
  u32           page      = get_page(address8);
  u32           size8     = p_request->get_byte_size();
  if (!m_rcw_have_first_transfer[port_num]) {
    m_rcw_have_first_transfer[port_num] = true;
    memcpy(&m_rcw_compare_data[port_num*m_width8], buffer, size8);
    if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
      ostringstream oss;
      xtsc_hex_dump(size8, buffer, oss);
      XTSC_DEBUG(m_text, "RCW  tag=" << p_request->get_tag() << " Compare address 0x" << hex << 
                         setfill('0') << setw(8) << address8 << setfill(' ') << " for bytes: " << oss.str());
    }
  }
  else {
    xtsc_byte_enables byte_enables = p_request->get_byte_enables();
    u32 mem_offset  = get_page_offset(address8);
    u8 *response_buffer = m_p_active_response[port_num]->get_buffer();
    memcpy(response_buffer, m_p_memory->m_page_table[page]+mem_offset, size8);
    bool compare_data_matches = true;
    xtsc_byte_enables bytes = byte_enables;
    for (u32 i = 0; i<size8; ++i) {
      if (bytes & 0x1) {
        if (m_rcw_compare_data[port_num*m_width8+i] != *(m_p_memory->m_page_table[page]+mem_offset)) {
          compare_data_matches = false;
          break;
        }
      }
      bytes >>= 1;
      mem_offset += 1;
    }
    if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
      mem_offset  = get_page_offset(address8);
      ostringstream oss1;
      xtsc_hex_dump(size8, m_p_memory->m_page_table[page]+mem_offset, oss1);
      if (compare_data_matches) {
        ostringstream oss2;
        xtsc_hex_dump(size8, buffer, oss2);
        XTSC_DEBUG(m_text, "RCW* tag=" << p_request->get_tag() << " bytes " << oss1.str() <<
                           " replaced with " << oss2.str() << " using byte_enables=0x" << hex << byte_enables);
      }
      else {
        XTSC_DEBUG(m_text, "RCW* tag=" << p_request->get_tag() << " bytes " << oss1.str() <<
                           " not replaced using byte_enables=0x" << hex << byte_enables);
      }
    }
    bytes = byte_enables;
    if (compare_data_matches) {
      mem_offset  = get_page_offset(address8);
      for (u32 i = 0; i<size8; ++i) {
        if (bytes & 0x1) {
          *(m_p_memory->m_page_table[page]+mem_offset) = buffer[i];
        }
        bytes >>= 1;
        mem_offset += 1;
      }
    }
    m_rcw_have_first_transfer[port_num] = false;
    send_response(port_num, m_p_memory->m_log_data_binary);
  }
}



void xtsc_component::xtsc_memory::do_write(u32 port_num) {
  xtsc_request       *p_request     = &m_p_active_request_info[port_num]->m_request;
  xtsc_address        address8      = p_request->get_byte_address();
  u32                 size8         = p_request->get_byte_size();
  const u8           *buffer        = p_request->get_buffer();
  xtsc_byte_enables   byte_enables  = p_request->get_byte_enables();
  u32                 page          = get_page(address8);
  xtsc_address        addr8         = address8 & (-1 ^ (size8-1));
  if (m_width8 && (size8 > m_width8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received WRITE tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds \"byte_width\" of "
        << m_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received WRITE tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds xtsc_max_bus_width8 of "
        << xtsc_max_bus_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  XTSC_DEBUG(m_text, "do_write address8=0x" << hex << address8 << " byte_enables=0x" << byte_enables <<
                     " addr8=0x" << addr8 << " size8=" << dec << size8);
  u32 mem_offset  = get_page_offset(addr8);
  u32 buf_offset  = 0;
  xtsc_byte_enables bytes = byte_enables;
  for (u32 i = 0; i<size8; ++i) {
    if (bytes & 0x1) {
      *(m_p_memory->m_page_table[page]+mem_offset) = buffer[buf_offset];
    }
    bytes >>= 1;
    mem_offset += 1;
    buf_offset += 1;
  }
  XTSC_DEBUG(m_text, m_p_active_request_info[port_num]->m_request);
  send_response(port_num, false);
}



void xtsc_component::xtsc_memory::do_block_write(u32 port_num) {
  xtsc_request *p_request       = &m_p_active_request_info[port_num]->m_request;
  xtsc_address  address8        = p_request->get_hardware_address();
  const u8     *buffer          = p_request->get_buffer();
  bool          last_transfer   = p_request->get_last_transfer();
  u32           page            = get_page(address8);
  u32           size8           = p_request->get_byte_size();
  xtsc_address  addr8           = address8 + (m_block_write_transfer_count[port_num] * size8);
  u32           mem_offset      = get_page_offset(addr8);
  u32           buf_offset      = 0;
  XTSC_DEBUG(m_text, "in do_block_write for tag=" << p_request->get_tag() << " xfer #" << 
                     (m_block_write_transfer_count[port_num]+1) << "/" << p_request->get_num_transfers());
  XTSC_DEBUG(m_text, "do_block_write address8=0x" << hex << address8 << " addr8=0x" << addr8);
  if (m_width8 && (size8 != m_width8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BLOCK_WRITE tag=" << p_request->get_tag() << " with byte size of " << size8
        << " (contradicts \"byte_width\" of " << m_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BLOCK_WRITE tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds xtsc_max_bus_width8 of "
        << xtsc_max_bus_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  for (u32 i = 0; i<size8; ++i) {
    *(m_p_memory->m_page_table[page]+mem_offset) = buffer[buf_offset];
    mem_offset += 1;
    buf_offset += 1;
  }
  if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
    u32 mem_offset  = get_page_offset(addr8);
    ostringstream oss;
    xtsc_hex_dump(size8, (m_p_memory->m_page_table[page]+mem_offset), oss);
    XTSC_DEBUG(m_text, "BLOCK_WRITE" << (last_transfer ? "*" : " ") << " tag=" << p_request->get_tag() <<
                      " [0x" << hex << setfill('0') << setw(8) << addr8 << setfill(' ') << "/" << size8 << "]= " << oss.str());
  }
  m_block_write_transfer_count[port_num] += 1;
  if (last_transfer) {
    if (m_block_write_transfer_count[port_num] != p_request->get_num_transfers()) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': ";
      oss << "Received BLOCK_WRITE tag=" << p_request->get_tag() << " with last_transfer flag set but only "
          << m_block_write_transfer_count[port_num] << " transfers out of an expected total of " << p_request->get_num_transfers()
          << " have occurred.";
      throw xtsc_exception(oss.str());
    }
    send_response(port_num, false);
    m_block_write_transfer_count[port_num] = 0;
  }
  else {
    if (m_block_write_transfer_count[port_num] == p_request->get_num_transfers()) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': ";
      oss << "Received BLOCK_WRITE tag=" << p_request->get_tag() << " with last_transfer flag NOT set but all "
          << m_block_write_transfer_count[port_num] << " transfers have occurred.";
      throw xtsc_exception(oss.str());
    }
  }
}



void xtsc_component::xtsc_memory::do_burst_write(u32 port_num) {
  xtsc_request         *p_request       = &m_p_active_request_info[port_num]->m_request;
  xtsc_address          address8        = p_request->get_byte_address();
  const u8             *buffer          = p_request->get_buffer();
  xtsc_byte_enables     byte_enables    = p_request->get_byte_enables();
  bool                  last_transfer   = p_request->get_last_transfer();
  u32                   page            = get_page(address8);
  u32                   size8           = p_request->get_byte_size();
  u32                   mem_offset      = get_page_offset(address8);
  u32                   buf_offset      = 0;
  XTSC_DEBUG(m_text, "in do_burst_write for tag=" << p_request->get_tag() << " xfer #" << 
                     (m_burst_write_transfer_count[port_num]+1) << "/" << p_request->get_num_transfers());
  XTSC_DEBUG(m_text, "do_burst_write address8=0x" << hex << address8 << " byte_enables=0x" << byte_enables);
  if (m_width8 && (size8 > m_width8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BURST_WRITE tag=" << p_request->get_tag() << " with byte size of " << size8
        << " (contradicts \"byte_width\" of " << m_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': ";
    oss << "Received BURST_WRITE tag=" << p_request->get_tag() << " with byte size of " << size8 << " (exceeds xtsc_max_bus_width8 of "
        << xtsc_max_bus_width8 << ")";
    throw xtsc_exception(oss.str());
  }
  xtsc_byte_enables  bytes = byte_enables;
  u32  first_bit = byte_enables & 0x1;
  bool other_bit_found = false;
  bool illegal_byte_enables = false;
  for (u32 i = 0; i<size8; ++i) {
    if (bytes & 0x1) {
      *(m_p_memory->m_page_table[page]+mem_offset) = buffer[buf_offset];
    }
    if (((bytes & 0x1) != first_bit)) {
      other_bit_found = true;
    }
    else if (other_bit_found) {
      illegal_byte_enables = true;
    }
    bytes >>= 1;
    mem_offset += 1;
    buf_offset += 1;

  }
  bool first_transfer = (p_request->get_transfer_number() == 1);
  if ((byte_enables == 0) || (last_transfer && (first_bit == 0)) || (first_transfer && (first_bit == 1) && other_bit_found)) {
    illegal_byte_enables = true;
  }
  if (illegal_byte_enables) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': " << "Illegal byte enables: " << *p_request;
    throw xtsc_exception(oss.str());
  }
  if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(DEBUG_LOG_LEVEL)) {
    u32 mem_offset  = get_page_offset(address8);
    ostringstream oss;
    xtsc_hex_dump(size8, (m_p_memory->m_page_table[page]+mem_offset), oss);
    XTSC_DEBUG(m_text, "BURST_WRITE" << (last_transfer ? "*" : " ") << " tag=" << p_request->get_tag() <<
                      " [0x" << hex << setfill('0') << setw(8) << address8 << setfill(' ') << "/" << size8 << "]= " << oss.str());
  }
  m_burst_write_transfer_count[port_num] += 1;
  if (last_transfer) {
    if (m_burst_write_transfer_count[port_num] != p_request->get_num_transfers()) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': ";
      oss << "Received BURST_WRITE tag=" << p_request->get_tag() << " with last_transfer flag set but only "
          << m_burst_write_transfer_count[port_num] << " transfers out of an expected total of " << p_request->get_num_transfers()
          << " have occurred.";
      throw xtsc_exception(oss.str());
    }
    send_response(port_num, false);
    m_burst_write_transfer_count[port_num] = 0;
  }
  else {
    if (m_burst_write_transfer_count[port_num] == p_request->get_num_transfers()) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': ";
      oss << "Received BURST_WRITE tag=" << p_request->get_tag() << " with last_transfer flag NOT set but all "
          << m_burst_write_transfer_count[port_num] << " transfers have occurred.";
      throw xtsc_exception(oss.str());
    }
  }
}



void xtsc_component::xtsc_memory::send_response(u32 port_num, bool log_data_binary) {
  u32 tries = 0;
  while (true) {
    tries += 1;
    XTSC_INFO(m_text, *m_p_active_response[port_num] << " Port #" << port_num << " Try #" << tries);
    xtsc_log_memory_response_event(m_binary, INFO_LOG_LEVEL, false, 0, log_data_binary, *m_p_active_response[port_num]);
    if ((*m_respond_ports[port_num])->nb_respond(*m_p_active_response[port_num])) {
      break;
    }
    if (m_immediate_timing) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': ";
      oss << "nb_respond() returned false which is not allowed when \"immediate_timing\" is true: " << *m_p_active_response[port_num];
      throw xtsc_exception(oss.str());
    }
    wait(m_response_repeat);
  }
}



xtsc_component::xtsc_memory::request_info *xtsc_component::xtsc_memory::new_request_info(const xtsc_request&     request,
                                                                                         xtsc_response::status_t status)
{
  if (m_request_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new request_info");
    return new request_info(request, status);
  }
  else {
    request_info *p_request_info = m_request_pool.back();
    m_request_pool.pop_back();
    p_request_info->init(request, status);
    return p_request_info;
  }
}



void xtsc_component::xtsc_memory::delete_request_info(request_info*& p_request_info) {
  m_request_pool.push_back(p_request_info);
  p_request_info = 0;
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  m_memory.peek(address8, size8, buffer);
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  m_memory.poke(address8, size8, buffer);
}



bool xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_fast_access(xtsc_fast_access_request &request) {
  xtsc_address address8    = request.get_translated_request_address();
  xtsc_address page_start8 = address8 & ~(m_memory.m_p_memory->m_page_size8 - 1);
  xtsc_address page_end8   = page_start8 + m_memory.m_p_memory->m_page_size8 - 1;

  if ((address8 < m_memory.m_start_address8) || (address8 > m_memory.m_end_address8)) {
    ostringstream oss;
    oss << "Memory access out-of-range (address=0x" << hex << address8 << ") in nb_fast_access() of memory '"
        << m_memory.name() << "' (Valid range: 0x" << m_memory.m_start_address8 << "-0x"
        << m_memory.m_end_address8 << ").";
    throw xtsc_exception(oss.str());
  }

  // Allow any fast access?
  if (!m_memory.m_use_fast_access) {
    request.deny_access();
    XTSC_INFO(m_memory.m_text, hex << setfill('0') << "nb_fast_access: address8=0x" << address8 <<
                               " page=[0x" << page_start8 << "-0x" << page_end8 << "] deny_access");
    return true;
  }
  
  string access("raw access");
  if (m_memory.m_use_raw_access) {
    u32 page = m_memory.get_page(address8);
    u8 *raw_bytes = m_memory.m_p_memory->m_page_table[page];
    request.allow_raw_access(page_start8, (u32*)raw_bytes, m_memory.m_p_memory->m_page_size8, 0);
    xtsc_fast_access_block page_block(address8, page_start8, page_end8);
    request.restrict_to_block(page_block);
  }
  else {
    request.allow_interface_access(m_memory.get_fast_access_object());
    access = "interface access";
  }
  
  for (u32 i=0; i<m_memory.m_deny_fast_access.size(); i+=2) {
    xtsc_address begin = m_memory.m_deny_fast_access[i];
    xtsc_address end   = m_memory.m_deny_fast_access[i+1];
    if (((begin >= page_start8) && (begin <= page_end8)) ||
        ((end   >= page_start8) && (end   <= page_end8)) ||
        ((begin <  page_start8) && (end   >  page_end8)))
    {
      if ((address8 >= begin) && (address8 <= end)) {
        // Deny fast access
        request.deny_access();
        xtsc_fast_access_block deny_block(address8, begin, end);
        request.restrict_to_block(deny_block);
        access = "deny access";
        break;
      }
      request.remove_address_range(address8, begin, end);
      XTSC_DEBUG(m_memory.m_text, hex << setfill('0') << "nb_fast_access: address8=0x" << address8 <<
                                 " page=[0x" << page_start8 << "-0x" << page_end8 << "]" <<
                                 " remove=[0x" << begin << "-0x" << end << "]");
    }
  }

  if (xtsc_is_text_logging_enabled() && m_memory.m_text.isEnabledFor(VERBOSE_LOG_LEVEL)) {
    xtsc_fast_access_block block = request.get_local_block(address8);
    XTSC_VERBOSE(m_memory.m_text, hex << setfill('0') << "nb_fast_access: address8=0x" << address8 << " block=[0x" <<
                                  block.get_block_address() << "-0x" << block.get_block_end_address() << "] " << access);
  }

  return true;
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_request(const xtsc_request& request) {
  XTSC_INFO(m_memory.m_text, request << " Port #" << m_port_num);
  xtsc_log_memory_request_event(m_memory.m_binary, INFO_LOG_LEVEL, true, 0, m_memory.m_p_memory->m_log_data_binary, request);
  // Check if we should NACC this request for testing purposes (non-script based)
  if (m_memory.do_nacc_failure(request, m_port_num)) {
    xtsc_response response(request, xtsc_response::RSP_NACC);
    XTSC_INFO(m_memory.m_text, response << " (Test RSP_NACC Port #" << m_port_num << ")");
    xtsc_log_memory_response_event(m_memory.m_binary, INFO_LOG_LEVEL, false, 0, false, response);
    (*m_memory.m_respond_ports[m_port_num])->nb_respond(response);
    return;
  }
  // Check if we should NACC this request because we don't have room for it
  if (!m_memory.m_immediate_timing && (m_memory.m_request_fifo[m_port_num]->num_free() == 0)) {
    xtsc_response response(request, xtsc_response::RSP_NACC);
    XTSC_INFO(m_memory.m_text, response << " (Request FIFO full Port #" << m_port_num << ")");
    xtsc_log_memory_response_event(m_memory.m_binary, INFO_LOG_LEVEL, false, 0, false, response);
    (*m_memory.m_respond_ports[m_port_num])->nb_respond(response);
    return;
  }
  // Do special handling based on "script_file"
  xtsc_response::status_t status = xtsc_response::RSP_OK;
  if (m_memory.m_p_script_stream != NULL) {
    m_memory.m_prev_port = m_port_num;
    m_memory.m_prev_hit  = m_memory.compute_special_response(request, m_port_num, status, m_memory.m_prev_type);
    m_memory.m_script_thread_event.notify();
    if (m_memory.m_prev_hit && (status == xtsc_response::RSP_NACC)) {
      xtsc_response response(request, xtsc_response::RSP_NACC);
      XTSC_INFO(m_memory.m_text, response << " (script_file directed)");
      xtsc_log_memory_response_event(m_memory.m_binary, INFO_LOG_LEVEL, false, 0, false, response);
      (*m_memory.m_respond_ports[m_port_num])->nb_respond(response);
      return;
    }
  }
  request_info *p_request_info = m_memory.new_request_info(request, status);
  if (m_memory.m_immediate_timing) {
    // Accept this one as our current transaction
    m_memory.m_p_active_request_info[m_port_num] = p_request_info;
    m_memory.do_active_request(m_port_num);
  }
  else {
    m_memory.m_request_fifo[m_port_num]->nb_write(p_request_info);
    m_memory.m_worker_thread_event[m_port_num].notify(SC_ZERO_TIME);
    XTSC_DEBUG(m_memory.m_text, "nb_request() called m_worker_thread_event[" << m_port_num << "].notify(SC_ZERO_TIME): " <<
                                p_request_info->m_request);
  }
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_load_retired(xtsc_address address8) {
  XTSC_INFO(m_memory.m_text, "nb_load_retired(0x" << setfill('0') << hex << setw(8) << address8 << ") called");
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_retire_flush() {
  XTSC_INFO(m_memory.m_text, "nb_retire_flush() called");
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::nb_lock(bool lock) {
  XTSC_INFO(m_memory.m_text, "nb_lock(" << boolalpha << lock << ") called");
}



void xtsc_component::xtsc_memory::xtsc_request_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_memory '" << m_memory.name() << "' m_request_exports[" << m_port_num << "]: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_memory.m_text, "Binding '" << port.name() << "' to xtsc_memory::m_request_exports[" << m_port_num << "]");
  m_p_port = &port;
}



xtsc::xtsc_fast_access_if *xtsc_component::xtsc_memory::get_fast_access_object() const {
  return m_fast_access_object;
}



void xtsc_component::xtsc_memory::script_thread() {

  try {

    while (get_words() != 0) {
      if (m_words[0] == "wait") {
        if (m_words.size() < 2) {
          ostringstream oss;
          oss << "WAIT command is missing arguments:" << endl;
          oss << m_line;
          oss << m_p_script_stream->info_for_exception();
          throw xtsc_exception(oss.str());
        }
        else if (m_words.size() == 2) {
          sc_time duration = m_clock_period * get_double(1, "duration");
          XTSC_DEBUG(m_text, "waiting " << duration);
          wait(duration);
        }
        else if ((m_words.size() == 4) || (m_words.size() == 5)) {
          u32 port = m_num_ports;
          if (m_words[1] != "*") {
            port = get_u32(1, "port");
            if (port >= m_num_ports) {
              ostringstream oss;
              oss << "port=" << port << " is out of range (0-" << (m_num_ports-1) << "):" << endl;
              oss << m_line;
              oss << m_p_script_stream->info_for_exception();
              throw xtsc_exception(oss.str());
            }
          }
          u32 type = get_request_type_code(2);
          u32 match = 0;
               if (m_words[3] == "*"   ) { match = 2; }
          else if (m_words[3] == "hit" ) { match = 1; }
          else if (m_words[3] == "miss") { match = 0; }
          else {
            ostringstream oss;
            oss << "WAIT command match argument (#3) is invalid (expected *|hit|miss):" << endl;
            oss << m_line;
            oss << m_p_script_stream->info_for_exception();
            throw xtsc_exception(oss.str());
          }
          u32 count = ((m_words.size() == 4) ? 1 : get_u32(4, "count"));
          if (count == 0) {
            ostringstream oss;
            oss << "WAIT command count cannot be zero:" << endl;
            oss << m_line;
            oss << m_p_script_stream->info_for_exception();
            throw xtsc_exception(oss.str());
          }
          do {
            bool got_one = false;
            do {
              XTSC_DEBUG(m_text, "Doing wait with current count of " << count << " for command: " << m_line);
              wait(m_script_thread_event);
              if ((port == m_prev_port) || (port == m_num_ports)) {
                if ((type == m_prev_type) || (type == 5)) {
                  if ((!match && !m_prev_hit) || (match && m_prev_hit) || (match == 2)) {
                    got_one = true;
                  }
                }
              }
            } while (!got_one);
            count -= 1;
          } while (count != 0);
        }
        else {
          ostringstream oss;
          oss << "WAIT command has missing/extra arguments:" << endl;
          oss << m_line;
          oss << m_p_script_stream->info_for_exception();
          throw xtsc_exception(oss.str());
        }
      }
      else if ((m_words[0] == "sync") || (m_words[0] == "synchronize")) {
        if (m_words.size() != 2) {
          ostringstream oss;
          oss << "SYNC command has missing/extra arguments:" << endl;
          oss << m_line;
          oss << m_p_script_stream->info_for_exception();
          throw xtsc_exception(oss.str());
        }
        sc_time time = m_clock_period * get_double(1, "time");
        sc_time now = sc_time_stamp();
        sc_time delay = (time <= now) ? SC_ZERO_TIME : (time - now);
        XTSC_DEBUG(m_text, "sync to time " << time << " requires delay of " << delay << " (no wait for 0 delay)");
        if (delay != SC_ZERO_TIME) {
          wait(delay);
        }
      }
      else if (m_words[0] == "info") {
        XTSC_INFO(m_text, m_line);
      }
      else if (m_words[0] == "note") {
        XTSC_NOTE(m_text, m_line);
      }
      else if ((m_words.size() > 1) && (m_words[1] == "stop")) {
        sc_time delay = m_clock_period * get_double(0, "delay");
        XTSC_DEBUG(m_text, "delaying " << delay << " before calling sc_stop()");
        wait(delay);
        XTSC_INFO(m_text, "calling sc_stop()");
        sc_stop();
      }
      else if ((m_words[0] == "dump")) {
        log4xtensa::LogLevel log_level = log4xtensa::INFO_LOG_LEVEL;
        if (m_words.size() > 1) {
          if (m_words[1] == "note") {
            log_level = log4xtensa::NOTE_LOG_LEVEL;
          }
        }
        ostringstream oss;
        oss << "Addresses:" << endl;
        dump_addresses(oss);
        xtsc_log_multiline(m_text, log_level, oss.str(), 2);
      }
      else if (m_words[0] == "clear") {
        if (m_words.size() == 1) {
          clear_addresses();
        }
        else {
          xtsc_address low_address;
          xtsc_address high_address;
          bool is_range = get_addresses(1, "address/address-range", low_address, high_address);
          map<xtsc_address, address_info*> *p_map = (is_range ? &m_address_range_map : &m_address_map);
          map<xtsc_address, address_info*>::iterator im = p_map->find(low_address);
          if ((im == p_map->end()) || (im->second->m_high_address != high_address)) {
            ostringstream oss;
            oss << "No entry for the address/address-range specified:" << endl;
            oss << m_line;
            oss << m_p_script_stream->info_for_exception();
            throw xtsc_exception(oss.str());
          }
          XTSC_INFO(m_text, "Clearing " << *im->second);
          p_map->erase(low_address);
        }
      }
      else if (((m_words.size() == 4) || (m_words.size() == 5)) &&
               ((m_words[3] == "okay" )         ||
                (m_words[3] == "ok")            ||
                (m_words[3] == "nacc")          ||
                (m_words[3] == "address_error") ||
                (m_words[3] == "data_error" )   ||
                (m_words[3] == "address_data_error")))
      {
        xtsc_address low_address;
        xtsc_address high_address;
        bool is_range = get_addresses(0, "address/address-range", low_address, high_address);
        map<xtsc_address, address_info*> *p_map = (is_range ? &m_address_range_map : &m_address_map);
        map<xtsc_address, address_info*>::iterator im = p_map->find(low_address);
        if (im != p_map->end()) {
          ostringstream oss;
          oss << "Duplicate entry for the address/address-range specified:" << endl;
          oss << m_line;
          oss << m_p_script_stream->info_for_exception();
          throw xtsc_exception(oss.str());
        }
        if (is_range) {
          for (im = m_address_range_map.begin(); im != m_address_range_map.end(); ++im) {
            address_info &info = *im->second;
            if (((low_address  >= info.m_low_address) && (low_address  <= info.m_high_address)) || 
                ((high_address >= info.m_low_address) && (high_address <= info.m_high_address)))
            {
              ostringstream oss;
              oss << "Specified address range, 0x" << hex << low_address << "-0x" << high_address << ", overlaps entry: "
                  << info << endl;
              oss << m_line;
              oss << m_p_script_stream->info_for_exception();
              throw xtsc_exception(oss.str());
            }
          }
        }
        if ((low_address < m_start_address8) || (high_address > m_end_address8)) {
          ostringstream oss;
          if (is_range) {
            oss << "Specified address range, 0x" << hex << low_address << "-0x" << high_address;
          }
          else {
            oss << "Specified address, 0x" << hex << low_address;
          }
          oss << ", is not within xtsc_memory '" << name() << "' (0x" << m_start_address8 << "-0x" << m_end_address8 << "):" << endl;
          oss << m_line;
          oss << m_p_script_stream->info_for_exception();
          throw xtsc_exception(oss.str());
        }
        u32 port = m_num_ports;
        if (m_words[1] != "*") {
          port = get_u32(1, "port");
          if (port >= m_num_ports) {
            ostringstream oss;
            oss << "port=" << port << " is out of range (0-" << (m_num_ports-1) << "):" << endl;
            oss << m_line;
            oss << m_p_script_stream->info_for_exception();
            throw xtsc_exception(oss.str());
          }
        }
        u32 type = get_request_type_code(2);
        xtsc_response::status_t status;
             if (m_words[3] == "okay")               status = xtsc_response::RSP_OK;
        else if (m_words[3] == "ok")                 status = xtsc_response::RSP_OK;
        else if (m_words[3] == "nacc")               status = xtsc_response::RSP_NACC;
        else if (m_words[3] == "address_error")      status = xtsc_response::RSP_ADDRESS_ERROR;
        else if (m_words[3] == "data_error")         status = xtsc_response::RSP_DATA_ERROR;
        else if (m_words[3] == "address_data_error") status = xtsc_response::RSP_ADDRESS_DATA_ERROR;
        else  {
          ostringstream oss;
          oss << "Program Bug: in line " << __LINE__ << " of file " << __FILE__;
          throw xtsc_exception(oss.str());
        }
        u32 limit = (m_words.size() == 4) ? 0 : get_u32(4, "limit");
        address_info *p_info = new address_info(low_address, high_address, is_range, port, m_num_ports, type, status, limit);
        (*p_map)[low_address] = p_info;
      }
      else {
        ostringstream oss;
        oss << "Syntax error in line:" << endl;
        oss << m_line;
        oss << m_p_script_stream->info_for_exception();
        throw xtsc_exception(oss.str());
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in SC_THREAD, script_thread, of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, log4xtensa::FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



int xtsc_component::xtsc_memory::get_words() {
  m_line_count = m_p_script_stream->get_words(m_words, m_line, true);
  XTSC_DEBUG(m_text, "get_words(): " << m_line);
  return m_words.size();
}



u32 xtsc_component::xtsc_memory::get_u32(u32 index, const string& argument_name) {
  u32 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing:" << endl;
    oss << m_line;
    oss << m_p_script_stream->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtou32(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number:" << endl;
    oss << m_line;
    oss << m_p_script_stream->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



double xtsc_component::xtsc_memory::get_double(u32 index, const string& argument_name) {
  double value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing:" << endl;
    oss << m_line;
    oss << m_p_script_stream->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtod(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number:" << endl;
    oss << m_line;
    oss << m_p_script_stream->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



bool xtsc_component::xtsc_memory::get_addresses(u32             index,
                                                const string&   argument_name,
                                                xtsc_address&   low_address,
                                                xtsc_address&   high_address)
{
  bool is_range = false;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing:" << endl;
    oss << m_line;
    oss << m_p_script_stream->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  if (m_words[index] == "*") {
    low_address  = m_start_address8;
    high_address = m_end_address8;
    return true;
  }
  string low  = m_words[index];
  string high = "";
  string::size_type pos = low.find_first_of("-");
  if ((pos != string::npos) && (pos != 0)) {
    is_range = true;
    high = low.substr(pos+1);
    low  = low.substr(0, pos);
  }
  try {
    low_address = xtsc_strtou32(low);
    if (is_range) {
      high_address = xtsc_strtou32(high);
      if (high_address < low_address) {
        ostringstream oss;
        oss << "highAddr (0x" << hex << high_address << ") cannot be less than low_address (0x" << low_address << "):" << endl;
        oss << m_line;
        oss << m_p_script_stream->info_for_exception();
        throw xtsc_exception(oss.str());
      }
    }
    else {
      high_address = low_address;
    }
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to address(es):" << endl;
    oss << m_line;
    oss << m_p_script_stream->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return is_range;
}



void xtsc_component::xtsc_memory::clear_addresses() {
  XTSC_INFO(m_text, "Clearing all addresses");
  for (u32 i=0; i<2; ++i) {
    map<xtsc_address, address_info*> *p_map = (i ? &m_address_range_map : &m_address_map);
    map<xtsc_address, address_info*>::iterator im;
    for (im = p_map->begin(); im != p_map->end(); ++im) {
      delete im->second;
    }
    p_map->clear();
  }
}



void xtsc_component::xtsc_memory::dump_addresses(std::ostream& os) {
  for (u32 i=0; i<2; ++i) {
    map<xtsc_address, address_info*> *p_map = (i ? &m_address_range_map : &m_address_map);
    map<xtsc_address, address_info*>::iterator im;
    for (im = p_map->begin(); im != p_map->end(); ++im) {
      os << *im->second << endl;
    }
  }
}



bool xtsc_component::xtsc_memory::compute_special_response(const xtsc::xtsc_request&    request,
                                                           xtsc::u32                    port_num,
                                                           xtsc_response::status_t&     status,
                                                           xtsc::u32&                   type)
{
  bool hit = false;
  xtsc_address address8 = request.get_byte_address();
  switch (request.get_type()) {
      case xtsc_request::READ:          type = 0; break;
      case xtsc_request::BLOCK_READ:    type = 1; break;
      case xtsc_request::RCW:           type = 2; break;
      case xtsc_request::WRITE:         type = 3; break;
      case xtsc_request::BLOCK_WRITE:   type = 4; break;
      default:  throw xtsc_exception("PROGRAM BUG: Unknown xtsc_request type in xtsc_memory::compute_special_response()");
  }
  address_info *p_info = NULL;
  map<xtsc_address, address_info*>::iterator im = m_address_map.find(address8);
  if (im != m_address_map.end() && (!im->second->m_finished)) {
    p_info = im->second;
  }
  else {
    for (im = m_address_range_map.begin(); im != m_address_range_map.end(); ++im) {
      address_info &info = *im->second;
      if (!info.m_finished && (info.m_low_address <= address8) && (address8 <= info.m_high_address)) {
        p_info = &info;
        break;
      }
    }
  }
  // Did we find this address in one of the address lists?
  if (p_info) {
    // Does the request type match (or is it a don't care)?
    if ((p_info->m_type == type) || (p_info->m_type == 5)) {
      // Does the port number match (or is it a don't care)?
      if ((p_info->m_port_num == port_num) || (p_info->m_port_num == m_num_ports)) {
        XTSC_VERBOSE(m_text, "0x" << hex << setw(8) << setfill('0') << address8 << ": Special response: " << *p_info);
        status = p_info->m_status;
        p_info->used();
        hit = true;
      }
    }
  }
  return hit;
}



u32 xtsc_component::xtsc_memory::get_request_type_code(u32 index) {
  u32 type = 0;
       if (m_words[index] == "*"          ) { type = 5; }
  else if (m_words[index] == "read"       ) { type = 0; }
  else if (m_words[index] == "block_read" ) { type = 1; }
  else if (m_words[index] == "rcw"        ) { type = 2; }
  else if (m_words[index] == "write"      ) { type = 3; }
  else if (m_words[index] == "block_write") { type = 4; }
  else {
    ostringstream oss;
    oss << "type argument (#" << index << ") invalid (expected *|read|block_read|rcw|write|block_write) in file '"
        << m_script_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  return type;
}



void xtsc_component::xtsc_memory::address_info::dump(ostream& os) const {

  // Save state of stream
  char c = os.fill('0');
  ios::fmtflags old_flags = os.flags();

  os << hex << "0x" << setfill('0') << setw(8) << m_low_address;
  if (m_is_range) {
    os << "-0x" << setw(8) << m_high_address;
  }
  else {
    os << "           ";
  }
  os << " " << dec;
  if (m_port_num == m_num_ports) {
    os << "*";
  }
  else {
    os << m_port_num;
  }
  os << " ";
  os << ((m_type == 0) ? "read       " :
         (m_type == 1) ? "block_read " :
         (m_type == 2) ? "rcw        " :
         (m_type == 3) ? "write      " :
         (m_type == 4) ? "block_write" :
                         "*          ");
  os << " " << xtsc_response::get_status_name(m_status) << " " << m_count << "/";
  if (m_limit) {
    os << m_limit;
  }
  else {
    os << "<NoLimit>";
  }
  if (m_finished) {
    os << " Finished";
  }

  // Restore state of stream
  os.fill(c);
  os.flags(old_flags);

}



bool xtsc_component::xtsc_memory::address_info::used() {
  m_count += 1;
  if (m_limit && m_count >= m_limit) {
    m_finished = true;
  }
  return m_finished;
}



std::ostream& xtsc_component::operator<<(std::ostream& os, const xtsc_component::xtsc_memory::address_info& info) {
  info.dump(os);
  return os;
}




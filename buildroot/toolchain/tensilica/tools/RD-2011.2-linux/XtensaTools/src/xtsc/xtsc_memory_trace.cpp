// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cerrno>
#include <algorithm>
#include <ostream>
#include <string>
#include <xtsc/xtsc_memory_trace.h>
#include <xtsc/xtsc_arbiter.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_dma_engine.h>
#include <xtsc/xtsc_master.h>
#include <xtsc/xtsc_pin2tlm_memory_transactor.h>
#include <xtsc/xtsc_router.h>
#include <xtsc/xtsc_logging.h>

using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;



xtsc_component::xtsc_memory_trace_parms::xtsc_memory_trace_parms(const xtsc_core&       core,
                                                                 const char            *memory_name,
                                                                 sc_trace_file         *p_trace_file,
                                                                 u32                    num_ports)
{
  string lc(memory_name ? memory_name : "");
  transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
  if (lc == "inbound_pif" || lc == "snoop") {
    lc = "pif";
  }
  xtsc_core::memory_port mem_port = xtsc_core::get_memory_port(lc.c_str()); 
  if (!core.has_memory_port(mem_port)) {
    ostringstream oss;
    oss << "xtsc_memory_trace_parms: core '" << core.name() << "' has no " << memory_name << " memory interface.";
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

  u32   width8          = core.get_memory_byte_width(mem_port);
  bool  big_endian      = core.is_big_endian();

  init(width8, big_endian, p_trace_file, num_ports);
}



xtsc_component::xtsc_memory_trace::xtsc_memory_trace(sc_module_name module_name, const xtsc_memory_trace_parms& trace_parms) :
  sc_module             (module_name),
  m_width8              (trace_parms.get_u32("byte_width")),
  m_big_endian          (trace_parms.get_bool("big_endian")),
  m_p_trace_file        ((sc_trace_file*)trace_parms.get_void_pointer("vcd_handle")),
  m_num_ports           (trace_parms.get_non_zero_u32("num_ports")),
  m_enable_tracing      (trace_parms.get_bool("enable_tracing")),
  m_text                (log4xtensa::TextLogger::getInstance(name()))
{

  if ((m_width8 != 4) && (m_width8 != 8) && (m_width8 != 16) && (m_width8 != 32) && (m_width8 != 64)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "' has illegal \"byte_width\"=" << m_width8 << " (legal values are 4|8|16|32|64)";
    throw xtsc_exception(oss.str());
  }

  sc_trace_file *p_tf = m_p_trace_file;

  if (!m_p_trace_file) {
    m_p_trace_file = sc_create_vcd_trace_file("waveforms");
    if (!m_p_trace_file) {
      throw xtsc_exception("xtsc_memory_trace: creation of VCD file \"waveforms.vcd\" failed");
    }
  }

  m_request_ports       = new sc_port<xtsc_request_if>*  [m_num_ports];
  m_respond_exports     = new sc_export<xtsc_respond_if>*[m_num_ports];
  m_respond_impl        = new xtsc_respond_if_impl*      [m_num_ports];

  m_respond_ports       = new sc_port<xtsc_respond_if>*  [m_num_ports];
  m_request_exports     = new sc_export<xtsc_request_if>*[m_num_ports];
  m_request_impl        = new xtsc_request_if_impl*      [m_num_ports];

  for (u32 i=0; i<m_num_ports; i++) {
    ostringstream oss1, oss2, oss3, oss4, oss5, oss6;

    oss1 << "m_request_ports["   << i << "]";
    oss2 << "m_respond_exports[" << i << "]";
    oss3 << "rsp";

    oss4 << "m_respond_ports["   << i << "]";
    oss5 << "m_request_exports[" << i << "]";
    oss6 << "req";

    if (m_num_ports > 1) {
      oss3 << "(" << i << ")";
      oss6 << "(" << i << ")";
    }

    m_request_ports  [i] = new sc_port<xtsc_request_if>  (oss1.str().c_str());
    m_respond_exports[i] = new sc_export<xtsc_respond_if>(oss2.str().c_str());
    m_respond_impl   [i] = new xtsc_respond_if_impl      (oss3.str().c_str(), *this, i);
    m_respond_ports  [i] = new sc_port<xtsc_respond_if>  (oss4.str().c_str());
    m_request_exports[i] = new sc_export<xtsc_request_if>(oss5.str().c_str());
    m_request_impl   [i] = new xtsc_request_if_impl      (oss6.str().c_str(), *this, i);

    (*m_respond_exports[i])(*m_respond_impl[i]);
    (*m_request_exports[i])(*m_request_impl[i]);

  }

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_memory_trace '" << name() << "':");
  XTSC_LOG(m_text, ll,        " byte_width      = "   << m_width8);
  XTSC_LOG(m_text, ll,        " big_endian      = "   << boolalpha << m_big_endian);
  XTSC_LOG(m_text, ll,        " vcd_handle      = "   << p_tf);
  XTSC_LOG(m_text, ll,        " num_ports       = "   << m_num_ports);
  XTSC_LOG(m_text, ll,        " enable_tracing  = "   << boolalpha << m_enable_tracing);

  reset(true);

}



xtsc_component::xtsc_memory_trace::~xtsc_memory_trace(void) {

  for (u32 i=0; i<m_num_ports; i++) {
    delete m_request_ports[i];
    delete m_respond_exports[i];
    delete m_respond_ports[i];
    delete m_request_exports[i];
  }

  delete [] m_request_ports;
  delete [] m_respond_exports;
  delete [] m_respond_ports;
  delete [] m_request_exports;

}



void xtsc_component::xtsc_memory_trace::reset(bool /*hard_reset*/) {
  XTSC_INFO(m_text, "xtsc_memory_trace::reset()");

}



void xtsc_component::xtsc_memory_trace::connect(xtsc_arbiter& arbiter, u32 trace_port) {
  if (trace_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid trace_port=" << trace_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  arbiter.m_request_port(*m_request_exports[trace_port]);
  (*m_respond_ports[trace_port])(arbiter.m_respond_export);
}



void xtsc_component::xtsc_memory_trace::connect(xtsc_cohctrl& cohctrl, xtsc_cohctrl::port_type type, u32 cohctrl_port, u32 trace_port) {
  u32 num_clients = cohctrl.get_num_clients();
  if (trace_port >= m_num_ports) {
    ostringstream oss;
    oss << kind() << " '" << name() << "':  Invalid trace_port specified to connect():  trace_port=" << trace_port;
    oss << ".  Valid range is 0 to " << (m_num_ports - 1) << ".";
    throw xtsc_exception(oss.str());
  }
  if ((type != xtsc_cohctrl::PT_MEMORY) && (cohctrl_port >= num_clients)) {
    ostringstream oss;
    oss << "Invalid cohctrl_port=" << cohctrl_port << " in connect(): " << endl;
    oss << cohctrl.kind() << " '" << cohctrl.name() << "' has " << num_clients
        << " ports numbered from 0 to " << num_clients-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (type == xtsc_cohctrl::PT_SNOOP) {
    (*cohctrl.m_snoop_ports[cohctrl_port])(*m_request_exports[trace_port]);
    (*m_respond_ports[trace_port])(*cohctrl.m_snoop_exports[cohctrl_port]);
  }
  else if (type == xtsc_cohctrl::PT_MEMORY) {
    cohctrl.m_request_port(*m_request_exports[trace_port]);
    (*m_respond_ports[trace_port])(cohctrl.m_respond_export);
  }
  else if (type == xtsc_cohctrl::PT_CLIENT) {
    (*m_request_ports[trace_port])(*cohctrl.m_client_exports[cohctrl_port]);
    (*cohctrl.m_client_ports[cohctrl_port])(*m_respond_exports[trace_port]);
  }
  else {
    ostringstream oss;
    oss << kind() << " '" << name() << "':  Invalid xtsc_cohctrl::port_type specified to connect()";
    throw xtsc_exception(oss.str());
  }
}



u32 xtsc_component::xtsc_memory_trace::connect(xtsc_core& core, const char *memory_port_name, u32 port_num, bool single_connect) {
  u32 num_connected = 1;
  if (port_num >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid port_num=" << port_num << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  string lc(memory_port_name ? memory_port_name : "");
  transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
  if (lc == "inbound_pif" || lc == "snoop") {
    (*m_request_ports[port_num])(core.get_request_export(memory_port_name));
    core.get_respond_port(memory_port_name)(*m_respond_exports[port_num]);
  }
  else {
    u32 width8 = core.get_memory_byte_width(xtsc_core::get_memory_port(memory_port_name));
    if (m_width8 && (width8 != m_width8)) {
      ostringstream oss;
      oss << "Memory interface data width mismatch: " << endl;
      oss << kind() << " '" << name() << "' is " << m_width8 << " bytes wide, but '" << memory_port_name << "' interface of" << endl;
      oss << "xtsc_core '" << core.name() << "' is " << width8 << " bytes wide.";
      throw xtsc_exception(oss.str());
    }
    core.get_request_port(memory_port_name)(*m_request_exports[port_num]);
    (*m_respond_ports[port_num])(core.get_respond_export(memory_port_name));
    // Should we connect a 2nd port?
    xtsc_core::memory_port memory_port = xtsc_core::get_memory_port(memory_port_name);
    if (!single_connect && (port_num+1 < m_num_ports) && core.is_dual_ported(xtsc_core::is_xlmi(memory_port))) {
      if (xtsc_core::is_ls_dual_port(memory_port, 0)) {
        // Don't connect if 2nd port of xtsc_memory_trace has already been connected.
        if (!m_request_impl[port_num+1]->is_connected()) {
          const char *dual_name = xtsc_core::get_memory_port_name(memory_port+1);
          sc_port<xtsc_request_if, NSPP>& dual_port = core.get_request_port(dual_name);
          // Don't connect if 2nd port of xtsc_core has already been connected.
          // This test is not reliable so we'll catch any errors and carry on (which
          // works for the OSCI simulator--but maybe not for others).
          if (!dual_port.get_interface()) {
            try {
              dual_port(*m_request_exports[port_num+1]);
              (*m_respond_ports[port_num+1])(core.get_respond_export(dual_name));
              num_connected += 1;
            } catch (...) {
              XTSC_INFO(m_text, "Core '" << core.name() << "' 2nd LD/ST memory port '" << dual_name << "' is already bond.");
            }
          }
        }
      }
    }
  }
  return num_connected;
}



void xtsc_component::xtsc_memory_trace::connect(xtsc_dma_engine& dma_engine, u32 trace_port) {
  if (trace_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid trace_port=" << trace_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  dma_engine.m_request_port(*m_request_exports[trace_port]);
  (*m_respond_ports[trace_port])(dma_engine.m_respond_export);
}



void xtsc_component::xtsc_memory_trace::connect(xtsc_master& master, u32 trace_port) {
  if (trace_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid trace_port=" << trace_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  master.m_request_port(*m_request_exports[trace_port]);
  (*m_respond_ports[trace_port])(master.m_respond_export);
}



u32 xtsc_component::xtsc_memory_trace::connect(xtsc_pin2tlm_memory_transactor&  pin2tlm,
                                               u32                              tran_port,
                                               u32                              trace_port,
                                               bool                             single_connect)
{
  u32 tran_ports = pin2tlm.get_num_ports();
  if (tran_port >= tran_ports) {
    ostringstream oss;
    oss << "Invalid tran_port=" << tran_port << " in connect(): " << endl;
    oss << pin2tlm.kind() << " '" << pin2tlm.name() << "' has " << tran_ports << " ports numbered from 0 to " << tran_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (trace_port >= m_num_ports) {
    ostringstream oss;
    oss << "Invalid trace_port=" << trace_port << " in connect(): " << endl;
    oss << kind() << " '" << name() << "' has " << m_num_ports << " ports numbered from 0 to " << m_num_ports-1 << endl;
    throw xtsc_exception(oss.str());
  }

  u32 num_connected = 0;

  while ((tran_port < tran_ports) && (trace_port < m_num_ports)) {

    (*pin2tlm.m_request_ports[tran_port])(*m_request_exports[trace_port]);
    (*m_respond_ports[trace_port])(*pin2tlm.m_respond_exports[tran_port]);

    trace_port += 1;
    tran_port += 1;
    num_connected += 1;

    if (single_connect) break;
    if (tran_port >= tran_ports) break;
    if (trace_port >= m_num_ports) break;
  }

  return num_connected;

}



void xtsc_component::xtsc_memory_trace::connect(xtsc_router& router, u32 router_port, u32 trace_port) {
  u32 num_slaves = router.get_num_slaves();
  if (router_port >= num_slaves) {
    ostringstream oss;
    oss << "Invalid router_port=" << router_port << " in connect(): " << endl;
    oss << router.kind() << " '" << router.name() << "' has " << num_slaves << " ports numbered from 0 to " << num_slaves-1 << endl;
    throw xtsc_exception(oss.str());
  }
  if (trace_port >= m_num_ports) {
    ostringstream oss;
    oss << kind() << " '" << name() << "':  Invalid trace_port specified to xtsc_memory_trace:connect():  trace_port=" << trace_port;
    oss << ".  Valid range is 0 to " << (m_num_ports - 1) << ".";
    throw xtsc_exception(oss.str());
  }
  (*router.m_request_ports[router_port])(*m_request_exports[trace_port]);
  (*m_respond_ports[trace_port])(*router.m_respond_exports[router_port]);
}



xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::xtsc_request_if_impl(const char               *object_name,
                                                                              xtsc_memory_trace&        trace,
                                                                              u32                       port_num) :
  sc_object     (object_name),
  m_trace       (trace),
  m_p_port      (0),
  m_port_num    (port_num),
  m_data        (m_trace.m_width8*8)
{

  m_nb_request_count            = 0;
  m_address8                    = 0;
  m_data                        = 0;
  m_size8                       = 0;
  m_pif_attribute               = 0xFFFFFFFF;
  m_route_id                    = 0;
  m_type                        = 0;
  m_num_transfers               = 0;
  m_byte_enables                = 0;
  m_id                          = 0;
  m_priority                    = 0;
  m_last_transfer               = 0;
  m_pc                          = 0;
  m_tag                         = 0;
  m_instruction_fetch           = 0;
  m_coherence                   = 0;
  m_snoop_virtual_address       = 0;
  m_hw_address8                 = 0;
  m_transfer_num                = 0;

  string prefix(name());

  sc_trace(m_trace.m_p_trace_file, m_nb_request_count,          (prefix+".count"        ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_address8,                  (prefix+".address"      ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_data,                      (prefix+".data"         ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_size8,                     (prefix+".size"         ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_pif_attribute,             (prefix+".pif_attribute").c_str());
  sc_trace(m_trace.m_p_trace_file, m_route_id,                  (prefix+".route_id"     ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_type,                      (prefix+".type"         ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_num_transfers,             (prefix+".num_transfers").c_str());
  sc_trace(m_trace.m_p_trace_file, m_byte_enables,              (prefix+".byte_enables" ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_id,                        (prefix+".id"           ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_priority,                  (prefix+".priority"     ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_last_transfer,             (prefix+".last_transfer").c_str());
  sc_trace(m_trace.m_p_trace_file, m_pc,                        (prefix+".pc"           ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_tag,                       (prefix+".tag"          ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_instruction_fetch,         (prefix+".ifetch"       ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_coherence,                 (prefix+".coherence"    ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_snoop_virtual_address,     (prefix+".snoop_vaddr"  ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_hw_address8,               (prefix+".hw_address"   ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_transfer_num,              (prefix+".transfer_num" ).c_str());
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  (*m_trace.m_request_ports[m_port_num])->nb_peek(address8, size8, buffer);
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  (*m_trace.m_request_ports[m_port_num])->nb_poke(address8, size8, buffer);
}



bool xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_peek_coherent(xtsc_address     virtual_address8,
                                                                               xtsc_address     physical_address8,
                                                                               u32              size8,
                                                                               u8              *buffer)
{
  return (*m_trace.m_request_ports[m_port_num])->nb_peek_coherent(virtual_address8, physical_address8, size8, buffer);
}



bool xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_poke_coherent(xtsc_address     virtual_address8,
                                                                               xtsc_address     physical_address8,
                                                                               u32              size8,
                                                                               const u8        *buffer)
{
  return (*m_trace.m_request_ports[m_port_num])->nb_poke_coherent(virtual_address8, physical_address8, size8, buffer);
}



bool xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_fast_access(xtsc_fast_access_request &request) {
  return (*m_trace.m_request_ports[m_port_num])->nb_fast_access(request);
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_request(const xtsc_request& request) {
  XTSC_DEBUG(m_trace.m_text, request << " Port #" << m_port_num);

  // Update values for tracing
  if (m_trace.m_enable_tracing) {
    m_nb_request_count += 1;
    m_address8                  = request.get_byte_address();
    m_size8                     = request.get_byte_size();
    m_pif_attribute             = request.get_pif_attribute();
    m_route_id                  = request.get_route_id();
    m_type                      = (u8) request.get_type();
    m_num_transfers             = request.get_num_transfers();
    m_byte_enables              = request.get_byte_enables();
    m_id                        = request.get_id();
    m_priority                  = request.get_priority();
    m_last_transfer             = request.get_last_transfer();
    m_pc                        = request.get_pc();
    m_tag                       = request.get_tag();
    m_instruction_fetch         = request.get_instruction_fetch();
    m_coherence                 = (u8) request.get_coherence();
    m_snoop_virtual_address     = request.get_snoop_virtual_address();
    m_hw_address8               = request.get_hardware_address();
    m_transfer_num              = request.get_transfer_number();
    const u8 *buf               = request.get_buffer();
    m_data = 0;
    for (u32 i=0; i<m_size8; ++i) {
      u32 index = i;
      if (m_trace.m_big_endian) {
        index = m_size8 - 1 - i;
      }
      m_data.range(i*8+7, i*8) = buf[index];
    }
  }

  // Forward the call
  (*m_trace.m_request_ports[m_port_num])->nb_request(request);
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_load_retired(xtsc_address address8) {
  (*m_trace.m_request_ports[m_port_num])->nb_load_retired(address8);
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_retire_flush() {
  (*m_trace.m_request_ports[m_port_num])->nb_retire_flush();
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::nb_lock(bool lock) {
  (*m_trace.m_request_ports[m_port_num])->nb_lock(lock);
}



void xtsc_component::xtsc_memory_trace::xtsc_request_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_memory_trace '" << m_trace.name() << "' " << name() << ": " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_trace.m_text, "Binding '" << port.name() << "' to " << name());
  m_p_port = &port;

}



xtsc_component::xtsc_memory_trace::xtsc_respond_if_impl::xtsc_respond_if_impl(const char               *object_name,
                                                                              xtsc_memory_trace&        trace,
                                                                              u32                       port_num) :
  sc_object     (object_name),
  m_trace       (trace),
  m_p_port      (0),
  m_port_num    (port_num),
  m_data        (m_trace.m_width8*8)
{

  m_nb_respond_count    = 0;
  m_rsp_ok_count        = 0;
  m_rsp_nacc_count      = 0;
  m_address8            = 0;
  m_data                = 0;
  m_size8               = 0;
  m_route_id            = 0;
  m_status              = 0;
  m_id                  = 0;
  m_priority            = 0;
  m_last_transfer       = 0;
  m_pc                  = 0;
  m_tag                 = 0;
  m_snoop               = 0;
  m_snoop_data          = 0;
  m_coherence           = 0;

  string prefix(name());

  sc_trace(m_trace.m_p_trace_file, m_nb_respond_count,  (prefix+".count"        ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_rsp_ok_count,      (prefix+".rsp_ok"       ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_rsp_nacc_count,    (prefix+".rsp_nacc"     ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_address8,          (prefix+".address"      ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_data,              (prefix+".data"         ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_size8,             (prefix+".size"         ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_route_id,          (prefix+".route_id"     ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_status,            (prefix+".status"       ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_id,                (prefix+".id"           ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_priority,          (prefix+".priority"     ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_last_transfer,     (prefix+".last_transfer").c_str());
  sc_trace(m_trace.m_p_trace_file, m_pc,                (prefix+".pc"           ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_tag,               (prefix+".tag"          ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_snoop,             (prefix+".snoop"        ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_snoop_data,        (prefix+".snoop_data"   ).c_str());
  sc_trace(m_trace.m_p_trace_file, m_coherence,         (prefix+".coherence"    ).c_str());
}



bool xtsc_component::xtsc_memory_trace::xtsc_respond_if_impl::nb_respond(const xtsc_response& response) {
  XTSC_DEBUG(m_trace.m_text, response << " Port #" << m_port_num);

  // Update values for tracing
  if (m_trace.m_enable_tracing) {
    m_nb_respond_count += 1;
    if (response.get_status() == xtsc_response::RSP_OK)   m_rsp_ok_count   += 1;
    if (response.get_status() == xtsc_response::RSP_NACC) m_rsp_nacc_count += 1;
    m_address8      = response.get_byte_address();
    m_size8         = response.get_byte_size();
    m_route_id      = response.get_route_id();
    m_status        = (u8) response.get_status();
    m_id            = response.get_id();
    m_priority      = response.get_priority();
    m_last_transfer = response.get_last_transfer();
    m_pc            = response.get_pc();
    m_tag           = response.get_tag();
    m_snoop         = response.is_snoop();
    m_snoop_data    = response.has_snoop_data();
    m_coherence     = (u8) response.get_coherence();
    const u8 *buf   = response.get_buffer();
    m_data = 0;
    for (u32 i=0; i<m_size8; ++i) {
      u32 index = i;
      if (m_trace.m_big_endian) {
        index = m_size8 - 1 - i;
      }
      m_data.range(i*8+7, i*8) = buf[index];
    }
  }

  // Forward the call
  return (*m_trace.m_respond_ports[m_port_num])->nb_respond(response);
}



void xtsc_component::xtsc_memory_trace::xtsc_respond_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_memory_trace '" << m_trace.name() << "' " << name() << ": " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_trace.m_text, "Binding '" << port.name() << "' to " << name());
  m_p_port = &port;
}




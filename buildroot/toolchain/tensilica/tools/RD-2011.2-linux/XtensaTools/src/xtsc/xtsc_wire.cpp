// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <sstream>
#include <xtsc/xtsc_wire.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_interrupt_distributor.h>
#include <xtsc/xtsc_mmio.h>
#include <xtsc/xtsc_tx_loader.h>
#include <xtsc/xtsc_wire_logic.h>
#include <xtsc/xtsc_wire_source.h>
#include <xtsc/xtsc_logging.h>


using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::READ_WIRE;
using log4xtensa::WRITE_STATE;
using log4xtensa::UNKNOWN_PC;
using log4xtensa::UNKNOWN;



xtsc_component::xtsc_wire_parms::xtsc_wire_parms(const xtsc_core&       core,
                                                 const char            *core_intf_name,
                                                 const char            *write_file,
                                                 const char            *read_file,
                                                 bool                   wraparound)
{
  u32 width1;
  if (core.has_export_state(core_intf_name) || core.has_import_wire(core_intf_name)) {
    width1 = core.get_tie_bit_width(core_intf_name);
  }
  else if (core.has_system_output_wire(core_intf_name)) {
    width1 = core.get_sysio_bit_width(core_intf_name);
  }
  else {
    ostringstream oss;
    oss << "xtsc_wire_parms: core '" << core.name() << "' has no export state, import wire, or system-level output named '"
        << core_intf_name << "'.";
    throw xtsc_exception(oss.str());
  }
  init(width1, write_file, read_file, wraparound);
}



xtsc_component::xtsc_wire::xtsc_wire(sc_module_name module_name, const xtsc_wire_parms& wire_parms) :
  sc_module             (module_name),
  m_width1              (wire_parms.get_non_zero_u32("bit_width")),
  m_width8              ((m_width1+7)/8),
  m_initial_value       (m_width1),
  m_value               (m_width1),
  m_p_trace_file        (static_cast<sc_trace_file*>(const_cast<void*>(wire_parms.get_void_pointer("vcd_handle")))),
  m_text                (log4xtensa::TextLogger::getInstance(name())),
  m_binary              (log4xtensa::BinaryLogger::getInstance(name())),
  m_log_data_binary     (true),
  m_timestamp           (false),
  m_read_file_value     (m_width1)
{

  m_p_wire              = new sc_unsigned(m_width1);
  m_initial_value       = wire_parms.get_c_str("initial_value");
  m_use_wire            = true;
  m_write_file          = 0;
  m_read_file           = 0;

  // Handle write_file
  const char *write_file = wire_parms.get_c_str("write_file");
  if (write_file && write_file[0]) {
    m_write_file_name = write_file;
    m_use_wire = false;
    m_write_file = new ofstream(m_write_file_name.c_str(), ios::out);
    if (!m_write_file->is_open()) {
      ostringstream oss;
      oss << "xtsc_wire '" << name() << "' cannot open write_file '" << m_write_file_name << "'.";
      throw xtsc_exception(oss.str());
    }
    m_timestamp = wire_parms.get_bool("timestamp");
  }

  // Handle read_file
  m_wraparound  = wire_parms.get_bool("wraparound");
  const char *read_file = wire_parms.get_c_str("read_file");
  if (read_file && read_file[0]) {
    m_read_file_name = read_file;
    m_use_wire = false;
    m_read_file = new xtsc_script_file(m_read_file_name.c_str(), "\"read_file\"", name(), "xtsc_wire", m_wraparound);
  }

  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, m_value, name());
  }

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_wire '" << name() << "':");
  XTSC_LOG(m_text, ll,        " bit_width               = "   << m_width1);
  XTSC_LOG(m_text, ll,        " initial_value           = "   << m_initial_value.to_string(SC_HEX));
  XTSC_LOG(m_text, ll,        " vcd_handle              = "   << m_p_trace_file);
  XTSC_LOG(m_text, ll,        " write_file              = "   << m_write_file_name);
  if (m_write_file) {
  XTSC_LOG(m_text, ll,        " timestamp               = "   << boolalpha << m_timestamp);
  }
  XTSC_LOG(m_text, ll,        " read_file               = "   << m_read_file_name);
  XTSC_LOG(m_text, ll,        " wraparound              = "   << boolalpha << m_wraparound);

  reset();

}



xtsc_component::xtsc_wire::~xtsc_wire() {
  if (m_write_file) delete m_write_file;
  if (m_read_file)  delete m_read_file;
}



void xtsc_component::xtsc_wire::reset(bool /*hard_reset*/) {
  XTSC_INFO(m_text, "xtsc_wire::reset()");

  m_read_file_value             = 0;
  m_has_read_file_value         = true;
  m_read_file_line_number       = 0;
  m_next_word_index             = 0;

  *m_p_wire = m_initial_value;

  m_words.clear();

  if (m_write_file) {
    m_write_file->close();
    m_write_file->clear();
    m_write_file->open(m_write_file_name.c_str(), ios::out);
    if (!m_write_file->is_open()) {
      ostringstream oss;
      oss << "xtsc_wire '" << name() << "' reset() method cannot open write_file '" << m_write_file_name << "'.";
      throw xtsc_exception(oss.str());
    }
  }

  if (m_read_file) {
    m_read_file->reset();
    get_next_read_file_value();
  }

}



void xtsc_component::xtsc_wire::connect(xtsc_core& core, const char *core_intf_name) {
  u32 wo = 0;
  bool output = true;
  if (core.has_export_state(core_intf_name)) {
    core.get_export_state(core_intf_name)(*this);
    wo = core.get_tie_bit_width(core_intf_name);
  }
  else if (core.has_import_wire(core_intf_name)) {
    core.get_import_wire(core_intf_name)(*this);
    wo = core.get_tie_bit_width(core_intf_name);
  }
  else if (core.has_system_output_wire(core_intf_name)) {
    output = false;
    core.get_system_output_wire(core_intf_name)(*this);
    wo = core.get_sysio_bit_width(core_intf_name);
  }
  else {
    ostringstream oss;
    oss << "xtsc_wire::connect: core '" << core.name() << "' has no export state, import wire, or system-level output named '"
        << core_intf_name << "'.";
    throw xtsc_exception(oss.str());
  }
  u32 wi = m_width1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit " << (output ? "output" : "input") << " '" << core_intf_name
        << "' of xtsc_core '" << core.name() << "' to " << wi << "-bit xtsc_wire '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_wire::connect(xtsc_wire_source& source, const char *output_name) {
  string output((output_name && output_name[0]) ? output_name : source.get_default_output_name().c_str());
  u32 wo = source.get_bit_width(output_name);
  u32 wi = m_width1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output << "' of xtsc_wire_source '" << source.name()
        << "' to " << wi << "-bit xtsc_wire '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  source.get_tlm_output(output_name)(*this);
}



void xtsc_component::xtsc_wire::connect(xtsc_mmio& mmio, const char *output_name) {
  u32 wo = mmio.get_bit_width(output_name);
  u32 wi = m_width1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_mmio '" << mmio.name()
        << "' to " << wi << "-bit xtsc_wire '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  mmio.get_output(output_name)(*this);
}



void xtsc_component::xtsc_wire::connect(xtsc_wire_logic& logic, const char *output_name) {
  u32 wo = logic.get_bit_width(output_name);
  u32 wi = m_width1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_wire_logic '" << logic.name()
        << "' to " << wi << "-bit xtsc_wire '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  logic.get_output(output_name)(*this);
}



void xtsc_component::xtsc_wire::connect(xtsc_tx_loader& loader, const char *output_name) {
  u32 wo = 0;
  u32 wi = m_width1;
  string output(output_name ? output_name : "");
  if (output == "Done") {
    wo = 1;
    loader.m_done(*this);
  }
  else if (output == "Mode") {
    wo = 3;
    loader.m_mode(*this);
  }
  else {
    ostringstream oss;
    oss << "xtsc_tx_loader '" << loader.name() << "' has no output named \"" << output
        << "\" (valid output names are \"Done\" and \"Mode\")";
    throw xtsc_exception(oss.str());
  }
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output << "' of xtsc_tx_loader '" << loader.name()
        << "' to " << wi << "-bit xtsc_wire '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_wire::connect(xtsc_interrupt_distributor& distributor, const char *output_name) {
  u32 wo = distributor.get_bit_width(output_name);
  u32 wi = m_width1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_interrupt_distributor '"
        << distributor.name() << "' to " << wi << "-bit xtsc_wire '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  distributor.get_output(output_name)(*this);
}



void xtsc_component::xtsc_wire::nb_write(const sc_unsigned& value) {
  if (static_cast<u32>(value.length()) != m_width1) {
    ostringstream oss;
    oss << "ERROR: Value of width=" << value.length() << " bits written to xtsc_wire '" << name() << "' of width=" << m_width1;
    throw xtsc_exception(oss.str());
  }
  m_value = value;
  if (m_use_wire) {
    *m_p_wire = value;
    XTSC_INFO(m_text, "Write wire (TLM) " << value.to_string(SC_HEX));
    xtsc_log_tie_port_event(m_binary, INFO_LOG_LEVEL, UNKNOWN, WRITE_STATE, UNKNOWN_PC, m_log_data_binary, value);
  }
  else {
    if (m_write_file) {
      ostringstream oss;
      oss << value.to_string(SC_HEX);
      XTSC_INFO(m_text, "Write to file " << oss.str());
      if (m_timestamp) {
        string buf;
        oss << " // " << setprecision(xtsc_get_text_logging_time_precision()) << fixed << setw(xtsc_get_text_logging_time_width())
                      << (sc_core::sc_time_stamp() / xtsc_get_system_clock_period()) << xtsc_log_delta_cycle(buf);
      }
      *m_write_file << oss.str() << endl;
    }
    else {
      ostringstream oss;
      oss << "nb_write() called for xtsc_wire '" << name() << "', but no write_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
}



sc_unsigned xtsc_component::xtsc_wire::nb_read() {
  if (m_use_wire) {
    m_value = *m_p_wire;
    XTSC_INFO(m_text, "Read wire (TLM) " << m_value.to_string(SC_HEX));
    xtsc_log_tie_port_event(m_binary, INFO_LOG_LEVEL, UNKNOWN, READ_WIRE, UNKNOWN_PC, m_log_data_binary, m_value);
  }
  else {
    if (m_read_file) {
      m_value = m_read_file_value;
      XTSC_INFO(m_text, "Read from file " << m_value.to_string(SC_HEX));
      get_next_read_file_value();
    }
    else {
      ostringstream oss;
      oss << "nb_read() called for xtsc_wire '" << name() << "', but no read_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
  return m_value;
}



void xtsc_component::xtsc_wire::get_next_read_file_value() {
  if (!m_has_read_file_value) {
    return;
  }
  if (m_next_word_index >= m_words.size()) {
    m_read_file_line_number = m_read_file->get_words(m_words, m_line);
    if (!m_read_file_line_number) {
      m_has_read_file_value = false;
      return;
    }
    m_next_word_index = 0;
  }
  // Prevent sign extension that sc_unsigned does when assigned a hex string with the high bit set
  string word_no_sign_extend = m_words[m_next_word_index];
  if ((word_no_sign_extend.size() > 2) && (word_no_sign_extend.substr(0,2) == "0x")) {
    word_no_sign_extend.insert(2, "0");
  }
  try {
    m_read_file_value = word_no_sign_extend.c_str();
  }
  catch (...) {
    ostringstream oss;
    oss << "Cannot convert word #" << (m_next_word_index+1) << " (\"" << m_words[m_next_word_index] << "\") to number:" << endl;
    oss << m_line;
    oss << m_read_file->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  m_next_word_index += 1;
}



void xtsc_component::xtsc_wire::register_port(sc_port_base& port, const char *if_typename) {
  XTSC_INFO(m_text, "Binding '" << port.name() << "' to xtsc_wire '" << name() << "'");
}




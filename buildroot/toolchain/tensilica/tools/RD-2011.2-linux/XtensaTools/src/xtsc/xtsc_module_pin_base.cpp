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
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_fast_access.h>
#include <xtsc/xtsc_module_pin_base.h>
//#include <xtsc/xtsc_tlm2pin_memory_transactor.h>
#include <xtsc/xtsc_core.h>


using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace log4xtensa;
using namespace xtsc;



typedef sc_in <bool>          bool_input;
typedef sc_in <sc_uint_base>  uint_input;
typedef sc_in <sc_bv_base>    wide_input;
typedef sc_out<bool>          bool_output;
typedef sc_out<sc_uint_base>  uint_output;
typedef sc_out<sc_bv_base>    wide_output;



typedef xtsc_component::xtsc_module_pin_base::memory_interface_type     memory_interface_type;



#if !defined(_WIN32)
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::READ;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::WRITE;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::BLOCK_READ;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::BLOCK_WRITE;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::BURST_READ;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::BURST_WRITE;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::RCW;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::SNOOP_SHARED;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::SNOOP_EXCLUSIVE;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::m_type_mask;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::m_bl_num_transfers_mask;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::m_bu_num_transfers_mask;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::m_num_transfers_shift;
const u32 xtsc_component::xtsc_module_pin_base::req_cntl::m_last_mask;

const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::OK;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::AErr;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::DErr;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::ADErr;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_status_mask;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_status_shift;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_last_mask;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_last_transfer_bit;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_data_mask;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_no_data_mask;
const u32 xtsc_component::xtsc_module_pin_base::resp_cntl::m_shared_mask;
#endif



string xtsc_component::xtsc_module_pin_base::get_interface_uc(const char *interface_name) {
  string memory_interface = (interface_name ? interface_name : "");
  string interface_uc(memory_interface);
  transform(memory_interface.begin(), memory_interface.end(), interface_uc.begin(), ::toupper);
  return interface_uc;
}



memory_interface_type xtsc_component::xtsc_module_pin_base::get_interface_type(xtsc_core::memory_port mem_port) {
  switch (mem_port) {
    case xtsc_core::MEM_DRAM0LS0:
    case xtsc_core::MEM_DRAM0LS1: return DRAM0;

    case xtsc_core::MEM_DRAM1LS0:
    case xtsc_core::MEM_DRAM1LS1: return DRAM1;

    case xtsc_core::MEM_DROM0LS0:
    case xtsc_core::MEM_DROM0LS1: return DROM0;

    case xtsc_core::MEM_IRAM0:    return IRAM0;
    case xtsc_core::MEM_IRAM1:    return IRAM1;

    case xtsc_core::MEM_IROM0:    return IROM0;

    case xtsc_core::MEM_URAM0:    return URAM0;

    case xtsc_core::MEM_XLMI0LS0:
    case xtsc_core::MEM_XLMI0LS1: return XLMI0;

    case xtsc_core::MEM_PIF:      return PIF; 

    default: {
      ostringstream oss;
      oss << " xtsc_module_pin_base: Unsupported mem_port = " << mem_port << endl;
      throw xtsc_exception(oss.str());
    }
  }
}



memory_interface_type xtsc_component::xtsc_module_pin_base::get_interface_type(const string& interface_name) {
  return get_interface_type(xtsc_core::get_memory_port(interface_name));
}



const char *xtsc_component::xtsc_module_pin_base::get_interface_name(memory_interface_type interface_type) {
  switch (interface_type) {
    case DRAM0: return "DRAM0";
    case DRAM1: return "DRAM1";
    case DROM0: return "DROM0";
    case IRAM0: return "IRAM0";
    case IRAM1: return "IRAM1";
    case IROM0: return "IROM0";
    case URAM0: return "URAM0";
    case XLMI0: return "XLMI0";
    case PIF:   return "PIF";
    default: {
      ostringstream oss;
      oss << "Program Bug in xtsc_module_pin_base::get_interface_name: Unhandled memory_interface_type = " << interface_type;
      throw xtsc_exception(oss.str());
    }
  }
}



u32 xtsc_component::xtsc_module_pin_base::get_number_of_enables(memory_interface_type interface_type, u32 byte_width) {
  u32 bits = byte_width;
  if ((interface_type == IRAM0) || (interface_type == IRAM1) || (interface_type == IROM0)) {
    bits /= 4;
  }
  return bits;
}



xtsc_component::xtsc_module_pin_base::xtsc_module_pin_base(sc_module&           module,
                                                           u32                  num_sets,
                                                           sc_trace_file       *p_trace_file,
                                                           const string&        suffix) :
  m_sc_module           (module),
  m_num_sets            (num_sets),
  m_p_trace_file        (p_trace_file),
  m_suffix              (suffix)
{
  if (m_num_sets < 1) {
    ostringstream oss;
    oss << "xtsc_module_pin_base: " << m_sc_module.kind() << "'" << m_sc_module.name() << "': num_sets of 0 is not allowed";
    throw xtsc_exception(oss.str());
  }

  for (u32 i=0; i<m_num_sets; ++i) {
    m_vector_set_input.      push_back(new set_string);
    m_vector_set_output.     push_back(new set_string);
    m_vector_set_bool_input. push_back(new set_string);
    m_vector_set_uint_input. push_back(new set_string);
    m_vector_set_wide_input. push_back(new set_string);
    m_vector_set_bool_output.push_back(new set_string);
    m_vector_set_uint_output.push_back(new set_string);
    m_vector_set_wide_output.push_back(new set_string);
  }

}



xtsc_component::xtsc_module_pin_base::~xtsc_module_pin_base(void) {
  // Do any required clean-up here
}



void xtsc_component::xtsc_module_pin_base::confirm_valid_set_id(xtsc::u32 set_id) const {
  if (set_id >= m_num_sets) {
    ostringstream oss;
    oss << "xtsc_module_pin_base: " << m_sc_module.kind() << " '" << m_sc_module.name() << "': set_id=" << set_id
        << " is out of range [0-" << (m_num_sets-1) << "]";
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_module_pin_base::confirm_unique_name_and_valid_set_id(const std::string& port_name, xtsc::u32 set_id) const {
  confirm_valid_set_id(set_id);
  if (m_set_port.find(port_name) != m_set_port.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base: " << m_sc_module.kind() << " '" << m_sc_module.name() << "': Cannot add new port '" << port_name
        << "' because a port with that name already exists.";
    throw xtsc_exception(oss.str());
  }
}



bool_input& xtsc_component::xtsc_module_pin_base::add_bool_input(const string& port_name, bool append_id, u32 set_id) {
  u32 id = 0;
  string full_port_name = port_name;
  if (append_id) {
    id = set_id;
    ostringstream oss;
    oss << set_id;
    full_port_name = port_name + oss.str();
  }
  full_port_name += m_suffix;
  confirm_unique_name_and_valid_set_id(full_port_name, id);
  sc_in<bool> *port = new sc_in<bool>(full_port_name.c_str());
  m_map_bool_input[full_port_name] = port;
  m_vector_set_bool_input[id]->insert(full_port_name);
  m_vector_set_input[id]->insert(full_port_name);
  m_set_input.insert(full_port_name);
  m_set_port.insert(full_port_name);
  m_map_port_size[full_port_name] = 1;
  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, *port, port->name());
  }
  return *port;
}



uint_input& xtsc_component::xtsc_module_pin_base::add_uint_input(const string& port_name, u32 num_bits, bool append_id, u32 set_id) {
  sc_length_context length_context(num_bits);
  u32 id = 0;
  string full_port_name = port_name;
  if (append_id) {
    id = set_id;
    ostringstream oss;
    oss << set_id;
    full_port_name = port_name + oss.str();
  }
  full_port_name += m_suffix;
  confirm_unique_name_and_valid_set_id(full_port_name, id);
  sc_in<sc_uint_base> *port = new sc_in<sc_uint_base>(full_port_name.c_str());
  m_map_uint_input[full_port_name] = port;
  m_vector_set_uint_input[id]->insert(full_port_name);
  m_vector_set_input[id]->insert(full_port_name);
  m_set_input.insert(full_port_name);
  m_set_port.insert(full_port_name);
  m_map_port_size[full_port_name] = num_bits;
  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, *port, port->name());
  }
  return *port;
}



wide_input& xtsc_component::xtsc_module_pin_base::add_wide_input(const string& port_name, u32 num_bits, bool append_id, u32 set_id) {
  sc_length_context length_context(num_bits);
  u32 id = 0;
  string full_port_name = port_name;
  if (append_id) {
    id = set_id;
    ostringstream oss;
    oss << set_id;
    full_port_name = port_name + oss.str();
  }
  full_port_name += m_suffix;
  confirm_unique_name_and_valid_set_id(full_port_name, id);
  sc_in<sc_bv_base> *port = new sc_in<sc_bv_base>(full_port_name.c_str());
  m_map_wide_input[full_port_name] = port;
  m_vector_set_wide_input[id]->insert(full_port_name);
  m_vector_set_input[id]->insert(full_port_name);
  m_set_input.insert(full_port_name);
  m_set_port.insert(full_port_name);
  m_map_port_size[full_port_name] = num_bits;
  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, *port, port->name());
  }
  return *port;
}



bool_output& xtsc_component::xtsc_module_pin_base::add_bool_output(const string& port_name, bool append_id, u32 set_id) {
  u32 id = 0;
  string full_port_name = port_name;
  if (append_id) {
    id = set_id;
    ostringstream oss;
    oss << set_id;
    full_port_name = port_name + oss.str();
  }
  full_port_name += m_suffix;
  confirm_unique_name_and_valid_set_id(full_port_name, id);
  sc_out<bool> *port = new sc_out<bool>(full_port_name.c_str());
  m_map_bool_output[full_port_name] = port;
  m_vector_set_bool_output[id]->insert(full_port_name);
  m_vector_set_output[id]->insert(full_port_name);
  m_set_output.insert(full_port_name);
  m_set_port.insert(full_port_name);
  m_map_port_size[full_port_name] = 1;
  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, *port, port->name());
  }
  return *port;
}



uint_output& xtsc_component::xtsc_module_pin_base::add_uint_output(const string& port_name, u32 num_bits, bool append_id, u32 set_id) {
  sc_length_context length_context(num_bits);
  u32 id = 0;
  string full_port_name = port_name;
  if (append_id) {
    id = set_id;
    ostringstream oss;
    oss << set_id;
    full_port_name = port_name + oss.str();
  }
  full_port_name += m_suffix;
  confirm_unique_name_and_valid_set_id(full_port_name, id);
  sc_out<sc_uint_base> *port = new sc_out<sc_uint_base>(full_port_name.c_str());
  m_map_uint_output[full_port_name] = port;
  m_vector_set_uint_output[id]->insert(full_port_name);
  m_vector_set_output[id]->insert(full_port_name);
  m_set_output.insert(full_port_name);
  m_set_port.insert(full_port_name);
  m_map_port_size[full_port_name] = num_bits;
  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, *port, port->name());
  }
  return *port;
}



wide_output& xtsc_component::xtsc_module_pin_base::add_wide_output(const string& port_name, u32 num_bits, bool append_id, u32 set_id) {
  sc_length_context length_context(num_bits);
  u32 id = 0;
  string full_port_name = port_name;
  if (append_id) {
    id = set_id;
    ostringstream oss;
    oss << set_id;
    full_port_name = port_name + oss.str();
  }
  full_port_name += m_suffix;
  confirm_unique_name_and_valid_set_id(full_port_name, id);
  sc_out<sc_bv_base> *port = new sc_out<sc_bv_base>(full_port_name.c_str());
  m_map_wide_output[full_port_name] = port;
  m_vector_set_wide_output[id]->insert(full_port_name);
  m_vector_set_output[id]->insert(full_port_name);
  m_set_output.insert(full_port_name);
  m_set_port.insert(full_port_name);
  m_map_port_size[full_port_name] = num_bits;
  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, *port, port->name());
  }
  return *port;
}



bool xtsc_component::xtsc_module_pin_base::has_input(const string& port_name) const {
  return (m_set_input.find(port_name) != m_set_input.end());
}



bool xtsc_component::xtsc_module_pin_base::has_output(const string& port_name) const {
  return (m_set_output.find(port_name) != m_set_output.end());
}



bool xtsc_component::xtsc_module_pin_base::has_bool_input(const string& port_name) const {
  return (m_map_bool_input.find(port_name) != m_map_bool_input.end());
}



bool xtsc_component::xtsc_module_pin_base::has_uint_input(const string& port_name) const {
  return (m_map_uint_input.find(port_name) != m_map_uint_input.end());
}



bool xtsc_component::xtsc_module_pin_base::has_wide_input(const string& port_name) const {
  return (m_map_wide_input.find(port_name) != m_map_wide_input.end());
}



bool xtsc_component::xtsc_module_pin_base::has_bool_output(const string& port_name) const {
  return (m_map_bool_output.find(port_name) != m_map_bool_output.end());
}



bool xtsc_component::xtsc_module_pin_base::has_uint_output(const string& port_name) const {
  return (m_map_uint_output.find(port_name) != m_map_uint_output.end());
}



bool xtsc_component::xtsc_module_pin_base::has_wide_output(const string& port_name) const {
  return (m_map_wide_output.find(port_name) != m_map_wide_output.end());
}



set<string> xtsc_component::xtsc_module_pin_base::get_input_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_input[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_output_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_output[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_bool_input_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_bool_input[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_uint_input_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_uint_input[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_wide_input_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_wide_input[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_bool_output_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_bool_output[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_uint_output_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_uint_output[set_id];
}



set<string> xtsc_component::xtsc_module_pin_base::get_wide_output_set(u32 set_id) const {
  confirm_valid_set_id(set_id);
  return *m_vector_set_wide_output[set_id];
}



xtsc::u32 xtsc_component::xtsc_module_pin_base::get_bit_width(const string& port_name) const {
  map_string_u32::const_iterator im = m_map_port_size.find(port_name);
  if (im == m_map_port_size.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_bit_width(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_in/sc_out named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return im->second;
}



bool_input& xtsc_component::xtsc_module_pin_base::get_bool_input(const string& port_name) const {
  map_bool_input::const_iterator ii = m_map_bool_input.find(port_name);
  if (ii == m_map_bool_input.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_bool_input(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_in<bool> named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return *(ii->second);
}



uint_input& xtsc_component::xtsc_module_pin_base::get_uint_input(const string& port_name) const {
  map_uint_input::const_iterator ii = m_map_uint_input.find(port_name);
  if (ii == m_map_uint_input.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_uint_input(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_in<sc_uint_base> named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return *(ii->second);
}



wide_input& xtsc_component::xtsc_module_pin_base::get_wide_input(const string& port_name) const {
  map_wide_input::const_iterator ii = m_map_wide_input.find(port_name);
  if (ii == m_map_wide_input.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_wide_input(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_in<sc_bv_base> named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return *(ii->second);
}



bool_output& xtsc_component::xtsc_module_pin_base::get_bool_output(const string& port_name) const {
  map_bool_output::const_iterator ii = m_map_bool_output.find(port_name);
  if (ii == m_map_bool_output.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_bool_output(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_out<bool> named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return *(ii->second);
}



uint_output& xtsc_component::xtsc_module_pin_base::get_uint_output(const string& port_name) const {
  map_uint_output::const_iterator ii = m_map_uint_output.find(port_name);
  if (ii == m_map_uint_output.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_uint_output(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_out<sc_uint_base> named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return *(ii->second);
}



wide_output& xtsc_component::xtsc_module_pin_base::get_wide_output(const string& port_name) const {
  map_wide_output::const_iterator ii = m_map_wide_output.find(port_name);
  if (ii == m_map_wide_output.end()) {
    ostringstream oss;
    oss << "xtsc_module_pin_base::get_wide_output(): " << m_sc_module.kind() << " '" << m_sc_module.name()
        << "' has no sc_out<sc_bv_base> named '" << port_name << "'";
    throw xtsc_exception(oss.str());
  }
  return *(ii->second);
}



void xtsc_component::xtsc_module_pin_base::dump_ports(ostream& os, u32 set_id) {
  if (set_id != 0xFFFFFFFF) confirm_valid_set_id(set_id);

  // Save state of stream
  char c = os.fill(' ');
  ios::fmtflags old_flags = os.flags();

  for (u32 i=0; i<m_num_sets; ++i) {
    if ((set_id != 0xFFFFFFFF) && (set_id != i)) continue;
    if (set_id == 0xFFFFFFFF) {
      os << "Port set_id=" << i << ":" << endl;
    }
    for (set<string>::const_iterator ii = m_vector_set_input[i]->begin(); ii != m_vector_set_input[i]->end(); ++ii) {
      string port_name = *ii;
      u32    port_size = get_bit_width(port_name);

      if (m_map_bool_input.find(port_name) != m_map_bool_input.end()){
        os <<  "sc_in <bool>         ";
      }
      else if (m_map_uint_input.find(port_name) != m_map_uint_input.end()){
        os << "sc_in <sc_uint<" << setw(3) << port_size << "> >";
      }
      else {
        os << "sc_in <sc_bv  <" << setw(3) << port_size << "> >";
      }
      os << " " << port_name << ";" << endl;
    }
    for (set<string>::const_iterator io = m_vector_set_output[i]->begin(); io != m_vector_set_output[i]->end(); ++io) {
      string port_name = *io;
      u32    port_size = get_bit_width(port_name);

      if (m_map_bool_output.find(port_name) != m_map_bool_output.end()){
        os <<  "sc_out<bool>         ";
      }
      else if (m_map_uint_output.find(port_name) != m_map_uint_output.end()){
        os << "sc_out<sc_uint<" << setw(3) << port_size << "> >";
      }
      else {
        os << "sc_out<sc_bv  <" << setw(3) << port_size << "> >";
      }
      os << " " << port_name << ";" << endl;
    }
  }

  // Restore state of stream
  os.fill(c);
  os.flags(old_flags);
}



void xtsc_component::xtsc_module_pin_base::req_cntl::dump(ostream& os) const {
  u32   value   = m_value.to_uint();
  u32   type    = (value & m_type_mask);
  u32   row     = ((value & m_last_mask) ? 1 : 0);
  char  flag    = ((value & m_last_mask) ? '*' : ' ');
  u32   bl_num  = (value & m_bl_num_transfers_mask) >> m_num_transfers_shift;
  static const char *nums[2][4] = {
    { "2  ", "4  ", "8  ", "16 " },
    { "2* ", "4* ", "8* ", "16*" },
  };
  switch (type) {
    case READ:            os << "Rd" << flag << "   "; break;
    case WRITE:           os << "Wr" << flag << "   "; break;
    case RCW:             os << "RCW" << flag << "  "; break;
    case BLOCK_READ:      os << "BlR" << nums[row][bl_num] << ""; break;
    case BLOCK_WRITE:     os << "BlW" << nums[row][bl_num] << ""; break;
    case BURST_READ:      os << "BuR" << get_num_transfers() << flag << " "; break;
    case BURST_WRITE:     os << "BuW" << get_num_transfers() << flag << " "; break;
    case SNOOP_SHARED:    os << "SnS" << flag << "  "; break;
    case SNOOP_EXCLUSIVE: os << "SnE" << flag << "  "; break;
    default:              os << "Unk" << flag << "  "; break;
  }
}



void xtsc_component::xtsc_module_pin_base::resp_cntl::dump(ostream& os) const {
  u32 value = m_value.to_uint();
  if (m_snoop) {
    os << ((value & m_data_mask  ) ? "D" : "N")         // D=Data       N=No data
       << ((value & m_shared_mask) ? "S" : "I")         // S=Shared     I=Invalid
       << ((value & m_last_mask  ) ? "*" : " ");        // *=last       
  }
  else {
    u32   status  = ((value & m_status_mask) >> m_status_shift);
    char  last    = ((value & m_last_mask) ? '*' : ' ');
    switch (status) {
      case OK:    os << "OK" << last << "    "; break;
      case AErr:  os << "AErr" << last << "  "; break;
      case DErr:  os << "DErr" << last << "  "; break;
      case ADErr: os << "ADErr" << last << " "; break;
    }
  }
}



namespace xtsc_component {

ostream& operator<<(ostream& os, const xtsc_module_pin_base::req_cntl& req) {
  req.dump(os);
  return os;
}



ostream& operator<<(ostream& os, const xtsc_module_pin_base::resp_cntl& resp) {
  resp.dump(os);
  return os;
}

}




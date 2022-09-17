// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cstdlib>
#include <ostream>
#include <string>
#include <xtsc_sd/xtsc_sd.h>
#include "xtsc_ahb2pif_bridge_sd.h"

using namespace std;
using namespace xtsc;
using namespace xtsc_sd;



u32 xtsc_ahb2pif_bridge_sd::response_info::m_num_created = 0;
u32 xtsc_ahb2pif_bridge_sd::request_info::m_num_created = 0;

static const u32 READ_ID = 0x3F;




xtsc_ahb2pif_bridge_sd::xtsc_ahb2pif_bridge_sd(sc_mx_m_base* c, const sc_module_name &module_name) :
  sc_mx_import_module           (c, module_name, "xtsc_ahb2pif_bridge_sd"),
  m_p_ahb_slave_port            (new sc_mx_transaction_if_impl("m_p_ahb_slave_port", *this)),
  m_inbound_pif_request_port    ("m_inbound_pif_request_port"),
  m_inbound_pif_respond_export  ("m_inbound_pif_respond_export"),
  m_respond_impl                ("m_respond_impl", *this),
  m_text                        (log4xtensa::TextLogger::getInstance(getInstanceName())),
  m_response_fifo               ("m_response_fifo", 16)
{

  m_p_address_range_stream      = NULL;
  m_p_signals                   = NULL;

  m_init_complete               = false;

  m_pif_byte_width              = 0;
  m_ahb_byte_width              = 0;
  m_big_endian                  = false;
  m_start_byte_address          = 0;
  m_bridge_byte_size            = 0;
  m_address_range_file          = "";
  m_pif_clock_period            = 0;
  m_request_phase               = 0;
  m_nacc_wait_time              = 0;
  m_max_burst_size              = 0xFFFFFFFF;
  m_priority                    = 3;
  m_wait_write_response         = true;


  defineParameter("pif_byte_width",             "4",            MX_PARAM_VALUE,  0);
  defineParameter("ahb_byte_width",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("big_endian",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("start_byte_address",         "0",            MX_PARAM_VALUE,  0);
  defineParameter("bridge_byte_size",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("address_range_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("pif_clock_period",           "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("request_phase",              "400",          MX_PARAM_VALUE,  0);
  defineParameter("nacc_wait_time",             "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("max_burst_size",             "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("priority",                   "3",            MX_PARAM_VALUE,  0);
  defineParameter("wait_write_response",        "true",         MX_PARAM_BOOL,   0);


  registerPort(m_p_ahb_slave_port, m_p_ahb_slave_port->getName());

  // Do this so we have a clock slave port
  sc_mx_clocked();
  registerPort(dynamic_cast<sc_mx_clock_slave_p_base*>(this), "clk-in");

  registerSCGenericMasterPort(&m_inbound_pif_request_port,     "m_inbound_pif_request_port");
  registerSCGenericSlavePort (&m_inbound_pif_respond_export,   "m_inbound_pif_respond_export");

  m_inbound_pif_respond_export(m_respond_impl);

  xtsc_set_text_logging_time_precision(3);

}



xtsc_ahb2pif_bridge_sd::~xtsc_ahb2pif_bridge_sd(void) {
}



// The only valid values for size are 1, 2, and 4*n
void xtsc_ahb2pif_bridge_sd::xmemcpy(void *dst, const void *src, u32 size) {
  if (m_big_endian && (size > 1)) {
    if (size == 2) {
      u8 *d = (u8 *) dst;
      const u8 *s = (const u8 *) src;
      d[0] = s[1];
      d[1] = s[0];
    }
    else {
      if (size & 0x3) {
        stringstream oss;
        oss << "xtsc_ahb2pif_bridge_sd '" << getInstanceName() << "': Invalid size=" << size << " in call to xmemcpy";
        throw xtsc_exception(oss.str());
      }
      u32 *ds = (u32 *) dst;
      const u32 *sr = (const u32 *) src;
      u32 chunks = size/4;
      for (u32 i=0; i<chunks; ++i) {
        u8 *d = (u8 *) &ds[i];
        const u8 *s = (const u8 *) &sr[i];
        for (int j=0, k=3; j<4; ++j, --k) {
          d[k] = s[j];
        }
      }
    }
  }
  else {
    memcpy(dst, src, size);
  }
}



string xtsc_ahb2pif_bridge_sd::getProperty(MxPropertyType property) {
  string description; 
  switch (property) {    
    case MX_PROP_LOADFILE_EXTENSION:
           return "";
    case MX_PROP_REPORT_FILE_EXT:
           return "yes";
    case MX_PROP_COMPONENT_TYPE:
           return "Other"; 
    case MX_PROP_COMPONENT_VERSION:
           return "0.2";
    case MX_PROP_MSG_PREPEND_NAME:
           return "yes"; 
    case MX_PROP_DESCRIPTION:
           description = "xtsc_ahb2pif_bridge_sd";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



void xtsc_ahb2pif_bridge_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_ahb2pif_bridge_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }


  if (name == "pif_byte_width") {
    status = MxConvertStringToValue(value, &m_pif_byte_width);
  }
  else if (name == "ahb_byte_width") {
    status = MxConvertStringToValue(value, &m_ahb_byte_width);
  }
  else if (name == "big_endian") {
    status = MxConvertStringToValue(value, &m_big_endian);
  }
  else if (name == "start_byte_address") {
    status = MxConvertStringToValue(value, &m_start_byte_address);
  }
  else if (name == "bridge_byte_size") {
    status = MxConvertStringToValue(value, &m_bridge_byte_size);
  }
  else if (name == "address_range_file") {
    m_address_range_file = value;
  }
  else if (name == "pif_clock_period") {
    status = MxConvertStringToValue(value, &m_pif_clock_period);
  }
  else if (name == "request_phase") {
    status = MxConvertStringToValue(value, &m_request_phase);
  }
  else if (name == "nacc_wait_time") {
    status = MxConvertStringToValue(value, &m_nacc_wait_time);
  }
  else if (name == "max_burst_size") {
    status = MxConvertStringToValue(value, &m_max_burst_size);
  }
  else if (name == "priority") {
    u32 temp = 0;
    status = MxConvertStringToValue(value, &temp);
    m_priority = (u8) temp;
  }
  else if (name == "wait_write_response") {
    status = MxConvertStringToValue(value, &m_wait_write_response);
  }



  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_ahb2pif_bridge_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



void xtsc_ahb2pif_bridge_sd::init() {
  xtsc_sd_initialize();
  XTSC_DEBUG(m_text, "in xtsc_ahb2pif_bridge_sd::init()");

  m_time_resolution_sct = sc_get_time_resolution();

  SC_THREAD(request_thread);

  m_init_complete = true;

  // Do we have an address range file?
  if (m_address_range_file != "") {
    m_p_address_range_stream = new xtsc_script_file(m_address_range_file.c_str(), "\"address_range_file\"",
                                                    getInstanceName().c_str(), getName().c_str(), false);
    while (get_words() != 0) {
      xtsc_address low_address = get_u32(0, "<LowAddr>");
      xtsc_address high_address = get_u32(1, "<HighAddr>");
      string range_name = (m_words.size() > 2) ? m_words[2] : getInstanceName();
      m_address_ranges.push_back(new address_range(low_address, high_address, range_name));
      XTSC_INFO(m_text, "Address Range: 0x" << hex << setfill('0') << setw(8) << low_address << "-0x" << setw(8) << high_address <<
                        " " << range_name);
    }
  }
  else {
    xtsc_address low_address = m_start_byte_address;
    xtsc_address high_address = 0xFFFFFFFF;
    if (m_bridge_byte_size) {
      if ((0xFFFFFFFF - low_address) > m_bridge_byte_size) {
        high_address = low_address + m_bridge_byte_size;
      }
    }
    string range_name = getInstanceName();
    m_address_ranges.push_back(new address_range(low_address, high_address, range_name));
    XTSC_INFO(m_text, "Address Range: 0x" << hex << setfill('0') << setw(8) << low_address << "-0x" << setw(8) << high_address <<
                      " " << range_name);
  }

  if ((m_pif_byte_width != 4) && (m_pif_byte_width != 8) && (m_pif_byte_width != 16)) {
    ostringstream oss;
    oss << "xtsc_ahb2pif_bridge_sd '" << getInstanceName() << "': \"pif_byte_width\"=" << m_pif_byte_width << " is not supported.";
    throw xtsc_exception(oss.str());
  }

  u32 ahb_byte_width = m_ahb_byte_width;
  if (m_ahb_byte_width == 0) {
    m_ahb_byte_width = m_pif_byte_width;
  }

  if ((m_ahb_byte_width != 4) && (m_ahb_byte_width != 8) && (m_ahb_byte_width != 16)) {
    ostringstream oss;
    oss << "xtsc_ahb2pif_bridge_sd '" << getInstanceName() << "': \"ahb_byte_width\"=" << m_ahb_byte_width << " is not supported.";
    throw xtsc_exception(oss.str());
  }

  // Get PIF clock period 
  if (m_pif_clock_period == 0xFFFFFFFF) {
    m_pif_clock_period_sct = xtsc_get_system_clock_period();
  }
  else {
    m_pif_clock_period_sct = m_time_resolution_sct * m_pif_clock_period;
  }
  m_pif_clock_period_value = m_pif_clock_period_sct.value();

  // Get PIF clock phase for submitting requests to PIF 
  if (m_request_phase == 0xFFFFFFFF) {
    m_request_phase_sct = SC_ZERO_TIME;
  }
  else {
    m_request_phase_sct = m_time_resolution_sct * m_request_phase;
  }
  m_request_phase_plus_one_sct = m_request_phase_sct + m_pif_clock_period_sct;

  // Get RSP_NACC wait time
  if (m_nacc_wait_time == 0xFFFFFFFF) {
    m_nacc_wait_time_sct = m_pif_clock_period_sct;
  }
  else {
    m_nacc_wait_time_sct = m_time_resolution_sct * m_nacc_wait_time;
    if (m_nacc_wait_time_sct > m_pif_clock_period_sct) {
      ostringstream oss;
      oss << getName() << " '" << getInstanceName() << "': \"nacc_wait_time\" of " << m_nacc_wait_time_sct
          << " exceeds clock period of " << m_pif_clock_period_sct;
      throw xtsc_exception(oss.str());
    }
  }

  // This needs to be in init() so m_ahb_byte_width is set.  Supposedly this is legal.  
  // See SDDG para. 3.4.1:  "Port properties should not change after the reset stage".
  MxTransactionProperties props;
  AHB_INIT_TRANSACTION_PROPERTIES(props);
  props.supportsBurst = true;
  props.dataBitwidth = m_ahb_byte_width*8;
  m_p_ahb_slave_port->setProperties(&props);

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_ahb2pif_bridge_sd '" << getInstanceName() << "':");
  XTSC_LOG(m_text, ll,        " pif_byte_width          = "   << m_pif_byte_width);
  XTSC_LOG(m_text, ll,        " ahb_byte_width          = "   << ahb_byte_width << " => " << m_ahb_byte_width);
  XTSC_LOG(m_text, ll,        " big_endian              = "   << boolalpha << m_big_endian);
  XTSC_LOG(m_text, ll, hex << " start_byte_address      = 0x" << m_start_byte_address);
  XTSC_LOG(m_text, ll, hex << " bridge_byte_size        = 0x" << m_bridge_byte_size << (m_bridge_byte_size ? " " : " (4GB)"));
  XTSC_LOG(m_text, ll,        " address_range_file      = "   << m_address_range_file);
  if (m_pif_clock_period == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, hex << " pif_clock_period        = 0x" << m_pif_clock_period << " (" << m_pif_clock_period_sct << ")");
  } else {
  XTSC_LOG(m_text, ll,        " pif_clock_period        = "   << m_pif_clock_period << " (" << m_pif_clock_period_sct << ")");
  }
  if (m_request_phase == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, hex << " request_phase           = 0x" << m_request_phase << " (no not synchronize)");
  } else {
  XTSC_LOG(m_text, ll,        " request_phase           = "   << m_request_phase << " (" << m_request_phase_sct << ")");
  }
  if (m_nacc_wait_time == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, hex << " nacc_wait_time          = 0x" << m_nacc_wait_time << " (" << m_nacc_wait_time_sct << ")");
  } else {
  XTSC_LOG(m_text, ll,        " nacc_wait_time          = "   << m_nacc_wait_time << " (" << m_nacc_wait_time_sct << ")");
  }
  XTSC_LOG(m_text, ll, hex << " max_burst_size          = 0x" << m_max_burst_size);
  XTSC_LOG(m_text, ll,        " priority                = "   << (u32) m_priority);
  XTSC_LOG(m_text, ll,        " wait_write_response     = "   << boolalpha << m_wait_write_response);


}



void xtsc_ahb2pif_bridge_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  XTSC_INFO(m_text, kind() << "::reset()");

  m_line                        = "";
  m_line_count                  = 0;
  m_waiting_for_nacc            = false;
  m_request_got_nacc            = false;
  m_write_pending               = false;
  m_final_write_got_rsp_ok      = false;
  m_tag                         = 0;
  m_num_outstanding_requests    = 0;
  m_addr_phase_read             = false;
  m_htrans_apr                  = 0;
  m_hsize_apr                   = 0;
  m_wrapx_apr                   = false;
  m_factor_apr                  = 0;
  m_incr_apr                    = false;
  m_single_apr                  = false;
  m_haddr_apr                   = 0;
  m_p_request_info              = NULL;
  m_p_response_info             = NULL;
  m_current_pif_read_size8      = 0;
  m_read_buffer_offset          = 0;
  m_factor_dpw                  = 0;

  sc_mx_import_module::reset(level, filelist);
}



void xtsc_ahb2pif_bridge_sd::terminate() {
  XTSC_DEBUG(m_text, "In xtsc_ahb2pif_bridge_sd::terminate()");
  sc_mx_import_module::terminate();
  xtsc_finalize();
}



void xtsc_ahb2pif_bridge_sd::interconnect() {
  XTSC_DEBUG(m_text, "in xtsc_ahb2pif_bridge_sd::interconnect()");
  getClockMaster()->registerClockSlave(this, MX_PHASE_UPDATE);
}



int xtsc_ahb2pif_bridge_sd::get_words() {
  m_line_count = m_p_address_range_stream->get_words(m_words, m_line);
  XTSC_DEBUG(m_text, "get_words(): " << m_line);
  return m_words.size();
}




u32 xtsc_ahb2pif_bridge_sd::get_u32(u32 index, const string& argument_name) {
  u32 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in file '" << m_address_range_file << "' on line #"
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtou32(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number in file '"
        << m_address_range_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  return value;
}



xtsc_ahb2pif_bridge_sd::request_info *xtsc_ahb2pif_bridge_sd::new_request_info() {
  XTSC_DEBUG(m_text, "In new_request_info and pool is " << (m_request_info_pool.empty() ? "empty" : "not empty"));
  if (m_request_info_pool.empty()) {
    request_info *p_request_info = new request_info();
    return p_request_info;
  }
  else {
    request_info *p_request_info = m_request_info_pool.back();
    m_request_info_pool.pop_back();
    p_request_info->initialize();
    return p_request_info;
  }
}



void xtsc_ahb2pif_bridge_sd::delete_request_info(request_info*& p_request_info) {
  XTSC_DEBUG(m_text, "In delete_request_info");
  m_request_info_pool.push_back(p_request_info);
  p_request_info = 0;
}



xtsc_ahb2pif_bridge_sd::response_info *xtsc_ahb2pif_bridge_sd::new_response_info(const xtsc_response& response) {
  request_info *p_request_info = NULL;
  if (m_response_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new response_info");
    return new response_info(response, p_request_info);
  }
  else {
    response_info *p_response_info = m_response_pool.back();
    m_response_pool.pop_back();
    p_response_info->initialize(response, p_request_info);
    return p_response_info;
  }
}



void xtsc_ahb2pif_bridge_sd::delete_response_info(response_info*& p_response_info) {
  m_response_pool.push_back(p_response_info);
  p_response_info = 0;
}



void xtsc_ahb2pif_bridge_sd::request_thread() {
  try {
    while (true) {
      XTSC_DEBUG(m_text, "request_thread calling wait(m_requests_pending_event)");
      wait(m_requests_pending_event);
      XTSC_DEBUG(m_text, "request_thread woke up");
      while (!m_request_info_deque.empty()) {
        request_info *p_request_info = m_request_info_deque.front();
        m_request_info_deque.pop_front();
        p_request_info->m_request.set_priority(m_priority);
        u32 tries = 0;
        bool is_read = (p_request_info->m_request.get_id() == READ_ID);
        do {
          // Aligned to the user-specified clock phase
          if (m_request_phase != 0xFFFFFFFF) {
            sc_time now = sc_time_stamp();
            sc_time phase_now = (now.value() % m_pif_clock_period_value) * m_time_resolution_sct;
            if (phase_now > m_request_phase_sct) {
              wait(m_request_phase_plus_one_sct - phase_now);
            }
            else if (phase_now < m_request_phase_sct) {
              wait(m_request_phase_sct - phase_now);
            }
            XTSC_DEBUG(m_text, "Aligned to user-specified clock phase");
          }
          tries += 1;
          XTSC_INFO(m_text, p_request_info->m_request << " Try #" << tries);
          m_request_got_nacc = false;
          m_waiting_for_nacc = true;
          m_inbound_pif_request_port->nb_request(p_request_info->m_request);
          if (is_read) {
            wait(m_last_read_response_event);
          }
          else if (m_wait_write_response && p_request_info->m_request.get_last_transfer()) {
            wait(m_write_response_event);
            if (!m_request_got_nacc && (m_num_outstanding_requests == 1)) {
              m_final_write_got_rsp_ok = true;
            }
          }
          else {
            wait(m_nacc_wait_time_sct);
          }
          m_waiting_for_nacc = false;
        } while (m_request_got_nacc);
        delete_request_info(p_request_info);
        m_num_outstanding_requests -= 1;
      }
    }
  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in SC_THREAD of " << getName() << " '" << getInstanceName() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, log4xtensa::FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



void xtsc_ahb2pif_bridge_sd::update() {
  XTSC_TRACE(m_text, "update(): m_response_fifo.num_free()=" << m_response_fifo.num_free() <<
                     " request_info::m_num_created=" << request_info::m_num_created <<
                     " response_info::m_num_created=" << response_info::m_num_created <<
                     " m_addr_phase_read=" << m_addr_phase_read << " m_p_signals->hready=" << m_p_signals->hready <<
                     " m_num_outstanding_requests=" << m_num_outstanding_requests <<
                     " m_htrans_apr=" << m_htrans_apr <<
                     " m_incr_apr=" << m_incr_apr <<
                     " m_single_apr=" << m_single_apr);
  if (m_addr_phase_read && m_p_signals->hready && ((m_htrans_apr == AHB_TRANS_NONSEQ) || m_incr_apr || m_single_apr)) {
    m_tag = 0;
    u32 total_size = m_hsize_apr * m_factor_apr;
    xtsc_address aligned_address8 = (m_haddr_apr & ~(total_size - 1));
    bool is_total_size_aligned = ((m_haddr_apr ^ aligned_address8) == 0);
    u32 transfers_per_request = min((u32)16, (is_total_size_aligned ? ((total_size + m_pif_byte_width - 1) / m_pif_byte_width) : 1));
    bool do_block_read = (transfers_per_request > 1);
    xtsc_request::type_t type = (do_block_read ? xtsc_request::BLOCK_READ : xtsc_request::READ);
    m_current_pif_read_size8 = (((total_size >= m_pif_byte_width) && is_total_size_aligned) ? m_pif_byte_width :
                                                                                              min(m_hsize_apr, m_pif_byte_width));
    u32 num_requests = total_size / (transfers_per_request * m_current_pif_read_size8);
    xtsc_address address8 = m_haddr_apr;
    XTSC_VERBOSE(m_text, "NONSEQ/INCR read: address8=0x" << hex << address8 <<
                         " aligned_address8=" << aligned_address8 <<
                         " m_wrapx_apr=" << boolalpha << m_wrapx_apr <<
                         " m_hsize_apr=" << dec << m_hsize_apr <<
                         " m_factor_apr=" << dec << m_factor_apr <<
                         " total_size=" << dec << total_size <<
                         " m_current_pif_read_size8=" << dec << m_current_pif_read_size8 <<
                         " is_total_size_aligned=" << boolalpha << is_total_size_aligned <<
                         " do_block_read=" << boolalpha << do_block_read <<
                         " num_requests=" << num_requests <<
                         " transfers_per_request=" << transfers_per_request);
    for (u32 i=0; i<num_requests; ++i) {
      request_info *p_request_info = new_request_info();
      p_request_info->m_request.initialize(type, address8, m_current_pif_read_size8, m_tag, transfers_per_request);
      if (m_tag == 0) {
        m_tag = p_request_info->m_request.get_tag();
      }
      p_request_info->m_request.set_id(READ_ID);
      m_request_info_deque.push_back(p_request_info);
      m_num_outstanding_requests += 1;
      XTSC_DEBUG(m_text, "Queueing PIF request: " << p_request_info->m_request);
      address8 += (m_current_pif_read_size8 * transfers_per_request);
      if (m_wrapx_apr && (address8 >= aligned_address8 + total_size)) {
        address8 = aligned_address8;
      }
    }
    m_requests_pending_event.notify(SC_ZERO_TIME);
    XTSC_DEBUG(m_text, "Calling m_requests_pending_event.notify()");
  }
  m_addr_phase_read = false;
}



MxStatus xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::read(MxU64 addr, MxU32* value, MxU32* ctrl) {
  MxStatus status = MX_STATUS_OK;
  ctrl[AHB_IDX_ACK] = AHB_ACK_DONE;
  u32 htrans = AHB_ACC_DECODE_HTRANS(ctrl[AHB_IDX_ACC]);
  if (ctrl[AHB_IDX_CYCLE] == AHB_CYCLE_ADDR) {
    if ((htrans != AHB_TRANS_IDLE) && (htrans != AHB_TRANS_BUSY)) {
      u32 hburst = AHB_ACC_DECODE_HBURST(ctrl[AHB_IDX_ACC]);
      m_bridge.m_addr_phase_read = true;
      m_bridge.m_htrans_apr = htrans;
      m_bridge.m_haddr_apr = static_cast<xtsc_address>(addr);
      m_bridge.m_hsize_apr = xtsc_sd_get_ctrl_size(ctrl);
      m_bridge.m_factor_apr = xtsc_sd_get_ctrl_burst_factor(ctrl);
      m_bridge.m_incr_apr = (hburst == AHB_BURST_INCR);
      m_bridge.m_single_apr = (hburst == AHB_BURST_SINGLE);
      m_bridge.m_wrapx_apr = ((hburst == AHB_BURST_WRAP4) || (hburst == AHB_BURST_WRAP8) || (hburst == AHB_BURST_WRAP16));
      u32 total_size = m_bridge.m_hsize_apr * m_bridge.m_factor_apr;
      if (total_size > m_bridge.m_max_burst_size) {
        status = MX_STATUS_ERROR;
        ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
        ostringstream oss;
        oss << "Total burst size (" << total_size << ") exceeds \"max_burst_size\" ("
            << m_bridge.m_max_burst_size << ") in read() method of module '" << m_bridge.getInstanceName() << "'.";
        throw xtsc_exception(oss.str());
      }
    }
  }
  else {
    if ((htrans != AHB_TRANS_IDLE) && (htrans != AHB_TRANS_BUSY)) {
      xtsc_address  address8 = static_cast<xtsc_address>(addr);
      u8           *buf      = reinterpret_cast<u8 *>(value);
      u32           size8    = xtsc_sd_get_ctrl_size(ctrl);
      u32           mask     = size8 - 1;
      if (xtsc_is_text_logging_enabled() && m_bridge.m_text.isEnabledFor(log4xtensa::DEBUG_LOG_LEVEL)) {
        ostringstream oss;
        xtsc_hex_dump(16, buf, oss);
        XTSC_DEBUG(m_bridge.m_text, "buf before read: [0x" << hex << setfill('0') << setw(8) << address8 << "/" << size8 <<
                                   "]= " << oss.str());
      }
      if (size8 < 4) {
        memset(buf, 0, 4);
      }
      if (address8 & mask) {
        status = MX_STATUS_ERROR;
        ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
        ostringstream oss;
        oss << "Illegal address/size in read() method of module '" << m_bridge.getInstanceName() << "' address=0x" << hex
            << address8 << " size=" << dec << size8;
        throw xtsc_exception(oss.str());
      }
      else {
        // Do we have a partially consumed response_info object or a response_info object in m_response_info?
        if (m_bridge.m_p_response_info || m_bridge.m_response_fifo.nb_read(m_bridge.m_p_response_info)) {
          if (m_bridge.m_p_response_info->m_response.get_status() != xtsc_response::RSP_OK) {
            status = MX_STATUS_ERROR;
            ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
            ostringstream oss;
            oss << "Module '" << m_bridge.getInstanceName() << "' received response: " << m_bridge.m_p_response_info->m_response;
            throw xtsc_exception(oss.str());
          }
          else {
            u8 *buffer = m_bridge.m_p_response_info->m_response.get_buffer();
            if (size8 > m_bridge.m_current_pif_read_size8) {
              // PIF is narrower then AHB and we have to accumulate multiple PIF read responses to satisfy this one AHB beat
              m_bridge.xmemcpy(&m_bridge.m_read_buffer[m_bridge.m_read_buffer_offset], buffer, m_bridge.m_current_pif_read_size8);
              m_bridge.m_read_buffer_offset += m_bridge.m_current_pif_read_size8;
              // Have we got enough read data back from the PIF?
              if (m_bridge.m_read_buffer_offset >= size8) {
                m_bridge.xmemcpy(buf, m_bridge.m_read_buffer, size8);
                m_bridge.m_read_buffer_offset = 0;
              }
              else {
                status = MX_STATUS_WAIT;
              }
              m_bridge.delete_response_info(m_bridge.m_p_response_info);
            }
            else {
              u32 offset = m_bridge.m_p_response_info->m_buffer_offset;
              m_bridge.xmemcpy(buf, &buffer[offset], size8);
              m_bridge.m_p_response_info->m_buffer_offset += size8;
              if (m_bridge.m_p_response_info->m_buffer_offset >= m_bridge.m_current_pif_read_size8) {
                m_bridge.delete_response_info(m_bridge.m_p_response_info);
              }
            }
          }
        }
        else {
          status = MX_STATUS_WAIT;
        }
      }
    }
  }
  XTSC_DEBUG(m_bridge.m_text, "read()  " << xtsc_sd_get_ctrl_lock_string(ctrl) <<
                              " addr=0x" << hex << setfill('0') << setw(8) << (u32)addr <<
                              " " << xtsc_sd_get_ctrl_type_string(ctrl) << " " << xtsc_sd_get_ctrl_cycle_string(ctrl) <<
                              " " << xtsc_sd_get_ctrl_ack_string(ctrl) << " " << dec << xtsc_sd_get_ctrl_size(ctrl) <<
                              " " << xtsc_sd_get_ctrl_burst_string(ctrl) << " " << xtsc_sd_get_ctrl_trans_string(ctrl) <<
                              " " << xtsc_sd_get_status_string(status));
  return status;
}



MxStatus xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::write(MxU64 addr, MxU32* value, MxU32* ctrl) {
  MxStatus status = MX_STATUS_OK;
  ctrl[AHB_IDX_ACK] = AHB_ACK_DONE;
  u32 htrans = AHB_ACC_DECODE_HTRANS(ctrl[AHB_IDX_ACC]);
  if ((ctrl[AHB_IDX_CYCLE] == AHB_CYCLE_DATA) && (htrans != AHB_TRANS_IDLE) && (htrans != AHB_TRANS_BUSY)) {
    xtsc_address  address8 = static_cast<xtsc_address>(addr);
    u8           *buf      = reinterpret_cast<u8 *>(value);
    u32           size8    = xtsc_sd_get_ctrl_size(ctrl);
    u32           mask     = size8 - 1;
    if (address8 & mask) {
      status = MX_STATUS_ERROR;
      ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
      ostringstream oss;
      oss << "Illegal address/size in write() method of module '" << m_bridge.getInstanceName() << "' address=0x" << hex
          << address8 << " size=" << dec << size8;
      throw xtsc_exception(oss.str());
    }
    else {
      XTSC_DEBUG(m_bridge.m_text, "m_write_pending=" << boolalpha << m_bridge.m_write_pending <<
                                 " m_num_outstanding_requests=" << m_bridge.m_num_outstanding_requests);
      if (m_bridge.m_write_pending) {
        if ((!m_bridge.m_wait_write_response && (m_bridge.m_num_outstanding_requests == 0)) ||
            ( m_bridge.m_wait_write_response && m_bridge.m_final_write_got_rsp_ok))
        {
          m_bridge.m_write_pending = false;
          m_bridge.m_final_write_got_rsp_ok = false;
        }
        else {
          status = MX_STATUS_WAIT;
        }
      }
      else {
        if (htrans == AHB_TRANS_NONSEQ) {
          m_bridge.m_tag = 0;
          m_bridge.m_factor_dpw = xtsc_sd_get_ctrl_burst_factor(ctrl);
          m_bridge.m_write_total_size = size8 * m_bridge.m_factor_dpw;
          if (m_bridge.m_write_total_size > m_bridge.m_max_burst_size) {
            status = MX_STATUS_ERROR;
            ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
            ostringstream oss;
            oss << "Total burst size (" << m_bridge.m_write_total_size << ") exceeds \"max_burst_size\" ("
                << m_bridge.m_max_burst_size << ") in write() method of module '" << m_bridge.getInstanceName() << "'.";
            throw xtsc_exception(oss.str());
          }
          u32 is_total_size_aligned = ((address8 & (m_bridge.m_write_total_size - 1)) == 0);
          if (is_total_size_aligned) {
            m_bridge.m_write_num_transfers = (m_bridge.m_write_total_size + m_bridge.m_pif_byte_width - 1) / m_bridge.m_pif_byte_width;
            m_bridge.m_write_transfer_size = ((m_bridge.m_write_num_transfers == 1) ? m_bridge.m_write_total_size :
                                                                                      m_bridge.m_pif_byte_width);
          }
          else {
            m_bridge.m_write_transfer_size = min(size8, m_bridge.m_pif_byte_width);
            m_bridge.m_write_num_transfers = m_bridge.m_write_total_size / m_bridge.m_write_transfer_size;
          }
          bool do_block_write = (m_bridge.m_write_num_transfers > 1) && is_total_size_aligned;
          m_bridge.m_write_type = (do_block_write ? xtsc_request::BLOCK_WRITE : xtsc_request::WRITE);
          m_bridge.m_write_num_blocks = 1;
          if (m_bridge.m_write_num_transfers > 16) {
            m_bridge.m_write_num_blocks = m_bridge.m_write_num_transfers / 16;
            m_bridge.m_write_num_transfers = 16;
          }
          m_bridge.m_write_transfer_count = 0;
          XTSC_DEBUG(m_bridge.m_text, "NONSEQ: address8=0x" << hex << address8 <<
                                     " m_write_total_size=" << dec << m_bridge.m_write_total_size <<
                                     " m_write_num_transfers=" << m_bridge.m_write_num_transfers <<
                                     " do_block_write=" << boolalpha << do_block_write <<
                                     " m_write_num_blocks=" << m_bridge.m_write_num_blocks <<
                                     " m_write_transfer_size=" << m_bridge.m_write_transfer_size);
        }

        bool last_transfer = (m_bridge.m_write_type == xtsc_request::WRITE) ||
                             (m_bridge.m_write_transfer_count + 1 == m_bridge.m_write_num_transfers);
        if (m_bridge.m_p_request_info == NULL) {
          u32 num_transfers = ((m_bridge.m_write_type == xtsc_request::WRITE) ? 1 : m_bridge.m_write_num_transfers);
          m_bridge.m_p_request_info = m_bridge.new_request_info();
          if ((m_bridge.m_write_transfer_count == 0) || (m_bridge.m_write_type == xtsc_request::WRITE)) {
            m_bridge.m_p_request_info->m_request.initialize(m_bridge.m_write_type, address8, m_bridge.m_write_transfer_size,
                                                            m_bridge.m_tag, num_transfers, 0xFFFF, last_transfer);
            if (m_bridge.m_tag == 0) {
              m_bridge.m_tag = m_bridge.m_p_request_info->m_request.get_tag();
            }
          }
          else {
            m_bridge.m_p_request_info->m_request.initialize(m_bridge.m_tag, address8, m_bridge.m_write_transfer_size,
                                                            num_transfers, last_transfer);
          }
        }

        // How many PIF requests worth of data does this AHB beat give us?
        u32 num_bytes = m_bridge.m_p_request_info->m_buffer_offset + size8;
        u32 num_pif_requests = num_bytes / m_bridge.m_write_transfer_size;
        XTSC_DEBUG(m_bridge.m_text, "num_pif_requests=" << num_pif_requests << " num_bytes=" << num_bytes << " size8=" << size8);

        // How many bytes from the AHB beat should be copied to the nascent request?
        u32 space_available = m_bridge.m_write_transfer_size - m_bridge.m_p_request_info->m_buffer_offset;
        u32 num_bytes_to_copy = (size8 < space_available) ? size8 : space_available;
        u8 *buffer = m_bridge.m_p_request_info->m_request.get_buffer();
        m_bridge.xmemcpy(&buffer[m_bridge.m_p_request_info->m_buffer_offset], buf, num_bytes_to_copy);
        m_bridge.m_p_request_info->m_buffer_offset += num_bytes_to_copy;

        // Do we have enough data for at least one complete PIF request?
        if (num_pif_requests > 0) {
          for (u32 i=0; i<num_pif_requests; ++i) {
            // Create the request (skip on first pass 'cuz it's already been done)
            if (i != 0) {
              address8 += m_bridge.m_write_transfer_size;
              XTSC_DEBUG(m_bridge.m_text, "address8=0x" << hex << address8);
              m_bridge.m_p_request_info = m_bridge.new_request_info();
              if (m_bridge.m_write_type == xtsc_request::WRITE) {
                m_bridge.m_p_request_info->m_request.initialize(xtsc_request::WRITE, address8, m_bridge.m_write_transfer_size,
                                                                m_bridge.m_tag);
              }
              else {
                last_transfer = (m_bridge.m_write_transfer_count + 1 == m_bridge.m_write_num_transfers);
                m_bridge.m_p_request_info->m_request.initialize(m_bridge.m_tag, address8, m_bridge.m_write_transfer_size,
                                                                m_bridge.m_write_num_transfers, last_transfer);
              }
              buffer = m_bridge.m_p_request_info->m_request.get_buffer();
              m_bridge.xmemcpy(buffer, &buf[num_bytes_to_copy*i], num_bytes_to_copy);
            }
            m_bridge.m_request_info_deque.push_back(m_bridge.m_p_request_info);
            m_bridge.m_num_outstanding_requests += 1;
            XTSC_DEBUG(m_bridge.m_text, "Queueing PIF request: " << m_bridge.m_p_request_info->m_request);
            m_bridge.m_write_transfer_count += 1;
            if (m_bridge.m_write_transfer_count == m_bridge.m_write_num_transfers) {
              m_bridge.m_write_transfer_count = 0;
            }
            m_bridge.m_p_request_info = NULL;
          }
          m_bridge.m_requests_pending_event.notify(SC_ZERO_TIME);
          XTSC_DEBUG(m_bridge.m_text, "Calling m_requests_pending_event.notify(); last_transfer=" << boolalpha << last_transfer <<
                                     " num_pif_requests=" << num_pif_requests);
          if (last_transfer) {
            status = MX_STATUS_WAIT;
            m_bridge.m_write_pending = true;
          }
        }
      }
    }
  }
  XTSC_DEBUG(m_bridge.m_text, "write() " << xtsc_sd_get_ctrl_lock_string(ctrl) <<
                              " addr=0x" << hex << setfill('0') << setw(8) << (u32)addr <<
                              " " << xtsc_sd_get_ctrl_type_string(ctrl) << " " << xtsc_sd_get_ctrl_cycle_string(ctrl) <<
                              " " << xtsc_sd_get_ctrl_ack_string(ctrl) << " " << dec << xtsc_sd_get_ctrl_size(ctrl) <<
                              " " << xtsc_sd_get_ctrl_burst_string(ctrl) << " " << xtsc_sd_get_ctrl_trans_string(ctrl) <<
                              " " << xtsc_sd_get_status_string(status));
  return status;
}



MxStatus xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::readDbg(MxU64 addr, MxU32* value, MxU32* ctrl) {
  try {
    xtsc_address address8 = static_cast<xtsc_address>(addr);
    u8          *buf      = reinterpret_cast<u8 *>(value);
    u32          size8     = 1;
    switch (ctrl[AHB_IDX_TYPE]) {
      case AHB_TYPE_BYTE:   size8 =  1; break;
      case AHB_TYPE_HWORD:  size8 =  2; break;
      case AHB_TYPE_WORD:   size8 =  4; break;
      case AHB_TYPE_DWORD:  size8 =  8; break;
      case AHB_TYPE_128BIT: size8 = 16; break;
      default: return MX_STATUS_ERROR;
    }
    if (m_bridge.m_big_endian) {
      u8 b[16];
      m_bridge.m_inbound_pif_request_port->nb_peek(address8, size8, b);
      m_bridge.xmemcpy(buf, b, size8);
    }
    else {
      m_bridge.m_inbound_pif_request_port->nb_peek(address8, size8, buf);
    }
  }
  catch (...) {
    return MX_STATUS_ERROR;
  }
  return MX_STATUS_OK;
}



MxStatus xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::writeDbg(MxU64 addr, MxU32* value, MxU32* ctrl) {
  try {
    xtsc_address address8 = static_cast<xtsc_address>(addr);
    u8           *buf     = reinterpret_cast<u8 *>(value);
    u32           size8    = 1;
    switch (ctrl[AHB_IDX_TYPE]) {
      case AHB_TYPE_BYTE:   size8 =  1; break;
      case AHB_TYPE_HWORD:  size8 =  2; break;
      case AHB_TYPE_WORD:   size8 =  4; break;
      case AHB_TYPE_DWORD:  size8 =  8; break;
      case AHB_TYPE_128BIT: size8 = 16; break;
      default: return MX_STATUS_ERROR;
    }
    if (m_bridge.m_big_endian) {
      u8 b[16];
      m_bridge.xmemcpy(b, buf, size8);
      m_bridge.m_inbound_pif_request_port->nb_poke(address8, size8, b);
    }
    else {
      m_bridge.m_inbound_pif_request_port->nb_poke(address8, size8, buf);
    }
  }
  catch (...) {
    return MX_STATUS_ERROR;
  }
  return MX_STATUS_OK;
}



MxStatus xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::readReq(MxU64                     /*addr*/,
                                                                    MxU32                   * /*value*/,
                                                                    MxU32                   * ctrl,
                                                                    MxTransactionCallbackIF * /*callback*/)
{
  if (ctrl) {
    m_bridge.m_p_signals = reinterpret_cast<TAHBSignals*>(ctrl);
    return MX_STATUS_OK;
  }
  return MX_STATUS_NOTSUPPORTED;
}



void xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::register_port(sc_core::sc_port_base& port, const char * /*if_typename*/) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_ahb2pif_bridge_sd '" << m_bridge.getInstanceName() << "' m_p_ahb_slave_port: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_bridge.m_text, "Binding '" << port.name() << "' to xtsc_ahb2pif_bridge_sd::m_p_ahb_slave_port");
  m_p_port = &port;
}



int xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::getNumRegions() {
  XTSC_DEBUG(m_bridge.m_text, "in xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::getNumRegions()");
  return m_bridge.m_address_ranges.size();
}



void xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::getAddressRegions(MxU64* start, MxU64* size, std::string* name) {
  XTSC_DEBUG(m_bridge.m_text, "in xtsc_ahb2pif_bridge_sd::sc_mx_transaction_if_impl::getAddressRegions()");
  for (u32 i=0; i<m_bridge.m_address_ranges.size(); ++i) {
    start[i] = m_bridge.m_address_ranges[i]->m_start_address;
    size[i]  = m_bridge.m_address_ranges[i]->m_byte_size;
    name[i]  = m_bridge.m_address_ranges[i]->m_name;
  }
}



bool xtsc_ahb2pif_bridge_sd::xtsc_respond_if_impl::nb_respond(const xtsc_response& response) {
  XTSC_INFO(m_bridge.m_text, response);
  if (!m_bridge.m_response_fifo.num_free()) {
    ostringstream oss;
    oss << "xtsc_ahb2pif_bridge_sd '" << m_bridge.getInstanceName() << "' received response but response fifo is full: " << response << endl;
    throw xtsc_exception(oss.str());
  }
  xtsc_response::status_t status = response.get_status();
  if (status == xtsc_response::RSP_NACC) {
    if (m_bridge.m_waiting_for_nacc) {
      m_bridge.m_request_got_nacc = true;
      if (response.get_id() == READ_ID) {
        m_bridge.m_last_read_response_event.notify(SC_ZERO_TIME);
      }
      else if (m_bridge.m_wait_write_response) {
        m_bridge.m_write_response_event.notify(SC_ZERO_TIME);
      }
      return true;
    }
    else {
      ostringstream oss;
      oss << "xtsc_ahb2pif_bridge_sd '" << m_bridge.getInstanceName() << "' received nacc too late: " << response << endl;
      oss << " - Possibly something is wrong with the downstream device" << endl;
      oss << " - Possibly this bridge's \"nacc_wait_time\" needs to be adjusted";
      throw xtsc_exception(oss.str());
    }
  }
  else if (status != xtsc_response::RSP_OK) {
    ostringstream oss;
    oss << "xtsc_ahb2pif_bridge_sd '" << m_bridge.getInstanceName() << "' received PIF error response: " << response << endl;
    throw xtsc_exception(oss.str());
  }
  if (response.get_id() == READ_ID) {
    // This is a response to a READ|BLOCK_READ
    response_info *p_response_info = m_bridge.new_response_info(response);
    m_bridge.m_response_fifo.nb_write(p_response_info);
    if (response.get_last_transfer()) {
      m_bridge.m_last_read_response_event.notify(SC_ZERO_TIME);
    }
  }
  else {
    // This is a response to a WRITE|BLOCK_WRITE
    if (m_bridge.m_wait_write_response) {
      m_bridge.m_write_response_event.notify(SC_ZERO_TIME);
    }
  }
  return true;
}



void xtsc_ahb2pif_bridge_sd::xtsc_respond_if_impl::register_port(sc_core::sc_port_base& port, const char * /*if_typename*/) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_ahb2pif_bridge_sd '" << m_bridge.getInstanceName()
        << "' m_inbound_pif_respond_export: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_bridge.m_text, "Binding '" << port.name() << "' to xtsc_ahb2pif_bridge_sd::m_inbound_pif_respond_export");
  m_p_port = &port;
}



class xtsc_ahb2pif_bridge_sdFactory : public MxFactory {
public:
  xtsc_ahb2pif_bridge_sdFactory() : MxFactory ("xtsc_ahb2pif_bridge_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_ahb2pif_bridge_sd(c, id.c_str()); 
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_ahb2pif_bridge_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}





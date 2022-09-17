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
#include "xtsc_ahb_memory_sd.h"

using namespace std;
using namespace xtsc;
using namespace xtsc_sd;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;
using log4xtensa::DEBUG_LOG_LEVEL;






xtsc_ahb_memory_sd::xtsc_ahb_memory_sd(sc_mx_m_base* c, const string &s) :
  sc_mx_module          (c, s.c_str()),
  m_p_ahb_slave_port    (new sc_mx_transaction_if_impl("m_p_ahb_slave_port", *this)),
  m_text                (log4xtensa::TextLogger::getInstance(getInstanceName()))
{

  m_p_request_stream            = NULL;
  m_p_signals                   = NULL;
  m_init_complete               = false;


  m_byte_width                  = 4;
  m_big_endian                  = false;
  m_start_byte_address          = 0;
  m_memory_byte_size            = 0;
  m_page_byte_size              = 0x4000;
  m_initial_value_file          = "";
  m_memory_fill_byte            = 0;
  m_read_only                   = false;
  m_script_file                 = "";
  m_wraparound                  = false;


  defineParameter("byte_width",                 "4",            MX_PARAM_VALUE,  0);
  defineParameter("big_endian",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("start_byte_address",         "0",            MX_PARAM_VALUE,  0);
  defineParameter("memory_byte_size",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("page_byte_size",             "0x4000",       MX_PARAM_VALUE,  0);
  defineParameter("initial_value_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("memory_fill_byte",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("read_only",                  "false",        MX_PARAM_BOOL,   0);
  defineParameter("script_file",                "",             MX_PARAM_STRING, 0);
  defineParameter("wraparound",                 "false",        MX_PARAM_BOOL,   0);


  registerPort(m_p_ahb_slave_port, m_p_ahb_slave_port->getName());

  // Do this so we have a clock slave port
  sc_mx_clocked();
  registerPort(dynamic_cast<sc_mx_clock_slave_p_base*>(this), "clk-in");
}



xtsc_ahb_memory_sd::~xtsc_ahb_memory_sd(void) {
  clear_addresses();
}



void xtsc_ahb_memory_sd::xmemcpy(void *dst, const void *src, u32 size) {
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
        oss << "xtsc_ahb_memory_sd '" << getInstanceName() << "': Invalid size=" << size << " in call to xmemcpy";
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



string xtsc_ahb_memory_sd::getProperty(MxPropertyType property) {
  string description; 
  switch (property) {    
    case MX_PROP_LOADFILE_EXTENSION:
           return "";
    case MX_PROP_REPORT_FILE_EXT:
           return "yes";
    case MX_PROP_COMPONENT_TYPE:
           return "Memory"; 
    case MX_PROP_COMPONENT_VERSION:
           return "0.1";
    case MX_PROP_MSG_PREPEND_NAME:
           return "yes"; 
    case MX_PROP_DESCRIPTION:
           description = "xtsc_ahb_memory_sd";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



void xtsc_ahb_memory_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_ahb_memory_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "byte_width") {
    status = MxConvertStringToValue(value, &m_byte_width);
  }
  else if (name == "big_endian") {
    status = MxConvertStringToValue(value, &m_big_endian);
  }
  else if (name == "start_byte_address") {
    status = MxConvertStringToValue(value, &m_start_byte_address);
  }
  else if (name == "memory_byte_size") {
    status = MxConvertStringToValue(value, &m_memory_byte_size);
  }
  else if (name == "page_byte_size") {
    status = MxConvertStringToValue(value, &m_page_byte_size);
  }
  else if (name == "initial_value_file") {
    m_initial_value_file = value;
  }
  else if (name == "memory_fill_byte") {
    u32 fill_byte;
    status = MxConvertStringToValue(value, &fill_byte);
    m_memory_fill_byte = (u8) (fill_byte & 0xFF);
  }
  else if (name == "read_only") {
    status = MxConvertStringToValue(value, &m_read_only);
  }
  else if (name == "script_file") {
    m_script_file = value;
  }
  else if (name == "wraparound") {
    status = MxConvertStringToValue(value, &m_wraparound);
  }



  if (status == MxConvert_SUCCESS) {
    sc_mx_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_ahb_memory_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



void xtsc_ahb_memory_sd::init() {
  xtsc_sd_initialize();
  XTSC_INFO(m_text, "in xtsc_ahb_memory_sd::init()");

  m_init_complete = true;

  if (m_script_file != "") {
    m_p_request_stream = new xtsc_script_file(m_script_file.c_str(), "\"script_file\"", getInstanceName().c_str(),
                                              getName().c_str(), m_wraparound);
  }

  m_end_byte_address = (m_memory_byte_size ? (m_start_byte_address + m_memory_byte_size - 1) : 0xFFFFFFFF);

  if (m_end_byte_address <= m_start_byte_address) {
    ostringstream oss;
    oss << "Invalid memory start_byte_address (0x" << hex << m_start_byte_address << ") or memory_byte_size (0x" << m_memory_byte_size 
        << ") in xtsc_ahb_memory_sd '" << getInstanceName() << "' constructor.";
    throw xtsc_exception(oss.str());
  }

  if ((m_byte_width != 4) && (m_byte_width != 8) && (m_byte_width != 16)) {
    ostringstream oss;
    oss << "Invalid \"byte_width\" parameter value (" << m_byte_width << ") in xtsc_ahb_memory_sd '" << getInstanceName()
        << "' (must be 4|8|16).";
    throw xtsc_exception(oss.str());
  }

  // This needs to be in init() so m_byte_width is set.  See SDDG para. 3.4.1.
  MxTransactionProperties props;
  AHB_INIT_TRANSACTION_PROPERTIES(props);
  props.supportsBurst = true;
  props.dataBitwidth = m_byte_width*8;
  m_p_ahb_slave_port->setProperties(&props);

  if (m_page_byte_size < 16*m_byte_width) {
    ostringstream oss;
    oss << "Memory page_byte_size (0x" << hex << m_page_byte_size << ") is too small (minimum is 0x" << 16*m_byte_width
        << ") in xtsc_ahb_memory_sd '" << getInstanceName() << "' constructor.";
    throw xtsc_exception(oss.str());
  }

  m_page_size8_log2 = 0;
  u32 shift_value = m_page_byte_size;
  for (u32 i=0; i<32; ++i) {
    if (shift_value & 0x1) m_page_size8_log2 = i;
    shift_value >>= 1;
  }
  if ((1U << m_page_size8_log2) != m_page_byte_size) {
    ostringstream oss;
    oss << "Memory page_byte_size (0x" << hex << m_page_byte_size << ") is not a power of 2 in xtsc_ahb_memory_sd '"
        << getInstanceName() << "' constructor.";
    throw xtsc_exception(oss.str());
  }
  XTSC_DEBUG(m_text, "m_page_size8_log2=" << m_page_size8_log2);

  if ((m_memory_byte_size == 0) || (m_memory_byte_size >= -m_page_byte_size)) {
    m_num_pages = (((0xFFFFFFFF - m_page_byte_size) + 1) >> m_page_size8_log2) + 1;
  }
  else {
    m_num_pages = (m_memory_byte_size + m_page_byte_size - 1) >> m_page_size8_log2;
  }
  m_page_table = new u8*[m_num_pages];

  for (u32 i=0; i<m_num_pages; i++) { m_page_table[i] = 0; }

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_ahb_memory_sd '" << getInstanceName() << "':");
  XTSC_LOG(m_text, ll,        " byte_width              = "   << m_byte_width);
  XTSC_LOG(m_text, ll,        " big_endian              = "   << boolalpha << m_big_endian);
  XTSC_LOG(m_text, ll, hex << " start_byte_address      = 0x" << m_start_byte_address);
  XTSC_LOG(m_text, ll, hex << " memory_byte_size        = 0x" << m_memory_byte_size << (m_memory_byte_size ? " " : " (4GB)"));
  XTSC_LOG(m_text, ll, hex << " End byte address        = 0x" << m_end_byte_address);
  XTSC_LOG(m_text, ll,        " read_only               = "   << boolalpha << m_read_only);
  XTSC_LOG(m_text, ll, hex << " page_byte_size          = 0x" << m_page_byte_size);
  XTSC_LOG(m_text, ll,        " initial_value_file      = "   << m_initial_value_file);
  XTSC_LOG(m_text, ll, hex << " memory_fill_byte        = 0x" << (u32) m_memory_fill_byte);
  XTSC_LOG(m_text, ll,        " script_file             = "   << m_script_file);
  XTSC_LOG(m_text, ll,        " wraparound              = "   << boolalpha << m_wraparound);


}



void xtsc_ahb_memory_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  XTSC_INFO(m_text, "xtsc_ahb_memory_sd::reset()");

  m_line                        = "";
  m_line_count                  = 0;
  m_transaction_count           = 0;
  m_hit_count                   = 0;
  m_miss_count                  = 0;
  m_cycle                       = 0ULL;
  m_waiting_cycle               = 0ULL;
  m_state                       = NEXT;
  m_count_limit                 = 0;
  m_waitstates                  = 0;
  m_no_waitstate                = false;

  // Initialize memory contents?
  if (m_initial_value_file != "") {
    xtsc_script_file    file(m_initial_value_file.c_str(), "initial_value_file", getInstanceName().c_str(), getName().c_str(), false);
    xtsc_address        offset = 0;
    u32                 line_count;
    vector<string>      words;
    string              line;
    while ((line_count = file.get_words(words, line)) != 0) {
      for (u32 i=0; i<words.size(); ++i) {
        string word = words[i];
        bool be_offset = (word[0] == '@');
        if (be_offset) {
          word = word.substr(1);
        }
        u32 value = 0;
        try {
          value = xtsc_strtou32(word);
        }
        catch (const xtsc_exception&) {
            ostringstream oss;
            oss << getName() << " '" << getInstanceName() << "':  Cannot convert word #" << i+1 << " '" << word
                << "' to number on line #" << line_count << " of \"initial_value_file\" '" << m_initial_value_file << "': " << line;
            throw xtsc_exception(oss.str());
        }
        if (be_offset) {
          offset = value;
        }
        else {
          if (value > 0xFF) {
            ostringstream oss;
            oss << getName() << " '" << getInstanceName() << "':  Value at word #" << i+1 << " '" << word
                << "' exceeds 255 (0xFF) on line #" << line_count << " of \"initial_value_file\" '" << m_initial_value_file
                << "': " << line;
            throw xtsc_exception(oss.str());
          }
          if (m_memory_byte_size && (offset >= m_memory_byte_size)) {
            ostringstream oss;
            oss << getName() << " '" << getInstanceName() << "':  Memory offset (0x" << hex << offset << ") for value at word #"
                << dec << i+1 << " '" << word << "' violates \"memory_byte_size\" (0x" << hex << m_memory_byte_size << ") on line #"
                << dec << line_count << " of \"initial_value_file\" '" << m_initial_value_file << "': " << line;
            throw xtsc_exception(oss.str());
          }
          u8 buffer = static_cast<u8>(value);
          poke(m_start_byte_address + offset++, 1, &buffer);
        }
      }
    }
  }

  if (m_p_request_stream) {
    m_p_request_stream->reset();
  }

  sc_mx_module::reset(level, filelist);
}



void xtsc_ahb_memory_sd::terminate() {
  XTSC_INFO(m_text, "In xtsc_ahb_memory_sd::terminate()");
  sc_mx_module::terminate();
  xtsc_finalize();
}



void xtsc_ahb_memory_sd::peek(xtsc_address address8, u32 size8, u8 *buffer, bool log) {
  if ((address8 < m_start_byte_address) || ((address8 + size8 - 1) > m_end_byte_address)) {
    ostringstream oss;
    oss << "Memory access out-of-range (address=0x" << hex << address8 << " and byte size=0x" << size8 
        << ") in peek() of memory '" << getInstanceName() << "' (Valid range: 0x" << m_start_byte_address
        << "-0x" << m_end_byte_address << ").";
    throw xtsc_exception(oss.str());
  }
  xtsc_address  addr8      = address8;
  u32           first_page = get_page(addr8);
  u32           last_page  = get_page(addr8 + size8 - 1);
  u32           bytes_left = size8;
  u32           buf_offset = 0;
  for (u32 page=first_page; page <= last_page; ++page) {
    u32 mem_offset  = get_page_offset(addr8);
    u32 mem_rem     = m_page_byte_size - mem_offset;
    u32 chunk8      = min(mem_rem, bytes_left);
    for (u32 i = 0; i<chunk8; i++) {
      buffer[buf_offset] = *(m_page_table[page]+mem_offset);
      buf_offset += 1;
      mem_offset += 1;
    }
    addr8       += chunk8;
    bytes_left  -= chunk8;
  }

  if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(VERBOSE_LOG_LEVEL)) {
    buf_offset  = 0;
    ostringstream oss;
    oss << hex << setfill('0');
    for (u32 i = 0; i<size8; i++) {
      oss << setw(2) << (u32) buffer[buf_offset] << " ";
      buf_offset += 1;
    }
    if (log) {
      XTSC_VERBOSE(m_text, "peek: " << " [0x" << hex << address8 << "/" << size8 << "] = " << oss.str());
    }
    else {
      XTSC_DEBUG(m_text, "peek: " << " [0x" << hex << address8 << "/" << size8 << "] = " << oss.str());
    }
  }

}



void xtsc_ahb_memory_sd::poke(xtsc_address address8, u32 size8, const u8 *buffer, bool log) {
  if ((address8 < m_start_byte_address) || ((address8 + size8 - 1) > m_end_byte_address)) {
    ostringstream oss;
    oss << "Memory access out-of-range (address=0x" << hex << address8 << " and byte size=0x" << size8 
        << ") in poke() of memory '" << getInstanceName() << "' (Valid range: 0x" << m_start_byte_address
        << "-0x" << m_end_byte_address << ").";
    throw xtsc_exception(oss.str());
  }
  xtsc_address  addr8      = address8;
  u32           first_page = get_page(addr8);
  u32           last_page  = get_page(addr8 + size8 - 1);
  u32           bytes_left = size8;
  u32           buf_offset = 0;
  for (u32 page=first_page; page <= last_page; ++page) {
    u32 mem_offset  = get_page_offset(addr8);
    u32 mem_rem     = m_page_byte_size - mem_offset;
    u32 chunk8      = min(mem_rem, bytes_left);
    for (u32 i = 0; i<chunk8; i++) {
    *(m_page_table[page]+mem_offset) = buffer[buf_offset];
      buf_offset += 1;
      mem_offset += 1;
    }
    addr8       += chunk8;
    bytes_left  -= chunk8;
  }

  if (xtsc_is_text_logging_enabled() && m_text.isEnabledFor(VERBOSE_LOG_LEVEL)) {
    buf_offset  = 0;
    ostringstream oss;
    oss << hex << setfill('0');
    for (u32 i = 0; i<size8; i++) {
      oss << setw(2) << (u32) buffer[buf_offset] << " ";
      buf_offset += 1;
    }
    if (log) {
      XTSC_VERBOSE(m_text, "poke: " << " [0x" << hex << address8 << "/" << size8 << "] = " << oss.str());
    }
    else {
      XTSC_DEBUG(m_text, "poke: " << " [0x" << hex << address8 << "/" << size8 << "] = " << oss.str());
    }
  }

}



void xtsc_ahb_memory_sd::byte_dump(xtsc_address        address8,
                                   u32                 size8,
                                   ostream&            os,
                                   bool                left_to_right,
                                   u32                 bytes_per_line,
                                   bool                show_address,
                                   bool                show_hex_values,
                                   bool                do_column_heading,
                                   bool                show_ascii_values,
                                   bool                adjust_address)
{
  u8 *buffer = new u8[size8];
  peek(address8, size8, buffer);
  u32  initial_skipped_bytes = ((adjust_address && (bytes_per_line != 0)) ? (address8 % bytes_per_line) : 0);
  address8 -= initial_skipped_bytes;
  xtsc_hex_dump(left_to_right, size8, buffer, os, bytes_per_line, show_address, address8, show_hex_values,
                do_column_heading, show_ascii_values, initial_skipped_bytes);
  delete [] buffer;
}



u32 xtsc_ahb_memory_sd::get_page(xtsc_address address8) {
  // Check range
  if ((address8 < m_start_byte_address) || (address8 > m_end_byte_address)) {
    ostringstream oss;
    oss << "Memory access out-of-range (address=0x" << hex << address8 
        << ") in xtsc_ahb_memory_sd::get_page() of memory '" << getInstanceName() << "' (Valid range: 0x" << m_start_byte_address
        << "-0x" << m_end_byte_address << ").";
    throw xtsc_exception(oss.str());
  }
  // Allocate memory page if required
  u32 page = get_page_id(address8);
  if (!m_page_table[page]) {
    m_page_table[page] = new u8[m_page_byte_size];
    memset(m_page_table[page], m_memory_fill_byte, m_page_byte_size);
    XTSC_DEBUG(m_text, "Created page #" << page);
  }
  return page;
}



void xtsc_ahb_memory_sd::interconnect() {
  XTSC_INFO(m_text, "in xtsc_ahb_memory_sd::interconnect(): m_p_request_stream=" << m_p_request_stream);
  if (m_p_request_stream) {
    getClockMaster()->registerClockSlave(this, MX_PHASE_UPDATE);
  }
}



void xtsc_ahb_memory_sd::communicate() {
  // Should not be called because we didn't register for it
}



void xtsc_ahb_memory_sd::update() {
  // Should only be called if we have a "script_file"
  bool loop_again;
  do {
    loop_again = false;
    switch (m_state) {
      case NEXT: {
        if (get_words() != 0) {
          if (m_words[0] == "wait") {
            if (m_words.size() < 2) {
              ostringstream oss;
              oss << "WAIT command is missing arguments in file '" << m_script_file << "' on line #" << m_line_count << ": " << endl;
              oss << m_line;
              throw xtsc_exception(oss.str());
            }
            if (m_words[1] == "tran"        ||
                m_words[1] == "trans"       ||
                m_words[1] == "transaction" ||
                m_words[1] == "hit"         ||
                m_words[1] == "miss")
            {
              u32 count = ((m_words.size() <= 2) ?  1 : get_u32(2, "count"));
              if (count == 0) {
                ostringstream oss;
                oss << "WAIT command count cannot be zero in file '" << m_script_file << "' on line #" << m_line_count << ": " << endl;
                oss << m_line;
                throw xtsc_exception(oss.str());
              }
              if (m_words[1] == "hit") {
                m_count_limit = count + m_hit_count;
                m_state       = WAIT_HIT;
              }
              else if (m_words[1] == "miss") {
                m_count_limit = count + m_miss_count;
                m_state       = WAIT_MISS;
              }
              else {
                m_count_limit = count + m_transaction_count;
                m_state       = WAIT_TRANS;
              }
              continue;
            }
            u32 duration = get_u32(1, "duration");
            m_waiting_cycle = m_cycle + duration;
            m_state = WAIT_TIME;
            loop_again = true;  // Allows a duration of 0
            XTSC_DEBUG(m_text, "waiting " << duration);
            break;
          }
          else if ((m_words[0] == "sync") || (m_words[0] == "synchronize")) {
            if (m_words.size() != 2) {
              ostringstream oss;
              oss << "SYNC command has missing/extra arguments in file '" << m_script_file << "' on line #" << m_line_count
                  << ": " << endl;
              oss << m_line;
              throw xtsc_exception(oss.str());
            }
            u32 cycle = get_u32(1, "cycle");
            m_waiting_cycle = cycle;
            m_state = WAIT_TIME;
            loop_again = true;  // Allows a sync to the current or earlier time to mean now
            XTSC_DEBUG(m_text, "sync to cycle " << cycle);
          }
          else if (m_words[0] == "info") {
            XTSC_INFO(m_text, m_line);
          }
          else if (m_words[0] == "note") {
            XTSC_NOTE(m_text, m_line);
          }
          else if ((m_words[1] == "stop")) {
            u32 delay = get_u32(0, "delay");
            m_waiting_cycle = m_cycle + delay;
            m_state = DELAY;
            loop_again = true;
            XTSC_DEBUG(m_text, "delaying " << delay);
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
                oss << "No entry for the address/address-range specified in file '" << m_script_file << "' on line #" 
                    << m_line_count << ": " << endl;
                oss << m_line;
                throw xtsc_exception(oss.str());
              }
              XTSC_INFO(m_text, "Clearing " << *im->second);
              p_map->erase(low_address);
            }
          }
          else if ((m_words.size() > 1) && ((m_words[1] == "done" ) ||
                                            (m_words[1] == "retry") ||
                                            (m_words[1] == "split") ||
                                            (m_words[1] == "wait" ) ||
                                            (m_words[1] == "abort")))
          {
            xtsc_address low_address;
            xtsc_address high_address;
            bool is_range = get_addresses(0, "address/address-range", low_address, high_address);
            map<xtsc_address, address_info*> *p_map = (is_range ? &m_address_range_map : &m_address_map);
            map<xtsc_address, address_info*>::iterator im = p_map->find(low_address);
            if (im != p_map->end()) {
              ostringstream oss;
              oss << "Duplicate entry for the address/address-range specified in file '" << m_script_file << "' on line #" 
                  << m_line_count << ": " << endl;
              oss << m_line;
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
                  oss << "File '" << m_script_file << "' on line #" << m_line_count << ": " << endl;
                  oss << m_line;
                  throw xtsc_exception(oss.str());
                }
              }
            }
            if ((low_address < m_start_byte_address) || (high_address > m_end_byte_address))
            {
              ostringstream oss;
              if (is_range) {
                oss << "Specified address range, 0x" << hex << low_address << "-0x" << high_address;
              }
              else {
                oss << "Specified address, 0x" << hex << low_address;
              }
              oss << ", is not within xtsc_ahb_memory_sd '" << getInstanceName() << "' (0x" << m_start_byte_address << "-0x"
                  << m_end_byte_address << ") in file '" << m_script_file << "' on line #" << m_line_count << ": " << endl;
              oss << m_line;
              throw xtsc_exception(oss.str());
            }
            AHB_ACK_TYPE ack_type;
                 if (m_words[1] == "done" ) ack_type = AHB_ACK_DONE;
            else if (m_words[1] == "retry") ack_type = AHB_ACK_RETRY;
            else if (m_words[1] == "split") ack_type = AHB_ACK_SPLIT;
            else if (m_words[1] == "wait" ) ack_type = AHB_ACK_WAIT;
            else if (m_words[1] == "abort") ack_type = AHB_ACK_ABORT;
            else  {
              ostringstream oss;
              oss << "Program Bug: in line " << __LINE__ << " of file " << __FILE__;
              throw xtsc_exception(oss.str());
            }
            u32 waitstates = 1;
            u32 limit = 1;
            if (m_words.size() > 2) {
              waitstates = get_u32(2, "waitstates");
              if (m_words.size() > 3) {
                limit = get_u32(3, "limit");
              }
            }
            if (ack_type == AHB_ACK_WAIT) {
              if (waitstates == 0) {
                ostringstream oss;
                oss << "waitstates cannot be 0 in file '" << m_script_file << "' in line #" 
                    << m_line_count << ": " << endl;
                oss << m_line;
                throw xtsc_exception(oss.str());
              }
            }
            else {
              waitstates = 0;
            }
            address_info *p_info = new address_info(low_address, high_address, is_range, ack_type, waitstates, limit);
            (*p_map)[low_address] = p_info;
          }
          else {
            ostringstream oss;
            oss << "Unrecognized command: '" << m_words[1] << "' in file '" << m_script_file << "' in line #" 
                << m_line_count << ": " << endl;
            oss << m_line;
            throw xtsc_exception(oss.str());
          }
        }
        else {
          m_state = DONE;
        }
        break;
      }
      case WAIT_TIME: {
        if (m_cycle >= m_waiting_cycle) {
          m_state = NEXT;
        }
        break;
      }
      case WAIT_HIT: {
        if (m_hit_count >= m_count_limit) {
          m_state = NEXT;
        }
        break;
      }
      case WAIT_MISS: {
        if (m_miss_count >= m_count_limit) {
          m_state = NEXT;
        }
        break;
      }
      case WAIT_TRANS: {
        if (m_transaction_count >= m_count_limit) {
          m_state = NEXT;
        }
        break;
      }
      case DELAY: {
        if (m_cycle >= m_waiting_cycle) {
          m_state = NEXT;
          if (m_words[1] == "stop") {
            XTSC_INFO(m_text, "calling sc_stop()");
            sc_stop();
          }
          else {
            ostringstream oss;
            oss << "Program Bug in line " << __LINE__ << " of file " << __FILE__;
            oss << "Processing command: " << m_line;
            throw xtsc_exception(oss.str());
          }
        }
        break;
      }
      case DONE: {
        break;
      }
      default: {
        throw xtsc_exception("Program Bug:  Invalid m_state in xtsc_ahb_memory_sd::update()");
      }
    }
  } while ((m_state == NEXT) || loop_again);
  m_cycle += 1;
}



int xtsc_ahb_memory_sd::get_words() {
  m_line_count = m_p_request_stream->get_words(m_words, m_line, true);
  XTSC_DEBUG(m_text, "get_words(): " << m_line);
  return m_words.size();
}




u32 xtsc_ahb_memory_sd::get_u32(u32 index, const string& argument_name) {
  u32 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in file '" << m_script_file << "' on line #"
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
        << m_script_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  return value;
}



double xtsc_ahb_memory_sd::get_double(u32 index, const string& argument_name) {
  double value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in file '" << m_script_file << "' on line #" 
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtod(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number in file '"
        << m_script_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  return value;
}



bool xtsc_ahb_memory_sd::get_addresses(u32 index, const string& argument_name, xtsc_address& low_address, xtsc_address& high_address) {
  bool is_range = false;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in file '" << m_script_file << "' on line #" 
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  string low  = m_words[index];
  string high = "";
  string::size_type pos = low.find_first_of("-");
  if (pos != string::npos) {
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
        oss << "highAddr (0x" << hex << high_address << ") cannot be less than low_address (0x" << low_address << ") in file '"
            << m_script_file << "' on line #" << m_line_count << ": " << endl;
        oss << m_line;
        throw xtsc_exception(oss.str());
      }
    }
    else {
      high_address = low_address;
    }
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to address(es) in file '"
        << m_script_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  return is_range;
}



void xtsc_ahb_memory_sd::clear_addresses() {
  if (m_init_complete) {
    XTSC_INFO(m_text, "Clearing all addresses");
  }
  for (u32 i=0; i<2; ++i) {
    map<xtsc_address, address_info*> *p_map = (i ? &m_address_range_map : &m_address_map);
    map<xtsc_address, address_info*>::iterator im;
    for (im = p_map->begin(); im != p_map->end(); ++im) {
      delete im->second;
    }
    p_map->clear();
  }
}



void xtsc_ahb_memory_sd::dump_addresses(std::ostream& os) {
  for (u32 i=0; i<2; ++i) {
    map<xtsc_address, address_info*> *p_map = (i ? &m_address_range_map : &m_address_map);
    map<xtsc_address, address_info*>::iterator im;
    for (im = p_map->begin(); im != p_map->end(); ++im) {
      os << *im->second << endl;
    }
  }
}



bool xtsc_ahb_memory_sd::do_special_response(xtsc_address address8, MxU32* ctrl, MxStatus& status) {
  bool doit = true;
  if (m_no_waitstate) {
    m_no_waitstate = false;
    m_hit_count += 1;
  }
  else if (m_waitstates) {
    ctrl[AHB_IDX_ACK] = AHB_ACK_WAIT;
    status = MX_STATUS_WAIT;
    m_waitstates -= 1;
    if (!m_waitstates) {
      m_no_waitstate = true;
    }
  }
  else {
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
    if (p_info) {
      XTSC_VERBOSE(m_text, "0x" << hex << setw(8) << setfill('0') << address8 << ": Special response: " << *p_info);
      ctrl[AHB_IDX_ACK] = p_info->m_response;
      if (p_info->m_response == AHB_ACK_ABORT) {
        status = MX_STATUS_ERROR;
        m_hit_count += 1;
      }
      else {
        if (p_info->m_response == AHB_ACK_WAIT) {
          status = MX_STATUS_WAIT;
          m_waitstates = p_info->m_waitstates - 1;
          if (!m_waitstates) {
            m_no_waitstate = true;
          }
        }
        else {
          m_hit_count += 1;
        }
      }
      if (p_info->m_response != AHB_ACK_DONE) {
        doit = false;
      }
      p_info->used();
    }
    else {
      m_miss_count += 1;
    }
  }
  return doit;
}



MxStatus xtsc_ahb_memory_sd::sc_mx_transaction_if_impl::read(MxU64 addr, MxU32* value, MxU32* ctrl) {
  MxStatus status = MX_STATUS_OK;
  ctrl[AHB_IDX_ACK] = AHB_ACK_DONE;
  u32 htrans = AHB_ACC_DECODE_HTRANS(ctrl[AHB_IDX_ACC]);
  if ((ctrl[AHB_IDX_CYCLE] == AHB_CYCLE_DATA) && (htrans != AHB_TRANS_IDLE) && (htrans != AHB_TRANS_BUSY)) {
    if (!m_memory.m_waitstates) {
      m_memory.m_transaction_count += 1;
    }
    xtsc_address  address8 = static_cast<xtsc_address>(addr);
    u8           *buf      = reinterpret_cast<u8 *>(value);
    u32           size8    = xtsc_sd_get_ctrl_size(ctrl);
    u32           mask     = size8 - 1;
    if (address8 & mask) {
      status = MX_STATUS_ERROR;
      ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
      m_memory.m_miss_count += 1;
    }
    else {
      if (m_memory.do_special_response(address8, ctrl, status)) {
        memset(buf, 0, 16);
        if (m_memory.m_big_endian) {
          u8 b[16];
          m_memory.peek(address8, size8, b);
          m_memory.xmemcpy(buf, b, size8);
        }
        else {
          m_memory.peek(address8, size8, buf);
        }
        if (xtsc_is_text_logging_enabled() && m_memory.m_text.isEnabledFor(INFO_LOG_LEVEL)) {
          u32 buf_offset  = 0;
          ostringstream oss;
          oss << hex << setfill('0');
          for (u32 i = 0; i<size8; ++i) {
            oss << setw(2) << (u32) buf[buf_offset] << " ";
            buf_offset += 1;
          }
          XTSC_INFO(m_memory.m_text, "read  [0x" << hex << setfill('0') << setw(8) << address8 << "/" << size8 << "] = " << oss.str());
        }
      }
    }
  }
  XTSC_DEBUG(m_memory.m_text, "read()  " << xtsc_sd_get_ctrl_lock_string(ctrl) <<
                              " addr=0x" << hex << setfill('0') << setw(8) << (u32)addr <<
                              " " << xtsc_sd_get_ctrl_type_string(ctrl) << " " << xtsc_sd_get_ctrl_cycle_string(ctrl) <<
                              " " << xtsc_sd_get_ctrl_ack_string(ctrl) << " " << dec << xtsc_sd_get_ctrl_size(ctrl) <<
                              " " << xtsc_sd_get_ctrl_burst_string(ctrl) << " " << xtsc_sd_get_ctrl_trans_string(ctrl) <<
                              " " << xtsc_sd_get_status_string(status));
  return status;
}



MxStatus xtsc_ahb_memory_sd::sc_mx_transaction_if_impl::write(MxU64 addr, MxU32* value, MxU32* ctrl) {
  MxStatus status = MX_STATUS_OK;
  ctrl[AHB_IDX_ACK] = AHB_ACK_DONE;
  u32 htrans = AHB_ACC_DECODE_HTRANS(ctrl[AHB_IDX_ACC]);
  if ((ctrl[AHB_IDX_CYCLE] == AHB_CYCLE_DATA) && (htrans != AHB_TRANS_IDLE) && (htrans != AHB_TRANS_BUSY)) {
    if (!m_memory.m_waitstates) {
      m_memory.m_transaction_count += 1;
    }
    xtsc_address  address8 = static_cast<xtsc_address>(addr);
    u8           *buf      = reinterpret_cast<u8 *>(value);
    u32           size8    = xtsc_sd_get_ctrl_size(ctrl);
    u32           mask     = size8 - 1;
    if (address8 & mask) {
      status = MX_STATUS_ERROR;
      ctrl[AHB_IDX_ACK] = AHB_ACK_ABORT;
      m_memory.m_miss_count += 1;
    }
    else {
      if (m_memory.do_special_response(address8, ctrl, status)) {
        if (m_memory.m_big_endian) {
          u8 b[16];
          m_memory.xmemcpy(b, buf, size8);
          m_memory.poke(address8, size8, b);
        }
        else {
          m_memory.poke(address8, size8, buf);
        }
        if (xtsc_is_text_logging_enabled() && m_memory.m_text.isEnabledFor(INFO_LOG_LEVEL)) {
          u32 buf_offset  = 0;
          ostringstream oss;
          oss << hex << setfill('0');
          for (u32 i = 0; i<size8; ++i) {
            oss << setw(2) << (u32) buf[buf_offset] << " ";
            buf_offset += 1;
          }
          XTSC_INFO(m_memory.m_text, "write [0x" << hex << setfill('0') << setw(8) << address8 << "/" << size8 << "] = " << oss.str());
        }
      }
    }
  }
  XTSC_DEBUG(m_memory.m_text, "write() " << xtsc_sd_get_ctrl_lock_string(ctrl) <<
                              " addr=0x" << hex << setfill('0') << setw(8) << (u32)addr <<
                              " " << xtsc_sd_get_ctrl_type_string(ctrl) << " " << xtsc_sd_get_ctrl_cycle_string(ctrl) <<
                              " " << xtsc_sd_get_ctrl_ack_string(ctrl) << " " << dec << xtsc_sd_get_ctrl_size(ctrl) <<
                              " " << xtsc_sd_get_ctrl_burst_string(ctrl) << " " << xtsc_sd_get_ctrl_trans_string(ctrl) <<
                              " " << xtsc_sd_get_status_string(status));
  return status;
}



MxStatus xtsc_ahb_memory_sd::sc_mx_transaction_if_impl::readDbg(MxU64 addr, MxU32* value, MxU32* ctrl) {
  try {
    xtsc_address address8 = static_cast<xtsc_address>(addr);
    u8           *buf     = reinterpret_cast<u8 *>(value);
    u32           size    = 1;
    switch (ctrl[AHB_IDX_TYPE]) {
      case AHB_TYPE_BYTE:   size =  1; break;
      case AHB_TYPE_HWORD:  size =  2; break;
      case AHB_TYPE_WORD:   size =  4; break;
      case AHB_TYPE_DWORD:  size =  8; break;
      case AHB_TYPE_128BIT: size = 16; break;
      default: return MX_STATUS_ERROR;
    }
    if (m_memory.m_big_endian) {
      u8 b[16];
      m_memory.peek(address8, size, b, true);
      m_memory.xmemcpy(buf, b, size);
    }
    else {
      m_memory.peek(address8, size, buf, true);
    }
  }
  catch (...) {
    return MX_STATUS_ERROR;
  }
  return MX_STATUS_OK;
}



MxStatus xtsc_ahb_memory_sd::sc_mx_transaction_if_impl::writeDbg(MxU64 addr, MxU32* value, MxU32* ctrl) {
  try {
    xtsc_address address8 = static_cast<xtsc_address>(addr);
    u8           *buf     = reinterpret_cast<u8 *>(value);
    u32           size    = 1;
    switch (ctrl[AHB_IDX_TYPE]) {
      case AHB_TYPE_BYTE:   size =  1; break;
      case AHB_TYPE_HWORD:  size =  2; break;
      case AHB_TYPE_WORD:   size =  4; break;
      case AHB_TYPE_DWORD:  size =  8; break;
      case AHB_TYPE_128BIT: size = 16; break;
      default: return MX_STATUS_ERROR;
    }
    if (m_memory.m_big_endian) {
      u8 b[16];
      m_memory.xmemcpy(b, buf, size);
      m_memory.poke(address8, size, b, true);
    }
    else {
      m_memory.poke(address8, size, buf, true);
    }
  }
  catch (...) {
    return MX_STATUS_ERROR;
  }
  return MX_STATUS_OK;
}



MxStatus xtsc_ahb_memory_sd::sc_mx_transaction_if_impl::readReq(MxU64                     /*addr*/,
                                                                MxU32                   * /*value*/,
                                                                MxU32                   * ctrl,
                                                                MxTransactionCallbackIF * /*callback*/)
{
  if (ctrl) {
    m_memory.m_p_signals = reinterpret_cast<TAHBSignals*>(ctrl);
    return MX_STATUS_OK;
  }
  return MX_STATUS_NOTSUPPORTED;
}



void xtsc_ahb_memory_sd::sc_mx_transaction_if_impl::register_port(sc_core::sc_port_base& port, const char * /*if_typename*/) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_ahb_memory_sd '" << m_memory.getInstanceName() << "' m_p_ahb_slave_port: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_memory.m_text, "Binding '" << port.name() << "' to xtsc_ahb_memory_sd::m_p_ahb_slave_port");
  m_p_port = &port;
}



void xtsc_ahb_memory_sd::address_info::dump(ostream& os) const {

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
  os << " " << xtsc_sd_get_ack_string(m_response) << " " << dec << m_waitstates << " " << m_count << "/";
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



bool xtsc_ahb_memory_sd::address_info::used() {
  m_count += 1;
  if (m_limit && m_count >= m_limit) {
    m_finished = true;
  }
  return m_finished;
}



std::ostream& operator<<(std::ostream& os, const xtsc_ahb_memory_sd::address_info& info) {
  info.dump(os);
  return os;
}



class xtsc_ahb_memory_sdFactory : public MxFactory {
public:
  xtsc_ahb_memory_sdFactory() : MxFactory ("xtsc_ahb_memory_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    return new xtsc_ahb_memory_sd(c, id);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_ahb_memory_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




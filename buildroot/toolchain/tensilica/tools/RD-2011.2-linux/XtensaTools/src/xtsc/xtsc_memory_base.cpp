// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cstdlib>
#include <ostream>
#include <string>
#include <xtsc/xtsc_memory_base.h>
#include <xtsc/xtsc_logging.h>

using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;
using log4xtensa::DEBUG_LOG_LEVEL;




xtsc_component::xtsc_memory_base::xtsc_memory_base(const char  *name, 
                                                   const char  *kind,
                                                   u32          byte_width,
                                                   u32          start_byte_address,
                                                   u32          memory_byte_size,
                                                   u32          page_byte_size,
                                                   const char  *initial_value_file,
                                                   u8           memory_fill_byte) :
  m_name                (name),
  m_kind                (kind),
  m_start_address8      (start_byte_address),
  m_size8               (memory_byte_size),
  m_width8              (byte_width),
  m_initial_value_file  (""),
  m_p_initial_value_file(NULL),
  m_memory_fill_byte    (memory_fill_byte),
  m_text                (log4xtensa::TextLogger::getInstance(m_name)),
  m_binary              (log4xtensa::BinaryLogger::getInstance(m_name)),
  m_page_size8          (page_byte_size)
{

  m_log_data_binary             = true;
  m_end_address8                = (m_size8 ? (m_start_address8 + m_size8 - 1) : 0xFFFFFFFF);

  if (m_end_address8 <= m_start_address8) {
    ostringstream oss;
    oss << "xtsc_memory_base: Invalid memory start_byte_address (0x" << hex << m_start_address8 << ") or memory_byte_size (0x"
        << m_size8 << ") in " << m_kind << " '" << m_name << "'.";
    throw xtsc_exception(oss.str());
  }

  if ((m_width8 != 0) && (m_width8 != 4) && (m_width8 != 8) && (m_width8 != 16) && (m_width8 != 32) && (m_width8 != 64)) {
    ostringstream oss;
    oss << "xtsc_memory_base: Invalid \"byte_width\" parameter value (" << m_width8 << ") in " << m_kind << " '" << m_name
        << "' (must be 0|4|8|16|32|64).";
    throw xtsc_exception(oss.str());
  }

  u32 width8 = (m_width8 ? m_width8 : xtsc_max_bus_width8);
  if (m_page_size8 < 16*width8) {
    ostringstream oss;
    oss << "xtsc_memory_base: Memory page_byte_size (0x" << hex << m_page_size8 << ") is too small (minimum is 0x" << 16*width8
        << ") in " << m_kind << " '" << m_name << "'.";
    throw xtsc_exception(oss.str());
  }

  m_page_size8_log2 = 0;
  u32 shift_value = m_page_size8;
  for (u32 i=0; i<32; ++i) {
    if (shift_value & 0x1) m_page_size8_log2 = i;
    shift_value >>= 1;
  }
  if ((1U << m_page_size8_log2) != m_page_size8) {
    ostringstream oss;
    oss << "xtsc_memory_base: Memory page_byte_size (0x" << hex << m_page_size8 << ") is not a power of 2 in " << m_kind << " '"
        << m_name << "'.";
    throw xtsc_exception(oss.str());
  }
  XTSC_DEBUG(m_text, "m_page_size8_log2=" << m_page_size8_log2);

  if ((m_size8 == 0) || (m_size8 >= -m_page_size8)) {
    m_num_pages = (((0xFFFFFFFF - m_page_size8) + 1) >> m_page_size8_log2) + 1;
  }
  else {
    m_num_pages = (m_size8 + m_page_size8 - 1) >> m_page_size8_log2;
  }
  m_page_table = new u8*[m_num_pages];

  for (u32 i=0; i<m_num_pages; ++i) { m_page_table[i] = 0; }

  // Initialize memory contents?
  if (initial_value_file && initial_value_file[0]) {
    m_initial_value_file = initial_value_file;
    m_p_initial_value_file = new xtsc_script_file(m_initial_value_file.c_str(), "initial_value_file", m_name.c_str(),
                                                  m_kind.c_str(), false);
  }

}



xtsc_component::xtsc_memory_base::~xtsc_memory_base(void) {
}



void xtsc_component::xtsc_memory_base::load_initial_values() {
  if (m_p_initial_value_file) {
    xtsc_address        offset = 0;
    u32                 line_count;
    vector<string>      words;
    string              line;
    m_p_initial_value_file->reset();
    while ((line_count = m_p_initial_value_file->get_words(words, line)) != 0) {
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
            oss << "Cannot convert word #" << i+1 << " '" << word << "' to number:" << endl;
            oss << line;
            oss << m_p_initial_value_file->info_for_exception();
            throw xtsc_exception(oss.str());
        }
        if (be_offset) {
          offset = value;
        }
        else {
          if (value > 0xFF) {
            ostringstream oss;
            oss << "Value at word #" << i+1 << " '" << word << "' exceeds 255 (0xFF):" << endl;
            oss << line;
            oss << m_p_initial_value_file->info_for_exception();
            throw xtsc_exception(oss.str());
          }
          if (m_size8 && (offset >= m_size8)) {
            ostringstream oss;
            oss << "Memory offset (0x" << hex << offset << ") for value at word #" << dec << i+1 << " '" << word
                << "' violates \"memory_byte_size\"=0x" << hex << m_size8 << ":" << endl;
            oss << line;
            oss << m_p_initial_value_file->info_for_exception();
            throw xtsc_exception(oss.str());
          }
          u8 buffer = static_cast<u8>(value);
          poke(m_start_address8 + offset++, 1, &buffer);
        }
      }
    }
  }
}



void xtsc_component::xtsc_memory_base::peek(xtsc_address address8, u32 size8, u8 *buffer) {
  if ((address8 < m_start_address8) || ((address8 + size8 - 1) > m_end_address8)) {
    ostringstream oss;
    oss << "xtsc_memory_base: Memory access out-of-range (address=0x" << hex << address8 << " and byte size=0x" << size8 
        << ") in peek() of " << m_kind << " '" << m_name << "' (Valid range: 0x" << m_start_address8
        << "-0x" << m_end_address8 << ").";
    throw xtsc_exception(oss.str());
  }
  xtsc_address  addr8      = address8;
  u32           first_page = get_page(addr8);
  u32           last_page  = get_page(addr8 + size8 - 1);
  u32           bytes_left = size8;
  u32           buf_offset = 0;
  for (u32 page=first_page; page <= last_page; ++page) {
    u32 mem_offset  = get_page_offset(addr8);
    u32 mem_rem     = m_page_size8 - mem_offset;
    u32 chunk8      = min(mem_rem, bytes_left);
    for (u32 i = 0; i<chunk8; ++i) {
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
    for (u32 i = 0; i<size8; ++i) {
      oss << setw(2) << (u32) buffer[buf_offset] << " ";
      buf_offset += 1;
    }
    XTSC_VERBOSE(m_text, "peek: " << " [0x" << hex << address8 << "/" << dec << size8 << "] = " << oss.str());
  }
}



void xtsc_component::xtsc_memory_base::poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  if ((address8 < m_start_address8) || ((address8 + size8 - 1) > m_end_address8)) {
    ostringstream oss;
    oss << "xtsc_memory_base: Memory access out-of-range (address=0x" << hex << address8 << " and byte size=0x" << size8 
        << ") in poke() of " << m_kind << " '" << m_name << "' (Valid range: 0x" << m_start_address8 << "-0x" << m_end_address8 << ").";
    throw xtsc_exception(oss.str());
  }
  xtsc_address  addr8      = address8;
  u32           first_page = get_page(addr8);
  u32           last_page  = get_page(addr8 + size8 - 1);
  u32           bytes_left = size8;
  u32           buf_offset = 0;
  for (u32 page=first_page; page <= last_page; ++page) {
    u32 mem_offset  = get_page_offset(addr8);
    u32 mem_rem     = m_page_size8 - mem_offset;
    u32 chunk8      = min(mem_rem, bytes_left);
    for (u32 i = 0; i<chunk8; ++i) {
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
    for (u32 i = 0; i<size8; ++i) {
      oss << setw(2) << (u32) buffer[buf_offset] << " ";
      buf_offset += 1;
    }
    XTSC_VERBOSE(m_text, "poke: " << " [0x" << hex << address8 << "/" << dec << size8 << "] = " << oss.str());
  }
}



void xtsc_component::xtsc_memory_base::byte_dump(xtsc_address   address8,
                                                 u32            size8,
                                                 ostream&       os,
                                                 bool           left_to_right,
                                                 u32            bytes_per_line,
                                                 bool           show_address,
                                                 bool           show_hex_values,
                                                 bool           do_column_heading,
                                                 bool           show_ascii_values,
                                                 bool           adjust_address)
{
  u8 *buffer = new u8[size8];
  peek(address8, size8, buffer);
  u32  initial_skipped_bytes = ((adjust_address && (bytes_per_line != 0)) ? (address8 % bytes_per_line) : 0);
  address8 -= initial_skipped_bytes;
  xtsc_hex_dump(left_to_right, size8, buffer, os, bytes_per_line, show_address, address8, show_hex_values,
                do_column_heading, show_ascii_values, initial_skipped_bytes);
  delete [] buffer;
}



u32 xtsc_component::xtsc_memory_base::get_page(xtsc_address address8) {
  XTSC_DEBUG(m_text, "get_page for address8=0x" << hex << address8);
  // Check range
  if ((address8 < m_start_address8) || (address8 > m_end_address8)) {
    ostringstream oss;
    oss << "xtsc_memory_base: Memory access out-of-range (address=0x" << hex << address8 << ") in get_page() of " << m_kind << " '"
        << m_name << "' (Valid range: 0x" << m_start_address8 << "-0x" << m_end_address8 << ").";
    throw xtsc_exception(oss.str());
  }
  // Allocate memory page if required
  u32 page = get_page_id(address8);
  if (!m_page_table[page]) {
    m_page_table[page] = new u8[m_page_size8];
    memset(m_page_table[page], m_memory_fill_byte, m_page_size8);
    XTSC_DEBUG(m_text, "Created page #" << page);
  }
  return page;
}



u8 xtsc_component::xtsc_memory_base::read_u8(xtsc_address address8) {
  u32 page = get_page(address8);
  u32 mem_offset  = get_page_offset(address8);
  u8 value = *(m_page_table[page]+mem_offset);
  XTSC_DEBUG(m_text, setfill('0') << hex << "read_u8:  [0x" << setw(8) << address8 << "] = 0x" << setw(2) << (u32) value);
  return value;
}



void xtsc_component::xtsc_memory_base::write_u8(xtsc_address address8, u8 value) {
  u32 page = get_page(address8);
  u32 mem_offset  = get_page_offset(address8);
  *(m_page_table[page]+mem_offset) = value;
  XTSC_DEBUG(m_text, setfill('0') << hex << "write_u8: [0x" << setw(8) << address8 << "] = 0x" << setw(2) << (u32) value);
}



u32 xtsc_component::xtsc_memory_base::read_u32(xtsc_address address8, bool big_endian) {
  u32 page = get_page(address8);
  u32 mem_offset  = get_page_offset(address8);
  u32 value = 0;
  for (u32 i = 0; i<4; ++i) {
    u32 shift = 8*(big_endian ? 3-i : i);
    value |= (((u32)*(m_page_table[page]+mem_offset)) << shift);
    mem_offset += 1;
  }
  XTSC_DEBUG(m_text, setfill('0') << hex << "read_u32 (" << (big_endian ? "Big" : "Little") << " Endian) : [0x" <<
      setw(8) << address8 << "] = 0x" << setw(8) << value);
  return value;
}



void xtsc_component::xtsc_memory_base::write_u32(xtsc_address address8, u32 value, bool big_endian) {
  u32 page = get_page(address8);
  u32 mem_offset  = get_page_offset(address8);
  if (big_endian) {
    for (u32 i = 0; i<4; ++i) {
      *(m_page_table[page]+mem_offset) = ((value & 0xFF000000) >> 24);
      value <<= 8;
      mem_offset += 1;
    }
  }
  else {
    for (u32 i = 0; i<4; ++i) {
      *(m_page_table[page]+mem_offset) = (value & 0x000000FF);
      value >>= 8;
      mem_offset += 1;
    }
  }
  XTSC_DEBUG(m_text, setfill('0') << hex << "write_u32 (" << (big_endian ? "Big" : "Little") << " Endian) : [0x" <<
                     setw(8) << address8 << "] = 0x" << setw(8) << value);
}




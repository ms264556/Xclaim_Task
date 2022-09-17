// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cctype>
#include <algorithm>
#include "xtsc_ahb_master_sd.h"
#include <xtsc_sd/xtsc_sd.h>

using namespace std;
using namespace xtsc;
using namespace xtsc_sd;





xtsc_ahb_master_sd::xtsc_ahb_master_sd(sc_mx_m_base* c, const string &s) : 
  sc_mx_module          (c, s.c_str()),
  m_ahb_master_port     ("m_ahb_master_port"),
  m_text                (log4xtensa::TextLogger::getInstance(getInstanceName()))
{

  m_init_complete               = false;
  m_reset_called                = false;
  m_p_request_stream            = NULL;
  m_p_signals                   = NULL;

  m_script_file                 = "";
  m_byte_width                  = 4;
  m_wraparound                  = false;


  defineParameter("script_file",                "",             MX_PARAM_STRING, 0);
  defineParameter("byte_width",                 "4",            MX_PARAM_VALUE,  0);
  defineParameter("wraparound",                 "false",        MX_PARAM_BOOL,   0);

  registerPort(&m_ahb_master_port, "m_ahb_master_port");

  // Do this so we have a clock slave port
  sc_mx_clocked();
  registerPort(dynamic_cast<sc_mx_clock_slave_p_base*>(this), "clk-in");

}



xtsc_ahb_master_sd::~xtsc_ahb_master_sd() {
}



void xtsc_ahb_master_sd::init() {
  xtsc_sd_initialize();

  m_p_request_stream = new xtsc_script_file(m_script_file.c_str(), "\"script_file\"",  getInstanceName().c_str(),
                                            getName().c_str(), m_wraparound);

  MxTransactionProperties props;
  AHB_INIT_TRANSACTION_PROPERTIES(props);
  props.supportsBurst = true;
  props.dataBitwidth = m_byte_width*8;
  m_ahb_master_port.setProperties(&props);

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_ahb_master_sd '" << getInstanceName() << "':");
  XTSC_LOG(m_text, ll,        " script_file             = "   << m_script_file);
  XTSC_LOG(m_text, ll,        " byte_width              = "   << m_byte_width);
  XTSC_LOG(m_text, ll,        " wraparound              = "   << boolalpha << m_wraparound);

  m_init_complete = true;
}



void xtsc_ahb_master_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  XTSC_INFO(m_text, "xtsc_ahb_master_sd::reset() with m_p_signals=" << m_p_signals);

  m_line                        = "";
  m_line_count                  = 0;
  m_cycle                       = 0ULL;
  m_waiting_cycle               = 0ULL;
  m_state                       = NEXT;
  m_timeout                     = 0;
  m_hlock                       = false;
  m_size8                       = 0;
  for (u32 i=0; i<AHB_IDX_END; ++i) {
    m_ctrl_ap       [i] = 0;
    m_ctrl_dp       [i] = 0;
    m_ctrl_peek_poke[i] = 0;
  }
  m_ctrl_ap     [AHB_IDX_CYCLE] = AHB_CYCLE_ADDR;
  m_ctrl_dp     [AHB_IDX_CYCLE] = AHB_CYCLE_DATA;
  m_arb_to_addr                 = false;
  m_do_data_phase               = false;
  m_did_addr_phase              = false;
  m_own_addr_bus_next           = false;
  m_own_addr_bus                = false;
  m_address_dp                  = 0xFFFFFFFF;
  m_size8_dp                    = 0;

  m_words.clear();

  m_p_request_stream->reset();

  if (!m_reset_called) {
    m_reset_called = true;
    // Get the AHB signals
    m_ahb_master_port.readReq(0ULL, NULL, (MxU32*) &m_p_signals, NULL);
    XTSC_INFO(m_text, "In xtsc_ahb_master_sd::reset() with m_p_signals=" << m_p_signals);
    if (m_p_signals == NULL) {
      ostringstream oss;
      oss << "xtsc_ahb_master_sd '" << getInstanceName()
          << "': m_p_signals is NULL.  Perhaps this device is not connected to an AHB bus.";
      throw xtsc_exception(oss.str());
    }
  }
  sc_mx_module::reset(level, filelist);
}



void xtsc_ahb_master_sd::terminate() {
  XTSC_INFO(m_text, "In xtsc_ahb_master_sd::terminate()");

  sc_mx_module::terminate();
  xtsc_finalize();
}



void xtsc_ahb_master_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_ahb_master_sd::setParameter: Cannot change parameter <%s>"
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }


  if (name == "script_file") {
    m_script_file = value;
  }
  else if (name == "byte_width") {
    status = MxConvertStringToValue(value, &m_byte_width);
  }
  else if (name == "wraparound") {
    status = MxConvertStringToValue(value, &m_wraparound);
  }


  if (status == MxConvert_SUCCESS) {
    sc_mx_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_ahb_master_sd::setParameter: Illegal value <%s> "
                            "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_ahb_master_sd::getProperty(MxPropertyType property) {
  string description; 
  switch (property) {    
    case MX_PROP_LOADFILE_EXTENSION:
           return "";
    case MX_PROP_REPORT_FILE_EXT:
           return "yes";
    case MX_PROP_COMPONENT_TYPE:
           return "Other"; 
    case MX_PROP_COMPONENT_VERSION:
           return "0.1";
    case MX_PROP_MSG_PREPEND_NAME:
           return "yes"; 
    case MX_PROP_DESCRIPTION:
           description = "xtsc_ahb_master_sd";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



void xtsc_ahb_master_sd::interconnect() {
  getClockMaster()->registerClockSlave(this);
}



void xtsc_ahb_master_sd::communicate() {
  m_did_addr_phase = false;
  m_own_addr_bus = m_own_addr_bus_next && m_p_signals->hready_z1;
  MxGrant grant = m_ahb_master_port.checkForGrant(0);
  m_own_addr_bus_next = (grant == MX_GRANT_OK);
  
  MxStatus status;
  XTSC_DEBUG(m_text, "communicate() m_state=" << get_state_string() << " m_do_data_phase=" << boolalpha << m_do_data_phase <<
                     " m_cycle=" << m_cycle << " m_waiting_cycle=" << m_waiting_cycle);
  if (m_do_data_phase) {
    if (m_write_dp) {
      XTSC_VERBOSE(m_text, "DATA phase WRITE m_p_signals->hready_z1=" << m_p_signals->hready_z1 << " 0x" << hex << setw(8) <<
                           setfill('0') << m_address_dp << "/" << dec << m_size8_dp << 
                           (AHB_ACC_DECODE_HLOCK(m_ctrl_dp[AHB_IDX_ACC]) ? " LOCK " : " UNL  ") <<
                           xtsc_sd_convert_value_to_string(m_size8_dp, m_write_value_dp));
      status = m_ahb_master_port.write(m_address_dp, m_write_value_dp, m_ctrl_dp);
      if (status == MX_STATUS_OK) {
        ostringstream oss;
        xtsc_hex_dump(true, m_size8_dp, (u8*)m_write_value_dp, oss);
        XTSC_INFO(m_text, "Write: 0x" << hex << setw(8) << setfill('0') << m_address_dp <<
                          "/" << dec << m_size8_dp << ": " << oss.str());
      }
    }
    else {
      status = m_ahb_master_port.read(m_address_dp, m_read_value, m_ctrl_dp);
      XTSC_VERBOSE(m_text, "DATA phase READ  m_p_signals->hready_z1=" << m_p_signals->hready_z1 << " 0x" << hex << setw(8) <<
                           setfill('0') << m_address_dp << "/" << dec << m_size8_dp << 
                           (AHB_ACC_DECODE_HLOCK(m_ctrl_dp[AHB_IDX_ACC]) ? " LOCK " : " UNL  ") <<
                           xtsc_sd_convert_value_to_string(m_size8_dp, m_read_value));
      if (status == MX_STATUS_OK) {
        ostringstream oss;
        xtsc_hex_dump(true, m_size8_dp, (u8*)m_read_value, oss);
        XTSC_INFO(m_text, "Read:  0x" << hex << setw(8) << setfill('0') << m_address_dp <<
                          "/" << dec << m_size8_dp << ": " << oss.str());
      }
    }
    if (status != MX_STATUS_WAIT) {
      XTSC_DEBUG(m_text, "setting m_do_data_phase to false");
      m_do_data_phase = false;
    }
  }
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
            if (m_words[1] == "ready") {
              XTSC_DEBUG(m_text, "changing m_state from NEXT to READY");
              m_state = READY;
              loop_again = true;
              break;
            }
            u32 duration = get_u32(1, "duration");
            m_waiting_cycle = m_cycle + duration;
            XTSC_DEBUG(m_text, "changing m_state from NEXT to WAIT");
            m_state = WAIT;
            loop_again = true;
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
            XTSC_DEBUG(m_text, "changing m_state from NEXT to WAIT");
            m_state = WAIT;
            loop_again = true;
            XTSC_DEBUG(m_text, "sync to cycle " << cycle);
          }
          else if (m_words[0] == "info") {
            XTSC_INFO(m_text, m_line);
          }
          else if (m_words[0] == "note") {
            XTSC_NOTE(m_text, m_line);
          }
          else if ((m_words[1] == "peek"    ) ||
                   (m_words[1] == "poke"    ) ||
                   (m_words[1] == "read"    ) ||
                   (m_words[1] == "write"   ) ||
                   (m_words[1] == "assert"  ) ||
                   (m_words[1] == "deassert") ||
                   (m_words[1] == "stop"    )) {
            u32 delay = get_u32(0, "delay");
            m_waiting_cycle = m_cycle + delay;
            XTSC_DEBUG(m_text, "changing m_state from NEXT to DELAY");
            m_state = DELAY;
            loop_again = true;
            XTSC_DEBUG(m_text, "delaying " << delay);
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
          XTSC_DEBUG(m_text, "changing m_state from NEXT to DONE");
          m_state = DONE;
        }
        break;
      }
      case WAIT: {
        if (m_cycle >= m_waiting_cycle) {
          XTSC_DEBUG(m_text, "changing m_state from WAIT to NEXT");
          m_state = NEXT;
        }
        break;
      }
      case READY: {
        if (m_p_signals->hready_z1) {
          XTSC_DEBUG(m_text, "changing m_state from READY to NEXT");
          m_state = NEXT;
        }
        break;
      }
      case DELAY: {
        if (m_cycle >= m_waiting_cycle) {
          XTSC_DEBUG(m_text, "changing m_state from DELAY to NEXT");
          m_state = NEXT;
          if (m_words[1] == "peek") {
            // delay PEEK        address size 
            // virtual MxStatus readDbg (MxU64 addr, MxU32* value, MxU32* ctrl) = 0;
            u32 size8 = 0;
            u32 address8 = get_u32(2, "address");
            m_ctrl_peek_poke[AHB_IDX_TYPE] = convert_word_to_size(3, size8, NULL);
            if ((status = m_ahb_master_port.readDbg(address8, m_dbg_value, m_ctrl_peek_poke)) != MX_STATUS_OK) {
              ostringstream oss;
              oss << "readDbg() call failed for peek command in file '" << m_script_file << "' on line #" << m_line_count
                  << ": " << endl;
              oss << m_line;
              throw xtsc_exception(oss.str());
            }
            ostringstream oss;
            xtsc_hex_dump(true, size8, (u8*)m_dbg_value, oss);
            XTSC_INFO(m_text, "Peek:  0x" << hex << setw(8) << address8 << "/" << dec << size8 << ": " << oss.str());
          }
          else if (m_words[1] == "poke") {
            // delay POKE        address size b0 b1 . . . bN
            // virtual MxStatus writeDbg (MxU64 addr, MxU32* value, MxU32* ctrl) = 0;
            u32 size8 = 0;
            u32 address8 = get_u32(2, "address");
            m_ctrl_peek_poke[AHB_IDX_TYPE] = convert_word_to_size(3, size8, NULL);
            convert_words_to_value(4, size8, m_dbg_value);
            if ((status = m_ahb_master_port.writeDbg(address8, m_dbg_value, m_ctrl_peek_poke)) != MX_STATUS_OK) {
              ostringstream oss;
              oss << "writeDbg() call failed for poke command in file '" << m_script_file << "' on line #" << m_line_count
                  << ": " << endl;
              oss << m_line;
              throw xtsc_exception(oss.str());
            }
            ostringstream oss;
            xtsc_hex_dump(true, size8, (u8*)m_dbg_value, oss);
            XTSC_INFO(m_text, "Poke:  0x" << hex << setw(8) << address8 << "/" << dec << size8 << ": " << oss.str());
          }
          else if ((m_words[1] == "read") || (m_words[1] == "write")) {
            get_ahb_signals();
            XTSC_DEBUG(m_text, "changing m_state from DELAY to ARB");
            m_state = ARB;
            loop_again = true;
          }
          else if ((m_words[1] == "assert") || (m_words[1] == "deassert")) {
            string lock = (m_hlock ? "lock" : "unlock");
            if (m_words.size() > 2) {
              if (m_words[2] == "lock") {
                m_hlock = 1;
                lock    = "lock";
              }
              else if (m_words[2] == "unlock") {
                m_hlock = 0;
                lock    = "unlock";
              }
              else {
                ostringstream oss;
                oss << "Invalid lock argument: expected lock|unlock in argument #3 in file '" << m_script_file
                    << "' on line #" << m_line_count << ": " << endl;
                oss << m_line;
                throw xtsc_exception(oss.str());
              }
            }
            MxU32 addr = (((m_words[1] == "assert") ? 1 : 0) | (m_hlock ? 2 : 0));
            XTSC_VERBOSE(m_text, "hbusreq=" << m_words[1] << " hlock=" << lock << " calling requestAccess(" << addr <<")");
            m_ahb_master_port.requestAccess(addr);
          }
          else if (m_words[1] == "stop") {
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
      case ARB: {
        XTSC_DEBUG(m_text, "m_state=ARB m_own_addr_bus=" << m_own_addr_bus << " grant=" << xtsc_sd_get_grant_string(grant) <<
                           " m_p_signals->hready_z1=" << m_p_signals->hready_z1 << " m_timeout=" << m_timeout <<
                           " command: " << m_line);
        if (!m_own_addr_bus) {
          if (m_timeout != 0) {
            m_timeout -= 1;
            if (m_timeout == 0) {
              XTSC_DEBUG(m_text, "changing m_state from ARB to NEXT");
              m_state = NEXT;
              break;
            }
          }
          MxU32 addr = (1 | (m_hlock ? 2 : 0));
          XTSC_VERBOSE(m_text, "m_state=ARB calling requestAccess(" << addr <<")");
          m_ahb_master_port.requestAccess(addr);
          break;
        }
        XTSC_DEBUG(m_text, "changing m_state from ARB to ADDR");
        m_state = ADDR;
        m_arb_to_addr = true;
        // fall through
      }
      case ADDR: {
        XTSC_DEBUG(m_text, "m_state=ADDR m_arb_to_addr=" << m_arb_to_addr << " m_p_signals->hready_z1=" << m_p_signals->hready_z1 << 
                           " command: " << m_line);
        // m_arb_to_addr   => this is (the first cycle of) our address phase
        // !m_p_signals->hready_z1 => we're stuck in the address phase because of a wait state in somebody's data phase
        if (m_arb_to_addr || !m_p_signals->hready_z1) {
          m_arb_to_addr = false;
          m_did_addr_phase = true;
          if (m_write_ap) {
            XTSC_VERBOSE(m_text, "ADDR phase WRITE 0x" << hex << setw(8) << setfill('0') << m_address << "/" << dec << m_size8 <<
                                 (AHB_ACC_DECODE_HLOCK(m_ctrl_ap[AHB_IDX_ACC]) ? " LOCK " : " UNL  "));
            m_ahb_master_port.write(m_address, m_unused_value, m_ctrl_ap);
          }
          else {
            XTSC_VERBOSE(m_text, "ADDR phase READ  0x" << hex << setw(8) << setfill('0') << m_address << "/" << dec << m_size8 <<
                                 (AHB_ACC_DECODE_HLOCK(m_ctrl_ap[AHB_IDX_ACC]) ? " LOCK " : " UNL  "));
            m_ahb_master_port.read(m_address, m_unused_value, m_ctrl_ap);
          }
        }
        else {
          XTSC_DEBUG(m_text, "changing m_state from ADDR to NEXT");
          m_state = NEXT;
        }
        break;
      }
      case DONE: {
        break;
      }
      default: {
        throw xtsc_exception("Program Bug:  Invalid m_state in xtsc_ahb_master_sd::communicate()");
      }
    }
  } while ((m_state == NEXT) || loop_again);
  m_cycle += 1;
}



void xtsc_ahb_master_sd::update() {
  XTSC_DEBUG(m_text, "update(): m_p_signals->hready=" << m_p_signals->hready << " m_did_addr_phase=" << boolalpha << m_did_addr_phase <<
                     " old m_do_data_phase=" << m_do_data_phase);
  if (m_did_addr_phase && m_p_signals->hready) {
    if (!m_do_data_phase) {
      m_do_data_phase = true;
      m_address_dp = m_address;
      m_size8_dp   = m_size8;
      m_write_dp   = m_write_ap;
      memcpy(m_write_value_dp, m_write_value, 128);
      m_ctrl_dp[AHB_IDX_TYPE] = m_ctrl_ap[AHB_IDX_TYPE];
      m_ctrl_dp[AHB_IDX_ACC ] = m_ctrl_ap[AHB_IDX_ACC ];
    }
  }
}



int xtsc_ahb_master_sd::get_words() {
  m_line_count = m_p_request_stream->get_words(m_words, m_line, true);
  XTSC_DEBUG(m_text, "line #" << m_line_count << " get_words(): " << m_line);
  return m_words.size();
}




u32 xtsc_ahb_master_sd::get_u32(u32 index, const string& argument_name) {
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



double xtsc_ahb_master_sd::get_double(u32 index, const string& argument_name) {
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



// size can be 0|1|2|4|8|16|32|64|128 bytes not to exceed bus width
AHB_ACCESS_TYPE xtsc_ahb_master_sd::convert_word_to_size(u32 index, u32& size8, u32 *p_hsize) {
  u32 dummy_hsize = 0;
  u32 &hsize = (p_hsize ? *p_hsize : dummy_hsize);
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << "Size argument (#" << index+1 << ") missing in file '" << m_script_file << "' on line #" 
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  AHB_ACCESS_TYPE ret = AHB_TYPE_NONE;
  bool ok = true;
  try { size8 = xtsc_strtou32(m_words[index]); } catch (...) { ok = false; size8=0xFFFFFFFF; }
       if (size8 ==   0) {  hsize = 0; ret = AHB_TYPE_NONE;    }
  else if (size8 ==   1) {  hsize = 0; ret = AHB_TYPE_BYTE;    }
  else if (size8 ==   2) {  hsize = 1; ret = AHB_TYPE_HWORD;   }
  else if (size8 ==   4) {  hsize = 2; ret = AHB_TYPE_WORD;    }
  else if (size8 ==   8) {  hsize = 3; ret = AHB_TYPE_DWORD;   }
  else if (size8 ==  16) {  hsize = 4; ret = AHB_TYPE_128BIT;  }
  else if (size8 ==  32) {  hsize = 5; ret = AHB_TYPE_256BIT;  }
  else if (size8 ==  64) {  hsize = 6; ret = AHB_TYPE_512BIT;  }
  else if (size8 == 128) {  hsize = 7; ret = AHB_TYPE_1024BIT; }
  else { ok = false; }
  if (!ok || (size8 > m_byte_width)) {
    ostringstream oss;
    oss << "Size argument (#" << index+1 << "=\"" << m_words[index] << "\") invalid in file '" << m_script_file << "' on line #" 
        << m_line_count << ": " << endl;
    oss << m_line << endl;
    oss << "  Legal values are 1, 2, and n*4 as long as n*4 is less then m_byte_width=" << m_byte_width;
    throw xtsc_exception(oss.str());
  }
  return ret;
}



// delay POKE  address size b0 b1 . . . bN
// delay WRITE address size timeout lock trans burst prot b0 b1 . . . bN
void xtsc_ahb_master_sd::convert_words_to_value(u32 index, u32 size8, MxU32 *value) {
  if (index+size8-1 >= m_words.size()) {
    ostringstream oss;
    oss << "Some \"b0 b1 . . . bN\" arguments (b" << (m_words.size()-index) << " and greater) missing in file '" << m_script_file
        << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  if (size8 == 0) {
    ostringstream oss;
    oss << "size cannot be 0 in call to xtsc_ahb_master_sd::convert_words_to_value.  File '" << m_script_file << "' on line #" 
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  u8 *byte = (u8 *)value;
  for (u32 i=0; i<32; i++) {
    if (i >= size8) {
      byte[i] = 0;
    }
    else {
      ostringstream oss;
      oss << "b" << i;
      u32 value = get_u32(index+i, oss.str());
      byte[i] = (u8) value;
    }
  }
}



// delay READ  address size timeout lock trans burst prot
// delay WRITE address size timeout lock trans burst prot b0 b1 . . . bN
void xtsc_ahb_master_sd::get_ahb_signals() {
  // hwrite
  m_write_ap = (m_words[1] == "write");
  if ((m_write_ap && (m_words.size() < 10)) || (!m_write_ap && m_words.size() != 9)) {
    ostringstream oss;
    oss << "Invalid number of arguments in READ/WRITE cmd in file '" << m_script_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }

  // haddr
  m_address = get_u32(2, "address");

  // hsize
  u32 hsize = 0;
  m_ctrl_ap[AHB_IDX_TYPE] = convert_word_to_size(3, m_size8, &hsize);
  AHB_ACC_SET_HSIZE(m_ctrl_ap[AHB_IDX_ACC], hsize);

  // timeout
  m_timeout = get_u32(4, "timeout");

  // hlock
  bool lock = false;
  if (m_words[5] == "lock") {
    lock   = true;
    m_hlock = true;
  }
  else if (m_words[5] == "unlock") {
    lock = false;
    m_hlock = false;
  }
  else {
    ostringstream oss;
    oss << "Invalid lock argument (#6=\"" << m_words[5] << "\") in READ/WRITE cmd in file '" << m_script_file << "' on line #"
        << m_line_count << ": " << endl;
    oss << m_line << endl;
    oss << "Expected one of: lock|unlock";
    throw xtsc_exception(oss.str());
  }
  AHB_ACC_SET_HLOCK(m_ctrl_ap[AHB_IDX_ACC], (lock ? 1 : 0));

  // htrans
  htransValues htrans = AHB_TRANS_IDLE;
  if (m_words[6] == "idle") {
    htrans = AHB_TRANS_IDLE;
  }
  else if (m_words[6] == "busy") {
    htrans = AHB_TRANS_BUSY;
  }
  else if (m_words[6] == "nonseq") {
    htrans = AHB_TRANS_NONSEQ;
  }
  else if (m_words[6] == "seq") {
    htrans = AHB_TRANS_SEQ;
  }
  else {
    ostringstream oss;
    oss << "Invalid trans argument (#7=\"" << m_words[6] << "\") in READ/WRITE cmd in file '" << m_script_file << "' on line #"
        << m_line_count << ": " << endl;
    oss << m_line << endl;
    oss << "Expected one of: idle|busy|nonseq|seq";
    throw xtsc_exception(oss.str());
  }
  AHB_ACC_SET_HTRANS(m_ctrl_ap[AHB_IDX_ACC], htrans);

  // hburst
  hburstValues hburst = AHB_BURST_SINGLE;
  if (m_words[7] == "single") {
    hburst = AHB_BURST_SINGLE;
  }
  else if (m_words[7] == "incr") {
    hburst = AHB_BURST_INCR;
  }
  else if (m_words[7] == "wrap4") {
    hburst = AHB_BURST_WRAP4;
  }
  else if (m_words[7] == "incr4") {
    hburst = AHB_BURST_INCR4;
  }
  else if (m_words[7] == "wrap8") {
    hburst = AHB_BURST_WRAP8;
  }
  else if (m_words[7] == "incr8") {
    hburst = AHB_BURST_INCR8;
  }
  else if (m_words[7] == "wrap16") {
    hburst = AHB_BURST_WRAP16;
  }
  else if (m_words[7] == "incr16") {
    hburst = AHB_BURST_INCR16;
  }
  else {
    ostringstream oss;
    oss << "Invalid burst argument (#8=\"" << m_words[7] << "\") in READ/WRITE cmd in file '" << m_script_file << "' on line #"
        << m_line_count << ": " << endl;
    oss << m_line << endl;
    oss << "Expected one of: single|incr|wrap4|incr4|wrap8|incr8|wrap16|incr16";
    throw xtsc_exception(oss.str());
  }
  AHB_ACC_SET_HBURST(m_ctrl_ap[AHB_IDX_ACC], hburst);

  // hprot
  u32 hprot = get_u32(8, "prot");
  if (hprot > 15) {
    ostringstream oss;
    oss << "Invalid prot argument (#9=\"" << m_words[8] << "\") in READ/WRITE cmd in file '" << m_script_file << "' on line #"
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  AHB_ACC_SET_HPROT(m_ctrl_ap[AHB_IDX_ACC], hprot);

  if (m_write_ap) {
    convert_words_to_value(9, m_size8, m_write_value);
  }
}



char *xtsc_ahb_master_sd::get_state_string() {
  switch (m_state) {
    case NEXT:  return "NEXT ";
    case WAIT:  return "WAIT ";
    case READY: return "READY";
    case DELAY: return "DELAY";
    case ARB:   return "ARB  ";
    case ADDR:  return "ADDR ";
    case DONE:  return "DONE ";
    default:    return "UNKN ";
  }
}




class xtsc_ahb_master_sdFactory : public MxFactory {
public:
  xtsc_ahb_master_sdFactory() : MxFactory ("xtsc_ahb_master_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    return new xtsc_ahb_master_sd(c, id);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_ahb_master_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}





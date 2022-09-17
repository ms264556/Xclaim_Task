// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cerrno>
#include <cstdlib>
#include <ostream>
#include <xtsc_sd/xtsc_sd.h>
#include "xtsc_ahb_translator_sd.h"


#ifndef ULLONG_MAX
#define ULLONG_MAX   18446744073709551615ULL
#endif


using namespace std;
using namespace xtsc;
using namespace xtsc_sd;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;
using log4xtensa::DEBUG_LOG_LEVEL;





ostream& operator<<(ostream& os, const address_translation_entry& entry) {
  entry.dump(os);
  return os;
}



void address_translation_entry::dump(ostream& os) const {

  // Save state of stream
  char c = os.fill('0');
  ios::fmtflags old_flags = os.flags();

  os << setfill('0') << "0x" << hex << setw(8) << m_start_address << "-0x" << setw(8) << m_end_address;
  if (m_delta) {
    os << "=>0x" << setw(8) << (m_start_address + m_delta);
  }

  // Restore state of stream
  os.fill(c);
  os.flags(old_flags);

}




#if defined(_MSC_VER) && !defined(strtoull)
#define strtoull _strtoui64
#endif
static u64 strtou64(const string& str) {
  char *endp = 0;
  errno = 0;
  u64 value = strtoull(str.c_str(), &endp, 0);
  if ((str == "") || (*endp != 0) || ((errno == ERANGE) && ((value == 0) || (value == ULLONG_MAX)))) {
    ostringstream oss;
    oss << "Cannot convert string '" << str << "' to an unsigned 64-bit integer.";
    throw xtsc_exception(oss.str());
  }
  return value;
}



xtsc_ahb_translator_sd::xtsc_ahb_translator_sd(sc_mx_m_base* c, const string &s) :
  sc_mx_module          (c, s.c_str()),
  m_p_ahb_slave_port    (new sc_mx_transaction_if_impl("m_p_ahb_slave_port", *this)),
  m_ahb_master_port     ("m_ahb_master_port"),
  m_text                (log4xtensa::TextLogger::getInstance(getInstanceName()))
{

  m_init_complete               = false;
  m_p_translation_stream        = NULL;

  m_translation_file            = "";
  m_byte_width                  = 4;


  defineParameter("translation_file",           "",             MX_PARAM_STRING, 0);
  defineParameter("byte_width",                 "4",            MX_PARAM_VALUE,  0);

  registerPort(m_p_ahb_slave_port, m_p_ahb_slave_port->getName());
  registerPort(&m_ahb_master_port, "m_ahb_master_port");

}



xtsc_ahb_translator_sd::~xtsc_ahb_translator_sd(void) {
}



string xtsc_ahb_translator_sd::getProperty(MxPropertyType property) {
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
           description = "xtsc_ahb_translator_sd";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



void xtsc_ahb_translator_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_ahb_translator_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "translation_file") {
    m_translation_file = value;
  }
  else if (name == "byte_width") {
    status = MxConvertStringToValue(value, &m_byte_width);
  }


  if (status == MxConvert_SUCCESS) {
    sc_mx_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_ahb_translator_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



void xtsc_ahb_translator_sd::init() {
  xtsc_sd_initialize();
  XTSC_INFO(m_text, "in xtsc_ahb_translator_sd::init()");

  m_init_complete = true;

  m_p_translation_stream = new xtsc_script_file(m_translation_file.c_str(), "\"translation_file\"", getInstanceName().c_str(),
                                              getName().c_str());

  while ((m_line_count = m_p_translation_stream->get_words(m_words, m_line)) != 0) {
    u32 num_words = m_words.size();
    if (num_words != 3) {
      ostringstream oss;
      oss << kind() << " '" << name() << "':  Found " << num_words << " words (expected 3) on line #"
          << m_line_count << " of \"translation_file\" '" << m_translation_file << "': " << m_line;
      throw xtsc_exception(oss.str());
    }
    u64 value[3];
    string arg_name[3] = { "StartAddress", "EndAddress", "NewStartAddress" };
    for (u32 i=0; i<3; ++i) {
      value[i] = get_u64(i, arg_name[i]);
    }
    u64 start_address       = value[0];
    u64 end_address         = value[1];
    u64 new_start_address   = value[2];
    u64 delta               = new_start_address - start_address;
    if (end_address < start_address) {
      ostringstream oss;
      oss << kind() << " '" << name() << "':  EndAddress (0x" << hex << end_address
          << ") cannot be less than StartAddress (0x" << start_address << ") on line #" << dec << m_line_count
          << " of \"translation_file\" '" << m_translation_file << "': " << m_line;
      throw xtsc_exception(oss.str());
    }
    address_translation_entry *p_entry = new address_translation_entry(start_address, end_address, delta);
    m_translation_table.push_back(p_entry);
  }
  // Sort and check for overlap
  sort(m_translation_table.begin(), m_translation_table.end(), start_address_less_than);
  vector<address_translation_entry*>::iterator itt1 = m_translation_table.begin();
  if (itt1 != m_translation_table.end()) {
    XTSC_DEBUG(m_text, "translate: " << **itt1);
    vector<address_translation_entry*>::iterator itt2 = itt1;
    for (++itt2; itt2 != m_translation_table.end(); itt1 = itt2++) {
      XTSC_DEBUG(m_text, "translate: " << **itt2);
      if ((*itt1)->m_end_address >= (*itt2)->m_start_address) {
        ostringstream oss;
        oss << kind() << " '" << name() << "':  Two address translations overlap: "
            << **itt1 << " and " << **itt2;
        throw xtsc_exception(oss.str());
      }
    }
  }

  MxTransactionProperties props;
  AHB_INIT_TRANSACTION_PROPERTIES(props);
  props.supportsBurst = true;
  props.dataBitwidth = m_byte_width*8;
  m_ahb_master_port.setProperties(&props);
  m_p_ahb_slave_port->setProperties(&props);

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_ahb_translator_sd '" << getInstanceName() << "':");
  XTSC_LOG(m_text, ll,        " translation_file        = "   << m_translation_file);
  XTSC_LOG(m_text, ll,        " byte_width              = "   << m_byte_width);

}



void xtsc_ahb_translator_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  XTSC_INFO(m_text, "xtsc_ahb_translator_sd::reset()");

  sc_mx_module::reset(level, filelist);
}



void xtsc_ahb_translator_sd::terminate() {
  XTSC_INFO(m_text, "In xtsc_ahb_translator_sd::terminate()");
  sc_mx_module::terminate();
  xtsc_finalize();
}



MxU64 xtsc_ahb_translator_sd::translate(MxU64 addr) {
  MxU64 new_addr = addr;
  vector<address_translation_entry*>::iterator itt = m_translation_table.begin();
  for (; itt != m_translation_table.end(); ++itt) {
    if ((*itt)->m_start_address > addr) break;
    if (((*itt)->m_start_address <= addr) && ((*itt)->m_end_address >= addr)) {
      new_addr += (*itt)->m_delta;
      break;
    }
  }
  XTSC_DEBUG(m_text, "translate(0x" << hex << setfill('0') << setw(8) << addr << ") = 0x" << setw(8) << new_addr);
  return new_addr;
}



int xtsc_ahb_translator_sd::get_words() {
  m_line_count = m_p_translation_stream->get_words(m_words, m_line, true);
  XTSC_DEBUG(m_text, "get_words(): " << m_line);
  return m_words.size();
}




u64 xtsc_ahb_translator_sd::get_u64(u32 index, const string& argument_name) {
  u64 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in file '" << m_translation_file << "' on line #"
        << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  try {
    value = strtou64(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number in file '"
        << m_translation_file << "' on line #" << m_line_count << ": " << endl;
    oss << m_line;
    throw xtsc_exception(oss.str());
  }
  return value;
}



bool xtsc_ahb_translator_sd::start_address_less_than(const address_translation_entry* range1, const address_translation_entry* range2) {
  return (range1->m_start_address < range2->m_start_address);
}



MxGrant xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::requestAccess(MxU64 addr) {
  return m_translator.m_ahb_master_port.requestAccess(m_translator.translate(addr));
}



MxGrant xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::checkForGrant(MxU64 addr) {
  return m_translator.m_ahb_master_port.checkForGrant(m_translator.translate(addr));
}



MxStatus xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::read(MxU64 addr, MxU32* value, MxU32* ctrl) {
  return m_translator.m_ahb_master_port.read(m_translator.translate(addr), value, ctrl);
}



MxStatus xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::write(MxU64 addr, MxU32* value, MxU32* ctrl) {
  return m_translator.m_ahb_master_port.write(m_translator.translate(addr), value, ctrl);
}



MxStatus xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::readDbg(MxU64 addr, MxU32* value, MxU32* ctrl) {
  return m_translator.m_ahb_master_port.readDbg(m_translator.translate(addr), value, ctrl);
}



MxStatus xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::writeDbg(MxU64 addr, MxU32* value, MxU32* ctrl) {
  return m_translator.m_ahb_master_port.writeDbg(m_translator.translate(addr), value, ctrl);
}



MxStatus xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::readReq(MxU64                    addr,
                                                                    MxU32                   *value,
                                                                    MxU32                   *ctrl,
                                                                    MxTransactionCallbackIF *callback)
{
  return m_translator.m_ahb_master_port.readReq(m_translator.translate(addr), value, ctrl, callback);
}



void xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::register_port(sc_core::sc_port_base& port, const char * /*if_typename*/) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_ahb_translator_sd '" << m_translator.getInstanceName()
        << "' *m_p_ahb_slave_port: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_translator.m_text, "Binding '" << port.name() << "' to *xtsc_ahb_translator_sd::m_p_ahb_slave_port");
  m_p_port = &port;
}



int xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::getNumRegions() {
  XTSC_DEBUG(m_translator.m_text, "in xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::getNumRegions()");
  return m_translator.m_translation_table.size();
}



void xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::getAddressRegions(MxU64* start, MxU64* size, std::string* name) {
  XTSC_DEBUG(m_translator.m_text, "in xtsc_ahb_translator_sd::sc_mx_transaction_if_impl::getAddressRegions()");
  for (u32 i=0; i<m_translator.m_translation_table.size(); ++i) {
    start[i] = m_translator.m_translation_table[i]->m_start_address;
    size[i]  = m_translator.m_translation_table[i]->m_end_address - m_translator.m_translation_table[i]->m_start_address + 1;
    ostringstream oss;
    oss << "Region #" << i;
    name[i] = oss.str().c_str();
  }
}



class xtsc_ahb_translator_sdFactory : public MxFactory {
public:
  xtsc_ahb_translator_sdFactory() : MxFactory ("xtsc_ahb_translator_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    return new xtsc_ahb_translator_sd(c, id);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_ahb_translator_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_memory_pin_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



static string name_helper(u32 number) {
  ostringstream oss;
  oss << "xtsc_memory_pin_sd_" << number;
  return oss.str();
}



xtsc_memory_pin_sd::xtsc_memory_pin_sd(sc_mx_m_base* c, const sc_module_name &module_name, u32 bit_width) : 
  sc_mx_import_module   (c, module_name, name_helper(bit_width)),
  xtsc_module_pin_base  (*this, 1, NULL, ""),
  m_bit_width           (bit_width)
{
  m_init_complete               = false;
  m_start_byte_address          = 0;
  m_memory_byte_size            = 0;
  m_page_byte_size              = 0;
  m_initial_value_file          = "";
  m_memory_fill_byte            = 0;
  m_big_endian                  = false;
  m_clock_period                = 0;
  m_posedge_offset              = 0;
  m_sample_phase                = 0;
  m_drive_phase                 = 0;
  m_busy_percentage             = 0;
  m_write_responses             = true;
  m_void_resp_cntl              = 0;
  m_request_fifo_depth          = 0;
  m_read_delay                  = 0;
  m_block_read_delay            = 0;
  m_block_read_repeat           = 0;
  m_burst_read_delay            = 0;
  m_burst_read_repeat           = 0;
  m_rcw_repeat                  = 0;
  m_rcw_response                = 0;
  m_write_delay                 = 0;
  m_block_write_delay           = 0;
  m_block_write_repeat          = 0;
  m_block_write_response        = 0;
  m_burst_write_delay           = 0;
  m_burst_write_repeat          = 0;
  m_burst_write_response        = 0;
  m_vcd_name                    = "";

  defineParameter("start_byte_address",         "0x00000000",   MX_PARAM_VALUE,  0);
  defineParameter("memory_byte_size",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("page_byte_size",             "0x4000",       MX_PARAM_VALUE,  0);
  defineParameter("initial_value_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("memory_fill_byte",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("big_endian",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("clock_period",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("posedge_offset",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("sample_phase",               "0",            MX_PARAM_VALUE,  0);
  defineParameter("drive_phase",                "1",            MX_PARAM_VALUE,  0);
  defineParameter("busy_percentage",            "0",            MX_PARAM_VALUE,  0);
  defineParameter("write_responses",            "true",         MX_PARAM_BOOL,   0);
  defineParameter("void_resp_cntl",             "0xFF",         MX_PARAM_VALUE,  0);
  defineParameter("request_fifo_depth",         "2",            MX_PARAM_VALUE,  0);
  defineParameter("read_delay",                 "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_read_delay",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_read_repeat",          "1",            MX_PARAM_VALUE,  0);
  defineParameter("burst_read_delay",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("burst_read_repeat",          "1",            MX_PARAM_VALUE,  0);
  defineParameter("rcw_repeat",                 "1",            MX_PARAM_VALUE,  0);
  defineParameter("rcw_response",               "0",            MX_PARAM_VALUE,  0);
  defineParameter("write_delay",                "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_write_delay",          "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_write_repeat",         "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_write_response",       "0",            MX_PARAM_VALUE,  0);
  defineParameter("burst_write_delay",          "0",            MX_PARAM_VALUE,  0);
  defineParameter("burst_write_repeat",         "0",            MX_PARAM_VALUE,  0);
  defineParameter("burst_write_response",       "0",            MX_PARAM_VALUE,  0);
  defineParameter("vcd_name",                   "",             MX_PARAM_STRING, 0);

  u32 nb = bit_width;
  u32 nB = nb / 8;

  POReqValid            = &add_bool_input ("POReqValid",                false, 0);
  PIReqRdy              = &add_bool_output("PIReqRdy",                  false, 0);
  POReqAdrs             = &add_uint_input ("POReqAdrs",        32,      false, 0);
  POReqCntl             = &add_uint_input ("POReqCntl",         8,      false, 0);
  POReqData             = &add_wide_input ("POReqData",        nb,      false, 0);
  POReqDataBE           = &add_uint_input ("POReqDataBE",      nB,      false, 0);
  POReqId               = &add_uint_input ("POReqId",           6,      false, 0);
  POReqPriority         = &add_uint_input ("POReqPriority",     2,      false, 0);

  PIRespValid           = &add_bool_output("PIRespValid",               false, 0);
  PORespRdy             = &add_bool_input ("PORespRdy",                 false, 0);
  PIRespCntl            = &add_uint_output("PIRespCntl",        8,      false, 0);
  PIRespData            = &add_wide_output("PIRespData",       nb,      false, 0);
  PIRespId              = &add_uint_output("PIRespId",          6,      false, 0);
  PIRespPriority        = &add_uint_output("PIRespPriority",    2,      false, 0);

  registerSCGenericMasterPort(POReqValid,       "POReqValid");
  registerSCGenericMasterPort(PIReqRdy,         "PIReqRdy");
  registerSCGenericMasterPort(POReqAdrs,        "POReqAdrs");
  registerSCGenericMasterPort(POReqCntl,        "POReqCntl");
  registerSCGenericMasterPort(POReqData,        "POReqData");
  registerSCGenericMasterPort(POReqDataBE,      "POReqDataBE");
  registerSCGenericMasterPort(POReqId,          "POReqId");
  registerSCGenericMasterPort(POReqPriority,    "POReqPriority");

  registerSCGenericMasterPort(PIRespValid,      "PIRespValid");
  registerSCGenericMasterPort(PORespRdy,        "PORespRdy");
  registerSCGenericMasterPort(PIRespCntl,       "PIRespCntl");
  registerSCGenericMasterPort(PIRespData,       "PIRespData");
  registerSCGenericMasterPort(PIRespId,         "PIRespId");
  registerSCGenericMasterPort(PIRespPriority,   "PIRespPriority");
}



xtsc_memory_pin_sd::~xtsc_memory_pin_sd() {
}



void xtsc_memory_pin_sd::init() {
  xtsc_sd_initialize();

  xtsc_memory_pin_parms memory_parms("PIF", m_bit_width/8);
  memory_parms.set("start_byte_address",      m_start_byte_address);
  memory_parms.set("memory_byte_size",        m_memory_byte_size);
  memory_parms.set("page_byte_size",          m_page_byte_size);
  memory_parms.set("initial_value_file",      (char*)((m_initial_value_file=="") ? NULL : m_initial_value_file.c_str()));
  memory_parms.set("memory_fill_byte",        m_memory_fill_byte);
  memory_parms.set("big_endian",              m_big_endian);
  memory_parms.set("clock_period",            m_clock_period);
  memory_parms.set("posedge_offset",          m_posedge_offset);
  memory_parms.set("sample_phase",            m_sample_phase);
  memory_parms.set("drive_phase",             m_drive_phase);
  memory_parms.set("busy_percentage",         m_busy_percentage);
  memory_parms.set("write_responses",         m_write_responses);
  memory_parms.set("void_resp_cntl",          m_void_resp_cntl);
  memory_parms.set("request_fifo_depth",      m_request_fifo_depth);
  memory_parms.set("read_delay",              m_read_delay);
  memory_parms.set("block_read_delay",        m_block_read_delay);
  memory_parms.set("block_read_repeat",       m_block_read_repeat);
  memory_parms.set("burst_read_delay",        m_burst_read_delay);
  memory_parms.set("burst_read_repeat",       m_burst_read_repeat);
  memory_parms.set("rcw_repeat",              m_rcw_repeat);
  memory_parms.set("rcw_response",            m_rcw_response);
  memory_parms.set("write_delay",             m_write_delay);
  memory_parms.set("block_write_delay",       m_block_write_delay);
  memory_parms.set("block_write_repeat",      m_block_write_repeat);
  memory_parms.set("block_write_response",    m_block_write_response);
  memory_parms.set("burst_write_delay",       m_burst_write_delay);
  memory_parms.set("burst_write_repeat",      m_burst_write_repeat);
  memory_parms.set("burst_write_response",    m_burst_write_response);
  if (m_vcd_name != "") {
  memory_parms.set("vcd_handle",              xtsc_sd_get_trace_file(m_vcd_name));
  }
  m_p_memory = new xtsc_memory_pin((getInstanceName()+"_").c_str(), memory_parms);

  m_p_memory->get_bool_input ("POReqValid")     (*POReqValid);
  m_p_memory->get_bool_output("PIReqRdy")       (*PIReqRdy);
  m_p_memory->get_uint_input ("POReqAdrs")      (*POReqAdrs);
  m_p_memory->get_uint_input ("POReqCntl")      (*POReqCntl);
  m_p_memory->get_wide_input ("POReqData")      (*POReqData);
  m_p_memory->get_uint_input ("POReqDataBE")    (*POReqDataBE);
  m_p_memory->get_uint_input ("POReqId")        (*POReqId);
  m_p_memory->get_uint_input ("POReqPriority")  (*POReqPriority);

  m_p_memory->get_bool_output("PIRespValid")    (*PIRespValid);
  m_p_memory->get_bool_input ("PORespRdy")      (*PORespRdy);
  m_p_memory->get_uint_output("PIRespCntl")     (*PIRespCntl);
  m_p_memory->get_wide_output("PIRespData")     (*PIRespData);
  m_p_memory->get_uint_output("PIRespId")       (*PIRespId);
  m_p_memory->get_uint_output("PIRespPriority") (*PIRespPriority);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_memory_pin_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  if (sc_start_of_simulation_invoked()) {
    m_p_memory->reset(level == MX_RESET_HARD);
  }
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_memory_pin_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_memory_pin_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_memory_pin_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "start_byte_address") {
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
    status = MxConvertStringToValue(value, &m_memory_fill_byte);
  }
  else if (name == "big_endian") {
    status = MxConvertStringToValue(value, &m_big_endian);
  }
  else if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }
  else if (name == "posedge_offset") {
    status = MxConvertStringToValue(value, &m_posedge_offset);
  }
  else if (name == "sample_phase") {
    status = MxConvertStringToValue(value, &m_sample_phase);
  }
  else if (name == "drive_phase") {
    status = MxConvertStringToValue(value, &m_drive_phase);
  }
  else if (name == "busy_percentage") {
    status = MxConvertStringToValue(value, &m_busy_percentage);
  }
  else if (name == "write_responses") {
    status = MxConvertStringToValue(value, &m_write_responses);
  }
  else if (name == "void_resp_cntl") {
    status = MxConvertStringToValue(value, &m_void_resp_cntl);
  }
  else if (name == "request_fifo_depth") {
    status = MxConvertStringToValue(value, &m_request_fifo_depth);
  }
  else if (name == "read_delay") {
    status = MxConvertStringToValue(value, &m_read_delay);
  }
  else if (name == "block_read_delay") {
    status = MxConvertStringToValue(value, &m_block_read_delay);
  }
  else if (name == "block_read_repeat") {
    status = MxConvertStringToValue(value, &m_block_read_repeat);
  }
  else if (name == "burst_read_delay") {
    status = MxConvertStringToValue(value, &m_burst_read_delay);
  }
  else if (name == "burst_read_repeat") {
    status = MxConvertStringToValue(value, &m_burst_read_repeat);
  }
  else if (name == "rcw_repeat") {
    status = MxConvertStringToValue(value, &m_rcw_repeat);
  }
  else if (name == "rcw_response") {
    status = MxConvertStringToValue(value, &m_rcw_response);
  }
  else if (name == "write_delay") {
    status = MxConvertStringToValue(value, &m_write_delay);
  }
  else if (name == "block_write_delay") {
    status = MxConvertStringToValue(value, &m_block_write_delay);
  }
  else if (name == "block_write_repeat") {
    status = MxConvertStringToValue(value, &m_block_write_repeat);
  }
  else if (name == "block_write_response") {
    status = MxConvertStringToValue(value, &m_block_write_response);
  }
  else if (name == "burst_write_delay") {
    status = MxConvertStringToValue(value, &m_burst_write_delay);
  }
  else if (name == "burst_write_repeat") {
    status = MxConvertStringToValue(value, &m_burst_write_repeat);
  }
  else if (name == "burst_write_response") {
    status = MxConvertStringToValue(value, &m_burst_write_response);
  }
  else if (name == "vcd_name") {
    m_vcd_name = value;
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_memory_pin_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_memory_pin_sd::getProperty(MxPropertyType property) {
  ostringstream oss;
  switch (property) {
    case MX_PROP_LOADFILE_EXTENSION: {
      return "";
    }
    case MX_PROP_REPORT_FILE_EXT: {
      return "yes";
    }
    case MX_PROP_COMPONENT_TYPE: {
      return "Other"; 
    }
    case MX_PROP_COMPONENT_VERSION: {
      return "0.1";
    }
    case MX_PROP_MSG_PREPEND_NAME: {
      return "yes"; 
    }
    case MX_PROP_DESCRIPTION: {
      ostringstream oss;
      oss << "Maxsim wrapper for " << m_bit_width << "-bit PIF xtsc_memory_pin. "
          << "Compiled on " << __DATE__ << ", " << __TIME__; 
      return oss.str();
    }
    case MX_PROP_MXDI_SUPPORT: {
      return "no";
    }
    case MX_PROP_SAVE_RESTORE: {
      return "no";
    }
    default: {
      return "";
    }
  }
}



string xtsc_memory_pin_sd::getName(void) {
  return name_helper(m_bit_width);
}





class xtsc_memory_pin_sd_32Factory : public MxFactory {
public:
  xtsc_memory_pin_sd_32Factory() : MxFactory (name_helper(32)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_memory_pin_sd(c, id.c_str(), 32);
  }
};



class xtsc_memory_pin_sd_64Factory : public MxFactory {
public:
  xtsc_memory_pin_sd_64Factory() : MxFactory (name_helper(64)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_memory_pin_sd(c, id.c_str(), 64);
  }
};



class xtsc_memory_pin_sd_128Factory : public MxFactory {
public:
  xtsc_memory_pin_sd_128Factory() : MxFactory (name_helper(128)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_memory_pin_sd(c, id.c_str(), 128);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_memory_pin_sd_32Factory();
  new xtsc_memory_pin_sd_64Factory();
  new xtsc_memory_pin_sd_128Factory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}



#undef LOCATION
#define LOCATION __FILE__ << ":" << __LINE__

static map<string, xtsc_memory_pin*> memory_pin_map;



static xtsc_memory_pin& find_memory_pin(const char *dso_cookie, const char *method) {
  if (!dso_cookie || !dso_cookie[0]) {
    ostringstream oss;
    oss << LOCATION << " " << method << "() requires a non-null, non-empty dso_cookie";
    throw xtsc_exception(oss.str());
  }

  string memory_name(dso_cookie);
  xtsc_memory_pin *p_memory = NULL;

  map<string, xtsc_memory_pin*>::const_iterator i = memory_pin_map.find(memory_name);
  if (i != memory_pin_map.end()) {
    p_memory = i->second;
  }
  else {
    sc_simcontext *sc = sc_get_curr_simcontext();
    string kind("xtsc_memory_pin");
    for (sc_object *object = sc->first_object(); object!=0; object=sc->next_object()) {
      if (object->kind() == kind) {
        if (object->name() == memory_name) {
          p_memory = dynamic_cast<xtsc_memory_pin*>(object);
          memory_pin_map[memory_name] = p_memory;
          break;
        }
      }
    }
  }

  if (p_memory == NULL) {
    ostringstream oss;
    oss << LOCATION << " " << method << "(): Can't find xtsc_memory_pin with name of \"" << memory_name << "\" (from dso_cookie)";
    throw xtsc_exception(oss.str());
  }

  return *p_memory;
}



extern "C" void XTSC_SD_EXPORT peek(u32 address8, u32 size8, u8 *buffer, const char *dso_cookie, u32 ) {
  xtsc_memory_pin &memory_pin = find_memory_pin(dso_cookie, "peek");
  memory_pin.peek(address8, size8, buffer);
}



extern "C" void XTSC_SD_EXPORT poke(u32 address8, u32 size8, const u8 *buffer, const char *dso_cookie, u32 ) {
  xtsc_memory_pin &memory_pin = find_memory_pin(dso_cookie, "poke");
  memory_pin.poke(address8, size8, buffer);
}




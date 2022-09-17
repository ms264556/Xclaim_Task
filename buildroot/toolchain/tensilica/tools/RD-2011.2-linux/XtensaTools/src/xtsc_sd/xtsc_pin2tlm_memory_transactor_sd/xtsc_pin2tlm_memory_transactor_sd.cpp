// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_pin2tlm_memory_transactor_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



static string name_helper(u32 number) {
  ostringstream oss;
  oss << "xtsc_pin2tlm_memory_transactor_sd_" << number;
  return oss.str();
}



xtsc_pin2tlm_memory_transactor_sd::xtsc_pin2tlm_memory_transactor_sd(sc_mx_m_base* c, const sc_module_name &module_name, u32 bit_width) : 
  sc_mx_import_module   (c, module_name, name_helper(bit_width)),
  xtsc_module_pin_base  (*this, 1, NULL, ""),
  m_request_port        ("m_request_port"),
  m_respond_export      ("m_respond_export"),
  m_bit_width           (bit_width)
{
  m_init_complete               = false;
  m_start_byte_address          = 0;
  m_big_endian                  = false;
  m_clock_period                = 0;
  m_posedge_offset              = 0;
  m_sample_phase                = 0;
  m_output_delay                = 0;
  m_inbound_pif                 = false;
  m_prereject_responses         = false;
  m_vcd_name                    = "";

  defineParameter("start_byte_address",         "0x00000000",   MX_PARAM_VALUE,  0);
  defineParameter("big_endian",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("clock_period",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("posedge_offset",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("sample_phase",               "0",            MX_PARAM_VALUE,  0);
  defineParameter("output_delay",               "1",            MX_PARAM_VALUE,  0);
  defineParameter("inbound_pif",                "false",        MX_PARAM_BOOL,   0);
  defineParameter("prereject_responses",        "false",        MX_PARAM_BOOL,   0);
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

  registerSCGenericMasterPort(&m_request_port,  "m_request_port");
  registerSCGenericSlavePort(&m_respond_export, "m_respond_export");
}



xtsc_pin2tlm_memory_transactor_sd::~xtsc_pin2tlm_memory_transactor_sd() {
}



void xtsc_pin2tlm_memory_transactor_sd::init() {
  xtsc_sd_initialize();

  xtsc_pin2tlm_memory_transactor_parms pin2tlm_parms("PIF", m_bit_width/8);
  pin2tlm_parms.set("start_byte_address",      m_start_byte_address);
  pin2tlm_parms.set("big_endian",              m_big_endian);
  pin2tlm_parms.set("clock_period",            m_clock_period);
  pin2tlm_parms.set("posedge_offset",          m_posedge_offset);
  pin2tlm_parms.set("sample_phase",            m_sample_phase);
  pin2tlm_parms.set("output_delay",            m_output_delay);
  pin2tlm_parms.set("inbound_pif",             m_inbound_pif);
  pin2tlm_parms.set("prereject_responses",     m_prereject_responses);
  if (m_vcd_name != "") {
  pin2tlm_parms.set("vcd_handle",              xtsc_sd_get_trace_file(m_vcd_name));
  }
  m_p_pin2tlm = new xtsc_pin2tlm_memory_transactor((getInstanceName()+"_").c_str(), pin2tlm_parms);

  m_p_pin2tlm->get_bool_input ("POReqValid")    (*POReqValid);
  m_p_pin2tlm->get_bool_output("PIReqRdy")      (*PIReqRdy);
  m_p_pin2tlm->get_uint_input ("POReqAdrs")     (*POReqAdrs);
  m_p_pin2tlm->get_uint_input ("POReqCntl")     (*POReqCntl);
  m_p_pin2tlm->get_wide_input ("POReqData")     (*POReqData);
  m_p_pin2tlm->get_uint_input ("POReqDataBE")   (*POReqDataBE);
  m_p_pin2tlm->get_uint_input ("POReqId")       (*POReqId);
  m_p_pin2tlm->get_uint_input ("POReqPriority") (*POReqPriority);

  m_p_pin2tlm->get_bool_output("PIRespValid")   (*PIRespValid);
  m_p_pin2tlm->get_bool_input ("PORespRdy")     (*PORespRdy);
  m_p_pin2tlm->get_uint_output("PIRespCntl")    (*PIRespCntl);
  m_p_pin2tlm->get_wide_output("PIRespData")    (*PIRespData);
  m_p_pin2tlm->get_uint_output("PIRespId")      (*PIRespId);
  m_p_pin2tlm->get_uint_output("PIRespPriority")(*PIRespPriority);

  (*m_p_pin2tlm->m_request_ports[0])(m_request_port);
  m_respond_export(*m_p_pin2tlm->m_respond_exports[0]);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_pin2tlm_memory_transactor_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  if (sc_start_of_simulation_invoked()) {
    m_p_pin2tlm->reset(level == MX_RESET_HARD);
  }
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_pin2tlm_memory_transactor_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_pin2tlm_memory_transactor_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_pin2tlm_memory_transactor_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "start_byte_address") {
    status = MxConvertStringToValue(value, &m_start_byte_address);
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
  else if (name == "output_delay") {
    status = MxConvertStringToValue(value, &m_output_delay);
  }
  else if (name == "inbound_pif") {
    status = MxConvertStringToValue(value, &m_inbound_pif);
  }
  else if (name == "prereject_responses") {
    status = MxConvertStringToValue(value, &m_prereject_responses);
  }
  else if (name == "vcd_name") {
    m_vcd_name = value;
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_pin2tlm_memory_transactor_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_pin2tlm_memory_transactor_sd::getProperty(MxPropertyType property) {
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
      oss << "Maxsim wrapper for " << m_bit_width << "-bit PIF xtsc_pin2tlm_memory_transactor. "
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



string xtsc_pin2tlm_memory_transactor_sd::getName(void) {
  return name_helper(m_bit_width);
}





class xtsc_pin2tlm_memory_transactor_sd_32Factory : public MxFactory {
public:
  xtsc_pin2tlm_memory_transactor_sd_32Factory() : MxFactory (name_helper(32)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_pin2tlm_memory_transactor_sd(c, id.c_str(), 32);
  }
};



class xtsc_pin2tlm_memory_transactor_sd_64Factory : public MxFactory {
public:
  xtsc_pin2tlm_memory_transactor_sd_64Factory() : MxFactory (name_helper(64)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_pin2tlm_memory_transactor_sd(c, id.c_str(), 64);
  }
};



class xtsc_pin2tlm_memory_transactor_sd_128Factory : public MxFactory {
public:
  xtsc_pin2tlm_memory_transactor_sd_128Factory() : MxFactory (name_helper(128)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_pin2tlm_memory_transactor_sd(c, id.c_str(), 128);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_pin2tlm_memory_transactor_sd_32Factory();
  new xtsc_pin2tlm_memory_transactor_sd_64Factory();
  new xtsc_pin2tlm_memory_transactor_sd_128Factory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}



#undef LOCATION
#define LOCATION __FILE__ << ":" << __LINE__

static map<string, xtsc_pin2tlm_memory_transactor*> pin2tlm_map;



static xtsc_pin2tlm_memory_transactor& find_pin2tlm(const char *dso_cookie, const char *method) {
  if (!dso_cookie || !dso_cookie[0]) {
    ostringstream oss;
    oss << LOCATION << " " << method << "() requires a non-null, non-empty dso_cookie";
    throw xtsc_exception(oss.str());
  }

  string pin2tlm_name(dso_cookie);
  xtsc_pin2tlm_memory_transactor *p_memory = NULL;

  map<string, xtsc_pin2tlm_memory_transactor*>::const_iterator i = pin2tlm_map.find(pin2tlm_name);
  if (i != pin2tlm_map.end()) {
    p_memory = i->second;
  }
  else {
    sc_simcontext *sc = sc_get_curr_simcontext();
    string kind("xtsc_pin2tlm_memory_transactor");
    for (sc_object *object = sc->first_object(); object!=0; object=sc->next_object()) {
      if (object->kind() == kind) {
        if (object->name() == pin2tlm_name) {
          p_memory = dynamic_cast<xtsc_pin2tlm_memory_transactor*>(object);
          pin2tlm_map[pin2tlm_name] = p_memory;
          break;
        }
      }
    }
  }

  if (p_memory == NULL) {
    ostringstream oss;
    oss << LOCATION << " " << method << "(): Can't find xtsc_pin2tlm_memory_transactor with name of \"" << pin2tlm_name
        << "\" (from dso_cookie)";
    throw xtsc_exception(oss.str());
  }

  return *p_memory;
}



extern "C" void XTSC_SD_EXPORT peek(u32 address8, u32 size8, u8 *buffer, const char *dso_cookie, u32 port) {
  xtsc_pin2tlm_memory_transactor &pin2tlm = find_pin2tlm(dso_cookie, "peek");
  (*pin2tlm.m_request_ports[port])->nb_peek(address8, size8, buffer);
}



extern "C" void XTSC_SD_EXPORT poke(u32 address8, u32 size8, const u8 *buffer, const char *dso_cookie, u32 port) {
  xtsc_pin2tlm_memory_transactor &pin2tlm = find_pin2tlm(dso_cookie, "poke");
  (*pin2tlm.m_request_ports[port])->nb_poke(address8, size8, buffer);
}




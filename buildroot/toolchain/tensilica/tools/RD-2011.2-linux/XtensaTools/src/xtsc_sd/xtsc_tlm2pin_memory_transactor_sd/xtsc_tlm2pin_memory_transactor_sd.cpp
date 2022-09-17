// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_tlm2pin_memory_transactor_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



static string name_helper(u32 number) {
  ostringstream oss;
  oss << "xtsc_tlm2pin_memory_transactor_sd_" << number;
  return oss.str();
}



xtsc_tlm2pin_memory_transactor_sd::xtsc_tlm2pin_memory_transactor_sd(sc_mx_m_base* c, const sc_module_name &module_name, u32 bit_width) : 
  sc_mx_import_module   (c, module_name, name_helper(bit_width)),
  xtsc_module_pin_base  (*this, 1, NULL, ""),
  m_request_export      ("m_request_export"),
  m_respond_port        ("m_respond_port"),
  m_bit_width           (bit_width)
{
  m_init_complete               = false;
  m_start_byte_address          = 0;
  m_big_endian                  = false;
  m_dso_name                    = "";
  m_dso_cookie                  = "";
  m_cosim                       = false;
  m_shadow_memory               = false;
  m_initial_value_file          = "";
  m_memory_fill_byte            = 0;
  m_clock_period                = 0;
  m_posedge_offset              = 0;
  m_sample_phase                = 0;
  m_output_delay                = 0;
  m_request_fifo_depth          = 0;
  m_write_responses             = false;
  m_vcd_name                    = "";

  defineParameter("start_byte_address",         "0x00000000",   MX_PARAM_VALUE,  0);
  defineParameter("big_endian",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("dso_name",                   "",             MX_PARAM_STRING, 0);
  defineParameter("dso_cookie",                 "",             MX_PARAM_STRING, 0);
  defineParameter("cosim",                      "true",         MX_PARAM_BOOL,   0);
  defineParameter("shadow_memory",              "true",         MX_PARAM_BOOL,   0);
  defineParameter("initial_value_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("memory_fill_byte",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("clock_period",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("posedge_offset",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("sample_phase",               "0",            MX_PARAM_VALUE,  0);
  defineParameter("output_delay",               "1",            MX_PARAM_VALUE,  0);
  defineParameter("request_fifo_depth",         "1",            MX_PARAM_VALUE,  0);
  defineParameter("write_responses",            "false",        MX_PARAM_BOOL,   0);
  defineParameter("vcd_name",                   "",             MX_PARAM_STRING, 0);

  registerSCGenericSlavePort(&m_request_export, "m_request_export");
  registerSCGenericMasterPort(&m_respond_port, "m_respond_port");

  u32 nb = bit_width;
  u32 nB = nb / 8;

  POReqValid            = &add_bool_output("POReqValid",                false, 0);
  PIReqRdy              = &add_bool_input ("PIReqRdy",                  false, 0);
  POReqAdrs             = &add_uint_output("POReqAdrs",        32,      false, 0);
  POReqCntl             = &add_uint_output("POReqCntl",         8,      false, 0);
  POReqData             = &add_wide_output("POReqData",        nb,      false, 0);
  POReqDataBE           = &add_uint_output("POReqDataBE",      nB,      false, 0);
  POReqId               = &add_uint_output("POReqId",           6,      false, 0);
  POReqPriority         = &add_uint_output("POReqPriority",     2,      false, 0);

  PIRespValid           = &add_bool_input ("PIRespValid",               false, 0);
  PORespRdy             = &add_bool_output("PORespRdy",                 false, 0);
  PIRespCntl            = &add_uint_input ("PIRespCntl",        8,      false, 0);
  PIRespData            = &add_wide_input ("PIRespData",       nb,      false, 0);
  PIRespId              = &add_uint_input ("PIRespId",          6,      false, 0);
  PIRespPriority        = &add_uint_input ("PIRespPriority",    2,      false, 0);

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



xtsc_tlm2pin_memory_transactor_sd::~xtsc_tlm2pin_memory_transactor_sd() {
}



void xtsc_tlm2pin_memory_transactor_sd::init() {
  xtsc_sd_initialize();

  xtsc_tlm2pin_memory_transactor_parms tlm2pin_parms("PIF", m_bit_width/8);
  tlm2pin_parms.set("start_byte_address",      m_start_byte_address);
  tlm2pin_parms.set("big_endian",              m_big_endian);
  tlm2pin_parms.set("dso_name",                (char*)((m_dso_name=="") ? NULL : m_dso_name.c_str()));
  tlm2pin_parms.set("dso_cookie",              (char*)((m_dso_cookie=="") ? NULL : m_dso_cookie.c_str()));
  tlm2pin_parms.set("cosim",                   m_cosim);
  tlm2pin_parms.set("shadow_memory",           m_shadow_memory);
  tlm2pin_parms.set("initial_value_file",      (char*)((m_initial_value_file=="") ? NULL : m_initial_value_file.c_str()));
  tlm2pin_parms.set("memory_fill_byte",        m_memory_fill_byte);
  tlm2pin_parms.set("clock_period",            m_clock_period);
  tlm2pin_parms.set("posedge_offset",          m_posedge_offset);
  tlm2pin_parms.set("sample_phase",            m_sample_phase);
  tlm2pin_parms.set("output_delay",            m_output_delay);
  tlm2pin_parms.set("request_fifo_depth",      m_request_fifo_depth);
  tlm2pin_parms.set("write_responses",         m_write_responses);
  if (m_vcd_name != "") {
  tlm2pin_parms.set("vcd_handle",              xtsc_sd_get_trace_file(m_vcd_name));
  }
  m_p_tlm2pin = new xtsc_tlm2pin_memory_transactor((getInstanceName()+"_").c_str(), tlm2pin_parms);

  m_request_export(*m_p_tlm2pin->m_request_exports[0]);
  (*m_p_tlm2pin->m_respond_ports[0])(m_respond_port);

  m_p_tlm2pin->get_bool_output("POReqValid")    (*POReqValid);
  m_p_tlm2pin->get_bool_input ("PIReqRdy")      (*PIReqRdy);
  m_p_tlm2pin->get_uint_output("POReqAdrs")     (*POReqAdrs);
  m_p_tlm2pin->get_uint_output("POReqCntl")     (*POReqCntl);
  m_p_tlm2pin->get_wide_output("POReqData")     (*POReqData);
  m_p_tlm2pin->get_uint_output("POReqDataBE")   (*POReqDataBE);
  m_p_tlm2pin->get_uint_output("POReqId")       (*POReqId);
  m_p_tlm2pin->get_uint_output("POReqPriority") (*POReqPriority);

  m_p_tlm2pin->get_bool_input ("PIRespValid")   (*PIRespValid);
  m_p_tlm2pin->get_bool_output("PORespRdy")     (*PORespRdy);
  m_p_tlm2pin->get_uint_input ("PIRespCntl")    (*PIRespCntl);
  m_p_tlm2pin->get_wide_input ("PIRespData")    (*PIRespData);
  m_p_tlm2pin->get_uint_input ("PIRespId")      (*PIRespId);
  m_p_tlm2pin->get_uint_input ("PIRespPriority")(*PIRespPriority);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_tlm2pin_memory_transactor_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  if (sc_start_of_simulation_invoked()) {
    m_p_tlm2pin->reset(level == MX_RESET_HARD);
  }
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_tlm2pin_memory_transactor_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_tlm2pin_memory_transactor_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_tlm2pin_memory_transactor_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "start_byte_address") {
    status = MxConvertStringToValue(value, &m_start_byte_address);
  }
  else if (name == "big_endian") {
    status = MxConvertStringToValue(value, &m_big_endian);
  }
  else if (name == "dso_name") {
    m_dso_name = value;
  }
  else if (name == "dso_cookie") {
    m_dso_cookie = value;
  }
  else if (name == "cosim") {
    status = MxConvertStringToValue(value, &m_cosim);
  }
  else if (name == "shadow_memory") {
    status = MxConvertStringToValue(value, &m_shadow_memory);
  }
  else if (name == "initial_value_file") {
    m_initial_value_file = value;
  }
  else if (name == "memory_fill_byte") {
    status = MxConvertStringToValue(value, &m_memory_fill_byte);
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
  else if (name == "request_fifo_depth") {
    status = MxConvertStringToValue(value, &m_request_fifo_depth);
  }
  else if (name == "write_responses") {
    status = MxConvertStringToValue(value, &m_write_responses);
  }
  else if (name == "vcd_name") {
    m_vcd_name = value;
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_tlm2pin_memory_transactor_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_tlm2pin_memory_transactor_sd::getProperty(MxPropertyType property) {
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
      oss << "Maxsim wrapper for " << m_bit_width << "-bit PIF xtsc_tlm2pin_memory_transactor. "
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



string xtsc_tlm2pin_memory_transactor_sd::getName(void) {
  return name_helper(m_bit_width);
}





class xtsc_tlm2pin_memory_transactor_sd_32Factory : public MxFactory {
public:
  xtsc_tlm2pin_memory_transactor_sd_32Factory() : MxFactory (name_helper(32)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_tlm2pin_memory_transactor_sd(c, id.c_str(), 32);
  }
};



class xtsc_tlm2pin_memory_transactor_sd_64Factory : public MxFactory {
public:
  xtsc_tlm2pin_memory_transactor_sd_64Factory() : MxFactory (name_helper(64)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_tlm2pin_memory_transactor_sd(c, id.c_str(), 64);
  }
};



class xtsc_tlm2pin_memory_transactor_sd_128Factory : public MxFactory {
public:
  xtsc_tlm2pin_memory_transactor_sd_128Factory() : MxFactory (name_helper(128)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_tlm2pin_memory_transactor_sd(c, id.c_str(), 128);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_tlm2pin_memory_transactor_sd_32Factory();
  new xtsc_tlm2pin_memory_transactor_sd_64Factory();
  new xtsc_tlm2pin_memory_transactor_sd_128Factory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




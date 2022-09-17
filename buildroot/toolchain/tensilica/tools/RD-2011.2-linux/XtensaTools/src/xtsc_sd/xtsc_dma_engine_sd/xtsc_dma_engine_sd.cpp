// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

#include "xtsc_dma_engine_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



xtsc_dma_engine_sd::xtsc_dma_engine_sd(sc_mx_m_base* c, const sc_module_name &module_name) : 
  sc_mx_import_module   (c, module_name, "xtsc_dma_engine_sd"),
  m_request_export      ("m_request_export"),
  m_respond_port        ("m_respond_port"),
  m_request_port        ("m_request_port"),
  m_respond_export      ("m_respond_export")
{
  m_init_complete               = false;
  m_byte_width                  = 0;
  m_check_alignment             = false;
  m_clear_notify_value          = false;
  m_clock_period                = 0;
  m_delay_from_receipt          = false;
  m_immediate_timing            = false;
  m_memory_byte_size            = 0;
  m_nacc_wait_time              = 0;
  m_page_byte_size              = 0;
  m_read_delay                  = 0;
  m_read_priority               = 0;
  m_recovery_time               = 0;
  m_reg_base_address            = 0;
  m_request_fifo_depth          = 0;
  m_response_repeat             = 0;
  m_start_byte_address          = 0;
  m_turbo                       = false;
  m_use_fast_access             = false;
  m_use_raw_access              = false;
  m_write_delay                 = 0;
  m_write_priority              = 0;
  m_deny_fast_access.clear();

  defineParameter("byte_width",                 "0x4",          MX_PARAM_VALUE,  0);
  defineParameter("check_alignment",            "false",        MX_PARAM_BOOL,   0);
  defineParameter("clear_notify_value",         "false",        MX_PARAM_BOOL,   0);
  defineParameter("clock_period",               "0xffffffff",   MX_PARAM_VALUE,  0);
  defineParameter("delay_from_receipt",         "true",         MX_PARAM_BOOL,   0);
  defineParameter("deny_fast_access",           "",             MX_PARAM_STRING, 0);
  defineParameter("immediate_timing",           "false",        MX_PARAM_BOOL,   0);
  defineParameter("memory_byte_size",           "0x0",          MX_PARAM_VALUE,  0);
  defineParameter("nacc_wait_time",             "0xffffffff",   MX_PARAM_VALUE,  0);
  defineParameter("page_byte_size",             "0x4000",       MX_PARAM_VALUE,  0);
  defineParameter("read_delay",                 "0x0",          MX_PARAM_VALUE,  0);
  defineParameter("read_priority",              "0x2",          MX_PARAM_VALUE,  0);
  defineParameter("recovery_time",              "0x1",          MX_PARAM_VALUE,  0);
  defineParameter("reg_base_address",           "0x0",          MX_PARAM_VALUE,  0);
  defineParameter("request_fifo_depth",         "0x2",          MX_PARAM_VALUE,  0);
  defineParameter("response_repeat",            "0x1",          MX_PARAM_VALUE,  0);
  defineParameter("start_byte_address",         "0x0",          MX_PARAM_VALUE,  0);
  defineParameter("turbo",                      "false",        MX_PARAM_BOOL,   0);
  defineParameter("use_fast_access",            "false",        MX_PARAM_BOOL,   0);
  defineParameter("use_raw_access",             "false",        MX_PARAM_BOOL,   0);
  defineParameter("write_delay",                "0x0",          MX_PARAM_VALUE,  0);
  defineParameter("write_priority",             "0x2",          MX_PARAM_VALUE,  0);

  registerSCGenericSlavePort (&m_request_export,   "m_request_export");
  registerSCGenericMasterPort(&m_respond_port,     "m_respond_port");
  registerSCGenericMasterPort(&m_request_port,     "m_request_port");
  registerSCGenericSlavePort(&m_respond_export,    "m_respond_export");

}



xtsc_dma_engine_sd::~xtsc_dma_engine_sd() {
}



void xtsc_dma_engine_sd::init() {
  xtsc_sd_initialize();

  xtsc_dma_engine_parms dma_engine_parms(0);
  dma_engine_parms.set("byte_width",              m_byte_width);
  dma_engine_parms.set("check_alignment",         m_check_alignment);
  dma_engine_parms.set("clear_notify_value",      m_clear_notify_value);
  dma_engine_parms.set("clock_period",            m_clock_period);
  dma_engine_parms.set("delay_from_receipt",      m_delay_from_receipt);
  dma_engine_parms.set("immediate_timing",        m_immediate_timing);
  dma_engine_parms.set("memory_byte_size",        m_memory_byte_size);
  dma_engine_parms.set("nacc_wait_time",          m_nacc_wait_time);
  dma_engine_parms.set("page_byte_size",          m_page_byte_size);
  dma_engine_parms.set("read_delay",              m_read_delay);
  dma_engine_parms.set("read_priority",           m_read_priority);
  dma_engine_parms.set("recovery_time",           m_recovery_time);
  dma_engine_parms.set("reg_base_address",        m_reg_base_address);
  dma_engine_parms.set("request_fifo_depth",      m_request_fifo_depth);
  dma_engine_parms.set("response_repeat",         m_response_repeat);
  dma_engine_parms.set("start_byte_address",      m_start_byte_address);
  dma_engine_parms.set("turbo",                   m_turbo);
  dma_engine_parms.set("use_fast_access",         m_use_fast_access);
  dma_engine_parms.set("use_raw_access",          m_use_raw_access);
  dma_engine_parms.set("deny_fast_access",        m_deny_fast_access);
  dma_engine_parms.set("write_delay",             m_write_delay);
  dma_engine_parms.set("write_priority",          m_write_priority);

  m_p_dma_engine = new xtsc_dma_engine((getInstanceName()+"_").c_str(), dma_engine_parms);

  m_request_export(*m_p_dma_engine->m_request_exports[0]);
  (*m_p_dma_engine->m_respond_ports[0])(m_respond_port);

  m_p_dma_engine->m_request_port(m_request_port);
  m_respond_export(m_p_dma_engine->m_respond_export);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_dma_engine_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_dma_engine->reset();
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_dma_engine_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_dma_engine_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_dma_engine_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "byte_width") {
    status = MxConvertStringToValue(value, &m_byte_width);
  }
  else if (name == "check_alignment") {
    status = MxConvertStringToValue(value, &m_check_alignment);
  }
  else if (name == "clear_notify_value") {
    status = MxConvertStringToValue(value, &m_clear_notify_value);
  }
  else if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }
  else if (name == "delay_from_receipt") {
    status = MxConvertStringToValue(value, &m_delay_from_receipt);
  }
  else if (name == "deny_fast_access") {
    m_deny_fast_access.clear();
    bool bad_value = false;
    try { xtsc_strtou32vector(value, m_deny_fast_access); } catch (...) { bad_value = true; }
    // Note: MX_MSG_WARNING allows incomplete translation from value to m_deny_fast_access; however, we don't
    //       want to use MX_MSG_ERROR because that causes sdcanvas to terminate.
    if (bad_value) {
      ostringstream oss;
      oss << "ERROR: Cannot convert value #" << (m_deny_fast_access.size()+1) << " of parameter \"deny_fast_access\" to u32.";
      message(MX_MSG_WARNING, oss.str().c_str());
      status = MxConvert_CANNOT_CONVERT_STRING;
      m_deny_fast_access.clear();
    }
    else if (m_deny_fast_access.size() % 2) {
      message(MX_MSG_WARNING, "ERROR: Parameter \"deny_fast_access\" must contain an even number of values.");
      status = MxConvert_ILLEGAL_VALUE;
    }
  }
  else if (name == "immediate_timing") {
    status = MxConvertStringToValue(value, &m_immediate_timing);
  }
  else if (name == "memory_byte_size") {
    status = MxConvertStringToValue(value, &m_memory_byte_size);
  }
  else if (name == "nacc_wait_time") {
    status = MxConvertStringToValue(value, &m_nacc_wait_time);
  }
  else if (name == "page_byte_size") {
    status = MxConvertStringToValue(value, &m_page_byte_size);
  }
  else if (name == "read_delay") {
    status = MxConvertStringToValue(value, &m_read_delay);
  }
  else if (name == "read_priority") {
    status = MxConvertStringToValue(value, &m_read_priority);
  }
  else if (name == "recovery_time") {
    status = MxConvertStringToValue(value, &m_recovery_time);
  }
  else if (name == "reg_base_address") {
    status = MxConvertStringToValue(value, &m_reg_base_address);
  }
  else if (name == "request_fifo_depth") {
    status = MxConvertStringToValue(value, &m_request_fifo_depth);
  }
  else if (name == "response_repeat") {
    status = MxConvertStringToValue(value, &m_response_repeat);
  }
  else if (name == "start_byte_address") {
    status = MxConvertStringToValue(value, &m_start_byte_address);
  }
  else if (name == "turbo") {
    status = MxConvertStringToValue(value, &m_turbo);
  }
  else if (name == "use_fast_access") {
    status = MxConvertStringToValue(value, &m_use_fast_access);
  }
  else if (name == "use_raw_access") {
    status = MxConvertStringToValue(value, &m_use_raw_access);
  }
  else if (name == "write_delay") {
    status = MxConvertStringToValue(value, &m_write_delay);
  }
  else if (name == "write_priority") {
    status = MxConvertStringToValue(value, &m_write_priority);
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_dma_engine_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_dma_engine_sd::getProperty(MxPropertyType property) {
  ostringstream oss;
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
      oss << "Maxsim wrapper for xtsc_dma_engine.  "
          << "Compiled on " << __DATE__ << ", " << __TIME__; 
      return oss.str();
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_dma_engine_sd::getName(void) {
  return "xtsc_dma_engine_sd";
}





class xtsc_dma_engine_sdFactory : public MxFactory {
public:
  xtsc_dma_engine_sdFactory() : MxFactory ("xtsc_dma_engine_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_dma_engine_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_dma_engine_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




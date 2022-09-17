// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_arbiter_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



static string name_helper(u32 number) {
  ostringstream oss;
  oss << "xtsc_arbiter_sd_" << number;
  return oss.str();
}



xtsc_arbiter_sd::xtsc_arbiter_sd(sc_mx_m_base* c, const sc_module_name &module_name, u32 num_ports) : 
  sc_mx_import_module   (c, module_name, name_helper(num_ports)),
  m_request_port        ("m_request_port"),
  m_respond_export      ("m_respond_export"),
  m_num_ports           (num_ports)
{
  m_init_complete               = false;
  m_route_id_lsb                = 0;
  m_translation_file            = "";
  m_external_cbox               = false;
  m_immediate_timing            = false;
  m_request_fifo_depth          = 0;
  m_response_fifo_depth         = 0;
  m_clock_period                = 0;
  m_arbitration_phase           = 0;
  m_nacc_wait_time              = 0;
  m_one_at_a_time               = false;
  m_delay_from_receipt          = false;
  m_read_only                   = false;
  m_write_only                  = false;
  m_request_delay               = 0;
  m_response_delay              = 0;
  m_response_repeat             = 0;
  m_recovery_time               = 0;
  m_slave_byte_width            = 0;
  m_use_block_requests          = false;
  m_dram_lock                   = false;
  m_xfer_en_port                = 0;
  m_master_byte_widths.clear();
  m_request_fifo_depths.clear();

  defineParameter("route_id_lsb",               "0",            MX_PARAM_VALUE,  0);
  defineParameter("translation_file",           "",             MX_PARAM_STRING, 0);
  defineParameter("external_cbox",              "false",        MX_PARAM_BOOL,   0);
  defineParameter("immediate_timing",           "false",        MX_PARAM_BOOL,   0);
  defineParameter("request_fifo_depth",         "2",            MX_PARAM_VALUE,  0);
  defineParameter("response_fifo_depth",        "2",            MX_PARAM_VALUE,  0);
  defineParameter("clock_period",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("arbitration_phase",          "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("nacc_wait_time",             "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("one_at_a_time",              "true",         MX_PARAM_BOOL,   0);
  defineParameter("delay_from_receipt",         "true",         MX_PARAM_BOOL,   0);
  defineParameter("read_only",                  "false",        MX_PARAM_BOOL,   0);
  defineParameter("write_only",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("request_delay",              "1",            MX_PARAM_VALUE,  0);
  defineParameter("response_delay",             "1",            MX_PARAM_VALUE,  0);
  defineParameter("response_repeat",            "1",            MX_PARAM_VALUE,  0);
  defineParameter("recovery_time",              "1",            MX_PARAM_VALUE,  0);
  defineParameter("master_byte_widths",         "",             MX_PARAM_STRING, 0);
  defineParameter("slave_byte_width",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("use_block_requests",         "false",        MX_PARAM_BOOL,   0);
  defineParameter("dram_lock",                  "false",        MX_PARAM_BOOL,   0);
  defineParameter("xfer_en_port",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("request_fifo_depths",        "",             MX_PARAM_STRING, 0);

  m_respond_ports   = new sc_port  <xtsc_respond_if>*[m_num_ports];
  m_request_exports = new sc_export<xtsc_request_if>*[m_num_ports];
  for (u32 i=0; i<m_num_ports; ++i) {
    ostringstream oss1;
    oss1 << "m_respond_ports[" << i << "]";
    m_respond_ports[i] = new sc_port<xtsc_respond_if>(oss1.str().c_str());
    registerSCGenericMasterPort(m_respond_ports[i], oss1.str());

    ostringstream oss2;
    oss2 << "m_request_exports[" << i << "]";
    m_request_exports[i] = new sc_export<xtsc_request_if>(oss2.str().c_str());
    registerSCGenericSlavePort(m_request_exports[i], oss2.str());
  }

  registerSCGenericMasterPort(&m_request_port,     "m_request_port");
  registerSCGenericSlavePort (&m_respond_export,   "m_respond_export");

}



xtsc_arbiter_sd::~xtsc_arbiter_sd() {
}



void xtsc_arbiter_sd::init() {
  xtsc_sd_initialize();

  xtsc_arbiter_parms arbiter_parms;
  arbiter_parms.set("num_masters",             m_num_ports);
  arbiter_parms.set("route_id_lsb",            m_route_id_lsb);
  arbiter_parms.set("translation_file",        (char*)((m_translation_file=="") ? NULL : m_translation_file.c_str()));
  arbiter_parms.set("external_cbox",           m_external_cbox);
  arbiter_parms.set("immediate_timing",        m_immediate_timing);
  arbiter_parms.set("request_fifo_depth",      m_request_fifo_depth);
  arbiter_parms.set("response_fifo_depth",     m_response_fifo_depth);
  arbiter_parms.set("clock_period",            m_clock_period);
  arbiter_parms.set("arbitration_phase",       m_arbitration_phase);
  arbiter_parms.set("nacc_wait_time",          m_nacc_wait_time);
  arbiter_parms.set("one_at_a_time",           m_one_at_a_time);
  arbiter_parms.set("delay_from_receipt",      m_delay_from_receipt);
  arbiter_parms.set("read_only",               m_read_only);
  arbiter_parms.set("write_only",              m_write_only);
  arbiter_parms.set("request_delay",           m_request_delay);
  arbiter_parms.set("response_delay",          m_response_delay);
  arbiter_parms.set("response_repeat",         m_response_repeat);
  arbiter_parms.set("recovery_time",           m_recovery_time);
  arbiter_parms.set("master_byte_widths",      m_master_byte_widths);
  arbiter_parms.set("slave_byte_width",        m_slave_byte_width);
  arbiter_parms.set("use_block_requests",      m_use_block_requests);
  arbiter_parms.set("dram_lock",               m_dram_lock);
  arbiter_parms.set("xfer_en_port",            m_xfer_en_port);
  arbiter_parms.set("request_fifo_depths",     m_request_fifo_depths);

  m_p_arbiter = new xtsc_arbiter((getInstanceName()+"_").c_str(), arbiter_parms);

  for (u32 i=0; i<m_num_ports; ++i) {
    (*m_p_arbiter->m_respond_ports[i])(*m_respond_ports[i]);
    (*m_request_exports[i])(*m_p_arbiter->m_request_exports[i]);
  }

  m_respond_export(m_p_arbiter->m_respond_export);
  m_p_arbiter->m_request_port(m_request_port);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_arbiter_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_arbiter->reset(level == MX_RESET_HARD);
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_arbiter_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_arbiter_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_arbiter_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "route_id_lsb") {
    status = MxConvertStringToValue(value, &m_route_id_lsb);
  }
  else if (name == "translation_file") {
    m_translation_file = value;
  }
  else if (name == "external_cbox") {
    status = MxConvertStringToValue(value, &m_external_cbox);
  }
  else if (name == "immediate_timing") {
    status = MxConvertStringToValue(value, &m_immediate_timing);
  }
  else if (name == "request_fifo_depth") {
    status = MxConvertStringToValue(value, &m_request_fifo_depth);
  }
  else if (name == "response_fifo_depth") {
    status = MxConvertStringToValue(value, &m_response_fifo_depth);
  }
  else if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }
  else if (name == "arbitration_phase") {
    status = MxConvertStringToValue(value, &m_arbitration_phase);
  }
  else if (name == "nacc_wait_time") {
    status = MxConvertStringToValue(value, &m_nacc_wait_time);
  }
  else if (name == "one_at_a_time") {
    status = MxConvertStringToValue(value, &m_one_at_a_time);
  }
  else if (name == "delay_from_receipt") {
    status = MxConvertStringToValue(value, &m_delay_from_receipt);
  }
  else if (name == "read_only") {
    status = MxConvertStringToValue(value, &m_read_only);
  }
  else if (name == "write_only") {
    status = MxConvertStringToValue(value, &m_write_only);
  }
  else if (name == "request_delay") {
    status = MxConvertStringToValue(value, &m_request_delay);
  }
  else if (name == "response_delay") {
    status = MxConvertStringToValue(value, &m_response_delay);
  }
  else if (name == "response_repeat") {
    status = MxConvertStringToValue(value, &m_response_repeat);
  }
  else if (name == "recovery_time") {
    status = MxConvertStringToValue(value, &m_recovery_time);
  }
  else if (name == "master_byte_widths") {
    m_master_byte_widths.clear();
    bool bad_value = false;
    try { xtsc_strtou32vector(value, m_master_byte_widths); } catch (...) { bad_value = true; }
    // Note: MX_MSG_WARNING allows incomplete translation from value to m_master_byte_widths; however, we don't
    //       want to use MX_MSG_ERROR because that causes sdcanvas to terminate.
    if (bad_value) {
      ostringstream oss;
      oss << "ERROR: Cannot convert value #" << (m_master_byte_widths.size()+1) << " of parameter \"master_byte_widths\" to u32.";
      message(MX_MSG_WARNING, oss.str().c_str());
      status = MxConvert_CANNOT_CONVERT_STRING;
      m_master_byte_widths.clear();
    }
  }
  else if (name == "slave_byte_width") {
    status = MxConvertStringToValue(value, &m_slave_byte_width);
  }
  else if (name == "use_block_requests") {
    status = MxConvertStringToValue(value, &m_use_block_requests);
  }
  else if (name == "dram_lock") {
    status = MxConvertStringToValue(value, &m_dram_lock);
  }
  else if (name == "xfer_en_port") {
    status = MxConvertStringToValue(value, &m_xfer_en_port);
  }
  else if (name == "request_fifo_depths") {
    m_request_fifo_depths.clear();
    bool bad_value = false;
    try { xtsc_strtou32vector(value, m_request_fifo_depths); } catch (...) { bad_value = true; }
    // Note: MX_MSG_WARNING allows incomplete translation from value to m_request_fifo_depths; however, we don't
    //       want to use MX_MSG_ERROR because that causes sdcanvas to terminate.
    if (bad_value) {
      ostringstream oss;
      oss << "ERROR: Cannot convert value #" << (m_request_fifo_depths.size()+1) << " of parameter \"request_fifo_depths\" to u32.";
      message(MX_MSG_WARNING, oss.str().c_str());
      status = MxConvert_CANNOT_CONVERT_STRING;
      m_request_fifo_depths.clear();
    }
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_arbiter_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_arbiter_sd::getProperty(MxPropertyType property) {
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
      oss << "Maxsim wrapper for xtsc_arbiter with " << m_num_ports << " master(s)."
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



string xtsc_arbiter_sd::getName(void) {
  return name_helper(m_num_ports);
}





class xtsc_arbiter_sd_1Factory : public MxFactory {
public:
  xtsc_arbiter_sd_1Factory() : MxFactory (name_helper(1)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_arbiter_sd(c, id.c_str(), 1);
  }
};



class xtsc_arbiter_sd_2Factory : public MxFactory {
public:
  xtsc_arbiter_sd_2Factory() : MxFactory (name_helper(2)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_arbiter_sd(c, id.c_str(), 2);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_arbiter_sd_1Factory();
  new xtsc_arbiter_sd_2Factory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




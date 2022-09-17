// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_master_sd.h"
#include <xtsc_sd/xtsc_sd.h>

using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



xtsc_master_sd::xtsc_master_sd(sc_mx_m_base* c, const sc_module_name &module_name) :
  sc_mx_import_module   (c, module_name, "xtsc_master_sd"),
  m_request_port        ("m_request_port"),
  m_respond_export      ("m_respond_export"),
  m_p_master            (NULL)
{
  m_init_complete       = false;
  m_clock_period        = 32;
  m_return_value_file   = "";
  m_script_file         = "";
  m_wraparound          = false;

  defineParameter("clock_period",       "0xffffffff",   MX_PARAM_VALUE,  0);
  defineParameter("return_value_file",  "",             MX_PARAM_STRING, 0);
  defineParameter("script_file",        "",             MX_PARAM_STRING, 0);
  defineParameter("wraparound",         "false",        MX_PARAM_BOOL,   0);

  registerSCGenericMasterPort(&m_request_port, "m_request_port");
  registerSCGenericSlavePort(&m_respond_export, "m_respond_export");
  registerSCGenericSlavePort(&m_control, "m_control");

}



xtsc_master_sd::~xtsc_master_sd() {
}



void xtsc_master_sd::init() {
  xtsc_sd_initialize();

  xtsc_master_parms master_parms(
      m_script_file.c_str(),
      m_wraparound,
      (m_return_value_file=="" ? NULL : m_return_value_file.c_str())
  );
  master_parms.set("clock_period", m_clock_period);
  master_parms.set("control", true);
  m_p_master = new xtsc_master((getInstanceName()+"_").c_str(), master_parms);
  m_p_master->m_request_port(m_request_port);
  m_respond_export(m_p_master->m_respond_export);
  m_control(m_p_master->get_control_input());

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_master_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_master->reset(level == MX_RESET_HARD);
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_master_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}



void xtsc_master_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_master_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }
  else if (name == "return_value_file") {
    m_return_value_file = value;
  }
  else if (name == "script_file") {
    m_script_file = value;
  }
  else if (name == "wraparound") {
    status = MxConvertStringToValue(value, &m_wraparound);
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_master_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_master_sd::getProperty(MxPropertyType property) {
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
           description = "Maxsim wrapper for xtsc_master";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_master_sd::getName(void) {
  return "xtsc_master_sd";
}




class xtsc_master_sdFactory : public MxFactory {
public:
  xtsc_master_sdFactory() : MxFactory ("xtsc_master_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_master_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_master_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




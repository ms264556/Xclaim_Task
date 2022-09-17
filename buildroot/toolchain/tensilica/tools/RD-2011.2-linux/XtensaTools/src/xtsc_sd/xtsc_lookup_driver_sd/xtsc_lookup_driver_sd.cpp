// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_lookup_driver_sd.h"
#include <xtsc_sd/xtsc_sd.h>

using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



xtsc_lookup_driver_sd::xtsc_lookup_driver_sd(sc_mx_m_base* c, const sc_module_name &module_name) :
  sc_mx_import_module   (c, module_name, "xtsc_lookup_driver_sd"),
  m_lookup              ("m_lookup"),
  m_p_lookup_driver     (NULL)
{
  m_init_complete       = false;
  m_address_bit_width   = 0;
  m_data_bit_width      = 0;
  m_has_ready           = false;
  m_latency             = 0;
  m_script_file         = "";
  m_clock_period        = 0;
  m_poll_ready_delay    = 0;

  defineParameter("address_bit_width",  "32",           MX_PARAM_VALUE,  0);
  defineParameter("data_bit_width",     "32",           MX_PARAM_VALUE,  0);
  defineParameter("has_ready",          "false",        MX_PARAM_BOOL,   0);
  defineParameter("latency",            "1",            MX_PARAM_VALUE,  0);
  defineParameter("script_file",        "",             MX_PARAM_STRING, 0);
  defineParameter("clock_period",       "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("poll_ready_delay",   "0xFFFFFFFF",   MX_PARAM_VALUE,  0);

  registerSCGenericMasterPort(&m_lookup, "m_lookup");

}



xtsc_lookup_driver_sd::~xtsc_lookup_driver_sd() {
}



void xtsc_lookup_driver_sd::init() {
  xtsc_sd_initialize();

  xtsc_lookup_driver_parms lookup_driver_parms(m_address_bit_width, m_data_bit_width, m_has_ready, m_script_file.c_str());
  lookup_driver_parms.set("latency",                 m_latency);
  lookup_driver_parms.set("clock_period",            m_clock_period);
  lookup_driver_parms.set("poll_ready_delay",        m_poll_ready_delay);
  m_p_lookup_driver = new xtsc_lookup_driver((getInstanceName()+"_").c_str(), lookup_driver_parms);
  m_p_lookup_driver->m_lookup(m_lookup);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_lookup_driver_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_lookup_driver->reset(level == MX_RESET_HARD);
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_lookup_driver_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}



void xtsc_lookup_driver_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_lookup_driver_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "address_bit_width") {
    status = MxConvertStringToValue(value, &m_address_bit_width);
  }
  else if (name == "data_bit_width") {
    status = MxConvertStringToValue(value, &m_data_bit_width);
  }
  else if (name == "has_ready") {
    status = MxConvertStringToValue(value, &m_has_ready);
  }
  else if (name == "latency") {
    status = MxConvertStringToValue(value, &m_latency);
  }
  else if (name == "script_file") {
    m_script_file = value;
  }
  else if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }
  else if (name == "poll_ready_delay") {
    status = MxConvertStringToValue(value, &m_poll_ready_delay);
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_lookup_driver_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_lookup_driver_sd::getProperty(MxPropertyType property) {
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
           description = "Maxsim wrapper for xtsc_lookup_driver";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_lookup_driver_sd::getName(void) {
  return "xtsc_lookup_driver_sd";
}




class xtsc_lookup_driver_sdFactory : public MxFactory {
public:
  xtsc_lookup_driver_sdFactory() : MxFactory ("xtsc_lookup_driver_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_lookup_driver_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_lookup_driver_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




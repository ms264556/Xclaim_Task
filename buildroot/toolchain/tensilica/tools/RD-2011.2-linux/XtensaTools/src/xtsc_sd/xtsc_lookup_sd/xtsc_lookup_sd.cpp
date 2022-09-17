// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_lookup_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



xtsc_lookup_sd::xtsc_lookup_sd(sc_mx_m_base* c, const sc_module_name &module_name) : 
  sc_mx_import_module   (c, module_name, "xtsc_lookup_sd"),
  m_lookup              ("m_lookup")
{
  m_init_complete                 = false;
  m_ram                           = false;
  m_address_bit_width             = 0;
  m_data_bit_width                = 0;
  m_write_data_lsb                = 0;
  m_write_strobe_bit              = 0;
  m_active_high_strobe            = false;
  m_has_ready                     = false;
  m_pipeline_depth                = 0;
  m_enforce_latency               = false;
  m_latency                       = 0;
  m_delay                         = 0;
  m_lookup_table                  = "";
  m_default_data                  = "0x0";
  m_clock_period                  = 0;

  defineParameter("ram",                        "false",        MX_PARAM_BOOL,   0);
  defineParameter("address_bit_width",          "32",           MX_PARAM_VALUE,  0);
  defineParameter("data_bit_width",             "32",           MX_PARAM_VALUE,  0);
  defineParameter("write_data_lsb",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("write_strobe_bit",           "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("active_high_strobe",         "true",         MX_PARAM_BOOL,   0);
  defineParameter("has_ready",                  "false",        MX_PARAM_BOOL,   0);
  defineParameter("pipeline_depth",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("enforce_latency",            "true",         MX_PARAM_BOOL,   0);
  defineParameter("latency",                    "1",            MX_PARAM_VALUE,  0);
  defineParameter("delay",                      "0",            MX_PARAM_VALUE,  0);
  defineParameter("lookup_table",               "",             MX_PARAM_STRING, 0);
  defineParameter("default_data",               "0x0",          MX_PARAM_STRING, 0);
  defineParameter("clock_period",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);


  registerSCGenericSlavePort(&m_lookup, "m_lookup");
}



xtsc_lookup_sd::~xtsc_lookup_sd() {
}



void xtsc_lookup_sd::init() {
  xtsc_sd_initialize();

  xtsc_lookup_parms lookup_parms(32, 32, false);
  lookup_parms.set("ram",                     m_ram);
  lookup_parms.set("address_bit_width",       m_address_bit_width);
  lookup_parms.set("data_bit_width",          m_data_bit_width);
  lookup_parms.set("write_data_lsb",          m_write_data_lsb);
  lookup_parms.set("write_strobe_bit",        m_write_strobe_bit);
  lookup_parms.set("active_high_strobe",      m_active_high_strobe);
  lookup_parms.set("has_ready",               m_has_ready);
  lookup_parms.set("pipeline_depth",          m_pipeline_depth);
  lookup_parms.set("enforce_latency",         m_enforce_latency);
  lookup_parms.set("latency",                 m_latency);
  lookup_parms.set("delay",                   m_delay);
  lookup_parms.set("lookup_table",            (char*)((m_lookup_table=="") ? NULL : m_lookup_table.c_str()));
  lookup_parms.set("default_data",            (char*)((m_default_data=="") ? "0x0" : m_default_data.c_str()));
  lookup_parms.set("clock_period",            m_clock_period);
  m_p_lookup = new xtsc_lookup((getInstanceName()+"_").c_str(), lookup_parms);
  m_lookup(m_p_lookup->m_lookup);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_lookup_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_lookup->reset(level == MX_RESET_HARD);
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_lookup_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_lookup_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_lookup_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "ram") {
    status = MxConvertStringToValue(value, &m_ram);
  }
  else if (name == "address_bit_width") {
    status = MxConvertStringToValue(value, &m_address_bit_width);
  }
  else if (name == "data_bit_width") {
    status = MxConvertStringToValue(value, &m_data_bit_width);
  }
  else if (name == "write_data_lsb") {
    status = MxConvertStringToValue(value, &m_write_data_lsb);
  }
  else if (name == "write_strobe_bit") {
    status = MxConvertStringToValue(value, &m_write_strobe_bit);
  }
  else if (name == "active_high_strobe") {
    status = MxConvertStringToValue(value, &m_active_high_strobe);
  }
  else if (name == "has_ready") {
    status = MxConvertStringToValue(value, &m_has_ready);
  }
  else if (name == "pipeline_depth") {
    status = MxConvertStringToValue(value, &m_pipeline_depth);
  }
  else if (name == "enforce_latency") {
    status = MxConvertStringToValue(value, &m_enforce_latency);
  }
  else if (name == "latency") {
    status = MxConvertStringToValue(value, &m_latency);
  }
  else if (name == "delay") {
    status = MxConvertStringToValue(value, &m_delay);
  }
  else if (name == "lookup_table") {
    m_lookup_table = value;
  }
  else if (name == "default_data") {
    m_default_data = value;
  }
  else if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_lookup_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_lookup_sd::getProperty(MxPropertyType property) {
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
           description = "Maxsim wrapper for xtsc_lookup";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_lookup_sd::getName(void) {
  return "xtsc_lookup_sd";
}





class xtsc_lookup_sdFactory : public MxFactory {
public:
  xtsc_lookup_sdFactory() : MxFactory ("xtsc_lookup_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_lookup_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_lookup_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




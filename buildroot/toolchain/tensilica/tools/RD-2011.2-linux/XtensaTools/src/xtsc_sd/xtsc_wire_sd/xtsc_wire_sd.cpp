// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_wire_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



xtsc_wire_sd::xtsc_wire_sd(sc_mx_m_base* c, const sc_module_name &module_name) : 
  sc_mx_import_module   (c, module_name, "xtsc_wire_sd"),
  m_write               ("m_write"),
  m_read                ("m_read")
{
  m_init_complete       = false;
  m_bit_width           = 0;
  m_initial_value       = "0x0";
  m_write_file          = "";
  m_read_file           = "";
  m_wraparound          = false;
  m_timestamp           = false;

  defineParameter("bit_width",          "32",           MX_PARAM_VALUE,  0);
  defineParameter("initial_value",      "0x0",          MX_PARAM_STRING, 0);
  defineParameter("write_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("read_file",          "",             MX_PARAM_STRING, 0);
  defineParameter("wraparound",         "false",        MX_PARAM_BOOL,   0);
  defineParameter("timestamp",          "true",         MX_PARAM_BOOL,   0);

  registerSCGenericSlavePort(&m_write, "m_write");
  registerSCGenericSlavePort(&m_read,  "m_read");
}



xtsc_wire_sd::~xtsc_wire_sd() {
}



void xtsc_wire_sd::init() {
  xtsc_sd_initialize();

  xtsc_wire_parms wire_parms;
  wire_parms.set("bit_width",               m_bit_width);
  wire_parms.set("initial_value",           (char*)((m_initial_value=="") ? "0x0" : m_initial_value.c_str()));
  wire_parms.set("write_file",              (char*)((m_write_file=="") ? NULL : m_write_file.c_str()));
  wire_parms.set("read_file",               (char*)((m_read_file=="") ? NULL : m_read_file.c_str()));
  wire_parms.set("wraparound",              m_wraparound);
  wire_parms.set("timestamp",               m_timestamp);
  m_p_wire = new xtsc_wire((getInstanceName()+"_").c_str(), wire_parms);
  m_write(*m_p_wire);
  m_read(*m_p_wire);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_wire_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_wire->reset(level == MX_RESET_HARD);
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_wire_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_wire_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_wire_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "bit_width") {
    status = MxConvertStringToValue(value, &m_bit_width);
  }
  else if (name == "initial_value") {
    m_initial_value = value;
  }
  else if (name == "write_file") {
    m_write_file = value;
  }
  else if (name == "read_file") {
    m_read_file = value;
  }
  else if (name == "wraparound") {
    status = MxConvertStringToValue(value, &m_wraparound);
  }
  else if (name == "timestamp") {
    status = MxConvertStringToValue(value, &m_timestamp);
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_wire_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_wire_sd::getProperty(MxPropertyType property) {
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
           description = "Maxsim wrapper for xtsc_wire";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_wire_sd::getName(void) {
  return "xtsc_wire_sd";
}





class xtsc_wire_sdFactory : public MxFactory {
public:
  xtsc_wire_sdFactory() : MxFactory ("xtsc_wire_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_wire_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_wire_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




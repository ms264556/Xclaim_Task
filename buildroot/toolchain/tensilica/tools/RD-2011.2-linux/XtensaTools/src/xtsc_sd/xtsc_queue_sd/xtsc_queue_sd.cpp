// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_queue_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



xtsc_queue_sd::xtsc_queue_sd(sc_mx_m_base* c, const sc_module_name &module_name) : 
  sc_mx_import_module   (c, module_name, "xtsc_queue_sd"),
  m_producer            ("m_producer"),
  m_consumer            ("m_consumer")
{
  m_init_complete        = false;
  m_bit_width           = 0;
  m_depth               = 0;
  m_push_file           = "";
  m_pop_file            = "";
  m_wraparound          = false;
  m_timestamp           = false;

  defineParameter("bit_width",            "32",         MX_PARAM_VALUE,  0);
  defineParameter("depth",                "16",         MX_PARAM_VALUE,  0);
  defineParameter("push_file",            "",           MX_PARAM_STRING, 0);
  defineParameter("pop_file",             "",           MX_PARAM_STRING, 0);
  defineParameter("wraparound",           "false",      MX_PARAM_BOOL,   0);
  defineParameter("timestamp",            "true",         MX_PARAM_BOOL,   0);

  registerSCGenericSlavePort(&m_producer, "m_producer");
  registerSCGenericSlavePort(&m_consumer, "m_consumer");
}



xtsc_queue_sd::~xtsc_queue_sd() {
}



void xtsc_queue_sd::init() {
  xtsc_sd_initialize();

  xtsc_queue_parms queue_parms;
  queue_parms.set("bit_width",            m_bit_width);
  queue_parms.set("depth",                m_depth);
  queue_parms.set("push_file",            ((m_push_file == "") ? NULL : m_push_file.c_str()));
  queue_parms.set("pop_file",             ((m_pop_file  == "") ? NULL : m_pop_file .c_str()));
  queue_parms.set("wraparound",           m_wraparound);
  queue_parms.set("timestamp",            m_timestamp);
  m_p_queue = new xtsc_queue((getInstanceName()+"_").c_str(), queue_parms);
  m_producer(m_p_queue->m_producer);
  m_consumer(m_p_queue->m_consumer);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_queue_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_queue_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_queue_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_queue_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "bit_width") {
    status = MxConvertStringToValue(value, &m_bit_width);
  }
  else if (name == "depth") {
    status = MxConvertStringToValue(value, &m_depth);
  }
  else if (name == "push_file") {
    m_push_file = value;
  }
  else if (name == "pop_file") {
    m_pop_file = value;
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
    message(MX_MSG_WARNING, "xtsc_queue_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_queue_sd::getProperty(MxPropertyType property) {
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
           description = "Maxsim wrapper for xtsc_queue";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_queue_sd::getName(void) {
  return "xtsc_queue_sd";
}





class xtsc_queue_sdFactory : public MxFactory {
public:
  xtsc_queue_sdFactory() : MxFactory ("xtsc_queue_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_queue_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_queue_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




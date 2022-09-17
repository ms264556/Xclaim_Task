// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_tx_loader_sd.h"
#include <xtsc_sd/xtsc_sd.h>


using namespace xtsc;
using namespace xtsc_sd;



xtsc_tx_loader_sd::xtsc_tx_loader_sd(sc_mx_m_base* c, const sc_module_name &module_name) : 
  sc_mx_import_module   (c, module_name, "xtsc_tx_loader_sd"),
  m_tx_xfer_port        ("m_tx_xfer_port"),
  m_tx_xfer_export      ("m_tx_xfer_export"),
  m_done                ("m_done"),
  m_mode                ("m_mode"),
  m_producer            ("m_producer"),
  m_consumer            ("m_consumer")
{
  m_init_complete       = false;
  m_allow_overflow      = false;
  m_binary_format       = false;
  m_hex_format          = false;
  m_image_file          = "";
  m_read_fifo_depth     = 0;
  m_squelch_loading     = false;
  m_turbo               = false;

  defineParameter("allow_overflow",     "true",         MX_PARAM_BOOL,   0);
  defineParameter("binary_format",      "false",        MX_PARAM_BOOL,   0);
  defineParameter("hex_format",         "true",         MX_PARAM_BOOL,   0);
  defineParameter("image_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("read_fifo_depth",    "0x1",          MX_PARAM_VALUE,  0);
  defineParameter("squelch_loading",    "true",         MX_PARAM_BOOL,   0);
  defineParameter("turbo",              "false",        MX_PARAM_BOOL,   0);

  registerSCGenericMasterPort(&m_tx_xfer_port,   "m_tx_xfer_port");
  registerSCGenericSlavePort (&m_tx_xfer_export, "m_tx_xfer_export");
  registerSCGenericMasterPort(&m_done,           "m_done");
  registerSCGenericMasterPort(&m_mode,           "m_mode");
  registerSCGenericSlavePort (&m_producer,       "m_producer");
  registerSCGenericSlavePort (&m_consumer,       "m_consumer");
}



xtsc_tx_loader_sd::~xtsc_tx_loader_sd() {
}



void xtsc_tx_loader_sd::init() {
  xtsc_sd_initialize();

  xtsc_tx_loader_parms loader_parms;
  loader_parms.set("allow_overflow",          m_allow_overflow);
  loader_parms.set("binary_format",           m_binary_format);
  loader_parms.set("hex_format",              m_hex_format);
  loader_parms.set("image_file",              (char*)((m_image_file=="") ? NULL : m_image_file.c_str()));
  loader_parms.set("read_fifo_depth",         m_read_fifo_depth);
  loader_parms.set("squelch_loading",         m_squelch_loading);
  loader_parms.set("turbo",                   m_turbo);
  m_p_loader = new xtsc_tx_loader((getInstanceName()+"_").c_str(), loader_parms);
  m_p_loader->m_tx_xfer_port(m_tx_xfer_port);
  m_tx_xfer_export(m_p_loader->m_tx_xfer_export);
  m_p_loader->m_done(m_done);
  m_p_loader->m_mode(m_mode);
  m_producer(*m_p_loader->m_producer);
  m_consumer(*m_p_loader->m_consumer);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_tx_loader_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_tx_loader_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_tx_loader_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_tx_loader_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "allow_overflow") {
    status = MxConvertStringToValue(value, &m_allow_overflow);
  }
  else if (name == "binary_format") {
    status = MxConvertStringToValue(value, &m_binary_format);
  }
  else if (name == "hex_format") {
    status = MxConvertStringToValue(value, &m_hex_format);
  }
  else if (name == "image_file") {
    m_image_file = value;
  }
  else if (name == "read_fifo_depth") {
    status = MxConvertStringToValue(value, &m_read_fifo_depth);
  }
  else if (name == "squelch_loading") {
    status = MxConvertStringToValue(value, &m_squelch_loading);
  }
  else if (name == "turbo") {
    status = MxConvertStringToValue(value, &m_turbo);
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_tx_loader_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_tx_loader_sd::getProperty(MxPropertyType property) {
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
           description = "Maxsim wrapper for xtsc_tx_loader";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__; 
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



string xtsc_tx_loader_sd::getName(void) {
  return "xtsc_tx_loader_sd";
}





class xtsc_tx_loader_sdFactory : public MxFactory {
public:
  xtsc_tx_loader_sdFactory() : MxFactory ("xtsc_tx_loader_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_tx_loader_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_tx_loader_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




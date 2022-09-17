// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_memory_sd.h"


using namespace xtsc;
using namespace xtsc_component;
using namespace xtsc_sd;



static string name_helper(u32 number) {
  ostringstream oss;
  oss << "xtsc_memory_sd_" << number;
  return oss.str();
}



xtsc_memory_sd::xtsc_memory_sd(sc_mx_m_base* c, const sc_module_name &module_name, u32 num_ports) : 
  sc_mx_import_module   (c, module_name, name_helper(num_ports)),
  m_num_ports           (num_ports)
{
  m_init_complete               = false;
  m_byte_width                  = 0;
  m_delay_from_receipt          = false;
  m_read_delay                  = 0;
  m_block_read_delay            = 0;
  m_block_read_repeat           = 0;
  m_rcw_repeat                  = 0;
  m_rcw_response                = 0;
  m_write_delay                 = 0;
  m_block_write_delay           = 0;
  m_block_write_repeat          = 0;
  m_block_write_response        = 0;
  m_start_byte_address          = 0;
  m_memory_byte_size            = 0;
  m_request_fifo_depth          = 0;
  m_page_byte_size              = 0;
  m_read_only                   = false;
  m_initial_value_file          = "";
  m_memory_fill_byte            = 0;
  m_immediate_timing            = false;
  m_response_repeat             = 0;
  m_recovery_time               = 0;
  m_clock_period                = 0;
  m_use_fast_access             = false;
  m_use_raw_access              = false;
  m_script_file                 = "";
  m_wraparound                  = false;
  m_fail_status                 = 0;
  m_fail_request_mask           = 0;
  m_fail_percentage             = 0;
  m_fail_seed                   = 0;
  m_check_alignment             = false;
  m_burst_read_delay            = 0;
  m_burst_read_repeat           = 0;
  m_burst_write_delay           = 0;
  m_burst_write_repeat          = 0;
  m_burst_write_response        = 0;
  m_deny_fast_access.clear();

  defineParameter("byte_width",                 "0",            MX_PARAM_VALUE,  0);
  defineParameter("delay_from_receipt",         "true",         MX_PARAM_BOOL,   0);
  defineParameter("read_delay",                 "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_read_delay",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_read_repeat",          "1",            MX_PARAM_VALUE,  0);
  defineParameter("rcw_repeat",                 "1",            MX_PARAM_VALUE,  0);
  defineParameter("rcw_response",               "0",            MX_PARAM_VALUE,  0);
  defineParameter("write_delay",                "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_write_delay",          "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_write_repeat",         "0",            MX_PARAM_VALUE,  0);
  defineParameter("block_write_response",       "0",            MX_PARAM_VALUE,  0);
  defineParameter("start_byte_address",         "0",            MX_PARAM_VALUE,  0);
  defineParameter("memory_byte_size",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("request_fifo_depth",         "2",            MX_PARAM_VALUE,  0);
  defineParameter("page_byte_size",             "0x4000",       MX_PARAM_VALUE,  0);
  defineParameter("read_only",                  "false",        MX_PARAM_BOOL,   0);
  defineParameter("initial_value_file",         "",             MX_PARAM_STRING, 0);
  defineParameter("memory_fill_byte",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("immediate_timing",           "false",        MX_PARAM_BOOL,   0);
  defineParameter("response_repeat",            "1",            MX_PARAM_VALUE,  0);
  defineParameter("recovery_time",              "1",            MX_PARAM_VALUE,  0);
  defineParameter("clock_period",               "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("use_fast_access",            "true",         MX_PARAM_BOOL,   0);
  defineParameter("use_raw_access",             "true",         MX_PARAM_BOOL,   0);
  defineParameter("script_file",                "",             MX_PARAM_STRING, 0);
  defineParameter("wraparound",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("fail_status",                "4",            MX_PARAM_VALUE,  0);
  defineParameter("fail_request_mask",          "0x00000000",   MX_PARAM_VALUE,  0);
  defineParameter("fail_percentage",            "100",          MX_PARAM_VALUE,  0);
  defineParameter("fail_seed",                  "1",            MX_PARAM_VALUE,  0);
  defineParameter("check_alignment",            "false",        MX_PARAM_BOOL,   0);
  defineParameter("burst_read_delay",           "0",            MX_PARAM_VALUE,  0);
  defineParameter("burst_read_repeat",          "1",            MX_PARAM_VALUE,  0);
  defineParameter("burst_write_delay",          "0",            MX_PARAM_VALUE,  0);
  defineParameter("burst_write_repeat",         "1",            MX_PARAM_VALUE,  0);
  defineParameter("burst_write_response",       "0",            MX_PARAM_VALUE,  0);
  defineParameter("deny_fast_access",           "",             MX_PARAM_STRING, 0);

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
}



xtsc_memory_sd::~xtsc_memory_sd() {
}



void xtsc_memory_sd::init() {
  xtsc_sd_initialize();

  xtsc_memory_parms memory_parms;
  memory_parms.set("num_ports",               m_num_ports);
  memory_parms.set("byte_width",              m_byte_width);
  memory_parms.set("delay_from_receipt",      m_delay_from_receipt);
  memory_parms.set("read_delay",              m_read_delay);
  memory_parms.set("block_read_delay",        m_block_read_delay);
  memory_parms.set("block_read_repeat",       m_block_read_repeat);
  memory_parms.set("rcw_repeat",              m_rcw_repeat);
  memory_parms.set("rcw_response",            m_rcw_response);
  memory_parms.set("write_delay",             m_write_delay);
  memory_parms.set("block_write_delay",       m_block_write_delay);
  memory_parms.set("block_write_repeat",      m_block_write_repeat);
  memory_parms.set("block_write_response",    m_block_write_response);
  memory_parms.set("start_byte_address",      m_start_byte_address);
  memory_parms.set("memory_byte_size",        m_memory_byte_size);
  memory_parms.set("request_fifo_depth",      m_request_fifo_depth);
  memory_parms.set("page_byte_size",          m_page_byte_size);
  memory_parms.set("read_only",               m_read_only);
  memory_parms.set("initial_value_file",      (char*)((m_initial_value_file=="") ? NULL : m_initial_value_file.c_str()));
  memory_parms.set("memory_fill_byte",        m_memory_fill_byte);
  memory_parms.set("immediate_timing",        m_immediate_timing);
  memory_parms.set("response_repeat",         m_response_repeat);
  memory_parms.set("recovery_time",           m_recovery_time);
  memory_parms.set("clock_period",            m_clock_period);
  memory_parms.set("use_fast_access",         m_use_fast_access);
  memory_parms.set("use_raw_access",          m_use_raw_access);
  memory_parms.set("script_file",             (char*)((m_script_file=="") ? NULL : m_script_file.c_str()));
  memory_parms.set("wraparound",              m_wraparound);
  memory_parms.set("fail_status",             m_fail_status);
  memory_parms.set("fail_request_mask",       m_fail_request_mask);
  memory_parms.set("fail_percentage",         m_fail_percentage);
  memory_parms.set("fail_seed",               m_fail_seed);
  memory_parms.set("check_alignment",         m_check_alignment);
  memory_parms.set("burst_read_delay",        m_burst_read_delay);
  memory_parms.set("burst_read_repeat",       m_burst_read_repeat);
  memory_parms.set("burst_write_delay",       m_burst_write_delay);
  memory_parms.set("burst_write_repeat",      m_burst_write_repeat);
  memory_parms.set("burst_write_response",    m_burst_write_response);
  memory_parms.set("deny_fast_access",        m_deny_fast_access);
  m_p_memory = new xtsc_memory((getInstanceName()+"_").c_str(), memory_parms);

  for (u32 i=0; i<m_num_ports; ++i) {
    (*m_p_memory->m_respond_ports[i])(*m_respond_ports[i]);
    (*m_request_exports[i])(*m_p_memory->m_request_exports[i]);
  }

  features.nrMemSpaces = 1;

  u32 end_address = (m_memory_byte_size ? (m_start_byte_address + m_memory_byte_size - 1) : 0xFFFFFFFF);

  m_MemSpaceInfo.memSpaceId          = 0;
  m_MemSpaceInfo.bitsPerMau          = 8;
  m_MemSpaceInfo.minAddress          = m_start_byte_address;
  m_MemSpaceInfo.maxAddress          = end_address;
  m_MemSpaceInfo.nrMemBlocks         = 1;
  m_MemSpaceInfo.isProgramMemory     = 0;
  m_MemSpaceInfo.endianness          = 1; // endian: 0=mono-endian (arch defined), 1=LE, 2=BE
  strcpy(m_MemSpaceInfo.memSpaceName, getInstanceName().c_str());
  strcpy(m_MemSpaceInfo.description,  getInstanceName().c_str());

  m_MemBlockInfo.id                  = 0;
  m_MemBlockInfo.parentID            = 0;
  m_MemBlockInfo.startAddr           = m_start_byte_address;
  m_MemBlockInfo.endAddr             = end_address;
  m_MemBlockInfo.cyclesToAccess      = m_read_delay;
  m_MemBlockInfo.readWrite           = (m_read_only ? MXDI_MEM_ReadOnly : MXDI_MEM_ReadWrite);

  strcpy(m_MemBlockInfo.name,          getInstanceName().c_str());

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_memory_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  m_p_memory->reset(level == MX_RESET_HARD);
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_memory_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}




void xtsc_memory_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_memory_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "byte_width") {
    status = MxConvertStringToValue(value, &m_byte_width);
  }
  else if (name == "delay_from_receipt") {
    status = MxConvertStringToValue(value, &m_delay_from_receipt);
  }
  else if (name == "read_delay") {
    status = MxConvertStringToValue(value, &m_read_delay);
  }
  else if (name == "block_read_delay") {
    status = MxConvertStringToValue(value, &m_block_read_delay);
  }
  else if (name == "block_read_repeat") {
    status = MxConvertStringToValue(value, &m_block_read_repeat);
  }
  else if (name == "rcw_repeat") {
    status = MxConvertStringToValue(value, &m_rcw_repeat);
  }
  else if (name == "rcw_response") {
    status = MxConvertStringToValue(value, &m_rcw_response);
  }
  else if (name == "write_delay") {
    status = MxConvertStringToValue(value, &m_write_delay);
  }
  else if (name == "block_write_delay") {
    status = MxConvertStringToValue(value, &m_block_write_delay);
  }
  else if (name == "block_write_repeat") {
    status = MxConvertStringToValue(value, &m_block_write_repeat);
  }
  else if (name == "block_write_response") {
    status = MxConvertStringToValue(value, &m_block_write_response);
  }
  else if (name == "start_byte_address") {
    status = MxConvertStringToValue(value, &m_start_byte_address);
  }
  else if (name == "memory_byte_size") {
    status = MxConvertStringToValue(value, &m_memory_byte_size);
  }
  else if (name == "request_fifo_depth") {
    status = MxConvertStringToValue(value, &m_request_fifo_depth);
  }
  else if (name == "page_byte_size") {
    status = MxConvertStringToValue(value, &m_page_byte_size);
  }
  else if (name == "read_only") {
    status = MxConvertStringToValue(value, &m_read_only);
  }
  else if (name == "initial_value_file") {
    m_initial_value_file = value;
  }
  else if (name == "memory_fill_byte") {
    status = MxConvertStringToValue(value, &m_memory_fill_byte);
  }
  else if (name == "immediate_timing") {
    status = MxConvertStringToValue(value, &m_immediate_timing);
  }
  else if (name == "response_repeat") {
    status = MxConvertStringToValue(value, &m_response_repeat);
  }
  else if (name == "recovery_time") {
    status = MxConvertStringToValue(value, &m_recovery_time);
  }
  else if (name == "clock_period") {
    status = MxConvertStringToValue(value, &m_clock_period);
  }
  else if (name == "use_fast_access") {
    status = MxConvertStringToValue(value, &m_use_fast_access);
  }
  else if (name == "use_raw_access") {
    status = MxConvertStringToValue(value, &m_use_raw_access);
  }
  else if (name == "script_file") {
    m_script_file = value;
  }
  else if (name == "wraparound") {
    status = MxConvertStringToValue(value, &m_wraparound);
  }
  else if (name == "fail_status") {
    status = MxConvertStringToValue(value, &m_fail_status);
  }
  else if (name == "fail_request_mask") {
    status = MxConvertStringToValue(value, &m_fail_request_mask);
  }
  else if (name == "fail_percentage") {
    status = MxConvertStringToValue(value, &m_fail_percentage);
  }
  else if (name == "fail_seed") {
    status = MxConvertStringToValue(value, &m_fail_seed);
  }
  else if (name == "check_alignment") {
    status = MxConvertStringToValue(value, &m_check_alignment);
  }
  else if (name == "burst_read_delay") {
    status = MxConvertStringToValue(value, &m_burst_read_delay);
  }
  else if (name == "burst_read_repeat") {
    status = MxConvertStringToValue(value, &m_burst_read_repeat);
  }
  else if (name == "burst_write_delay") {
    status = MxConvertStringToValue(value, &m_burst_write_delay);
  }
  else if (name == "burst_write_repeat") {
    status = MxConvertStringToValue(value, &m_burst_write_repeat);
  }
  else if (name == "burst_write_response") {
    status = MxConvertStringToValue(value, &m_burst_write_response);
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
  }

  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_memory_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_memory_sd::getProperty(MxPropertyType property) {
  ostringstream oss;
  switch (property) {
    case MX_PROP_LOADFILE_EXTENSION: {
      return "";
    }
    case MX_PROP_REPORT_FILE_EXT: {
      return "yes";
    }
    case MX_PROP_COMPONENT_TYPE: {
      return "Memory"; 
    }
    case MX_PROP_COMPONENT_VERSION: {
      return "0.1";
    }
    case MX_PROP_MSG_PREPEND_NAME: {
      return "yes"; 
    }
    case MX_PROP_DESCRIPTION: {
      ostringstream oss;
      oss << "Maxsim wrapper for " << ((m_num_ports==1) ? "single" : "dual") << "-ported xtsc_memory. "
          << "Compiled on " << __DATE__ << ", " << __TIME__; 
      return oss.str();
    }
    case MX_PROP_MXDI_SUPPORT: {
      return "yes";
    }
    case MX_PROP_SAVE_RESTORE: {
      return "no";
    }
    default: {
      return "";
    }
  }
}



string xtsc_memory_sd::getName(void) {
  return name_helper(m_num_ports);
}



MxDI* xtsc_memory_sd::getMxDI() {
    return this;
}



// Memory related functions
MxdiReturn_t xtsc_memory_sd::MxdiMemGetSpaces(MxU32                 spaceIndex,
                                              MxU32                 memSpaceSlots,
                                              MxU32                *memSpaceCount,
                                              MxdiMemSpaceInfo_t   *memSpace)
{
  if ((spaceIndex != 0) || (memSpaceSlots < 1)) {
    *memSpaceCount = 0;
    return MXDI_STATUS_OK;
  }

  memSpace[0] = m_MemSpaceInfo;

  *memSpaceCount = 1;

  return MXDI_STATUS_OK;
}



MxdiReturn_t xtsc_memory_sd::MxdiMemGetBlocks(MxU32                 memorySpace,
                                              MxU32                 blockIndex,
                                              MxU32                 memBlockSlots,
                                              MxU32                *memBlockCount,
                                              MxdiMemBlockInfo_t   *memBlock)
{
  if (memorySpace != 0) {
    return MXDI_STATUS_IllegalArgument;
  }

  if ((memBlockSlots >= 1) && (blockIndex == 0)) {
    memBlock[0] = m_MemBlockInfo;
    *memBlockCount = 1;
  }
  else {
    *memBlockCount = 0;
  }

  return MXDI_STATUS_OK;
}



MxdiReturn_t xtsc_memory_sd::MxdiMemWrite(MxdiAddrComplete_t        startAddress,
                                          MxU32                     unitsToWrite,
                                          MxU32                     unitSizeInBytes,
                                          const MxU8               *data,
                                          MxU32                    *actualNumOfUnitsWritten,
                                          MxU8                      /*doSideEffects*/)
{
  xtsc_address address8 = (xtsc_address) startAddress.location.addr;
  // void poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer)
  m_p_memory->poke(address8, unitsToWrite*unitSizeInBytes, data);
  *actualNumOfUnitsWritten = unitsToWrite;
  return MXDI_STATUS_OK;
}



MxdiReturn_t xtsc_memory_sd::MxdiMemRead(MxdiAddrComplete_t         startAddress,
                                         MxU32                      unitsToRead,
                                         MxU32                      unitSizeInBytes,
                                         MxU8                      *data,
                                         MxU32                     *actualNumOfUnitsRead,
                                         MxU8                       /*doSideEffects*/)
{
  xtsc_address address8 = (xtsc_address) startAddress.location.addr;
  // void peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer)
  m_p_memory->peek(address8, unitsToRead*unitSizeInBytes, data);
  *actualNumOfUnitsRead = unitsToRead;
  return MXDI_STATUS_OK;
}





class xtsc_memory_sd_1Factory : public MxFactory {
public:
  xtsc_memory_sd_1Factory() : MxFactory (name_helper(1)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_memory_sd(c, id.c_str(), 1);
  }
};



class xtsc_memory_sd_2Factory : public MxFactory {
public:
  xtsc_memory_sd_2Factory() : MxFactory (name_helper(2)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_memory_sd(c, id.c_str(), 2);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_memory_sd_1Factory();
  new xtsc_memory_sd_2Factory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




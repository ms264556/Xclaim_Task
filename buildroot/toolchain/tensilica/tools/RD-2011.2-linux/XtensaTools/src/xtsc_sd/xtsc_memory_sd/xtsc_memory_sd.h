#ifndef _XTSC_MEMORY_SD_H_
#define _XTSC_MEMORY_SD_H_

// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include "sc_mx_import_module.h"
#include <xtsc_sd/xtsc_sd.h>
#include <xtsc/xtsc_memory.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_memory_sd : public sc_mx_import_module, public MxDIBase {
public:
  sc_port<xtsc::xtsc_respond_if>      **m_respond_ports;
  sc_export<xtsc::xtsc_request_if>    **m_request_exports;
      
  // constructor / destructor
  xtsc_memory_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 num_ports);
  virtual ~xtsc_memory_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

  MxDI* getMxDI();


  MxdiReturn_t MxdiMemGetSpaces(MxU32                   spaceIndex,
                                MxU32                   memSpaceSlots,
                                MxU32                  *memSpaceCount,
                                MxdiMemSpaceInfo_t     *memSpace);

  MxdiReturn_t MxdiMemGetBlocks(MxU32                   memorySpace,
                                MxU32                   blockIndex,
                                MxU32                   memBlockSlots,
                                MxU32                  *memBlockCount,
                                MxdiMemBlockInfo_t     *memBlock);

  MxdiReturn_t MxdiMemWrite(MxdiAddrComplete_t  startAddress,
                            MxU32               unitsToWrite,
                            MxU32               unitSizeInBytes,
                            const MxU8         *data,
                            MxU32              *actualNumOfUnitsWritten,
                            MxU8                doSideEffects);

  MxdiReturn_t MxdiMemRead(MxdiAddrComplete_t   startAddress,
                           MxU32                unitsToRead,
                           MxU32                unitSizeInBytes,
                           MxU8                *data,
                           MxU32               *actualNumOfUnitsRead,
                           MxU8                 doSideEffects);


private:

  xtsc::u32                             m_num_ports;
  bool                                  m_init_complete; 

  unsigned int                          m_byte_width;
  bool                                  m_delay_from_receipt;
  unsigned int                          m_read_delay;
  unsigned int                          m_block_read_delay;
  unsigned int                          m_block_read_repeat;
  unsigned int                          m_rcw_repeat;
  unsigned int                          m_rcw_response;
  unsigned int                          m_write_delay;
  unsigned int                          m_block_write_delay;
  unsigned int                          m_block_write_repeat;
  unsigned int                          m_block_write_response;
  unsigned int                          m_start_byte_address;
  unsigned int                          m_memory_byte_size;
  unsigned int                          m_request_fifo_depth;
  unsigned int                          m_page_byte_size;
  bool                                  m_read_only;
  string                                m_initial_value_file;
  unsigned int                          m_memory_fill_byte;
  bool                                  m_immediate_timing;
  unsigned int                          m_response_repeat;
  unsigned int                          m_recovery_time;
  unsigned int                          m_clock_period;
  bool                                  m_use_fast_access;
  bool                                  m_use_raw_access;
  string                                m_script_file;
  bool                                  m_wraparound;
  unsigned int                          m_fail_status;
  unsigned int                          m_fail_request_mask;
  unsigned int                          m_fail_percentage;
  unsigned int                          m_fail_seed;
  bool                                  m_check_alignment;
  unsigned int                          m_burst_read_delay;
  unsigned int                          m_burst_read_repeat;
  unsigned int                          m_burst_write_delay;
  unsigned int                          m_burst_write_repeat;
  unsigned int                          m_burst_write_response;
  vector<xtsc::u32>                     m_deny_fast_access;

  xtsc_component::xtsc_memory          *m_p_memory;

  MxdiMemSpaceInfo_t                    m_MemSpaceInfo;
  MxdiMemBlockInfo_t                    m_MemBlockInfo;
};

#endif  // _XTSC_MEMORY_SD_H_

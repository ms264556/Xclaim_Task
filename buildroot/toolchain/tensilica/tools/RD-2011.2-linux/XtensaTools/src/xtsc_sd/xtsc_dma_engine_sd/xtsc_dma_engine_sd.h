#ifndef _XTSC_DMA_ENGINE_SD_H_
#define _XTSC_DMA_ENGINE_SD_H_

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
#include <xtsc/xtsc_dma_engine.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_dma_engine_sd : public sc_mx_import_module {
public:
  // Communicate with master
  sc_export<xtsc::xtsc_request_if>      m_request_export;
  sc_port<xtsc::xtsc_respond_if>        m_respond_port;
      
  // Communicate with slave
  sc_port<xtsc::xtsc_request_if>        m_request_port;
  sc_export<xtsc::xtsc_respond_if>      m_respond_export;

  // constructor / destructor
  xtsc_dma_engine_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_dma_engine_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  xtsc::u32                             m_num_ports;
  bool                                  m_init_complete; 

  unsigned int                          m_byte_width;
  bool                                  m_check_alignment;
  bool                                  m_clear_notify_value;
  unsigned int                          m_clock_period;
  bool                                  m_delay_from_receipt;
  vector<xtsc::u32>                     m_deny_fast_access;
  bool                                  m_immediate_timing;
  unsigned int                          m_memory_byte_size;
  unsigned int                          m_nacc_wait_time;
  unsigned int                          m_page_byte_size;
  unsigned int                          m_read_delay;
  unsigned int                          m_read_priority;
  unsigned int                          m_recovery_time;
  unsigned int                          m_reg_base_address;
  unsigned int                          m_request_fifo_depth;
  unsigned int                          m_response_repeat;
  unsigned int                          m_start_byte_address;
  bool                                  m_turbo;
  bool                                  m_use_fast_access;
  bool                                  m_use_raw_access;
  unsigned int                          m_write_delay;
  unsigned int                          m_write_priority;

  xtsc_component::xtsc_dma_engine      *m_p_dma_engine;
};

#endif  // _XTSC_DMA_ENGINE_SD_H_

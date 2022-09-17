#ifndef _XTSC_ARBITER_SD_H_
#define _XTSC_ARBITER_SD_H_

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
#include <xtsc/xtsc_arbiter.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_arbiter_sd : public sc_mx_import_module {
public:
  // Communicate with masters
  sc_port<xtsc::xtsc_respond_if>      **m_respond_ports;
  sc_export<xtsc::xtsc_request_if>    **m_request_exports;
      
  // Communicate with slave
  sc_port<xtsc::xtsc_request_if>        m_request_port;
  sc_export<xtsc::xtsc_respond_if>      m_respond_export;
      
  // constructor / destructor
  xtsc_arbiter_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 num_ports);
  virtual ~xtsc_arbiter_sd();

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

  unsigned int                          m_route_id_lsb;
  string                                m_translation_file;
  bool                                  m_external_cbox;
  bool                                  m_immediate_timing;
  unsigned int                          m_request_fifo_depth;
  unsigned int                          m_response_fifo_depth;
  unsigned int                          m_clock_period;
  unsigned int                          m_arbitration_phase;
  unsigned int                          m_nacc_wait_time;
  bool                                  m_one_at_a_time;
  bool                                  m_delay_from_receipt;
  bool                                  m_read_only;
  bool                                  m_write_only;
  unsigned int                          m_request_delay;
  unsigned int                          m_response_delay;
  unsigned int                          m_response_repeat;
  unsigned int                          m_recovery_time;
  vector<xtsc::u32>                     m_master_byte_widths;
  unsigned int                          m_slave_byte_width;
  bool                                  m_use_block_requests;
  bool                                  m_dram_lock;
  unsigned int                          m_xfer_en_port;
  vector<xtsc::u32>                     m_request_fifo_depths;

  xtsc_component::xtsc_arbiter          *m_p_arbiter;
};

#endif  // _XTSC_ARBITER_SD_H_

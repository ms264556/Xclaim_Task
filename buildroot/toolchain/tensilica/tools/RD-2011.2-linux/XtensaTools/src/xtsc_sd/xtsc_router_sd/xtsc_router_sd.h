#ifndef _XTSC_ROUTER_SD_H_
#define _XTSC_ROUTER_SD_H_

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
#include <xtsc/xtsc_router.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_router_sd : public sc_mx_import_module {
public:
  // Communicate with master
  sc_export<xtsc::xtsc_request_if>      m_request_export;
  sc_port<xtsc::xtsc_respond_if>        m_respond_port;
      
  // Communicate with slaves
  sc_port<xtsc::xtsc_request_if>      **m_request_ports;
  sc_export<xtsc::xtsc_respond_if>    **m_respond_exports;

  // constructor / destructor
  xtsc_router_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 num_ports);
  virtual ~xtsc_router_sd();

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

  bool                                  m_default_routing;
  string                                m_routing_table;
  unsigned int                          m_default_delta;
  unsigned int                          m_default_port_num;
  unsigned int                          m_clock_period;
  bool                                  m_delay_from_receipt;
  bool                                  m_read_only;
  bool                                  m_write_only;
  unsigned int                          m_recovery_time;
  unsigned int                          m_request_delay;
  unsigned int                          m_nacc_wait_time;
  unsigned int                          m_response_delay;
  unsigned int                          m_response_repeat;
  bool                                  m_immediate_timing;
  unsigned int                          m_request_fifo_depth;
  unsigned int                          m_response_fifo_depth;
  vector<xtsc::u32>                     m_slave_byte_widths;
  unsigned int                          m_master_byte_width;
  bool                                  m_use_block_requests;
  vector<xtsc::u32>                     m_address_routing_bits;
  unsigned int                          m_read_delay;
  unsigned int                          m_write_delay;
  vector<xtsc::u32>                     m_response_fifo_depths;

  xtsc_component::xtsc_router          *m_p_router;
};

#endif  // _XTSC_ROUTER_SD_H_

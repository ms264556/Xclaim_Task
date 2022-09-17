#ifndef _XTSC_MEMORY_PIN_SD_H_
#define _XTSC_MEMORY_PIN_SD_H_

// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
#include <xtsc/xtsc_memory_pin.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_memory_pin_sd : public sc_mx_import_module, public xtsc_component::xtsc_module_pin_base {
public:

  sc_in <bool>                         *POReqValid;
  sc_out<bool>                         *PIReqRdy;
  sc_in <sc_uint_base >                *POReqAdrs;
  sc_in <sc_uint_base >                *POReqCntl;
  sc_in <sc_bv_base   >                *POReqData;
  sc_in <sc_uint_base >                *POReqDataBE;
  sc_in <sc_uint_base >                *POReqId;
  sc_in <sc_uint_base >                *POReqPriority;

  sc_out<bool>                         *PIRespValid;
  sc_in <bool>                         *PORespRdy;
  sc_out<sc_uint_base >                *PIRespCntl;
  sc_out<sc_bv_base   >                *PIRespData;
  sc_out<sc_uint_base >                *PIRespId;
  sc_out<sc_uint_base >                *PIRespPriority;

  // constructor / destructor
  xtsc_memory_pin_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 bit_width);
  virtual ~xtsc_memory_pin_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  xtsc::u32                             m_bit_width;
  bool                                  m_init_complete; 

  unsigned int                          m_start_byte_address;
  unsigned int                          m_memory_byte_size;
  unsigned int                          m_page_byte_size;
  string                                m_initial_value_file;
  unsigned int                          m_memory_fill_byte;
  bool                                  m_big_endian;
  unsigned int                          m_clock_period;
  unsigned int                          m_posedge_offset;
  unsigned int                          m_sample_phase;
  unsigned int                          m_drive_phase;
  unsigned int                          m_busy_percentage;
  bool                                  m_write_responses;
  unsigned int                          m_void_resp_cntl;
  unsigned int                          m_request_fifo_depth;
  unsigned int                          m_read_delay;
  unsigned int                          m_block_read_delay;
  unsigned int                          m_block_read_repeat;
  unsigned int                          m_burst_read_delay;
  unsigned int                          m_burst_read_repeat;
  unsigned int                          m_rcw_repeat;
  unsigned int                          m_rcw_response;
  unsigned int                          m_write_delay;
  unsigned int                          m_block_write_delay;
  unsigned int                          m_block_write_repeat;
  unsigned int                          m_block_write_response;
  unsigned int                          m_burst_write_delay;
  unsigned int                          m_burst_write_repeat;
  unsigned int                          m_burst_write_response;
  string                                m_vcd_name;

  xtsc_component::xtsc_memory_pin      *m_p_memory;
};

#endif  // _XTSC_MEMORY_PIN_SD_H_

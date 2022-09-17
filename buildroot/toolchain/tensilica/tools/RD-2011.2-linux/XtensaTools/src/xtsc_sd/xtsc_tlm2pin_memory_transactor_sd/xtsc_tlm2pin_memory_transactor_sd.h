#ifndef _XTSC_TLM2PIN_MEMORY_TRANSACTOR_SD_H_
#define _XTSC_TLM2PIN_MEMORY_TRANSACTOR_SD_H_

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
#include <xtsc/xtsc_tlm2pin_memory_transactor.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_tlm2pin_memory_transactor_sd : public sc_mx_import_module, public xtsc_component::xtsc_module_pin_base {
public:
  sc_export<xtsc::xtsc_request_if>      m_request_export;
  sc_port<xtsc::xtsc_respond_if>        m_respond_port;
      
  sc_out<bool>                         *POReqValid;
  sc_in <bool>                         *PIReqRdy;
  sc_out<sc_uint_base >                *POReqAdrs;
  sc_out<sc_uint_base >                *POReqCntl;
  sc_out<sc_bv_base   >                *POReqData;
  sc_out<sc_uint_base >                *POReqDataBE;
  sc_out<sc_uint_base >                *POReqId;
  sc_out<sc_uint_base >                *POReqPriority;

  sc_in <bool>                         *PIRespValid;
  sc_out<bool>                         *PORespRdy;
  sc_in <sc_uint_base >                *PIRespCntl;
  sc_in <sc_bv_base   >                *PIRespData;
  sc_in <sc_uint_base >                *PIRespId;
  sc_in <sc_uint_base >                *PIRespPriority;

  // constructor / destructor
  xtsc_tlm2pin_memory_transactor_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 bit_width);
  virtual ~xtsc_tlm2pin_memory_transactor_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  xtsc::u32                                             m_bit_width;
  bool                                                  m_init_complete; 

  unsigned int                                          m_start_byte_address;
  bool                                                  m_big_endian;
  string                                                m_dso_name;
  string                                                m_dso_cookie;
  bool                                                  m_cosim;
  bool                                                  m_shadow_memory;
  string                                                m_initial_value_file;
  unsigned int                                          m_memory_fill_byte;
  unsigned int                                          m_clock_period;
  unsigned int                                          m_posedge_offset;
  unsigned int                                          m_sample_phase;
  unsigned int                                          m_output_delay;
  unsigned int                                          m_request_fifo_depth;
  bool                                                  m_write_responses;
  string                                                m_vcd_name;

  xtsc_component::xtsc_tlm2pin_memory_transactor       *m_p_tlm2pin;
};

#endif  // _XTSC_TLM2PIN_MEMORY_TRANSACTOR_SD_H_

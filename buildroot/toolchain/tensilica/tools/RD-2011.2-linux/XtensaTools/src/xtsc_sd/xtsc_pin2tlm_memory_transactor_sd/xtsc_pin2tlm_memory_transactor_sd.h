#ifndef _XTSC_PIN2TLM_MEMORY_TRANSACTOR_SD_H_
#define _XTSC_PIN2TLM_MEMORY_TRANSACTOR_SD_H_

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
#include <xtsc/xtsc_pin2tlm_memory_transactor.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_pin2tlm_memory_transactor_sd : public sc_mx_import_module, public xtsc_component::xtsc_module_pin_base {
public:
  sc_port<xtsc::xtsc_request_if>        m_request_port;
  sc_export<xtsc::xtsc_respond_if>      m_respond_export;
      
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
  xtsc_pin2tlm_memory_transactor_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 bit_width);
  virtual ~xtsc_pin2tlm_memory_transactor_sd();

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
  unsigned int                                          m_clock_period;
  unsigned int                                          m_posedge_offset;
  unsigned int                                          m_sample_phase;
  unsigned int                                          m_output_delay;
  bool                                                  m_inbound_pif;
  bool                                                  m_prereject_responses;
  string                                                m_vcd_name;

  xtsc_component::xtsc_pin2tlm_memory_transactor       *m_p_pin2tlm;
};

#endif  // _XTSC_PIN2TLM_MEMORY_TRANSACTOR_SD_H_

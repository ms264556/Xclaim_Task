#ifndef _XTSC_LOOKUP_SD_H_
#define _XTSC_LOOKUP_SD_H_

// Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include "sc_mx_import_module.h"
#include <xtsc/xtsc_lookup.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_lookup_sd : public sc_mx_import_module {
public:

  sc_export<xtsc::xtsc_lookup_if>      m_lookup;       ///<  Driver binds to this

      
  // constructor / destructor
  xtsc_lookup_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_lookup_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  bool                                  m_init_complete; 

  bool                                  m_ram;
  unsigned int                          m_address_bit_width;
  unsigned int                          m_data_bit_width;
  unsigned int                          m_write_data_lsb;
  unsigned int                          m_write_strobe_bit;
  bool                                  m_active_high_strobe;
  bool                                  m_has_ready;
  unsigned int                          m_pipeline_depth;
  bool                                  m_enforce_latency;
  unsigned int                          m_latency;
  unsigned int                          m_delay;
  string                                m_lookup_table;
  string                                m_default_data;
  unsigned int                          m_clock_period;

  xtsc_component::xtsc_lookup          *m_p_lookup;
};

#endif  // _XTSC_LOOKUP_SD_H_

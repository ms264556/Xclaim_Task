#ifndef _XTSC_LOOKUP_DRIVER_SD_H_
#define _XTSC_LOOKUP_DRIVER_SD_H_

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
#include <xtsc/xtsc_lookup_driver.h>

#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


class xtsc_lookup_driver_sd : public sc_mx_import_module {
public:
  sc_port<xtsc::xtsc_lookup_if>         m_lookup;       ///<  TLM port to the lookup
      
  // constructor / destructor
  xtsc_lookup_driver_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_lookup_driver_sd();


  // overloaded sc_mx_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  bool                                  m_init_complete; 

  unsigned int                          m_address_bit_width;
  unsigned int                          m_data_bit_width;
  bool                                  m_has_ready;
  unsigned int                          m_latency;
  string                                m_script_file;
  unsigned int                          m_clock_period;
  unsigned int                          m_poll_ready_delay;

  xtsc_component::xtsc_lookup_driver   *m_p_lookup_driver;

};

#endif  // _XTSC_LOOKUP_DRIVER_SD_H_

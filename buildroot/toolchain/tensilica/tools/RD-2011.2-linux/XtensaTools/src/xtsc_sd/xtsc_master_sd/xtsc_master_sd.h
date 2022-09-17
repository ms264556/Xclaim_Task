#ifndef _XTSC_MASTER_SD_H_
#define _XTSC_MASTER_SD_H_

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
#include <xtsc/xtsc_master.h>

#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


class xtsc_master_sd : public sc_mx_import_module {
public:
  sc_port<xtsc::xtsc_request_if>        m_request_port;
  sc_export<xtsc::xtsc_respond_if>      m_respond_export;
  sc_export<xtsc::xtsc_wire_write_if>   m_control;              ///<  Optional 1-bit control input
      
  // constructor / destructor
  xtsc_master_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_master_sd();


  // overloaded sc_mx_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  bool                                  m_init_complete; 

  unsigned int                          m_clock_period;
  string                                m_return_value_file;
  string                                m_script_file;
  bool                                  m_wraparound;

  xtsc_component::xtsc_master          *m_p_master;

};

#endif  // _XTSC_MASTER_SD_H_

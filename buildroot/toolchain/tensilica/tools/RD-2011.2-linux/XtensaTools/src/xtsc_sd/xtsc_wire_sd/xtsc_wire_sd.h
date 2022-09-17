#ifndef _XTSC_WIRE_SD_H_
#define _XTSC_WIRE_SD_H_

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
#include <xtsc/xtsc_wire.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_wire_sd : public sc_mx_import_module {
public:

  sc_export<xtsc::xtsc_wire_write_if>   m_write;       ///<  Source binds to this
  sc_export<xtsc::xtsc_wire_read_if>    m_read;        ///<  Sink binds to this

      
  // constructor / destructor
  xtsc_wire_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_wire_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  bool                                  m_init_complete; 

  unsigned int                          m_bit_width;
  string                                m_initial_value;
  string                                m_write_file;
  string                                m_read_file;
  bool                                  m_wraparound;
  bool                                  m_timestamp;

  xtsc_component::xtsc_wire            *m_p_wire;
};

#endif  // _XTSC_WIRE_SD_H_

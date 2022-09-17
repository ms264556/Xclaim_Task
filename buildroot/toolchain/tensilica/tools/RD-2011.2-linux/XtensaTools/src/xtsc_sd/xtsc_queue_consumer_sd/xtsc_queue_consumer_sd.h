#ifndef _XTSC_QUEUE_CONSUMER_SD_H_
#define _XTSC_QUEUE_CONSUMER_SD_H_

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
#include <xtsc/xtsc_queue_consumer.h>

#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


class xtsc_queue_consumer_sd : public sc_mx_import_module {
public:
  sc_port<xtsc::xtsc_queue_pop_if>      m_queue;        ///<  TLM port to queue
      
  // constructor / destructor
  xtsc_queue_consumer_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_queue_consumer_sd();


  // overloaded sc_mx_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  bool                                  m_init_complete; 

  string                                m_script_file;
  unsigned int                          m_bit_width;
  unsigned int                          m_clock_period;
  bool                                  m_wraparound;

  xtsc_component::xtsc_queue_consumer  *m_p_queue_consumer;

};

#endif  // _XTSC_QUEUE_CONSUMER_SD_H_

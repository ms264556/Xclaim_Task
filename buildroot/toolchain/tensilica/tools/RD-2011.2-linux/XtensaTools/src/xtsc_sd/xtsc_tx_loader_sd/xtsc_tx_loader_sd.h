#ifndef _XTSC_TX_LOADER_SD_H_
#define _XTSC_TX_LOADER_SD_H_

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
#include <xtsc/xtsc_tx_loader.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



class xtsc_tx_loader_sd : public sc_mx_import_module {
public:

  sc_core::sc_port  <xtsc::xtsc_tx_xfer_if, NSPP>       m_tx_xfer_port;         ///<  From us to first TX in the chain
  sc_core::sc_export<xtsc::xtsc_tx_xfer_if>             m_tx_xfer_export;       ///<  From last TX in the chain to us

  sc_core::sc_port  <xtsc::xtsc_wire_write_if, NSPP>    m_done;                 ///<  Done output
  sc_core::sc_port  <xtsc::xtsc_wire_write_if, NSPP>    m_mode;                 ///<  Mode output

  sc_core::sc_export<xtsc::xtsc_queue_push_if>          m_producer;             ///<  Queue producer binds to this
  sc_core::sc_export<xtsc::xtsc_queue_pop_if>           m_consumer;             ///<  Queue consumer binds to this
      
  // constructor / destructor
  xtsc_tx_loader_sd(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_tx_loader_sd();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);

private:

  bool                                  m_init_complete; 

  bool                                  m_allow_overflow;
  bool                                  m_binary_format;
  bool                                  m_hex_format;
  string                                m_image_file;
  unsigned int                          m_read_fifo_depth;
  bool                                  m_squelch_loading;
  bool                                  m_turbo;

  xtsc::xtsc_tx_loader                 *m_p_loader;
};

#endif  // _XTSC_TX_LOADER_SD_H_

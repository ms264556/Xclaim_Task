#ifndef _XTSC_SC_SIGNAL_SC_BV_BASE_SD_H_
#define _XTSC_SC_SIGNAL_SC_BV_BASE_SD_H_

// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include "sc_mx_import_prim_channel.h"
#include <xtsc_sd/xtsc_sd.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }




class xtsc_length_context_base {
public:
  xtsc_length_context_base(xtsc::u32 length) :
    length_context(sc_length_param(length))
  {}
  void end() { length_context.end(); }
private:
  sc_length_context length_context;
};



class xtsc_sc_signal_sc_bv_base_sd :
  public xtsc_length_context_base,
  public sc_signal<sc_bv_base>,
  public sc_mx_import_prim_channel
{
public:

  xtsc_sc_signal_sc_bv_base_sd(sc_mx_m_base* c, const sc_module_name &module_name, xtsc::u32 bit_width);

private:

  xtsc::u32   m_bit_width;

};

#endif  // _XTSC_SC_SIGNAL_SC_BV_BASE_SD_H_

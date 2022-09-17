// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include "xtsc_sc_signal_sc_bv_base_sd.h"



using namespace xtsc;



static string name_helper(u32 number) {
  ostringstream oss;
  oss << "xtsc_sc_signal_sc_bv_base_sd_" << number;
  return oss.str();
}



xtsc_sc_signal_sc_bv_base_sd::xtsc_sc_signal_sc_bv_base_sd(sc_mx_m_base* c, const sc_module_name &module_name, u32 bit_width) : 
  xtsc_length_context_base      (bit_width),
  sc_signal<sc_bv_base>         (name_helper(bit_width).c_str()),
  sc_mx_import_prim_channel     (c, module_name, name_helper(bit_width)),
  m_bit_width                   (bit_width)
{
  end();

  registerSCGenericSlavePort(this, "in");
  registerSCGenericSlavePort(this, "out");
}



class xtsc_sc_signal_sc_bv_base_sd_32Factory : public MxFactory {
public:
  xtsc_sc_signal_sc_bv_base_sd_32Factory() : MxFactory (name_helper(32)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_sc_signal_sc_bv_base_sd(c, id.c_str(), 32);
  }
};



class xtsc_sc_signal_sc_bv_base_sd_64Factory : public MxFactory {
public:
  xtsc_sc_signal_sc_bv_base_sd_64Factory() : MxFactory (name_helper(64)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_sc_signal_sc_bv_base_sd(c, id.c_str(), 64);
  }
};



class xtsc_sc_signal_sc_bv_base_sd_128Factory : public MxFactory {
public:
  xtsc_sc_signal_sc_bv_base_sd_128Factory() : MxFactory (name_helper(128)) {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_sc_signal_sc_bv_base_sd(c, id.c_str(), 128);
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_sc_signal_sc_bv_base_sd_32Factory();
  new xtsc_sc_signal_sc_bv_base_sd_64Factory();
  new xtsc_sc_signal_sc_bv_base_sd_128Factory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




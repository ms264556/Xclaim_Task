// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <xtsc_sd/xtsc_tx_xfer_if_sd_mon.h>


using namespace xtsc_sd;



class xtsc_tx_xfer_if_sd_monFactory : public MxFactory {
public:
  xtsc_tx_xfer_if_sd_monFactory() : MxFactory ("xtsc_tx_xfer_if_sd_mon") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) {
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_tx_xfer_if_sd_mon(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_tx_xfer_if_sd_monFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}



// Avoid SoC Designer complaining about old version when module implementation is in another shared object
xtsc_sd::xtsc_tx_xfer_if_sd_mon::~xtsc_tx_xfer_if_sd_mon() { }


// Copyright (c) 2006-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

#include <xtsc_vp/xtsc_tx_loader_vp.h>

using namespace std;
using namespace sc_core;
using namespace xtsc;
using namespace xtsc_vp;



xtsc_vp::xtsc_tx_loader_vp::xtsc_tx_loader_vp(const sc_module_name &module_name) :
  sc_module                     (module_name),
  m_tx_xfer_port                ("m_tx_xfer_port"),
  m_tx_xfer_export              ("m_tx_xfer_export"),
  m_done                        ("m_done"),
  m_mode                        ("m_mode"),
  m_producer                    ("m_producer"),
  m_consumer                    ("m_consumer"),
  m_read_fifo_depth             ("/FIFO/read_fifo_depth", 1),
  m_allow_overflow              ("/Misc/allow_overflow",  true),
  m_binary_format               ("/Misc/binary_format",   false),
  m_hex_format                  ("/Misc/hex_format",      true),
  m_image_file                  ("/Misc/image_file",      ""),
  m_squelch_loading             ("/Misc/squelch_loading", true),
  m_clock_period                ("/Timing/clock_period",  0xFFFFFFFF),
  m_vcd_name                    ("/Trace/vcd_name",       ""),
  m_turbo                       ("/Turbo/turbo",          false),
  m_p_tx_loader                 (0)
{
  xtsc_vp_initialize();

  xtsc_tx_loader_parms tx_loader_parms;

  tx_loader_parms.set("allow_overflow",          m_allow_overflow);
  tx_loader_parms.set("binary_format",           m_binary_format);
  tx_loader_parms.set("clock_period",            m_clock_period);
  tx_loader_parms.set("hex_format",              m_hex_format);
  tx_loader_parms.set("image_file",              m_image_file.getValue().c_str());
  tx_loader_parms.set("read_fifo_depth",         m_read_fifo_depth);
  tx_loader_parms.set("squelch_loading",         m_squelch_loading);
  tx_loader_parms.set("turbo",                   m_turbo);
  if (m_vcd_name.getValue() != "") {
  tx_loader_parms.set("vcd_handle",              xtsc_vp_get_trace_file(m_vcd_name.getValue()));
  }

  tx_loader_parms.extract_parms(sc_argc(), sc_argv(), name());
  m_p_tx_loader = new xtsc_tx_loader("_", tx_loader_parms);

  m_p_tx_loader->m_tx_xfer_port(m_tx_xfer_port),
  m_tx_xfer_export(m_p_tx_loader->m_tx_xfer_export);
  m_p_tx_loader->m_done(m_done),
  m_p_tx_loader->m_mode(m_mode),
  m_producer(*m_p_tx_loader->m_producer);
  m_consumer(*m_p_tx_loader->m_consumer);
}



xtsc_vp::xtsc_tx_loader_vp::~xtsc_tx_loader_vp() {
  xtsc_vp_finalize();
}



void debut_xtsc_tx_loader_vp(void) {
  xtsc_tx_loader_vp debutante("debutante");
}




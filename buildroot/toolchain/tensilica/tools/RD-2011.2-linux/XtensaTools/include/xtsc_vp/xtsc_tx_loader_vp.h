#ifndef _XTSC_TX_LOADER_VP_H_
#define _XTSC_TX_LOADER_VP_H_

// Copyright (c) 2006-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <scml.h>
#include <xtsc/xtsc_tx_loader.h>
#include <xtsc_vp/xtsc_vp.h>



namespace xtsc_vp {


/// Wrapper for xtsc_tx_loader in Synopsys Virtual Prototyping Environment 
class xtsc_tx_loader_vp : public sc_core::sc_module {
public:

  sc_port  <xtsc::xtsc_tx_xfer_if, NSPP>     m_tx_xfer_port;    ///<  From us to first TX in the chain
  sc_export<xtsc::xtsc_tx_xfer_if>           m_tx_xfer_export;  ///<  From last TX in the chain to us

  sc_port  <xtsc::xtsc_wire_write_if, NSPP>  m_done;            ///<  Optional Done output
  sc_port  <xtsc::xtsc_wire_write_if, NSPP>  m_mode;            ///<  Optional Mode output

  sc_export<xtsc::xtsc_queue_push_if>        m_producer;        ///<  TLM: Optional queue producer binds to this
  sc_export<xtsc::xtsc_queue_pop_if>         m_consumer;        ///<  TLM: Optional queue consumer binds to this

  xtsc_tx_loader_vp(const sc_core::sc_module_name &module_name);
  virtual ~xtsc_tx_loader_vp();

private:

  scml_property<unsigned int>           m_read_fifo_depth;
  scml_property<bool>                   m_allow_overflow;
  scml_property<bool>                   m_binary_format;
  scml_property<bool>                   m_hex_format;
  scml_property<string>                 m_image_file;
  scml_property<bool>                   m_squelch_loading;
  scml_property<unsigned int>           m_clock_period;
  scml_property<string>                 m_vcd_name;
  scml_property<bool>                   m_turbo;

  xtsc::xtsc_tx_loader                 *m_p_tx_loader;

};



}  // namespace xtsc_vp


#endif  // _XTSC_TX_LOADER_VP_H_




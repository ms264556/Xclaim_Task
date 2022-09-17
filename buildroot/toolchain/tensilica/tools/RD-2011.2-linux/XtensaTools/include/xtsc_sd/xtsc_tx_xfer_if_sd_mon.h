#ifndef _XTSC_TX_XFER_IF_SD_MON_H_
#define _XTSC_TX_XFER_IF_SD_MON_H_

// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <xtsc_sd/xtsc_sd.h>
#include "sc_mx_import_module.h"
#include <xtsc/xtsc_tx_xfer_if.h>
#include <xtsc/xtsc_tx_xfer.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


namespace xtsc_sd {



class XTSC_SD_API xtsc_tx_xfer_if_sd_mon : public sc_mx_import_module, public MxDIBase {
public:
  sc_export<xtsc::xtsc_tx_xfer_if>      m_export;
  sc_port<xtsc::xtsc_tx_xfer_if>        m_port;
      
  // constructor / destructor
  xtsc_tx_xfer_if_sd_mon(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_tx_xfer_if_sd_mon();

  // overloaded sc_mx_import_module methods
  string getName();
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);
  MXDI* getMxDI();


  MxdiReturn_t MxdiRegGetGroups(MxU32           groupIndex,
                                MxU32           desiredNumOfRegGroups,
                                MxU32          *actualNumOfRegGroups,
                                MxdiRegGroup_t *reg);


  MxdiReturn_t MxdiRegGetMap(MxU32              groupID,
                             MxU32              regIndex,
                             MxU32              registerSlots,
                             MxU32             *registerCount,
                             MxdiRegInfo_t     *reg);


  MxdiReturn_t MxdiRegRead(MxU32 regCount, MxdiReg_t *reg, MxU32 *numRegsRead, MxU8 doSideEffects);



protected:

  /// Implementation of xtsc_tx_xfer_if.
  class xtsc_tx_xfer_if_impl : public xtsc::xtsc_tx_xfer_if, public sc_object {
  public:

    /// Constructor
    xtsc_tx_xfer_if_impl(const char *object_name, xtsc_tx_xfer_if_sd_mon& monitor) :
      sc_object (object_name),
      m_monitor (monitor),
      m_p_port  (NULL)
    {}

    /// @see xtsc::xtsc_tx_xfer_if
    virtual void nb_tx_xfer(xtsc::xtsc_tx_xfer& tx_xfer);

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_tx_xfer_if_sd_mon&     m_monitor;      ///< Our xtsc_tx_xfer_if_sd_mon object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };



private:

  xtsc_tx_xfer_if_impl                  m_tx_xfer_impl;                 ///<  m_export binds to these

  bool                                  m_init_complete; 
  bool                                  m_reset_called;                 ///<  True if reset() has been called

  // Register related info
  MxdiRegInfo_t                        *m_RegInfo;
  MxdiRegGroup_t                       *m_RegGroup;

  xtsc::u64                             m_num_tx_xfers;                 ///<  Total TX XFER's
  bool                                  m_done;                         ///<  m_data contains Done pin value 
  xtsc::u32                             m_address;                      ///<  The word address  (byte address = m_address*4)
  xtsc::u32                             m_data;                         ///<  The data
  bool                                  m_config_xfer;                  ///<  True if configuration transaction, false if regular
  bool                                  m_write;                        ///<  True if write transaction, false if read transaction
  bool                                  m_read_data;                    ///<  Set to true by the TX core targeted by a read transaction
  bool                                  m_turbo;                        ///<  Use fast-access (peek/poke)
  xtsc::u64                             m_tag;                          ///<  Unique tag per XFER transaction (artificial)

  log4xtensa::TextLogger&               m_text;                         ///<  Text logger
};


};  // namespace xtsc_sd 

#endif  // _XTSC_TX_XFER_IF_SD_MON_H_

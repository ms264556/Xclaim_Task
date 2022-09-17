#ifndef _XTSC_WIRE_READ_IF_SD_MON_H_
#define _XTSC_WIRE_READ_IF_SD_MON_H_

// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
#include <xtsc/xtsc_wire_read_if.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


namespace xtsc_sd {



class XTSC_SD_API xtsc_wire_read_if_sd_mon : public sc_mx_import_module, public MxDIBase {
public:
  sc_export<xtsc::xtsc_wire_read_if>            m_wire_read_export;
  sc_port<xtsc::xtsc_wire_read_if>              m_wire_read_port;
      
  SC_HAS_PROCESS(xtsc_wire_read_if_sd_mon);

  // constructor / destructor
  xtsc_wire_read_if_sd_mon(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_wire_read_if_sd_mon();

  // Thread to check the interface bit width
  void check_thread();

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

  /// Implementation of xtsc_wire_read_if.
  class xtsc_wire_read_if_impl : public xtsc::xtsc_wire_read_if, public sc_object {
  public:

    /// Constructor
    xtsc_wire_read_if_impl(const char *object_name, xtsc_wire_read_if_sd_mon& monitor) :
      sc_object (object_name),
      m_monitor (monitor),
      m_p_port  (NULL)
    {}

    /// @see xtsc::xtsc_wire_read_if
    sc_dt::sc_unsigned nb_read();

    /// @see xtsc::xtsc_wire_read_if
    xtsc::u32 nb_get_bit_width() { return m_monitor.m_width1; }


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_wire_read_if_sd_mon&   m_monitor;      ///< Our xtsc_wire_read_if_sd_mon object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };



private:

  xtsc_wire_read_if_impl                m_wire_read_impl;               ///< m_wire_read_export binds to these

  bool                                  m_init_complete;                ///< The init() method has been called.
  bool                                  m_reset_called;                 ///< True if reset() has been called
  unsigned int                          m_width1;                       ///< Bit width of wire
  unsigned int                          m_chunk_byte_width;             ///< Chunkify wire value into MxDI registers of this size
  unsigned int                          m_chunk_bit_width;              ///< Chunkify wire value into MxDI registers of this size

  // Register related info
  MxdiRegInfo_t                        *m_RegInfo;                      ///< MxDI register information
  MxdiRegGroup_t                       *m_RegGroup;                     ///< MxDI register group information
  xtsc::u32                             m_first_value_reg_index;        ///< Index of first value MxDI register
  xtsc::u32                             m_last_value_reg_index;         ///< Index of last value MxDI register
  xtsc::u32                             m_reg_count;                    ///< Total number of MxDI registers

  xtsc::u64                             m_num_nb_read;                  ///< Total calls to nb_read 

  sc_unsigned                          *m_p_value;                      ///< Last wire value read

  log4xtensa::TextLogger&               m_text;                         ///<  Text logger
};


};  // namespace xtsc_sd 

#endif  // _XTSC_WIRE_READ_IF_SD_MON_H_

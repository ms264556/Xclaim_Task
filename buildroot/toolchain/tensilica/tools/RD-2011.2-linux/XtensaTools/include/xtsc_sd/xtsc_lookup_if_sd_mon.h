#ifndef _XTSC_LOOKUP_IF_SD_MON_H_
#define _XTSC_LOOKUP_IF_SD_MON_H_

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
#include <xtsc/xtsc_lookup_if.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


namespace xtsc_sd {



class XTSC_SD_API xtsc_lookup_if_sd_mon : public sc_mx_import_module, public MxDIBase, public MxPI {
public:
  sc_export<xtsc::xtsc_lookup_if>       m_lookup_export;
  sc_port<xtsc::xtsc_lookup_if>         m_lookup_port;
      
  SC_HAS_PROCESS(xtsc_lookup_if_sd_mon);

  // constructor / destructor
  xtsc_lookup_if_sd_mon(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_lookup_if_sd_mon();

  // Thread to count notifications of the ready event; obtained from default_event()
  void ready_event_thread();

  // MxPI
  MxpiReturn_t MxPIGetProfilingStreams(MxU32 desiredNrStreams, MxU32 *actualNrStreams, MxPIStreamInfo_t *streams);

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

  /// Implementation of xtsc_lookup_if.
  class xtsc_lookup_if_impl : public xtsc::xtsc_lookup_if, public sc_object {
  public:

    /// Constructor
    xtsc_lookup_if_impl(const char *object_name, xtsc_lookup_if_sd_mon& monitor) :
      sc_object (object_name),
      m_monitor (monitor),
      m_p_port  (NULL)
    {}

    /// @see xtsc::xtsc_lookup_if
    void nb_send_address(const sc_dt::sc_unsigned& address);

    /// @see xtsc::xtsc_lookup_if
    bool nb_is_ready();

    /// @see xtsc::xtsc_lookup_if
    sc_dt::sc_unsigned nb_get_data();

    /// @see xtsc::xtsc_lookup_if
    xtsc::u32 nb_get_address_bit_width() { return m_monitor.m_address_bit_width; }

    /// @see xtsc::xtsc_lookup_if
    xtsc::u32 nb_get_data_bit_width() { return m_monitor.m_data_bit_width; }

    /**
     * Get the event that will be notified when the lookup data is available.
     *
     * @see xtsc::xtsc_lookup_if::default_event()
     */
    virtual const sc_event& default_event() const;


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_lookup_if_sd_mon&      m_monitor;      ///< Our xtsc_lookup_if_sd_mon object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };



private:

  xtsc_lookup_if_impl                   m_lookup_impl;                  ///< m_lookup_export binds to these

  bool                                  m_init_complete;                ///< The init() method has been called.
  bool                                  m_reset_called;                 ///< True if reset() has been called
  unsigned int                          m_address_bit_width;            ///< Bit width of lookup address
  unsigned int                          m_data_bit_width;               ///< Bit width of lookup data
  bool                                  m_ram;                          ///< True if lookup is functioning as a RAM

  // The following only apply if m_ram is true {
  unsigned int                          m_write_data_lsb;               ///< Starting bit pos in the lookup address of the write data
  unsigned int                          m_write_strobe_bit;             ///< Location of write stobe in the TIE address
  unsigned int                          m_ram_address_bit_width;        ///< Bit width of RAM address
  unsigned int                          m_ram_address_lsb;              ///< Starting bit pos in the lookup address of the RAM address
  unsigned int                          m_ram_address_msb;              ///< Ending bit pos in the lookup address of the RAM address
  unsigned int                          m_write_data_msb;               ///< Ending bit pos in the lookup address of the write data
  xtsc::u32                             m_first_ram_address_reg_index;  ///< Index of first RAM address MxDI register
  xtsc::u32                             m_last_ram_address_reg_index;   ///< Index of last RAM address MxDI register
  xtsc::u32                             m_first_write_data_reg_index;   ///< Index of first write data MxDI register
  xtsc::u32                             m_last_write_data_reg_index;    ///< Index of last write data MxDI register
  xtsc::u32                             m_write_strobe_reg_index;       ///< Index of write strobe MxDI register
  // The above only apply if m_ram is true }

  unsigned int                          m_chunk_byte_width;             ///< Chunkify address/data into MxDI registers of this size
  unsigned int                          m_chunk_bit_width;              ///< Chunkify address/data into MxDI registers of this size

  // Register related info
  MxdiRegInfo_t                        *m_RegInfo;                      ///< MxDI register information
  MxdiRegGroup_t                       *m_RegGroup;                     ///< MxDI register group information
  xtsc::u32                             m_first_address_reg_index;      ///< Index of first address MxDI register
  xtsc::u32                             m_last_address_reg_index;       ///< Index of last address MxDI register
  xtsc::u32                             m_first_data_reg_index;         ///< Index of first data MxDI register
  xtsc::u32                             m_last_data_reg_index;          ///< Index of last data MxDI register
  xtsc::u32                             m_reg_count;                    ///< Total number of MxDI registers

  xtsc::u64                             m_num_nb_is_ready_true;         ///< Total calls to nb_is_ready that returned true
  xtsc::u64                             m_num_nb_is_ready_false;        ///< Total calls to nb_is_ready that returned false
  xtsc::u64                             m_num_nb_send_address;          ///< Total calls to nb_send_address
  xtsc::u64                             m_num_nb_get_data;              ///< Total calls to nb_get_data
  xtsc::u64                             m_num_ready_events;             ///< Total times the ready event has been notified

  sc_unsigned                          *m_p_address;                    ///< Last lookup address sent
  sc_unsigned                          *m_p_data;                       ///< Last lookup data returned

  // MxPI
  MxPIChannelSymbolInfo_t               m_bool_symbols;                 ///< Symbols for true and false 
  MxPIStreamInfo_t                      m_nb_is_ready_stream_info;      ///< MxPI stream info
  MxPIStream_t                         *m_p_nb_is_ready_stream;
  MxPIChannel_t                        *m_nb_is_ready_channel_ptr_tab[1];
  MxPIChannel_t                         m_nb_is_ready_return_channel;

  log4xtensa::TextLogger&               m_text;                         ///<  Text logger
};


};  // namespace xtsc_sd 

#endif  // _XTSC_LOOKUP_IF_SD_MON_H_

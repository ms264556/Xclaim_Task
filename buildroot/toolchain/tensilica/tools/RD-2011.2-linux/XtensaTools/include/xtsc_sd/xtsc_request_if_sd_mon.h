#ifndef _XTSC_REQUEST_IF_SD_MON_H_
#define _XTSC_REQUEST_IF_SD_MON_H_

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
#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_request.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }


namespace xtsc_sd {



class XTSC_SD_API xtsc_request_if_sd_mon : public sc_mx_import_module, public MxDIBase, public MxPI {
public:
  sc_export<xtsc::xtsc_request_if>      m_request_export;
  sc_port<xtsc::xtsc_request_if>        m_request_port;
      
  // constructor / destructor
  xtsc_request_if_sd_mon(sc_mx_m_base* c, const sc_module_name &module_name);
  virtual ~xtsc_request_if_sd_mon();

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


  MxdiReturn_t MxdiMemGetSpaces(MxU32                   spaceIndex,
                                MxU32                   memSpaceSlots,
                                MxU32                  *memSpaceCount,
                                MxdiMemSpaceInfo_t     *memSpace);


  MxdiReturn_t MxdiMemGetBlocks(MxU32                   memorySpace,
                                MxU32                   blockIndex,
                                MxU32                   memBlockSlots,
                                MxU32                  *memBlockCount,
                                MxdiMemBlockInfo_t     *memBlock);


  MxdiReturn_t MxdiMemWrite(MxdiAddrComplete_t  startAddress,
                            MxU32               unitsToWrite,
                            MxU32               unitSizeInBytes,
                            const MxU8         *data,
                            MxU32              *actualNumOfUnitsWritten,
                            MxU8                doSideEffects);


  MxdiReturn_t MxdiMemRead(MxdiAddrComplete_t   startAddress,
                           MxU32                unitsToRead,
                           MxU32                unitSizeInBytes,
                           MxU8                *data,
                           MxU32               *actualNumOfUnitsRead,
                           MxU8                 doSideEffects);


protected:

  /// Implementation of xtsc_request_if.
  class xtsc_request_if_impl : virtual public xtsc::xtsc_request_if, public sc_object {
  public:

    /**
     * Constructor.
     * @param   monitor     A reference to the owning xtsc_request_if_sd_mon object.
     */
    xtsc_request_if_impl(const char *object_name, xtsc_request_if_sd_mon& monitor) :
      sc_object         (object_name),
      m_monitor         (monitor),
      m_p_port          (NULL)
    {}

    /// @see xtsc::xtsc_request_if
    void nb_request(const xtsc::xtsc_request& request);

    /// @see xtsc::xtsc_request_if
    void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /// @see xtsc::xtsc_request_if
    void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /// @see xtsc::xtsc_request_if
    bool nb_fast_access(xtsc::xtsc_fast_access_request &request);


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_request_if_sd_mon&     m_monitor;      ///< Our xtsc_request_if_sd_mon object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };



private:

  xtsc_request_if_impl                  m_request_impl;                 ///<  m_request_export binds to these

  bool                                  m_init_complete; 
  bool                                  m_end_of_elaboration; 
  bool                                  m_reset_called;                 ///< True if reset() has been called

  unsigned int                          m_byte_width;                   ///< Byte width of data interface
  unsigned int                          m_start_address8;               ///< Starting byte address of address space
  unsigned int                          m_space_size8;                  ///< Byte size of address space (0 => 4GBytes)

  // Register related info
  MxdiRegInfo_t                        *m_RegInfo;
  MxdiRegGroup_t                       *m_RegGroup;

  xtsc::u64                             m_num_requests;                 ///< Total requests
  xtsc::xtsc_address                    m_address8;                     ///< Byte address 
  xtsc::u32                             m_size8;                        ///< Byte size of each transfer
  xtsc::u8                              m_buffer[xtsc::xtsc_max_bus_width8];  ///< Data for RCW, WRITE, and BLOCK_WRITE
  xtsc::u32                             m_route_id;                     ///< Route ID for arbiters
  xtsc::xtsc_request::type_t            m_type;                         ///< Request type (READ, BLOCK_READ, etc)
  xtsc::u32                             m_num_transfers;                ///< Number of transfers
  xtsc::xtsc_byte_enables               m_byte_enables;                 ///< Byte enables
  xtsc::u8                              m_id;                           ///< Transaction ID
  xtsc::u8                              m_priority;                     ///< Transaction priority
  bool                                  m_last_transfer;                ///< True if last transfer of request
  xtsc::xtsc_address                    m_pc;                           ///< Program counter associated with request (artificial)
  xtsc::u64                             m_tag;                          ///< Unique tag per request-response set (artificial)

  // Memory related info
  MxdiMemSpaceInfo_t                   *m_MemSpaceInfo;
  MxdiMemBlockInfo_t                   *m_MemBlockInfo;

  // MxPI
  MxPIStreamInfo_t                      m_request_stream_info;          ///< MxPI stream info
  MxPIStream_t                         *m_p_request_stream;
  MxPIChannel_t                        *m_request_channel_ptr_tab[3];
  MxPIChannelSymbolInfo_t               m_request_type_symbols;         ///< xtsc_request::type_t symbols
  MxPIChannel_t                         m_request_type_channel;
  MxPIChannel_t                         m_request_address_channel;
  MxPIChannel_t                         m_request_size_channel;


#if MAXSIM_MAJOR_VERSION < 7
  helper                               *m_p_helper;
#endif
  log4xtensa::TextLogger&               m_text;                         ///<  Text logger
};


};  // namespace xtsc_sd 

#endif  // _XTSC_REQUEST_IF_SD_MON_H_

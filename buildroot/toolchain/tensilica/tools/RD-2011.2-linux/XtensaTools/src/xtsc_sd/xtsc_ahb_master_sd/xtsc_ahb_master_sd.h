#ifndef XTSC_AHB_MASTER_SD_H_
#define XTSC_AHB_MASTER_SD_H_

// Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */



#include <string>
#include <vector>
#include <fstream>
#include <xtsc/xtsc.h>
#include <maxsim.h>
#include <AHB_Transaction.h>


#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }




/**
 * A scripted AHB bus master.
 *
 * This module implements a master on an AHB bus that reads an input file
 * ("script_file") to determine when and what bus transactions to send.
 *
 * This module provides a simple means to deliver test transactions to an AHB system.
 *
 * Constructor parameters for an xtsc_ahb_master_sd object.
 *
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------------
  
   "script_file"        char*   The file to read the transactions from.  Each 
                                transaction takes one line in the file.  The supported
                                line formats are:

    delay PEEK  address size 
    delay POKE  address size b0 b1 . . . bN
    delay READ  address size timeout lock trans burst prot
    delay WRITE address size timeout lock trans burst prot b0 b1 . . . bN
    delay request lock
    delay STOP
    WAIT  duration
    WAIT  READY
    SYNC  cycle
    NOTE  message
    INFO  message
                                
                                1.  delay can be any positive integer to mean that many
                                    clock cycles.
                                2.  Integers can appear in decimal or hexadecimal (using
                                    '0x' prefix) format.
                                3.  Each field after READ or WRITE (except timeout)
                                    corresponds to an AHB signal that the master drives.
                                4.  size can be 0|1|2|4|8|16|32|64|128 bytes not to
                                    exceed "byte_width".
                                5.  timeout specfies how many cycles to keep trying to
                                    get a grant.  A value of 0 means to never timeout.
                                6.  lock can be LOCK|UNLOCK.  LOCK means to assert hlock.
                                    UNLOCK means to not assert hlock.  
                                7.  trans can be SEQ|NONSEQ|BUSY|IDLE.
                                8.  burst can be:
                                    SINGLE|INCR|WRAP4|INCR4|WRAP8|INCR8|WRAP16|INCR16
                                9.  prot can be any value from 0 to 15.
                               10.  "b0 b1 . . . bN" specify size bytes (that is,
                                    N=size-1) that will be stored in the Data parameter
                                    passed to the MxSI/CASI writeDbg() method (for POKE
                                    transaction) or the MxSI/CASI write() method (for
                                    WRITE transaction).  Some ARM documentation refers
                                    to the Data parameter as the value parameter.  In
                                    any case, it is a u32 pointer (unsigned int *) that
                                    points to an address in memory which will be taken
                                    to be the base of a byte array (using a u8*).  The
                                    bytes specified by "b0 b1 . . . bN" will be placed
                                    in this byte array in the order indicated by their
                                    index.  That is, if we have "u8 *byte = (u8*)Data;",
                                    then b0 will be placed in byte[0], b1 will be placed
                                    in byte[1], and so on.
                               11.  The "delay request lock" command will cause a call
                                    to requestAccess() after delaying delay cycles. 
                                      request can be ASSERT|DEASSERT.
                                      lock    can be LOCK|UNLOCK.
                                    If lock is not specified, it defaults to the current
                                    value of m_hlock.
                                    The main purposes for this command is to allow the 
                                    hbusreq signal to be deasserted after some
                                    transactions and to allow the hlock signal to be
                                    assert at least 1 clock cycle before the ADDR phase.
                                    The addr parameter in the requestAccess() call will
                                    be set according to the request and lock values:
                                      cmd arg  value     addr bit   value
                                      -------  --------  --------   -----
                                      request  DEASSERT     0         0
                                      request  ASSERT       0         1
                                      lock     UNLOCK       1         0
                                      lock     LOCK         1         1
                               12.  The "delay STOP" command will call sc_stop() after
                                    delaying delay cycles.
                               13.  The "WAIT duration" command can be used to cause a 
                                    wait of the specified duration.  duration can be a
                                    positive integer to mean that many clock cycles.
                               14.  The "WAIT READY" command can be used to cause a 
                                    wait until the HREADY signal is high.
                               15.  The "SYNC time" command can be used to cause a wait
                                    until the specified absolute simulation cycle.
                               16.  The NOTE and INFO commands can be used to cause
                                    the entire line to be logged at NOTE_LOG_LEVEL
                                    or INFO_LOG_LEVEL, respectively.
                               17.  Words are case insensitive.
                               18.  Comments, extra whitespace, blank lines, and lines
                                    between "#if 0" and "#endif" are ignored.  
                                    See xtsc_script_file.

   "byte_width"         u32     Bus read/write data interface width in bytes.

   "wraparound"         bool    Specifies what should happen when the end of file
                                (EOF) is reached on "script_file".  When EOF is reached
                                and "wraparound" is true, "script_file" will be reset
                                to the beginning of file and the script will be processed
                                again.  When EOF is reached and "wraparound" is false, 
                                the xtsc_ahb_master_sd object will cease issuing requests.
                                Default = false.

    \endverbatim
 *
 */
class xtsc_ahb_master_sd : public sc_mx_module {
public:
  MxTransactionMaster                   m_ahb_master_port;      ///<  Connect to AHB


  /**
   * Constructor for an xtsc_ahb_master_sd.
   *
   */
  xtsc_ahb_master_sd(sc_mx_m_base* c, const string &s);


  /// Destructor.
  ~xtsc_ahb_master_sd(void);


  virtual std::string getName() { return "xtsc_ahb_master_sd"; }


  /// Register with MxSI kernel
  void interconnect();


  // Overloaded sc_mx_module methods
  void setParameter(const string &name, const string &value);
  string getProperty(MxPropertyType property);
  void init();
  void terminate();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);


protected:

  /// Called from MxSI kernel
  void communicate();


  /// Called from MxSI kernel
  void update();


  /**
   * Helper method to convert size argument in script file
   * 
   * @param index       Index in m_words[].
   *
   * @param size8       Reference to u32 in which to return the size in bytes.
   *
   * @param p_hsize     Optional pointer to u32 in which to return the hsize value.
   *
   * @returns the AHB_ACCESS_TYPE corresponding to the size.
   */
  AHB_ACCESS_TYPE convert_word_to_size(xtsc::u32 index, xtsc::u32& size8, xtsc::u32 *p_hsize);


  /**
   * Helper method to convert a string in m_words[] to a value array.
   *
   * @param index       Starting index in m_words[].
   *
   * @param size8       Number of words in m_words[] (each word makes one byte)
   *
   * @param value       The value array.
   *
   */
  void convert_words_to_value(xtsc::u32 index, xtsc::u32 size8, MxU32 *value);


  /**
   * Helper method to get the AHB signal values from m_words[].
   */
  void get_ahb_signals();


  bool                                  m_init_complete;
  bool                                  m_reset_called;                         ///<  True if reset() has been called

  std::string                           m_script_file;                          ///<  The name of the script file
  xtsc::u32                             m_byte_width;                           ///<  The byte width of the bus' data interface
  bool                                  m_wraparound;                           ///<  Should script file wraparound at EOF

  log4xtensa::TextLogger&               m_text;                                 ///<  TextLogger
  xtsc::xtsc_script_file               *m_p_request_stream;                     ///<  Pointer to the script file 
  std::string                           m_line;                                 ///<  The current script file line
  xtsc::u32                             m_line_count;                           ///<  The current script file line number
  std::vector<std::string>              m_words;                                ///<  Tokenized words from m_line
  xtsc::u64                             m_cycle;                                ///<  Current cycle count
  xtsc::u64                             m_waiting_cycle;                        ///<  Cycle that we're waiting for
  MxU32                                 m_ctrl_peek_poke[AHB_IDX_END];          ///<  For MxSI API's peek/poke
  MxU32                                 m_ctrl_ap[AHB_IDX_END];                 ///<  For MxSI API's ap=address phase
  MxU32                                 m_ctrl_dp[AHB_IDX_END];                 ///<  For MxSI API's dp=data phase
  MxU32                                 m_unused_value  [32];                   ///<  For MxSI API's
  MxU32                                 m_read_value    [32];                   ///<  For MxSI API's
  MxU32                                 m_write_value   [32];                   ///<  For MxSI API's
  MxU32                                 m_write_value_dp[32];                   ///<  For MxSI API's
  MxU32                                 m_dbg_value     [32];                   ///<  For MxSI API's
  xtsc::xtsc_address                    m_address;                              ///<  The byte address for address phase
  xtsc::xtsc_address                    m_address_dp;                           ///<  The byte address for data phase
  xtsc::u32                             m_size8;                                ///<  The transaction size for address phase
  xtsc::u32                             m_size8_dp;                             ///<  The transaction size for data phase
  xtsc::u64                             m_timeout;                              ///<  Timeout for READ|WRITE arbitration
  bool                                  m_hlock;                                ///<  set by lock argument, clear by idle cycle
  bool                                  m_write_ap;                             ///<  address phase: true if write, false if read
  bool                                  m_write_dp;                             ///<  data phase: true if write, false if read
  TAHBSignals                          *m_p_signals;                            ///<  Published pointer to common AHB signal struct
  bool                                  m_do_data_phase;                        ///<  This master should read/write the data bus
  bool                                  m_own_addr_bus;                         ///<  This master owns the addr/ctrl bus this cycle
  bool                                  m_own_addr_bus_next;                    ///<  This master will own the addr/ctrl bus next cycle
  bool                                  m_did_addr_phase;                       ///<  We drove ADDR phase this cycle
  bool                                  m_arb_to_addr;                          ///<  Last cycle was ARB state and this cycle is ADDR


  enum {
    NEXT,       ///<  Get next command from m_script_file
    WAIT,       ///<  WAIT or SYNC command
    READY,      ///<  WAIT READY command
    DELAY,      ///<  Doing delay portion of PEEK|POKE|READ|WRITE command
    ARB,        ///<  Arbitration for READ|WRITE
    ADDR,       ///<  Address phase of READ|WRITE command
    DONE        ///<  End of m_script_file and m_wraparound is false
  } m_state;    ///<  Processing state 


  /// Get the text string of m_state
  char *get_state_string();

  /// Get the next vector of words from the script file
  int get_words();

  /// Extract a u32 value (named argument_name) from the word at m_words[index]
  xtsc::u32 get_u32(xtsc::u32 index, const std::string& argument_name);

  /// Extract a double value (named argument_name) from the word at m_words[index]
  double get_double(xtsc::u32 index, const std::string& argument_name);


};




#endif  // XTSC_AHB_MASTER_SD_H_

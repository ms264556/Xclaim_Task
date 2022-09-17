#ifndef XTSC_AHB_MEMORY_SD_H_
#define XTSC_AHB_MEMORY_SD_H_

// Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <map>
#include <xtsc/xtsc.h>
#include <maxsim.h>
#include <AHB_Transaction.h>



#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }




/**
 * An AHB memory.
 *
 * This module implements a maxsim AHB slave memory module.  
 * It can be provided a script file that allows it to function in
 * a test bench capacity by injecting responses (e.g. wait states)
 * in accordance with the script file.
 *
 *
 * Here are the parameters for this module:
 *
 *  \verbatim
   Name                   Type  Description
   ------------------     ----  -------------------------------------------------------

   "byte_width"           u32   Memory data interface width in bytes.  Valid values are
                                4, 8, and 16.
  
   "big_endian"           bool  This specifies the data layout on the AHB bus.  
                                Default = false.

   "start_byte_address"   u32   The starting byte address of this memory in the 4GB
                                address space.
  
   "memory_byte_size"     u32   The byte size of this memory.  0 means the memory
                                occupies all of the 4GB address space at and above
                                "start_byte_address".
  
   "page_byte_size"       u32   The byte size of a page of memory.  In the model,
                                memory is not allocated until it is accessed.  This
                                parameter specifies the allocation size.
                                Default is 16 Kilobytes (1024*16=16384=0x4000).
                                Minimum page size is 16*byte_width.

   "initial_value_file"   char* If not NULL or empty, this names a text file from which
                                to read the initial memory contents as byte values.
                                Default = NULL.
                                The text file format is:

                                ([@<Offset>] <Value>*)*

                                1.  Any number (<Offset> or <Value>) can be in decimal
                                    or hexadecimal (using '0x' prefix) format.
                                2.  @<Offset> is added to "start_byte_address".
                                3.  <Value> cannot exceed 255 (0xFF).
                                4.  If a <Value> entry is not immediately preceeded in
                                    the file by an @<Offset> entry, then its offset is
                                    one greater then the preceeding <Value> entry.
                                5.  If the first <Value> entry in the file is not 
                                    preceeded by an @<Offset> entry, then its offset
                                    is zero.
                                6.  Comments, extra whitespace, and blank lines are
                                    ignored.  See xtsc::xtsc_script_file.

                                Example text file contents:

                                   0x01 0x02 0x3    // First three bytes of the memory,
                                                    // 0x01 is at "start_byte_address"
                                   @0x1000 50       // The byte at offset 0x1000 is 50
                                   51 52            // The byte at offset 0x1001 is 51
                                                    // The byte at offset 0x1002 is 52
                             

   "memory_fill_byte"     u32   The low byte specifies the value used to initialize 
                                memory contents at address locations not initialize
                                from "initial_value_file".
                                Default = 0.

   "read_only"            bool  If true this memory model represents a ROM which
                                does not support write() transactions.  Use
                                "initial_value_file" to initialize the ROM.  The
                                writeDbg() method may also be used to initialize or
                                change the ROM contents.
                                Default = false.

   "script_file"        char*   The file to read responses from.  Normally,
                                xtsc_ahb_memory_sd responds with HREADY high and HRESP
                                equal to OKAY.  This script file allows you to specify
                                other responses.  You do this by specifying either a
                                single address or an address range and the response that
                                goes with that address or address range.  You can also
                                specify the time frame in which that response applies
                                and how many times that response is to be given.
                                Multiple single addresses and multiple address ranges
                                are supported.  Active lines in this file (that is lines
                                that aren't whitespace or comments) fall into one of
                                three categories.
                                  1. Response definition lines
                                  2. Timing control lines
                                  3. Debug/logging lines
                                The supported line formats organized by category are:

    Category 1: Response Definition Line Formats:
        lowAddr response waitstates [limit]
        lowAddr-highAddr response waitstates [limit]
        CLEAR lowAddr 
        CLEAR lowAddr-highAddr 
        CLEAR

    Category 2: Timing Control Line Formats:
        SYNC  cycle
        WAIT  duration
        WAIT  TRANSACTION [count]
        WAIT  HIT [count]
        WAIT  MISS [count]
        delay STOP

    Category 3: Debug/Logging Line Formats:
        DUMP  log_level
        NOTE  message
        INFO  message
                                
                                1.  Integers can appear in decimal or hexadecimal (using
                                    '0x' prefix) format.
                                2.  response can be DONE|WAIT|RETRY|SPLIT|ABORT.
                                3.  When response is WAIT, waitstates specifies how many
                                    wait states to insert.  In this case, waitstates must
                                    be greater then 0.  When response is anything except
                                    WAIT, waitstates is ignored.
                                4.  limit specifies how many times the specified special
                                    response is to be given.  A limit of 0, the default,
                                    means there is no limit.
                                5.  The CLEAR command cause the specified single address
                                    or address range to be removed from the list of
                                    single addresses or address ranges.  CLEAR by itself
                                    clears all single addresses and all address ranges.
                                6.  The "SYNC time" command can be used to cause a wait
                                    until the specified absolute simulation cycle.
                                7.  The "WAIT duration" command can be used to cause a 
                                    wait duration clock cycles.
                                8.  The "WAIT TRANSACTION" command can be used to cause
                                    a wait until count number of requests have been
                                    received.
                                9.  The "WAIT HIT" command can be used to cause
                                    a wait until count number of requests have been
                                    received that match one of the defined single
                                    addresses or address ranges.
                               10.  The "WAIT MISS" command can be used to cause
                                    a wait until count number of requests have been
                                    received that do not match any of the defined single
                                    addresses or address ranges.
                               11.  For the category 2 (timing control) line formats,
                                    count must be greater than 0 and defaults to 1.
                               12.  The "delay STOP" command will call sc_stop() after
                                    delaying delay cycles.
                               13.  delay can be any positive integer to mean that many
                                    clock cycles.
                               14.  The DUMP command will cause a list of all current
                                    single addresses and address ranges that have a
                                    response defined to be logged.  log_level can be
                                    INFO|NOTE.
                               15.  The NOTE and INFO commands can be used to cause
                                    the entire line to be logged at NOTE_LOG_LEVEL
                                    or INFO_LOG_LEVEL, respectively.
                               16.  Words are case insensitive.
                               17.  Comments, extra whitespace, blank lines, and lines
                                    between "#if 0" and "#endif" are ignored.  
                                    See xtsc_script_file.

   "wraparound"         bool    Specifies what should happen when the end of file
                                (EOF) is reached on "script_file".  When EOF is reached
                                and "wraparound" is true, "script_file" will be reset
                                to the beginning of file and the script will be processed
                                again.  When EOF is reached and "wraparound" is false, 
                                the xtsc_ahb_memory_sd object will cease processing the script
                                file itself but response definitions may still remain in
                                effect.
                                Default = false.

    \endverbatim
 *
 */
class xtsc_ahb_memory_sd : public sc_mx_module {
protected: class sc_mx_transaction_if_impl;
public:

  sc_mx_transaction_if_impl            *m_p_ahb_slave_port;             ///<  AHB masters connect to this


  /**
   * Constructor for an xtsc_ahb_memory_sd.
   */
  xtsc_ahb_memory_sd(sc_mx_m_base* c, const string &s);


  // The destructor.
  ~xtsc_ahb_memory_sd(void);


  /// memcpy with optional swizzle (if m_big_endian is true)
  void xmemcpy(void *dst, const void *src, xtsc::u32 size);


  virtual std::string getName() { return "xtsc_ahb_memory_sd"; }


  /// Register with MxSI kernel
  void interconnect();


  // Overloaded sc_mx_module methods
  string getProperty(MxPropertyType property);
  void setParameter(const string &name, const string &value);
  void init();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);
  void terminate();


  /**
   * Non-hardware reads (for example, reads by the debugger).
   */
  void peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer, bool log = false);


  /**
   * Non-hardware writes (for example, writes from the debugger).
   */
  void poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer, bool log = false);


  /**
   * This method dumps the specified number of bytes from the memory.  Each
   * line of output is divided into three columnar sections, each of which is
   * optional.  The first section contains an address.  The second section contains
   * a hex dump of some (possibly all) of the data (two hex nibbles and a space for
   * each byte from the memory).  The third section contains an ASCII dump of the 
   * same data.
   *
   * @param       address8                The starting byte address in memory.
   *                                      
   * @param       size8                   The number of bytes of data to dump.
   *
   * @param       os                      The ostream object to which the data is to be
   *                                      dumped.
   *
   * @param       left_to_right           If true, the data is dumped in the order:
   *                                      memory[0], memory[1], ..., memory[bytes_per_line-1].
   *                                      If false, the data is dumped in the order:
   *                                      memory[bytes_per_line-1], memory[bytes_per_line-2], ..., memory[0].
   *
   * @param       bytes_per_line          The number of bytes to dump on each line of output.
   *                                      If bytes_per_line is 0 then all size8 bytes are dumped 
   *                                      on a single line with no newline at the end.  If 
   *                                      bytes_per_line is non-zero, then all lines of output
   *                                      end in newline.
   *
   * @param       show_address            If true, the first columnar section contains an 
   *                                      address printed as an 8-hex-digit number with a 0x 
   *                                      prefix.  If false, the first columnar section is null
   *                                      and takes no space in the output.
   *
   * @param       show_hex_values         If true, the second (middle) columnar section of 
   *                                      hex data values is printed.  If false, the second
   *                                      columnar section is null and takes no space in the
   *                                      output.
   *
   * @param       do_column_heading       If true, print byte position column headings over 
   *                                      the hex values section.  If false, no column headings
   *                                      are printed.  If show_hex_values is false, then the
   *                                      do_column_heading value is ignored and no column
   *                                      headings are printed.
   *
   * @param       show_ascii_values       If true, the third (last) columnar section of ASCII
   *                                      data values is printed (if an ASCII value is a
   *                                      non-printable character a period is printed).  If 
   *                                      show_ascii_values is false, the third columnar
   *                                      section is null and takes no space in the output.
   *
   * @param       adjust_address          If adjust_address is true and address8 modulo 
   *                                      bytes_per_line is not 0, then offset the
   *                                      printed values on the first line of the hex and 
   *                                      ASCII columnar sections and adjust the printed 
   *                                      address so that the printed address modulo 
   *                                      bytes_per_line is always zero.  Otherwize, do not
   *                                      offset the first printed data values and do not
   *                                      adjust the printed address.
   */
  void byte_dump(xtsc::xtsc_address     address8,
                 xtsc::u32              size8,
                 std::ostream&          os                      = std::cout,
                 bool                   left_to_right           = true,
                 xtsc::u32              bytes_per_line          = 16,
                 bool                   show_address            = true,
                 bool                   show_hex_values         = true,
                 bool                   do_column_heading       = true,
                 bool                   show_ascii_values       = true,
                 bool                   adjust_address          = true);


  /// Dump all addresses and address ranges that are to receive special responses. 
  void dump_addresses(std::ostream& os);


  /// Clear all addresses and address ranges that are to receive special responses. 
  void clear_addresses();


  /**
   * POD class to help keep track of information related to a special address
   * or address range.
   * 
   * m_limit specifies how many times this address or address range should
   * get the special response.  0 means no limit.
   * m_count counts how many times this address or address range has gotten
   * the special response.
   * When m_count reaches m_limit then m_finished should be set to true.
   *
   */
  class address_info {
  public:
    address_info(xtsc::xtsc_address     low_address,
                 xtsc::xtsc_address     high_address,
                 bool                   is_range,
                 AHB_ACK_TYPE           response,
                 xtsc::u32              waitstates,
                 xtsc::u32              limit) :
      m_low_address     (low_address),
      m_high_address    (high_address),
      m_response        (response),
      m_waitstates      ((m_response == AHB_ACK_WAIT) ? waitstates : 0),
      m_limit           (limit),
      m_is_range        (is_range),
      m_count           (0),
      m_finished        (false)
    { }

    void dump(std::ostream& os = std::cout) const;

    /// Increments m_count, adjusts m_finished if required, returns m_finished
    bool used();

    xtsc::xtsc_address          m_low_address;
    xtsc::xtsc_address          m_high_address;
    AHB_ACK_TYPE                m_response;
    xtsc::u32                   m_waitstates;
    xtsc::u32                   m_limit;
    bool                        m_is_range;
    xtsc::u32                   m_count;
    bool                        m_finished;
  };


protected:

  /// Called from MxSI kernel
  void communicate();


  /// Called from MxSI kernel
  void update();


  /// Implementation of sc_mx_transaction_if.
  class sc_mx_transaction_if_impl : public sc_mx_transaction_slave, public sc_object {
  public:

    /**
     * Constructor.
     * @param   port_name   This port's name.
     * @param   memory      A reference to the owning xtsc_ahb_memory_sd object.
     */
    sc_mx_transaction_if_impl(const char *port_name, xtsc_ahb_memory_sd& memory) :
      sc_mx_transaction_slave   (&memory, port_name),
      sc_object                 (port_name),
      m_memory                  (memory),
      m_p_port                  (0)
    {}

    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }

    // Arbitration functions
    virtual MxGrant requestAccess(MxU64 /*addr*/) { return MX_GRANT_OK; }
    virtual MxGrant checkForGrant(MxU64 /*addr*/) { return MX_GRANT_OK; }

    // Synchronous access functions
    virtual MxStatus read    (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus write   (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus readDbg (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus writeDbg(MxU64 addr, MxU32* value, MxU32* ctrl);

    // Asynchronous access functions
    virtual MxStatus readReq (MxU64 addr, MxU32* value, MxU32* ctrl, MxTransactionCallbackIF* callback);

    // Memory map functions
    virtual int getNumRegions() { return 1; }
    virtual void getAddressRegions(MxU64* start, MxU64* size, std::string* name) {
      start[0] = m_memory.m_start_byte_address;
      size[0]  = m_memory.m_memory_byte_size;
      name[0]  = m_memory.name();
    }

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_ahb_memory_sd&         m_memory;       ///< Our xtsc_ahb_memory_sd object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us


  };



  /// Get the page of memory containing address8 (allocate as needed). 
  xtsc::u32 get_page(xtsc::xtsc_address address8);


  /// Get the page of storage corresponding to the specified address
  xtsc::u32 get_page_id(xtsc::xtsc_address address8) const {
    return (address8 - m_start_byte_address) >> m_page_size8_log2;
  }


  /// Get the offset into the page of storage corresponding to the specified address
  xtsc::u32 get_page_offset(xtsc::xtsc_address address8) const {
    return (address8 - m_start_byte_address) & (m_page_byte_size - 1);
  }


  /**
   * Handle special response for this address, if any.
   *
   * This method first searches m_address_map and then searches m_address_range_map.
   *
   * If the address is found and is not flagged as finished (i.e. if m_finished == false)
   * then status and ctrl[AHB_IDX_ACK] are modified as appropriate.
   *
   * @returns true if the request should be applied to memory (read/written).  Otherwise
   * returns false to indicate that memory storage should not be read/written.
   */
  bool do_special_response(xtsc::xtsc_address address8, MxU32* ctrl, MxStatus& status);



  bool                                  m_init_complete;

  xtsc::u32                             m_byte_width;                   ///< The byte width of this memories data interface
  bool                                  m_big_endian;                   ///<  True if data layout on the bus is big endian
  xtsc::xtsc_address                    m_start_byte_address;           ///< The starting byte address of this memory in 4GB space
  xtsc::u32                             m_memory_byte_size;             ///< The byte size of this memory
  xtsc::u32                             m_page_byte_size;               ///< Memory page size for allocation - must be a power of 2
  string                                m_initial_value_file;           ///< Name of file to read initial memory contents from
  xtsc::u8                              m_memory_fill_byte;             ///< Uninitialized memory has this value
  bool                                  m_read_only;                    ///< See "read_only" parameter
  string                                m_script_file;                  ///< The name of the optional script file
  bool                                  m_wraparound;                   ///< Should script file wraparound at EOF

  xtsc::xtsc_address                    m_end_byte_address;             ///< The ending byte address of this memory
  xtsc::u32                             m_num_pages;                    ///< The number of pages in this memory
  xtsc::u8                            **m_page_table;                   ///< The page table for this memory

  log4xtensa::TextLogger&               m_text;                         ///< Text logger

  xtsc::xtsc_script_file               *m_p_request_stream;             ///< Pointer to the optional script file object
  std::string                           m_line;                         ///< The current script file line
  xtsc::u32                             m_line_count;                   ///< The current script file line number
  std::vector<std::string>              m_words;                        ///< Tokenized words from m_line

  xtsc::u32                             m_page_size8_log2;              ///< Log base 2 of memory page size 


  xtsc::u64                             m_cycle;                        ///< Current cycle count
  xtsc::u64                             m_waiting_cycle;                ///< Cycle that we're waiting for
  xtsc::u32                             m_transaction_count;            ///< Count of transactions received (ADDR phase)
  xtsc::u32                             m_hit_count;                    ///< Count of transactons received that matched
  xtsc::u32                             m_miss_count;                   ///< Count of transactons received that did not match
  xtsc::u32                             m_count_limit;                  ///< Limit for transaction|hit|miss count
  xtsc::u32                             m_waitstates;                   ///< Number of remaining wait states
  xtsc::u32                             m_no_waitstate;                 ///< Force a no-wait-state cycle at the end of wait states

  std::map<xtsc::xtsc_address, address_info*> m_address_map;            ///< Map of single addresses
  std::map<xtsc::xtsc_address, address_info*> m_address_range_map;      ///< Map of address ranges

  enum {
    NEXT,       ///<  Get next command from m_script_file
    WAIT_TIME,  ///<  WAIT duration or SYNC cycle command
    WAIT_TRANS, ///<  WAIT transaction command
    WAIT_HIT,   ///<  WAIT hit command
    WAIT_MISS,  ///<  WAIT miss command
    DELAY,      ///<  Doing delay portion of STOP command
    DONE        ///<  End of m_script_file and m_wraparound is false
  } m_state;    ///<  Processing state 


  /// Get the next vector of words from the script file
  int get_words();

  /// Extract a u32 value (named argument_name) from the word at m_words[index]
  xtsc::u32 get_u32(xtsc::u32 index, const std::string& argument_name);

  /// Extract a double value (named argument_name) from the word at m_words[index]
  double get_double(xtsc::u32 index, const std::string& argument_name);

  /**
   * Method to convert an address or address range string into a pair of numeric
   * addresses.  An address range must be specified without any spaces.  For example,
   * 0x80000000-0x8FFFFFFF.
   *
   * @param index               The index of the string in m_words[].
   *
   * @param argument_name       The name of the argument being converted.  This name is
   *                            from the "script_file" line format documentation.
   *
   * @param low_address         The converted low address.
   *
   * @param high_address        If the string is an address range then this is the
   *                            converted high address.  Otherwise, this is equal to
   *                            low_address.
   *
   * @returns true if the string is an address range.
   */
  bool get_addresses(xtsc::u32           index,
                     const std::string&  argument_name,
                     xtsc::xtsc_address& low_address,
                     xtsc::xtsc_address& high_address);



public:

  TAHBSignals                          *m_p_signals;



};


std::ostream& operator<<(std::ostream& os, const xtsc_ahb_memory_sd::address_info& info);


#endif // XTSC_AHB_MEMORY_SD_H_

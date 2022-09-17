#ifndef XTSC_AHB_TRANSLATOR_SD_H_
#define XTSC_AHB_TRANSLATOR_SD_H_

// Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <vector>
#include <string>
#include <xtsc/xtsc.h>
#include <maxsim.h>
#include <AHB_Transaction.h>



#define LOG(msg) { cout << sc_time_stamp() << ": " << msg << endl; }



/**
 * This class represents an address translation entry to be used in forming an address
 * translation table.
 */
class address_translation_entry {
public:

  /**
   * Constructor.
   *
   * @param   start_address   The lowest byte address in the memory range.
   * @param   end_address     The highest byte address in the memory range.
   * @param   delta           The address translation to apply.  This amount
   *                          should be added to the address of each request.
   */
  address_translation_entry(MxU64 start_address, MxU64 end_address, MxU64 delta) :
    m_start_address   (start_address),
    m_end_address     (end_address),
    m_delta           (delta)
  {}

  MxU64   m_start_address;
  MxU64   m_end_address;
  MxU64   m_delta;

  void dump(std::ostream& os) const;

};


std::ostream& operator<<(std::ostream& os, const address_translation_entry& entry);



/**
 * An address translator for an AHB master.
 *
 * This model implements an address translator that can be placed between an AHB master
 * device and an AHB bus to provide address translations as defined in the file named by
 * the "translation_file" parameter.
 *
 * Here are the parameters for this module:
 *
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    -------------------------------------------------------

   "byte_width"         u32     Bus read/write data interface width in bytes.

   "translation_file"   char*   This names a text file which defines the address 
                                translations.  The file format is:
                                  StartAddress EndAddress NewStartAddress

    \endverbatim
 *
 */
class xtsc_ahb_translator_sd : public sc_mx_module {
protected: class sc_mx_transaction_if_impl;
public:

  sc_mx_transaction_if_impl        *m_p_ahb_slave_port; ///<  AHB masters connect to this
  MxTransactionMaster               m_ahb_master_port;  ///<  Connect to AHB


  /**
   * Constructor for an xtsc_ahb_translator_sd.
   */
  xtsc_ahb_translator_sd(sc_mx_m_base* c, const string &s);


  // The destructor.
  ~xtsc_ahb_translator_sd(void);


  virtual std::string getName() { return "xtsc_ahb_translator_sd"; }


  // Overloaded sc_mx_module methods
  string getProperty(MxPropertyType property);
  void setParameter(const string &name, const string &value);
  void init();
  void reset(MxResetLevel level, const MxFileMapIF *filelist);
  void terminate();

protected:

  /// Translate the address
  MxU64 translate(MxU64 addr);


  /// Implementation of sc_mx_transaction_if.
  class sc_mx_transaction_if_impl : public sc_mx_transaction_slave, public sc_object {
  public:

    /**
     * Constructor.
     * @param   port_name   This port's name.
     * @param   translator  A reference to the owning xtsc_ahb_translator_sd object.
     */
    sc_mx_transaction_if_impl(const char *port_name, xtsc_ahb_translator_sd& translator) :
      sc_mx_transaction_slave   (&translator, port_name),
      sc_object                 (port_name),
      m_translator              (translator),
      m_p_port                  (0)
    {}

    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }

    // Arbitration functions
    virtual MxGrant requestAccess(MxU64 addr);
    virtual MxGrant checkForGrant(MxU64 addr);

    // Synchronous access functions
    virtual MxStatus read    (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus write   (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus readDbg (MxU64 addr, MxU32* value, MxU32* ctrl);
    virtual MxStatus writeDbg(MxU64 addr, MxU32* value, MxU32* ctrl);

    // Asynchronous access functions
    virtual MxStatus readReq (MxU64 addr, MxU32* value, MxU32* ctrl, MxTransactionCallbackIF* callback);

    int getNumRegions();
    void getAddressRegions(MxU64* start, MxU64* size, std::string* name);

  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_ahb_translator_sd&     m_translator;   ///< Our xtsc_ahb_translator_sd object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us

  };


  bool                                  m_init_complete;            ///< True after init() has been called

  string                                m_translation_file;         ///< Name of file to read address translations from
  xtsc::u32                             m_byte_width;               ///<  The byte width of the bus' data interface

  xtsc::xtsc_script_file               *m_p_translation_stream;     ///< The address translation file stream
  std::string                           m_line;                     ///< The current address translation file line
  xtsc::u32                             m_line_count;               ///< The current address translation file line number
  std::vector<std::string>              m_words;                    ///< Tokenized words from m_line
  std::vector<address_translation_entry*>
                                        m_translation_table;        ///<  Table of address translations 

  log4xtensa::TextLogger&               m_text;                     ///< Text logger

  /// Get the next vector of words from the address translation file
  int get_words();

  /// Extract a u64 value (named argument_name) from the word at m_words[index]
  xtsc::u64 get_u64(xtsc::u32 index, const std::string& argument_name);

  /// Return true if range1.m_start_address is less than range2.m_start_address
  static bool start_address_less_than(const address_translation_entry* range1, const address_translation_entry* range2);

};


#endif // XTSC_AHB_TRANSLATOR_SD_H_

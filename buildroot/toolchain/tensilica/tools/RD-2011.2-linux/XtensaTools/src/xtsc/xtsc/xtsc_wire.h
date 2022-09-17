#ifndef _XTSC_WIRE_H_
#define _XTSC_WIRE_H_

// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <vector>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_wire_write_if.h>
#include <xtsc/xtsc_wire_read_if.h>



namespace xtsc {
class xtsc_core;
class xtsc_interrupt_distributor;
class xtsc_tx_loader;
}



namespace xtsc_component {

class xtsc_mmio;
class xtsc_wire_logic;
class xtsc_wire_source;


/**
 * Constructor parameters for a xtsc_wire object.
 *
 * This class contains the constructor parameters for a xtsc_wire object.
 * \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------
  
   "bit_width"          u32     Width of wire in bits.

   "initial_value"      char*   Initial value of wire.  Does not apply if
                                "read_file" is specified.
                                Default = "0x0"

   "vcd_handle"         void*   Pointer to SystemC VCD object (sc_trace_file *) or 0 if
                                tracing is not desired.  
                                Default = 0 (NULL).

   "write_file"         char*   Name of file to write values to instead of writing
                                them to the wire.  If the "write_file" parameter
                                is non-null and non-empty, then calls to nb_write()
                                will cause the passed value to be written to the
                                file instead of the wire.  If the file named by
                                the "write_file" parameter value does not exist,
                                it will be created.  If it does exist, it will be
                                overwritten.  If both "write_file" and "read_file"
                                parameters are null or empty, then calls to
                                nb_write() will cause the passed value to be written
                                to the wire.
                                Default = 0 (i.e. function as a normal wire).

   "timestamp"          bool    If true, then each value written to "write_file"
                                will include the SystemC timestamp as an
                                xtsc_script_file comment.  This parameter is ignored
                                unless "write_file" is non-null and non-empty.
                                Default = true.

   "read_file"          char*   Name of file to read values from instead of reading
                                them from the wire.  If the "read_file" parameter
                                is non-null and non-empty, then calls to nb_read()
                                will read their value from the file instead of
                                reading them from the wire.  The file named by the
                                "read_file" parameter must exist.  If both
                                "read_file" and "write_file" parameters are null or
                                empty, then calls to nb_read() will get their value
                                from the wire.  Values in the file can be
                                expressed in decimal or hexadecimal (using leading
                                '0x') format.  Element values must be separated by 
                                white-space.   See xtsc::xtsc_script_file.
                                Default = 0 (i.e. function as a normal wire).

   "wraparound"         bool    Specifies what should happen when the end of file
                                (EOF) is reached on "read_file".  When EOF is
                                reached and "wraparound" is true, "read_file" will
                                be reset to the beginning of file and nb_read() will
                                return the first value from the file.  When EOF is
                                reached and "wraparound" is false, nb_read() will
                                return the last value in "read_file" (or 0 if
                                "read_file" is empty).
                                Default = false.

   @Note  To cause xtsc_wire to function as a normal wire, set both "write_file" and
          "read_file" parameter values to null (the default) or empty and bind at least
          one sc_port<xtsc_wire_write_if> to the xtsc_wire and bind at least one
          sc_port<xtsc_wire_read_if> to the xtsc_wire.

          To cause xtsc_wire to function as an infinite sink of values written to it,
          specify a valid file name for "write_file" and bind at least one
          sc_port<xtsc_wire_write_if> to the xtsc_wire.

          To cause xtsc_wire to function as a source of values read from it, specify a
          valid and existing file name for "read_file" and bind at least one
          sc_port<xtsc_wire_read_if> to the xtsc_wire.

          To cause xtsc_wire to sink from one file and source from another, specify both
          file names and bind at least one sc_port<xtsc_wire_write_if> to the xtsc_wire
          and bind at least one sc_port<xtsc_wire_read_if> to the xtsc_wire.
          
   \endverbatim
 *
 *
 * @see xtsc_wire
 * @see xtsc::xtsc_parms
 */
class XTSC_COMP_API xtsc_wire_parms : public xtsc::xtsc_parms {
public:


  /**
   * Constructor for an xtsc_wire_parms object.
   *
   * @param     width1          Width of the wire in bits.
   *
   * @param     write_file      Name of file to write nb_write() values to instead
   *                            of writing them to the wire.
   *
   * @param     read_file       Name of file to read nb_read() values from instead
   *                            of reading them from the wire.
   *
   * @param     wraparound      Indicates if read_file should wraparound to the
   *                            beginning of the file after the end of file is
   *                            reached.
   */
  xtsc_wire_parms(xtsc::u32     width1          = 1,
                   const char  *write_file      = 0,
                   const char  *read_file       = 0,
                   bool         wraparound      = false)
  {
    init(width1, write_file, read_file, wraparound);
  }


  /**
   * Constructor for an xtsc_wire_parms object based upon an xtsc_core object and a
   * named TIE export state, import wire, or system-level output wire.
   *
   * This constructor will determine width1 by querying the core object and then pass it
   * to the init() method.  If desired, after the xtsc_wire_parms object is constructed,
   * its data members can be changed using the appropriate xtsc::xtsc_parms::set()
   * method before passing it to the xtsc_wire constructor.
   *
   * @param     core            A reference to the xtsc_core object upon which
   *                            to base the xtsc_wire_parms.
   *
   * @param     core_intf_name  The name of the TIE export state or import wire as it
   *                            appears in the user's TIE code or the name of the system
   *                            level output as it appears in the Xtensa
   *                            Microprocessor Data Book.  Only a subset of system
   *                            level outputs is supported.
   *                            See xtsc::xtsc_core::get_output_wire().
   *
   * @param     write_file      Name of file to write nb_write() values to instead
   *                            of writing them to the wire.
   *
   * @param     read_file       Name of file to read nb_read() values from instead
   *                            of reading them from the wire.
   *
   * @param     wraparound      Indicates if read_file should wraparound to the
   *                            beginning of the file after the end of file is
   *                            reached.
   *
   */
  xtsc_wire_parms(const xtsc::xtsc_core&        core,
                  const char                   *core_intf_name,
                  const char                   *write_file      = 0,
                  const char                   *read_file       = 0,
                  bool                          wraparound      = false);


  /**
   * Do initialization common to both constructors.
   */
  void init(xtsc::u32    width1          = 1,
            const char  *write_file      = 0,
            const char  *read_file       = 0,
            bool         wraparound      = false)
  {
    add("bit_width",            width1);
    add("write_file",           write_file);
    add("timestamp",            true);
    add("read_file",            read_file);
    add("wraparound",           wraparound);
    add("initial_value",        "0x0");
    add("vcd_handle",           (void*) 0); 
  }


  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_wire_parms"; }

};








/**
 * A wire implementation that connects using TLM-level ports.
 *
 * Example XTSC wire class that implements xtsc::xtsc_wire_write_if and 
 * xtsc::xtsc_wire_read_if. 
 *
 * To write to this xtsc_wire, bind an sc_port<xtsc_wire_write_if> object
 * to this xtsc_wire.  To read from this xtsc_wire, bind an 
 * sc_port<xtsc_wire_read_if> object to this xtsc_wire.
 *
 * Alternatively, use one of the xtsc_wire::connect() methods.
 *
 * Here is a block diagram of an xtsc_wire as it is used in the wire example:
 * @image html  Example_xtsc_wire.jpg
 * @image latex Example_xtsc_wire.eps "xtsc_wire Example" width=10cm
 *
 * @see xtsc_wire_parms
 * @see xtsc::xtsc_wire_read_if
 * @see xtsc::xtsc_wire_write_if
 */
class XTSC_COMP_API xtsc_wire :
          public sc_core::sc_module,
  virtual public xtsc::xtsc_wire_write_if,
  virtual public xtsc::xtsc_wire_read_if,
          public xtsc::xtsc_resettable
{
public:


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_wire"; }


  /**
   * Constructor for an xtsc_wire.
   *
   * @param     module_name     Name of the xtsc_wire sc_module.
   *
   * @param     wire_parms      The remaining parameters for construction.
   *
   * @see xtsc_wire_parms
   */
  xtsc_wire(sc_core::sc_module_name module_name, const xtsc_wire_parms& wire_parms);


  /// Destructor.
  ~xtsc_wire();


  /**
   * Reset the xtsc_wire.
   */
  void reset(bool hard_reset = false);


  /**
   * Connect to an xtsc_core.
   *
   * This method connects an export state, import wire, or system-level output of an
   * xtsc_core to this xtsc_wire.
   *
   * @param     core            The xtsc_core to connect to.
   *
   * @param     core_intf_name  The export state, import wire, or system-level output of
   *                            the xtsc_core that is to be connected to this xtsc_wire.
   *                            For an export state or an import wire, core_intf_name is
   *                            the name as it appears in the user's TIE code (it must
   *                            NOT begin with the "TIE_" prefix).  For a system-level
   *                            output, core_intf_name is the name as it appears in the
   *                            Xtensa microprocessor data book.
   *                            See xtsc::xtsc_core::get_output_wire().
   */
  void connect(xtsc::xtsc_core& core, const char *core_intf_name);


  /**
   * Connect to an xtsc_wire_source.
   *
   * This method connects an xtsc_wire_source to this xtsc_wire.
   *
   * @param     source          The xtsc_wire_source to connect to.
   *
   * @param     output_name     The output of the xtsc_wire_source to be connected to
   *                            this xtsc_wire.  If NULL then the default (first/only)
   *                            output of source will be connected.
   */
  void connect(xtsc_wire_source& source, const char *output_name = NULL);


  /**
   * Connect to an xtsc_mmio.
   *
   * This method connects the named output of the xtsc_mmio to this xtsc_wire.
   *
   * @param     mmio            The xtsc_mmio to connect to.
   *
   * @param     output_name     The output of the xtsc_mmio to be connected to this
   *                            xtsc_wire.
   *
   */
  void connect(xtsc_mmio& mmio, const char *output_name);


  /**
   * Connect to an xtsc_wire_logic.
   *
   * This method connects the named output of the xtsc_wire_logic to this xtsc_wire.
   *
   * @param     logic           The xtsc_wire_logic to connect to.
   *
   * @param     output_name     The output of the xtsc_wire_logic to be connected to
   *                            this xtsc_wire.
   */
  void connect(xtsc_wire_logic& logic, const char *output_name);


  /**
   * Connect to an xtsc_tx_loader.
   *
   * This method connects the named output of the xtsc_tx_loader to this xtsc_wire.
   *
   * @param     loader          The xtsc_tx_loader to connect to.
   *
   * @param     output_name     The output of the xtsc_tx_loader to be connected to
   *                            this xtsc_wire.  Valid output names are Mode and Done.
   */
  void connect(xtsc::xtsc_tx_loader& loader, const char *output_name);


  /**
   * Connect an xtsc_interrupt_distributor to this xtsc_wire.
   *
   * This method connects the specified output of the specified
   * xtsc_interrupt_distributor to this xtsc_wire.
   *
   * @param     distributor     The xtsc_interrupt_distributor to connect to.
   *
   * @param     output_name     The output of the xtsc_interrupt_distributor to be
   *                            connected to this xtsc_wire.
   */
  void connect(xtsc::xtsc_interrupt_distributor& distributor, const char *output_name);


  /**
   * This method is used to write a value to this xtsc_wire.
   * @see xtsc::xtsc_wire_write_if
   */
  void nb_write(const sc_dt::sc_unsigned& value);


  /**
   * This method is used to read a value from this xtsc_wire.
   * @see xtsc::xtsc_wire_read_if
   */
  sc_dt::sc_unsigned nb_read();


  /**
   * Returns the bit width of this wire implementation.
   * @see xtsc::xtsc_wire_read_if
   * @see xtsc::xtsc_wire_write_if
   */
  xtsc::u32 nb_get_bit_width() { return m_width1; }


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


protected:

  /// SystemC callback when something binds to us
  virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

  /// Set m_has_read_file_value and m_read_file_value
  void get_next_read_file_value();


  bool                                  m_use_wire;                ///<  Function as a normal wire or use read/write files

  xtsc::u32                             m_width1;                  ///<  Bit width of wire
  xtsc::u32                             m_width8;                  ///<  Byte width of wire
  sc_dt::sc_unsigned                   *m_p_wire;                  ///<  The wire
  sc_dt::sc_unsigned                    m_initial_value;           ///<  Initial value of the wire
  sc_dt::sc_unsigned                    m_value;                   ///<  Latest value written to or read from the wire
  sc_core::sc_trace_file               *m_p_trace_file;            ///<  Pointer to sc_trace_file or NULL if not tracing
  log4xtensa::TextLogger&               m_text;                    ///<  TextLogger
  log4xtensa::BinaryLogger&             m_binary;                  ///<  BinaryLogger
  bool                                  m_log_data_binary;         ///<  True if transaction data should be logged by m_binary
  std::string                           m_write_file_name;         ///<  Name of file to write values to
  std::string                           m_read_file_name;          ///<  Name of file to read values from
  std::ofstream                        *m_write_file;              ///<  File to write values to
  bool                                  m_timestamp;               ///<  From "timestamp" parameter
  xtsc::xtsc_script_file               *m_read_file;               ///<  File to read values from
  bool                                  m_wraparound;              ///<  Should "read_file" wraparound
  bool                                  m_has_read_file_value;     ///<  Does "read_file" have more values
  sc_dt::sc_unsigned                    m_read_file_value;         ///<  Current value from "read_file"
  std::vector<std::string>              m_words;                   ///<  Tokenized words from current line of "read_file"
  std::string                           m_line;                    ///<  Current line from "read_file"
  xtsc::u32                             m_next_word_index;         ///<  Index of current value word in m_words vector
  xtsc::u32                             m_read_file_line_number;   ///<  Current line of "read_file"

};



}  // namespace xtsc_component

#endif  // _XTSC_WIRE_H_

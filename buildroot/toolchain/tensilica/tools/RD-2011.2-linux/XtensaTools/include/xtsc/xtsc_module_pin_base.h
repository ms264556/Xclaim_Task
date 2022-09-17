#ifndef _XTSC_MODULE_PIN_BASE_H_
#define _XTSC_MODULE_PIN_BASE_H_

// Copyright (c) 2007-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

/**
 * @file
 */


#include <map>
#include <set>
#include <vector>
#include <string>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_response.h>





namespace xtsc_component {




/**
 * This is a base class for modules implementing pin-level interfaces, especially
 * pin-level memory interfaces.
 *
 * This class is responsible for containing and managing the sc_in<> and sc_out<>
 * ports of the derived class (the derived class should also inherit from sc_module).
 *
 * This class supports multi-ported memories.  A single-ported memory is said to have 1
 * port set, a dual-ported memory is said to have 2 port sets, etc.  The number of port
 * sets is specified using the num_sets parameter of the xtsc_module_pin_base
 * constructor.
 *
 * Three types of sc_in<> ports and three types of sc_out<> ports are supported:
 *  \verbatim
    Type                  Alias         Usage
    --------------------  -----------   ----------------------------------------------
    sc_in<bool>           bool_input    A scalar input (1 bit).
    sc_in<sc_uint_base>   uint_input    A vector input with 2-64 bits.
    sc_in<sc_bv_base>     wide_input    A vector input of arbitrary width.
    sc_out<bool>          bool_output   A scalar output (1 bit).
    sc_out<sc_uint_base>  uint_output   A vector output with 2-64 bits.
    sc_out<sc_bv_base>    wide_output   A vector output of arbitrary width.
    \endverbatim
 *
 * Ports are created using one of the 6 methods corresponding to the type of port
 * desired: add_bool_input(), add_uint_input(), add_wide_input(), add_bool_output(),
 * add_uint_output(), and add_wide_output().
 *
 * Once created, a reference to a port can be obtained using one of the 6 methods:
 * get_bool_input(), get_uint_input(), get_wide_input(), get_bool_output(),
 * get_uint_output(), and get_wide_output().
 *
 * @see xtsc_tlm2pin_memory_transactor
 * @see xtsc_pin2tlm_memory_transactor
 * @see xtsc_memory_pin
 */
class XTSC_COMP_API xtsc_module_pin_base {
public:

  // See the get_XXXX_input() and get_XXXX_output() methods

  // Shorthand aliases
  typedef std::map<std::string, xtsc::u32>              map_string_u32;
  typedef sc_core::sc_in <bool>                         bool_input;
  typedef sc_core::sc_in <sc_dt::sc_uint_base>          uint_input;
  typedef sc_core::sc_in <sc_dt::sc_bv_base>            wide_input;
  typedef sc_core::sc_out<bool>                         bool_output;
  typedef sc_core::sc_out<sc_dt::sc_uint_base>          uint_output;
  typedef sc_core::sc_out<sc_dt::sc_bv_base>            wide_output;
  typedef std::set<std::string>                         set_string;
  typedef std::vector<set_string*>                      vector_set_string;
  typedef std::map<std::string, bool_input*>            map_bool_input;
  typedef std::map<std::string, uint_input*>            map_uint_input;
  typedef std::map<std::string, wide_input*>            map_wide_input;
  typedef std::map<std::string, bool_output*>           map_bool_output;
  typedef std::map<std::string, uint_output*>           map_uint_output;
  typedef std::map<std::string, wide_output*>           map_wide_output;


  /// Memory interface type
  typedef enum memory_interface_type {
    DRAM0,
    DRAM1,
    DROM0,
    IRAM0,
    IRAM1,
    IROM0,
    URAM0,
    XLMI0,
    PIF,
  } memory_interface_type;


  /// Helper function to get an upper-case version of the "memory_interface" parameter
  static std::string get_interface_uc(const char *interface_name);


  /// Helper function to get the interface type given a memory_port
  static memory_interface_type get_interface_type(xtsc::xtsc_core::memory_port mem_port);


  /// Helper function to get the interface type given a string
  static memory_interface_type get_interface_type(const std::string& interface_name);


  /// Helper function to get the interface name given the interface_type
  static const char *get_interface_name(memory_interface_type interface_type);


  /// Helper function to get the number of bits (i.e. number of enables) in the byte/word enable port
  static xtsc::u32 get_number_of_enables(memory_interface_type interface_type, xtsc::u32 byte_width);


  /**
   * Constructor for a xtsc_module_pin_base.
   *
   * @param     module_name     Name of the xtsc_module_pin_base sc_module.
   *
   * @param     num_sets        Number of I/O port sets (e.g. a single-ported memory has
   *                            1 set of ports, a dual-ported memory has 2 sets of
   *                            ports, etc.)
   *
   * @param     p_trace_file    Pointer to VCD object (can be NULL to indicate no
   *                            tracing).  If not NULL, then all input and output pins
   *                            will be traced.
   *
   * @param     suffix          This constant suffix will be added to every port name.
   *                            This suffix can be set to the empty string ("") if no
   *                            suffix is desired.
   */
  xtsc_module_pin_base(sc_core::sc_module&      module,
                       xtsc::u32                num_sets,
                       sc_core::sc_trace_file  *p_trace_file,
                       const std::string&       suffix);


  /// Destructor.
  virtual ~xtsc_module_pin_base(void);


  /// Return true if the named input exists
  bool has_input(const std::string& port_name) const;


  /// Return true if the named output exists
  bool has_output(const std::string& port_name) const;


  /// Return true if the named input of type sc_in<bool> exists
  bool has_bool_input(const std::string& port_name) const;


  /// Return true if the named input of type sc_in<sc_uint_base> exists
  bool has_uint_input(const std::string& port_name) const;


  /// Return true if the named input of type sc_in<sc_bv_base> exists
  bool has_wide_input(const std::string& port_name) const;


  /// Return true if the named output of type sc_out<bool> exists
  bool has_bool_output(const std::string& port_name) const;


  /// Return true if the named output of type sc_out<sc_uint_base> exists
  bool has_uint_output(const std::string& port_name) const;


  /// Return true if the named output of type sc_out<sc_bv_base> exists
  bool has_wide_output(const std::string& port_name) const;


  /// Get the set of pin-level input names defined for this xtsc_module_pin_base
  set_string get_input_set(xtsc::u32 set_id = 0) const;


  /// Get the set of pin-level output names defined for this xtsc_module_pin_base
  set_string get_output_set(xtsc::u32 set_id = 0) const;


  /// Get the set of bool_input names belonging to the specified set_id
  set_string get_bool_input_set(xtsc::u32 set_id = 0) const;


  /// Get the set of uint_input names belonging to the specified set_id
  set_string get_uint_input_set(xtsc::u32 set_id = 0) const;


  /// Get the set of wide_input names belonging to the specified set_id
  set_string get_wide_input_set(xtsc::u32 set_id = 0) const;


  /// Get the set of bool_output names belonging to the specified set_id
  set_string get_bool_output_set(xtsc::u32 set_id = 0) const;


  /// Get the set of uint_output names belonging to the specified set_id
  set_string get_uint_output_set(xtsc::u32 set_id = 0) const;


  /// Get the set of wide_output names belonging to the specified set_id
  set_string get_wide_output_set(xtsc::u32 set_id = 0) const;


  /// Return the bit width of the name input/output port
  xtsc::u32 get_bit_width(const std::string& port_name) const;


  /**
   * Return a reference to the sc_in<bool> of the named input.
   *
   * This method is used for port binding. For example, to bind the "PIReqRdy"
   * sc_in<bool> of an xtsc_module_pin_base named pin to an sc_signal<bool>
   * named PIReqRdy:
   * \verbatim
       pin.get_bool_input("PIReqRdy")(PIReqRdy);
     \endverbatim
   *
   */
  bool_input& get_bool_input(const std::string& port_name) const;


  /**
   * Return a reference to the sc_in<sc_uint_base> of the named input.
   *
   * This method is used for port binding. For example, to bind the "PIRespCntl"
   * sc_in<sc_uint_base> of an xtsc_module_pin_base named pin to an
   * sc_signal<sc_uint_base> named PIRespCntl:
   * \verbatim
       pin.get_uint_input("PIRespCntl")(PIRespCntl);
     \endverbatim
   *
   */
  uint_input& get_uint_input(const std::string& port_name) const;


  /**
   * Return a reference to the sc_in<sc_bv_base> of the named input.
   *
   * This method is used for port binding. For example, to bind the "PIRespData"
   * sc_in<sc_bv_base> of an xtsc_module_pin_base named pin to an
   * sc_signal<sc_bv_base> named PIRespData:
   * \verbatim
       pin.get_wide_input("PIRespData")(PIRespData);
     \endverbatim
   *
   */
  wide_input& get_wide_input(const std::string& port_name) const;


  /**
   * Return a reference to the sc_out<bool> of the named output.
   *
   * This method is used for port binding. For example, to bind the "POReqValid"
   * sc_out<bool> of an xtsc_module_pin_base named pin to an sc_signal<bool>
   * named POReqValid:
   * \verbatim
       pin.get_bool_output("POReqValid")(POReqValid);
     \endverbatim
   *
   */
  bool_output& get_bool_output(const std::string& port_name) const;


  /**
   * Return a reference to the sc_out<sc_uint_base> of the named output.
   *
   * This method is used for port binding. For example, to bind the "POReqCntl"
   * sc_out<sc_uint_base> of an xtsc_module_pin_base named pin to an
   * sc_signal<sc_uint_base> named POReqCntl:
   * \verbatim
       pin.get_uint_output("POReqCntl")(POReqCntl);
     \endverbatim
   *
   */
  uint_output& get_uint_output(const std::string& port_name) const;


  /**
   * Return a reference to the sc_out<sc_bv_base> of the named output.
   *
   * This method is used for port binding. For example, to bind the "POReqData"
   * sc_out<sc_bv_base> of an xtsc_module_pin_base named pin to an
   * sc_signal<sc_bv_base> named POReqData:
   * \verbatim
       pin.get_wide_output("POReqData")(POReqData);
     \endverbatim
   *
   */
  wide_output& get_wide_output(const std::string& port_name) const;


  /// Return the number of port sets (memory ports) this device has
  xtsc::u32 get_num_sets() const { return m_num_sets; }


  /**
   * Dump name, type, and bit-width of all ports of the specified set.
   *
   * @param     os              The ostream object to dump to.
   *
   * @param     set_id          The port set to dump or 0xFFFFFFFF, the
   *                            default, to dump all port sets.
   */
  void dump_ports(std::ostream& os, xtsc::u32 set_id = 0xFFFFFFFF);


protected:


  /**
   * Class to manage the bits of POReqCntl/PIReqCntl
   */
  class req_cntl {
  public:

    typedef sc_dt::sc_uint_base         sc_uint_base;
    typedef xtsc::xtsc_request::type_t  type_t;
    typedef xtsc::u32                   u32;
    typedef xtsc::xtsc_request          xtsc_request;
    typedef xtsc::xtsc_exception        xtsc_exception;

    /// Construct a req_cntl given an xtsc_request
    req_cntl(const xtsc_request& request) : m_value(8) { init(request); }

    /// Construct a req_cntl given a u32
    req_cntl(u32 value) : m_value(8) { init(value); }

    /// Construct a req_cntl given a sc_uint_base (from the pin-level signal)
    req_cntl(const sc_uint_base& value) : m_value(8) { m_value = value; }

    /// Initialize a req_cntl given an xtsc_request.  Return the 8-bit ReqCntl value.
    const sc_uint_base& init(const xtsc_request& request) {
      bool   last   = request.get_last_transfer();
      bool   shared = (request.get_coherence() == xtsc_request::SHARED);
      u32    num    = request.get_num_transfers();
      type_t type   = request.get_type();
      m_value = 0;
      m_value |= (last ? m_last_mask : 0);
           if (type==xtsc_request::READ)        m_value |= READ;
      else if (type==xtsc_request::WRITE)       m_value |= WRITE;
      else if (type==xtsc_request::BLOCK_READ)  m_value |= BLOCK_READ;
      else if (type==xtsc_request::BLOCK_WRITE) m_value |= BLOCK_WRITE;
      else if (type==xtsc_request::BURST_READ)  m_value |= BURST_READ;
      else if (type==xtsc_request::BURST_WRITE) m_value |= BURST_WRITE;
      else if (type==xtsc_request::RCW)         m_value |= RCW;
      else if (type==xtsc_request::SNOOP)       m_value |= (shared ? SNOOP_SHARED : SNOOP_EXCLUSIVE);
      else {
        throw xtsc_exception("Unrecognized xtsc_request::type_t in xtsc_module_pin_base::req_cntl::init()");
      }
      if ((type==xtsc_request::BURST_READ) || (type==xtsc_request::BURST_WRITE)) {
        m_value |= ((num - 1) << m_num_transfers_shift);
      }
      else {
        m_value |= ((num==1 ? 0 : num==4 ? 1 : num==8 ? 2 : num==16 ? 3 : 0) << m_num_transfers_shift);
      }
      return m_value;
    }

    /// Initialize a req_cntl given a u32
    const sc_uint_base& init(u32 value) { m_value = value; return m_value; }

    /// Return the 8-bit ReqCntl value.
    const sc_uint_base& get_value() const { return m_value; }

    /// Return true if the last transfer bit is set
    bool get_last_transfer() const { return ((m_value.to_uint() & m_last_mask) != 0); }

    /**
     * Return the number of transfers.
     * \verbatim
         READ|WRITE|SNOOP_SHARED|SNOOP_EXCLUSIVE  1
         RCW  2
         BLOCK_READ|BLOCK_WRITE  2|4|8|16
         BURST_READ|BURST_WRITE  2|3|4|5|6|7|8
       \endverbatim
     */
    u32 get_num_transfers() const {
      u32 type = (m_value.to_uint() & m_type_mask);
      switch (type) {
        case READ:
        case WRITE:
        case SNOOP_SHARED:
        case SNOOP_EXCLUSIVE:
          return 1;
        case RCW:
          return 2;
        case BLOCK_READ:
        case BLOCK_WRITE:
          return (2 << ((m_value.to_uint() & m_bl_num_transfers_mask) >> m_num_transfers_shift));
        case BURST_READ:
        case BURST_WRITE:
          return (((m_value.to_uint() & m_bu_num_transfers_mask) >> m_num_transfers_shift) + 1);
        default:
          return 0;
      }
    }

    /// Return the xtsc_request type_t (READ|BLOCK_READ|BURST_READ|RCW|WRITE|BLOCK_WRITE|BURST_WRITE|SNOOP)
    type_t get_request_type() const {
      u32 type = (m_value.to_uint() & m_type_mask);
           if (type == READ)            return xtsc_request::READ;
      else if (type == WRITE)           return xtsc_request::WRITE;
      else if (type == BLOCK_READ)      return xtsc_request::BLOCK_READ;
      else if (type == BLOCK_WRITE)     return xtsc_request::BLOCK_WRITE;
      else if (type == BURST_READ)      return xtsc_request::BURST_READ;
      else if (type == BURST_WRITE)     return xtsc_request::BURST_WRITE;
      else if (type == RCW)             return xtsc_request::RCW;
      else if (type == SNOOP_SHARED)    return xtsc_request::SNOOP;
      else if (type == SNOOP_EXCLUSIVE) return xtsc_request::SNOOP;
      else {
        throw xtsc_exception("Unrecognized request type bits in xtsc_module_pin_base::req_cntl::get_request_type()");
      }
    }

    /// Return the request type as u32
    u32 get_type() const {
      return m_value.to_uint() & m_type_mask;
    }

    /// Dump contents
    void dump(std::ostream& os) const;

    static const u32    READ                    = 0x00;
    static const u32    WRITE                   = 0x80;
    static const u32    BLOCK_READ              = 0x10;
    static const u32    BLOCK_WRITE             = 0x90;
    static const u32    BURST_READ              = 0x30;
    static const u32    BURST_WRITE             = 0xB0;
    static const u32    RCW                     = 0x50;
    static const u32    SNOOP_SHARED            = 0x60;
    static const u32    SNOOP_EXCLUSIVE         = 0x70;

    static const u32    m_type_mask             = 0xF0; // 1111_0000
    static const u32    m_bl_num_transfers_mask = 0x06; // 0000_0110
    static const u32    m_bu_num_transfers_mask = 0x0E; // 0000_1110
    static const u32    m_num_transfers_shift   = 1;
    static const u32    m_last_mask             = 0x01; // 0000_0001

  private:

    sc_uint_base        m_value;

  };
  friend std::ostream& operator<<(std::ostream& os, const req_cntl& req);



  /**
   * Class to manage the bits of PORespCntl/PIRespCntl
   */
  class resp_cntl {
  public:

    typedef sc_dt::sc_uint_base                 sc_uint_base;
    typedef xtsc::xtsc_response::status_t       status_t;
    typedef xtsc::u32                           u32;
    typedef xtsc::xtsc_response                 xtsc_response;
    typedef xtsc::xtsc_exception                xtsc_exception;

    /// Construct a req_cntl given a status and a last transfer
    resp_cntl(u32 status, bool last) : m_value(8), m_snoop(false) { init(status, last); }

    /// Construct a req_cntl given shared, data, and last transfer flags (SNOOP response)
    resp_cntl(bool shared, bool data, bool last) : m_value(8), m_snoop(true) { init(shared, data, last); }

    /// Construct a resp_cntl given a u32
    resp_cntl(u32 value) : m_value(8), m_snoop(false) { init(value); }

    /// Construct a resp_cntl given a sc_uint_base (from the pin-level signal)
    resp_cntl(const sc_uint_base& value) : m_value(8), m_snoop(false) { m_value = value; }

    /// Initialize a req_cntl given a u32
    sc_uint_base init(u32 value) { m_snoop = false; m_value = value; return m_value; }

    /// Initialize a req_cntl given a status and a last transfer
    sc_uint_base init(u32 status, bool last) {
      m_snoop = false;
      m_value = 0;
      m_value |= ((status << m_status_shift) & m_status_mask);
      m_value |= (last ? m_last_mask : 0);
      return m_value;
    }

    /// Initialize a req_cntl given shared, data, and last transfer flags (SNOOP response)
    sc_uint_base init(bool shared, bool data, bool last) {
      m_snoop = true;
      m_value = 0;
      m_value |= (data   ? m_data_mask   : m_no_data_mask);
      m_value |= (shared ? m_shared_mask : 0);
      m_value |= (last   ? m_last_mask   : 0);
      return m_value;
    }

    /// Return the 8-bit ReqCntl value.
    sc_uint_base get_value() const { return m_value; }

    /// Set last transfer bit
    void set_last_transfer(bool last) { m_value[m_last_transfer_bit] = last; }

    /// Return true if the last transfer bit is set
    bool get_last_transfer() const { return ((m_value.to_uint() & m_last_mask) != 0); }

    /// Return the status type (RSP_OK|RSP_ADDRESS_ERROR|RSP_DATA_ERROR|RSP_ADDRESS_DATA_ERROR)
    status_t get_status() const {
      if (m_snoop)              return xtsc_response::RSP_OK;
      u32 status = ((m_value.to_uint() & m_status_mask) >> m_status_shift);
           if (status == OK)    return xtsc_response::RSP_OK;
      else if (status == AErr)  return xtsc_response::RSP_ADDRESS_ERROR;
      else if (status == DErr)  return xtsc_response::RSP_DATA_ERROR;
      else if (status == ADErr) return xtsc_response::RSP_ADDRESS_DATA_ERROR;
      else {
        throw xtsc_exception("Unrecognized status bits in xtsc_module_pin_base::resp_cntl::get_status()");
      }
    }

    /// Dump contents
    void dump(std::ostream& os) const;

    static const u32    OK                      = 0x0;
    static const u32    AErr                    = 0x1;
    static const u32    DErr                    = 0x2;
    static const u32    ADErr                   = 0x3;

    static const u32    m_status_mask           = 0x06;
    static const u32    m_status_shift          = 1;
    static const u32    m_last_mask             = 0x01;
    static const u32    m_last_transfer_bit     = 0;
    static const u32    m_data_mask             = 0x80;
    static const u32    m_no_data_mask          = 0x40;
    static const u32    m_shared_mask           = 0x20;

  private:

    sc_uint_base        m_value;
    bool                m_snoop;

  };
  friend std::ostream& operator<<(std::ostream& os, const resp_cntl& resp);



  /// Confirm set_id is valid
  void confirm_valid_set_id(xtsc::u32 set_id) const;


  /// Confirm port_name is unique and that set_id is valid
  void confirm_unique_name_and_valid_set_id(const std::string& port_name, xtsc::u32 set_id) const;


  /**
   * Add an sc_in<bool> with the specified name.
   *
   * @param     port_name       The base port name.  The full port name is formed by
   *                            concatenating port_name, set_id (if append_id is true),
   *                            and the optional suffix passed into the
   *                            xtsc_module_pin_base constructor.
   *
   * @param     append_id       If true, then set_id will be part of the full port name.
   *                            Otherwise, set_id will not be part of the full port name.
   *
   * @param     set_id          This specifies the set this port belongs.
   */
  bool_input& add_bool_input(const std::string& port_name, bool append_id, xtsc::u32 set_id);


  /**
   * Add an sc_in<sc_uint_base> with the specified name.
   *
   * @param     port_name       The base port name.  The full port name is formed by
   *                            concatenating port_name, set_id (if append_id is true),
   *                            and the optional suffix passed into the
   *                            xtsc_module_pin_base constructor.
   *
   * @param     num_bits        The number of bits.
   *
   * @param     append_id       If true, then set_id will be part of the full port name.
   *                            Otherwise, set_id will not be part of the full port name.
   *
   * @param     set_id          This specifies the set this port belongs.
   */
  uint_input& add_uint_input(const std::string& port_name, xtsc::u32 num_bits, bool append_id, xtsc::u32 set_id);


  /**
   * Add an sc_in<sc_bv_base> with the specified name.
   *
   * @param     port_name       The base port name.  The full port name is formed by
   *                            concatenating port_name, set_id (if append_id is true),
   *                            and the optional suffix passed into the
   *                            xtsc_module_pin_base constructor.
   *
   * @param     num_bits        The number of bits.
   *
   * @param     append_id       If true, then set_id will be part of the full port name.
   *                            Otherwise, set_id will not be part of the full port name.
   *
   * @param     set_id          This specifies the set this port belongs.
   */
  wide_input& add_wide_input(const std::string& port_name, xtsc::u32 num_bits, bool append_id, xtsc::u32 set_id);


  /**
   * Add an sc_out<bool> with the specified name.
   *
   * @param     port_name       The base port name.  The full port name is formed by
   *                            concatenating port_name, set_id (if append_id is true),
   *                            and the optional suffix passed into the
   *                            xtsc_module_pin_base constructor.
   *
   * @param     append_id       If true, then set_id will be part of the full port name.
   *                            Otherwise, set_id will not be part of the full port name.
   *
   * @param     set_id          This specifies the set this port belongs.
   */
  bool_output& add_bool_output(const std::string& port_name, bool append_id, xtsc::u32 set_id);


  /**
   * Add an sc_out<sc_uint_base> with the specified name.
   *
   * @param     port_name       The base port name.  The full port name is formed by
   *                            concatenating port_name, set_id (if append_id is true),
   *                            and the optional suffix passed into the
   *                            xtsc_module_pin_base constructor.
   *
   * @param     num_bits        The number of bits.
   *
   * @param     append_id       If true, then set_id will be part of the full port name.
   *                            Otherwise, set_id will not be part of the full port name.
   *
   * @param     set_id          This specifies the set this port belongs.
   */
  uint_output& add_uint_output(const std::string& port_name, xtsc::u32 num_bits, bool append_id, xtsc::u32 set_id);


  /**
   * Add an sc_out<sc_bv_base> with the specified name.
   *
   * @param     port_name       The base port name.  The full port name is formed by
   *                            concatenating port_name, set_id (if append_id is true),
   *                            and the optional suffix passed into the
   *                            xtsc_module_pin_base constructor.
   *
   * @param     num_bits        The number of bits.
   *
   * @param     append_id       If true, then set_id will be part of the full port name.
   *                            Otherwise, set_id will not be part of the full port name.
   *
   * @param     set_id          This specifies the set this port belongs.
   */
  wide_output& add_wide_output(const std::string& port_name, xtsc::u32 num_bits, bool append_id, xtsc::u32 set_id);




  sc_core::sc_module&           m_sc_module;                    ///< Reference to the sc_module being constructed with these pins
  xtsc::u32                     m_num_sets;                     ///< Number of sets of ports (e.g. number of memory ports, NOT sc_port)
  sc_core::sc_trace_file       *m_p_trace_file;                 ///< Pointer to optional VCD object
  std::string                   m_suffix;                       ///< Suffix to be added to every pin-level input and output name
  map_string_u32                m_map_port_size;                ///< Map of pin-level port name to bit width
  set_string                    m_set_port;                     ///< Set of names of all pin-level inputs and outputs
  set_string                    m_set_input;                    ///< Set of names of all pin-level inputs
  set_string                    m_set_output;                   ///< Set of names of all pin-level outputs

  vector_set_string             m_vector_set_input;             ///< Vector[i] is the set of names of all set #i pin-level inputs
  vector_set_string             m_vector_set_output;            ///< Vector[i] is the set of names of all set #i pin-level outputs
  vector_set_string             m_vector_set_bool_input;        ///< Vector[i] is the set of names of all set #i sc_in<bool>
  vector_set_string             m_vector_set_uint_input;        ///< Vector[i] is the set of names of all set #i sc_in<sc_uint_base>
  vector_set_string             m_vector_set_wide_input;        ///< Vector[i] is the set of names of all set #i sc_in<sc_bv_base>
  vector_set_string             m_vector_set_bool_output;       ///< Vector[i] is the set of names of all set #i sc_in<bool>
  vector_set_string             m_vector_set_uint_output;       ///< Vector[i] is the set of names of all set #i sc_in<sc_uint_base>
  vector_set_string             m_vector_set_wide_output;       ///< Vector[i] is the set of names of all set #i sc_in<sc_bv_base>

  map_bool_input                m_map_bool_input;               ///< The map of all sc_in<bool> inputs
  map_uint_input                m_map_uint_input;               ///< The map of all sc_in<sc_uint_base> inputs
  map_wide_input                m_map_wide_input;               ///< The map of all sc_in<sc_bv_base> inputs
  map_bool_output               m_map_bool_output;              ///< The map of all sc_in<bool> outputs
  map_uint_output               m_map_uint_output;              ///< The map of all sc_in<sc_uint_base> outputs
  map_wide_output               m_map_wide_output;              ///< The map of all sc_in<sc_bv_base> outputs

  static const xtsc::u32        m_id_bits = 6;                  ///< Number of bits in POReqId/PIRespId
  static const xtsc::u32        m_num_ids = 1 << m_id_bits;     ///< Number of possible ID's
  static const xtsc::u32        m_id_mask = m_num_ids - 1;      ///< Mask for POReqId/PIRespId
  static const xtsc::u32        m_vadrs_mask = 0x0003F000;      ///< Mask in virtual address of POReqCohVAdrsIndex
  static const xtsc::u32        m_vaddr_lo_mask = 0x00000FFF;   ///< Mask of low bits in virtual address below VAdrsIndex bits
  static const xtsc::u32        m_vadrs_shift = 12;             ///< Shift of masked virtual address to get POReqCohVAdrsIndex


};  // class xtsc_module_pin_base



}  // namespace xtsc_component



#endif // _XTSC_MODULE_PIN_BASE_H_

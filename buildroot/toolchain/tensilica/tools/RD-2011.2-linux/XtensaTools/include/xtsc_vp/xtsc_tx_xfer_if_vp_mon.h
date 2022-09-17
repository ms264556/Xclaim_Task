#ifndef _XTSC_TX_XFER_IF_VP_MON_H_
#define _XTSC_TX_XFER_IF_VP_MON_H_

// Copyright (c) 2006-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <xtsc_vp/xtsc_vp.h>
#include <xtsc/xtsc_tx_xfer_if.h>
#include <xtsc/xtsc_tx_xfer.h>



namespace xtsc_vp {



/**
 * Parameters for an xtsc_tx_xfer_if_vp_mon.
 *
 * Mainly this is to allow overriding at run time using the extract_parms mechanism.
 */
class xtsc_tx_xfer_if_vp_mon_parms : public xtsc::xtsc_parms {
public:

  xtsc_tx_xfer_if_vp_mon_parms(bool enable_tracing, const char *vcd_name) {
    add("enable_tracing",       enable_tracing);
    add("vcd_name",             vcd_name);
  }

  virtual const char* kind() const { return "xtsc_tx_xfer_if_vp_mon_parms"; }

};



/**
 * This module allows hardware watchpoints and tracing on an XTSC TX XFER interface.
 *
 * This module allows VPA hardware watchpoints to be place on an XTSC TX XFER interface
 * as well as waveform tracing of the interface.  The actual XTSC interface involved is
 * xtsc::xtsc_tx_xfer_if.
 */
class xtsc_tx_xfer_if_vp_mon : public sc_module {
public:

  typedef std::ostringstream            ostringstream;
  typedef xtsc::xtsc_tx_xfer_if         xtsc_tx_xfer_if;
  typedef xtsc::xtsc_exception          xtsc_exception;
  typedef xtsc::xtsc_tx_xfer            xtsc_tx_xfer;
  typedef xtsc::u64                     u64;
  typedef xtsc::u32                     u32;

  sc_export<xtsc_tx_xfer_if>            m_tx_xfer_export;               ///< Receive the XFER from upstream 
  sc_port<xtsc_tx_xfer_if, NSPP>        m_tx_xfer_port;                 ///< Forward the XFER downstream
      
  sc_out<u64>                           m_num_tx_xfers;                 ///< Total calls to nb_tx_xfer
  sc_out<bool>                          m_done;                         ///< m_data contains Done pin value 
  sc_out<u32>                           m_address;                      ///< The word address  (byte address = m_address*4)
  sc_out<u32>                           m_data;                         ///< The data
  sc_out<bool>                          m_config_xfer;                  ///< True if configuration transaction, false if regular
  sc_out<bool>                          m_write;                        ///< True if write transaction, false if read transaction
  sc_out<bool>                          m_read_data;                    ///< Set to true by the TX core targeted by a read transaction
  sc_out<bool>                          m_turbo;                        ///< Use fast-access (peek/poke)
  sc_out<u64>                           m_tag;                          ///< Unique tag per XFER transaction (artificial)


  SC_HAS_PROCESS(xtsc_tx_xfer_if_vp_mon);



#define XTSC_TX_XFER_IF_VP_MON_INITIALIZERS                                             \
    sc_module                     (module_name),                                        \
    m_tx_xfer_export              ("m_tx_xfer_export"),                                 \
    m_tx_xfer_port                ("m_tx_xfer_port"),                                   \
    m_num_tx_xfers                ("m_num_tx_xfers"),                                   \
    m_done                        ("m_done"),                                           \
    m_address                     ("m_address"),                                        \
    m_data                        ("m_data"),                                           \
    m_config_xfer                 ("m_config_xfer"),                                    \
    m_write                       ("m_write"),                                          \
    m_read_data                   ("m_read_data"),                                      \
    m_turbo                       ("m_turbo"),                                          \
    m_tag                         ("m_tag"),                                            \
    m_tx_xfer_impl                ("m_tx_xfer_impl", *this),                            \
    m_enable_tracing              ("/Trace/enable_tracing", false),                     \
    m_vcd_name                    ("/Trace/vcd_name",       ""),                        \
    m_p_trace_file                (0),                                                  \
    m_num_tx_xfers_cnt            (0),                                                  \
    m_connected                   (true),                                               \
    m_text                        (log4xtensa::TextLogger::getInstance(name()))         \



  // ctor for "external" monitor (outside of xtsc_core_vp and visible in Platform Creator)
  xtsc_tx_xfer_if_vp_mon(const sc_module_name &module_name) :
    XTSC_TX_XFER_IF_VP_MON_INITIALIZERS
  {
    string vcd_name = m_vcd_name;
    xtsc_tx_xfer_if_vp_mon_parms parms(m_enable_tracing, vcd_name.c_str());
    ctor_helper(parms, false);
  }



  // ctor for "internal" monitor (inside of xtsc_core_vp and not visible in Platform Creator)
  xtsc_tx_xfer_if_vp_mon(const sc_module_name &module_name, const string& vcd_name, bool connected = true) :
    XTSC_TX_XFER_IF_VP_MON_INITIALIZERS
  {
    xtsc_tx_xfer_if_vp_mon_parms parms((vcd_name != ""), vcd_name.c_str());
    m_connected = connected;
    ctor_helper(parms, true);
  }



  void ctor_helper(xtsc_tx_xfer_if_vp_mon_parms& parms, bool internal) {
    if (m_connected) {
      parms.extract_parms(sc_argc(), sc_argv(), name());
    }
    m_enable_tracing = parms.get_bool("enable_tracing");
    const char *vcd_name = parms.get_c_str("vcd_name");
    m_vcd_name = (vcd_name ? vcd_name : "");

    if (internal) {
      // If inside xtsc_core_vp then we have to cap.  If outside (visible in pct) then pct will cap. 
      m_num_tx_xfers_cap          = new sc_signal<u64> ("m_num_tx_xfers_cap");
      m_done_cap                  = new sc_signal<bool>("m_done_cap");
      m_address_cap               = new sc_signal<u32> ("m_address_cap");
      m_data_cap                  = new sc_signal<u32> ("m_data_cap");
      m_config_xfer_cap           = new sc_signal<bool>("m_config_xfer_cap");
      m_write_cap                 = new sc_signal<bool>("m_write_cap");
      m_read_data_cap             = new sc_signal<bool>("m_read_data_cap");
      m_turbo_cap                 = new sc_signal<bool>("m_turbo_cap");
      m_tag_cap                   = new sc_signal<u64> ("m_tag_cap");

      m_num_tx_xfers            (*m_num_tx_xfers_cap);
      m_done                    (*m_done_cap);
      m_address                 (*m_address_cap);
      m_data                    (*m_data_cap);
      m_config_xfer             (*m_config_xfer_cap);
      m_write                   (*m_write_cap);
      m_read_data               (*m_read_data_cap);
      m_turbo                   (*m_turbo_cap);
      m_tag                     (*m_tag_cap);
    }

    if (m_connected) {
      // Add dummy SCML command to make it visible in VPA
      SCML_COMMAND_PROCESSOR(handle_scml_commands);
      SCML_ADD_COMMAND("dummy",   0, 0, "dummy",            "Dummy command that doesn't do anything.");

      if (m_enable_tracing) {
        m_p_trace_file = xtsc_vp_get_trace_file(m_vcd_name);

        sc_trace(m_p_trace_file, m_num_tx_xfers,                m_num_tx_xfers          .name());
        sc_trace(m_p_trace_file, m_done,                        m_done                  .name());
        sc_trace(m_p_trace_file, m_address,                     m_address               .name());
        sc_trace(m_p_trace_file, m_data,                        m_data                  .name());
        sc_trace(m_p_trace_file, m_config_xfer,                 m_config_xfer           .name());
        sc_trace(m_p_trace_file, m_write,                       m_write                 .name());
        sc_trace(m_p_trace_file, m_read_data,                   m_read_data             .name());
        sc_trace(m_p_trace_file, m_turbo,                       m_turbo                 .name());
        sc_trace(m_p_trace_file, m_tag,                         m_tag                   .name());

      }

      log4xtensa::LogLevel ll = xtsc::xtsc_get_constructor_log_level();
      XTSC_LOG(m_text, ll,        "Constructed xtsc_tx_xfer_if_vp_mon '" << name() << "'" << ":");
      XTSC_LOG(m_text, ll,        " internal                = "   << std::boolalpha << internal);
      XTSC_LOG(m_text, ll,        " connected               = "   << std::boolalpha << m_connected);
      XTSC_LOG(m_text, ll,        " enable_tracing          = "   << std::boolalpha << m_enable_tracing);
      XTSC_LOG(m_text, ll,        " vcd_name                = "   << m_vcd_name);

    }

    m_tx_xfer_export(m_tx_xfer_impl);

  }



  virtual ~xtsc_tx_xfer_if_vp_mon() {
  }



  void end_of_elaboration() {
    m_num_tx_xfers              .write(0);
    m_done                      .write(0);
    m_address                   .write(0);
    m_data                      .write(0);
    m_config_xfer               .write(0);
    m_write                     .write(0);
    m_read_data                 .write(0);
    m_turbo                     .write(0);
    m_tag                       .write(0);
  }



  string handle_scml_commands(const vector<string>& cmd) {
    ostringstream oss;
    if (cmd[0] == "dummy") {
      oss << "dummy command executed";
    }
    return oss.str();
  }



protected:


  /// Implementation of xtsc_tx_xfer_if.
  class xtsc_tx_xfer_if_impl : public xtsc_tx_xfer_if, public sc_object {
  public:

    /// Constructor
    xtsc_tx_xfer_if_impl(const char *object_name, xtsc_tx_xfer_if_vp_mon& monitor) :
      sc_object (object_name),
      m_monitor (monitor),
      m_p_port  (NULL)
    {}

    /// @see xtsc::xtsc_tx_xfer_if
    void nb_tx_xfer(xtsc_tx_xfer& tx_xfer) {
      m_monitor.m_num_tx_xfers  .write(++m_monitor.m_num_tx_xfers_cnt);
      m_monitor.m_done          .write(tx_xfer.get_done());
      m_monitor.m_address       .write(tx_xfer.get_address());
      m_monitor.m_data          .write(tx_xfer.get_data());
      m_monitor.m_config_xfer   .write(tx_xfer.get_config_xfer());
      m_monitor.m_write         .write(tx_xfer.get_write());
      m_monitor.m_read_data     .write(tx_xfer.get_read_data());
      m_monitor.m_turbo         .write(tx_xfer.get_turbo());
      m_monitor.m_tag           .write(tx_xfer.get_tag());

      m_monitor.m_tx_xfer_port->nb_tx_xfer(tx_xfer);
    }


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename) {
      XTSC_INFO(m_monitor.m_text, "In register_port");
      if (m_p_port) {
        ostringstream oss;
        oss << "Illegal multiple binding detected to xtsc_tx_xfer_if_vp_mon '" << m_monitor.name() << "' m_tx_xfer_export" << endl;
        oss << "  " << port.name() << endl;
        throw xtsc_exception(oss.str());
      }
      m_p_port = &port;
    }

    xtsc_tx_xfer_if_vp_mon&     m_monitor;      ///< Our xtsc_tx_xfer_if_vp_mon object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
  };


private:


  xtsc_tx_xfer_if_impl                  m_tx_xfer_impl;                 ///< m_tx_xfer_export binds to these

  scml_property<bool>                   m_enable_tracing;               ///< Trace to VCD file
  scml_property<string>                 m_vcd_name;                     ///< VCD file name
  sc_trace_file                        *m_p_trace_file;                 ///< The VCD file

  u64                                   m_num_tx_xfers_cnt;             ///< To count m_num_tx_xfers

  sc_signal<u64>                       *m_num_tx_xfers_cap;             ///< To cap m_num_tx_xfers
  sc_signal<bool>                      *m_done_cap;                     ///< To cap m_done
  sc_signal<u32>                       *m_address_cap;                  ///< To cap m_address
  sc_signal<u32>                       *m_data_cap;                     ///< To cap m_data
  sc_signal<bool>                      *m_config_xfer_cap;              ///< To cap m_config_xfer
  sc_signal<bool>                      *m_write_cap;                    ///< To cap m_write
  sc_signal<bool>                      *m_read_data_cap;                ///< To cap m_read_data
  sc_signal<bool>                      *m_turbo_cap;                    ///< To cap m_turbo
  sc_signal<u64>                       *m_tag_cap;                      ///< To cap m_tag

  bool                                  m_connected;                    ///< false if internal to xtsc_core_vp and not monitored

  log4xtensa::TextLogger&               m_text;                         ///< Text logger
};


};  // namespace xtsc_vp 


#endif  // _XTSC_TX_XFER_IF_VP_MON_H_


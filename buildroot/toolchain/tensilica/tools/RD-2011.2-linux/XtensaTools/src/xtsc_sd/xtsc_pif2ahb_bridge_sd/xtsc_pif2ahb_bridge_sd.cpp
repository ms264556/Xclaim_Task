// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cctype>
#include <algorithm>
#include <xtsc_sd/xtsc_sd.h>
#include "xtsc_pif2ahb_bridge_sd.h"

using namespace std;
using namespace xtsc;
using namespace xtsc_sd;



xtsc_pif2ahb_bridge_sd::xtsc_pif2ahb_bridge_sd(sc_mx_m_base* c, const sc_module_name &module_name) : 
  sc_mx_import_module   (c, module_name, "xtsc_pif2ahb_bridge_sd"),
  m_pif_request_export  ("m_pif_request_export"),
  m_pif_respond_port    ("m_pif_respond_port"),
  m_write_bus_error     ("m_write_bus_error"),
  m_ahb_master_port     ("m_ahb_master_port"),
  m_request_impl        ("m_request_impl", *this),
  m_text                (log4xtensa::TextLogger::getInstance(getInstanceName())),
  m_zero                (1),
  m_one                 (1)
{
  m_init_complete               = false;
  m_reset_called                = false;
  m_p_signals                   = NULL;
  m_zero                        = 0;
  m_one                         = 1;

  m_pif_byte_width              = 4;
  m_ahb_byte_width              = 0;
  m_big_endian                  = false;
  m_immediate_write_response    = false;
  m_ignore_write_errors         = false;
  m_wrap                        = true;
  m_hprot                       = 0x3;
  m_lock                        = false;
  m_lock_block                  = false;
  m_lock_block_2                = false;
  m_delay_from_receipt          = false;
  m_pif_clock_period            = 0xFFFFFFFF;
  m_recovery_time               = 1;
  m_read_response_delay         = 1;
  m_write_response_delay        = 1;
  m_response_repeat             = 1;
  m_request_fifo_depth          = 2;
  m_response_fifo_depth         = 2;
  m_status                      = xtsc_response::RSP_OK;

  defineParameter("pif_byte_width",             "4",            MX_PARAM_VALUE,  0);
  defineParameter("ahb_byte_width",             "0",            MX_PARAM_VALUE,  0);
  defineParameter("big_endian",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("immediate_write_response",   "false",        MX_PARAM_BOOL,   0);
  defineParameter("ignore_write_errors",        "false",        MX_PARAM_BOOL,   0);
  defineParameter("wrap",                       "true",         MX_PARAM_BOOL,   0);
  defineParameter("hprot",                      "0x3",          MX_PARAM_VALUE,  0);
  defineParameter("lock",                       "false",        MX_PARAM_BOOL,   0);
  defineParameter("lock_block",                 "false",        MX_PARAM_BOOL,   0);
  defineParameter("lock_block_2",               "false",        MX_PARAM_BOOL,   0);
  defineParameter("pif_clock_period",           "0xFFFFFFFF",   MX_PARAM_VALUE,  0);
  defineParameter("delay_from_receipt",         "true",         MX_PARAM_BOOL,   0);
  defineParameter("recovery_time",              "1",            MX_PARAM_VALUE,  0);
  defineParameter("read_response_delay",        "1",            MX_PARAM_VALUE,  0);
  defineParameter("write_response_delay",       "1",            MX_PARAM_VALUE,  0);
  defineParameter("response_repeat",            "1",            MX_PARAM_VALUE,  0);
  defineParameter("request_fifo_depth",         "2",            MX_PARAM_VALUE,  0);
  defineParameter("response_fifo_depth",        "2",            MX_PARAM_VALUE,  0);

  registerSCGenericSlavePort(&m_pif_request_export, "m_pif_request_export");
  registerSCGenericMasterPort(&m_pif_respond_port, "m_pif_respond_port");
  registerSCGenericMasterPort(&m_write_bus_error, "m_write_bus_error");
  registerPort(&m_ahb_master_port, "m_ahb_master_port");

  // Do this so we have a clock slave port
  sc_mx_clocked();
  registerPort(dynamic_cast<sc_mx_clock_slave_p_base*>(this), "clk-in");
}



xtsc_pif2ahb_bridge_sd::~xtsc_pif2ahb_bridge_sd() {
}



// The only valid values for size are 1, 2, and 4*n
void xtsc_pif2ahb_bridge_sd::xmemcpy(void *dst, const void *src, u32 size) {
  if (m_big_endian && (size > 1)) {
    if (size == 2) {
      u8 *d = (u8 *) dst;
      const u8 *s = (const u8 *) src;
      d[0] = s[1];
      d[1] = s[0];
    }
    else {
      if (size & 0x3) {
        stringstream oss;
        oss << "xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "': Invalid size=" << size << " in call to xmemcpy";
        throw xtsc_exception(oss.str());
      }
      u32 *ds = (u32 *) dst;
      const u32 *sr = (const u32 *) src;
      u32 chunks = size/4;
      for (u32 i=0; i<chunks; ++i) {
        u8 *d = (u8 *) &ds[i];
        const u8 *s = (const u8 *) &sr[i];
        for (int j=0, k=3; j<4; ++j, --k) {
          d[k] = s[j];
        }
      }
    }
  }
  else {
    memcpy(dst, src, size);
  }
}



void xtsc_pif2ahb_bridge_sd::init() {
  xtsc_sd_initialize();
  XTSC_DEBUG(m_text, "In xtsc_pif2ahb_bridge_sd::init()");

  if (!m_request_fifo_depth) {
    throw xtsc_exception("xtsc_pif2ahb_bridge_sd: parameter request_fifo_depth cannot be 0");
  }

  if (!m_response_fifo_depth) {
    throw xtsc_exception("xtsc_pif2ahb_bridge_sd: parameter response_fifo_depth cannot be 0");
  }


  if ((m_pif_byte_width != 4) && (m_pif_byte_width != 8) && (m_pif_byte_width != 16)) {
    ostringstream oss;
    oss << "xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "': \"pif_byte_width\"=" << m_pif_byte_width << " is not supported.";
    throw xtsc_exception(oss.str());
  }

  u32 ahb_byte_width = m_ahb_byte_width;
  if (m_ahb_byte_width == 0) {
    m_ahb_byte_width = m_pif_byte_width;
  }

  if (m_ahb_byte_width == 4) {
    m_ahb_native_access_type    = AHB_TYPE_WORD;
  }
  else if (m_ahb_byte_width == 8) {
    m_ahb_native_access_type    = AHB_TYPE_DWORD;
  }
  else if (m_ahb_byte_width == 16) {
    m_ahb_native_access_type    = AHB_TYPE_128BIT;
  }
  else {
    ostringstream oss;
    oss << "xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "': \"ahb_byte_width\"=" << m_ahb_byte_width << " is not supported.";
    throw xtsc_exception(oss.str());
  }

  m_ahb_width8_mask             = m_ahb_byte_width - 1;
  m_max_beat_width8             = ((m_pif_byte_width < m_ahb_byte_width) ? m_pif_byte_width : m_ahb_byte_width);

  // Get clock period 
  if (m_pif_clock_period == 0xFFFFFFFF) {
    m_pif_clock_period_sct = xtsc_get_system_clock_period();
  }
  else {
    m_pif_clock_period_sct = sc_get_time_resolution() * m_pif_clock_period;
  }

  m_read_response_delay_sct     = m_pif_clock_period_sct * m_read_response_delay;
  m_write_response_delay_sct    = m_pif_clock_period_sct * m_write_response_delay;
  m_response_repeat_sct         = m_pif_clock_period_sct * m_response_repeat;
  m_recovery_time_sct           = m_pif_clock_period_sct * m_recovery_time;

  m_pif_request_export(m_request_impl);

  SC_THREAD(response_thread);
  SC_THREAD(write_bus_error_thread);

  // Set transaction properties
  MxTransactionProperties props;
  AHB_INIT_TRANSACTION_PROPERTIES(props);
  props.supportsBurst = true;
  props.dataBitwidth = m_ahb_byte_width*8;
  m_ahb_master_port.setProperties(&props);

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "' (in init() method):");
  XTSC_LOG(m_text, ll,        " pif_byte_width          = "   << m_pif_byte_width);
  XTSC_LOG(m_text, ll,        " ahb_byte_width          = "   << ahb_byte_width << " => " << m_ahb_byte_width);
  XTSC_LOG(m_text, ll,        " big_endian              = "   << boolalpha << m_big_endian);
  XTSC_LOG(m_text, ll,        " immediate_write_response= "   << boolalpha << m_immediate_write_response);
  XTSC_LOG(m_text, ll,        " ignore_write_errors     = "   << boolalpha << m_ignore_write_errors);
  XTSC_LOG(m_text, ll, hex << " hprot                   = 0x" << m_hprot);
  XTSC_LOG(m_text, ll,        " lock                    = "   << boolalpha << m_lock);
  XTSC_LOG(m_text, ll,        " lock_block              = "   << boolalpha << m_lock_block);
  XTSC_LOG(m_text, ll,        " lock_block_2            = "   << boolalpha << m_lock_block_2);
  XTSC_LOG(m_text, ll,        " wrap                    = "   << boolalpha << m_wrap);
  if (m_pif_clock_period == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, hex << " pif_clock_period        = 0x" << m_pif_clock_period << " (" << m_pif_clock_period_sct << ")");
  } else {
  XTSC_LOG(m_text, ll,        " pif_clock_period        = "   << m_pif_clock_period << " (" << m_pif_clock_period_sct << ")");
  }
  XTSC_LOG(m_text, ll,        " delay_from_receipt      = "   << boolalpha << m_delay_from_receipt);
  XTSC_LOG(m_text, ll,        " read_response_delay     = "   << m_read_response_delay);
  XTSC_LOG(m_text, ll,        " write_response_delay    = "   << m_write_response_delay);
  XTSC_LOG(m_text, ll,        " response_repeat         = "   << m_response_repeat);
  XTSC_LOG(m_text, ll,        " recovery_time           = "   << m_recovery_time);
  XTSC_LOG(m_text, ll,        " request_fifo_depth      = "   << m_request_fifo_depth);
  XTSC_LOG(m_text, ll,        " response_fifo_depth     = "   << m_response_fifo_depth);

  // This simplifies logic checks later on
  if (m_lock) {
    m_lock_block = true;
  }
  if (m_lock_block) {
    m_lock_block_2 = true;
  }

  m_p_request_fifo  = new sc_fifo<request_info*>(m_request_fifo_depth);
  m_p_response_fifo = new sc_fifo<response_info*>(m_response_fifo_depth);

  sc_mx_import_module::init();
  m_init_complete = true;
}



void xtsc_pif2ahb_bridge_sd::reset(MxResetLevel level, const MxFileMapIF *filelist) {
  XTSC_INFO(m_text, kind() << "::reset()");

  m_state                       = IDLE;
  m_htrans_next                 = AHB_TRANS_NONSEQ;
  m_hlock                       = 0;
  m_do_hlock                    = false;
  for (u32 i=0; i<AHB_IDX_END; ++i) {
    m_ctrl_default  [i]         = 0;
    m_ctrl_ap       [i]         = 0;
    m_ctrl_dp       [i]         = 0;
    m_ctrl_peek_poke[i]         = 0;
  }
  m_ctrl_default[AHB_IDX_TYPE ] = AHB_TYPE_DWORD;
  m_ctrl_default[AHB_IDX_CYCLE] = AHB_CYCLE_ADDR;
  m_ctrl_ap     [AHB_IDX_CYCLE] = AHB_CYCLE_ADDR;
  m_ctrl_dp     [AHB_IDX_CYCLE] = AHB_CYCLE_DATA;
  m_ctrl_dp     [AHB_IDX_ACK]   = AHB_ACK_DONE;
  m_hreq                        = 0;
  m_arb_to_addr                 = false;
  m_do_data_phase               = false;
  m_did_addr_phase              = false;
  m_did_unlock                  = false;
  m_did_lock_cycle              = false;
  m_own_addr_bus_last           = false;
  m_own_addr_bus_next           = false;
  m_own_addr_bus                = false;
  m_do_unlock                   = false;
  m_hwrite                      = false;
  m_end_of_burst                = true;
  m_early_burst_termination     = false;
  m_haddr                       = 0x00000000;
  m_hsize                       = 0;
  m_is_rcw1                     = false;
  m_addr_phase_was_rcw1         = false;
  m_rcw1_got_rsp_address_error  = false;
  m_rcw1_got_rsp_address_error_z1 = false;
  m_rcw_read_completed          = false;
  m_rcw_read_completed_z1       = false;
  m_rcw_read_data_matched       = false;
  m_p_nascent_response_info     = NULL;
  m_p_nascent_response_buffer   = NULL;
  m_p_ahb_beat_info_ap          = NULL;
  m_p_ahb_beat_info_dp          = NULL;

  m_last_response_time_stamp    = SC_ZERO_TIME -  m_recovery_time_sct;

  if (!m_reset_called) {
    m_reset_called = true;
    // Get the AHB signals
    m_ahb_master_port.readReq(0ULL, NULL, (MxU32*) &m_p_signals, NULL);
    XTSC_INFO(m_text, "In xtsc_pif2ahb_bridge_sd::reset() with m_p_signals=" << m_p_signals);
    if (m_p_signals == NULL) {
      ostringstream oss;
      oss << "xtsc_pif2ahb_bridge_sd '" << getInstanceName()
          << "': m_p_signals is NULL. Perhaps this device is not connected to an AHB bus.";
      throw xtsc_exception(oss.str());
    }
  }
  sc_mx_import_module::reset(level, filelist);
}



void xtsc_pif2ahb_bridge_sd::terminate() {
  sc_mx_import_module::terminate();
  xtsc_finalize();
}



void xtsc_pif2ahb_bridge_sd::setParameter(const string &name, const string &value) {
  MxConvertErrorCodes status = MxConvert_SUCCESS;

  if (m_init_complete) {
    message(MX_MSG_WARNING, "xtsc_pif2ahb_bridge_sd::setParameter: Cannot change parameter <%s>" \
                            " at runtime. Assignment ignored.", name.c_str());
    return;
  }

  if (name == "pif_byte_width") {
    status = MxConvertStringToValue(value, &m_pif_byte_width);
  }
  else if (name == "ahb_byte_width") {
    status = MxConvertStringToValue(value, &m_ahb_byte_width);
  }
  else if (name == "big_endian") {
    status = MxConvertStringToValue(value, &m_big_endian);
  }
  else if (name == "immediate_write_response") {
    status = MxConvertStringToValue(value, &m_immediate_write_response);
  }
  else if (name == "ignore_write_errors") {
    status = MxConvertStringToValue(value, &m_ignore_write_errors);
  }
  else if (name == "wrap") {
    status = MxConvertStringToValue(value, &m_wrap);
  }
  else if (name == "hprot") {
    status = MxConvertStringToValue(value, &m_hprot);
  }
  else if (name == "lock") {
    status = MxConvertStringToValue(value, &m_lock);
  }
  else if (name == "lock_block") {
    status = MxConvertStringToValue(value, &m_lock_block);
  }
  else if (name == "lock_block_2") {
    status = MxConvertStringToValue(value, &m_lock_block_2);
  }
  else if (name == "pif_clock_period") {
    status = MxConvertStringToValue(value, &m_pif_clock_period);
  }
  else if (name == "delay_from_receipt") {
    status = MxConvertStringToValue(value, &m_delay_from_receipt);
  }
  else if (name == "recovery_time") {
    status = MxConvertStringToValue(value, &m_recovery_time);
  }
  else if (name == "read_response_delay") {
    status = MxConvertStringToValue(value, &m_read_response_delay);
  }
  else if (name == "write_response_delay") {
    status = MxConvertStringToValue(value, &m_write_response_delay);
  }
  else if (name == "response_repeat") {
    status = MxConvertStringToValue(value, &m_response_repeat);
  }
  else if (name == "request_fifo_depth") {
    status = MxConvertStringToValue(value, &m_request_fifo_depth);
  }
  else if (name == "response_fifo_depth") {
    status = MxConvertStringToValue(value, &m_response_fifo_depth);
  }


  if (status == MxConvert_SUCCESS) {
    sc_mx_import_module::setParameter(name, value);
  }
  else {
    message(MX_MSG_WARNING, "xtsc_pif2ahb_bridge_sd::setParameter: Illegal value <%s> "
             "passed for parameter <%s>. Assignment ignored.", value.c_str(), name.c_str());
  }
}



string xtsc_pif2ahb_bridge_sd::getProperty(MxPropertyType property) {
  string description;
  switch (property) {    
    case MX_PROP_LOADFILE_EXTENSION:
           return "";
    case MX_PROP_REPORT_FILE_EXT:
           return "yes";
    case MX_PROP_COMPONENT_TYPE:
           return "Other";
    case MX_PROP_COMPONENT_VERSION:
           return "0.2";
    case MX_PROP_MSG_PREPEND_NAME:
           return "yes";
    case MX_PROP_DESCRIPTION:
           description = "xtsc_pif2ahb_bridge_sd";
           return description + " Compiled on " + __DATE__ + ", " + __TIME__;
    case MX_PROP_MXDI_SUPPORT:
           return "no";
    case MX_PROP_SAVE_RESTORE:
           return "no";
    default:
           return "";
  }
}



void xtsc_pif2ahb_bridge_sd::interconnect() {
  getClockMaster()->registerClockSlave(this);
}



void xtsc_pif2ahb_bridge_sd::communicate_helper(ahb_beat_info*&           p_ahb_beat_info,
                                                xtsc_response::status_t   status,
                                                bool                      immediate_write_response)
{
  XTSC_DEBUG(m_text, "enter communicate_helper: ahb_beat_info: " << *p_ahb_beat_info <<
                     " immediate=" << immediate_write_response << " status=" << xtsc_response::get_status_name(status));
  bool error_response = (status != xtsc_response::RSP_OK);
  // For errors, drain and recycle all associated ahb_beat_info objects
  if (error_response) {
    while (!m_ahb_beat_info_deque.empty()) {
      ahb_beat_info *p_next_ahb_beat_info = m_ahb_beat_info_deque.front();
      if (p_next_ahb_beat_info->m_p_request != p_ahb_beat_info->m_p_request) break;
      m_ahb_beat_info_deque.pop_front();
      XTSC_DEBUG(m_text, "Discarding ahb_beat_info: " << *p_next_ahb_beat_info);
      delete_ahb_beat_info(p_next_ahb_beat_info);
    }
    if (m_p_ahb_beat_info_ap) {
      XTSC_DEBUG(m_text, "Discarding ADDR phase ahb_beat_info: " << *m_p_ahb_beat_info_ap);
      delete_ahb_beat_info(m_p_ahb_beat_info_ap);
    }
  }
  // Is there an xtsc_response associated with this beat?
  if (p_ahb_beat_info->m_send_response || error_response || immediate_write_response) {
    bool send_response = true;
    xtsc_request::type_t type = p_ahb_beat_info->m_p_request->get_type();
    if (m_immediate_write_response &&
        !immediate_write_response &&
        ((type == xtsc_request::BLOCK_WRITE) || (type == xtsc_request::WRITE)))
    {
      send_response = false;
      if (error_response) {
        if (m_ignore_write_errors) {
          XTSC_INFO(m_text, "Ignoring AHB error on request: " << *p_ahb_beat_info->m_p_request);
        }
        else {
          ostringstream oss;
          oss << "xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "': AHB error on request: " << *p_ahb_beat_info->m_p_request
              << endl;
          oss << "If you want to ignore AHB errors set \"ignore_write_errors\" to true." << endl;
          oss << "Alternatively, set \"immediate_write_response\" to false." << endl;
          throw xtsc_exception(oss.str());
        }
      }
    }
    if (send_response) {
      if (m_p_nascent_response_info == NULL) {
        m_p_nascent_response_info   = new_response_info(*p_ahb_beat_info->m_p_request);
      }
      m_p_nascent_response_info->m_p_response->set_status(status);
      m_p_nascent_response_info->m_p_response->set_last_transfer(p_ahb_beat_info->m_last_transfer);
      m_p_nascent_response_info->m_p_response->set_id(p_ahb_beat_info->m_p_request->get_id());
      if (!m_p_response_fifo->nb_write(m_p_nascent_response_info)) {
        ostringstream oss;
        oss << "Fatal Error: m_p_response_fifo is full in xtsc_pif2ahb_bridge_sd::communicate()." << endl;
        oss << "You may wish to increase \"response_fifo_depth\".";
        throw xtsc_exception(oss.str());
      }
      m_response_thread_event.notify(SC_ZERO_TIME);
    }
    if (!immediate_write_response) {
      m_p_nascent_response_info = NULL;
    }
  }
  if (!immediate_write_response) {
    // Does this beat mean we are done with an xtsc_request?
    if (p_ahb_beat_info->m_completes_request || error_response) {
      // Get the xtsc_request from the sc_fifo
      request_info *p_request_info = NULL;
      if (!m_p_request_fifo->nb_read(p_request_info)) {
        throw xtsc_exception("Program Bug: m_p_request_fifo is empty in xtsc_pif2ahb_bridge_sd::communicate()");
      }
      // Do a sanity check
      if (&p_request_info->m_request != p_ahb_beat_info->m_p_request) {
        ostringstream oss;
        oss << "Program Bug: xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "': &p_request_info->m_request ("
            << &p_request_info->m_request << ") != p_ahb_beat_info->m_p_request (" << p_ahb_beat_info->m_p_request << ")";
        throw xtsc_exception(oss.str());
      }
      // Recycle the request_info
      delete_request_info(p_request_info);
    }
    // Recycle the ahb_beat_info
    delete_ahb_beat_info(p_ahb_beat_info);
  }
  XTSC_DEBUG(m_text, "exit communicate_helper");
}



void xtsc_pif2ahb_bridge_sd::communicate() {
  m_did_addr_phase    = false;
  m_did_unlock        = false;
  m_did_lock_cycle    = false;
  bool have_grant     = m_own_addr_bus_next;
  m_own_addr_bus      = have_grant && m_p_signals->hready_z1;
  MxGrant grant       = m_ahb_master_port.checkForGrant(0);
  m_own_addr_bus_next = (grant == MX_GRANT_OK);

  MxStatus status;
  XTSC_DEBUG(m_text, "communicate() m_state=" << get_state_string() << " m_do_data_phase=" << boolalpha << m_do_data_phase <<
                     " m_own_addr_bus=" << m_own_addr_bus << " m_own_addr_bus_next=" << m_own_addr_bus_next <<
                     " m_p_request_fifo avail/free: " << m_p_request_fifo->num_available() << "/" << m_p_request_fifo->num_free());
  if (m_do_data_phase) {
    u32 size8 = (1 << m_p_ahb_beat_info_dp->m_hsize);
    u32 rcw_match_data = m_p_ahb_beat_info_dp->m_hdata[0];
    // Create a response if we don't already have one from the previous beat
    if (m_p_nascent_response_info == NULL) {
      m_p_nascent_response_info   = new_response_info(*m_p_ahb_beat_info_dp->m_p_request);
      m_p_nascent_response_buffer = m_p_nascent_response_info->m_p_response->get_buffer();
    }
    // Do the data phase read or write
    if (m_p_ahb_beat_info_dp->m_hwrite) {
      status = m_ahb_master_port.write(m_p_ahb_beat_info_dp->m_haddr, m_p_ahb_beat_info_dp->m_hdata, m_ctrl_dp);
    }
    else {
      status = m_ahb_master_port.read(m_p_ahb_beat_info_dp->m_haddr, m_p_ahb_beat_info_dp->m_hdata, m_ctrl_dp);
    }
    XTSC_VERBOSE(m_text, "DATA phase " << (m_p_ahb_beat_info_dp->m_hwrite ? "WRITE" : "READ ") << " 0x" << hex << setw(8) <<
                           setfill('0') << m_p_ahb_beat_info_dp->m_haddr << "/" << dec << size8 << " " <<
                           xtsc_sd_convert_value_to_string(size8, m_p_ahb_beat_info_dp->m_hdata) << " (0x" << hex <<
                           m_ctrl_dp[0] << " 0x" << m_ctrl_dp[1] << " 0x" << m_ctrl_dp[2] << " 0x" << m_ctrl_dp[3] <<
                           ") status=" << status);
    if (status == MX_STATUS_WAIT) {
      ; // Do nothing
    }
    else {
      // We check both the return value (status) and the value in the AHB_IDX_ACK position.
      // Note: AHB_casi_TSPort converts all return values except MX_STATUS_WAIT to MX_STATUS_OK
      if ((status == MX_STATUS_OK) && (m_ctrl_dp[AHB_IDX_ACK] == AHB_ACK_DONE)) {
        // Capture the read data
        xtsc_request::type_t type = m_p_ahb_beat_info_dp->m_p_request->get_type();
        if ((type == xtsc_request::READ) || (type == xtsc_request::BLOCK_READ) || m_p_ahb_beat_info_dp->m_is_rcw1) {
          xmemcpy(&m_p_nascent_response_buffer[m_p_ahb_beat_info_dp->m_offset], (u8*)m_p_ahb_beat_info_dp->m_hdata, size8);
          if (m_p_ahb_beat_info_dp->m_is_rcw1) {
            m_rcw_read_completed = true;
            m_rcw_read_data_matched = (m_p_ahb_beat_info_dp->m_hdata[0] == rcw_match_data);
          }
        }
        communicate_helper(m_p_ahb_beat_info_dp, xtsc_response::RSP_OK, false);
        m_do_data_phase = false;
      }
      else {
        XTSC_DEBUG(m_text, "Sending RSP_ADDRESS_ERROR");
        communicate_helper(m_p_ahb_beat_info_dp, xtsc_response::RSP_ADDRESS_ERROR, false);
        m_do_data_phase = false;
        if (m_addr_phase_was_rcw1) {
          m_rcw1_got_rsp_address_error = true;
        }
      }
    }
  }
  bool loop_again;
  do {
    loop_again = false;
    switch (m_state) {
      case IDLE: {
        if (!m_ahb_beat_info_deque.empty()) {
          m_p_ahb_beat_info_ap = m_ahb_beat_info_deque.front();
          m_ahb_beat_info_deque.pop_front();
          XTSC_DEBUG(m_text, "m_state=IDLE got AHB beat: " << *m_p_ahb_beat_info_ap);
          if (m_immediate_write_response &&
              m_p_ahb_beat_info_ap->m_p_request->get_last_transfer() &&
              m_p_ahb_beat_info_ap->m_first_beat)
          {
            xtsc_request::type_t type = m_p_ahb_beat_info_ap->m_p_request->get_type();
            if ((type == xtsc_request::WRITE) || (type == xtsc_request::BLOCK_WRITE)) {
              communicate_helper(m_p_ahb_beat_info_ap, xtsc_response::RSP_OK, true);
            }
          }
          m_do_hlock = m_p_ahb_beat_info_ap->m_do_hlock;
          m_state = ARB;
          loop_again = true;
        }
        else {
          // Turn off our requests
          if (m_hreq) {
            XTSC_DEBUG(m_text, "calling requestAccess(0) m_state=IDLE");
            m_ahb_master_port.requestAccess(0);
            m_hreq = 0;
          }
        }
        break;
      }
      case ARB: {
        XTSC_DEBUG(m_text, "m_state=ARB grant=" << xtsc_sd_get_grant_string(grant) << " m_p_signals->hready_z1=" <<
                           m_p_signals->hready_z1);
        if (!m_own_addr_bus) {
          if (m_hreq != 1) {
            m_hreq = 1;
            MxU32 request_access = 1;
            // requestAccess() can only assert HLOCK, it cannot deassert it
            if (m_do_hlock) {
              request_access = 3;
              m_hlock = 1;
            }
            XTSC_DEBUG(m_text, "calling requestAccess(" << request_access << ") m_state=ARB");
            m_ahb_master_port.requestAccess(request_access);
          }
          break;
        }
        if (m_do_hlock && !m_hlock) {
          XTSC_DEBUG(m_text, "changing m_state from ARB to LOCK");
          m_state = LOCK;
        }
        else {
          XTSC_DEBUG(m_text, "changing m_state from ARB to ADDR");
          m_state = ADDR;
          m_arb_to_addr = true;
        }
        loop_again = true;
        break;
      }
      case LOCK: {
        XTSC_DEBUG(m_text, "m_state=LOCK m_p_signals->hready_z1=" << m_p_signals->hready_z1 <<
                           " m_p_signals->hready=" << m_p_signals->hready);
        m_hlock         = 1;
        m_do_unlock     = false;
        m_haddr         = m_p_ahb_beat_info_ap->m_haddr;
        m_hwrite        = m_p_ahb_beat_info_ap->m_hwrite;
        m_hsize         = m_p_ahb_beat_info_ap->m_hsize;
        MxU32 hburst    = m_p_ahb_beat_info_ap->m_hburst;
        MxU32 htrans    = AHB_TRANS_IDLE;
        m_ctrl_ap[AHB_IDX_TYPE] = m_p_ahb_beat_info_ap->m_type;
        AHB_ACC_SET_HSIZE( m_ctrl_ap[AHB_IDX_ACC], m_p_ahb_beat_info_ap->m_hsize);
        AHB_ACC_SET_HLOCK( m_ctrl_ap[AHB_IDX_ACC], m_hlock);
        AHB_ACC_SET_HTRANS(m_ctrl_ap[AHB_IDX_ACC], htrans);
        AHB_ACC_SET_HBURST(m_ctrl_ap[AHB_IDX_ACC], hburst);
        AHB_ACC_SET_HPROT( m_ctrl_ap[AHB_IDX_ACC], m_p_ahb_beat_info_ap->m_hprot);
        m_did_lock_cycle = true;
        m_state = ARB;
        break;
      }
      case ADDR: {
        XTSC_DEBUG(m_text, "m_state=ADDR m_arb_to_addr=" << m_arb_to_addr << " m_p_signals->hready_z1=" << m_p_signals->hready_z1);
        // m_arb_to_addr           => This is (the first cycle of) our address phase
        // !m_p_signals->hready_z1 => We're stuck in the address phase because of a wait state in somebody's data phase
        if (m_arb_to_addr || !m_p_signals->hready_z1) {
          m_end_of_burst = m_p_ahb_beat_info_ap->m_end_of_burst;
          m_arb_to_addr = false;
          m_did_addr_phase = true;
          m_addr_phase_was_rcw1 = m_p_ahb_beat_info_ap->m_is_rcw1;
          m_rcw_read_completed = false;
          m_haddr      = m_p_ahb_beat_info_ap->m_haddr;
          m_hwrite     = m_p_ahb_beat_info_ap->m_hwrite;
          m_hsize      = m_p_ahb_beat_info_ap->m_hsize;
          m_do_unlock  = m_p_ahb_beat_info_ap->m_do_unlock;
          if (m_do_unlock) {
            m_do_hlock = false;
          }
          m_hlock      = (m_do_hlock ? 1 : 0);
          MxU32 hburst = m_p_ahb_beat_info_ap->m_hburst;
          MxU32 htrans = m_p_ahb_beat_info_ap->m_htrans;
          if (m_early_burst_termination) {
            hburst = AHB_BURST_SINGLE;
            htrans = AHB_TRANS_NONSEQ;
          }

          m_ctrl_ap[AHB_IDX_TYPE] = m_p_ahb_beat_info_ap->m_type;
          AHB_ACC_SET_HSIZE( m_ctrl_ap[AHB_IDX_ACC], m_p_ahb_beat_info_ap->m_hsize);
          AHB_ACC_SET_HLOCK( m_ctrl_ap[AHB_IDX_ACC], m_hlock);
          AHB_ACC_SET_HTRANS(m_ctrl_ap[AHB_IDX_ACC], htrans);
          AHB_ACC_SET_HBURST(m_ctrl_ap[AHB_IDX_ACC], hburst);
          AHB_ACC_SET_HPROT( m_ctrl_ap[AHB_IDX_ACC], m_p_ahb_beat_info_ap->m_hprot);

          XTSC_DEBUG(m_text, "ADDR phase " << (m_hwrite ? "WRITE" : "READ ") << " 0x" << hex << setw(8) << setfill('0') << m_haddr <<
                             " (0x" << m_ctrl_ap[0] << " 0x" << m_ctrl_ap[1] << " 0x" << m_ctrl_ap[2] << " 0x" << m_ctrl_ap[3] << ")");
          if (m_hwrite) {
            m_ahb_master_port.write(m_haddr, m_unused_value, m_ctrl_ap);
          }
          else {
            m_ahb_master_port.read(m_haddr, m_unused_value, m_ctrl_ap);
          }
        }
        else {
          // Previous cycle was a no-wait-state address cycle
          if (!m_end_of_burst && !have_grant) {
            if (!m_early_burst_termination) {
              XTSC_DEBUG(m_text, "Detected early burst termination");
              m_early_burst_termination = true;
            }
          }
          if (m_early_burst_termination && m_end_of_burst) {
            XTSC_DEBUG(m_text, "Completed final ADDR phase of early terminated burst");
            m_early_burst_termination = false;
          }
          if (m_addr_phase_was_rcw1) {
            m_state = WAIT_RCW;
          }
          else {
            m_state = IDLE;
          }
          loop_again = true;
        }
        break;
      }
      case WAIT_RCW: {
        // Make sure we complete the data phase of the READ RCW beat before doing the addr phase of the WRITE RCW beat
        XTSC_DEBUG(m_text, "wRCW: m_rcw_read_completed_z1=" << boolalpha << m_rcw_read_completed_z1  <<
                           " m_p_request_fifo->num_available()=" << m_p_request_fifo->num_available());
        if ((m_rcw_read_completed_z1 || m_rcw1_got_rsp_address_error_z1) && m_p_request_fifo->num_available()) {
          if (m_rcw_read_data_matched && !m_rcw1_got_rsp_address_error_z1) {
            // Allow the write beat to be driven on the AHB bus 
            m_state = IDLE;
          }
          else {
            // Do NOT drive the write beat on the AHB bus 
            ahb_beat_info *p_ahb_beat_info = m_ahb_beat_info_deque.front();
            m_do_unlock = p_ahb_beat_info->m_do_unlock;
            m_ahb_beat_info_deque.pop_front();
            xtsc_response::status_t status = (m_rcw1_got_rsp_address_error_z1 ? xtsc_response::RSP_ADDRESS_ERROR :
                                                                                xtsc_response::RSP_OK);
            communicate_helper(p_ahb_beat_info, status , false);
            m_state = (m_do_unlock ? UNLOCK : IDLE);
          }
          m_rcw1_got_rsp_address_error = false;
          loop_again = true;
        }
        break;
      }
      case UNLOCK: {
        XTSC_DEBUG(m_text, "UNLOCK m_did_addr_phase=" << boolalpha << m_did_addr_phase << " m_own_addr_bus=" << m_own_addr_bus);
        if (m_did_addr_phase) {
          ; // Revisit this state next cycle
        }
        else if (m_own_addr_bus) {
          XTSC_DEBUG(m_text, "DATA phase READ : Unlocking address/control bus");
          m_ahb_master_port.read(0x0, m_unused_value, m_ctrl_default);
          m_did_addr_phase = true;
          m_did_unlock     = true;
          m_do_hlock       = false;
          m_hlock          = 0;
          m_state          = IDLE;
        }
        else {
          throw xtsc_exception("Program Bug:  m_state=UNLOCK but we don't own the ADDR/control bus in "
                               "xtsc_pif2ahb_bridge_sd::communicate()");
        }
        break;
      }
      default: {
        throw xtsc_exception("Program Bug:  Invalid m_state in xtsc_pif2ahb_bridge_sd::communicate()");
      }
    }
  } while (loop_again);
  // If we own the address/control buses but we haven't driven them yet, then ...
  //   Case 1: If we have the bus locked, then drive "correct" values except HTRANS is IDLE
  //           This can happen in between the 2 xfers of an RCW or during the LOCK state.
  //   Case 2: Otherwise, drive all 0's
  if (m_own_addr_bus && !m_did_addr_phase) {
    if (m_do_hlock) {
      XTSC_DEBUG(m_text, "ADDR phase " << (m_hwrite ? "WRITE" : "READ ") << ": Driving IDLE cycle");
      AHB_ACC_SET_HTRANS(m_ctrl_ap[AHB_IDX_ACC], AHB_TRANS_IDLE);
      u32 incr = (((m_state == WAIT_RCW) || m_did_lock_cycle) ? 0 : (1 << m_hsize));
      if (m_hwrite) {
        m_ahb_master_port.write(m_haddr+incr, m_unused_value, m_ctrl_ap);
      }
      else {
        m_ahb_master_port.read(m_haddr+incr, m_unused_value, m_ctrl_ap);
      }
    }
    else {
      XTSC_DEBUG(m_text, "ADDR phase READ : Driving m_ctrl_default for unused default grant.");
      m_ahb_master_port.read(0x0, m_unused_value, m_ctrl_default);
    }
  }
  m_rcw_read_completed_z1 = m_rcw_read_completed;
  m_rcw1_got_rsp_address_error_z1 = m_rcw1_got_rsp_address_error;
}



void xtsc_pif2ahb_bridge_sd::update() {
  XTSC_DEBUG(m_text, "update(): m_p_signals->hready=" << m_p_signals->hready << " m_own_addr_bus=" << boolalpha << m_own_addr_bus <<
                     " old m_do_data_phase=" << m_do_data_phase);
  if (m_did_addr_phase && !m_did_unlock && !m_did_lock_cycle && m_p_signals->hready) {
    if (!m_do_data_phase) {
      assert(m_p_ahb_beat_info_ap);
      m_do_data_phase = true;
      m_p_ahb_beat_info_dp = m_p_ahb_beat_info_ap;
      m_p_ahb_beat_info_ap = NULL;
      m_ctrl_dp[AHB_IDX_TYPE] = m_ctrl_ap[AHB_IDX_TYPE];
      m_ctrl_dp[AHB_IDX_ACC ] = m_ctrl_ap[AHB_IDX_ACC ];
    }
  }
}



char *xtsc_pif2ahb_bridge_sd::get_state_string() {
  switch (m_state) {
    case IDLE:     return "IDLE ";
    case ARB:      return "ARB  ";
    case LOCK:     return "LOCK ";
    case ADDR:     return "ADDR ";
    case WAIT_RCW: return "wRCW ";
    case UNLOCK:   return "UNLK ";
    default:       return "UNKN ";
  }
}



void xtsc_pif2ahb_bridge_sd::generate_ahb_beat_info(const xtsc::xtsc_request& request) {

  XTSC_DEBUG(m_text, request << " Generating ahb_beat_info");

  xtsc_request::type_t  type            = request.get_type();
  xtsc_address          address8        = request.get_byte_address();
  u32                   size8           = request.get_byte_size();
  u32                   num_transfers   = request.get_num_transfers();
  bool                  last_transfer   = request.get_last_transfer();
  u32                   num_ahb_beats   = ((size8 <= m_max_beat_width8) ? 1 : (size8 / m_max_beat_width8));
  u32                   tot_ahb_beats   = num_ahb_beats * num_transfers;
  if (type == xtsc_request::READ || type == xtsc_request::BLOCK_READ) {
    num_ahb_beats = tot_ahb_beats;
  }
  bool                  block           = ((type == xtsc_request::BLOCK_WRITE) || (type == xtsc_request::BLOCK_READ));
  bool                  block_2         = (block && (num_transfers == 2));
  bool                  hwrite          = ((type == xtsc_request::WRITE      ) ||
                                           (type == xtsc_request::BLOCK_WRITE) ||
                                           (type == xtsc_request::RCW && last_transfer));
  MxU32                 haddr           = address8;
  MxU32                 htrans          = m_htrans_next;
  u32                   ahb_size8       = ((num_ahb_beats == 1) ? size8 : m_max_beat_width8);
  MxU32                 hsize           = ((ahb_size8 ==   1) ? 0 :
                                           (ahb_size8 ==   2) ? 1 :
                                           (ahb_size8 ==   4) ? 2 :
                                           (ahb_size8 ==   8) ? 3 :
                                           (ahb_size8 ==  16) ? 4 :
                                           (ahb_size8 ==  32) ? 5 :
                                           (ahb_size8 ==  64) ? 6 : 7);
  MxU32                 hlock           = (m_lock_block_2 && block_2) ||
                                          (m_lock_block   && block)   ||
                                          (m_lock         && ((type == xtsc_request::RCW) || (num_ahb_beats > 1)));
  MxU32                 hburst          = ((type == xtsc_request::RCW) ? AHB_BURST_SINGLE :
                                           (block_2                  ) ? AHB_BURST_SINGLE :
                                           (tot_ahb_beats ==  1      ) ? AHB_BURST_SINGLE :
                                           (tot_ahb_beats ==  4      ) ? (m_wrap ? AHB_BURST_WRAP4  : AHB_BURST_INCR4)  :
                                           (tot_ahb_beats ==  8      ) ? (m_wrap ? AHB_BURST_WRAP8  : AHB_BURST_INCR8)  :
                                           (tot_ahb_beats == 16      ) ? (m_wrap ? AHB_BURST_WRAP16 : AHB_BURST_INCR16) :
                                           AHB_BURST_INCR);
  MxU32                 hprot           = m_hprot;
  MxU32                 hdata[4]        = { 0, 0, 0, 0};


  const u8             *p_buffer        = request.get_buffer();
  u32                   offset          = 0;


  m_htrans_next = ((last_transfer || (type == xtsc_request::RCW)) ? AHB_TRANS_NONSEQ : AHB_TRANS_SEQ);

  m_is_rcw1 = ((type == xtsc_request::RCW) && !last_transfer);

  if (type == xtsc_request::BLOCK_WRITE || type == xtsc_request::BLOCK_READ) {
    if (size8 != m_pif_byte_width) {
      ostringstream oss;
      oss << "xtsc_pif2ahb_bridge_sd '" << getInstanceName() << "' received BLOCK request whose transfer size (" << size8
          << ") does not match its bus width (" << m_pif_byte_width << "): " << request;
      throw xtsc_exception(oss.str());
    }
  }

  for (u32 i=0; i<num_ahb_beats; ++i) {

    u32 next_offset = offset;
    if ((type == xtsc_request::READ) || (type == xtsc_request::BLOCK_READ)) {
      next_offset = (offset + ahb_size8) % m_pif_byte_width;
    } 

    if (hwrite || (type == xtsc_request::RCW)) {
      xmemcpy(hdata, &p_buffer[ahb_size8*i], ahb_size8);
    }

    bool last_beat = ((i + 1) == num_ahb_beats);

    bool completes_request = last_beat;

    bool end_of_burst = (block_2 || (last_beat && (m_htrans_next == AHB_TRANS_NONSEQ)));

    bool send_response = (((type == xtsc_request::READ)        && last_beat)                  ||
                          ((type == xtsc_request::WRITE)       && last_beat)                  ||
                          ((type == xtsc_request::BLOCK_READ)  && (next_offset == 0))         ||
                          ((type == xtsc_request::BLOCK_WRITE) && last_transfer && last_beat) ||
                          ((type == xtsc_request::RCW)         && last_transfer));

    bool response_last_transfer = (send_response && ((type != xtsc_request::BLOCK_READ) || last_beat));

    bool unlock = (hlock && last_beat && last_transfer);

    ahb_beat_info *p_info = new_ahb_beat_info(hwrite, haddr, htrans, hlock, hsize, hburst, hprot, hdata, unlock, m_is_rcw1,
                                              (i==0), completes_request, send_response, response_last_transfer, end_of_burst, 
                                              offset, &request);
    m_ahb_beat_info_deque.push_back(p_info);

    XTSC_INFO(m_text, *p_info);

    offset  = next_offset;
    haddr  += ahb_size8;
    if (!block_2) {
      htrans  = AHB_TRANS_SEQ;
    }
  }

}



xtsc_pif2ahb_bridge_sd::request_info *xtsc_pif2ahb_bridge_sd::new_request_info(const xtsc_request& request) {
  if (m_request_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new request_info for: " << request);
    return new request_info(request);
  }
  else {
    XTSC_DEBUG(m_text, "Recycling an old request_info for: " << request);
    request_info *p_request_info = m_request_pool.back();
    m_request_pool.pop_back();
    p_request_info->m_request = request;
    p_request_info->m_time_stamp = sc_time_stamp();
    return p_request_info;
  }
}



void xtsc_pif2ahb_bridge_sd::delete_request_info(request_info*& p_request_info) {
  XTSC_DEBUG(m_text, "Deleting request_info: " << p_request_info->m_request);
  m_request_pool.push_back(p_request_info);
  p_request_info = 0;
}



xtsc_pif2ahb_bridge_sd::ahb_beat_info *xtsc_pif2ahb_bridge_sd::new_ahb_beat_info(bool                      hwrite,
                                                                                 MxU32                     haddr,
                                                                                 MxU32                     htrans,
                                                                                 MxU32                     hlock,
                                                                                 MxU32                     hsize,
                                                                                 MxU32                     hburst,
                                                                                 MxU32                     hprot,
                                                                                 MxU32                     hdata[4],
                                                                                 bool                      unlock,
                                                                                 bool                      is_rcw1,
                                                                                 bool                      first_beat,
                                                                                 bool                      completes_request,
                                                                                 bool                      send_response,
                                                                                 bool                      last_transfer,
                                                                                 bool                      end_of_burst,
                                                                                 xtsc::u32                 offset,
                                                                                 const xtsc::xtsc_request *p_request)
{
  if (m_ahb_beat_info_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new ahb_beat_info for: offset=" << offset << " " << *p_request);
    return new ahb_beat_info(hwrite, haddr, htrans, hlock, hsize, hburst, hprot, hdata, unlock, is_rcw1,
                             first_beat, completes_request, send_response, last_transfer, end_of_burst, offset, p_request);
  }
  else {
    XTSC_DEBUG(m_text, "Recycling an old ahb_beat_info for: offset=" << offset << " " << *p_request);
    ahb_beat_info *p_ahb_beat_info = m_ahb_beat_info_pool.back();
    m_ahb_beat_info_pool.pop_back();
    p_ahb_beat_info->init(hwrite, haddr, htrans, hlock, hsize, hburst, hprot, hdata, unlock, is_rcw1,
                          first_beat, completes_request, send_response, last_transfer, end_of_burst, offset, p_request);
    return p_ahb_beat_info;
  }
}




void xtsc_pif2ahb_bridge_sd::delete_ahb_beat_info(ahb_beat_info*& p_ahb_beat_info) {
  XTSC_DEBUG(m_text, "Deleting ahb_beat_info for: offset=" << p_ahb_beat_info->m_offset << " " << *p_ahb_beat_info->m_p_request);
  m_ahb_beat_info_pool.push_back(p_ahb_beat_info);
  p_ahb_beat_info = 0;
}



void xtsc_pif2ahb_bridge_sd::ahb_beat_info::init(bool                      hwrite,
                                                 MxU32                     haddr,
                                                 MxU32                     htrans,
                                                 MxU32                     hlock,
                                                 MxU32                     hsize,
                                                 MxU32                     hburst,
                                                 MxU32                     hprot,
                                                 MxU32                     hdata[4],
                                                 bool                      unlock,
                                                 bool                      is_rcw1,
                                                 bool                      first_beat,
                                                 bool                      completes_request,
                                                 bool                      send_response,
                                                 bool                      last_transfer,
                                                 bool                      end_of_burst,
                                                 xtsc::u32                 offset,
                                                 const xtsc::xtsc_request *p_request)
{
  m_hwrite                  = hwrite;
  m_haddr                   = haddr;
  m_htrans                  = htrans;
  m_do_hlock                = hlock;
  m_hsize                   = hsize;
  m_hburst                  = hburst;
  m_hprot                   = hprot;
  memcpy(m_hdata, hdata, 16);
  m_do_unlock               = unlock;
  m_is_rcw1                 = is_rcw1;
  m_first_beat              = first_beat;
  m_completes_request       = completes_request;
  m_send_response           = send_response;
  m_last_transfer           = last_transfer;
  m_end_of_burst            = end_of_burst;
  m_offset                  = offset;
  m_p_request               = p_request;
       if (hsize == 0) { m_type = AHB_TYPE_BYTE;    }
  else if (hsize == 1) { m_type = AHB_TYPE_HWORD;   }
  else if (hsize == 2) { m_type = AHB_TYPE_WORD;    }
  else if (hsize == 3) { m_type = AHB_TYPE_DWORD;   }
  else if (hsize == 4) { m_type = AHB_TYPE_128BIT;  }
  else if (hsize == 5) { m_type = AHB_TYPE_256BIT;  }
  else if (hsize == 6) { m_type = AHB_TYPE_512BIT;  }
  else if (hsize == 7) { m_type = AHB_TYPE_1024BIT; }
  else {
    ostringstream oss;
    oss << "xtsc_pif2ahb_bridge_sd::ahb_beat_info::init(): Can't convert hsize=" << hsize << " into AHB_ACCESS_TYPE";
    throw xtsc_exception(oss.str());
  }
}



void xtsc_pif2ahb_bridge_sd::ahb_beat_info::dump(ostream& os, bool dump_data) const {

  // Save state of stream
  char c = os.fill('0');
  ios::fmtflags old_flags = os.flags();

  u32 size8 = (1 << m_hsize);

  os << (m_hwrite ? "W" : "R") << (m_do_hlock ? "L" : "U") << " [0x" << hex << setw(8) << m_haddr << "/" << dec << size8 << "] ";

  if (m_htrans == AHB_TRANS_IDLE) {
    os << "IDLE";
  }
  else if (m_htrans == AHB_TRANS_BUSY) {
    os << "BUSY";
  }
  else if (m_htrans == AHB_TRANS_NONSEQ) {
    os << "NSEQ";
  }
  else if (m_htrans == AHB_TRANS_SEQ) {
    os << "SEQ ";
  }
  else {
    os << "UNKN";
  }

  os << (m_end_of_burst ? "*" : " ");

  if (m_hburst == AHB_BURST_SINGLE) {
    os << "SNGL ";
  }
  else if (m_hburst == AHB_BURST_INCR) {
    os << "INCx ";
  }
  else if (m_hburst == AHB_BURST_WRAP4) {
    os << "WRP4 ";
  }
  else if (m_hburst == AHB_BURST_INCR4) {
    os << "INC4 ";
  }
  else if (m_hburst == AHB_BURST_WRAP8) {
    os << "WRP8 ";
  }
  else if (m_hburst == AHB_BURST_INCR8) {
    os << "INC8 ";
  }
  else if (m_hburst == AHB_BURST_WRAP16) {
    os << "WRP16";
  }
  else if (m_hburst == AHB_BURST_INCR16) {
    os << "INC16";
  }

  os << dec << " 0x" << m_hprot << " " << (m_is_rcw1 ? "RCW" : "   ") << " ";
  os << (m_completes_request ? "EOR "  : "    " );
  os << (m_send_response     ? "RSP "  : "    " );
  os << (m_last_transfer     ? "LAST " : "     ");
  os << (m_do_unlock         ? "UNL "  : "    ");

  if (dump_data && (m_hwrite || m_is_rcw1)) {
    os << "= ";
    xtsc_hex_dump(size8, (u8*)m_hdata, os);
  }

  // Restore state of stream
  os.fill(c);
  os.flags(old_flags);

}



std::ostream& operator<<(std::ostream& os, const xtsc_pif2ahb_bridge_sd::ahb_beat_info& info) {
  info.dump(os, true);
  return os;
}



xtsc_pif2ahb_bridge_sd::response_info *xtsc_pif2ahb_bridge_sd::new_response_info(const xtsc_request& request) {
  response_info *p_response_info = NULL;
  if (m_response_pool.empty()) {
    XTSC_DEBUG(m_text, "Creating a new response_info for: " << request);
    p_response_info = new response_info(request);
  }
  else {
    XTSC_DEBUG(m_text, "Recycling an old response_info for: " << request);
    p_response_info = m_response_pool.back();
    m_response_pool.pop_back();
    p_response_info->init(request);
  }
  XTSC_DEBUG(m_text, "p_response_info=" << p_response_info << " ->m_p_response=" << p_response_info->m_p_response <<
                     ": " << *p_response_info->m_p_response);
  return p_response_info;
}



void xtsc_pif2ahb_bridge_sd::delete_response_info(response_info*& p_response_info) {
  XTSC_DEBUG(m_text, "Deleting response_info for: " << *p_response_info->m_p_response);
  delete p_response_info->m_p_response;
  p_response_info->m_p_response = NULL;
  m_response_pool.push_back(p_response_info);
  p_response_info = 0;
}



void xtsc_pif2ahb_bridge_sd::response_thread() {
  try {
    while (true) {
      wait(m_response_thread_event);
      XTSC_DEBUG(m_text, "response_thread woke up.");
      while (m_p_response_fifo->num_available()) {
        response_info *p_response_info;
        m_p_response_fifo->nb_read(p_response_info);
        XTSC_DEBUG(m_text, "response_thread got: " << *p_response_info->m_p_response);
        // Calculate delay (net => No Earlier Than time)
        sc_time response_delay    = (p_response_info->m_is_read ? m_read_response_delay_sct : m_write_response_delay_sct);
        sc_time receipt_net       = p_response_info->m_time_stamp + response_delay;
        sc_time last_response_net = m_last_response_time_stamp + (m_delay_from_receipt ? m_recovery_time_sct : response_delay);
        sc_time latest_net        = (receipt_net > last_response_net) ? receipt_net : last_response_net;
        sc_time now               = sc_time_stamp();
        sc_time delay = (latest_net <= now) ? SC_ZERO_TIME : (latest_net - now);
        XTSC_DEBUG(m_text, "response_thread() doing wait for " << delay << ": m_last_response_time_stamp=" << m_last_response_time_stamp <<
                           " m_recovery_time_sct=" << m_recovery_time_sct);
        wait(delay);
        if (p_response_info->m_is_write && (p_response_info->m_p_response->get_status() != xtsc_response::RSP_OK)) {
          m_write_bus_error_event.notify(); // Immediate notification
        }
        // Send the response
        u32 tries = 0;
        while (true) {
          tries += 1;
          XTSC_INFO(m_text, *p_response_info->m_p_response << " Try #" << tries);
          if (m_pif_respond_port->nb_respond(*p_response_info->m_p_response)) {
            break;
          }
          wait(m_response_repeat_sct);
        };
        m_last_response_time_stamp = sc_time_stamp();
        delete_response_info(p_response_info);
      }
    }
  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in response_thread of " << getName() << " '" << getInstanceName() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, log4xtensa::FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



void xtsc_pif2ahb_bridge_sd::write_bus_error_thread() {
  try {
    if (!m_write_bus_error.get_interface()) {
      XTSC_INFO(m_text, "The m_write_bus_error port is not bound.");
      return;
    }
    while (true) {
      wait(m_write_bus_error_event);
      XTSC_DEBUG(m_text, "write_bus_error_thread woke up.");
      m_write_bus_error->nb_write(m_one);
      wait(m_pif_clock_period_sct);
      m_write_bus_error->nb_write(m_zero);
    }
  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in write_bus_error_thread of " << getName() << " '" << getInstanceName() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, log4xtensa::FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }
}



void xtsc_pif2ahb_bridge_sd::dump_container_info(std::ostream& os) const {
  os << "sc_fifo m_p_request_fifo:        " << m_p_request_fifo->num_available() << "/" << m_p_request_fifo->num_free() << endl;
  os << "vector  m_request_pool:        " << m_request_pool.size() << "/" << m_request_pool.max_size() << endl;

  os << "sc_fifo m_p_response_fifo:       " << m_p_response_fifo->num_available() << "/" << m_p_response_fifo->num_free() << endl;
  os << "vector  m_response_pool:       " << m_response_pool.size() << "/" << m_response_pool.max_size() << endl;

  os << "deque   m_ahb_beat_info_deque: " << m_ahb_beat_info_deque.size() << "/" << m_ahb_beat_info_deque.max_size() << endl;
  os << "vector  m_ahb_beat_info_pool:  " << m_ahb_beat_info_pool.size() << "/" << m_ahb_beat_info_pool.max_size() << endl;
}



/*
 * Big Endian:
 * Just peek/poke one byte at a time
 *
 * Little Endian:
 * Arbitrary size and alignment are handled by breaking each peek/poke into three parts:
 *   - header
 *   - body
 *   - trailer
 *
 * The following table shows the size of these 3 parts as a function of the 2 least-
 * significant bits of address (Adr) and size for the case of a 32-bit (4-byte) data bus.
 *
 * Table entry format is:   header_bytes:body_bytes:trailer_bytes
 * 
 *        Adr    Adr    Adr    Adr
 * size   0x0    0x1    0x2    0x3
 * ----  -----  -----  -----  -----
 * 1     1:0:0  1:0:0  1:0:0  1:0:0
 * 2     2:0:0  2:0:0  2:0:0  1:0:1
 * 3     3:0:0  3:0:0  2:0:1  1:0:2
 * 4     0:4:0  3:0:1  2:0:2  1:0:3
 * 5     0:4:1  3:0:2  2:0:3  1:4:0
 * 6     0:4:2  3:0:3  2:4:0  1:4:1
 * 7     0:4:3  3:4:0  2:4:1  1:4:2
 */
void xtsc_pif2ahb_bridge_sd::xtsc_request_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  XTSC_VERBOSE(m_bridge.m_text, "nb_peek addr=0x" << hex << address8 << " size8=0x" << size8);
  MxStatus status;
  MxU32 buf[4] = { 0, 0, 0, 0};

  if (m_bridge.m_big_endian) {
    m_bridge.m_ctrl_peek_poke[AHB_IDX_TYPE] = AHB_TYPE_BYTE;
    for (u32 i=0; i<size8; ++i) {
      u32 addr8 = address8+i;
      if ((status = m_bridge.m_ahb_master_port.readDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_peek): readDbg() failed:  address8=0x"
            << hex << address8 << " size=1";
        throw xtsc_exception(oss.str());
      }
      buffer[i] = *(u8*)buf;
    }
  }
  else {

    m_bridge.m_ctrl_peek_poke[AHB_IDX_TYPE] = m_bridge.m_ahb_native_access_type;

    u32 header_bytes  = ((address8 & m_bridge.m_ahb_width8_mask) ?
                          min(size8, (m_bridge.m_ahb_byte_width - (address8 & m_bridge.m_ahb_width8_mask))) :
                          ((size8 < m_bridge.m_ahb_byte_width) ? size8 : 0));
    u32 trailer_bytes = (size8 - header_bytes) & m_bridge.m_ahb_width8_mask;
    u32 body_bytes    = size8 - header_bytes - trailer_bytes;

    u32 addr8 = address8;

    if (header_bytes) {
      u32 offset = addr8 & m_bridge.m_ahb_width8_mask;
      addr8 = addr8 & (-m_bridge.m_ahb_byte_width);  // align addr8 to AHB bus width boundary
      if ((status = m_bridge.m_ahb_master_port.readDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_peek header_bytes block): readDbg() failed:  address8=0x"
            << hex << address8 << " size8=0x" << size8;
        throw xtsc_exception(oss.str());
      }
      u8 *p_d = reinterpret_cast<u8*>(&buf);
      p_d += offset;
      memcpy(buffer, p_d, header_bytes);
      buffer += header_bytes;
      addr8 += m_bridge.m_ahb_byte_width;
    }

    while (body_bytes) {
      if ((status = m_bridge.m_ahb_master_port.readDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_peek body_bytes block): readDbg() failed:  address8=0x"
            << hex << address8 << " size8=0x" << size8;
        throw xtsc_exception(oss.str());
      }
      memcpy(buffer, &buf, m_bridge.m_ahb_byte_width);
      buffer += m_bridge.m_ahb_byte_width;
      addr8 += m_bridge.m_ahb_byte_width;
      body_bytes -= m_bridge.m_ahb_byte_width;
    }

    if (trailer_bytes) {
      if ((status = m_bridge.m_ahb_master_port.readDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_peek trailer_bytes block): readDbg() failed:  "
            << "address8=0x" << hex << address8 << " size8=0x" << size8;
        throw xtsc_exception(oss.str());
      }
      memcpy(buffer, &buf, trailer_bytes);
      buffer += trailer_bytes;
      addr8 += m_bridge.m_ahb_byte_width;
    }

  }
}



// See comments above the nb_peek method
void xtsc_pif2ahb_bridge_sd::xtsc_request_if_impl::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  XTSC_VERBOSE(m_bridge.m_text, "nb_poke addr=0x" << hex << address8 << " size8=0x" << size8);
  MxStatus status;
  MxU32 buf[4] = { 0, 0, 0, 0};

  if (m_bridge.m_big_endian) {
    m_bridge.m_ctrl_peek_poke[AHB_IDX_TYPE] = AHB_TYPE_BYTE;
    for (u32 i=0; i<size8; ++i) {
      u32 addr8 = address8+i;
      *(u8*)buf = buffer[i];
      if ((status = m_bridge.m_ahb_master_port.writeDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_poke): writeDbg() failed:  address8=0x"
            << hex << address8 << " size8=1";
        throw xtsc_exception(oss.str());
      }
    }
  }
  else {

    m_bridge.m_ctrl_peek_poke[AHB_IDX_TYPE] = m_bridge.m_ahb_native_access_type;

    u32 header_bytes  = ((address8 & m_bridge.m_ahb_width8_mask) ?
                          min(size8, (m_bridge.m_ahb_byte_width - (address8 & m_bridge.m_ahb_width8_mask))) :
                          ((size8 < m_bridge.m_ahb_byte_width) ? size8 : 0));
    u32 trailer_bytes = (size8 - header_bytes) & m_bridge.m_ahb_width8_mask;
    u32 body_bytes    = size8 - header_bytes - trailer_bytes;

    u32 addr8 = address8;

    if (header_bytes) {
      u32 offset = addr8 & m_bridge.m_ahb_width8_mask;
      addr8 = addr8 & (-m_bridge.m_ahb_byte_width);  // align addr8 to AHB bus width boundary
      nb_peek(addr8, m_bridge.m_ahb_byte_width, (u8*)&buf);
      u8 *p_d = reinterpret_cast<u8*>(&buf);
      p_d += offset;
      memcpy(p_d, buffer, header_bytes);
      if ((status = m_bridge.m_ahb_master_port.writeDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_poke header_bytes block): writeDbg() failed:  address8=0x"
            << hex << address8 << " size8=0x" << size8;
        throw xtsc_exception(oss.str());
      }
      buffer += header_bytes;
      addr8 += m_bridge.m_ahb_byte_width;
    }

    while (body_bytes) {
      memcpy(&buf, buffer, m_bridge.m_ahb_byte_width);
      if ((status = m_bridge.m_ahb_master_port.writeDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_poke body_bytes block): writeDbg() failed:  address8=0x"
            << hex << address8 << " size8=0x" << size8;
        throw xtsc_exception(oss.str());
      }
      buffer += m_bridge.m_ahb_byte_width;
      addr8 += m_bridge.m_ahb_byte_width;
      body_bytes -= m_bridge.m_ahb_byte_width;
    }

    if (trailer_bytes) {
      nb_peek(addr8, m_bridge.m_ahb_byte_width, (u8*)&buf);
      memcpy(&buf, buffer, trailer_bytes);
      if ((status = m_bridge.m_ahb_master_port.writeDbg(addr8, buf, m_bridge.m_ctrl_peek_poke)) != MX_STATUS_OK) {
        ostringstream oss;
        oss << m_bridge.getInstanceName() << " (in xtsc_pif2ahb_bridge_sd::nb_poke trailer_bytes block): writeDbg() failed:  address8=0x"
            << hex << address8 << " size8=0x" << size8;
        throw xtsc_exception(oss.str());
      }
      buffer += trailer_bytes;
      addr8 += m_bridge.m_ahb_byte_width;
    }

  }
}



bool xtsc_pif2ahb_bridge_sd::xtsc_request_if_impl::nb_fast_access(xtsc_fast_access_request & /* request */) {
  return false;
}



void xtsc_pif2ahb_bridge_sd::xtsc_request_if_impl::nb_request(const xtsc_request& request) {
  XTSC_INFO(m_bridge.m_text, request);

  // Check if we've got room for this request
  if (m_bridge.m_p_request_fifo->num_free() == 0) {
    xtsc_response response(request, xtsc_response::RSP_NACC);
    XTSC_INFO(m_bridge.m_text, response << " (Request fifo full)");
    m_bridge.m_pif_respond_port->nb_respond(response);
    return;
  }

  // Do some error checking
  xtsc_request::type_t    type     = request.get_type();
  xtsc_address            address8 = request.get_byte_address();
  u32                     size8    = request.get_byte_size();
  if (address8 % size8) {
    XTSC_INFO(m_bridge.m_text, request << " RSP_ADDRESS_ERROR - address is not size-aligned");
    m_bridge.m_status = xtsc_response::RSP_ADDRESS_ERROR;
  }
  else if (m_bridge.m_is_rcw1 && ((type != xtsc_request::RCW) || !request.get_last_transfer())) {
    XTSC_INFO(m_bridge.m_text, request << " RSP_ADDRESS_ERROR - expected RCW with last transfer flag set");
    m_bridge.m_status = xtsc_response::RSP_ADDRESS_ERROR;
  }
  else if ((type == xtsc_request::RCW) && (size8 != 4)) {
    XTSC_INFO(m_bridge.m_text, request << " RSP_ADDRESS_ERROR - RCW byte size must be 4");
    m_bridge.m_status = xtsc_response::RSP_ADDRESS_ERROR;
  }

  if (size8 > m_bridge.m_pif_byte_width) {
    ostringstream oss;
    oss << m_bridge.getInstanceName() << ": request size exceeds PIF width (" << m_bridge.m_pif_byte_width << "): " << request;
    throw xtsc_exception(oss.str());
  }

  // Is there an error?
  if (m_bridge.m_status != xtsc_response::RSP_OK) {
    // Yes, there is an error.  If this is a last_transfer, then generate an error response.
    if (request.get_last_transfer()) {
      response_info *p_response_info = m_bridge.new_response_info(request);
      p_response_info->m_p_response->set_status(m_bridge.m_status);
      if (!m_bridge.m_p_response_fifo->nb_write(p_response_info)) {
        throw xtsc_exception("Program Bug: m_p_response_fifo is full in xtsc_pif2ahb_bridge_sd::xtsc_request_if_impl::nb_request()");
      }
      m_bridge.m_response_thread_event.notify(SC_ZERO_TIME);
      m_bridge.m_status = xtsc_response::RSP_OK;
    }
  }
  else {
    // No error, so service the request.  First, create our copy of the request
    request_info *p_request_info = m_bridge.new_request_info(request);
    // Add request to tracking fifo
    m_bridge.m_p_request_fifo->nb_write(p_request_info);
    XTSC_DEBUG(m_bridge.m_text, "Wrote m_p_request_fifo: " << p_request_info->m_request);
    // Generate AHB "script" to service this request
    m_bridge.generate_ahb_beat_info(p_request_info->m_request);
  }
}



void xtsc_pif2ahb_bridge_sd::xtsc_request_if_impl::register_port(sc_core::sc_port_base& port, const char * /* if_typename */) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_pif2ahb_bridge_sd '" << m_bridge.getInstanceName() << "' m_pif_request_export: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_bridge.m_text, "Binding '" << port.name() << "' to xtsc_pif2ahb_bridge_sd::m_pif_request_export");
  m_p_port = &port;
}



xtsc_pif2ahb_bridge_sd::response_info::response_info(const xtsc::xtsc_request& request) {
  init(request);
}



void xtsc_pif2ahb_bridge_sd::response_info::init(const xtsc::xtsc_request& request) {
  m_p_response = new xtsc::xtsc_response(request);
  m_time_stamp = sc_time_stamp();
  m_is_read  = (request.get_type() == xtsc_request::READ  || request.get_type() == xtsc_request::BLOCK_READ );
  m_is_write = (request.get_type() == xtsc_request::WRITE || request.get_type() == xtsc_request::BLOCK_WRITE);
}



class xtsc_pif2ahb_bridge_sdFactory : public MxFactory {
public:
  xtsc_pif2ahb_bridge_sdFactory() : MxFactory ("xtsc_pif2ahb_bridge_sd") {}
  sc_mx_m_base *createInstance(sc_mx_m_base *c, const string &id) { 
    maxsim::setDoNotRegisterInSystemC(false);
    maxsim::sc_mx_import_delete_new_module_name();
    return new xtsc_pif2ahb_bridge_sd(c, id.c_str());
  }
};



extern "C" XTSC_SD_EXPORT void MxInit(void) {
  new xtsc_pif2ahb_bridge_sdFactory();
}



extern "C" XTSC_SD_EXPORT void MxInit_SCImport(void) {
}




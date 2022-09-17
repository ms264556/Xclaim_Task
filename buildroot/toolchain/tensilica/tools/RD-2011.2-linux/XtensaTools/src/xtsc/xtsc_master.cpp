// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cctype>
#include <algorithm>
#include <xtsc/xtsc_master.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_cohctrl.h>
#include <xtsc/xtsc_mmio.h>
#include <xtsc/xtsc_wire_logic.h>

using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
using namespace sc_dt;
#endif
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;


typedef xtsc_request::coherence_t coherence_t;



xtsc_component::xtsc_master::xtsc_master(sc_module_name module_name, const xtsc_master_parms& master_parms) :
  sc_module             (module_name),
  m_request_port        ("m_request_port"),
  m_respond_export      ("m_respond_export"),
  m_text                (log4xtensa::TextLogger::getInstance(name())),
  m_control             (master_parms.get_bool("control")),
  m_control_bound       (false),
  m_p_control           (NULL),
  m_p_write_impl        (NULL),
  m_control_value       (1),
  m_wraparound          (master_parms.get_bool("wraparound")),
  m_script_file         (master_parms.get_c_str("script_file")),
  m_script_file_stream  (m_script_file.c_str(), "script_file",  name(), kind(), m_wraparound),
  m_return_value_file   (""),
  m_p_return_value_file (0),
  m_format              (master_parms.get_non_zero_u32("format")),
  m_p_port              (0)
{

  if (m_control) {
    m_p_control = new wire_write_export("control");
    m_p_write_impl = new xtsc_wire_write_if_impl("control__impl", *this);
    (*m_p_control)(*m_p_write_impl);
  }
  m_control_value = 0;

  // Handle return value file
  const char *return_value_file = master_parms.get_c_str("return_value_file");
  if (return_value_file && return_value_file[0]) {
    m_return_value_file   = return_value_file;
    m_p_return_value_file = new xtsc_script_file(return_value_file, "return_value_file",  name(), kind(), true);
  }

  // Get clock period 
  m_time_resolution = sc_get_time_resolution();
  u32 clock_period = master_parms.get_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = sc_get_time_resolution() * clock_period;
  }
  m_clock_period_value = m_clock_period.value();
  u32 posedge_offset = master_parms.get_u32("posedge_offset");
  if (posedge_offset == 0xFFFFFFFF) {
    m_posedge_offset = xtsc_get_system_clock_posedge_offset();
  }
  else {
    m_posedge_offset = posedge_offset * m_time_resolution;
  }
  if (m_posedge_offset >= m_clock_period) {
    ostringstream oss;
    oss << kind() << " '" << name() << "' parameter error:" << endl;
    oss << "\"posedge_offset\" (0x" << hex << posedge_offset << "=>" << m_posedge_offset
        << ") must be strictly less than \"clock_period\" (0x" << clock_period << "=>" << m_clock_period << ")";
    throw xtsc_exception(oss.str());
  }
  m_posedge_offset_value = m_posedge_offset.value();
  m_has_posedge_offset = (m_posedge_offset != SC_ZERO_TIME);

  SC_THREAD(request_thread);

  m_respond_export(*this);

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, "Constructed xtsc_master '" << name() << "':");
  XTSC_LOG(m_text, ll, " control                 = "   << boolalpha << m_control);
  XTSC_LOG(m_text, ll, " script_file             = "   << m_script_file);
  XTSC_LOG(m_text, ll, " wraparound              = "   << boolalpha << m_wraparound);
  XTSC_LOG(m_text, ll, " return_value_file       = "   << m_return_value_file);
  if (clock_period == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, " clock_period            = 0xFFFFFFFF => " << m_clock_period.value() << " (" << m_clock_period << ")");
  } else {
  XTSC_LOG(m_text, ll, " clock_period            = "   << clock_period << " (" << m_clock_period << ")");
  }
  if (posedge_offset == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll, " posedge_offset          = 0xFFFFFFFF => " << m_posedge_offset.value() << " (" << m_posedge_offset << ")");
  } else {
  XTSC_LOG(m_text, ll, " posedge_offset          = "   << posedge_offset << " (" << m_posedge_offset << ")");
  }
  XTSC_LOG(m_text, ll, " format                  = "   << m_format);

  reset();

}



xtsc_component::xtsc_master::~xtsc_master(void) {
}



void xtsc_component::xtsc_master::reset(bool /*hard_reset*/) {
  XTSC_INFO(m_text, "xtsc_master::reset()");

  m_line                        = "";
  m_return_value_line           = "";
  m_line_count                  = 0;
  m_return_value_line_count     = 0;
  m_return_value_index          = 0;
  m_block_write_tag             = 0L;
  m_burst_write_tag             = 0L;
  m_rcw_tag                     = 0L;
  m_last_request_tag            = 0L;
  m_last_request_got_response   = false;
  m_virtual_address_delta       = 0;
  m_last_request_got_nacc       = false;
  m_last_response_status        = xtsc_response::RSP_OK;
  m_fetch                       = false;
  m_use_coherent_peek_poke      = false;
  m_set_xfer_en                 = false;
  m_pif_attribute               = 0xFFFFFFFF;

  m_words.clear();
  m_return_values.clear();

  m_script_file_stream.reset();

  if (m_p_return_value_file) {
    m_p_return_value_file->reset();
  }

}



sc_export<xtsc_wire_write_if>& xtsc_component::xtsc_master::get_control_input() const {
  if (!m_control) {
    ostringstream oss;
    oss << "xtsc_master '" << name() << "' has \"control\" false, so get_control_input() should not be called.";
    throw xtsc_exception(oss.str());
  }
  return *m_p_control;
}



void xtsc_component::xtsc_master::connect(xtsc_wire_logic& logic, const char *output_name) {
  if (!m_control) {
    ostringstream oss;
    oss << "'" << name() << "' has \"control\" false, so xtsc_master::connect(xtsc_wire_logic&, ...) should not be called.";
    throw xtsc_exception(oss.str());
  }
  u32 wo = logic.get_bit_width(output_name);
  u32 wi = 1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_wire_logic '" << logic.name()
        << "' to " << wi << "-bit control input of xtsc_master '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  logic.get_output(output_name)(*m_p_control);
}



void xtsc_component::xtsc_master::connect(xtsc_mmio& mmio, const char *output_name) {
  if (!m_control) {
    ostringstream oss;
    oss << "'" << name() << "' has \"control\" false, so xtsc_master::connect(xtsc_mmio&, ...) should not be called.";
    throw xtsc_exception(oss.str());
  }
  u32 wo = mmio.get_bit_width(output_name);
  u32 wi = 1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_mmio '" << mmio.name()
        << "' to " << wi << "-bit control input of xtsc_master '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  mmio.get_output(output_name)(*m_p_control);
}



void xtsc_component::xtsc_master::connect(xtsc_core& core, const char *port) {
  m_request_port(core.get_request_export(port));
  core.get_respond_port(port)(m_respond_export);
}



void xtsc_component::xtsc_master::connect(xtsc_cohctrl& cohctrl, u32 port_num) {
  u32 num_clients = cohctrl.get_num_clients();
  if (port_num >= num_clients) {
    ostringstream oss;
    oss << "'" << name() << "':  Invalid xtsc_cohctrl port_num specified to xtsc_master::connect():  port_num=" << port_num;
    oss << ".  Valid range is 0 to " << (num_clients - 1) << ".";
    throw xtsc_exception(oss.str());
  }
  m_request_port(*cohctrl.m_client_exports[port_num]);
  (*cohctrl.m_client_ports[port_num])(m_respond_export);
}



bool xtsc_component::xtsc_master::nb_respond(const xtsc_response& response) {
  bool return_value = true;
  xtsc_response::status_t status = response.get_status();
  if (status == xtsc_response::RSP_NACC) {
    m_last_response_status = status;
    if (response.get_tag() == m_last_request_tag) {
      m_last_request_got_response = true;
      m_last_request_got_nacc = true;
    }
  }
  else {
    return_value = get_return_value();
    if (return_value) {
      m_last_response_status = status;
      if (response.get_tag() == m_last_request_tag) {
        m_last_request_got_response = true;
      }
    }
  }

  if (return_value) {
    m_received_response_event.notify(SC_ZERO_TIME);
  }

  XTSC_INFO(m_text, response << (return_value ? "" : " (nb_respond returning false)"));
  return return_value;
}



void xtsc_component::xtsc_master::request_thread(void) {

  try {

    xtsc_request request;
    double      delay             = 0;
    double      prev_req_delay    = 0;
    bool        prev_req_do_delay = false;

    while (get_words() != 0) {

      XTSC_DEBUG(m_text, "\"script_file\" line #" << m_line_count << ": " << m_line);

      bool send     = true;
      bool poke     = false;
      bool peek     = false;
      bool stop     = false;
      bool lock     = false;
      bool lock_arg = false;
      bool retire   = false;
      bool flush    = false;
      bool is_req   = false;
      bool do_delay = (m_words[0] != "now");
      xtsc_address address8 = 0;
      u32          size = 0;

      if (m_words[0] == "wait") {
        send     = false;
        do_delay = false;
        if (m_words.size() < 2) {
          ostringstream oss;
          oss << "WAIT command is missing arguments: " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        if ((m_words[1] == "rsp") || (m_words[1] == "respond") || (m_words[1] == "response") || (m_words[1] == "tag")) {
          bool is_wait_tag = (m_words[1] == "tag");
          bool has_repeat_value = (m_words.size() > 2);
          u32 repeat = 0xFFFFFFFF;
          if (has_repeat_value) {
            repeat = get_u32(2, "repeat");
          }
          if (is_wait_tag) {
            XTSC_DEBUG(m_text, "waiting for response with tag=" << request.get_tag());
          }
          else {
            XTSC_DEBUG(m_text, "waiting for any response");
          }
          do {
            wait(m_received_response_event);
          } while (is_wait_tag && !m_last_request_got_response);
          if (repeat && (m_last_response_status == xtsc_response::RSP_NACC)) {
            do {
              // delay if previous request command had a delay
              if (prev_req_do_delay) {
                sc_time retry_delay = prev_req_delay * m_clock_period;
                XTSC_DEBUG(m_text, "Got RSP_NACC. Waiting before retry: " << retry_delay);
                wait(retry_delay);
              }
              ostringstream oss;
              request.dump(oss, true);
              XTSC_INFO(m_text, oss.str());
              m_last_request_got_response = false;
              m_last_request_got_nacc = false;
              m_request_port->nb_request(request);
              if (is_wait_tag) {
                XTSC_DEBUG(m_text, "waiting for response with tag=" << request.get_tag() << " again");
              }
              else {
                XTSC_DEBUG(m_text, "waiting for any response again");
              }
              do {
                wait(m_received_response_event);
              } while (is_wait_tag && !m_last_request_got_response);
              if (has_repeat_value) {
                repeat -= 1;
              }
            } while (repeat && (m_last_response_status == xtsc_response::RSP_NACC));
          }
        }
        else if ((m_words[1] == "nacc") || (m_words[1] == "rsp_nacc")) {
          bool has_timeout_value = (m_words.size() > 2);
          double timeout_factor = 1.0;
          if (has_timeout_value) {
            timeout_factor = get_double(2, "timeout");
          }
          bool has_repeat_value = (m_words.size() > 3);
          u32 repeat = 0xFFFFFFFF;
          if (has_repeat_value) {
            repeat = get_u32(3, "repeat");
          }
          sc_time timeout = timeout_factor * m_clock_period;
          XTSC_DEBUG(m_text, "waiting for response with timeout: " << timeout);
          wait(timeout);
          if (repeat && m_last_request_got_nacc) {
            do {
              // delay if previous request command had a delay
              if (prev_req_do_delay) {
                sc_time retry_delay = prev_req_delay * m_clock_period;
                if (retry_delay > timeout) {
                  retry_delay -= timeout;
                  XTSC_DEBUG(m_text, "Got RSP_NACC. Waiting before retry: " << retry_delay);
                  wait(retry_delay);
                }
              }
              ostringstream oss;
              request.dump(oss, true);
              XTSC_INFO(m_text, oss.str());
              m_last_request_got_response = false;
              m_last_request_got_nacc = false;
              m_request_port->nb_request(request);
              XTSC_DEBUG(m_text, "waiting again for response with timeout: " << timeout);
              wait(timeout);
              if (has_repeat_value) {
                repeat -= 1;
              }
            } while (repeat && m_last_request_got_nacc);
          }
        }
        else if (m_words[1] == "control") {
          if (!m_control) {
            ostringstream oss;
            oss << "WAIT CONTROL command cannot be used unless the \"control\" parameter is set to true:" << endl;
            oss << m_line;
            oss << m_script_file_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
          if (!m_control_bound) {
            ostringstream oss;
            oss << "WAIT CONTROL command cannot be used unless something is connected to the control input:" << endl;
            oss << m_line;
            oss << m_script_file_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
          if (m_words.size() <= 4) {
            u32 count = 1;
            if (m_words.size() == 4) {
              count = get_u32(3, "count");
            }
            if ((m_words.size() == 2) || (m_words[2] == "write")) {
              u32 target_write_count = m_control_write_count + count;
              while (target_write_count != m_control_write_count) {
                wait(m_control_write_event);
              }
            }
            else if (m_words[2] == "change") {
              u32 target_change_count = m_control_change_count + count;
              while (target_change_count != m_control_change_count) {
                wait(m_control_write_event);
              }
            }
            else {
              u32 value = get_u32(2, "value");
              if (value > 1) {
                ostringstream oss;
                oss << "value = " << value << " is not allowed (must be 0 or 1) in command:" << endl;
                oss << m_line;
                oss << m_script_file_stream.info_for_exception();
                throw xtsc_exception(oss.str());
              }
              if (m_control_value.to_uint() == value) { count -= 1; }
              while (count) {
                wait(m_control_write_event);
                if (m_control_value.to_uint() == value) { count -= 1; }
              }
            }
          }
        }
        else {
          double time = get_double(1, "duration");
          sc_time duration = time * m_clock_period;
          XTSC_DEBUG(m_text, "waiting " << duration);
          wait(duration);
        }
      }
      else if ((m_words[0] == "sync") || (m_words[0] == "synchronize")) {
        send     = false;
        do_delay = false;
        if (m_words.size() != 2) {
          ostringstream oss;
          oss << "SYNC command has missing/extra arguments: " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        double time = get_double(1, "time");
        if (time < 1.0) {
          sc_time sync_phase = time * m_clock_period;
          sc_time now = sc_time_stamp();
          sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
          if (m_has_posedge_offset) {
            if (phase_now < m_posedge_offset) {
              phase_now += m_clock_period;
            }
            phase_now -= m_posedge_offset;
          }
          XTSC_DEBUG(m_text, "sync_phase=" << sync_phase << " phase_now=" << phase_now);
          sc_time delta = ((sync_phase >= phase_now) ? (sync_phase - phase_now) : (m_clock_period + sync_phase - phase_now));
          if (delta != SC_ZERO_TIME) {
            XTSC_DEBUG(m_text, "waiting " << delta << " to sync to phase " << sync_phase);
            wait(delta);
          }
        }
        else {
          sc_time absolute_time = time * m_clock_period;
          sc_time now = sc_time_stamp();
          if (absolute_time > now) {
            sc_time delta = absolute_time - now;
            XTSC_DEBUG(m_text, "waiting " << delta << " to sync to time " << absolute_time);
            wait(delta);
          }
        }
      }
      else if (m_words[0] == "fetch") {
        send     = false;
        do_delay = false;
        if ((m_words.size() != 2) || ((m_words[1] != "on") && (m_words[1] != "off"))) {
          ostringstream oss;
          oss << "Syntax error (expected FETCH ON|OFF): " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        m_fetch = (m_words[1] == "on");
      }
      else if (m_words[0] == "coherent") {
        send     = false;
        do_delay = false;
        if ((m_words.size() != 2) || ((m_words[1] != "on") && (m_words[1] != "off"))) {
          ostringstream oss;
          oss << "Syntax error (expected COHERENT ON|OFF): " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        m_use_coherent_peek_poke = (m_words[1] == "on");
      }
      else if (m_words[0] == "attribute") {
        send     = false;
        do_delay = false;
        if (m_words.size() != 2) {
          ostringstream oss;
          oss << "Syntax error (expected ATTRIBUTE attr|OFF): " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        m_pif_attribute = (m_words[1] == "off") ? 0xFFFFFFFF : get_u32(1, "attr");
      }
      else if (m_words[0] == "xfer_en") {
        send     = false;
        do_delay = false;
        if ((m_words.size() != 2) || ((m_words[1] != "on") && (m_words[1] != "off"))) {
          ostringstream oss;
          oss << "Syntax error (expected XFER_EN ON|OFF): " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        m_set_xfer_en = (m_words[1] == "on");
      }
      else if (m_words[0] == "format") {
        send     = false;
        do_delay = false;
        if ((m_words.size() != 2) || ((m_words[1] != "1") && (m_words[1] != "2"))) {
          ostringstream oss;
          oss << "Syntax error (expected FORMAT 1|2): " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        m_format = ((m_words[1] == "1") ? 1 : 2);
      }
      else if (m_words[0] == "info") {
        send     = false;
        do_delay = false;
        XTSC_INFO(m_text, m_line);
      }
      else if (m_words[0] == "note") {
        send     = false;
        do_delay = false;
        XTSC_NOTE(m_text, m_line);
      }
      else if (m_words[0] == "virtual") {
        send     = false;
        do_delay = false;
        if (m_words.size() != 2) {
          ostringstream oss;
          oss << "Syntax error (expected VIRTUAL addr_delta): " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        m_virtual_address_delta = get_u32(1, "addr_delta");
      }
      else {
        if (m_words.size() == 1) {
          ostringstream oss;
          oss << "Line has too few words: " << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        if (m_words[1] == "stop") {
          // delay STOP
          send = false;
          stop = true;
        }
        else if (m_words[1] == "lock") {
          // delay LOCK lock
          if (m_words.size() < 3) {
            ostringstream oss;
            oss << "Line has too few words: " << endl;
            oss << m_line;
            oss << m_script_file_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
          if ((m_words[2] == "true") || (m_words[2] == "t") || (m_words[2] == "1") || (m_words[2] == "on")) {
            lock_arg = true;
          }
          else if ((m_words[2] == "false") || (m_words[2] == "f") || (m_words[2] == "0") || (m_words[2] == "off")) {
            lock_arg = false;
          }
          send = false;
          lock = true;
        }
        else if (m_words[1] == "retire") {
          // delay RETIRE address
          if (m_words.size() < 3) {
            ostringstream oss;
            oss << "Line has too few words: " << endl;
            oss << m_line;
            oss << m_script_file_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
          address8 = get_u32(2, "address");
          send = false;
          retire = true;
        }
        else if (m_words[1] == "flush") {
          // delay FLUSH
          send = false;
          flush = true;
        }
        else {
          address8 = get_u32(2, "address");
          size     = get_u32(3, "size");
          if (m_words[1] == "poke") {
            // delay POKE address size b0 b1 ... bN
            set_buffer(4, size, true);
            send = false;
            poke = true;
          }
          else if (m_words[1] == "peek") {
            // delay PEEK address size 
            send = false;
            peek = true;
          }
          else if (m_words[1] == "read") {
            // 0     1    2       3    4        5  6        7  8
            // delay READ address size route_id id priority pc
            // delay READ address size route_id id priority pc coh
            u32          route_id = get_u32(4, "route_id");
            u8           id       = static_cast<u8>(get_u32(5, "id"));
            u8           priority = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address pc       = get_u32(7, "pc");
            u32          offset   = 0;
            coherence_t  coh      = optionally_get_coherence(8, offset);
            xtsc_byte_enables be  = (0xFFFFFFFFFFFFFFFFull >> (64 - size));
            request.initialize(xtsc_request::READ, address8, size, 0, 1, be , true, route_id, id, priority, pc);
            request.set_coherence(coh);
            request.set_instruction_fetch(m_fetch);
            check_for_too_many_parameters(8 + offset);
            is_req = true;
          }
          else if (m_words[1] == "block_read") {
            // 0     1          2       3    4        5  6        7  8         9
            // delay BLOCK_READ address size route_id id priority pc num_xfers 
            // delay BLOCK_READ address size route_id id priority pc coh       num_xfers 
            u32          route_id      = get_u32(4, "route_id");
            u8           id            = static_cast<u8>(get_u32(5, "id"));
            u8           priority      = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address pc            = get_u32(7, "pc");
            u32          offset        = 0;
            coherence_t  coh           = optionally_get_coherence(8, offset);
            u32          num_xfers     = get_u32(8+offset, "num_xfers");
            request.initialize(xtsc_request::BLOCK_READ, address8, size, 0, num_xfers, 0, true,
                               route_id, id, priority, pc);
            request.set_coherence(coh);
            request.set_instruction_fetch(m_fetch);
            check_for_too_many_parameters(9 + offset);
            is_req = true;
          }
          else if (m_words[1] == "burst_read") {
            // 0     1          2       3    4        5  6        7  8         9
            // delay BURST_READ address size route_id id priority pc num_xfers 
            // delay BURST_READ address size route_id id priority pc coh       num_xfers 
            u32          route_id      = get_u32(4, "route_id");
            u8           id            = static_cast<u8>(get_u32(5, "id"));
            u8           priority      = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address pc            = get_u32(7, "pc");
            u32          offset        = 0;
            coherence_t  coh           = optionally_get_coherence(8, offset);
            u32          num_xfers     = get_u32(8+offset, "num_xfers");
            request.initialize(xtsc_request::BURST_READ, address8, size, 0, num_xfers, (0xFFFFFFFFFFFFFFFFull >> (64 - size)), true,
                               route_id, id, priority, pc);
            request.set_coherence(coh);
            check_for_too_many_parameters(9 + offset);
            is_req = true;
          }
          else if (m_words[1] == "write") {
            // 0     1     2       3    4        5  6        7  8            9            10
            // delay WRITE address size route_id id priority pc byte_enables b0           b1 ... bN
            // delay WRITE address size route_id id priority pc coh          byte_enables b0 b1 ... bN
            u32               route_id = get_u32(4, "route_id");
            u8                id       = static_cast<u8>(get_u32(5, "id"));
            u8                priority = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address      pc       = get_u32(7, "pc");
            u32               offset   = 0;
            coherence_t       coh      = optionally_get_coherence(8, offset);
            xtsc_byte_enables b        = get_u64(8+offset, "byte_enables");
            request.initialize(xtsc_request::WRITE, address8, size, 0, 1, b, true, route_id, id, priority, pc);
            set_buffer(request, 9+offset, size);
            request.set_coherence(coh);
            check_for_too_many_parameters(9+offset+size);
            check_for_too_few_parameters(9+offset+size);
            is_req = true;
          }
          else if (m_words[1] == "block_write") {
            // 0     1           2       3    4        5  6        7  8         8         9          10         11 
            // delay BLOCK_WRITE address size route_id id priority pc num_xfers last_xfer first_xfer b0         b1 ... bN
            // delay BLOCK_WRITE address size route_id id priority pc coh       num_xfers last_xfer  first_xfer b0 b1 ... bN
            u32          route_id      = get_u32(4, "route_id");
            u8           id            = static_cast<u8>(get_u32(5, "id"));
            u8           priority      = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address pc            = get_u32(7, "pc");
            u32          offset        = 0;
            coherence_t  coh           = optionally_get_coherence(8, offset);
            u32          num_xfers     = get_u32(8+offset, "num_xfers");
            bool         last          = (get_u32(9+offset, "last_xfer") != 0);
            bool         first         = (get_u32(10+offset, "first_xfer") != 0);
            if (first) {
              request.initialize(xtsc_request::BLOCK_WRITE, address8, size, 0, num_xfers, 0, last,
                                 route_id, id, priority, pc);
              m_block_write_tag = request.get_tag();
            }
            else {
              request.initialize(m_block_write_tag, address8, size, num_xfers, last, route_id, id, priority, pc);
            }
            set_buffer(request, 11+offset, size);
            request.set_coherence(coh);
            check_for_too_many_parameters(11+offset+size);
            check_for_too_few_parameters(11+offset+size);
            is_req = true;
          }
          else if (m_words[1] == "burst_write") {
            // 0     1           2       3    4        5  6        7  8         9         10           11           12         13
            // delay BURST_WRITE address size route_id id priority pc num_xfers xfer_num  byte_enables hw_address   b0         b1 ... bN
            // delay BURST_WRITE address size route_id id priority pc coh       num_xfers xfer_num     byte_enables hw_address b0 b1 ... bN
            u32                 route_id        = get_u32(4, "route_id");
            u8                  id              = static_cast<u8>(get_u32(5, "id"));
            u8                  priority        = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address        pc              = get_u32(7, "pc");
            u32                 offset          = 0;
            coherence_t         coh             = optionally_get_coherence(8, offset);
            u32                 num_xfers       = get_u32(8+offset, "num_xfers");
            u32                 xfer_num        = get_u32(9+offset, "xfer_num");
            xtsc_byte_enables   b               = get_u64(10+offset, "byte_enables");
            xtsc_address        hw_address      = get_u32(11+offset, "hw_address");
            if (!xfer_num || (xfer_num > num_xfers)) {
              ostringstream oss;
              oss << "Invalid xfer_num found in BURST_WRITE (transfers are numbered between 1 and num_xfers): " << endl;
              oss << m_line;
              oss << m_script_file_stream.info_for_exception();
              throw xtsc_exception(oss.str());
            }
            if (xfer_num == 1) {
              request.initialize(xtsc_request::BURST_WRITE, address8, size, 0, num_xfers, b, false, route_id, id, priority, pc);
              m_burst_write_tag = request.get_tag();
            }
            else {
              request.initialize(hw_address, m_burst_write_tag, address8, size, num_xfers, xfer_num, b, route_id, id, priority, pc);
            }
            set_buffer(request, 12+offset, size);
            request.set_coherence(coh);
            check_for_too_many_parameters(12+offset+size);
            check_for_too_few_parameters(12+offset+size);
            is_req = true;
          }
          else if (m_words[1] == "rcw") {
            // 0     1   2       3    4        5  6        7  8         9         10        11           12
            // delay RCW address size route_id id priority pc num_xfers last_xfer b0        b1           b2 b3
            // delay RCW address size route_id id priority pc coh       num_xfers last_xfer byte_enables b0 b1 b2 b3
            u32                 route_id      = get_u32(4, "route_id");
            u8                  id            = static_cast<u8>(get_u32(5, "id"));
            u8                  priority      = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address        pc            = get_u32(7, "pc");
            u32                 offset        = 0;
            coherence_t         coh           = optionally_get_coherence(8, offset);
            u32                 num_xfers     = get_u32(8+offset, "num_xfers");
            bool                last          = (get_u32(9+offset, "last_xfer") != 0);
            xtsc_byte_enables   b             = ((m_format > 1) ? get_u64(11, "byte_enables") : (0xFFFFFFFFFFFFFFFFull >> (64 - size)));
            if (last) {
              request.initialize(m_rcw_tag, address8, route_id, id, priority, pc);
              request.set_byte_enables(b);
              request.set_byte_size(size);
            }
            else {
              request.initialize(xtsc_request::RCW, address8, size, 0, num_xfers, b, false, route_id, id, priority, pc);
              m_rcw_tag = request.get_tag();
            }
            set_buffer(request, 10+offset*2, size);
            request.set_coherence(coh);
            check_for_too_many_parameters(10+offset*2+size);
            check_for_too_few_parameters(10+offset*2+size);
            is_req = true;
          }
          else if (m_words[1] == "snoop") {
            // 0     1     2       3    4        5  6        7  8             9
            // delay SNOOP address size route_id id priority pc num_transfers 
            // delay SNOOP address size route_id id priority pc coh           num_transfers 
            u32          route_id      = get_u32(4, "route_id");
            u8           id            = static_cast<u8>(get_u32(5, "id"));
            u8           priority      = static_cast<u8>(get_u32(6, "priority"));
            xtsc_address pc            = get_u32(7, "pc");
            u32          offset        = 0;
            coherence_t  coh           = optionally_get_coherence(8, offset);
            u32          num_transfers = get_u32(8+offset, "num_transfers");
            request.initialize(xtsc_request::SNOOP, address8, size, 0, num_transfers, 0, true,
                               route_id, id, priority, pc);
            request.set_coherence(coh);
            check_for_too_many_parameters(9 + offset);
            is_req = true;
          }
          else {
            ostringstream oss;
            oss << "Unrecognized request type '" << m_words[1] << "':" << endl;
            oss << m_line;
            oss << m_script_file_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
        }
        if (is_req) {
          request.set_snoop_virtual_address(address8+m_virtual_address_delta);
          if (m_set_xfer_en) {
            request.set_xfer_en(true);
          }
        }
      }


      if (do_delay) {
          delay = get_double(0, "delay");
          XTSC_DEBUG(m_text, "waiting for " << (delay * m_clock_period));
          wait(delay * m_clock_period);
      }
      if (send) {
        if (m_pif_attribute != 0xFFFFFFFF) {
          request.set_pif_attribute(m_pif_attribute);
        }
        ostringstream oss;
        request.dump(oss, true);
        XTSC_INFO(m_text, oss.str());
        m_last_request_tag = request.get_tag();
        m_last_request_got_response = false;
        m_last_request_got_nacc = false;
        m_request_port->nb_request(request);
      }
      if (poke) {
        ostringstream oss;
        oss << "poke 0x" << hex << setfill('0') << setw(8) << address8 << setfill(' ') << ": ";
        xtsc_hex_dump(true, size, m_buffer, oss);
        if (m_use_coherent_peek_poke) {
          oss << " virtual=0x" << hex << setfill('0') << setw(8) << (address8+m_virtual_address_delta);
        }
        XTSC_INFO(m_text, oss.str());
        if (m_use_coherent_peek_poke) {
          m_request_port->nb_poke_coherent(address8+m_virtual_address_delta, address8, size, m_buffer);
        }
        else {
          m_request_port->nb_poke(address8, size, m_buffer);
        }
      }
      if (peek) {
        if (size > m_buffer_size) {
          ostringstream oss;
          oss << "Peek size of " << size << " exceeds maximum peek size of " << m_buffer_size << ":" << endl;
          oss << m_line;
          oss << m_script_file_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        if (m_use_coherent_peek_poke) {
          m_request_port->nb_peek_coherent(address8+m_virtual_address_delta, address8, size, m_buffer);
        }
        else {
          m_request_port->nb_peek(address8, size, m_buffer);
        }
        ostringstream oss;
        oss << "peek 0x" << hex << setfill('0') << setw(8) << address8 << setfill(' ') << ": ";
        xtsc_hex_dump(true, size, m_buffer, oss);
        if (m_use_coherent_peek_poke) {
          oss << " virtual=0x" << hex << setfill('0') << setw(8) << (address8+m_virtual_address_delta);
        }
        XTSC_INFO(m_text, oss.str());
      }
      if (stop) {
        sc_stop();
        wait();
      }
      if (lock) {
        XTSC_INFO(m_text, "nb_lock(" << boolalpha << lock_arg << ")");
        m_request_port->nb_lock(lock_arg);
      }
      if (retire) {
        XTSC_INFO(m_text, "nb_load_retired(0x" << hex << setfill('0') << setw(8) << address8 << ")");
        m_request_port->nb_load_retired(address8);
      }
      if (flush) {
        XTSC_INFO(m_text, "nb_retire_flush()");
        m_request_port->nb_retire_flush();
      }

      if (is_req) {
        prev_req_delay    = delay;
        prev_req_do_delay = do_delay;
      }
    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in SC_THREAD of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, log4xtensa::FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



int xtsc_component::xtsc_master::get_words() {
  m_line_count = m_script_file_stream.get_words(m_words, m_line, true);
  return m_words.size();
}




u32 xtsc_component::xtsc_master::get_u32(u32 index, const string& argument_name) {
  u32 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in command:" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtou32(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number in:" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



u64 xtsc_component::xtsc_master::get_u64(u32 index, const string& argument_name) {
  u64 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in command:" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtou64(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number in:" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



double xtsc_component::xtsc_master::get_double(u32 index, const string& argument_name) {
  double value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing in command:" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtod(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number:" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



xtsc::xtsc_request::coherence_t xtsc_component::xtsc_master::optionally_get_coherence(u32 index, u32& offset) {
  xtsc_request::coherence_t coherence = xtsc_request::INVALID;
  offset = 0;
  if (m_format > 1) {
    offset = 1;
    if (index >= m_words.size()) {
      ostringstream oss;
      oss << "Too few arguments in request: " << m_line << endl;
      m_script_file_stream.dump_last_line_info(oss);
      throw xtsc_exception(oss.str());
    }
    try {
      u32 value = xtsc_strtou32(m_words[index]);
      coherence = static_cast<xtsc_request::coherence_t>(value);
    }
    catch (const xtsc_exception&) {
      ostringstream oss;
      oss << "Cannot convert coh argument (#" << index+1 << ") '" << m_words[index] << "' to number:" << endl;
      oss << m_line;
      oss << m_script_file_stream.info_for_exception();
      throw xtsc_exception(oss.str());
    }
    if (coherence > xtsc_request::EXCLUSIVE) {
      ostringstream oss;
      oss << "coh argument (#" << index+1 << ") '" << m_words[index] << "' out-of-range:" << endl;
      oss << m_line;
      oss << m_script_file_stream.info_for_exception();
      throw xtsc_exception(oss.str());
    }
  }
  return coherence;
}



void xtsc_component::xtsc_master::check_for_too_many_parameters(u32 number_expected) {
  if (m_words.size() > number_expected) {
    ostringstream oss;
    oss << "xtsc_master '" << name() << "' found too many words: exp=" << number_expected << " has=" << m_words.size() << ": ";
    m_script_file_stream.dump_last_line_info(oss);
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_master::check_for_too_few_parameters(u32 number_expected) {
  if (m_words.size() < number_expected) {
    ostringstream oss;
    oss << "xtsc_master '" << name() << "' found too few words: exp=" << number_expected << " has=" << m_words.size() << ": ";
    m_script_file_stream.dump_last_line_info(oss);
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_master::set_buffer(u32 index, u32 size8, bool is_poke) {
  XTSC_DEBUG(m_text, "set_buffer: index=" << index << " size8=" << size8 << " m_words.size()=" << m_words.size());
  if (size8 > m_buffer_size) {
    ostringstream oss;
    oss << "xtsc_master '" << name() << "' line " << m_line_count << " requires buffer size of " << size8
        << " which exceeds maximum buffer size of " << m_buffer_size;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  memset(m_buffer, 0, xtsc_max_bus_width8);
  if (size8 > xtsc_max_bus_width8) {
    if (!is_poke) {
      ostringstream oss;
      oss << "Size (" << size8 << ") exceeds maximum bus width (" << xtsc_max_bus_width8 << "):" << endl;
      oss << m_line;
      oss << m_script_file_stream.info_for_exception();
      throw xtsc_exception(oss.str());
    }
    memset(m_buffer, 0, size8);
  }
  u32 size = m_words.size();
  for (u32 i=0; i<size8; ++i) {
    if (index+i >= size) break;
    u32 value = get_u32(index+i, "data");
    m_buffer[i] = static_cast<u8>(value);
  }
}



void xtsc_component::xtsc_master::set_buffer(xtsc_request& request, u32 index, u32 size8) {
  XTSC_DEBUG(m_text, "set_buffer: index=" << index << " size8=" << size8 << " m_words.size()=" << m_words.size());
  u8 buffer[xtsc_max_bus_width8];
  memset(buffer, 0, xtsc_max_bus_width8);
  if (size8 > xtsc_max_bus_width8) {
    ostringstream oss;
    oss << "Size (" << size8 << ") exceeds maximum bus width (" << xtsc_max_bus_width8 << "):" << endl;
    oss << m_line;
    oss << m_script_file_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  u32 size = m_words.size();
  for (u32 i=0; i<size8; ++i) {
    if (index+i >= size) break;
    u32 value = get_u32(index+i, "data");
    buffer[i] = static_cast<u8>(value);
  }
  request.set_buffer(size8, buffer);
}



bool xtsc_component::xtsc_master::get_return_value() {
  bool result = true;
  if (m_return_value_file != "") {
    if (m_return_value_index >= m_return_values.size()) {
      m_return_value_line_count = m_p_return_value_file->get_words(m_return_values, m_return_value_line, true);
      m_return_value_index = 0;
    }
    string value = m_return_values[m_return_value_index];
    if ((value == "0") || (value == "false") || (value == "f")) {
      result = false;
    }
    else if ((value == "1") || (value == "true") || (value == "t")) {
      result = true;
    }
    else {
      ostringstream oss;
      oss << "Invalid return value '" << value << "' at word #" << (m_return_value_index + 1) << ":" << endl;
      oss << m_return_value_line;
      oss << m_p_return_value_file->info_for_exception();
      throw xtsc_exception(oss.str());
    }
    m_return_value_index += 1;
  }
  return result;
}



void xtsc_component::xtsc_master::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_master '" << name() << "' m_respond_export: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_text, "Binding '" << port.name() << "' to xtsc_master::m_respond_export");
  m_p_port = &port;
}



void xtsc_component::xtsc_master::xtsc_wire_write_if_impl::nb_write(const sc_unsigned& value) {
  if (static_cast<u32>(value.length()) != m_bit_width) {
    ostringstream oss;
    oss << "ERROR: Value of width=" << value.length() << " bits written to sc_export \"" << m_master.m_p_control->name()
        << "\" of width=" << m_bit_width << " in xtsc_master '" << m_master.name() << "'";
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_master.m_text, m_master.m_p_control->name() << " <= " << value.to_string(SC_HEX));
  m_master.m_control_write_count += 1;
  if (value != m_master.m_control_value) {
    m_master.m_control_value = value;
    m_master.m_control_change_count += 1;
  }
  m_master.m_control_write_event.notify(SC_ZERO_TIME);
}



void xtsc_component::xtsc_master::xtsc_wire_write_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to sc_export<xtsc_wire_write_if> \"" << m_master.m_p_control->name()
        << "\" of xtsc_master '" << m_master.name() << "'" << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_master.m_text, "Binding '" << port.name() << "' to sc_export<xtsc_wire_write_if> \"" << m_master.m_p_control->name() <<
                             "\" of xtsc_master '" << m_master.name() << "'");
  m_p_port = &port;
  m_master.m_control_bound = true;
}




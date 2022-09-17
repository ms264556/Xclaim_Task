// Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <cctype>
#include <algorithm>
#include <xtsc/xtsc_queue_producer.h>
#include <xtsc/xtsc_tx_loader.h>
#include <xtsc/xtsc_mmio.h>
#include <xtsc/xtsc_wire_logic.h>


using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::VERBOSE_LOG_LEVEL;



xtsc_component::xtsc_queue_producer::xtsc_queue_producer(sc_module_name module_name, const xtsc_queue_producer_parms& producer_parms) :
  sc_module             (module_name),
  m_push                ("m_push"),
  m_data                ("m_data"),
  m_full                ("m_full"),
  m_queue               ("m_queue"),
  m_text                (log4xtensa::TextLogger::getInstance(name())),
  m_control             (producer_parms.get_bool("control")),
  m_control_bound       (false),
  m_p_control           (NULL),
  m_p_write_impl        (NULL),
  m_control_value       (1),
  m_test_vector_stream  (producer_parms.get_c_str("script_file"), "script_file",  name(), kind(), false),
  m_script_file         (producer_parms.get_c_str("script_file")),
  m_width1              (producer_parms.get_non_zero_u32("bit_width")),
  m_pin_level           (producer_parms.get_bool("pin_level")),
  m_value               (m_width1),
  m_value_bv            ((int)m_width1),
  m_zero                (1),
  m_zero_bv             (1),
  m_one                 (1),
  m_one_bv              (1),
  m_assert_delta_cycle  (0xFFFFFFFFFFFFFFFFull),
  m_push_floating       ("m_push_floating", 1, m_text),
  m_data_floating       ("m_data_floating", producer_parms.get_non_zero_u32("bit_width"), m_text),
  m_full_floating       ("m_full_floating", 1, m_text)
{

  m_zero        = 0;
  m_zero_bv     = 0;
  m_one         = 1;
  m_one_bv      = 1;

  if (m_control) {
    m_p_control = new wire_write_export("control");
    m_p_write_impl = new xtsc_wire_write_if_impl("control__impl", *this);
    (*m_p_control)(*m_p_write_impl);
  }
  m_control_value = 0;

  // Get clock period 
  m_time_resolution = sc_get_time_resolution();
  u32 clock_period = producer_parms.get_non_zero_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = sc_get_time_resolution() * clock_period;
  }
  m_clock_period_value = m_clock_period.value();
  m_previous_push = SC_ZERO_TIME - m_clock_period;
  u32 posedge_offset = producer_parms.get_u32("posedge_offset");
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

  // Get clock phase when the full signal is to be sampled
  u32 sample_phase = producer_parms.get_u32("sample_phase");
  if (sample_phase >= m_clock_period.value()) {
    ostringstream oss;
    oss << "xtsc_queue_producer '" << name() << "' parameter error:" << endl;
    oss << "  \"sample_phase\" (" << sample_phase
        << ") must be strictly less than \"clock_period\" (";
    if (clock_period == 0xFFFFFFFF) {
      oss << "0xFFFFFFFF => " << m_clock_period.value();
    }
    else {
      oss << clock_period;
    }
    oss << ")";
    throw xtsc_exception(oss.str());
  }
  m_sample_phase = sc_get_time_resolution() * sample_phase;

  // Get how long after the full signal is sampled that the push signal should be deasserted
  u32 deassert_delay = producer_parms.get_u32("deassert_delay");
  if (deassert_delay >= m_clock_period.value()) {
    ostringstream oss;
    oss << "  \"deassert_delay\" (" << deassert_delay
        << ") must be strictly less than \"clock_period\" (";
    if (clock_period == 0xFFFFFFFF) {
      oss << "0xFFFFFFFF => " << m_clock_period.value();
    }
    else {
      oss << clock_period;
    }
    oss << ")";
    throw xtsc_exception(oss.str());
  }
  m_deassert_delay = sc_get_time_resolution() * deassert_delay;


  SC_THREAD(script_thread);
  if (m_pin_level) {
    m_p_trace_file = static_cast<sc_trace_file*>(const_cast<void*>(producer_parms.get_void_pointer("vcd_handle")));
    SC_THREAD(sample_thread);
    SC_THREAD(request_thread);
    m_queue(*this);
    if (m_p_trace_file) {
      sc_trace(m_p_trace_file, m_push, m_push.name());
      sc_trace(m_p_trace_file, m_data, m_data.name());
      sc_trace(m_p_trace_file, m_full, m_full.name());
    }
  }
  else {
    m_p_trace_file = NULL;
    m_push(m_push_floating);
    m_full(m_full_floating);
    m_data(m_data_floating);
  }


  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, "Constructed xtsc_queue_producer '" << name() << "':");
  XTSC_LOG(m_text, ll, " control                 = "   << boolalpha << m_control);
  XTSC_LOG(m_text, ll, " pin_level               = "   << boolalpha << m_pin_level);
  if (m_pin_level) {
  XTSC_LOG(m_text, ll, " vcd_handle              = "   << m_p_trace_file);
  }
  XTSC_LOG(m_text, ll, " script_file             = "   << m_script_file);
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
  XTSC_LOG(m_text, ll, " sample_phase            = "   << sample_phase << " (" << m_sample_phase << ")");
  XTSC_LOG(m_text, ll, " deassert_delay          = "   << deassert_delay << " (" << m_deassert_delay << ")");

  reset(true);

}



xtsc_component::xtsc_queue_producer::~xtsc_queue_producer(void) {
}



void xtsc_component::xtsc_queue_producer::reset(bool /*hard_reset*/) {
  XTSC_INFO(m_text, "xtsc_queue_producer::reset()");

  m_words.clear();
  m_line        = "";
  m_line_count  = 0;
  m_no_timeout  = false;
  m_test_vector_stream.reset();

}



sc_export<xtsc_wire_write_if>& xtsc_component::xtsc_queue_producer::get_control_input() const {
  if (!m_control) {
    ostringstream oss;
    oss << "xtsc_queue_producer '" << name() << "' has \"control\" false, so get_control_input() should not be called.";
    throw xtsc_exception(oss.str());
  }
  return *m_p_control;
}



void xtsc_component::xtsc_queue_producer::connect(xtsc_tx_loader& loader) {
  if (loader.pin_level() || m_pin_level) {
    ostringstream oss;
    oss << "xtsc_tx_loader '" << loader.name() << "' has \"pin_level\" " << boolalpha << loader.pin_level()
        << " and " << kind() << " '" << name() << "' has \"pin_level\" " << m_pin_level
        << ", but this connect() method does not support pin-level connections.";
    throw xtsc_exception(oss.str());
  }
  m_queue(*loader.m_producer);
}



void xtsc_component::xtsc_queue_producer::connect(xtsc_wire_logic& logic, const char *output_name) {
  if (!m_control) {
    ostringstream oss;
    oss << "'" << name() << "' has \"control\" false, so xtsc_queue_producer::connect(xtsc_wire_logic&, ...) should not be called.";
    throw xtsc_exception(oss.str());
  }
  u32 wo = logic.get_bit_width(output_name);
  u32 wi = 1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_wire_logic '" << logic.name()
        << "' to " << wi << "-bit control input of xtsc_queue_producer '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  logic.get_output(output_name)(*m_p_control);
}



void xtsc_component::xtsc_queue_producer::connect(xtsc_mmio& mmio, const char *output_name) {
  if (!m_control) {
    ostringstream oss;
    oss << "'" << name() << "' has \"control\" false, so xtsc_queue_producer::connect(xtsc_mmio&, ...) should not be called.";
    throw xtsc_exception(oss.str());
  }
  u32 wo = mmio.get_bit_width(output_name);
  u32 wi = 1;
  if (wo != wi) {
    ostringstream oss;
    oss << "Width mismatch:  cannot connect " << wo << "-bit output '" << output_name << "' of xtsc_mmio '" << mmio.name()
        << "' to " << wi << "-bit control input of xtsc_queue_producer '" << name() << "'";
    throw xtsc_exception(oss.str());
  }
  mmio.get_output(output_name)(*m_p_control);
}



void xtsc_component::xtsc_queue_producer::script_thread(void) {

  try {

    while ((m_line_count = m_test_vector_stream.get_words(m_words, m_line, true)) != 0) {

      XTSC_DEBUG(m_text, "\"script_file\" line #" << m_line_count << ": " << m_line);

      if (m_words[0] == "wait") {
        if ((m_words.size() >= 2) && (m_words[1] == "control")) {
          if (!m_control) {
            ostringstream oss;
            oss << "WAIT CONTROL command cannot be used unless the \"control\" parameter is set to true:" << endl;
            oss << m_line;
            oss << m_test_vector_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
          if (!m_control_bound) {
            ostringstream oss;
            oss << "WAIT CONTROL command cannot be used unless something is connected to the control input:" << endl;
            oss << m_line;
            oss << m_test_vector_stream.info_for_exception();
            throw xtsc_exception(oss.str());
          }
          if (m_words.size() <= 4) {
            u32 count = 1;
            if (m_words.size() == 4) {
              count = get_u32(3, "<count>");
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
              u32 value = get_u32(2, "<value>");
              if (value > 1) {
                ostringstream oss;
                oss << "<value> = " << value << " is not allowed (must be 0 or 1) in command:" << endl;
                oss << m_line;
                oss << m_test_vector_stream.info_for_exception();
                throw xtsc_exception(oss.str());
              }
              if (m_control_value.to_uint() == value) { count -= 1; }
              while (count) {
                wait(m_control_write_event);
                if (m_control_value.to_uint() == value) { count -= 1; }
              }
            }
            continue;
          }
        }
        if (m_words.size() != 2) {
          ostringstream oss;
          oss << "WAIT command has missing/extra/invalid arguments:" << endl;
          oss << m_line;
          oss << m_test_vector_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
        double time = get_double(1, "duration");
        sc_time duration = time * m_clock_period;
        XTSC_DEBUG(m_text, "waiting " << duration);
        wait(duration);
        continue;
      }

      if ((m_words[0] == "sync") || (m_words[0] == "synchronize")) {
        if (m_words.size() != 2) {
          ostringstream oss;
          oss << "SYNC command has missing/extra arguments:" << endl;
          oss << m_line;
          oss << m_test_vector_stream.info_for_exception();
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
        continue;
      }

      if (m_words[0] == "note") {
        XTSC_NOTE(m_text, m_line);
        continue;
      }

      if (m_words[0] == "info") {
        XTSC_INFO(m_text, m_line);
        continue;
      }

      u32 vi = ((m_words.size() == 1) ? 0 : 1); // Index of <value>

      if (m_words[0] == "now") {
        if (vi == 0) {
          ostringstream oss;
          oss << "Command has missing arguments:" << endl;
          oss << m_line;
          oss << m_test_vector_stream.info_for_exception();
          throw xtsc_exception(oss.str());
        }
      }
      else if (vi == 0) {
        // Sync to request phase
        sc_time sync_phase = m_request_phase;
        sc_time now = sc_time_stamp();
        sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
        if (m_has_posedge_offset) {
          if (phase_now < m_posedge_offset) {
            phase_now += m_clock_period;
          }
          phase_now -= m_posedge_offset;
        }
        if (m_previous_push == now) {
          sync_phase += m_clock_period;
        }
        XTSC_DEBUG(m_text, "sync_phase=" << sync_phase << " phase_now=" << phase_now);
        sc_time delta = ((sync_phase >= phase_now) ? (sync_phase - phase_now) : (m_clock_period + sync_phase - phase_now));
        if (delta != SC_ZERO_TIME) {
          XTSC_DEBUG(m_text, "waiting " << delta << " to sync to phase " << sync_phase);
          wait(delta);
        }
      }
      else {
          double delay = get_double(0, "delay");
          XTSC_DEBUG(m_text, "script_thread is delaying for " << delay << " clock periods");
          wait(delay * m_clock_period);
          XTSC_DEBUG(m_text, "script_thread done with delay");
      }

      if (vi && m_words[1] == "stop") {
        XTSC_INFO(m_text, "script_thread calling sc_stop()");
        sc_stop();
        wait();
      }

      // Prevent sign extension that sc_unsigned does when assigned a hex string with the high bit set
      string word_no_sign_extend = m_words[vi];
      if ((word_no_sign_extend.size() > 2) && (word_no_sign_extend.substr(0,2) == "0x")) {
        word_no_sign_extend.insert(2, "0");
      }
      try {
        m_value = word_no_sign_extend.c_str();
      }
      catch (...) {
        ostringstream oss;
        oss << "Cannot convert <value> argument (#" << (vi+1) << ") '" << m_words[vi] << "' to number:" << endl;
        oss << m_line;
        oss << m_test_vector_stream.info_for_exception();
        throw xtsc_exception(oss.str());
      }
      m_value_bv = m_value;
      double timeout = ((m_words.size() <= 2) ? -1 : get_double(2, "timeout"));
      m_no_timeout =  (timeout < 0) ? true : false;

      if (m_pin_level) {
        // pin-level interface
        XTSC_DEBUG(m_text, "script_thread is notifying m_assert");
        m_assert_delta_cycle = sc_delta_count();
        m_assert.notify();      // Immediate notification
        m_previous_push = sc_time_stamp();
        if (!m_no_timeout) {
          m_deassert.notify(timeout * m_clock_period);
        }
        XTSC_DEBUG(m_text, "script_thread is waiting for m_next_request event");
        wait(m_next_request);
        XTSC_DEBUG(m_text, "script_thread received m_next_request event");
      }
      else {
        // TLM interface
        for (double count = 1; m_no_timeout || (count <= timeout); count += 1) {
          u64 ticket;
          if (m_queue->nb_can_push()) {
            m_queue->nb_push(m_value, ticket);
            m_previous_push = sc_time_stamp();
            XTSC_INFO(m_text, "Pushed 0x" << m_value.to_string(SC_HEX).substr(m_width1%4 ? 2 : 3) << " ticket=0x" << ticket);
            break;
          }
          else {
            XTSC_DEBUG(m_text, "Cannot push 0x" << m_value.to_string(SC_HEX).substr(m_width1%4 ? 2 : 3) << " ticket=0x" << ticket <<
                               " (queue is full)");
          }
          wait(m_clock_period);
        }
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



void xtsc_component::xtsc_queue_producer::sample_thread(void) {

  try {

    wait(m_sample_phase + m_posedge_offset);

    while (true) {
      if ((m_push.read() != m_zero_bv) && (m_full.read() == m_zero_bv)) {
        XTSC_INFO(m_text, "Pushed 0x" << m_data.read().to_string(SC_HEX).substr(m_width1%4 ? 2 : 3));
        m_deassert.notify(m_deassert_delay);
      }
      wait(m_clock_period);
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



void xtsc_component::xtsc_queue_producer::request_thread(void) {

  try {

    m_push.write(m_zero_bv);

    while (true) {
      if (m_assert_delta_cycle == sc_delta_count()) {
        XTSC_DEBUG(m_text, "request_thread is skipping wait for m_assert event.");
      }
      else {
        XTSC_DEBUG(m_text, "request_thread is waiting for m_assert event.");
        wait(m_assert);
        XTSC_DEBUG(m_text, "request_thread received m_assert event.  m_value_bv=0x" << hex << m_value_bv);
      }
      m_push.write(m_one_bv);
      m_data.write(m_value_bv);
      wait(m_deassert);
      XTSC_DEBUG(m_text, "request_thread received m_deassert event");
      m_push.write(m_zero_bv);
      m_data.write(m_zero_bv);
      m_next_request.notify();  // Immediate notification
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



u32 xtsc_component::xtsc_queue_producer::get_u32(u32 index, const string& argument_name) {
  u32 value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing:" << endl;
    oss << m_line;
    oss << m_test_vector_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtou32(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number:" << endl;
    oss << m_line;
    oss << m_test_vector_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



double xtsc_component::xtsc_queue_producer::get_double(u32 index, const string& argument_name) {
  double value = 0;
  if (index >= m_words.size()) {
    ostringstream oss;
    oss << argument_name << " argument (#" << index+1 << ") missing:" << endl;
    oss << m_line;
    oss << m_test_vector_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  try {
    value = xtsc_strtod(m_words[index]);
  }
  catch (const xtsc_exception&) {
    ostringstream oss;
    oss << "Cannot convert " << argument_name << " argument (#" << index+1 << ") '" << m_words[index] << "' to number:" << endl;
    oss << m_line;
    oss << m_test_vector_stream.info_for_exception();
    throw xtsc_exception(oss.str());
  }
  return value;
}



void xtsc_component::xtsc_queue_producer::end_of_elaboration() {
  u32  width1 = (m_pin_level ? m_data->read().length() : m_queue->nb_get_bit_width());
  if (width1 != m_width1) {
    ostringstream oss;
    oss << "Width mismatch ERROR: xtsc_queue_producer '" << name() << "' has configured width of " << m_width1
        << " bits bound to queue interface (channel) of width " << width1 << ".";
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_queue_producer::xtsc_wire_write_if_impl::nb_write(const sc_unsigned& value) {
  if (static_cast<u32>(value.length()) != m_bit_width) {
    ostringstream oss;
    oss << "ERROR: Value of width=" << value.length() << " bits written to sc_export \"" << m_producer.m_p_control->name()
        << "\" of width=" << m_bit_width << " in xtsc_queue_producer '" << m_producer.name() << "'";
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_producer.m_text, m_producer.m_p_control->name() << " <= 0x" <<
                               value.to_string(SC_HEX).substr(m_producer.m_width1%4 ? 2 : 3));
  m_producer.m_control_write_count += 1;
  if (value != m_producer.m_control_value) {
    m_producer.m_control_value = value;
    m_producer.m_control_change_count += 1;
  }
  m_producer.m_control_write_event.notify(SC_ZERO_TIME);
}



void xtsc_component::xtsc_queue_producer::xtsc_wire_write_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to sc_export<xtsc_wire_write_if> \"" << m_producer.m_p_control->name()
        << "\" of xtsc_queue_producer '" << m_producer.name() << "'" << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_producer.m_text, "Binding '" << port.name() << "' to sc_export<xtsc_wire_write_if> \"" <<
                               m_producer.m_p_control->name() << "\" of xtsc_queue_producer '" << m_producer.name() << "'");
  m_p_port = &port;
  m_producer.m_control_bound = true;
}




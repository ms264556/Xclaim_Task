// Copyright (c) 2005-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <sstream>
#include <xtsc/xtsc_queue_pin.h>
#include <xtsc/xtsc_logging.h>
#include <xtsc/xtsc.h>



using namespace std;
#if SYSTEMC_VERSION >= 20050601
using namespace sc_core;
#endif
using namespace sc_dt;
using namespace xtsc;
using log4xtensa::INFO_LOG_LEVEL;
using log4xtensa::PUSH;
using log4xtensa::POP;
using log4xtensa::PUSH_FAILED;
using log4xtensa::POP_FAILED;
using log4xtensa::UNKNOWN_PC;
using log4xtensa::UNKNOWN;



xtsc_component::xtsc_queue_pin::xtsc_queue_pin(sc_module_name module_name, const xtsc_queue_pin_parms& queue_parms) :
  sc_module             (module_name),
  m_width1              (queue_parms.get_non_zero_u32("bit_width")),
  m_push                ("m_push"),
  m_data_in             ("m_data_in"),
  m_full                ("m_full"),
  m_pop                 ("m_pop"),
  m_empty               ("m_empty"),
  m_data_out            ("m_data_out"),
  m_push_floating       ("m_push_floating", 1, log4xtensa::TextLogger::getInstance(name())),
  m_data_in_floating    ("m_data_in_floating", m_width1, log4xtensa::TextLogger::getInstance(name())),
  m_full_floating       ("m_full_floating", 1, log4xtensa::TextLogger::getInstance(name())),
  m_pop_floating        ("m_pop_floating", 1, log4xtensa::TextLogger::getInstance(name())),
  m_empty_floating      ("m_empty_floating", 1, log4xtensa::TextLogger::getInstance(name())),
  m_data_out_floating   ("m_data_out_floating", m_width1, log4xtensa::TextLogger::getInstance(name())),
  m_quantity            ("m_quantity"),
  m_zero                (1),
  m_one                 (1),
  m_depth               (queue_parms.get_u32("depth")),
  m_text                (log4xtensa::TextLogger::getInstance(name())),
  m_binary              (log4xtensa::BinaryLogger::getInstance(name())),
  m_log_data_binary     (true),
  m_timestamp           (false),
  m_pop_file_element    ((int)m_width1)
{

  m_zero                        = 0;
  m_one                         = 1;
  m_rp                          = 0;
  m_wp                          = 0;
  m_p_trace_file                = static_cast<sc_trace_file*>(const_cast<void*>(queue_parms.get_void_pointer("vcd_handle")));

  m_pop_file_element            = 0;
  m_use_fifo                    = true;
  m_has_pop_file_element        = true;
  m_pop_file_line_number        = 0;
  m_next_word_index             = 0;
  m_push_file                   = 0;
  m_pop_file                    = 0;
  m_element_ptrs                = 0;
  m_tickets                     = 0;
  m_time_resolution             = sc_get_time_resolution();

  m_words.clear();

  // Handle push_file
  m_push_file_name = queue_parms.get_c_str("push_file");
  if (m_push_file_name && !m_push_file_name[0]) m_push_file_name = 0;
  if (m_push_file_name) {
    m_use_fifo = false;
    m_push_file = new ofstream(m_push_file_name, ios::out);
    if (!m_push_file->is_open()) {
      ostringstream oss;
      oss << "xtsc_queue_pin '" << name() << "' cannot open push_file '" << m_push_file_name << "'.";
      throw xtsc_exception(oss.str());
    }
    m_timestamp = queue_parms.get_bool("timestamp");
  }

  // Handle pop_file
  m_pop_file_name = queue_parms.get_c_str("pop_file");
  if (m_pop_file_name && !m_pop_file_name[0]) m_pop_file_name = 0;
  m_wraparound  = queue_parms.get_bool("wraparound");
  if (m_pop_file_name) {
    m_use_fifo = false;
    m_pop_file = new xtsc_script_file(m_pop_file_name, "pop_file",  name(), kind(), m_wraparound);
    get_next_pop_file_element();
  }



  // Get clock period 
  u32 clock_period = queue_parms.get_non_zero_u32("clock_period");
  if (clock_period == 0xFFFFFFFF) {
    m_clock_period = xtsc_get_system_clock_period();
  }
  else {
    m_clock_period = m_time_resolution * clock_period;
  }
  m_clock_period_value = m_clock_period.value();
  u32 posedge_offset = queue_parms.get_u32("posedge_offset");
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

  // Get clock phase when the signals are to be sampled
  m_sample_phase_value = queue_parms.get_u32("sample_phase");
  if (m_sample_phase_value >= m_clock_period_value) {
    ostringstream oss;
    oss << "xtsc_queue_pin '" << name() << "' parameter error:" << endl;
    oss << "  \"sample_phase\" (" << m_sample_phase_value
        << ") must be strictly less than \"clock_period\" (";
    if (clock_period == 0xFFFFFFFF) {
      oss << "0xFFFFFFFF => " << m_clock_period_value;
    }
    else {
      oss << clock_period;
    }
    oss << ")";
    throw xtsc_exception(oss.str());
  }
  m_sample_phase = m_time_resolution * m_sample_phase_value;

  m_quantity.write(0);

  if (m_use_fifo) {

    if (m_depth == 0) {
      // This will generate an exception in a standard format 
      queue_parms.get_non_zero_u32("depth");
    }

    // Create a pool of sc_bv_base objects with pool size equal to the fifo depth
    m_element_ptrs = new sc_bv_base*[m_depth];
    for (u32 i=0; i<m_depth; i++) {
      m_element_ptrs[i] = new sc_bv_base((int)m_width1);
      *m_element_ptrs[i] = m_zero;
    }
    m_tickets = new u64[m_depth];

    SC_THREAD(worker_thread);
  }
  else {
    SC_THREAD(file_worker_thread);
  }

  if (m_p_trace_file) {
    sc_trace(m_p_trace_file, m_push,      m_push    .name());
    sc_trace(m_p_trace_file, m_data_in,   m_data_in .name());
    sc_trace(m_p_trace_file, m_full,      m_full    .name());

    sc_trace(m_p_trace_file, m_pop,       m_pop     .name());
    sc_trace(m_p_trace_file, m_empty,     m_empty   .name());
    sc_trace(m_p_trace_file, m_data_out,  m_data_out.name());

    sc_trace(m_p_trace_file, m_quantity,  m_quantity.name());
  }

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, "Constructed xtsc_queue_pin '" << name() << "':");
  XTSC_LOG(m_text, ll, " depth                   = "   << m_depth);
  XTSC_LOG(m_text, ll, " bit_width               = "   << m_width1);
  XTSC_LOG(m_text, ll, " push_file               = "   << (m_push_file_name ? m_push_file_name : ""));
  if (m_push_file_name) {
  XTSC_LOG(m_text, ll, " timestamp               = "   << boolalpha << m_timestamp);
  }
  XTSC_LOG(m_text, ll, " pop_file                = "   << (m_pop_file_name  ? m_pop_file_name  : ""));
  XTSC_LOG(m_text, ll, " wraparound              = "   << (m_wraparound ? "true" : "false"));
  XTSC_LOG(m_text, ll, " vcd_handle              = "   << m_p_trace_file);
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
  XTSC_LOG(m_text, ll, " sample_phase            = "   << m_sample_phase_value << " (" << m_sample_phase << ")");

}



xtsc_component::xtsc_queue_pin::~xtsc_queue_pin() {
  if (m_push_file) delete m_push_file;
  if (m_pop_file)  delete m_pop_file;
  if (m_element_ptrs) {
    for (u32 i=0; i<m_depth; i++) {
      delete m_element_ptrs[i];
    }
    delete [] m_element_ptrs;
  }
  if (m_tickets) delete [] m_tickets;
}



void xtsc_component::xtsc_queue_pin::worker_thread() {

  try {


    m_full.write(m_zero);
    m_empty.write(m_one);
    m_data_out.write(m_zero);

    wait(m_sample_phase + m_posedge_offset);

    while (true) {

      bool was_popped     = false;
      bool was_pushed     = false;
      bool pop_failed     = false;
      bool push_failed    = false;
      u32  quantity       = m_quantity.read();

      // Was queue pop attempted?
      if (m_pop.read() != m_zero) {
        // Was it successful?
        if (m_empty.read() == m_zero) {
          was_popped = true;
        }
        else {
          pop_failed = true;
        }
      }

      // Was queue push attempted?
      if (m_push.read() != m_zero) {
        // Was it successful?
        if (m_full.read() == m_zero) {
          was_pushed = true;
        }
        else {
          push_failed = true;
        }
      }

      // If either popped or pushed, then handle them
      if (was_popped || was_pushed) {
        quantity += (was_pushed ? 1 : 0) - (was_popped ? 1 : 0);                         
        m_quantity.write(quantity);

        if (was_popped) {
          XTSC_INFO(m_text, "Popped (ticket=" << m_tickets[m_rp] << " cnt=" << quantity << "): 0x" << hex << *m_element_ptrs[m_rp]);
          xtsc_log_queue_event(m_binary, INFO_LOG_LEVEL, m_tickets[m_rp], UNKNOWN, POP, UNKNOWN_PC, quantity, m_depth, 
                               m_log_data_binary, *m_element_ptrs[m_rp]);
          *m_element_ptrs[m_rp] = m_zero;  // Zero out old value
          m_rp = (m_rp + 1) % m_depth;
        }

        if (was_pushed) {
          *m_element_ptrs[m_wp] = m_data_in.read();
          m_tickets      [m_wp] = xtsc_create_queue_ticket();
          XTSC_INFO(m_text, "Pushed (ticket=" << m_tickets[m_wp] << " cnt=" << quantity << "): 0x" << hex << *m_element_ptrs[m_wp]);
          xtsc_log_queue_event(m_binary, INFO_LOG_LEVEL, m_tickets[m_wp], UNKNOWN, PUSH, UNKNOWN_PC, quantity, m_depth, 
                               m_log_data_binary, *m_element_ptrs[m_wp]);
          m_wp = (m_wp + 1) % m_depth;
        }

        // Drive new output (there might be no data in the queue, in which case zeroes will be driven)
        m_data_out.write(*m_element_ptrs[m_rp]);

      }

      // If it was popped but not pushed, then it is not full and it might have become empty
      if (was_popped && !was_pushed) {
        m_full.write(m_zero);
        if (m_rp == m_wp) {
          m_empty.write(m_one);
        }
      }
      // If it was pushed but not popped, then it is not empty and it might have become full
      else if (!was_popped && was_pushed) {
        m_empty.write(m_zero);
        if (m_rp == m_wp) {
          m_full.write(m_one);
        }
      }

      // Log failures
      if (pop_failed) {
        xtsc_log_queue_event(m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, POP_FAILED, UNKNOWN_PC, quantity, m_depth, 
                             false, m_zero);
      }
      if (push_failed) {
        xtsc_log_queue_event(m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, PUSH_FAILED, UNKNOWN_PC, quantity, m_depth,
                             m_log_data_binary, m_data_in.read());
      }

      // Wait
      if (was_pushed || was_popped) {
        wait(m_clock_period);
      }
      else {
        XTSC_DEBUG(m_text, "worker_thread is sleeping . . .");
        wait(m_push.default_event() | m_pop.default_event());

        // Need to ensure we're synchronized to sample phase
        u64 now_value = sc_time_stamp().value();
        u64 now_phase_value = now_value % m_clock_period_value;
        if (m_has_posedge_offset) {
          if (now_phase_value < m_posedge_offset_value) {
            now_phase_value += m_clock_period_value;
          }
          now_phase_value -= m_posedge_offset_value;
        }
        if (now_phase_value != m_sample_phase_value) {
          u64 delay_value = 0;
          if (now_phase_value < m_sample_phase_value) {
            delay_value = m_sample_phase_value - now_phase_value;
          }
          else {
            delay_value = m_clock_period_value + m_sample_phase_value - now_phase_value;
          }
          sc_time delay = delay_value * m_time_resolution;
          XTSC_DEBUG(m_text, "worker_thread was woken up and will now wait " << delay << " to sync to sample phase.");
          wait(delay);
        }
        else {
          XTSC_DEBUG(m_text, "worker_thread was woken up and was already synchronized to sample phase.");
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



void xtsc_component::xtsc_queue_pin::file_worker_thread() {

  try {

    m_full.write(m_zero);
    m_data_out.write(m_pop_file_element);
    m_empty.write(m_has_pop_file_element ? m_zero : m_one);

    wait(m_sample_phase + m_posedge_offset);

    while (true) {

      bool was_popped     = false;
      bool was_pushed     = (m_push_file_name && (m_push.read() != m_zero));
      bool pop_failed     = false;
      u32  quantity       = m_quantity.read();

      // Was queue pop attempted?
      if (m_pop_file_name && (m_pop.read() != m_zero)) {
        // Was it successful?
        if (m_has_pop_file_element) {
          was_popped = true;
        }
        else {
          pop_failed = true;
        }
      }

      if (was_popped) {
        quantity -= 1;
        XTSC_INFO(m_text, "Popped from file 0x" << hex << m_pop_file_element);
        get_next_pop_file_element();
        m_data_out.write(m_pop_file_element);
        m_empty.write(m_has_pop_file_element ? m_zero : m_one);
      }

      if (was_pushed) {
        quantity += 1;
        ostringstream oss;
        oss << "0x" << hex << m_data_in.read();
        if (m_timestamp) {
          string buf;
          oss << " // " << setprecision(xtsc_get_text_logging_time_precision()) << fixed << setw(xtsc_get_text_logging_time_width())
                        << (sc_core::sc_time_stamp() / xtsc_get_system_clock_period()) << xtsc_log_delta_cycle(buf);
        }
        *m_push_file << oss.str() << endl;
        XTSC_INFO(m_text, "Pushed to file " << oss.str());
      }

      m_quantity.write(quantity);

      // Wait
      if (was_pushed || was_popped) {
        wait(m_clock_period);
      }
      else {
        XTSC_DEBUG(m_text, "file_worker_thread is sleeping . . .");
        wait(m_push.default_event() | m_pop.default_event());

        // Need to ensure we're synchronized to sample phase
        u64 now_value = sc_time_stamp().value();
        u64 now_phase_value = now_value % m_clock_period_value;
        if (m_has_posedge_offset) {
          if (now_phase_value < m_posedge_offset_value) {
            now_phase_value += m_clock_period_value;
          }
          now_phase_value -= m_posedge_offset_value;
        }
        if (now_phase_value != m_sample_phase_value) {
          u64 delay_value = 0;
          if (now_phase_value < m_sample_phase_value) {
            delay_value = m_sample_phase_value - now_phase_value;
          }
          else {
            delay_value = m_clock_period_value + m_sample_phase_value - now_phase_value;
          }
          sc_time delay = delay_value * m_time_resolution;
          XTSC_DEBUG(m_text, "file_worker_thread was woken up and will now wait " << delay << " to sync to sample phase.");
          wait(delay);
        }
        else {
          XTSC_DEBUG(m_text, "file_worker_thread was woken up and was already synchronized to sample phase.");
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



void xtsc_component::xtsc_queue_pin::before_end_of_elaboration() {
  if (!m_use_fifo) {

    if (!m_push_file_name) {
      if (m_push.get_interface() || m_data_in.get_interface() || m_full.get_interface()) {
        ostringstream oss;
        oss << "User has bound to the push interface of xtsc_queue_pin '" << name()
            << "' but no push_file was specified at construction time.";
        throw xtsc_exception(oss.str());
      }
      m_push(m_push_floating);
      m_data_in(m_data_in_floating);
      m_full(m_full_floating);
    }

    if (!m_pop_file_name) {
      if (m_pop.get_interface() || m_data_out.get_interface() || m_empty.get_interface()) {
        ostringstream oss;
        oss << "User has bound to the pop interface of xtsc_queue_pin '" << name()
            << "' but no pop_file was specified at construction time.";
        throw xtsc_exception(oss.str());
      }
      m_pop(m_pop_floating);
      m_data_out(m_data_out_floating);
      m_empty(m_empty_floating);
    }

  }
}



void xtsc_component::xtsc_queue_pin::get_next_pop_file_element() {
  if (!m_has_pop_file_element) {
    return;
  }
  if (m_next_word_index >= m_words.size()) {
    m_pop_file_line_number = m_pop_file->get_words(m_words, m_line);
    if (!m_pop_file_line_number) {
      m_has_pop_file_element = false;
      m_pop_file_element = 0;
      return;
    }
    m_next_word_index = 0;
  }
  try {
    m_pop_file_element = m_words[m_next_word_index].c_str();
  }
  catch (...) {
    ostringstream oss;
    oss << "Cannot convert word #" << (m_next_word_index+1) << " (\"" << m_words[m_next_word_index] << "\") to number:" << endl;
    oss << m_line;
    oss << m_pop_file->info_for_exception();
    throw xtsc_exception(oss.str());
  }
  m_next_word_index += 1;
}




// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <sstream>
#include <xtsc/xtsc_queue.h>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_queue_producer.h>
#include <xtsc/xtsc_queue_consumer.h>
#include <xtsc/xtsc_logging.h>


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



xtsc_component::xtsc_queue_parms::xtsc_queue_parms(const xtsc_core&     core,
                                                   const char          *queue_name,
                                                   u32                  depth,
                                                   const char          *push_file,
                                                   const char          *pop_file,
                                                   bool                 wraparound)
{
  if (!core.has_input_queue(queue_name) && !core.has_output_queue(queue_name)) {
    ostringstream oss;
    oss << "xtsc_queue_parms: core '" << core.name() << "' has no input/output queue interface named '" << queue_name << "'.";
    throw xtsc_exception(oss.str());
  }
  u32 width1 = core.get_tie_bit_width(queue_name);
  init(width1, depth, push_file, pop_file, wraparound);
}



xtsc_component::xtsc_queue::xtsc_queue(sc_module_name module_name, const xtsc_queue_parms& queue_parms) :
  sc_module             (module_name),
  m_producer            (queue_parms.get_u32("num_producers") > 1 ? "m_producer[0]" : "m_producer"),
  m_consumer            (queue_parms.get_u32("num_consumers") > 1 ? "m_consumer[0]" : "m_consumer"),
  m_producers           (NULL),
  m_consumers           (NULL),
  m_push_impl           (NULL),
  m_pop_impl            (NULL),
  m_push_multi_impl     (NULL),
  m_pop_multi_impl      (NULL),
  m_num_producers       (queue_parms.get_u32("num_producers")),
  m_num_consumers       (queue_parms.get_u32("num_consumers")),
  m_multi_client        ((m_num_producers > 1) || (m_num_consumers > 1)),
  m_depth               (queue_parms.get_u32("depth")),
  m_fifo                ("m_fifo", (m_depth ? m_depth : 1)),
  m_use_fifo            (true),
  m_element_ptrs        (NULL),
  m_tickets             (NULL),
  m_skid_index          (0),
  m_skid_fifos          (NULL),
  m_skid_buffers        (NULL),
  m_skid_tickets        (NULL),
  m_jerk_index          (0),
  m_jerk_fifos          (NULL),
  m_jerk_buffers        (NULL),
  m_jerk_tickets        (NULL),
  m_dummy               (1),    // Length of 1 bit
  m_width1              (queue_parms.get_non_zero_u32("bit_width")),
  m_width8              ((m_width1+7)/8),
  m_text                (log4xtensa::TextLogger::getInstance(name())),
  m_binary              (log4xtensa::BinaryLogger::getInstance(name())),
  m_log_data_binary     (true),
  m_push_file           (NULL),
  m_timestamp           (false),
  m_pop_file            (NULL),
  m_pop_file_element    (m_width1),
  m_nonempty_events     (NULL),
  m_nonfull_events      (NULL)
{

  m_dummy = 0;                  // Value of 0

  // Handle push_file
  const char *push_file = queue_parms.get_c_str("push_file");
  if (push_file && push_file[0]) {
    m_push_file_name = push_file;
    m_use_fifo = false;
    m_push_file = new ofstream(m_push_file_name.c_str(), ios::out);
    if (!m_push_file->is_open()) {
      ostringstream oss;
      oss << "xtsc_queue '" << name() << "' cannot open push_file '" << m_push_file_name << "'.";
      throw xtsc_exception(oss.str());
    }
    m_timestamp = queue_parms.get_bool("timestamp");
  }

  // Handle pop_file
  m_wraparound  = queue_parms.get_bool("wraparound");
  const char *pop_file = queue_parms.get_c_str("pop_file");
  if (pop_file && pop_file[0]) {
    m_pop_file_name = pop_file;
    m_use_fifo = false;
    m_pop_file = new xtsc_script_file(m_pop_file_name.c_str(), "pop_file",  name(), kind(), m_wraparound);
    get_next_pop_file_element();
  }

  if (m_use_fifo) {
    if (m_depth == 0) {
      // This will generate an exception in a standard format 
      queue_parms.get_non_zero_u32("depth");
    }
    // Create a pool of sc_unsigned objects with pool size equal to the fifo depth
    m_element_ptrs = new sc_unsigned*[m_depth];
    for (u32 i=0; i<m_depth; i++) {
      m_element_ptrs[i] = new sc_unsigned(m_width1);
    }
    m_tickets = new u64[m_depth];
  }

  if (m_multi_client) {
    m_producers         = new sc_export<xtsc_queue_push_if>*[m_num_producers];
    m_push_multi_impl   = new xtsc_queue_push_if_multi_impl*[m_num_producers];
    if (m_use_fifo) {
      m_skid_fifos      = new sc_fifo<int>                 *[m_num_producers];
      m_skid_buffers    = new sc_unsigned                  *[m_num_producers];
      m_skid_tickets    = new u64                           [m_num_producers];
      m_nonfull_events  = new sc_event                     *[m_num_producers];
      m_jerk_fifos      = new sc_fifo<int>                 *[m_num_consumers];
      m_jerk_buffers    = new sc_unsigned                  *[m_num_consumers];
      m_jerk_tickets    = new u64                           [m_num_consumers];
      m_nonempty_events = new sc_event                     *[m_num_consumers];
      SC_METHOD(delta_cycle_method);
      sensitive << m_push_pop_event;
    }
    for (u32 i=0; i < m_num_producers; ++i) {
      if (i == 0) {
        m_producers[0] = &m_producer;
      }
      else {
        ostringstream oss1;
        oss1 << "m_producer[" << i << "]";
        m_producers[i] = new sc_export<xtsc_queue_push_if>(oss1.str().c_str());
      }
      ostringstream oss2;
      oss2 << "m_push_multi_impl[" << i << "]";
      m_push_multi_impl[i] = new xtsc_queue_push_if_multi_impl(oss2.str().c_str(), *this, i);
      (*m_producers[i])(*m_push_multi_impl[i]);
      if (m_use_fifo) {
        ostringstream oss3;
        oss3 << "m_skid_fifos[" << i << "]";
        m_skid_fifos    [i] = new sc_fifo<int>(oss3.str().c_str(), 1);
        m_skid_buffers  [i] = new sc_unsigned(m_width1);
        m_skid_tickets  [i] = 0;
        m_nonfull_events[i] = new sc_event();
      }
    }
    m_consumers         = new sc_export<xtsc_queue_pop_if>*[m_num_consumers];
    m_pop_multi_impl    = new xtsc_queue_pop_if_multi_impl*[m_num_consumers];
    for (u32 i=0; i < m_num_consumers; ++i) {
      if (i == 0) {
        m_consumers[0] = &m_consumer;
      }
      else {
        ostringstream oss1;
        oss1 << "m_consumer[" << i << "]";
        m_consumers[i] = new sc_export<xtsc_queue_pop_if>(oss1.str().c_str());
      }
      ostringstream oss2;
      oss2 << "m_pop_multi_impl[" << i << "]";
      m_pop_multi_impl[i] = new xtsc_queue_pop_if_multi_impl(oss2.str().c_str(), *this, i);
      (*m_consumers[i])(*m_pop_multi_impl[i]);
      if (m_use_fifo) {
        ostringstream oss3;
        oss3 << "m_jerk_fifos[" << i << "]";
        m_jerk_fifos     [i] = new sc_fifo<int>(oss3.str().c_str(), 1);
        m_jerk_buffers   [i] = new sc_unsigned(m_width1);
        m_jerk_tickets   [i] = 0;
        m_nonempty_events[i] = new sc_event();
      }
    }
  }
  else {
    m_push_impl = new xtsc_queue_push_if_impl("m_push_impl", *this),
    m_pop_impl  = new xtsc_queue_pop_if_impl("m_pop_impl", *this),
    m_producer(*m_push_impl);
    m_consumer(*m_pop_impl);
  }

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll,        "Constructed xtsc_queue '" << name() << "'" << (m_multi_client ? " (Multi-client)" : "") << ":");
  XTSC_LOG(m_text, ll,        " num_producers           = "   << m_num_producers);
  XTSC_LOG(m_text, ll,        " num_consumers           = "   << m_num_consumers);
  XTSC_LOG(m_text, ll,        " depth                   = "   << m_depth);
  XTSC_LOG(m_text, ll,        " bit_width               = "   << m_width1);
  XTSC_LOG(m_text, ll,        " push_file               = "   << m_push_file_name);
  if (m_push_file_name != "") {
  XTSC_LOG(m_text, ll,        " timestamp               = "   << boolalpha << m_timestamp);
  }
  XTSC_LOG(m_text, ll,        " pop_file                = "   << m_pop_file_name);
  XTSC_LOG(m_text, ll,        " wraparound              = "   << (m_wraparound ? "true" : "false"));

  reset();

}



xtsc_component::xtsc_queue::~xtsc_queue() {
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



void xtsc_component::xtsc_queue::reset(bool /*hard_reset*/) {
  XTSC_INFO(m_text, "xtsc_queue::reset()");
  m_pop_file_element            = 0;
  m_has_pop_file_element        = true;
  m_pop_file_line_number        = 0;
  m_next_word_index             = 0;

  m_words.clear();

  m_next = 0;

  if (m_push_file) {
    m_push_file->close();
    m_push_file->clear();
    m_push_file->open(m_push_file_name.c_str(), ios::out);
    if (!m_push_file->is_open()) {
      ostringstream oss;
      oss << "xtsc_queue '" << name() << "' reset() method cannot open push_file '" << m_push_file_name << "'.";
      throw xtsc_exception(oss.str());
    }
  }

  if (m_pop_file) {
    m_pop_file->reset();
    get_next_pop_file_element();
  }
}



u32 xtsc_component::xtsc_queue::get_num_producers() {
  return m_num_producers;
}



u32 xtsc_component::xtsc_queue::get_num_consumers() {
  return m_num_consumers;
}



void xtsc_component::xtsc_queue::connect(xtsc::xtsc_core& core, const char *queue_name, u32 port_num) {
  if (core.has_input_queue(queue_name)) {
    if (port_num >= m_num_consumers) {
      ostringstream oss;
      oss << "xtsc_queue::connect called with port_num=" << port_num << " which is not strictly less than " << m_num_consumers
          << " (from \"m_num_consumers\")"; 
      throw xtsc_exception(oss.str());
    }
    core.get_input_queue(queue_name)(m_multi_client ? *m_consumers[port_num] : m_consumer);
  }
  else if (core.has_output_queue(queue_name)) {
    if (port_num >= m_num_producers) {
      ostringstream oss;
      oss << "xtsc_queue::connect called with port_num=" << port_num << " which is not strictly less than " << m_num_producers
          << " (from \"m_num_producers\")"; 
      throw xtsc_exception(oss.str());
    }
    core.get_output_queue(queue_name)(m_multi_client ? *m_producers[port_num] : m_producer);
  }
  else {
    ostringstream oss;
    oss << "xtsc_queue::connect: core '" << core.name() << "' has no input/output queue interface named '" << queue_name << "'.";
    throw xtsc_exception(oss.str());
  }
}



void xtsc_component::xtsc_queue::connect(xtsc_queue_producer& producer, u32 port_num) {
  if (port_num >= m_num_producers) {
    ostringstream oss;
    oss << "xtsc_queue::connect called with port_num=" << port_num << " which is not strictly less than " << m_num_producers
        << " (from \"m_num_producers\")"; 
    throw xtsc_exception(oss.str());
  }
  producer.m_queue(m_multi_client ? *m_producers[port_num] : m_producer);
}



void xtsc_component::xtsc_queue::connect(xtsc_queue_consumer& consumer, u32 port_num) {
  if (port_num >= m_num_consumers) {
    ostringstream oss;
    oss << "xtsc_queue::connect called with port_num=" << port_num << " which is not strictly less than " << m_num_consumers
        << " (from \"m_num_consumers\")"; 
    throw xtsc_exception(oss.str());
  }
  consumer.m_queue(m_multi_client ? *m_consumers[port_num] : m_consumer);
}



void xtsc_component::xtsc_queue::delta_cycle_method() {
  XTSC_DEBUG(m_text, "delta_cycle_method()");

  for (u32 limit = m_skid_index + m_num_producers; ((m_skid_index < limit) && (m_fifo.num_free() != 0)); ++m_skid_index) {
    u32 port = m_skid_index % m_num_producers;
    if (m_skid_fifos[port]->num_available() != 0) {
      int dummy = 0;
      m_skid_fifos[port]->nb_read(dummy);
      m_fifo.nb_write(m_next);
      m_tickets[m_next] = m_skid_tickets[port];
      *m_element_ptrs[m_next] = *m_skid_buffers[port];
      m_next = (m_next + 1) % m_depth;
      m_nonfull_events[port]->notify(SC_ZERO_TIME);
      m_push_pop_event.notify(SC_ZERO_TIME);
      XTSC_DEBUG(m_text, "delta_cycle_method() moved from skid buffer #" << port << " to fifo: 0x" << hex << *m_skid_buffers[port]);
    }
  }
  m_skid_index = m_skid_index % m_num_producers;

  for (u32 limit = m_jerk_index + m_num_consumers; ((m_jerk_index < limit) && (m_fifo.num_available() != 0)); ++m_jerk_index) {
    u32 port = m_jerk_index % m_num_consumers;
    if (m_jerk_fifos[port]->num_free() != 0) {
      int dummy = 0;
      m_jerk_fifos[port]->nb_write(dummy);
      int index = 0;
      m_fifo.nb_read(index);
      m_jerk_tickets[port] = m_tickets[index];
      *m_jerk_buffers[port] = *m_element_ptrs[index];
      m_nonempty_events[port]->notify(SC_ZERO_TIME);
      m_push_pop_event.notify(SC_ZERO_TIME);
      XTSC_DEBUG(m_text, "delta_cycle_method() moved from fifo to jerk buffer #" << port << ": 0x" << hex << *m_jerk_buffers[port]);
    }
  }
  m_jerk_index = m_jerk_index % m_num_consumers;

}




void xtsc_component::xtsc_queue::get_next_pop_file_element() {
  if (!m_has_pop_file_element) {
    return;
  }
  if (m_next_word_index >= m_words.size()) {
    m_pop_file_line_number = m_pop_file->get_words(m_words, m_line);
    if (!m_pop_file_line_number) {
      m_has_pop_file_element = false;
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



bool xtsc_component::xtsc_queue::xtsc_queue_push_if_impl::nb_can_push() {
  bool can_push = false;
  if (m_queue.m_use_fifo) {
    if (m_queue.m_fifo.num_free() != 0) {
      can_push = true;
    }
    else {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, PUSH_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, false, m_queue.m_dummy);
      can_push = false;
    }
  }
  else {
    if (m_queue.m_push_file) {
      can_push = true;
    }
    else {
      ostringstream oss;
      oss << "nb_can_push() called for xtsc_queue '" << m_queue.name() << "', but no push_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
  XTSC_DEBUG(m_queue.m_text, "nb_can_push()=" << boolalpha << can_push);
  return can_push;
}



bool xtsc_component::xtsc_queue::xtsc_queue_pop_if_impl::nb_can_pop() {
  bool can_pop = false;
  if (m_queue.m_use_fifo) {
    if (m_queue.m_fifo.num_available() != 0) {
      can_pop = true;
    }
    else {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, POP_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, false, m_queue.m_dummy);
      can_pop = false;
    }
  }
  else {
    if (m_queue.m_pop_file) {
      can_pop = m_queue.m_has_pop_file_element;
    }
    else {
      ostringstream oss;
      oss << "nb_can_pop() called for xtsc_queue '" << m_queue.name() << "', but no pop_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
  XTSC_DEBUG(m_queue.m_text, "nb_can_pop()=" << boolalpha << can_pop);
  return can_pop;
}




bool xtsc_component::xtsc_queue::xtsc_queue_push_if_impl::nb_push(const sc_unsigned& element, u64& ticket) {
  if (static_cast<u32>(element.length()) != m_queue.m_width1) {
    ostringstream oss;
    oss << "ERROR: Element of width=" << element.length() << " bits added to queue '" << m_queue.name() << "' of width="
        << m_queue.m_width1;
    throw xtsc_exception(oss.str());
  }
  if (m_queue.m_use_fifo) {
    if (!m_queue.m_fifo.nb_write(m_queue.m_next)) {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, PUSH_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, m_queue.m_log_data_binary, element);
      return false;
    }
    ticket = xtsc_create_queue_ticket();
    m_queue.m_tickets[m_queue.m_next] = ticket;
    *m_queue.m_element_ptrs[m_queue.m_next] = element;
    XTSC_INFO(m_queue.m_text, "Pushed (ticket=" << ticket << " cnt=" << m_queue.m_fifo.num_available()+1 <<
                              "): 0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3));
    xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, ticket, UNKNOWN, PUSH, UNKNOWN_PC, m_queue.m_fifo.num_available()+1,
                         m_queue.m_depth, m_queue.m_log_data_binary, element);
    m_queue.m_next = (m_queue.m_next + 1) % m_queue.m_depth;
    // whenever the number available is 0 (i.e. it was empty), then notify one delta cycle later that it is no longer empty
    if (m_queue.m_fifo.num_available() == 0) {
      m_queue.m_nonempty_event.notify(SC_ZERO_TIME);
    }

    return true;
  }
  else {
    ticket = 0ULL;
    if (m_queue.m_push_file) {
      ostringstream oss;
      oss << "0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3);
      if (m_queue.m_timestamp) {
        string buf;
        oss << " // " << setprecision(xtsc_get_text_logging_time_precision()) << fixed << setw(xtsc_get_text_logging_time_width())
                      << (sc_core::sc_time_stamp() / xtsc_get_system_clock_period()) << xtsc_log_delta_cycle(buf);
      }
      *m_queue.m_push_file << oss.str() << endl;
      XTSC_INFO(m_queue.m_text, "Pushed to file " << oss.str());
      return true;
    }
    else {
      ostringstream oss;
      oss << "nb_push() called for xtsc_queue '" << m_queue.name() << "', but no push_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
}



void xtsc_component::xtsc_queue::xtsc_queue_push_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_queue '" << m_queue.name() << "' m_producer export: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_queue.m_text, "Binding '" << port.name() << "' to xtsc_queue::m_producer");
  m_p_port = &port;
}



bool xtsc_component::xtsc_queue::xtsc_queue_pop_if_impl::nb_pop(sc_unsigned& element, u64& ticket) {
  if (m_queue.m_use_fifo) {
    int index = 0;
    if (!m_queue.m_fifo.nb_read(index)) {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, POP_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, false, m_queue.m_dummy);
      return false;
    }
    element = *m_queue.m_element_ptrs[index];
    ticket = m_queue.m_tickets[index];
    XTSC_INFO(m_queue.m_text, "Popped (ticket=" << ticket << " cnt=" << m_queue.m_fifo.num_available() <<
                               "): 0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3));
    xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, ticket, UNKNOWN, POP, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                         m_queue.m_depth, m_queue.m_log_data_binary, element);
    // whenever the number free is 0 (i.e. it was full), then notify one delta cycle later that it is no longer full
    if (m_queue.m_fifo.num_free() == 0) {
      m_queue.m_nonfull_event.notify(SC_ZERO_TIME);
    }
    return true;
  }
  else {
    ticket = 0ULL;
    if (m_queue.m_pop_file) {
      if (m_queue.m_has_pop_file_element) {
        element = m_queue.m_pop_file_element;
        XTSC_INFO(m_queue.m_text, "Popped from file 0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3));
        m_queue.get_next_pop_file_element();
        return true;
      }
      else {
        return false;
      }
    }
    else {
      ostringstream oss;
      oss << "nb_pop() called for xtsc_queue '" << m_queue.name() << "', but no pop_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
}



void xtsc_component::xtsc_queue::xtsc_queue_pop_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_queue '" << m_queue.name() << "' m_consumer export: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_queue.m_text, "Binding '" << port.name() << "' to xtsc_queue::m_consumer");
  m_p_port = &port;
}



bool xtsc_component::xtsc_queue::xtsc_queue_push_if_multi_impl::nb_can_push() {
  bool can_push = false;
  if (m_queue.m_use_fifo) {
    if (m_queue.m_skid_fifos[m_port_num]->num_free() != 0) {
      can_push = true;
    }
    else {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, PUSH_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, false, m_queue.m_dummy);
      can_push = false;
    }
  }
  else {
    if (m_queue.m_push_file) {
      can_push = true;
    }
    else {
      ostringstream oss;
      oss << "nb_can_push() called for xtsc_queue '" << m_queue.name() << "', but no push_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
  XTSC_DEBUG(m_queue.m_text, "nb_can_push()=" << boolalpha << can_push << " (m_port_num=" << m_port_num << ")");
  return can_push;
}



bool xtsc_component::xtsc_queue::xtsc_queue_push_if_multi_impl::nb_push(const sc_unsigned& element, u64& ticket) {
  if (static_cast<u32>(element.length()) != m_queue.m_width1) {
    ostringstream oss;
    oss << "ERROR: Element of width=" << element.length() << " bits added to queue '" << m_queue.name() << "' of width="
        << m_queue.m_width1;
    throw xtsc_exception(oss.str());
  }
  if (m_queue.m_use_fifo) {
    if (!nb_can_push()) {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, PUSH_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, m_queue.m_log_data_binary, element);
      return false;
    }
    ticket = xtsc_create_queue_ticket();
    m_queue.m_skid_tickets[m_port_num] = ticket;
    m_queue.m_skid_fifos[m_port_num]->nb_write(0);
    *m_queue.m_skid_buffers[m_port_num] = element;
    XTSC_INFO(m_queue.m_text, "Pushed (ticket=" << ticket << " cnt=" << m_queue.m_fifo.num_available()+1 << "): 0x" <<
                              element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3) << " (Port #" << m_port_num << ")");
    xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, ticket, UNKNOWN, PUSH, UNKNOWN_PC, m_queue.m_fifo.num_available()+1,
                         m_queue.m_depth, m_queue.m_log_data_binary, element);
    m_queue.m_push_pop_event.notify(SC_ZERO_TIME);
    return true;
  }
  else {
    ticket = 0ULL;
    if (m_queue.m_push_file) {
      ostringstream oss;
      oss << "0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3);
      if (m_queue.m_timestamp) {
        string buf;
        oss << " // " << setprecision(xtsc_get_text_logging_time_precision()) << fixed << setw(xtsc_get_text_logging_time_width())
                      << (sc_core::sc_time_stamp() / xtsc_get_system_clock_period()) << xtsc_log_delta_cycle(buf);
      }
      *m_queue.m_push_file << oss.str() << endl;
      XTSC_INFO(m_queue.m_text, "Pushed to file " << oss.str() << " (Port #" << m_port_num << ")");
      return true;
    }
    else {
      ostringstream oss;
      oss << "nb_push() called for xtsc_queue '" << m_queue.name() << "', but no push_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
}



void xtsc_component::xtsc_queue::xtsc_queue_push_if_multi_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to '" << m_queue.name() << "' xtsc_queue::m_producers[" << m_port_num << "] export: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_queue.m_text, "Binding '" << port.name() << "' to xtsc_queue::m_producers[" << m_port_num << "]");
  m_p_port = &port;
}



bool xtsc_component::xtsc_queue::xtsc_queue_pop_if_multi_impl::nb_can_pop() {
  bool can_pop = false;
  if (m_queue.m_use_fifo) {
    if (m_queue.m_jerk_fifos[m_port_num]->num_available() != 0) {
      can_pop = true;
    }
    else {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, POP_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, false, m_queue.m_dummy);
    }
  }
  else {
    if (m_queue.m_pop_file) {
      can_pop = m_queue.m_has_pop_file_element;
    }
    else {
      ostringstream oss;
      oss << "nb_can_pop() called for xtsc_queue '" << m_queue.name() << "', but no pop_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
  XTSC_DEBUG(m_queue.m_text, "nb_can_pop()=" << boolalpha << can_pop << " (m_port_num=" << m_port_num << ")");
  return can_pop;
}



bool xtsc_component::xtsc_queue::xtsc_queue_pop_if_multi_impl::nb_pop(sc_unsigned& element, u64& ticket) {
  if (m_queue.m_use_fifo) {
    if (!nb_can_pop()) {
      xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, 0, UNKNOWN, POP_FAILED, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                           m_queue.m_depth, false, m_queue.m_dummy);
      return false;
    }
    int dummy = 0;
    m_queue.m_jerk_fifos[m_port_num]->nb_read(dummy);
    element = *m_queue.m_jerk_buffers[m_port_num];
    ticket  =  m_queue.m_jerk_tickets[m_port_num];
    XTSC_INFO(m_queue.m_text, "Popped (ticket=" << ticket << "): 0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3) <<
                              " (Port #" << m_port_num << ")");
    xtsc_log_queue_event(m_queue.m_binary, INFO_LOG_LEVEL, ticket, UNKNOWN, POP, UNKNOWN_PC, m_queue.m_fifo.num_available(),
                         m_queue.m_depth, m_queue.m_log_data_binary, element);
    m_queue.m_push_pop_event.notify(SC_ZERO_TIME);
    return true;
  }
  else {
    ticket = 0ULL;
    if (m_queue.m_pop_file) {
      if (m_queue.m_has_pop_file_element) {
        element = m_queue.m_pop_file_element;
        XTSC_INFO(m_queue.m_text, "Popped from file 0x" << element.to_string(SC_HEX).substr(m_queue.m_width1%4 ? 2 : 3) <<
                                  " (Port #" << m_port_num << ")");
        m_queue.get_next_pop_file_element();
        return true;
      }
      else {
        return false;
      }
    }
    else {
      ostringstream oss;
      oss << "nb_pop() called for xtsc_queue '" << m_queue.name() << "', but no pop_file was provided at construction time.";
      throw xtsc_exception(oss.str());
    }
  }
}



void xtsc_component::xtsc_queue::xtsc_queue_pop_if_multi_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to '" << m_queue.name() << "' xtsc_queue::m_consumers[" << m_port_num << "] export: "
        << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_queue.m_text, "Binding '" << port.name() << "' to xtsc_queue::m_consumers[" << m_port_num << "]");
  m_p_port = &port;
}




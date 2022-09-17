// Copyright (c) 2005-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


#include "xtsc/xtsc_dma_engine.h"
#include <string>
#include <sstream>

using namespace std;
using namespace sc_core;
using namespace log4xtensa;
using namespace xtsc;




xtsc_component::xtsc_dma_engine::xtsc_dma_engine(sc_module_name module_name, const xtsc_dma_engine_parms& dma_parms) :
  xtsc_memory       (module_name, dma_parms),
  m_request_port    ("m_request_port"),
  m_respond_export  ("m_respond_export"),
  m_respond_impl    ("m_respond_impl", *this)
{

  m_reg_base_address    = dma_parms.get_u32("reg_base_address");
  m_read_priority       = dma_parms.get_u32("read_priority");
  m_write_priority      = dma_parms.get_u32("write_priority");
  m_clear_notify_value  = dma_parms.get_bool("clear_notify_value");
  m_turbo               = dma_parms.get_bool("turbo");

  xtsc_address reg_end_address8 = m_reg_base_address + 256 + 256 - 1;
  if ((m_reg_base_address < m_start_address8) || (reg_end_address8 > m_end_address8)) {
    ostringstream oss;
    oss << kind() << " '" << name() << "': Minimum DMA register space (0x" << hex << setfill('0') << setw(8) << m_reg_base_address 
        << "-0x" << setw(8) << reg_end_address8 << ") is not completely contained with the xtsc_memory address range (0x"
        << setw(8) << m_start_address8 << "-0x" << m_end_address8 << ")";
    throw xtsc_exception(oss.str());
  }

  u32 clock_period = dma_parms.get_u32("clock_period");
  m_time_resolution = sc_get_time_resolution();
  u32 nacc_wait_time    = dma_parms.get_u32("nacc_wait_time");
  if (nacc_wait_time == 0xFFFFFFFF) {
    m_nacc_wait_time = m_clock_period;
  }
  else {
    m_nacc_wait_time = m_time_resolution * nacc_wait_time;
    if (m_nacc_wait_time > m_clock_period) {
      ostringstream oss;
      oss << kind() << " '" << name() << "': \"nacc_wait_time\"=" << m_nacc_wait_time << " exceeds clock period of " << m_clock_period;
      throw xtsc_exception(oss.str());
    }
  }

  m_clock_period_value = m_clock_period.value();
  u32 posedge_offset = dma_parms.get_u32("posedge_offset");
  if (posedge_offset == 0xFFFFFFFF) {
    m_posedge_offset = xtsc_get_system_clock_posedge_offset();
  }
  else {
    m_posedge_offset = posedge_offset * m_time_resolution;
  }
  if (m_posedge_offset >= m_clock_period) {
    ostringstream oss;
    oss << kind() << " '" << name() << "' parameter error:" << endl;
    oss << "\"posedge_offset\" (0x" << hex << posedge_offset << "=" << dec << posedge_offset << "=>" << m_posedge_offset
        << ") must be strictly less than \"clock_period\" (0x" << hex << clock_period << "=" << dec << clock_period << "=>"
        << m_clock_period << ")";
    throw xtsc_exception(oss.str());
  }
  m_has_posedge_offset = (m_posedge_offset != SC_ZERO_TIME);
  m_posedge_offset_plus_one = m_posedge_offset + m_clock_period;

  for (u32 i=0; i<16; ++i) {
    m_p_block_read_response[i] = 0;
  }

  SC_THREAD(dma_thread);

  m_respond_export(m_respond_impl);

  log4xtensa::LogLevel ll = xtsc_get_constructor_log_level();
  XTSC_LOG(m_text, ll, hex << " reg_base_address    = 0x" << setfill('0') << setw(8) << m_reg_base_address);
  XTSC_LOG(m_text, ll,        " read_priority       = "   << (u32) m_read_priority);
  XTSC_LOG(m_text, ll,        " write_priority      = "   << (u32) m_write_priority);
  XTSC_LOG(m_text, ll,        " clear_notify_value  = "   << boolalpha << m_clear_notify_value);
  XTSC_LOG(m_text, ll,        " turbo               = "   << boolalpha << m_turbo);
  if (posedge_offset == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll,        " posedge_offset      = 0xFFFFFFFF => " << m_posedge_offset.value() << " (" << m_posedge_offset << ")");
  } else {
  XTSC_LOG(m_text, ll,        " posedge_offset      = "   << posedge_offset << " (" << m_posedge_offset << ")");
  }
  if (nacc_wait_time == 0xFFFFFFFF) {
  XTSC_LOG(m_text, ll,        " nacc_wait_time      = 0xFFFFFFFF => " << m_nacc_wait_time.value() << " (" << m_nacc_wait_time << ")");
  } else {
  XTSC_LOG(m_text, ll,        " nacc_wait_time      = "   << nacc_wait_time << " (" << m_nacc_wait_time << ")");
  }

  m_deny_fast_access.push_back(m_reg_base_address);
  m_deny_fast_access.push_back(m_reg_base_address+3);
  XTSC_INFO(m_text, "No fast access to xtsc_dma_request num_descriptors register: 0x" << hex << m_reg_base_address <<
                    "-0x" << m_reg_base_address+3);

  reset();
}



void xtsc_component::xtsc_dma_engine::check_for_go_byte(xtsc_address address8, u32 size8, const u8 *buffer) {
  u32 offset = m_reg_base_address - address8;
  if (offset < size8) {
    if (m_busy) {
      ostringstream oss;
      oss << kind() << " \"" << name() << "\": num_descriptors register written while xtsc_dma_engine is already busy.";
      throw xtsc_exception(oss.str());
    }
    if (((size8 > 0) && (*(buffer+offset+0) != 0)) ||
        ((size8 > 1) && (*(buffer+offset+1) != 0)) ||
        ((size8 > 2) && (*(buffer+offset+2) != 0)) ||
        ((size8 > 3) && (*(buffer+offset+3) != 0)))
    {
      XTSC_DEBUG(m_text, "num_descriptors register written.  Notifying dma_thread.");
      m_dma_thread_event.notify(SC_ZERO_TIME);
    }
  }
}



void xtsc_component::xtsc_dma_engine::do_write(u32 port_num) {
  xtsc_memory::do_write(port_num);
  xtsc_request       *p_request     = &m_p_active_request_info[port_num]->m_request;
  xtsc_address        address8      = p_request->get_byte_address();
  u32                 size8         = p_request->get_byte_size();
  const u8           *buffer        = p_request->get_buffer();
  check_for_go_byte(address8, size8, buffer);
}



void xtsc_component::xtsc_dma_engine::do_block_write(u32 port_num) {
  xtsc_memory::do_block_write(port_num);
  xtsc_request *p_request = &m_p_active_request_info[port_num]->m_request;
  xtsc_address  address8  = p_request->get_byte_address();
  const u8     *buffer    = p_request->get_buffer();
  check_for_go_byte(address8, m_width8, buffer);
}



void xtsc_component::xtsc_dma_engine::sync_to_posedge(bool always_wait) {
  sc_time now = sc_time_stamp();
  sc_time phase_now = (now.value() % m_clock_period_value) * m_time_resolution;
  if (m_has_posedge_offset) {
    if (phase_now < m_posedge_offset) {
      phase_now += m_clock_period;
    }
    phase_now -= m_posedge_offset;
  }
  if (phase_now < m_posedge_offset) {
    wait(m_posedge_offset - phase_now);
  }
  else if (phase_now > m_posedge_offset) {
    wait(m_posedge_offset_plus_one - phase_now);
  }
  else if (always_wait) {
    wait(m_clock_period);
  }
}



void xtsc_component::xtsc_dma_engine::reset(bool hard_reset) {
  m_num_block_transfers = 0;
  m_p_single_response = 0;
  m_p_block_write_response = 0;
  m_block_read_response_count = 0;
  xtsc_memory::reset(hard_reset);
}



void xtsc_component::xtsc_dma_engine::dma_thread() {

  try {

    xtsc_dma_request req;

    while(true) {

      m_busy = false;
      wait(m_dma_thread_event);
      if (!read_u32(m_reg_base_address+0x00, false /* don't care */)) continue;
      m_busy = true;
      bool big_endian = false;
      req.num_descriptors         = read_u32(m_reg_base_address+0x00, false);
      if (req.num_descriptors > 255) {
        req.num_descriptors       = read_u32(m_reg_base_address+0x00, true);
        if (req.num_descriptors > 255) {
          ostringstream oss;
          oss << kind() << " \"" << name() << "\": num_descriptors cannot exceed 255"
              << " (DMA request address: 0x" << hex << setfill('0') << setw(8) << (m_reg_base_address+0x00) << ")";
          throw xtsc_exception(oss.str());
        }
        big_endian = true;
      }
      req.notify_address8       = read_u32(m_reg_base_address+0x04, big_endian);
      req.notify_value          = read_u32(m_reg_base_address+0x08, big_endian);
      req.turboxim_event_id     = read_u32(m_reg_base_address+0x0C, big_endian);


      if (req.notify_address8 & (m_width8-1)) {
        ostringstream oss;
        oss << kind() << " \"" << name() << "\": notify_address8=0x" << hex << req.notify_address8
            << " is not aligned to PIF width=" << dec << m_width8
            << " (DMA request address: 0x" << hex << setfill('0') << setw(8) << (m_reg_base_address+0x04) << ")";
        throw xtsc_exception(oss.str());
      }

      XTSC_INFO(m_text, "DMA request:" << hex
          << " num_descriptors="        << req.num_descriptors
          << " notify_address8=0x"      << req.notify_address8
          << " notify_value=0x"        << req.notify_value
      );

      xtsc_address reg_end_address8 = m_reg_base_address + 256 + (256 * req.num_descriptors) - 1;
      if ((m_reg_base_address < m_start_address8) || (reg_end_address8 > m_end_address8)) {
        ostringstream oss;
        oss << kind() << " '" << name() << "': Required DMA register space (0x" << hex << setfill('0') << setw(8) << m_reg_base_address 
            << "-0x" << setw(8) << reg_end_address8 << ") is not completely contained with the xtsc_memory address range (0x"
            << setw(8) << m_start_address8 << "-0x" << m_end_address8 << ")";
        throw xtsc_exception(oss.str());
      }

      for (u32 N=1; N<=req.num_descriptors; ++N) {

        xtsc_dma_descriptor dsc;

        dsc.source_address8             = read_u32(m_reg_base_address+(0x100*N)+0x00, big_endian);
        dsc.destination_address8        = read_u32(m_reg_base_address+(0x100*N)+0x04, big_endian);
        dsc.size8                       = read_u32(m_reg_base_address+(0x100*N)+0x08, big_endian);
        dsc.num_transfers               = read_u32(m_reg_base_address+(0x100*N)+0x0C, big_endian);

        // Sanity checks
        if ((dsc.num_transfers != 1) &&
            (dsc.num_transfers != 2) &&
            (dsc.num_transfers != 4) &&
            (dsc.num_transfers != 8) &&
            (dsc.num_transfers != 16))
        {
          ostringstream oss;
          oss << kind() << " \"" << name() << "\": num_transfers=" << dsc.num_transfers << " is invalid (must be 1|2|4|8|16)"
              << " (descriptor address: 0x" << hex << setfill('0') << setw(8) << (m_reg_base_address+(0x100*N+0x0C)) << ")";
          throw xtsc_exception(oss.str());
        }
        if (dsc.size8 & (m_width8*dsc.num_transfers-1)) {
          ostringstream oss;
          oss << kind() << " \"" << name() << "\": size8=0x" << hex << dsc.size8
              << " is not aligned to the block transfer size=0x" << (m_width8*dsc.num_transfers)
              << " (descriptor address: 0x" << hex << setfill('0') << setw(8) << (m_reg_base_address+(0x100*N+0x08)) << ")";
          throw xtsc_exception(oss.str());
        }
        if (dsc.source_address8 & (m_width8*dsc.num_transfers-1)) {
          ostringstream oss;
          oss << kind() << " \"" << name() << "\": source_address8=0x" << hex << dsc.source_address8
              << " is not aligned to the block transfer size=0x" << (m_width8*dsc.num_transfers)
              << " (descriptor address: 0x" << hex << setfill('0') << setw(8) << (m_reg_base_address+(0x100*N+0x08)) << ")";
          throw xtsc_exception(oss.str());
        }
        if (dsc.destination_address8 & (m_width8*dsc.num_transfers-1)) {
          ostringstream oss;
          oss << kind() << " \"" << name() << "\": destination_address8=0x" << hex << dsc.destination_address8
              << " is not aligned to the block transfer size=0x" << (m_width8*dsc.num_transfers)
              << " (descriptor address: 0x" << hex << setfill('0') << setw(8) << (m_reg_base_address+(0x100*N+0x08)) << ")";
          throw xtsc_exception(oss.str());
        }

        XTSC_INFO(m_text, "Descriptor #" << N << "/" << req.num_descriptors << ":" << hex
            << " source_address8=0x"            << dsc.source_address8
            << " destination_address8=0x"       << dsc.destination_address8
            << " size8=0x"                      << dsc.size8
            << " num_transfers=0x"              << dsc.num_transfers
        );

        if (m_turbo) {
          use_turbo(dsc);
        }
        else if (dsc.num_transfers == 1) {
          use_single_transfers(dsc);
        }
        else {
          use_block_transfers(dsc);
        }

      }

      xtsc_response::status_t status = remote_write_u32(req.notify_address8, req.notify_value, big_endian);
      if (status != xtsc_response::RSP_OK) {
        ostringstream oss;
        oss << kind() << " \"" << name() << "\": unable to write notify address=0x" << hex << req.notify_address8
            << ".  status=" << xtsc_response::get_status_name(status);
        throw xtsc_exception(oss.str());
      }
      if (m_clear_notify_value) {
        remote_write_u32(req.notify_address8, 0, big_endian);
      }

      if (req.turboxim_event_id) {
        xtsc_fire_turboxim_event_id(req.turboxim_event_id);
      }

      m_busy = false;
      XTSC_INFO(m_text, "DMA done");

    }

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught in SC_THREAD of " << kind() << " '" << name() << "'." << endl;
    oss << "what(): " << error.what() << endl;
    xtsc_log_multiline(m_text, FATAL_LOG_LEVEL, oss.str(), 2);
    throw;
  }

}



// Use nb_peek/nb_poke
void xtsc_component::xtsc_dma_engine::use_turbo(xtsc_dma_descriptor &dsc) {
  u32 chunk_size = dsc.num_transfers * m_width8;
  while (dsc.size8) {
    m_request_port->nb_peek(dsc.source_address8, chunk_size, m_data);
    m_request_port->nb_poke(dsc.destination_address8, chunk_size, m_data);
    dsc.source_address8 += chunk_size;
    dsc.destination_address8 += chunk_size;
    dsc.size8 -= chunk_size;
  }
}



// Use non-block READ/WRITE
void xtsc_component::xtsc_dma_engine::use_single_transfers(xtsc_dma_descriptor &dsc) {
  while (dsc.size8) {
    xtsc_response::status_t status = remote_read(dsc.source_address8, m_width8, 0xFFFF, m_data);
    if (status != xtsc_response::RSP_OK) {
      ostringstream oss;
      oss << kind() << " \"" << name() << "\": unable to read source address=0x" << hex << dsc.source_address8
          << ".  status=" << xtsc_response::get_status_name(status);
      throw xtsc_exception(oss.str());
    }
    wait(m_clock_period);
    status = remote_write(dsc.destination_address8, m_width8, 0xFFFF, m_data);
    if (status != xtsc_response::RSP_OK) {
      ostringstream oss;
      oss << kind() << " \"" << name() << "\": unable to write destination address=0x" << hex << dsc.destination_address8
          << ".  status=" << xtsc_response::get_status_name(status);
      throw xtsc_exception(oss.str());
    }
    dsc.source_address8 += m_width8;
    dsc.destination_address8 += m_width8;
    dsc.size8 -= m_width8;
  }
}



// Use BLOCK_READ/BLOCK_WRITE
void xtsc_component::xtsc_dma_engine::use_block_transfers(xtsc_dma_descriptor &dsc) {
  m_num_block_transfers = dsc.num_transfers;
  xtsc_response::status_t status;
  while (dsc.size8) {
    sync_to_posedge(true);
    m_request.initialize(xtsc_request::BLOCK_READ,      // type
                         dsc.source_address8,           // address
                         m_width8,                      // size
                         0,                             // tag  (0 => XTSC assigns tag)
                         dsc.num_transfers,             // num_transfers
                         0xFFFF,                        // byte_enables (ignored)
                         true,                          // last_transfer
                         0,                             // route_id
                         m_block_read_id,               // id
                         m_read_priority                // priority
                         );

    u32 tries = 0;
    do {
      if (m_p_block_read_response[0]) {
        delete m_p_block_read_response[0];
        m_p_block_read_response[0] = 0;
      }
      tries += 1;
      XTSC_INFO(m_text, m_request << " Try #" << tries);
      m_block_read_response_count = 0;
      m_request_port->nb_request(m_request);
      wait(m_clock_period);
    } while (m_p_block_read_response[0] && (m_p_block_read_response[0]->get_status() == xtsc_response::RSP_NACC));

    m_block_write_sent_count = 0;
    u64 tag = 0;
    while (m_block_write_sent_count < m_num_block_transfers) {
      while (m_block_write_sent_count >= m_block_read_response_count) {
        wait(m_block_read_response_available_event);
        XTSC_DEBUG(m_text, "use_block_transfers 0x" << hex << dsc.source_address8 << ": tag=" << dec << m_request.get_tag() <<
                           " got m_block_read_response_available_event");
      }
      sc_time req_net = m_p_block_read_response_time[m_block_write_sent_count] + m_clock_period;
      while (sc_time_stamp() < req_net) {
        sync_to_posedge(true);
      }

      status = m_p_block_read_response[m_block_write_sent_count]->get_status();
      if (status != xtsc_response::RSP_OK) {
        ostringstream oss;
        oss << kind() << " \"" << name() << "\": unable to block read source address=0x" << hex << dsc.source_address8
            << ".  status=" << xtsc_response::get_status_name(status);
        throw xtsc_exception(oss.str());
      }

      if (m_block_write_sent_count == 0) {
        m_request.initialize(xtsc_request::BLOCK_WRITE,     // type
                             dsc.destination_address8,      // address
                             m_width8,                      // size
                             0,                             // tag  (0 => XTSC assigns tag)
                             dsc.num_transfers,             // num_transfers
                             0xFFFF,                        // byte_enables (ignored)
                             false,                         // last_transfer
                             0,                             // route_id
                             m_block_write_id,              // id
                             m_write_priority               // priority
                             );
        tag = m_request.get_tag();
      }
      else {
        bool last = ((m_block_write_sent_count + 1) == m_num_block_transfers);
        m_request.initialize(tag,                           // tag
                             dsc.destination_address8,      // address
                             m_width8,                      // size
                             dsc.num_transfers,             // num_transfers
                             last,                          // last_transfer
                             0,                             // route_id
                             m_block_write_id,              // id
                             m_write_priority               // priority
                             );
      }

      memcpy(m_request.get_buffer(), m_p_block_read_response[m_block_write_sent_count]->get_buffer(), m_width8);
      u32 tries = 0;
      do {
        if (m_p_block_write_response) {
          delete m_p_block_write_response;
          m_p_block_write_response = 0;
        }
        tries += 1;
        XTSC_INFO(m_text, m_request << "Try #" << tries);
        m_request_port->nb_request(m_request);
        wait(m_clock_period);
      } while (m_p_block_write_response && (m_p_block_write_response->get_status() == xtsc_response::RSP_NACC));

      m_block_write_sent_count += 1;
      dsc.destination_address8 += m_width8;

    }

    if (!m_p_block_write_response) {
      wait(m_block_write_response_available_event);
    }

    status = m_p_block_write_response->get_status();
    if (status != xtsc_response::RSP_OK) {
      ostringstream oss;
      oss << kind() << " \"" << name() << "\": unable to block write destination address=0x" << hex << dsc.destination_address8
          << ".  status=" << xtsc_response::get_status_name(status);
      throw xtsc_exception(oss.str());
    }


    u32 bytes_in_block = m_width8 * m_num_block_transfers;
    dsc.source_address8 += bytes_in_block;
    dsc.size8 -= bytes_in_block;

  }
  m_num_block_transfers = 0;
}



xtsc_response::status_t xtsc_component::xtsc_dma_engine::remote_read(xtsc_address       address8,
                                                                     u32                size8,
                                                                     xtsc_byte_enables  byte_enables,
                                                                     u8                *buffer)
{
  m_request.initialize(xtsc_request::READ,      // type
                       address8,                // address8
                       size8,                   // size8
                       0,                       // tag  (0 => XTSC assigns)
                       1,                       // num_transfers
                       byte_enables,            // byte_enables
                       true,                    // last_transfer
                       0,                       // route_id
                       m_read_id,               // id
                       m_read_priority          // priority
                       );
  u32 tries = 0;
  do {
    tries += 1;
    XTSC_INFO(m_text, m_request << "Try #" << tries);
    m_request_port->nb_request(m_request);
    wait(m_single_response_available_event);
    XTSC_DEBUG(m_text, "remote_read 0x" << hex << address8 << ": tag=" << dec << m_request.get_tag() <<
                       " got m_single_response_available_event");
  } while (m_p_single_response->get_status() == xtsc_response::RSP_NACC);

  if (m_p_single_response->get_status() == xtsc_response::RSP_OK) {
    memcpy(buffer, m_p_single_response->get_buffer(), size8);
  }

  return m_p_single_response->get_status();
}



xtsc_response::status_t xtsc_component::xtsc_dma_engine::remote_write_u32(xtsc_address address8, u32 data, bool big_endian) {
  u8 buffer[4];
  u8 *p_data = reinterpret_cast<u8*>(&data);
  xtsc_byte_enables byte_enables = 0x000F;
  if (big_endian) {
    for (int i=0; i<4; i++) {
      buffer[i] = p_data[3-i];
    }
    p_data = buffer;
  }
  return remote_write(address8, 4, byte_enables, p_data);
}



xtsc_response::status_t xtsc_component::xtsc_dma_engine::remote_write(xtsc_address      address8,
                                                                      u32               size8,
                                                                      xtsc_byte_enables byte_enables,
                                                                      u8               *buffer)
{
  m_request.initialize(xtsc_request::WRITE,     // type
                       address8,                // address8
                       size8,                   // size8
                       0,                       // tag (0 => XTSC assigns)
                       1,                       // num_transfers
                       byte_enables,            // byte_enables
                       true,                    // last_transfer
                       0,                       // route_id
                       m_write_id,              // id
                       m_write_priority         // priority
                       );
  memcpy(m_request.get_buffer(), buffer, size8);
  u32 tries = 0;
  do {
    tries += 1;
    XTSC_INFO(m_text, m_request << "Try #" << tries);
    m_request_port->nb_request(m_request);
    wait(m_single_response_available_event);
    XTSC_DEBUG(m_text, "remote_write 0x" << hex << address8 << ": tag=" << dec << m_request.get_tag() << 
                       " got m_single_response_available_event");
  } while (m_p_single_response->get_status() == xtsc_response::RSP_NACC);

  return m_p_single_response->get_status();
}



bool xtsc_component::xtsc_dma_engine::xtsc_respond_if_impl::nb_respond(const xtsc_response& response) {
  XTSC_INFO(m_dma.m_text, response);
  u8 rsp_id = response.get_id();
  if ((rsp_id == m_dma.m_read_id) || (rsp_id == m_dma.m_write_id)) {
    if (m_dma.m_p_single_response) {
      delete m_dma.m_p_single_response;
      m_dma.m_p_single_response = 0;
    }
    m_dma.m_p_single_response = new xtsc_response(response);
    XTSC_DEBUG(m_dma.m_text, "nb_respond() called for tag=" << response.get_tag() <<
                             " notifying m_dma.m_single_response_available_event");
    m_dma.m_single_response_available_event.notify(SC_ZERO_TIME);
  }
  else if (rsp_id == m_dma.m_block_read_id) {
    if (m_dma.m_block_read_response_count >= m_dma.m_num_block_transfers) {
      ostringstream oss;
      oss << m_dma.kind() << " '" << m_dma.name() << "' nb_respond(): Received " << (m_dma.m_block_read_response_count+1)
          << " BLOCK_READ responses.  " << m_dma.m_num_block_transfers << " were expected.";
      throw xtsc_exception(oss.str());
    }
    if (m_dma.m_p_block_read_response[m_dma.m_block_read_response_count]) {
      delete m_dma.m_p_block_read_response[m_dma.m_block_read_response_count];
      m_dma.m_p_block_read_response[m_dma.m_block_read_response_count] = 0;
    }
    m_dma.m_p_block_read_response[m_dma.m_block_read_response_count] = new xtsc_response(response);
    m_dma.m_p_block_read_response_time[m_dma.m_block_read_response_count] = sc_time_stamp();
    XTSC_DEBUG(m_dma.m_text, "nb_respond() called for tag=" << response.get_tag() <<
                             " notifying m_dma.m_block_read_response_available_event");
    m_dma.m_block_read_response_available_event.notify(SC_ZERO_TIME);
    m_dma.m_block_read_response_count += 1;
  }
  else if (rsp_id == m_dma.m_block_write_id) {
    if (m_dma.m_p_block_write_response) {
      delete m_dma.m_p_block_write_response;
      m_dma.m_p_block_write_response = 0;
    }
    m_dma.m_p_block_write_response = new xtsc_response(response);
    XTSC_DEBUG(m_dma.m_text, "nb_respond() called for tag=" << response.get_tag() <<
                             " notifying m_dma.m_block_write_response_available_event");
    m_dma.m_block_write_response_available_event.notify(SC_ZERO_TIME);
  }
  else {
    ostringstream oss;
    oss << m_dma.kind() << " '" << m_dma.name() << "' nb_respond(): Got response with unsupported id=" << (u32) rsp_id;
    throw xtsc_exception(oss.str());
  }
  return true;
}



void xtsc_component::xtsc_dma_engine::xtsc_respond_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to xtsc_dma_engine '" << m_dma.name() << "' m_respond_export: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_dma.m_text, "Binding '" << port.name() << "' to xtsc_dma_engine::m_respond_export");
  m_p_port = &port;
}




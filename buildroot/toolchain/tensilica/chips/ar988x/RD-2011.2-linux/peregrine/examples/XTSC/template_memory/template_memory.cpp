// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include <iostream>
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_fast_access.h>
#include "template_memory.h"

using namespace std;
using namespace sc_core;
using namespace log4xtensa;
using namespace xtsc;



xtsc_component::template_memory::template_memory(sc_module_name module_name, const template_memory_parms& memory_parms) :
  sc_module             (module_name),
  m_request_export      ("m_request_export"),
  m_respond_port        ("m_respond_port"),
  m_request_impl        ("m_request_impl", *this),
  m_text                (TextLogger::getInstance(name()))
{

  // Initialize remaining members
  m_busy        = false;
  m_width8      = memory_parms.get_u32("byte_width");
  m_fill_byte   = memory_parms.get_int("fill_byte");

  // Get the response time as a u32 and save it as an sc_time
  u32 response_time = memory_parms.get_u32("response_time");
  m_response_time = response_time * sc_get_time_resolution();

  // Tell SystemC to run this funtion in a SystemC thread process
  SC_THREAD(response_thread);

  // Bind the export to the implementation
  m_request_export(m_request_impl);

  // Log our construction
  XTSC_INFO(m_text, "Constructed template_memory '" << name() << "':");
  XTSC_INFO(m_text, " byte_width         =   " << m_width8);
  XTSC_INFO(m_text, " fill_byte          =   " << m_fill_byte);
  XTSC_INFO(m_text, " response_time      =   " << response_time << " (" << m_response_time << ")");
}



xtsc_component::template_memory::~template_memory(void) {
  // Do any required clean-up here
}



void xtsc_component::template_memory::xtsc_request_if_impl::nb_peek(xtsc_address address8, u32 size8, u8 *buffer) {
  // Log the peek
  XTSC_VERBOSE(m_memory.m_text, "nb_peek: address=0x" << hex << address8 << " size=" << dec << size8);

  // Fill the peek buffer with the fill byte
  memset(buffer, m_memory.m_fill_byte, size8);
}



void xtsc_component::template_memory::xtsc_request_if_impl::nb_poke(xtsc_address address8, u32 size8, const u8 *buffer) {
  // Log the call
  XTSC_VERBOSE(m_memory.m_text, "nb_poke: address=0x" << hex << address8 << " size=" << dec << size8);

  // Don't do anything with the poke data
}



bool xtsc_component::template_memory::xtsc_request_if_impl::nb_fast_access(xtsc_fast_access_request &request) {
  // Log the call
  XTSC_VERBOSE(m_memory.m_text, "nb_fast_access: using peek/poke");

  request.allow_peek_poke_access();
  return true;
}



void xtsc_component::template_memory::xtsc_request_if_impl::nb_request(const xtsc_request& request) {
  // Log the request
  XTSC_INFO(m_memory.m_text, request);

  // Can we accept the request at this time?
  if (m_memory.m_busy) {
    // No. We're already busy.  Create an RSP_NACC response.
    xtsc_response response(request, xtsc_response::RSP_NACC, true);
    // Log the response
    XTSC_INFO(m_memory.m_text, response);
    // Send the response
    m_memory.m_respond_port->nb_respond(response);
  }
  else {
    // Yes.  We accept this request, so now we're busy.
    m_memory.m_busy = true;
    // Create our copy of the request
    m_memory.m_active_request = request;
    // Notify response_thread
    m_memory.m_response_event.notify(m_memory.m_response_time);
  }
}



void xtsc_component::template_memory::xtsc_request_if_impl::register_port(sc_port_base& port, const char *if_typename) {
  if (m_p_port) {
    ostringstream oss;
    oss << "Illegal multiple binding detected to template_memory '" << m_memory.name() << "' m_request_export: " << endl;
    oss << "  " << port.name() << endl;
    oss << "  " << m_p_port->name();
    throw xtsc_exception(oss.str());
  }
  XTSC_INFO(m_memory.m_text, "Binding '" << port.name() << "' to '" << m_memory.name() << ".m_request_export'");
  m_p_port = &port;
}



void xtsc_component::template_memory::response_thread(void) {

  // A try/catch block in sc_main will not catch an exception thrown from
  // an SC_THREAD, so we'll catch them here, log them, then rethrow them.
  try {

    // Loop forever
    while (true) {

      // Tell nb_request that we're no longer busy
      m_busy = false;

      // Wait for nb_request to tell us there's something to do 
      wait(m_response_event);

      // Create response from request
      xtsc_response response(m_active_request, xtsc_response::RSP_OK);

      // Handle request according to its type
      switch (m_active_request.get_type()) {

        // READ: Fill the buffer and send a single response
        case xtsc_request::READ: {
          memset(response.get_buffer(), m_fill_byte, m_active_request.get_byte_size());
          send_response(response);
          break;
        }

        // BLOCK_READ: Fill the buffer and send the requested number of responses
        case xtsc_request::BLOCK_READ: {
          memset(response.get_buffer(), m_fill_byte, m_width8);
          u32 num_transfers = m_active_request.get_num_transfers();
          for (u32 i=1; i<=num_transfers; ++i) {
            bool last = (i == num_transfers);
            response.set_last_transfer(last);
            send_response(response);
            if (!last) {
              wait(m_response_time);
            }
          }
          break;
        }

        // RCW: Ignore write data and only send a response to the last transfer
        case xtsc_request::RCW: {
          if (m_active_request.get_last_transfer()) {
            memset(response.get_buffer(), m_fill_byte, 4);
            send_response(response);
          }
          break;
        }

        // WRITE:  Ignore the data and send a single response
        case xtsc_request::WRITE: {
          send_response(response);
          break;
        }


        // BLOCK_WRITE: Ignore data and only send a response to the last transfer
        case xtsc_request::BLOCK_WRITE: {
          if (m_active_request.get_last_transfer()) {
            send_response(response);
          }
          break;
        }

        // We covered all the cases, but just in case . . .
        default: {
          ostringstream oss;
          oss << "Unsupported request type=" << m_active_request.get_type_name();
          throw xtsc_exception(oss.str());
        }
      }
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



void xtsc_component::template_memory::send_response(xtsc_response &response) {
  // Log the response
  XTSC_INFO(m_text, response);
  
  // Send the response until it is accepted
  while (!m_respond_port->nb_respond(response)) {
    XTSC_INFO(m_text, response << " <-- REJECTED");
    wait(m_response_time);
  }
}




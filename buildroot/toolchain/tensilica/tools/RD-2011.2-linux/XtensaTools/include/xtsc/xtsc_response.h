#ifndef _xtsc_response_h_
#define _xtsc_response_h_

// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <xtsc/xtsc.h>
#include <xtsc/xtsc_request.h>


namespace xtsc {


/**
 * Class representing a PIF, XLMI, local memory, or inbound PIF response transfer.
 *
 * The general 2-step procedure to create a response is:
 *  -# Construct the xtsc_response object using an xtsc_request object.
 *  -# Use the set_buffer() or get_buffer() methods to fill in the data payload.  This
 *     step is only needed for responses to READ, BLOCK_READ, BURST_READ, RCW, and SNOOP
 *     request types.
 *
 * For protocol and timing information specific to xtsc_core, see
 * xtsc_core::Information_on_memory_interface_protocols.
 *
 * @see xtsc_respond_if
 * @see xtsc_request_if
 * @see xtsc_request
 * @see xtsc_core::Information_on_memory_interface_protocols.
 * @see xtsc_component::xtsc_arbiter
 * @see xtsc_component::xtsc_dma_engine
 * @see xtsc_component::xtsc_master
 * @see xtsc_component::xtsc_mmio
 * @see xtsc_component::xtsc_memory
 * @see xtsc_component::xtsc_router
 * @see xtsc_component::xtsc_slave
 */
class XTSC_API xtsc_response {
public:

  /**
   *  Enumeration used to identify bus errors and busy information.
   */
  typedef enum status_t {
    RSP_OK                  = 0,        ///< Okay
    RSP_ADDRESS_ERROR       = 1,        ///< Address error
    RSP_DATA_ERROR          = 2,        ///< Data error
    RSP_ADDRESS_DATA_ERROR  = 3,        ///< Address and data error
    RSP_NACC                = 4         ///< Transaction not accepted (busy)
  } status_t;


  /**
   * Enumeration used to identify cache coherence information.
   */
  typedef enum coherence_t {
    INVALID            = 0,     ///<  Non-coherent response, May Be Exclusive response, or Invalid snoop response
    SHARED             = 1,     ///<  Must Be Shared response or Shared snoop response
  } coherence_t;



  /**
   * Constructor.  A response is always constructed starting with a request.
   *
   * @param     request         The request that this response is in response to.
   * @param     status          The response status.  See status_t.
   * @param     last_transfer   True if this is the last transfer of the response
   *                            sequence, otherwise false.  For READ, WRITE, 
   *                            BLOCK_WRITE, BURST_WRITE, and RCW, this is always true.
   *                            For BLOCK_READ, BURST_READ, and SNOOP, this is false
   *                            except for last response transfer in the sequence of 
   *                            BLOCK_READ, BURST_READ, or SNOOP response transfers.
   */
  xtsc_response(const xtsc_request& request, status_t status = RSP_OK, bool last_transfer = true);


  /**
   * Get the starting byte address.  The starting byte address is determined from the
   * xtsc_request object used to create this response and it cannot be changed.
   *
   * @Note This address does not change for multiple BLOCK_READ, BURST_READ, or SNOOP
   *       responses.
   */
  xtsc_address get_byte_address() const { return m_address8; }


  /**
   * Get the number of bytes of data in this response's buffer.  This is an 
   * artificial field (i.e. not in real hardware) used for logging/debugging.
   */
  u32 get_byte_size() const { return m_size8; }


  /**
   * Set the response status.
   *
   * @see status_t
   */
  void set_status(status_t status) { m_status = status; }


  /**
   * Get the response status.
   *
   * @see status_t
   */
  status_t get_status() const { return m_status; }


  /**
   * Get a c-string corresponding to this response's status.
   */
  const char *get_status_name() const;


  /**
   * Get a c-string corresponding to the specified response status.
   *
   * @param     status          The status whose c-string is desired.
   */
  static const char *get_status_name(status_t status);


  /**
   * Set whether or not this is the last transfer of the response.
   *
   * @param     last_transfer   True if this is the last transfer of the response
   *                            sequence, otherwise false.  For READ, WRITE, 
   *                            BLOCK_WRITE, BURST_WRITE, and RCW, this is always true.
   *                            For BLOCK_READ, BURST_READ, and SNOOP, this is false
   *                            except for last response transfer in the sequence of 
   *                            BLOCK_READ, BURST_READ, or SNOOP response transfers.
   */
  void set_last_transfer(bool last_transfer) { m_last_transfer = last_transfer; }


  /**
   * Get whether or not this is the last transfer of the response.
   *
   * @see set_last_transfer
   */
  bool get_last_transfer() const { return m_last_transfer; }


  /**
   * Return true if this is a snoop response, else return false;
   */
  bool is_snoop() const { return m_snoop; }


  /**
   * Set whether or not this is a snoop response with data.
   *
   * @param     snoop_data      True if this is a snoop response AND it has data.
   */
  void set_snoop_data(bool snoop_data) { m_snoop_data = snoop_data; }


  /**
   * Return true if this is a snoop response with data, else return false.
   *
   * @see set_snoop_data
   */
  bool has_snoop_data() const { return m_snoop_data; }


  /**
   * Set the cache coherence information of this response.
   *
   * @param     coherence       The coherence information of this response.
   */
  void set_coherence(coherence_t coherence) { m_coherence = coherence; }


  /**
   * Get the cache coherence information of this response.
   *
   * @see set_coherence
   */
  coherence_t get_coherence() const { return m_coherence; }


  /**
   * Set the processor program counter (PC) associated with this request.
   *
   * @param     pc      The PC associated with this request.  If no meaningful
   *                    PC can be associated with the request use 0xFFFFFFFF.
   *                    This signal is not in the hardware, but is provided for
   *                    debugging and logging purposes.
   */
  void set_pc(xtsc_address pc) { m_pc = pc; }


  /**
   * Get the processor program counter (PC) associated with this request.
   *
   * @see set_pc
   */
  xtsc_address get_pc() const { return m_pc; }


  /**
   * Set the route ID.  
   *
   * @param     route_id        Arbiters add bits to this field as needed to be able 
   *                            to route the return response.  Terminal devices must 
   *                            echo this field back verbatim in the response (this 
   *                            is taken care of automatically in the constructor, so
   *                            terminal devices do not need to call this method).  
   */
  void set_route_id(u32 route_id) { m_route_id = route_id; }


  /**
   * Get the route ID.
   *
   * @see set_route_id
   */
  u32 get_route_id() const { return m_route_id; }


  /**
   * Set the transaction ID.
   *
   * @param     id              The transaction ID.  Terminal devices must echo this 
   *                            field back verbatim in the response (this is taken
   *                            care of automatically in the constructor, so terminal
   *                            devices typically do not need to call this method).
   *                            Master devices are free to use this field to support
   *                            multiple outstanding PIF requests.
   *
   */
  void set_id(u8 id) { m_id = id; }


  /**
   * Get the transaction ID.
   *
   * @see set_id
   */
  u8 get_id() const { return m_id; }


  /**
   * Set the priority.
   *
   * Hardware only supports priority values of 0-3 (i.e. 2 bits).
   *  \verbatim
     Value  Meaning
     -----  -------
      0     Low
      1     Medium low
      2     Medium high
      3     High
      \endverbatim
   * The Xtensa LX processor issues all PIF requests at medium-high (2) priority and
   * ignores the priority bits of PIF responses.  For responses to inbound PIF requests,
   * the Xtenasa LX processor sets the response priority equal to the request priority.
   * Default = The priority in the request used to create this response.
   */
  void set_priority(u8 priority) { m_priority = priority; }


  /**
   * Get the priority.
   * @see set_priority
   */
  u8 get_priority() const { return m_priority; }


  /**
   * Set the response's transfer buffer (payload).  Used only for READ,
   * BLOCK_READ, BURST_READ, RCW, and SNOOP.
   * Data is arranged in the buffer as follows:
   * Let address8 be the address returned by xtsc_request::get_byte_address().
   * Let size8 be the transfer size returned by xtsc_response::get_byte_size().
   *  \verbatim
     The byte corresponding to address8+0       is in buffer[0].
     The byte corresponding to address8+1       is in buffer[1].
        . . .
     The byte corresponding to address8+size8-1 is in buffer[size8-1].
      \endverbatim
   * This format applies regardless of host and target endianess.
   *
   */
  void set_buffer(const u8 *buffer) { memcpy(m_buffer, buffer, m_size8); }


  /**
   * Get a pointer to the response's transfer data suitable for reading (but not
   * writing) the data.  Used only for READ, BLOCK_READ, BURST_READ, RCW, and SNOOP.
   *
   * @see set_buffer()
   */
  const u8 *get_buffer() const { return m_buffer; }


  /**
   * Get a pointer to the response's transfer data suitable either for reading 
   * or writing the data.  Used only for READ, BLOCK_READ, BURST_READ, RCW, and SNOOP.
   *
   * The buffer size is 64 bytes to accommodate the widest possible Xtensa 
   * memory interface; however, you should only use the first N bytes where
   * N is the size of the actual memory interface in use.  
   *
   * Caution:  Writing past the 16th byte results in undefined bad behavior.
   *
   * @see set_buffer()
   */
  u8 *get_buffer() { return m_buffer; }


  /**
   * Get this response's tag.  This is an artificial number (not in hardware) 
   * useful for correlating requests and responses in, for example, a log file.
   * For READ and WRITE, the tag is the same for a request and its corresponding
   * response.
   * For BLOCK_WRITE and BURST_WRITE, the tag is the same for all request transfers and
   * the single response transfer of the BLOCK_WRITE or BURST_WRITE sequence.
   * For BLOCK_READ, BURST_READ, and SNOOP, the tag is the same for the single request
   * transfer and all the response transfers in the block.
   * For RCW, the tag is the same for both request transfers and the single response
   * transfer.
   * A request that gets RSP_NACC maintains the same tag when the request is repeated.
   */
  u64 get_tag() const { return m_tag; }


  /**
   * This method dumps this response's info to the specified ostream object, optionally 
   * including data (if applicable).
   *
   * The format of the output is:
   * \verbatim
       tag=<Tag> pc=<PC> <Status>*[<Address>/<Coherence>/<ID>]: <Data>

       Where:
        <Tag>           is m_tag in decimal.
        <PC>            is m_pc in hexadecimal.
        <Status>        is RSP_OK|RSP_ADDRESS_ERROR|RSP_DATA_ERROR|
                        RSP_ADDRESS_DATA_ERROR|RSP_NACC.
        *               indicates m_last_transfer is true.
        <Address>       is m_address8 in hexadecimal.
        <Coherence>     is m_coherence in decimal.
        <ID>            is m_id in decimal.
        <Data>          is a colon followed by the contents of m_buffer in
                        hexadecimal (without leading '0x').  This field is only present
                        if dump_data is true AND m_status is RSP_OK AND m_size8 is non-
                        zero (indicating this is a response to a READ, BLOCK_READ,
                        BURST_READ, or RCW request or that m_snoop_data is true).
     \endverbatim
   *
   * @param     os              The ostream object to which the info should be dumped.
   *
   * @param     dump_data       If true and the response has data, then the response's buffer
   *                            contents are dumped.  Otherwise, the buffer is not dumped.
   */
  void dump(std::ostream& os = std::cout, bool dump_data = true) const;


  /**
   * Helper class to make it easy to dump xtsc_response to an ostream with or without data values
   *
   * @see show_data
   */
  class stream_dumper {
  public:
    void dump(std::ostream & os) const {
      if (m_p_response) m_p_response->dump(os, m_show_data);
    }
    static const xtsc_response *m_p_response;
    static bool                 m_show_data;
  };

  /**
   * This method makes it easy to dump an xtsc_response to an ostream
   * while programatically determining whether or not to include
   * the payload data (if any).
   *
   * Usage:
   *  \verbatim
             cout << response                  << endl;  // Show data
             cout << response.show_data(true)  << endl;  // Show data
             cout << response.show_data(false) << endl;  // Don't show data
      \endverbatim
   */
  const stream_dumper& show_data(bool show) const {
    stream_dumper::m_p_response = this;
    stream_dumper::m_show_data  = show;
    return m_stream_dumper;
  }


private:

  // Default construction is not allowed
  xtsc_response();

  xtsc_address          m_address8;                     ///< Starting byte address (artificial)
  u8                    m_buffer[xtsc_max_bus_width8];  ///< Data for READ, BLOCK_READ, BURST_READ, RCW, and SNOOP
  u32                   m_size8;                        ///< Byte size of each transfer (artificial)
  u32                   m_route_id;                     ///< Route ID for arbiters
  status_t              m_status;                       ///< Response status
  u8                    m_id;                           ///< Transaction ID
  u8                    m_priority;                     ///< Transaction priority
  bool                  m_last_transfer;                ///< True if last transfer of response
  bool                  m_snoop;                        ///< True if this is a snoop response, otherwise false
  bool                  m_snoop_data;                   ///< True if this is a snoop response with data, otherwise false
  coherence_t           m_coherence;                    ///< Cache Coherence information
  xtsc_address          m_pc;                           ///< Program counter associated with request (artificial)

  u64                   m_tag;                          ///< Unique tag per request-response set (artificial)
  static stream_dumper  m_stream_dumper;                ///< To assist with printing (dumping)

};


/**
 * Dump an xtsc_response object.
 *
 * This operator dumps an xtsc_response object using the 
 * xtsc_response::dump() method.
 *
 */
XTSC_API std::ostream& operator<<(std::ostream& os, const xtsc_response& response);


/**
 * Dump an xtsc_response object.
 *
 * This operator dumps an xtsc_response object using the
 * xtsc_response::stream_dumper::dump() method.
 *
 */
XTSC_API std::ostream& operator<<(std::ostream& os, const xtsc_response::stream_dumper& dumper);


} // namespace xtsc



#endif  // _xtsc_response_h_

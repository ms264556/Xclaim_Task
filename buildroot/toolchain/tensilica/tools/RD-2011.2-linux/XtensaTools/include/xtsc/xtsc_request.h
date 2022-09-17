#ifndef _xtsc_request_h_
#define _xtsc_request_h_

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


namespace xtsc {

/**
 * Class representing a PIF, XLMI, local memory, snoop, or inbound PIF request transfer.
 *
 * The general 2-step procedure to create a request is:
 *  -# Construct the xtsc_request object using the appropriate xtsc_request constructor
 *     for the type of request you want to create (see type_t).  These xtsc_request
 *     constructors all have parameters.
 *  -# Use the set_buffer() or get_buffer() methods to fill in the data payload.  This
 *     step is only needed for RCW, WRITE, BLOCK_WRITE, and BURST_WRITE request types.
 *
 * If you wish to preconstruct an empty xtsc_request object before you know the request
 * type (for example to create a pool of xtsc_request objects to improve performance),
 * use the xtsc_request constructor that takes no parameters.  When you are ready to use
 * the xtsc_request object:
 *  -# Call the the appropriate xtsc_request initialize() method for the type of request
 *     you want to create (see type_t).
 *  -# Use the set_buffer() or get_buffer() methods to fill in the data payload.  This
 *     step is only needed for RCW, WRITE, BLOCK_WRITE, and BURST_WRITE request types.
 *
 * If desired, the above two procedures can be mixed.  For example, create an RCW
 * xtsc_request using the main xtsc_request constructor and then, when the time comes to
 * create the second RCW request, call the special initialize() method meant for the
 * second RCW request.  This same technique can be used for BLOCK_WRITE and BURST_WRITE
 * transactions.
 *
 * For protocol and timing information specific to xtsc_core, see
 * xtsc_core::Information_on_memory_interface_protocols.
 *
 * @see xtsc_request_if
 * @see xtsc_respond_if
 * @see xtsc_response
 * @see xtsc_core::Information_on_memory_interface_protocols.
 * @see xtsc_component::xtsc_arbiter
 * @see xtsc_component::xtsc_dma_engine
 * @see xtsc_component::xtsc_master
 * @see xtsc_component::xtsc_mmio
 * @see xtsc_component::xtsc_memory
 * @see xtsc_component::xtsc_router
 * @see xtsc_component::xtsc_slave
 */
class XTSC_API xtsc_request {
public:


  /**
   * Enumeration used to identify the request type.
   *
   * @Note xtsc_core neither accepts nor generates burst transactions (BURST_READ and
   *       BURST_WRITE).
   */
  typedef enum type_t {
    READ               = 0x00,  ///<  Single read
    BLOCK_READ         = 0x10,  ///<  Block read (num_transfers = 2|4|8|16)
    BURST_READ         = 0x30,  ///<  Burst read (num_transfers = 2|3|4|5|6|7|8)
    RCW                = 0x50,  ///<  Read-conditional-write
    WRITE              = 0x80,  ///<  Write
    BLOCK_WRITE        = 0x90,  ///<  Block write (num_transfers = 2|4|8|16)
    BURST_WRITE        = 0xB0,  ///<  Burst write (num_transfers = 2|3|4|5|6|7|8)
    SNOOP              = 0x60,  ///<  Snoop request to an Xtensa config supporting data cache coherence
  } type_t;


  /**
   * Enumeration used to identify cache coherence information.
   */
  typedef enum coherence_t {
    INVALID            = 0,     ///<  Non-coherent request
    SHARED             = 1,     ///<  READ|BLOCK_READ request for shared or SNOOP request for shared
    EXCLUSIVE          = 2      ///<  READ|BLOCK_READ request for exclusive or SNOOP request for exclusive
  } coherence_t;


  /**
   * Constructor for an empty xtsc_request object used to create a pool of pre-allocated
   * xtsc_request objects.  Before using an xtsc_request object that was created with
   * this constructor, either assign another xtsc_request object to it or call one of
   * the initialize() methods on it.
   */
  xtsc_request();


  /**
   * Constructor for most kinds of xtsc_request objects.
   *
   * This constructor is used to create the following kinds of xtsc_request objects:
   *  - READ
   *  - BLOCK_READ
   *  - BURST_READ
   *  - RCW         (first transfer only)
   *  - WRITE
   *  - BLOCK_WRITE (first transfer only)
   *  - BURST_WRITE (first transfer only)
   *  - SNOOP
   *
   * @param     type            Type of request. See xtsc_request::type_t.
   * @param     address8        Byte address of request.  See set_byte_address().
   * @param     size8           Size in bytes of each transfer.  See set_byte_size().
   * @param     tag             The tag if it has already been assigned by the Xtensa
   *                            ISS.  If not, pass in 0 (the default) and a new non-zero
   *                            tag will be assigned (and can be obtained by calling
   *                            the get_tag() method).
   * @param     num_transfers   See set_num_transfers().
   * @param     byte_enables    See set_byte_enables().
   * @param     last_transfer   See set_last_transfer().
   * @param     route_id        The route ID.  See set_route_id().
   * @param     id              The transaction ID.  See set_id().
   * @param     priority        The transaction priority.  See set_priority().
   * @param     pc              The associated processor program counter. See set_pc().
   */
  xtsc_request( type_t                type,
                xtsc_address          address8,
                u32                   size8,
                u64                   tag             = 0,
                u32                   num_transfers   = 1,
                xtsc_byte_enables     byte_enables    = 0xFFFF,
                bool                  last_transfer   = true,
                u32                   route_id        = 0,
                u8                    id              = 0,
                u8                    priority        = 2,
                xtsc_address          pc              = 0xFFFFFFFF
              );


  /**
   * Constructor for second through last request of a BLOCK_WRITE.
   *
   * @param     tag             The tag from the first request of the BLOCK_WRITE
   *                            sequence.
   * @param     address8        Byte address of request.  This should increment
   *                            by size8 (bus width) for each request of the BLOCK_WRITE
   *                            sequence.
   * @param     size8           Size in bytes of each transfer.  For BLOCK_WRITE, this
   *                            is always equal to the bus width.
   * @param     num_transfers   The total number of BLOCK_WRITE request transfers.
   *                            This should be the same number as in the first 
   *                            request of the BLOCK_WRITE sequence.
   * @param     last_transfer   True if this is the last request of the BLOCK_WRITE
   *                            sequence. 
   * @param     route_id        The route ID.  See set_route_id().
   * @param     id              The transaction ID.  See set_id().
   * @param     priority        The transaction priority.  See set_priority().
   * @param     pc              The associated processor program counter. See set_pc().
   */
  xtsc_request( u64                   tag,
                xtsc_address          address8,
                u32                   size8,
                u32                   num_transfers,
                bool                  last_transfer,
                u32                   route_id        = 0,
                u8                    id              = 0,
                u8                    priority        = 2,
                xtsc_address          pc              = 0xFFFFFFFF
              );


  /**
   * Constructor for the second (last) request transfer of a RCW sequence.
   *
   * @param     tag             The tag from the first request of the RCW
   *                            sequence.
   * @param     address8        Byte address of request.  This should be the same
   *                            as the address of the first RCW request transfer.
   * @param     route_id        The route ID.  See set_route_id().
   * @param     id              The transaction ID.  See set_id().
   * @param     priority        The transaction priority.  See set_priority().
   * @param     pc              The associated processor program counter. See set_pc().
   *
   * @Note  This constructor sets the byte size to 4 and the byte enables to 0xF.  If
   *        you want an RCW request with a byte size other then 4, then call
   *        set_byte_size() after calling this constructor.  If you want byte enables
   *        other then 0xF, call set_byte_enables() after calling this constructor.
   *
   * @Note  If you use literals or non-byte integers for both id and priority, then at
   *        least one of them will need to be cast to a u8 to disambiguate this
   *        constructor from the BLOCK_WRITE constructor.  For example:
   *  \verbatim
               xtsc_request rcw2(tag, address8, 0, 0, (u8) 0, pc);
      \endverbatim
   */
  xtsc_request( u64                     tag,
                xtsc_address            address8,
                u32                     route_id        = 0,
                u8                      id              = 0,
                u8                      priority        = 2,
                xtsc_address            pc              = 0xFFFFFFFF
              );


  /**
   * Constructor for second through last request of a BURST_WRITE.
   *
   * @param     hw_address8     This should be equal to the lowest byte address enabled
   *                            by a byte enable in the first request of the sequence.
   *                            See get_hardware_address().
   * @param     tag             The tag from the first request of the BURST_WRITE
   *                            sequence.
   * @param     address8        Byte address of request.  This starts out bus-width-
   *                            aligned and should increment by the bus width for each
   *                            request of the BURST_WRITE sequence.
   * @param     size8           Size in bytes of each transfer.  This should be equal to
   *                            the bus width.  See set_byte_size().
   * @param     num_transfers   The total number of BURST_WRITE request transfers.
   *                            This should be the same number as in the first 
   *                            request of the BURST_WRITE sequence.
   * @param     transfer_num    The sequential number of this transfer (valid values are
   *                            2 through num_transfers).
   * @param     byte_enables    The bytes to be written.  See set_byte_enables().
   * @param     route_id        The route ID.  See set_route_id().
   * @param     id              The transaction ID.  See set_id().
   * @param     priority        The transaction priority.  See set_priority().
   * @param     pc              The associated processor program counter. See set_pc().
   */
  xtsc_request( xtsc_address          hw_address8,
                u64                   tag,
                xtsc_address          address8,
                u32                   size8,
                u32                   num_transfers,
                u32                   transfer_num,
                xtsc_byte_enables     byte_enables    = 0xFFFF,
                u32                   route_id        = 0,
                u8                    id              = 0,
                u8                    priority        = 2,
                xtsc_address          pc              = 0xFFFFFFFF
              );


  /**
   * Initializer for most kinds of xtsc_request objects.
   *
   * This method is used to initialize the following kinds of xtsc_request objects:
   *  - READ
   *  - BLOCK_READ
   *  - BURST_READ
   *  - RCW         (first transfer only)
   *  - WRITE
   *  - BLOCK_WRITE (first transfer only)
   *  - BURST_WRITE (first transfer only)
   *  - SNOOP
   *
   * See the documentation for the corresponding constructor.
   */
  void initialize(type_t                type,
                  xtsc_address          address8,
                  u32                   size8,
                  u64                   tag             = 0,
                  u32                   num_transfers   = 1,
                  xtsc_byte_enables     byte_enables    = 0xFFFF,
                  bool                  last_transfer   = true,
                  u32                   route_id        = 0,
                  u8                    id              = 0,
                  u8                    priority        = 2,
                  xtsc_address          pc              = 0xFFFFFFFF
                 );


  /**
   * Initializer for second through last request of a BLOCK_WRITE.
   *
   * See the documentation for the corresponding constructor.
   */
  void initialize(u64                   tag,
                  xtsc_address          address8,
                  u32                   size8,
                  u32                   num_transfers,
                  bool                  last_transfer,
                  u32                   route_id        = 0,
                  u8                    id              = 0,
                  u8                    priority        = 2,
                  xtsc_address          pc              = 0xFFFFFFFF
                 );


  /**
   * Initializer for the second (last) request transfer of a RCW sequence.
   *
   * See the documentation for the corresponding constructor.
   */
  void initialize(u64                   tag,
                  xtsc_address          address8,
                  u32                   route_id        = 0,
                  u8                    id              = 0,
                  u8                    priority        = 2,
                  xtsc_address          pc              = 0xFFFFFFFF
                 );


  /**
   * Initializer for second through last request of a BURST_WRITE.
   *
   * See the documentation for the corresponding constructor.
   */
  void initialize(xtsc_address          hw_address8,
                  u64                   tag,
                  xtsc_address          address8,
                  u32                   size8,
                  u32                   num_transfers,
                  u32                   transfer_num,
                  xtsc_byte_enables     byte_enables    = 0xFFFF,
                  u32                   route_id        = 0,
                  u8                    id              = 0,
                  u8                    priority        = 2,
                  xtsc_address          pc              = 0xFFFFFFFF
                );


  /**
   * Set the byte address.
   *
   * @param     address8        Byte address of request.  Should be size-aligned (that
   *                            is, address8 modulo m_size8 should be 0).  For
   *                            BLOCK_WRITE requests, address8 starts out aligned to
   *                            m_size8*num_transfers and increases by bus width
   *                            (m_size8) on each subsequent request transfer in the
   *                            sequence.  For BURST_WRITE requests, address8 should
   *                            start out bus width (m_size8) aligned and should
   *                            increase by the bus width (m_size8) on each subsequent
   *                            request transfer in the sequence.  For BLOCK_READ, 
   *                            BURST_READ, and SNOOP  requests, address8 should be
   *                            aligned to the bus width (m_size8); it does not have to
   *                            be aligned to m_size8*num_transfers (for BLOCK_READ,
   *                            this is to allow critical word first access to memory).
   *                            address8 is the same for both request transfers of an
   *                            RCW sequence.
   *
   * @Note  For a PIF interface, the address in a BLOCK_WRITE xtsc_request differs
   *        from the address in the PIF hardware specification.  In XTSC, the
   *        address changes with each request of a BLOCK_WRITE sequence to reflect
   *        the target address for that transfer.  In the hardware specification,
   *        the address for each transfer is constant and reflects the starting
   *        address of the block.  
   *
   * @Note  For a PIF interface, the address in a BURST_WRITE xtsc_request differs
   *        from the address in the PIF hardware specification.  In XTSC, the
   *        address starts out aligned with the bus width and increases by the bus width
   *        for each request of a BURST_WRITE sequence.  In the hardware specification,
   *        the address for all transfers in the sequence is constant and equal to the
   *        lowest address enabled by a byte enable in the first transfer.
   *
   * @Note  The address corresponding to the hardware specification is available by
   *        calling the get_hardware_address() method.
   *
   * @See get_byte_address().
   * @See get_hardware_address().
   */
  void set_byte_address(xtsc_address address8) { m_address8 = address8; }


  /**
   * Get the byte address.
   *
   * @see set_byte_address
   */
  xtsc_address get_byte_address() const { return m_address8; }


  /**
   * Get the byte address corresponding to the PIF hardware address signals.
   *
   * For all request types except BLOCK_WRITE and BURST_WRITE, this method returns the
   * same address as the get_byte_address() method.  For all BLOCK_WRITE requests in a
   * sequence, this method returns the address of the start of the block.  For the first
   * BURST_WRITE request in a sequence, this method returns the hardware address
   * inferred by the starting address and the byte enables; that is, the lowest byte
   * address enabled by a byte enable.  For all other BURST_WRITE requests, this method
   * returns the value set by the constructor or the initialize() method (which is
   * supposed to be the lowest byte address enabled in the first request of the
   * sequence).
   *
   * @see get_byte_address
   */
  xtsc_address get_hardware_address() const;


  /**
   * Set the size in bytes of each transfer.
   *
   * @param     size8           Size in bytes of each transfer.  For BLOCK_READ,
   *                            BLOCK_WRITE, BURST_READ, BURST_WRITE, and SNOOP, this is
   *                            always equal to the bus width.  For READ, WRITE, and
   *                            RCW, size8 must be a power of 2 less than or equal to
   *                            bus width.
   */
  void set_byte_size(u32 size8) { m_size8 = size8; }


  /**
   * Get the size in bytes of each transfer.
   *
   * @see set_byte_size()
   */
  u32 get_byte_size() const { return m_size8; }


  /**
   * Set the PIF request attributes of each transfer.
   *
   * @param     pif_attribute   PIF request attributes.
   */
  void set_pif_attribute(u32 pif_attribute) { m_pif_attribute = (pif_attribute == 0xFFFFFFFF ? 0xFFFFFFFF : (pif_attribute & 0xFFF)); }


  /**
   * Get the PIF request attributes of each transfer.
   *
   * @see set_pif_attribute()
   */
  u32 get_pif_attribute() const { return m_pif_attribute; }


  /**
   * Set the route ID.
   *
   * @param     route_id        Arbiters add bits to this field as needed to be able 
   *                            to route the return response.  Terminal devices must 
   *                            echo this field back verbatim in the response.  
   */
  void set_route_id(u32 route_id) { m_route_id = route_id; }


  /**
   * Get the route ID.
   *
   * @see set_route_id()
   */
  u32 get_route_id() const { return m_route_id; }


  /**
   * Set the request type. 
   *
   * @see type_t
   */
  void set_type(type_t type) { m_type = type; }


  /**
   * Get the request type.
   *
   * @see type_t
   */
  type_t get_type() const { return m_type; }


  /**
   * Get a c-string corresponding to this request's type.
   */
  const char *get_type_name() const;


  /**
   * Get a c-string corresponding to the specified request type.
   *
   * @param     type    The desired type.
   */
  static const char *get_type_name(type_t type);


  /**
   * Set the number of transfers.  
   *
   * @param     num_transfers   For READ and WRITE, this is 1.
   *                            For BLOCK_READ, this is the number of response 
   *                            transfers expected by this request (2|4|8|16).
   *                            For BURST_READ, this is the number of response 
   *                            transfers expected by this request (2-8).
   *                            For BLOCK_WRITE this is the number of request 
   *                            transfers in the BLOCK_WRITE sequence (2|4|8|16).  
   *                            For BURST_WRITE this is the number of request 
   *                            transfers in the BURST_WRITE sequence (2-8).  
   *                            For RCW, this is 2.
   *                            For SNOOP, this is the number of response with data
   *                            transfers expected by this request (1|2|4|8|16).
   */
  void set_num_transfers(u32 num_transfers) { m_num_transfers = num_transfers; }


  /**
   * Get the number of transfers.
   *
   * @see set_num_transfers
   */
  u32 get_num_transfers() const { return m_num_transfers; }


  /**
   * Get which request transfer this is in a sequence of request transfers.
   *
   * For READ, BLOCK_READ, BURST_READ, WRITE, and SNOOP, this always returns 1.  For
   * BLOCK_WRITE and BURST_WRITE, this returns which request transfer this is in the
   * sequence of BLOCK_WRITE or BURST_WRITE request transfers (starting with 1 and going
   * up to m_num_transfers).  For RCW, this returns 1 for the first request transfer and
   * 2 for the second request transfer.
   */
  u32 get_transfer_number() const;


  /**
   * Set the byte enables.
   *
   * @param     byte_enables    Let address8 be the address set by set_byte_address() or
   *                            an xtsc_request constructor or initialize() method.  For
   *                            READ, WRITE, RCW, and BURST_WRITE, bit 0 of byte_enables
   *                            applies to address8, bit 1 of byte_enables applies to
   *                            address8+1, ..., bit (size8-1) of byte_enables applies
   *                            to address8+size8-1.  For BLOCK_WRITE, BLOCK_READ,
   *                            BURST_READ, and SNOOP, byte_enables is not used.
   *                            For the middle requests in a BURST_WRITE sequence (that
   *                            is, requests that are neither the first nor the last in
   *                            the sequence), all bytes of the bus should be enabled.
   */
  void set_byte_enables(xtsc_byte_enables byte_enables) { m_byte_enables = byte_enables; }


  /**
   * Get the byte enables.
   *
   * @see set_byte_enables
   */
  xtsc_byte_enables get_byte_enables() const { return m_byte_enables; }


  /**
   * Set the transaction ID.
   *
   * @param     id              The transaction ID.  Terminal devices must echo this 
   *                            field back verbatim in the response.  Master devices
   *                            are free to use this field to support multiple
   *                            outstanding PIF requests.
   */
  void set_id(u8 id) { m_id = id; }


  /**
   * Get the transaction ID.
   *
   * @see set_id
   */
  u8 get_id() const { return m_id; }


  /**
   * Set the transaction priority.
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
   * ignores the priority bits of PIF responses.  Inbound PIF requests to instruction
   * RAMs always have priority over processor generated instruction fetches regardless
   * of the value set using the set_priority() method.  Inbound PIF requests to data
   * RAMs and XLMI will take priority over processor generated loads and stores if
   * priority is set to 3 (High).  A priority of 0, 1, or 2 will result in the inbound
   * PIF request getting access to the XLMI or data RAM when it is free.  If an inbound
   * PIF request with a priority less then 3 has been blocked for several cycles by
   * processor generated loads or stores, then bubbles will be inserted in the Xtensa
   * processor pipeline to allow the request to make forward progress.
   */
  void set_priority(u8 priority) { m_priority = priority; }


  /**
   * Get the transaction priority.
   * @see set_priority
   */
  u8 get_priority() const { return m_priority; }


  /**
   * Set whether or not this is the last transfer of the request.
   *
   * @param     last_transfer   True if this is the last transfer of the request.  For
   *                            READ, BLOCK_READ, BURST_READ, WRITE, and SNOOP, this should
   *                            always be true.  For all but the last transfer of a
   *                            BLOCK_WRITE, BURST_WRITE, or RCW sequence, this should
   *                            be false and for the last transfer it should be true.
   */
  void set_last_transfer(bool last_transfer) { m_last_transfer = last_transfer; }


  /**
   * Get whether or not this is the last transfer of the request.
   *
   * @see set_last_transfer
   */
  bool get_last_transfer() const { return m_last_transfer; }


  /**
   * Set whether or not this is request is for an instruction fetch.
   *
   * @param     instruction_fetch       True indicates this request is for an
   *                                    instruction fetch.
   *
   * @Note Typically m_instruction_fetch should be false if m_type is neither READ nor
   *       BLOCK_READ.
   */
  void set_instruction_fetch(bool instruction_fetch) { m_instruction_fetch = instruction_fetch; }


  /**
   * Get whether or not this is request is for an instruction fetch.
   *
   * @see set_instruction_fetch
   */
  bool get_instruction_fetch() const { return m_instruction_fetch; }


  /**
   * Set whether or not this is request is from the Xtensa top XFER control block.
   *
   * @param     xfer_en         True indicates this request is a local memory request
   *                            originating from the Xtensa top XFER control block and
   *                            therefore must not be RSP_NACC'd.
   *
   * @Note This interface is only applicable to TX Xtensa cores which have the
   *       bootloader interface and at least one local memory with a busy signal.
   */
  void set_xfer_en(bool xfer_en) { m_xfer_en = xfer_en; }


  /**
   * Get whether or not this is request is from the Xtensa top XFER control block.
   *
   * @Note This interface is only applicable to TX Xtensa cores which have the
   *       bootloader interface and at least one local memory with a busy signal.
   *
   * @see set_xfer_en
   */
  bool get_xfer_en() const { return m_xfer_en; }


  /**
   * Set the cache coherence information of this request.
   *
   * @param     coherence       The coherence information of this request.
   *                            For READ and BLOCK_READ, this may be any value defined
   *                            by coherence_t.
   *                            For SNOOP, this may be SHARED or EXCLUSIVE.
   *                            For WRITE and BLOCK_WRITE, this is always INVALID.
   */
  void set_coherence(coherence_t coherence) { m_coherence = coherence; }


  /**
   * Get the cache coherence information of this request.
   *
   * @see set_coherence
   */
  coherence_t get_coherence() const { return m_coherence; }


  /**
   * Set the virtual address for coherent/snoop requests.
   *
   * @param     snoop_virtual_address   The virtual address for use by the snoop
   *                                    controller.
   */
  void set_snoop_virtual_address(xtsc_address snoop_virtual_address) { m_snoop_virtual_address = snoop_virtual_address; }


  /**
   * Get the virtual address associated with this request (for coherent/snoop requests).
   *
   * @see set_snoop_virtual_address
   */
  xtsc_address get_snoop_virtual_address() const { return m_snoop_virtual_address; }


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
   * Set the request's transfer buffer (data payload).
   *
   * This method should only be used for RCW, WRITE, BLOCK_WRITE, and BURST_WRITE
   * request transaction types.
   *
   * For RCW transactions, the first RCW request transfer contains that data that the
   * target should use to compare with the current memory contents and the second RCW
   * request transfer contains the data that should replace the current contents if the
   * first transfers data matched the current contents.
   *
   * Data is arranged in the buffer as follows:
   * Let address8 be the address set by set_byte_address() or the 
   * xtsc_request constructor or initialize() method.
   * Let size8 be the transfer size set by set_byte_size() or the
   * xtsc_request constructor. 
   *  \verbatim
     The byte corresponding to address8+0       is in buffer[0].
     The byte corresponding to address8+1       is in buffer[1].
        . . .
     The byte corresponding to address8+size8-1 is in buffer[size8-1].
      \endverbatim
   * This format applies regardless of host and target endianess.
   * @Note The above mapping applies regardless of byte enables; however, byte enables
   *       may dictate that certain bytes in the buffer are meaningless and not to be
   *       used.  See set_byte_enables().
   */
  void set_buffer(u32 size8, const u8 *buffer);


  /**
   * Get a pointer to the request's transfer data suitable only for reading 
   * the data.
   *
   * @see set_buffer()
   */
  const u8 *get_buffer() const { return m_buffer; }


  /**
   * Get a pointer to the request's transfer data suitable either for reading 
   * or writing the data.
   *
   * The buffer size is 64 bytes to accommodate the widest possible Xtensa 
   * memory interface; however, you should only use the first N bytes where
   * N is the size of the actual memory interface in use.  
   *
   * @Warning  Writing past the 64th byte results in undefined bad behavior.
   *
   * @see set_buffer()
   */
  u8 *get_buffer() { return m_buffer; }


  /**
   * Get this request's tag.  This is an artificial number (not in hardware) 
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
   * This method dumps this request's info to the specified ostream object, optionally 
   * including data (if applicable).
   *
   * The format of the output is:
   * \verbatim
       tag=<Tag> pc=<PC> <Type>* [<Address>/<Num>/<ByteEnables>/<Coherence>/<ID>/<Attr>]<F> <Data> <X>

       Where:
        <Tag>           is m_tag in decimal.
        <PC>            is m_pc in hexadecimal.
        <Type>          is READ|BLOCK_READ|BURST_READ|RCW|WRITE|BLOCK_WRITE|BURST_WRITE|
                        SNOOP.
        *               indicates m_last_transfer is true.
        <Address>       is m_address8 in hexadecimal.
        <Num>           is m_size8 for READ|WRITE, or m_num_transfers for BLOCK_READ|
                        BURST_READ|SNOOP, or get_transfer_number() for RCW|BLOCK_WRITE|
                        BURST_WRITE.
        <ByteEnables>   is m_byte_enables in hexadecimal.
        <Coherence>     is m_coherence in decimal.
        <ID>            is m_id in decimal.
        <Attr>          is m_pif_attribute in hex.  <Attr> is not present if 
                        m_pif_attribute has not been set (i.e is 0xFFFFFFFF).
        <F>             is the letter "F" if m_instruction_fetch is true.  Otherwise,
                        it is null (not present).
        <Data>          is the equal sign followed by the contents of m_buffer in
                        hexadecimal (without leading '0x').  This field is only present
                        if dump_data is true AND m_type is RCW|WRITE|BLOCK_WRITE|
                        BURST_WRITE.
        <X>             is the letter "X" if m_xfer_en is true.  Otherwise, it is null
                        (not present).
     \endverbatim
   *
   * @param     os              The ostream object to which the info should be dumped.
   *
   * @param     dump_data       If true, and the request type is RCW|WRITE|BLOCK_WRITE|
   *                            BURST_WRITE, then the request's buffer contents are
   *                            dumped.  Otherwise, the buffer is not dumped.
   *
   */
  void dump(std::ostream& os = std::cout, bool dump_data = true) const;


  /**
   * Helper class to make it easy to dump xtsc_request to an ostream with or without
   * data values.
   *
   * @see show_data
   */
  class stream_dumper {
  public:
    void dump(std::ostream & os) const {
      if (m_p_request) m_p_request->dump(os, m_show_data);
    }
    static const xtsc_request  *m_p_request;
    static bool                 m_show_data;
  };

  /**
   * This method makes it easy to dump an xtsc_request to an ostream while
   * programatically determining whether or not to include the payload data (if any).
   *
   * Usage:
   *  \verbatim
             cout << request                  << endl;   // Show data
             cout << request.show_data(true)  << endl;   // Show data
             cout << request.show_data(false) << endl;   // Don't show data
      \endverbatim
   */
  const stream_dumper& show_data(bool show) const {
    stream_dumper::m_p_request = this;
    stream_dumper::m_show_data = show;
    return m_stream_dumper;
  }


private:

  // Initialize to all 0's
  void zeroize();

  xtsc_address          m_address8;                     ///< Byte address 
  u8                    m_buffer[xtsc_max_bus_width8];  ///< Data for RCW, WRITE, and BLOCK_WRITE
  u32                   m_size8;                        ///< Byte size of each transfer
  u32                   m_pif_attribute;                ///< PIF request attributes (12 bits).  0xFFFFFFFF => not set.
  u32                   m_route_id;                     ///< Route ID for arbiters
  type_t                m_type;                         ///< Request type (READ, BLOCK_READ, etc)
  u32                   m_num_transfers;                ///< Number of transfers
  xtsc_byte_enables     m_byte_enables;                 ///< Byte enables
  u8                    m_id;                           ///< Transaction ID
  u8                    m_priority;                     ///< Transaction priority
  bool                  m_last_transfer;                ///< True if last transfer of request
  bool                  m_instruction_fetch;            ///< True if request is for an instruction fetch, otherwise false
  bool                  m_xfer_en;                      ///< True if request is from Xtensa top XFER control block
  coherence_t           m_coherence;                    ///< Cache Coherence information
  xtsc_address          m_snoop_virtual_address;        ///< Virtual address for snoop controller
  xtsc_address          m_pc;                           ///< Program counter associated with request (artificial)
  xtsc_address          m_hw_address8;                  ///< Address that would appear in hardware.  BURST_WRITE only.
  u32                   m_transfer_num;                 ///< Number of this transfer.  BURST_WRITE only.  (artificial)

  u64                   m_tag;                          ///< Unique tag per request-response set (artificial)
  static stream_dumper  m_stream_dumper;                ///< To assist with printing (dumping)

};



/**
 * Dump an xtsc_request object.
 *
 * This operator dumps an xtsc_request object using the
 * xtsc_request::dump() method.
 *
 */
XTSC_API std::ostream& operator<<(std::ostream& os, const xtsc_request& request);


/**
 * Dump an xtsc_request object.
 *
 * This operator dumps an xtsc_request object using the
 * xtsc_request::stream_dumper::dump() method.
 *
 */
XTSC_API std::ostream& operator<<(std::ostream& os, const xtsc_request::stream_dumper& dumper);






} // namespace xtsc



#endif  // _xtsc_request_h_

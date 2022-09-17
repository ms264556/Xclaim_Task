#ifndef _XTSC_QUEUE_POP_IF_H_
#define _XTSC_QUEUE_POP_IF_H_

// Copyright (c) 2005-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */


#include <xtsc/xtsc_types.h>

namespace xtsc {



/**
 * This interface is for connecting between a consumer and a queue.
 *
 * This interface along with the xtsc_queue_push_if is used for one way
 * communication between a producer and a consumer through a queue.
 *
 * A producer has an sc_port<xtsc_queue_push_if>, a consumer has an 
 * sc_port<xtsc_queue_pop_if>, and a queue typically has an 
 * sc_export<xtsc_queue_push_if> and an sc_export<xtsc_queue_pop_if>.
 *
 * Often the producer and the consumer are instances of xtsc_core.
 *
 * @Note The methods of xtsc_queue_pop_if are all non-blocking in the OSCI 
 *       TLM sense.  That is, they must NEVER call wait() either directly or 
 *       indirectly.  The "nb_" method prefix stands for Non-Blocking.
 *
 * @see xtsc_queue_push_if
 * @see xtsc_create_queue_ticket
 * @see xtsc_component::xtsc_queue
 * @see xtsc_core
 */
class XTSC_API xtsc_queue_pop_if : virtual public sc_core::sc_interface {
public:


  /**
   * This method is used to determine if the queue has at least one element
   * in it.
   *
   * @return true if the queue has at least one element in it, returns false if
   *          the queue is empty.
   *
   * This non-blocking method must never call the SystemC wait() method (either
   * directly or indirectly).
   *
   */
  virtual bool nb_can_pop() = 0;


  /**
   * This method is used remove an element from the queue.  
   *
   * @param     element         The sc_unsigned object in which to return
   *                            the element popped from the queue.
   *
   * @param     ticket          An optional reference to a u64 object in which to return
   *                            a ticket number associated with this queue element.  If
   *                            a component which implements this interface choses to
   *                            support the ticket concept, it should create and return
   *                            a unique ticket number for each element pushed into the
   *                            component (see xtsc_create_queue_ticket()).  The unique
   *                            ticket number should be maintained with the element and
   *                            be returned here when the element is popped from the
   *                            component.  It is recommended that a component that
   *                            implements both the xtsc_queue_push_if and
   *                            xtsc_queue_pop_if interfaces support this ticket
   *                            concept.  For some devices (e.g. ones that implement
   *                            just one of the two interfaces), the ticket concept may
   *                            not be meaningful and they should just return a constant
   *                            ticket value (e.g. 0).  Typically, queue clients (such
   *                            as xtsc_core) use this ticket for non-hardware purposes
   *                            such as logging, profiling, and debugging.  Clients are
   *                            free to ignore the ticket.
   *
   * @return false if no element can be removed because the queue is empty,
   *          otherwise returns true.
   *
   * This non-blocking method must never call the SystemC wait() method (either
   * directly or indirectly).
   *
   */
  virtual bool nb_pop(sc_dt::sc_unsigned& element, u64& ticket = pop_ticket) = 0;


  /**
   * Get the element width in bits that the queue implementation will pop.
   *
   * This method allows the queue consumer to confirm that the implementation
   * will pop elements of the expected bit width.
   *
   * This non-blocking method must never call the SystemC wait() method (either
   * directly or indirectly).
   *
   */
  virtual u32 nb_get_bit_width() = 0;


  /**
   * Return the no-longer-empty event.
   *
   * Clients can call this method to get a reference to an event that will be notified
   * whenever the queue transitions from empty to not empty.
   *
   * @Note
   * Sub-classes must override this method and return their no-longer-empty event.
   */
  virtual const sc_core::sc_event& default_event() const { return sc_interface::default_event(); }


protected:
  static u64 pop_ticket;
};


} // namespace xtsc

#endif  // _XTSC_QUEUE_POP_IF_H_

#ifndef _F2Q_ADAPTER_H_
#define _F2Q_ADAPTER_H_

// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

/**
 * @file 
 */



#include <xtsc/xtsc.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_queue_pop_if.h>



/**
 * Constructor parameters for a f2q_adapter object.
 *
 *  \verbatim
   Name                 Type    Description
   ------------------   ----    --------------------------------------------------------

   "p_fifo"             void*   An optional pointer to a sc_fifo object. If NULL, a
                                sc_fifo will be created using the following parameters.
                
   "bit_width"          u32     Width of each sc_fifo element in bits.

   "depth"              u32     Number of elements the sc_fifo can hold.  Ignored if
                                p_fifo is not NULL.
  
    \endverbatim
 *
 * @see xtsc_parms
 */
class f2q_adapter_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for a f2q_adapter_parms object. 
   *
   * @param     p_fifo          An optional pointer to a sc_fifo object. If NULL, a
   *                            sc_fifo will be created using the following parameters.
   * 
   * @param     bit_width       Width of each sc_fifo element in bits.
   *
   * @param     depth           Number of elements the sc_fifo can hold.  Ignored if 
   *                            p_fifo is not NULL.
   *                    
   */
  f2q_adapter_parms(sc_core::sc_fifo<sc_dt::sc_unsigned> *p_fifo = NULL,
                    xtsc::u32                             bit_width = 32,
                    xtsc::u32                             depth  = 16)
  {
    add("p_fifo",               (void*) p_fifo);
    add("bit_width",            bit_width);
    add("depth",                depth);
  }

  /// Return what kind of xtsc_parms this is (our C++ type)
  const char *kind() const { return "f2q_adapter_parms"; }
};




/**
 * This module provides an adapter from a sc_fifo to the xtsc_queue_pop_if interface.
 * The sc_fifo can already exist and be passed in to this module via the
 * f2q_adapter_parms object, or, alternatively, this module will create the sc_fifo
 * and make it available via the get_fifo() method.
 *
 * One possible usage for this module is to allow a TIE input queue of xtsc::xtsc_core
 * to read from (pop) a sc_fifo.
 */
class f2q_adapter : public sc_core::sc_module {
public:

  sc_core::sc_export<xtsc::xtsc_queue_pop_if>  m_consumer;      ///<  Consumer binds to this


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "f2q_adapter"; }


  /**
   * Constructor for a f2q_adapter.
   *
   * @param     module_name     Name of the f2q_adapter sc_module.
   *
   * @param     parms           The remaining parameters for construction.
   *
   * @see f2q_adapter_parms
   */
  f2q_adapter(sc_core::sc_module_name module_name, const f2q_adapter_parms &parms);

  /// Destructor
  ~f2q_adapter();


  /// Get a reference to the sc_fifo that this adapter is using
  sc_core::sc_fifo<sc_dt::sc_unsigned>& get_fifo() { return m_fifo; }

protected:


  /**
   * Static method to either get the sc_fifo passed in via the f2q_adapter_parms
   * object or to create one.  If "p_fifo" in parms is not NULL then the sc_fifo that it
   * points to will be returned.  Otherwise, a sc_fifo will be created and returned.
   * The our_fifo flag will be set to true if this method created the sc_fifo.  
   * Otherwise, our_fifo will be set to false.
   *
   * @param     parms           The construction parameters.
   *
   * @param     our_fifo        Set to false if "p_fifo" in parms is not NULL.
   *                            Otherwise, set to true.
   */
  static sc_core::sc_fifo<sc_dt::sc_unsigned>& get_or_create_fifo(const f2q_adapter_parms &parms, bool& our_fifo);


  /// Implementation of xtsc_queue_pop_if.
  class xtsc_queue_pop_if_impl : public xtsc::xtsc_queue_pop_if, public sc_core::sc_object  {
  public:

    /// Constructor
    xtsc_queue_pop_if_impl(const char *object_name, f2q_adapter& adapter) :
      sc_object         (object_name),
      m_adapter         (adapter),
      m_p_port          (0)
    {}

    /// @see xtsc::xtsc_queue_pop_if
    bool nb_can_pop() { return (m_adapter.m_fifo.num_free() != 0); }

    /// @see xtsc::xtsc_queue_pop_if
    bool nb_pop(sc_dt::sc_unsigned& element, xtsc::u64& ticket = pop_ticket);

    /// @see xtsc::xtsc_queue_pop_if
    xtsc::u32 nb_get_bit_width() { return m_adapter.m_width1; }

    /**
     * Get the event that will be notified when the sc_fifo transitions from full
     * to not full.
     */
    virtual const sc_core::sc_event& default_event() const { return m_adapter.m_fifo.data_written_event(); }

  private:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    f2q_adapter&                m_adapter;              ///< Our f2q_adapter object
    sc_core::sc_port_base      *m_p_port;               ///< Port that is bound to us
  };




  xtsc_queue_pop_if_impl                m_pop_impl;     ///<  m_consumer binds to this
  sc_core::sc_fifo<sc_dt::sc_unsigned>& m_fifo;         ///<  The sc_fifo
  bool                                  m_our_fifo;     ///<  True if we created m_fifo
  xtsc::u32                             m_width1;       ///<  The bit width of elements in m_fifo
  log4xtensa::TextLogger&               m_text;         ///<  Text logger
};

#endif  // _F2Q_ADAPTER_H_

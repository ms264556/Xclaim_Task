#ifndef _XTSC_WIRE_READ_IF_H_
#define _XTSC_WIRE_READ_IF_H_

// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
 * Interface for reading (sinking) a wire.
 *
 * This TLM-level interface is for connecting between a wire sink (e.g. an import
 * wire of an xtsc_core) and a wire implementation provided by the user (e.g.
 * xtsc_component::xtsc_wire).  The wire sink has an sc_port<xtsc_wire_read_if> used
 * to connect to the wire implementation which either inherits from this interface or
 * has an sc_export<xtsc_wire_read_if>.
 *
 * @Note The methods of xtsc_wire_read_if are all non-blocking in the OSCI 
 *       TLM sense.  That is, they must NEVER call wait() either directly or 
 *       indirectly.  The "nb_" method prefix stands for Non-Blocking.
 *
 * @see xtsc_wire_read_if
 * @see xtsc_core::get_import_wire
 * @see xtsc_component::xtsc_wire
 */
class XTSC_API xtsc_wire_read_if : virtual public sc_core::sc_interface {
public:


  /**
   * This method is called by the wire sink to read the value of the wire.
   *
   * For the typical case of an xtsc_core TIE import wire, this method is called on each
   * clock cycle in which the Xtensa core reads a value from the TIE_xxx interface
   * (where xxx is the import wire name in the user TIE code).
   *
   * This non-blocking method must never call the SystemC wait() method (either directly
   * or indirectly).
   *
   */
  virtual sc_dt::sc_unsigned nb_read() = 0;



  /**
   * Get the wire width in bits.
   *
   * This method allows the wire sink to confirm that the implementation is using the
   * correct size for the wire.  
   *
   * This non-blocking method must never call the SystemC wait() method (either directly
   * or indirectly).
   *
   */
  virtual u32 nb_get_bit_width() = 0;


};





} // namespace xtsc

#endif  // _XTSC_WIRE_READ_IF_H_

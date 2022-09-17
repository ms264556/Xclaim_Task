#ifndef _XTSC_VP_H_
#define _XTSC_VP_H_

// Copyright (c) 2006-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <scml.h>
#include <xtsc/xtsc.h>
#include <xtsc/xtsc_core.h>



namespace xtsc_vp {



///  Method to initialize XTSC_VP
void xtsc_vp_initialize();



///  Method to finalize XTSC_VP
void xtsc_vp_finalize();



/**
 * Get or create the named VCD sc_trace_file.
 *
 * The first time this method is called for the named sc_trace_file, it will be created.
 * Subsequent calls with the same name will return the already created sc_trace_file.
 */
sc_core::sc_trace_file *xtsc_vp_get_trace_file(const std::string& name);



/// Allow integration layer to hook into the ISS thread
void xtsc_vp_register_iss_thread_callbacks(xtsc::xtsc_core &core, 
                                           void *arg,
                                           void (*before_step_cpu)(void*),
                                           bool (*after_step_cpu)(void*));



/// Print the debug cheat sheet but only print Xplorer info the first time
void xtsc_vp_print_debug_cheat_sheet(log4xtensa::TextLogger& logger, xtsc::u32 debugger_port, const char *target_program);




/**
 * For putting registers in a modified alphabetical order:
 *   1)  Case-insensitive
 *   2)  Numbered registers appear in numeric order.  For example
 *       registers AR10, AR11, ..., AR19 all appear after register
 *       AR9 (instead of after register AR1 and before register
 *       AR2--which would be strictly alphabetical).
 */
bool xtsc_vp_is_first_less_then_second(const std::string& first, const std::string& second);



/**
 * Return true if name consists of a non-zero number of uppercase letters
 * followed by a non-zero number of decimal digits.
 * When returning true set alpha to the number of uppercase letters and
 * set numeric to the number of decimal digits
 */
bool xtsc_vp_is_numbered_register(const std::string& name, xtsc::u32& alpha, xtsc::u32& numeric);



/// Convert std::string to a c-str array by tokenizing on commas
char **xtsc_vp_create_c_str_array(const std::string &csv);

}  // namespace xtsc_vp




#endif  // _XTSC_VP_H_

#ifndef _XTSC_SD_H_
#define _XTSC_SD_H_

// Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <string>
#include <map>
#include <xtsc/xtsc.h>
#include <maxsim.h>
#include <sc_mx_import_module.h>



#if defined(XTSC_SD_DLL)
#define XTSC_SD_EXPORT __declspec(dllexport)
#if defined(EXPORT_XTSC_SD_INTERFACE)
#define XTSC_SD_API XTSC_SD_EXPORT
#else
#define XTSC_SD_API __declspec(dllimport)
#endif
#else
#define XTSC_SD_EXPORT 
#define XTSC_SD_API
#endif

namespace xtsc_sd {


///  Method to initialize XTSC
XTSC_SD_API void xtsc_sd_initialize();


class XTSC_SD_API helper : public sc_module {
public:
  helper(sc_module_name module_name) : sc_module(module_name) {
    m_end_of_elaboration = false;
  }
  void before_end_of_elaboration() {
  }
  void end_of_elaboration() {
    m_end_of_elaboration = true;
  }
  bool m_end_of_elaboration;
};



XTSC_SD_API char *xtsc_sd_get_grant_string(MxGrant grant);


XTSC_SD_API const char *xtsc_sd_get_status_string(MxStatus status);


XTSC_SD_API const char *xtsc_sd_get_status_string(MxStatus status);


XTSC_SD_API const char *xtsc_sd_get_ctrl_type_string(MxU32* ctrl);


XTSC_SD_API const char *xtsc_sd_get_ctrl_cycle_string(MxU32* ctrl);


XTSC_SD_API const char *xtsc_sd_get_ctrl_ack_string(MxU32* ctrl);


XTSC_SD_API const char *xtsc_sd_get_ctrl_lock_string(MxU32* ctrl);


XTSC_SD_API const char *xtsc_sd_get_ctrl_burst_string(MxU32* ctrl);


XTSC_SD_API xtsc::u32 xtsc_sd_get_ctrl_burst_factor(MxU32* ctrl);


XTSC_SD_API const char *xtsc_sd_get_ctrl_trans_string(MxU32* ctrl);


XTSC_SD_API xtsc::u32 xtsc_sd_get_ctrl_size(MxU32* ctrl);


XTSC_SD_API const char *xtsc_sd_get_ack_string(xtsc::u32 ack_type);


/**
 * Helper method to convert the m_value array to a string.
 *
 * @param size8       Number of bytes of value array to convert.
 *
 * @param value       The value array.
 *
 */
XTSC_SD_API std::string xtsc_sd_convert_value_to_string(xtsc::u32 size8, MxU32 *value);


/**
 * Get or create the named VCD sc_trace_file.
 *
 * The first time this method is called for the named sc_trace_file, it will be created.
 * Subsequent calls with the same name will return the already created sc_trace_file.
 */
XTSC_SD_API sc_core::sc_trace_file *xtsc_sd_get_trace_file(const std::string& name);


}  // namespace xtsc_sd

#endif // _XTSC_SD_H_

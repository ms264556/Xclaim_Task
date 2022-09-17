// Copyright (c) 2006-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.


#include <xtsc/xtsc.h>
#include <xtsc_vp/xtsc_vp.h>
#include <map>

// License check mechanism changed with F-2011.06-Beta and later
#if !defined(SYNOPSYS_LICENSING_VERSION_2)
// CoWare licensing
#include "CwrIpClm.h"
#else
// Synopsys licensing.  Ashwani@Synopsys email: 16 May 2011
#include "ip_module_init/ip_module_init.h"
#endif


using namespace std;
using namespace xtsc;
using namespace xtsc_vp;


static map<string, sc_trace_file*> trace_file_map;


#define SCML_PROPERTY_REGISTRY_getBln(Parm) \
       scml_property_registry::inst().getBoolProperty(scml_property_registry::GLOBAL, "/HIERARCHY", "/xtsc_vp/" #Parm)

#define SCML_PROPERTY_REGISTRY_getInt(Parm) \
  (u32) scml_property_registry::inst().getIntProperty(scml_property_registry::GLOBAL, "/HIERARCHY", "/xtsc_vp/" #Parm)

#define SCML_PROPERTY_REGISTRY_getStr(Parm) \
     scml_property_registry::inst().getStringProperty(scml_property_registry::GLOBAL, "/HIERARCHY", "/xtsc_vp/" #Parm)



void xtsc_vp::xtsc_vp_initialize() {
  xtsc_initialize_parms init_parms;
  static bool first_time = true;

  // Only extract parameters the first time (system_clock_factor cannot be changed after this)
  if (first_time) {
    first_time = false;

    init_parms.set("breakpoint_csv_file",                 SCML_PROPERTY_REGISTRY_getStr(breakpoint_csv_file)              .c_str());
    init_parms.set("call_sc_stop_on_finalize",            SCML_PROPERTY_REGISTRY_getBln(call_sc_stop_on_finalize));
    init_parms.set("constructor_log_level",               SCML_PROPERTY_REGISTRY_getStr(constructor_log_level)            .c_str());
    init_parms.set("coordinated_debugging",               SCML_PROPERTY_REGISTRY_getBln(coordinated_debugging));
    init_parms.set("hex_dump_left_to_right",              SCML_PROPERTY_REGISTRY_getBln(hex_dump_left_to_right));
    init_parms.set("posedge_offset_factor",               SCML_PROPERTY_REGISTRY_getInt(posedge_offset_factor));
    init_parms.set("simcall_csv_file",                    SCML_PROPERTY_REGISTRY_getStr(simcall_csv_file)                 .c_str());
    init_parms.set("stop_after_all_cores_exit",           SCML_PROPERTY_REGISTRY_getBln(stop_after_all_cores_exit));
    init_parms.set("system_clock_factor",                 SCML_PROPERTY_REGISTRY_getInt(system_clock_factor));
    init_parms.set("target_memory_limit",                 SCML_PROPERTY_REGISTRY_getInt(target_memory_limit));
    init_parms.set("text_logging_config_file",            SCML_PROPERTY_REGISTRY_getStr(text_logging_config_file)         .c_str());
    init_parms.set("text_logging_delta_cycle_digits",     SCML_PROPERTY_REGISTRY_getInt(text_logging_delta_cycle_digits));
    init_parms.set("text_logging_disable",                SCML_PROPERTY_REGISTRY_getBln(text_logging_disable));
    init_parms.set("text_logging_time_precision",         SCML_PROPERTY_REGISTRY_getInt(text_logging_time_precision));
    init_parms.set("text_logging_time_width",             SCML_PROPERTY_REGISTRY_getInt(text_logging_time_width));
    init_parms.set("turbo",                               SCML_PROPERTY_REGISTRY_getBln(turbo));
    init_parms.set("turbo_max_relaxed_cycles",            SCML_PROPERTY_REGISTRY_getInt(turbo_max_relaxed_cycles));
    init_parms.set("turbo_min_sync",                      SCML_PROPERTY_REGISTRY_getBln(turbo_min_sync));
    init_parms.set("xtsc_finalize_unwind",                SCML_PROPERTY_REGISTRY_getBln(xtsc_finalize_unwind));

    vector<u32> clock_phase_delta_factors;
    xtsc_strtou32vector(SCML_PROPERTY_REGISTRY_getStr(clock_phase_delta_factors), clock_phase_delta_factors);
    init_parms.set("clock_phase_delta_factors",           clock_phase_delta_factors);

    vector<u32> simcall_csv_format;
    xtsc_strtou32vector(SCML_PROPERTY_REGISTRY_getStr(simcall_csv_format), simcall_csv_format);
    init_parms.set("simcall_csv_format",                  simcall_csv_format);

    init_parms.extract_parms(sc_argc(), sc_argv(), "xtsc");

    if (init_parms.get_u32("posedge_offset_factor") == 0xFFFFFFFF) {
      clock_phase_delta_factors = init_parms.get_u32_vector("clock_phase_delta_factors");
      if (clock_phase_delta_factors.size() >= 2) {
        u32 system_clock_factor   = init_parms.get_u32("system_clock_factor");
        u32 phase_A_delta         = clock_phase_delta_factors[0];
        u32 phase_B_delta         = clock_phase_delta_factors[1];
        u32 posedge_offset_factor = system_clock_factor - phase_A_delta - phase_B_delta;
        init_parms.set("posedge_offset_factor", posedge_offset_factor);
      }
    }
  }

  // Call this every time so the that xtsc_finalize_unwind can be handled properly
  xtsc_initialize(init_parms);
}



void xtsc_vp::xtsc_vp_finalize() {
  xtsc_finalize();
}



sc_trace_file *xtsc_vp::xtsc_vp_get_trace_file(const string& name) {
  map<string, sc_trace_file*>::const_iterator i = trace_file_map.find(name);
  if (i != trace_file_map.end()) {
    return i->second;
  }
  sc_trace_file * p_trace_file = sc_create_vcd_trace_file(name.c_str());
  if (!p_trace_file) {
    ostringstream oss;
    oss << "xtsc_vp_get_trace_file: creation of sc_trace_file \"" << name << "\" failed";
    throw xtsc_exception(oss.str());
  }
  trace_file_map.insert(make_pair(name, p_trace_file));
  return p_trace_file;
}




bool xtsc_vp::xtsc_vp_is_first_less_then_second(const string& first, const string& second) {
  string F = first;
  transform(F.begin(), F.end(), F.begin(), ::toupper);
  string S = second;
  transform(S.begin(), S.end(), S.begin(), ::toupper);
  u32 a1 = 0;
  u32 a2 = 0;
  u32 n1 = 0;
  u32 n2 = 0;
  if ((F.length() == S.length()) ||
      !xtsc_vp_is_numbered_register(F, a1, n1) ||
      !xtsc_vp_is_numbered_register(S, a2, n2) ||
      (F.substr(0,a1) != S.substr(0,a2)) ||
      (n1 > 8) ||
      (n2 > 8))
  {
    return (F < S);
  }
  u32 num1 = xtsc_strtou32(F.substr(a1));
  u32 num2 = xtsc_strtou32(S.substr(a2));
  return (num1 < num2);
}



bool xtsc_vp::xtsc_vp_is_numbered_register(const string& name, u32& alpha, u32& numeric) {
  string::size_type a = name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  if ((a == 0) || (a == string::npos)) return false;
  string::size_type n = name.substr(a).find_first_not_of("0123456789");
  if (n != string::npos) return false;
  alpha = (u32) a;
  numeric = name.length() - alpha;
  return true;
}



char **xtsc_vp::xtsc_vp_create_c_str_array(const string &csv) {
  char **args = NULL;
  if (csv != "") {
    char comma = ',';
    u32 num_commas = count(csv.begin(), csv.end(), comma);
    args = new char*[num_commas+2];
    args[num_commas+1] = NULL;
    int argi = 0;
    string::const_iterator beg = csv.begin();
    string::const_iterator i = csv.begin();
    for (; i != csv.end(); ++i) {
      if (*i == comma) {
        string value(beg, i);
        args[argi++] = xtsc_copy_c_str(value.c_str());
        beg = i + 1;
      }
    }
    string value(beg, i);
    args[argi++] = xtsc_copy_c_str(value.c_str());
  }
  return args;
}






namespace xtsc {
  // If we decide to expose this to XTSC users at large then put a bridge method in xtsc_core
  extern void xtsc_register_iss_thread_callbacks(xtsc_core &core,
                                                 void *arg,
                                                 void (*before_step_cpu)(void*),
                                                 bool (*after_step_cpu)(void*)
                                                 );

  extern void xtsc_core_vp_setup(xtsc_core& core);

}



void xtsc_vp::xtsc_vp_register_iss_thread_callbacks(xtsc_core &core,
                                                    void *arg,
                                                    void (*before_step_cpu)(void*),
                                                    bool (*after_step_cpu)(void*)
                                                   )
{

  if (xtsc_get_xtsc_initialize_parms().get_bool("coordinated_debugging")) {
    xtsc::xtsc_register_iss_thread_callbacks(core, arg, before_step_cpu, after_step_cpu);

    // We do this here and this way (instead of using the CWR_SYSTEMC macro in xtsc_core) so
    // that we don't change things for EXTSC
    xtsc::xtsc_core_vp_setup(core);
  }

// License check mechanism changed with F-2011.06-Beta and later
#if !defined(SYNOPSYS_LICENSING_VERSION_2)
// CoWare licensing E2010.09 and earlier
  XTSC_INFO(core.get_text_logger(), "Doing CoWare license checkout");
  if (core.get_parms().get_bool("IsPreconfiguredCore")) {
    cwr_2005_2_clm_register_core("diamond", "ca", "2005.2", 1);
  }
  else {
    cwr_2005_2_clm_register_core("xtensa",  "ca", "2005.2", 0);
  }
#else
// Synopsys licensing.  Ashwani@Synopsys email: 16 May 2011
  string psp  (core.get_parms().get_bool("IsPreconfiguredCore") ? "TENSILICA_DIAMOND_PSP" : "TENSILICA_XTENSA_PSP");
  string turbo(core.get_parms().get_bool("SimTurbo")            ? "turbo"                 : "cycle-accurate");
  XTSC_INFO(core.get_text_logger(), "Doing Synopsys license checkout for " << turbo << " " << psp);
  IpModuleInit *p_synopsys_license_mechanism = new IpModuleInit();
  p_synopsys_license_mechanism->init(&core, psp, turbo);
#endif

}



void xtsc_vp::xtsc_vp_print_debug_cheat_sheet(log4xtensa::TextLogger& logger, u32 debugger_port, const char *target_program) {
  cout << endl;
  static bool first_time = true;
  if (first_time) {
  first_time = false;
  cout << "To debug from Xtensa Xplorer the first time:" << endl;
  cout << "  - Launch Xplorer (if it is not already running)" << endl;
  cout << "  - Following the drop-down menu sequence:" << endl;
  cout << "      Run>Open Debug Dialog" << endl;
  cout << "  - In the left pane of the Debug dialog box right-click on \"MP Launch\" and select \"New\"" << endl;
  cout << "  - In the right pane of the Debug dialog box:" << endl;
  cout << "    - Give your launch configuration a name (e.g xt_siminfo).  See *Note." << endl;
  cout << "    - Ensure the \"Simulator Selection\" tab is selected." << endl;
  cout << "    - Under \"Select MP Simulator Launch Type\":" << endl;
  cout << "      - For \"Type\" select \"Attach to Simulator\"" << endl;
  cout << "    - Under \"MP Launch Options\":" << endl;
  cout << "      - In the \"Simulator\" text box, enter \"${xt_siminfo}\"" << endl;
  cout << "    - Under \"Debugger Attach Options\", select the desired option(s)." << endl;
  cout << "    - Click on \"Apply\"" << endl;
  cout << "  - Click on \"Debug\"" << endl;
  cout << "  *Note: On subsequent runs, you can reuse the above launch using the name you gave it." << endl;
  cout << "         Hint: The F11 shortcut key to repeat the previous launch can come in handy here." << endl;
  cout << endl;
  cout << "To debug using xt-gdb:" << endl;
  cout << endl;
  }
  cout << "  xt-gdb " << (target_program ? target_program : "") << endl;
  cout << "  (xt-gdb) target remote localhost:" << debugger_port << endl;
  cout << endl;
}





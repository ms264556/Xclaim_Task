// Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include <ctime>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_memory.h>
#include <xtsc/xtsc_memory_trace.h>


using namespace std;
using namespace sc_core;
using namespace log4xtensa;
using namespace xtsc;
using namespace xtsc_component;


// Create a logger for sc_main
static TextLogger& logger = TextLogger::getInstance("sc_main");



int sc_main(int argc, char *argv[]) {

  try {

    // Initialize
    xtsc_initialize_parms init_parms("../TextLogger.txt");
    init_parms.extract_parms(argc, argv, "xtsc");
    xtsc_initialize(init_parms);

    // Configuration parameters for xtsc_core
    xtsc_core_parms core_parms(CONFIG_NAME, XTENSA_REGISTRY, "");
    core_parms.set("SimTargetOutput", "core0_output.log");
    core_parms.extract_parms(argc, argv, "core0");

    // Construct the core
    xtsc_core core0("core0", core_parms);

    // Create VCD file "waveforms.vcd"
    sc_trace_file *waveforms = sc_create_vcd_trace_file("waveforms");
    static sc_clock *p_clk = new sc_clock("clk", 1000*sc_get_time_resolution(), 0.5, 0*sc_get_time_resolution(), true);
    sc_trace(waveforms, *p_clk, p_clk->name());

    // Iterate all possible memory ports 
    xtsc_core::memory_port p;
    xtsc_memory_trace  *p_trace = NULL;
    xtsc_memory        *p_mem   = NULL;
    for (p=xtsc_core::MEM_FIRST; p<=xtsc_core::MEM_LAST; ++p) {
      // Get the memory port name
      const char *port_name = xtsc_core::get_memory_port_name(p);
      // Does this core have such a memory port?
      if (core0.has_memory_port(port_name)) {
        // Skip the 2nd LD/ST unit for this type of memory because
        // it would have been taken care of in the previous pass.
        if (!xtsc_core::is_ls_dual_port(p, 1)) {

          // Create an xtsc_memory_trace name
          string memory_trace_name = string("trace_") + string(xtsc_core::get_memory_port_name(p, true));
          // Create xtsc_memory_trace parameters corresponding to this memory port
          xtsc_memory_trace_parms memory_trace_parms(core0, port_name, waveforms);
          memory_trace_parms.extract_parms(argc, argv, memory_trace_name);
          // Construct the xtsc_memory_trace
          p_trace = new xtsc_memory_trace(memory_trace_name.c_str(), memory_trace_parms);

          // Create a memory name
          string mem_name = string("mem_") + string(xtsc_core::get_memory_port_name(p, true));
          // Create memory parameters corresponding to this memory port
          xtsc_memory_parms memory_parms(core0, port_name);
          memory_parms.extract_parms(argc, argv, mem_name);
          // Construct the memory
          p_mem = new xtsc_memory(mem_name.c_str(), memory_parms);

          // Connect the core to the xtsc_memory_trace
          p_trace->connect(core0, port_name);
          // Connect the xtsc_memory_trace to the xtsc_memory
          p_mem->connect(*p_trace);

        }
      }
    }

    // Load program
    core0.load_program("target/memory_test.out");

    // Set-up debugging according to command line arguments, if any
    xtsc_core::setup_multicore_debug(argc, argv);

    // Start and run the simulation
    sc_start();

    // Clean-up
    xtsc_finalize();

  }
  catch (const exception& error) {
    ostringstream oss;
    oss << "std::exception caught: " << endl;
    oss << error.what() << endl;
    xtsc_log_multiline(logger, FATAL_LOG_LEVEL, oss.str(), 2);
    cerr << oss.str();
  }

  return 0;
}


// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include <ostream>
#include <string>
#include <xtsc/xtsc_core.h>
#include <xtsc/xtsc_memory.h>
#include <xtsc/xtsc_lookup_pin.h>

using namespace std;
using namespace sc_core;
using namespace log4xtensa;
using namespace xtsc;
using namespace xtsc_component;

static TextLogger& logger = TextLogger::getInstance("sc_main");


int sc_main(int argc, char *argv[]) {

  try {

    // Initialize
    xtsc_initialize_parms init_parms("../TextLogger.txt");
    init_parms.extract_parms(argc, argv, "xtsc");
    xtsc_initialize(init_parms);

    // Configuration parameters for xtsc_core
    const char *pli[] = { "lut", NULL };
    xtsc_core_parms core_parms(CONFIG_NAME, XTENSA_REGISTRY, TDK_DIR);
    core_parms.set("SimTargetOutput", "core0_output.log");
    core_parms.set("SimPinLevelInterfaces", pli);
    core_parms.extract_parms(argc, argv, "core0");

    // Construct the core
    xtsc_core core0("core0", core_parms);

    // Configure, construct, and connect pif memory for core0
    xtsc_memory_parms memory_parms(core0, "pif");
    memory_parms.extract_parms(argc, argv, "core0_pif");
    xtsc_memory core0_pif("core0_pif", memory_parms);
    core0_pif.connect(core0, "pif");

    // Create a file for signal-level tracing
    sc_trace_file *p_tf = sc_create_vcd_trace_file("waveforms");

    // Create a clock just to enhance the tracing
    sc_clock clk("clk", xtsc_get_system_clock_period());
    sc_trace(p_tf, clk, "clk");

    // Instantiate the lookup table
    u32 address_width   = core0.get_pin_bit_width("TIE_lut_Out");
    u32 data_width      = core0.get_pin_bit_width("TIE_lut_In");
    xtsc_lookup_pin_parms tbl_parms(address_width, data_width, true, "lut.rom", "0xDEADBEEF", p_tf);
    tbl_parms.extract_parms(argc, argv, "tbl");
    xtsc_lookup_pin tbl("tbl", tbl_parms);

    // Signals for connecting the lookup
    xtsc_signal_sc_bv_base TIE_lut_Out    ("TIE_lut_Out",     address_width);
    xtsc_signal_sc_bv_base TIE_lut_Out_Req("TIE_lut_Out_Req", 1);
    xtsc_signal_sc_bv_base TIE_lut_In     ("TIE_lut_In",      data_width);
    xtsc_signal_sc_bv_base TIE_lut_Rdy    ("TIE_lut_Rdy",     1);

    // Connect the core to the lookup signals
    core0.get_output_pin("TIE_lut_Out"    )(TIE_lut_Out);
    core0.get_output_pin("TIE_lut_Out_Req")(TIE_lut_Out_Req);
    core0.get_input_pin ("TIE_lut_In"     )(TIE_lut_In );
    core0.get_input_pin ("TIE_lut_Rdy"    )(TIE_lut_Rdy);

    // Connect the xtsc_lookup_pin to the lookup signals
    tbl.m_address(TIE_lut_Out);
    tbl.m_req    (TIE_lut_Out_Req);
    tbl.m_data   (TIE_lut_In );
    tbl.m_ready  (TIE_lut_Rdy);

    // Load program
    core0.load_program("target/lookup_test.out");

    // Set-up debugging according to command line arguments, if any
    core0.setup_debug(argc, argv);

    XTSC_INFO(logger, "Starting SystemC simulation.");

    // Start and run the simulation
    sc_start();

    XTSC_INFO(logger, "SystemC simulation ended.");

    // Close the trace file
    sc_close_vcd_trace_file(p_tf);

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


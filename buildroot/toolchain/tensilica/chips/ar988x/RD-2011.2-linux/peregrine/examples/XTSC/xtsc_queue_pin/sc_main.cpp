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
#include <xtsc/xtsc_queue_pin.h>

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

    // Can do this to speed up simulation (or pass -xtsc.text_logging_disable=true on the command line)
    //xtsc_enable_text_logging(false);

    // Configuration parameters for first core
    const char *core0_pli[] = { "OUTQ1", NULL };
    xtsc_core_parms core0_parms(CONFIG_NAME, XTENSA_REGISTRY, TDK_DIR);
    core0_parms.set("SimTargetOutput", "core0_output.log");
    core0_parms.set("SimPinLevelInterfaces", core0_pli);
    core0_parms.extract_parms(argc, argv, "core0");

    // Construct core0
    xtsc_core core0("core0", core0_parms);

    // Configuration parameters for second core
    const char *core1_pli[] = { "INQ1", NULL };
    xtsc_core_parms core1_parms(CONFIG_NAME, XTENSA_REGISTRY, TDK_DIR);
    core1_parms.set("SimTargetOutput", "core1_output.log");
    core1_parms.set("SimPinLevelInterfaces", core1_pli);
    core1_parms.extract_parms(argc, argv, "core1");

    // Construct core1
    xtsc_core core1("core1", core1_parms);

    // Create a file for signal-level tracing
    sc_trace_file *p_tf = sc_create_vcd_trace_file("waveforms");

    // Create a clock just to enhance the tracing
    sc_clock clk("clk", xtsc_get_system_clock_period());
    sc_trace(p_tf, clk, "clk");

    // Get queue bit width
    u32 bits = core0.get_pin_bit_width("TIE_OUTQ1");

    // Configuration parameters for a TIE queue
    xtsc_queue_pin_parms Q1_parms(bits, 2, 0, 0, false, p_tf);
    Q1_parms.extract_parms(argc, argv, "Q1");

    // Construct queue Q1
    xtsc_queue_pin Q1("Q1", Q1_parms);

    // Signals for connecting the queue
    xtsc_signal_sc_bv_base TIE_OUTQ1        ("TIE_OUTQ1",          bits);
    xtsc_signal_sc_bv_base TIE_OUTQ1_Full   ("TIE_OUTQ1_Full",     1);
    xtsc_signal_sc_bv_base TIE_OUTQ1_PushReq("TIE_OUTQ1_PushReq",  1);
    xtsc_signal_sc_bv_base TIE_INQ1         ("TIE_INQ1",           bits);
    xtsc_signal_sc_bv_base TIE_INQ1_Empty   ("TIE_INQ1_Empty",     1);
    xtsc_signal_sc_bv_base TIE_INQ1_PopReq  ("TIE_INQ1_PopReq",    1);

    // Connect queue input to signals
    Q1.m_data_in (TIE_OUTQ1);
    Q1.m_push    (TIE_OUTQ1_PushReq);
    Q1.m_full    (TIE_OUTQ1_Full);

    // Connect queue output to signals
    Q1.m_data_out(TIE_INQ1);
    Q1.m_empty   (TIE_INQ1_Empty);
    Q1.m_pop     (TIE_INQ1_PopReq);

    // Connect core0 to queue input signals
    core0.get_output_pin("TIE_OUTQ1")        (TIE_OUTQ1);
    core0.get_input_pin ("TIE_OUTQ1_Full")   (TIE_OUTQ1_Full);
    core0.get_output_pin("TIE_OUTQ1_PushReq")(TIE_OUTQ1_PushReq);

    // Connect core1 to queue output signals
    core1.get_input_pin ("TIE_INQ1")       (TIE_INQ1);
    core1.get_input_pin ("TIE_INQ1_Empty") (TIE_INQ1_Empty);
    core1.get_output_pin("TIE_INQ1_PopReq")(TIE_INQ1_PopReq);

    // Configure, construct, and connect pif memory for core0
    xtsc_memory_parms core0_pif_parms(core0, "pif");
    core0_pif_parms.extract_parms(argc, argv, "core0_pif");
    xtsc_memory core0_pif("core0_pif", core0_pif_parms);
    core0_pif.connect(core0, "pif");

    // Configure, construct, and connect pif memory for core1
    xtsc_memory_parms core1_pif_parms(core1, "pif");
    core1_pif_parms.extract_parms(argc, argv, "core1_pif");
    xtsc_memory core1_pif("core1_pif", core1_pif_parms);
    core1_pif.connect(core1, "pif");

    // Load programs
    core0.load_program("target/producer.out");
    core1.load_program("target/consumer.out");

    // Set-up debugging according to command line arguments, if any
    xtsc_core::setup_multicore_debug(argc, argv);

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


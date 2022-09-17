// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


#include "../common.inc"

-set_make_parm=testbench_name=Testbench
-set_make_parm=verilog_files=$(XTSC_SCRIPT_FILE_PATH)/Testbench.v

-no_default_mem


-set_wrapper_parm=vcd_handle=waveforms_core0
-create_wrapper=Xtensa0

-create_clock=clk0
-connect_clock_wrapper=clk0,CLK

-set_core_parm=SimTargetOutput=core0_output.log
-set_core_parm=SimTargetProgram=$(XTSC_SCRIPT_FILE_PATH)/target/hello.out
-create_core=core0

-memory_port=pif
-connect_core_memory=core0,core0_pif

-memory_port=dram0
-connect_core_memory=core0,core0_dram0

-memory_pin_port=pif
-set_pin2tlm_parm=inbound_pif=true
-create_pin2tlm=pin2tlm

-connect_wrapper_pin2tlm=pin2tlm
-connect_pin2tlm_core=pin2tlm,core0


-set_wrapper_parm=vcd_handle=waveforms_core1
-create_wrapper=Master

-set_clock_parm=vcd_handle=waveforms_core1
-create_clock=clk1

-set_master_parm=script_file=$(XTSC_SCRIPT_FILE_PATH)/../../../xtsc_pin2tlm_memory_transactor/master.vec
-create_master=master

-memory_pin_port=pif
-set_tlm2pin_parm=cosim=true
-set_tlm2pin_parm=inbound_pif=true
-set_tlm2pin_parm=write_responses=false
-create_tlm2pin=tlm2pin

-connect_master_tlm2pin=master,tlm2pin
-connect_tlm2pin_wrapper=tlm2pin


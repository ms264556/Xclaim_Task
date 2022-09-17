// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#environment
#ifeq ($(XTSC_RUN_COSIM_USE_DSO),)
#define EXTRA_PLUS_ARGS=+preloadmems
#endif

#include "../common.inc"

-set_make_parm=testbench_name=Testbench
-set_make_parm=verilog_files=$(XTSC_SCRIPT_FILE_PATH)/Testbench.v,$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_pifmem.sv,$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_iram0.sv,$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_iram1.sv,$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_dram0.sv,$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_dram1.sv

#include "../extra.wrapper.IO.inc"
-set_wrapper_parm=extra_inputs=$(EXTRA_INPUTS)
-set_wrapper_parm=extra_outputs=$(EXTRA_OUTPUTS)
-set_wrapper_parm=vcd_handle=waveforms
-set_wrapper_parm=fully_connect_cores=core0
#ifeq ($(XTSC_RUN_COSIM_USE_DSO),)
-set_wrapper_parm=readmemh_cores=core0
-set_wrapper_parm=shadow_memory=true
#endif
-create_wrapper=Xm_peregrineXtensa

-set_core_parm=SimTargetProgram=$(XTSC_SCRIPT_FILE_PATH)/target/memory_test.out
-set_core_parm=SimTargetOutput=core0_output.log
-set_core_parm=SimPinLevelInterfaces=*
-create_core=core0


#ifneq ($(XTSC_RUN_COSIM_USE_DSO),)

-memory_pin_port=pif
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_pifmem.so
-set_tlm2pin_parm=dso_cookie=Testbench.pif
-set_tlm2pin_parm=cosim=true
-connect_core_tlm2pin=core0,pif,tlm2pin_pif
-connect_tlm2pin_wrapper=tlm2pin_pif

-memory_pin_port=iram0
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_iram0.so
-set_tlm2pin_parm=dso_cookie=Testbench.iram0
-set_tlm2pin_parm=cosim=true
-connect_core_tlm2pin=core0,iram0,tlm2pin_iram0
-connect_tlm2pin_wrapper=tlm2pin_iram0

-memory_pin_port=iram1
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_iram1.so
-set_tlm2pin_parm=dso_cookie=Testbench.iram1
-set_tlm2pin_parm=cosim=true
-connect_core_tlm2pin=core0,iram1,tlm2pin_iram1
-connect_tlm2pin_wrapper=tlm2pin_iram1

-memory_pin_port=dram0
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_dram0.so
-set_tlm2pin_parm=dso_cookie=Testbench.dram0
-set_tlm2pin_parm=cosim=true
-connect_core_tlm2pin=core0,dram0,tlm2pin_dram0
-connect_tlm2pin_wrapper=tlm2pin_dram0

-memory_pin_port=dram1
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_dram1.so
-set_tlm2pin_parm=dso_cookie=Testbench.dram1
-set_tlm2pin_parm=cosim=true
-connect_core_tlm2pin=core0,dram1,tlm2pin_dram1
-connect_tlm2pin_wrapper=tlm2pin_dram1

#endif

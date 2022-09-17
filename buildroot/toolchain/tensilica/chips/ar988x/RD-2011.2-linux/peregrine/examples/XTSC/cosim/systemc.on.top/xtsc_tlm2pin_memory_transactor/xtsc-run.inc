// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include "../common.inc"

#ifeq ($(XTSC_RUN_COSIM_VENDOR),synopsys)
#define READMEMH_FILE_NAME=
#define TOP=sYsTeMcToP
#define SYNOPSYS_NAME_DOUBLING
#else
#define READMEMH_FILE_NAME=<none>
#define TOP=sc_main
#endif

#ifeq ($(XTSC_RUN_COSIM_VENDOR_VERSION),Y-2006.06-SP1-9)
#define TOP=simv
#undef SYNOPSYS_NAME_DOUBLING
#endif

#ifeq ($(XTSC_RUN_COSIM_VENDOR_VERSION),B-2008.12)
#undef SYNOPSYS_NAME_DOUBLING
#endif

#ifeq ($(XTSC_RUN_COSIM_VENDOR_VERSION),B-2008.12-3)
#undef SYNOPSYS_NAME_DOUBLING
#endif

-set_core_parm=SimTargetOutput=core0_output.log
-set_core_parm=SimTargetProgram=$(XTSC_SCRIPT_FILE_PATH)/target/memory_test.out
-create_core=core0

-memory_pin_port=pif
-set_tlm2pin_parm=cosim=true
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_pifmem.so
#ifdef SYNOPSYS_NAME_DOUBLING
-set_tlm2pin_parm=dso_cookie=$(TOP).pif.pif
#else
-set_tlm2pin_parm=dso_cookie=$(TOP).pif
#endif
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=module_name=peregrine_pifmem
-set_proxy_parm=readmemh_file_name=$(READMEMH_FILE_NAME)
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_pifmem.sv
-connect_core_tlm2pin=core0,pif,tlm2pin_pif
-connect_tlm2pin_proxy=tlm2pin_pif,pif
-connect_clock_proxy=CLK,pif

-memory_pin_port=iram0
-set_tlm2pin_parm=cosim=true
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_iram0.so
#ifdef SYNOPSYS_NAME_DOUBLING
-set_tlm2pin_parm=dso_cookie=$(TOP).iram0.iram0
#else
-set_tlm2pin_parm=dso_cookie=$(TOP).iram0
#endif
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=module_name=peregrine_iram0
-set_proxy_parm=readmemh_file_name=$(READMEMH_FILE_NAME)
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_iram0.sv
-connect_core_tlm2pin=core0,iram0,tlm2pin_iram0
-connect_tlm2pin_proxy=tlm2pin_iram0,iram0
-connect_clock_proxy=CLK,iram0

-memory_pin_port=iram1
-set_tlm2pin_parm=cosim=true
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_iram1.so
#ifdef SYNOPSYS_NAME_DOUBLING
-set_tlm2pin_parm=dso_cookie=$(TOP).iram1.iram1
#else
-set_tlm2pin_parm=dso_cookie=$(TOP).iram1
#endif
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=module_name=peregrine_iram1
-set_proxy_parm=readmemh_file_name=$(READMEMH_FILE_NAME)
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_iram1.sv
-connect_core_tlm2pin=core0,iram1,tlm2pin_iram1
-connect_tlm2pin_proxy=tlm2pin_iram1,iram1
-connect_clock_proxy=CLK,iram1

-memory_pin_port=dram0
-set_tlm2pin_parm=cosim=true
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_dram0.so
#ifdef SYNOPSYS_NAME_DOUBLING
-set_tlm2pin_parm=dso_cookie=$(TOP).dram0.dram0
#else
-set_tlm2pin_parm=dso_cookie=$(TOP).dram0
#endif
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=module_name=peregrine_dram0
-set_proxy_parm=readmemh_file_name=$(READMEMH_FILE_NAME)
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_dram0.sv
-connect_core_tlm2pin=core0,dram0,tlm2pin_dram0
-connect_tlm2pin_proxy=tlm2pin_dram0,dram0
-connect_clock_proxy=CLK,dram0

-memory_pin_port=dram1
-set_tlm2pin_parm=cosim=true
-set_tlm2pin_parm=dso_name=$(XTSC_SCRIPT_FILE_PATH)/dso/$(XTSC_RUN_COSIM_VENDOR)/lib_peregrine_dram1.so
#ifdef SYNOPSYS_NAME_DOUBLING
-set_tlm2pin_parm=dso_cookie=$(TOP).dram1.dram1
#else
-set_tlm2pin_parm=dso_cookie=$(TOP).dram1
#endif
-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=module_name=peregrine_dram1
-set_proxy_parm=readmemh_file_name=$(READMEMH_FILE_NAME)
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/../../verilog.sources/peregrine_dram1.sv
-connect_core_tlm2pin=core0,dram1,tlm2pin_dram1
-connect_tlm2pin_proxy=tlm2pin_dram1,dram1
-connect_clock_proxy=CLK,dram1


// Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include "../common.inc"

-no_default_local_mem
-xtensa_params=$(XTSC_SCRIPT_FILE_PATH)/../../../TIE/example.tdk

-set_make_parm=testbench_name=Testbench
-set_make_parm=verilog_files=$(XTSC_SCRIPT_FILE_PATH)/Testbench.v


-set_wrapper_parm=vcd_handle=waveforms
-create_wrapper=Xtensa0

-create_clock=clk
-connect_clock_wrapper=clk,CLK

-set_core_parm=SimTargetOutput=core0_output.log
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/core0.out
-create_core=core0
-create_memory=core0_pif

-set_router_parm=routing_table=$(XTSC_SCRIPT_FILE_PATH)/routing.tab
-set_router_parm=num_slaves=2
-set_router_parm=default_port_num=0
-create_router=router
-set_mmio_parm=definition_file=$(XTSC_SCRIPT_FILE_PATH)/mmio.txt
-set_mmio_parm=byte_width=4
-set_mmio_parm=swizzle_bytes=false
-create_mmio=mmio

-connect_core_router=core0,pif,router
-connect_router_memory=router,0,core0_pif
-connect_router_mmio=router,1,mmio
-connect_mmio_wrapper=mmio,BInterruptXX
-connect_wrapper_mmio=TIE_EXPSTATE,EXPSTATE,mmio


-create_wrapper=Xtensa1

-set_core_parm=SimTargetOutput=core1_output.log
-set_core_parm=SimPinLevelInterfaces=EXPSTATE,BInterruptXX
-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/core1.out
-create_core=core1
-create_memory=core1_pif
-connect_core_wrapper=core1,EXPSTATE
-connect_wrapper_core=BInterruptXX,core1
-connect_core_memory=core1,0,core1_pif


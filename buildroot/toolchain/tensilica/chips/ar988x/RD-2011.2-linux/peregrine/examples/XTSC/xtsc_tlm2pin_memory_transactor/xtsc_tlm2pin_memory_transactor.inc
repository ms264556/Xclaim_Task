// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-set_core_parm=SimTargetOutput=core0_output.log
-core_program=target/memory_test.out
-create_core=core0

-create_vcd=waveforms

-memory_pin_port=pif
-set_memory_pin_parm=vcd_handle=waveforms
-connect_core_tlm2pin=core0,tlm2pin_pif
-connect_tlm2pin_memory_pin=tlm2pin_pif,mem_pif

-memory_pin_port=iram0
-set_memory_pin_parm=vcd_handle=waveforms
-connect_core_tlm2pin=core0,tlm2pin_iram0
-connect_tlm2pin_memory_pin=tlm2pin_iram0,mem_iram0

-memory_pin_port=iram1
-set_memory_pin_parm=vcd_handle=waveforms
-connect_core_tlm2pin=core0,tlm2pin_iram1
-connect_tlm2pin_memory_pin=tlm2pin_iram1,mem_iram1

-memory_pin_port=dram0
-set_memory_pin_parm=vcd_handle=waveforms
-connect_core_tlm2pin=core0,tlm2pin_dram0
-connect_tlm2pin_memory_pin=tlm2pin_dram0,mem_dram0

-memory_pin_port=dram1
-set_memory_pin_parm=vcd_handle=waveforms
-connect_core_tlm2pin=core0,tlm2pin_dram1
-connect_tlm2pin_memory_pin=tlm2pin_dram1,mem_dram1


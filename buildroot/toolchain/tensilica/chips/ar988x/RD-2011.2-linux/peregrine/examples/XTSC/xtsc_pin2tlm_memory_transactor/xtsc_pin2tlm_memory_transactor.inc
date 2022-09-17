// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-no_default_mem

-create_vcd=waveforms

-set_clock_parm=vcd_handle=waveforms
-create_clock=clk

-set_core_parm=SimTargetOutput=core0_output.log
-core_program=target/hello.out

-memory_port=dram0
-connect_core_memory=core0,core0_dram0

-memory_port=pif
-connect_core_memory=core0,core0_pif

-set_master_parm=script_file=master.vec

-memory_pin_port=pif
-set_tlm2pin_parm=inbound_pif=true
-set_tlm2pin_parm=write_responses=false
-set_tlm2pin_parm=vcd_handle=waveforms
-connect_master_tlm2pin=master,tlm2pin

-set_pin2tlm_parm=inbound_pif=true
-set_pin2tlm_parm=vcd_handle=waveforms
-connect_tlm2pin_pin2tlm=tlm2pin,pin2tlm

-connect_pin2tlm_core=pin2tlm,core0



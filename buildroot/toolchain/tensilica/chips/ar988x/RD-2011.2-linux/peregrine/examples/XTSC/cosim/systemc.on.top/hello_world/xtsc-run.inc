// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

#include "../common.inc"

-set_proxy_parm=vcd_handle=waveforms
-set_proxy_parm=module_name=ResetBlock
-set_proxy_parm=verilog_file=$(XTSC_SCRIPT_FILE_PATH)/ResetBlock.v

-core_program=$(XTSC_SCRIPT_FILE_PATH)/target/hello.out
-set_core_parm=SimPinLevelInterfaces=BReset
-set_core_parm=SimVcdHandle=waveforms

-connect_clock_proxy=CLK,BReset
-connect_proxy_core=BReset,BReset,core0

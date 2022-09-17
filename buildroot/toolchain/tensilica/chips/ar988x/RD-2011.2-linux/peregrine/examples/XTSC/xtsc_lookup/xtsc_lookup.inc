// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-xtensa_params=../TIE/example.tdk
-set_core_parm=SimTargetOutput=core0_output.log
-core_program=target/lookup_test.out
-set_lookup_parm=address_bit_width=8
-set_lookup_parm=data_bit_width=32
-set_lookup_parm=has_ready=true
-set_lookup_parm=lookup_table=lut.rom
-set_lookup_parm=default_data=0xDEADBEEF
-connect_core_lookup=core0,lut,tbl

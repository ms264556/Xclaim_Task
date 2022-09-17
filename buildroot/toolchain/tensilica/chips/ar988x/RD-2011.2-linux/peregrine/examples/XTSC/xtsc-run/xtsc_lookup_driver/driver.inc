// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=TextLogger.txt
-squelch_ctor=verbose
-set_driver_parm=address_bit_width=8
-set_driver_parm=data_bit_width=48
-set_driver_parm=has_ready=true
-set_driver_parm=latency=1
-set_driver_parm=script_file=driver.vec
-set_lookup_parm=address_bit_width=8
-set_lookup_parm=data_bit_width=48
-set_lookup_parm=has_ready=true
-set_lookup_parm=enforce_latency=true
-set_lookup_parm=latency=1
-set_lookup_parm=delay=3
-set_lookup_parm=lookup_table=lut.rom
-set_lookup_parm=default_data=0xBAD1BAD1BAD1
-connect_driver_lookup=driver,lut
// -cycle_limit=100

// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-set_core_parm=SimTargetOutput=core0_output.log
-set_core_parm=SimClients=trace --level 1 core0_trace.log
-core_program=target/memory_test.out
-create_core=core0

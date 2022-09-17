// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-no_default_mem
-set_core_parm=SimTargetOutput=core0_output.log
-core_program=target/router_test.out
-create_core=core0
-set_router_parm=routing_table=routing.tab
-set_router_parm=num_slaves=2
-set_router_parm=default_port_num=1
-connect_core_router=core0,router
-create_memory=sysrom
-create_memory=sysram
-connect_router_memory=router,0,sysrom
-connect_router_memory=router,1,sysram

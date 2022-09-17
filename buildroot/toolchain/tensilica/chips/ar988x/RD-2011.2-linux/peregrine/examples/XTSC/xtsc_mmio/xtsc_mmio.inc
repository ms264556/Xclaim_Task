// Customer ID=8327; Build=0x3b95c; Copyright (c) 2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
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
-core_program=target/core0.out
-create_core=core0
-create_memory=core0_pif
-set_core_parm=SimTargetOutput=core1_output.log
-core_program=target/core1.out
-create_core=core1
-create_memory=core1_pif
-set_router_parm=routing_table=routing.tab
-set_router_parm=num_slaves=2
-set_router_parm=default_port_num=0
-create_router=router
-set_mmio_parm=definition_file=mmio.txt
-set_mmio_parm=byte_width=4
-set_mmio_parm=swizzle_bytes=false
-create_mmio=mmio

-connect_core_router=core0,pif,router
-connect_router_memory=router,0,core0_pif
-connect_router_mmio=router,1,mmio

-connect_core_mmio=core1,EXPSTATE,mmio
-connect_mmio_core=mmio,BInterruptXX,core1
-connect_core_memory=core1,0,core1_pif

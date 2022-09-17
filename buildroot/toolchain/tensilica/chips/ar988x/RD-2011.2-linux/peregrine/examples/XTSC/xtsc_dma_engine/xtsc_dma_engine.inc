// Customer ID=8327; Build=0x3b95c; Copyright (c) 2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-no_default_local_memories

-core_program=target/main.out
-create_core=core0

-set_router_parm=immediate_timing=true
-set_router_parm=num_slaves=3
-set_router_parm=default_port_num=0
-set_router_parm=routing_table=core0_router.tab
-create_router=core0_router

-set_dma_parm=byte_width=4
-set_dma_parm=clear_notify_value=false
-set_dma_parm=reg_base_address=0xC0001000
-create_dma=dma

-set_router_parm=immediate_timing=true
-set_router_parm=num_slaves=3
-set_router_parm=default_port_num=0
-set_router_parm=routing_table=dma_router.tab
-create_router=dma_router

-set_arbiter_parm=num_masters=2
-create_arbiter=pif_arbiter

-set_arbiter_parm=num_masters=2
-create_arbiter=mmio_arbiter

-set_mmio_parm=definition_file=mmio.def
-set_mmio_parm=swizzle_bytes=false
-create_mmio=mmio

-memory_port=pif
-set_memory_parm=initial_value_file=pifmem.dat
-create_memory=pifmem

-memory_port=dram0
-create_memory=dram0

-connect_core_router=core0,pif,core0_router
-connect_router_dma=core0_router,1,dma
-connect_router_arbiter=core0_router,0,1,pif_arbiter
-connect_dma_router=dma,dma_router
-connect_router_core=dma_router,1,inbound_pif,core0
-connect_router_arbiter=dma_router,0,0,pif_arbiter
-connect_router_arbiter=dma_router,2,1,mmio_arbiter
-connect_router_arbiter=core0_router,2,0,mmio_arbiter
-connect_arbiter_mmio=mmio_arbiter,mmio
-connect_mmio_core=mmio,BInterrupt00,core0
-connect_arbiter_memory=pif_arbiter,pifmem
-connect_core_memory=core0,dram0


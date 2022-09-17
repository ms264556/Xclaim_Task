// Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


-core_program=main.out
-create_core=core0

-set_router_parm=num_slaves=2
-set_router_parm=routing_table=routing.tab
-set_router_parm=immediate_timing=true
-set_router_parm=default_port_num=0
-create_router=router

-memory_port=pif
-create_memory=pif

-set_mmio_parm=definition_file=mmio.txt
-set_mmio_parm=always_write=false
-set_mmio_parm=swizzle_bytes=false
-create_mmio=mmio

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt00.vec
-create_source=BInterrupt00

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt01.vec
-create_source=BInterrupt01

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt02.vec
-create_source=BInterrupt02

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt03.vec
-create_source=BInterrupt03

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt04.vec
-create_source=BInterrupt04

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt05.vec
-create_source=BInterrupt05

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt06.vec
-create_source=BInterrupt06

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt07.vec
-create_source=BInterrupt07

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt08.vec
-create_source=BInterrupt08

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt09.vec
-create_source=BInterrupt09

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt10.vec
-create_source=BInterrupt10

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt11.vec
-create_source=BInterrupt11

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt12.vec
-create_source=BInterrupt12

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt13.vec
-create_source=BInterrupt13

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt14.vec
-create_source=BInterrupt14

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt15.vec
-create_source=BInterrupt15

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt16.vec
-create_source=BInterrupt16

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt17.vec
-create_source=BInterrupt17

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt18.vec
-create_source=BInterrupt18

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt19.vec
-create_source=BInterrupt19

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt20.vec
-create_source=BInterrupt20

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt21.vec
-create_source=BInterrupt21

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt22.vec
-create_source=BInterrupt22

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt23.vec
-create_source=BInterrupt23

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt24.vec
-create_source=BInterrupt24

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt25.vec
-create_source=BInterrupt25

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt26.vec
-create_source=BInterrupt26

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt27.vec
-create_source=BInterrupt27

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt28.vec
-create_source=BInterrupt28

-set_source_parm=bit_width=1
-set_source_parm=control=true
-set_source_parm=script_file=BInterrupt29.vec
-create_source=BInterrupt29

-connect_core_router=core0,pif,router
-connect_router_memory=router,0,pif
-connect_router_mmio=router,1,mmio
-connect_mmio_source=mmio,BInterrupt00,BInterrupt00
-connect_mmio_source=mmio,BInterrupt01,BInterrupt01
-connect_mmio_source=mmio,BInterrupt02,BInterrupt02
-connect_mmio_source=mmio,BInterrupt03,BInterrupt03
-connect_mmio_source=mmio,BInterrupt04,BInterrupt04
-connect_mmio_source=mmio,BInterrupt05,BInterrupt05
-connect_mmio_source=mmio,BInterrupt06,BInterrupt06
-connect_mmio_source=mmio,BInterrupt07,BInterrupt07
-connect_mmio_source=mmio,BInterrupt08,BInterrupt08
-connect_mmio_source=mmio,BInterrupt09,BInterrupt09
-connect_mmio_source=mmio,BInterrupt10,BInterrupt10
-connect_mmio_source=mmio,BInterrupt11,BInterrupt11
-connect_mmio_source=mmio,BInterrupt12,BInterrupt12
-connect_mmio_source=mmio,BInterrupt13,BInterrupt13
-connect_mmio_source=mmio,BInterrupt14,BInterrupt14
-connect_mmio_source=mmio,BInterrupt15,BInterrupt15
-connect_mmio_source=mmio,BInterrupt16,BInterrupt16
-connect_mmio_source=mmio,BInterrupt17,BInterrupt17
-connect_mmio_source=mmio,BInterrupt18,BInterrupt18
-connect_mmio_source=mmio,BInterrupt19,BInterrupt19
-connect_mmio_source=mmio,BInterrupt20,BInterrupt20
-connect_mmio_source=mmio,BInterrupt21,BInterrupt21
-connect_mmio_source=mmio,BInterrupt22,BInterrupt22
-connect_mmio_source=mmio,BInterrupt23,BInterrupt23
-connect_mmio_source=mmio,BInterrupt24,BInterrupt24
-connect_mmio_source=mmio,BInterrupt25,BInterrupt25
-connect_mmio_source=mmio,BInterrupt26,BInterrupt26
-connect_mmio_source=mmio,BInterrupt27,BInterrupt27
-connect_mmio_source=mmio,BInterrupt28,BInterrupt28
-connect_mmio_source=mmio,BInterrupt29,BInterrupt29
-connect_source_core=BInterrupt00,core0
-connect_source_core=BInterrupt01,core0
-connect_source_core=BInterrupt02,core0
-connect_source_core=BInterrupt03,core0
-connect_source_core=BInterrupt04,core0
-connect_source_core=BInterrupt05,core0
-connect_source_core=BInterrupt06,core0
-connect_source_core=BInterrupt07,core0
-connect_source_core=BInterrupt08,core0
-connect_source_core=BInterrupt09,core0
-connect_source_core=BInterrupt10,core0
-connect_source_core=BInterrupt11,core0
-connect_source_core=BInterrupt12,core0
-connect_source_core=BInterrupt13,core0
-connect_source_core=BInterrupt14,core0
-connect_source_core=BInterrupt15,core0
-connect_source_core=BInterrupt16,core0
-connect_source_core=BInterrupt17,core0
-connect_source_core=BInterrupt18,core0
-connect_source_core=BInterrupt19,core0
-connect_source_core=BInterrupt20,core0
-connect_source_core=BInterrupt21,core0
-connect_source_core=BInterrupt22,core0
-connect_source_core=BInterrupt23,core0
-connect_source_core=BInterrupt24,core0
-connect_source_core=BInterrupt25,core0
-connect_source_core=BInterrupt26,core0
-connect_source_core=BInterrupt27,core0
-connect_source_core=BInterrupt28,core0
-connect_source_core=BInterrupt29,core0


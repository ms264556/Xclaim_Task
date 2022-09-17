// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt
-xtensa_params=../TIE/example.tdk

-set_producer_parm=script_file=loader_driver.vec
-set_producer_parm=control=true
-create_producer=loader_driver

-create_loader=loader

-core_program=
-set_core_parm=ProcessorID=0
-create_core=core0

-core_program=
-set_core_parm=ProcessorID=1
-create_core=core1

-set_queue_parm=depth=2
-set_queue_parm=bit_width=96
-create_queue=Q01

-set_queue_parm=depth=2
-set_queue_parm=bit_width=96
-create_queue=Q10

-set_logic_parm=definition_file=logic.def
-create_logic=logic

-connect_core_queue=core0,OUTQ1,Q01
-connect_queue_core=Q01,INQ1,core1

-connect_core_queue=core1,OUTQ1,Q10
-connect_queue_core=Q10,INQ1,core0

-connect_producer_loader=loader_driver,loader
-connect_loader_core=loader,core0
-connect_core_core=core0,core1
-connect_core_loader=core1,loader
-connect_core_logic=core0,CoreHalted,CoreHalted0,logic
-connect_core_logic=core1,CoreHalted,CoreHalted1,logic
-connect_logic_producer=logic,JobDone,loader_driver

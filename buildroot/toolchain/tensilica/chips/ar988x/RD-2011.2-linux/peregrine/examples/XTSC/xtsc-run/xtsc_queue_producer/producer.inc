// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../../TextLogger.txt
-xtensa-params=../../TIE/example.tdk
-set_producer_parm=script_file=producer.vec
-set_producer_parm=bit_width=16
-set_queue_parm=bit_width=16
-set_core_parm=SimTargetOutput=core0_output.log
-core_program=consumer.out
-connect_producer_queue=producer,queue
-connect_queue_core=queue,INQ2,core0

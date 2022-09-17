// Customer ID=8327; Build=0x3b95c; Copyright (c) 2008-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


// Instructions: Include this file in your xtsc-run script like this
//   #include "<PutPathHere>/extra.wrapper.IO.inc"
//   --set_wrapper_parm=extra_inputs=$(EXTRA_INPUTS)
//   --set_wrapper_parm=extra_outputs=$(EXTRA_OUTPUTS)
//   --create_wrapper=my_wrapper

#define EXTRA_INPUTS=CLK,1,TDebugInterrupt,1,OCDHaltOnReset,1,JTCK,1,JTDI,1,TClockDR,1,TInst,5,TResetB,1,TShiftDR,1,TUpdateDR,1,TUpdateIR,1,TMode,1
#define EXTRA_OUTPUTS=XOCDModePulse,1,XTDO,1,XTDV,1,XOCDMode,1

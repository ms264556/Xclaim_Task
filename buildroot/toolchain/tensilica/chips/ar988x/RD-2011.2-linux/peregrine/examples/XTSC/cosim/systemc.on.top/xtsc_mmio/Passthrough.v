// Customer ID=8327; Build=0x3b95c; Copyright (c) 2008-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

`define DISPLAY_INPUTS

module Passthrough(
  mmio_BInterruptXX,
  EXPSTATE,
  TIE_EXPSTATE,
  BInterruptXX 
);

  input                 mmio_BInterruptXX;
  input  [  31:0]       TIE_EXPSTATE;

  output [  31:0]       EXPSTATE;
  output                BInterruptXX;

  assign EXPSTATE     = TIE_EXPSTATE;
  assign BInterruptXX = mmio_BInterruptXX;

`ifdef DISPLAY_INPUTS
  always @(mmio_BInterruptXX) $display("%t mmio_BInterruptXX = 0x%h", $time, mmio_BInterruptXX);
  always @(TIE_EXPSTATE     ) $display("%t TIE_EXPSTATE      = 0x%h", $time, TIE_EXPSTATE     );
`endif

endmodule

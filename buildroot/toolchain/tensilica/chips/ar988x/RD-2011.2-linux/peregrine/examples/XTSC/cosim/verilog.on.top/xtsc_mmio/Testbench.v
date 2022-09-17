// Customer ID=8327; Build=0x3b95c; Copyright (c) 2008-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

`define DISPLAY_IO

module Testbench;   // {

  wire                  CLK;

  wire                  BInterruptXX;
  wire          [31:0]  TIE_EXPSTATE;


  Xtensa0 core0(
    .CLK                (CLK),
    .BInterruptXX       (BInterruptXX),
    .TIE_EXPSTATE       (TIE_EXPSTATE)
  );

  Xtensa1 core1(
    .BInterruptXX       (BInterruptXX),
    .TIE_EXPSTATE       (TIE_EXPSTATE)
  );

  initial begin
    $dumpfile("waveforms.verilog.vcd");
    $dumpvars(0, Testbench);
  end

`ifdef DISPLAY_IO
  always @(TIE_EXPSTATE) begin $display("%t TIE_EXPSTATE  = 0x%h", $time, TIE_EXPSTATE); end
  always @(BInterruptXX) begin $display("%t BInterruptXX  = 0x%h", $time, BInterruptXX); end
`endif


endmodule   // }




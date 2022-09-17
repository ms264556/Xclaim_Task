// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

//`timescale 1ns / 1ps

// Note: This model does not support latency

`define DISPLAY_IO

module lookup(CLK, TIE_lut_Out, TIE_lut_Out_Req, TIE_lut_In, TIE_lut_Rdy);

  input                 CLK;
  input  [   7:0]       TIE_lut_Out;
  input                 TIE_lut_Out_Req;                        // We don't use request

  output [  31:0]       TIE_lut_In;
  output                TIE_lut_Rdy;

  reg    [  31:0]       TIE_lut_In      = 32'hDEADBEEF;
  reg                   TIE_lut_Rdy     = 1'h1;                 // We're always ready

  initial begin
    if($test$plusargs("dumpvars")) begin
      $dumpvars();
    end
  end

`ifdef DISPLAY_IO
  always @(TIE_lut_Out    ) $display("%t TIE_lut_Out     = 0x%h", $time, TIE_lut_Out);
  always @(TIE_lut_Out_Req) $display("%t TIE_lut_Out_Req = %d",   $time, TIE_lut_Out_Req);
  always @(TIE_lut_In     ) $display("%t TIE_lut_In      = 0x%h", $time, TIE_lut_In );
`endif

  always @(posedge CLK) begin
         if (TIE_lut_Out == 8'h00) TIE_lut_In <= 32'hfacef00d;
    else if (TIE_lut_Out == 8'h33) TIE_lut_In <= 32'h33333333;
    else if (TIE_lut_Out == 8'h77) TIE_lut_In <= 32'h77777777;
    else if (TIE_lut_Out == 8'hbb) TIE_lut_In <= 32'hbbbbbbbb;
    else if (TIE_lut_Out == 8'h32) TIE_lut_In <= 32'h12345678;
    else if (TIE_lut_Out == 8'hff) TIE_lut_In <= 32'hffffffff;
    else if (TIE_lut_Out == 8'h10) TIE_lut_In <= 32'h10101010;
    else if (TIE_lut_Out == 8'h11) TIE_lut_In <= 32'hcafebabe;
    else if (TIE_lut_Out == 8'h22) TIE_lut_In <= 32'h22222222;
    else if (TIE_lut_Out == 8'hdd) TIE_lut_In <= 32'hdddddddd;
    else                           TIE_lut_In <= 32'hDEADBEEF;
  end


endmodule


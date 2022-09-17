// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

// `timescale 1ns / 1ps

`define ABITS           8
`define INITIAL_VALUE   32'hBAD1BAD1
`define NO_ACTION_VALUE 32'hDEADBEEF


module lookup(CLK, TIE_lookup_ram_Out_Req, TIE_lookup_ram_Out, TIE_lookup_ram_In);

  input                         CLK;
  input                         TIE_lookup_ram_Out_Req;
  input         [40:0]          TIE_lookup_ram_Out;
  output        [31:0]          TIE_lookup_ram_In;

  reg           [31:0]          TIE_lookup_ram_In       = `INITIAL_VALUE;
  reg           [31:0]          mem[0:(1<<`ABITS)-1];

  wire          [`ABITS-1:0]    address;
  wire          [31:0]          data;
  wire                          write;

  integer                       i;

  assign write      = TIE_lookup_ram_Out[40:40];
  assign address    = TIE_lookup_ram_Out[39:32];
  assign data       = TIE_lookup_ram_Out[31:0];


  initial begin
    if($test$plusargs("dumpvars")) begin
      $dumpvars();
    end
  end

  initial begin
      for (i=0; i<(1<<`ABITS); i=i+1) begin
          mem[i] = `INITIAL_VALUE;
      end
  end


  always @(posedge CLK) begin
      if (TIE_lookup_ram_Out_Req) begin
          if (write) begin
              mem[address] <= data;
              $display("%t writing mem[0x%x] <= 0x%x", $time, address, data);
          end
          else begin
              TIE_lookup_ram_In <= mem[address];
              $display("%t reading mem[0x%x] => 0x%x", $time, address, mem[address]);
          end
      end
      else begin
          TIE_lookup_ram_In <= `NO_ACTION_VALUE;
      end
  end


endmodule

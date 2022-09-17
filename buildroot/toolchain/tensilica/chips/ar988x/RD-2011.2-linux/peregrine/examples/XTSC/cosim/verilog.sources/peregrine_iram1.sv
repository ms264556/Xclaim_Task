// Customer ID=8327; Build=0x3b95c; Copyright (c) 2003-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


/**********************************************************************
 *                    -- Module: peregrine_iram1 --
 **********************************************************************/
module peregrine_iram1 (

	IRam1Addr,
	IRam1En,
	IRam1Wr,
	IRam1WrData,
	IRam1LoadStore,
	IRam1Data,
	CLK);

input  [16:0] IRam1Addr;     
input         IRam1En;       
input         IRam1Wr;       
input  [31:0] IRam1WrData;   
input         IRam1LoadStore; 
output [31:0] IRam1Data;     
input         CLK;           



   parameter file_name = "readmemh.data/iram1.data";

   initial begin
    if($test$plusargs("dumpvars")) begin
      $dumpvars();
    end
   end

   initial begin
     repeat (1) @(posedge CLK);
     if ($test$plusargs("preloadmems") && (file_name != "<none>")) begin
       $display("%t Preloading memory %m from %s", $time, file_name);
       $readmemh(file_name, mem.dataArray);
     end
   end // initial begin

    peregrine_iram1_xtmemory mem(
			.CLK(CLK),
			.address0 (IRam1Addr),
			.enable0 (IRam1En),
			.writeData0 (IRam1WrData),
			.writeEnable0 (IRam1Wr),
			.xtout0 (IRam1Data)
			);





  int mem_beg  =  32'h00980000;
  int mem_end  =  32'h009fffff;

`ifdef MODEL_TECH
  export "DPI-SC" function peek_peregrine_iram1;
`else
  export "DPI-C" function peek_peregrine_iram1;
`endif
  function int peek_peregrine_iram1;
    input int unsigned address;
    inout int unsigned byte_lanes_3210;
    reg [31:0] peek_data;
    int unsigned index;
    int unsigned word;
    word = 0;
    peek_peregrine_iram1 = 1;
    if (!(address & 2'h3) && ((address >= mem_beg) && (address <= mem_end))) begin
      peek_peregrine_iram1 = 0;
      index = (address - mem_beg) >> 2;
      peek_data = mem.dataArray[index];
      byte_lanes_3210 = (peek_data >> (word * 32)) & 32'hFFFFFFFF;
    end
  endfunction


`ifdef MODEL_TECH
  export "DPI-SC" function poke_peregrine_iram1;
`else
  export "DPI-C" function poke_peregrine_iram1;
`endif
  function int poke_peregrine_iram1;
    input int unsigned address;
    input int unsigned byte_lanes_3210;
    reg [31:0] poke_data;
    int unsigned index;
    int unsigned word;
    word = 0;
    poke_peregrine_iram1 = 1;
    if (!(address & 2'h3) && ((address >= mem_beg) && (address <= mem_end))) begin
      index = (address - mem_beg) >> 2;
      poke_data = mem.dataArray[index];
      poke_data = byte_lanes_3210;
      poke_peregrine_iram1 = 0;
      mem.dataArray[index] = poke_data;
    end
  endfunction


endmodule




/**********************************************************************
 *                -- Module: peregrine_iram1_xtmemory --
 **********************************************************************/
module peregrine_iram1_xtmemory (

	CLK,
	enable0,
	address0,
	writeData0,
	writeEnable0,
	xtout0);

parameter nwords=131072,
	width=32,
	awidth=17,
	lsb=0;

input               CLK;         
input               enable0;     
input  [awidth-1:0] address0;    
input  [width-1:0]  writeData0;  
input               writeEnable0; 
output [width-1:0]  xtout0;      
   
   reg [width-1:0] 	    dataArray [0:nwords-1];


   reg [width-1:0] 	    data0;
   reg [width-1:0] 	    data0_M;
   reg 			    enable20;


   always @(posedge CLK) begin
      //   --- Read/Write port 0 ---
      if (enable0 && writeEnable0) begin
	    // write
	    data0_M = writeData0;
	    dataArray[address0[awidth-1:lsb]] = data0_M;
      end // if (enable && writeEnable)
   end // always @ (posedge CLK)

   always @(posedge CLK) begin
      enable20 <= #1 enable0;
      if (enable0) begin
	data0 <= #1 dataArray[address0[awidth-1:lsb]];
      end // if (enable)
   end // always @ (posedge CLK)

   assign xtout0 =data0;

endmodule



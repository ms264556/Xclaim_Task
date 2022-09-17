// Customer ID=8327; Build=0x3b95c; Copyright (c) 2008-2009 Tensilica Inc.  ALL RIGHTS RESERVED.
// 
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.




// To enable route ID, uncomment the following and change xxx to the number of bits minus 1
// `define peregrine_siomem_ROUTE_ID



module peregrine_siomem(
  CLK,

  POReqValid,
  PIReqRdy,
  POReqCntl,
  POReqAdrs,
  POReqData,
  POReqDataBE,
  POReqId,
  POReqPriority,

  PIRespValid,
  PORespRdy,
  PIRespCntl,
  PIRespData,
  PIRespId,
  PIRespPriority

`ifdef peregrine_siomem_ROUTE_ID
  ,
  POReqRouteId,
  PIRespRouteId
`endif
);

  input                 CLK;

  input                 POReqValid;
  output                PIReqRdy;
  input         [  7:0] POReqCntl;
  input         [ 31:0] POReqAdrs;
  input         [ 31:0] POReqData;
  input         [  3:0] POReqDataBE;
  input         [  5:0] POReqId;
  input         [  1:0] POReqPriority;

  output                PIRespValid;
  input                 PORespRdy;
  output        [  7:0] PIRespCntl;
  output        [ 31:0] PIRespData;
  output        [  5:0] PIRespId;
  output        [  1:0] PIRespPriority;

`ifdef peregrine_siomem_ROUTE_ID
  // Change xxx to the number of bits minus 1
  input         [xxx:0] POReqRouteId;
  output        [xxx:0] PIRespRouteId;
  reg           [xxx:0] PIRespRouteId           =  0;
`endif

  reg           [  7:0] PIRespCntl              =   8'h0;
  reg           [  5:0] PIRespId                =   6'h0;
  reg           [  1:0] PIRespPriority          =   2'h0;
  reg                   PIReqRdy                =   1'h0;
  reg                   PIRespValid             =   1'h0;

  reg                   siomem_write            =   1'h0;
  reg           [ 26:0] siomem_index            =  27'h0;
  reg           [  3:0] siomem_byte_enables     =   4'h0;
  reg           [ 31:0] siomem_write_data       =  32'h0;
  wire          [ 31:0] siomem_read_data;

  // Synopsys Y-2006.06-SP1-9 does not support verilog parameters in a SystemC-on-Top simulation, so we hard code a value
  parameter file_name = "../siomem.data";



  integer               bus_byte_width          = 4;

  integer               READ                    =   4'b0000;
  integer               WRITE                   =   4'b1000;
  integer               BLOCK_READ              =   4'b0001;
  integer               BURST_READ              =   4'b0011;
  integer               BLOCK_WRITE             =   4'b1001;
  integer               BURST_WRITE             =   4'b1011;
  integer               RCW                     =   4'b0101;

  integer               RSP_NONE                =   8'hFF;       // Value to drive when there is no response
  integer               RSP_OK                  =   7'b0000000_; // PIRespCntl[7:1]
  integer               RSP_ADDR_ERROR          =   7'b0000001_; // PIRespCntl[7:1]

  integer               request_prev            =   1'h0;        // There was a request on the prev cycle which was accepted
  integer               request_last            =   1'h0;        // The accepted request was a last-transfer request
  integer               num_responses           =   0;           // Num of responses expected due to accepted last-xfer req
  integer               response_prev           =   1'h0;        // There was a response on the prev cycle which was accepted
  integer               response_count          =   0;           // Count down of remaining responses

  integer               siomem_hit              =   1'h0;        // Request accepted last cycle targets siomem
  integer               siomem_start            =  32'hC0000000; // Start address of siomem
  integer               siomem_end              =  32'hDFFFFFFF; // End address of siomem

  integer               address                 =  32'h00000000; // Save POReqAdrs of "first" transfer
  integer               error_resp_pending      =   1'h0;        // Keep track until last transfer request that error response is due 
  integer               first_transfer          =   1'h1;        // The next request to come will be a "first" transfer
  integer               write_beat              =   5'h0;        // Keep track of BLOCK_WRITE|BURST_WRITE beats
  integer               read_beat               =   5'h0;        // Keep track of BLOCK_READ|BURST_READ beats
  integer               block_read_max_beat     =  0;            // Support BLOCK_READ wrap-around (for critical word first)
  integer               block_read_min_beat     =  0;            // Support BLOCK_READ wrap-around (for critical word first)
  integer               is_rcw                  =   1'h0;        // RCW
  integer               is_write                =   1'h0;        // WRITE|BLOCK_WRITE|BURST_WRITE|RCW
  integer               addr_byte_lanes         =   2'h0;        // Bits of POReqAdrs which indicate byte lane
  integer               aligned_ok              =   1'h0;        // Legal combination of address low-order bits and byte enables
  integer               do_write                =   1'h0;        // WRITE|BLOCK_WRITE|BURST_WRITE|(RCW#2 && rcw_data_matches)
  integer               start_read              =   1'h0;        // READ|BLOCK_READ|BURST_READ|RCW#1
  integer               read_byte_enables       =   4'h0;        // Remember read byte enables
  integer               reading_siomem          =   1'h0;        // Remember if reads are from siomem
  wire                  reading_siomem_ud;                       // Remember if reads are from siomem (unit delay)
  integer               reading                 =   1'h0;        // Keep track of whether reads are in progress
  integer               reading_delayed         =   1'h0;        // delayed one cycle
  integer               do_read                 =   1'h0;        // Do a read this cycle
  wire                  do_read_ud;                              // Do a read this cycle (unit delay)
  integer               compare_rcw_data        =   1'h0;        // Flag to trigger an RCW compare at negedge clock
  integer               rcw_data_matches        =   1'h0;        // True if RCW compare data matches memory
  reg           [ 31:0] rcw_compare_data        =  32'h0;        // POReqData from RCW#1
  reg           [ 31:0] rcw_old_data            =  32'h0;        // Original contents of memory at RCW address
  reg           [  3:0] rcw_byte_enables        =   4'h0;        // POReqDataBE from RCW#1

  reg   /*sparse*/      [ 31:0] mem[0:27'h7FFFFFF];
  reg                   [ 31:0] wdata;
  wire                  [ 31:0] rdata;



  initial begin
    if($test$plusargs("dumpvars")) begin
      $dumpvars();
    end
    if (!$test$plusargs("preloadmems") || (file_name == "") || (file_name == "<none>")) begin
      $display("%t Not preloading memory %m.", $time);
    end
    else begin
      $display("%t Preloading memory %m from %s", $time, file_name);
      $readmemh(file_name, mem);
    end
  end



  always @(posedge CLK) begin
    reading_delayed     = reading;
    response_prev       = PIRespValid && PORespRdy;
    request_prev        = POReqValid && PIReqRdy;
    request_last        = request_prev && POReqCntl[0];
    is_rcw              = request_prev && (POReqCntl[7:4] == RCW);
    compare_rcw_data    = (is_rcw && first_transfer) ? 1 : 0;
    is_write            = request_prev && ((POReqCntl[7:4] == WRITE) ||
                                           (POReqCntl[7:4] == BLOCK_WRITE) ||
                                           (POReqCntl[7:4] == BURST_WRITE) ||
                                           (POReqCntl[7:4] == RCW));
    addr_byte_lanes     = POReqAdrs[1:0];
    aligned_ok          = request_prev && (
                          ((addr_byte_lanes == 2'h0) && (POReqDataBE == 4'b0001)) ||
                          ((addr_byte_lanes == 2'h1) && (POReqDataBE == 4'b0010)) ||
                          ((addr_byte_lanes == 2'h2) && (POReqDataBE == 4'b0100)) ||
                          ((addr_byte_lanes == 2'h3) && (POReqDataBE == 4'b1000)) ||
                          ((addr_byte_lanes == 2'h0) && (POReqDataBE == 4'b0011)) ||
                          ((addr_byte_lanes == 2'h2) && (POReqDataBE == 4'b1100)) ||
                          ((addr_byte_lanes == 2'h0) && (POReqDataBE == 4'b1111)) ||
                          0);
    siomem_hit          = request_prev && (POReqAdrs >= siomem_start) && (POReqAdrs <= siomem_end);
    error_resp_pending  = (error_resp_pending && !response_prev) ||
                          (request_prev && !aligned_ok) ||
                          (request_prev && !siomem_hit);
    num_responses       = error_resp_pending             ? 1                     :  // An error response is pending
                          !request_last                  ? 0                     :  // Last cycle didn't have a last-xfer req accepted
                          (POReqCntl[7:4] == BLOCK_READ) ? (2 << POReqCntl[2:1]) :  // BLOCK_READ
                          (POReqCntl[7:4] == BURST_READ) ? (POReqCntl[3:1] + 1)  :  // BURST_READ
                          (POReqCntl[7:4] == WRITE)      ? 0                     :  // WRITE
                          (POReqCntl[7:4] == BLOCK_WRITE)? 0                     :  // BLOCK_WRITE
                          (POReqCntl[7:4] == BURST_WRITE)? 0                     :  // BURST_WRITE
                                                           1;                       // All others
    response_count      = (request_prev ? num_responses : (response_count - (response_prev ? 1 : 0)));
    PIRespValid         = (response_count != 0);
    PIReqRdy            = (response_count == 0);
    do_write            = request_prev && !error_resp_pending && ((POReqCntl[7:4] == WRITE) ||
                                                                  (POReqCntl[7:4] == BLOCK_WRITE) ||
                                                                  (POReqCntl[7:4] == BURST_WRITE) ||
                                                                 ((POReqCntl[7:4] == RCW) && request_last && rcw_data_matches));
    start_read          = request_prev && !error_resp_pending && ((POReqCntl[7:4] == READ) ||
                                                                  (POReqCntl[7:4] == BLOCK_READ) ||
                                                                  (POReqCntl[7:4] == BURST_READ) ||
                                                                 ((POReqCntl[7:4] == RCW) && !request_last));
    do_read             = start_read || (reading && (response_count != 0));
    write_beat          = !do_write ? write_beat : first_transfer ? 0 : (write_beat + (is_rcw ? 0 : 1));
    read_beat           = !do_read  ? read_beat  : start_read     ? 0 : (read_beat  + ((is_rcw || !response_prev) ? 0 : 1));

    if (request_prev && first_transfer) begin
      address           = POReqAdrs;
      PIRespId          = POReqId;
`ifdef peregrine_siomem_ROUTE_ID
      PIRespRouteId     = POReqRouteId;
`endif
      PIRespPriority    = POReqPriority;
      if (POReqCntl[7:4] == BLOCK_READ) begin
        block_read_max_beat = num_responses - ((address % (bus_byte_width * num_responses)) / bus_byte_width) - 1;
        block_read_min_beat = block_read_max_beat - num_responses + 1;
      end
      else begin
        block_read_max_beat = 16;
        block_read_min_beat = 0;
      end
    end

    if (do_read && (read_beat > block_read_max_beat)) begin
      read_beat         = block_read_min_beat;
    end

    rcw_compare_data    = compare_rcw_data ? POReqData   :  32'h0;
    rcw_byte_enables    = compare_rcw_data ? POReqDataBE :   4'h0;

    if (start_read) begin
      reading_siomem    = siomem_hit;
      read_byte_enables = POReqDataBE;
    end

    siomem_write                =   1'h0;
    siomem_index                =  27'h0;
    siomem_byte_enables         =   4'h0;
    siomem_write_data           =  32'h0;

    if (do_write) begin
      siomem_write              = 1'h1;
      siomem_index              = address[28:2] + write_beat;
      siomem_byte_enables       = POReqDataBE;
      siomem_write_data         = POReqData;
    end
    else if (do_read) begin
      if (reading_siomem) begin
        siomem_index            = address[28:2] + read_beat;
        siomem_byte_enables     = read_byte_enables;
      end
    end

    PIRespCntl         <= #1 ((response_count == 0) ? RSP_NONE : 
                              { (error_resp_pending ? RSP_ADDR_ERROR : RSP_OK), ((response_count == 1) ? 1'h1 : 1'h0)});

    reading            <= #1 (start_read || (reading_delayed && PIRespValid));
    first_transfer     <= #1 request_prev ? request_last : first_transfer;
  end


  always @(negedge CLK) begin
    rcw_data_matches = 1'h0;
    if (compare_rcw_data) begin
      rcw_old_data = siomem_read_data;
      rcw_data_matches = 1'h1;
      if ((rcw_byte_enables[ 0] && (rcw_compare_data[  7:  0] !== rcw_old_data[  7:  0])) ||
          (rcw_byte_enables[ 1] && (rcw_compare_data[ 15:  8] !== rcw_old_data[ 15:  8])) ||
          (rcw_byte_enables[ 2] && (rcw_compare_data[ 23: 16] !== rcw_old_data[ 23: 16])) ||
          (rcw_byte_enables[ 3] && (rcw_compare_data[ 31: 24] !== rcw_old_data[ 31: 24]))) rcw_data_matches = 1'h0;
    end
  end


  assign #1 do_read_ud          = do_read;
  assign #1 reading_siomem_ud   = reading_siomem;
  assign PIRespData             = do_read_ud ?  siomem_read_data : 32'h0;

  assign rdata = mem[siomem_index];

  assign #1 siomem_read_data = {
                         (siomem_byte_enables[ 3] ? rdata[ 31: 24] : 8'h0),
                         (siomem_byte_enables[ 2] ? rdata[ 23: 16] : 8'h0),
                         (siomem_byte_enables[ 1] ? rdata[ 15:  8] : 8'h0),
                         (siomem_byte_enables[ 0] ? rdata[  7:  0] : 8'h0)};

  always @(posedge CLK) begin
    if (siomem_write) begin
      wdata = mem[siomem_index];
      if (siomem_byte_enables[ 0]) wdata[  7:  0] = siomem_write_data[  7:  0];
      if (siomem_byte_enables[ 1]) wdata[ 15:  8] = siomem_write_data[ 15:  8];
      if (siomem_byte_enables[ 2]) wdata[ 23: 16] = siomem_write_data[ 23: 16];
      if (siomem_byte_enables[ 3]) wdata[ 31: 24] = siomem_write_data[ 31: 24];
      mem[siomem_index] = wdata;
    end
  end

`ifdef MODEL_TECH
  export "DPI-SC" function peek_peregrine_siomem;
`else
  export "DPI-C" function peek_peregrine_siomem;
`endif
  function int peek_peregrine_siomem;
    input int unsigned address;
    inout int unsigned byte_lanes_3210;
    reg [31:0] peek_data;
    int unsigned index;
    int unsigned word;
    word = 0;
    peek_peregrine_siomem = 1;
    if (!(address & 2'h3) && ((address >= siomem_start) && (address <= siomem_end))) begin
      peek_peregrine_siomem = 0;
      index = (address - siomem_start) >> 2;
      peek_data = mem[index];
      byte_lanes_3210 = (peek_data >> (word * 32)) & 32'hFFFFFFFF;
    end
  endfunction


`ifdef MODEL_TECH
  export "DPI-SC" function poke_peregrine_siomem;
`else
  export "DPI-C" function poke_peregrine_siomem;
`endif
  function int poke_peregrine_siomem;
    input int unsigned address;
    input int unsigned byte_lanes_3210;
    reg [31:0] poke_data;
    int unsigned index;
    int unsigned word;
    word = 0;
    poke_peregrine_siomem = 1;
    if (!(address & 2'h3) && ((address >= siomem_start) && (address <= siomem_end))) begin
      index = (address - siomem_start) >> 2;
      poke_data = mem[index];
      poke_data = byte_lanes_3210;
      poke_peregrine_siomem = 0;
      mem[index] = poke_data;
    end
  endfunction


endmodule




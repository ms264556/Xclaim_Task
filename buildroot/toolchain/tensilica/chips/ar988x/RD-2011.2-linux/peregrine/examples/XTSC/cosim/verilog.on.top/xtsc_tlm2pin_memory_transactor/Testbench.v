// Customer ID=8327; Build=0x3b95c; Copyright (c) 2005-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.


module Testbench;   // {

  wire                  CLK;

  wire                  POReqValid;
  wire                  PIReqRdy;
  wire          [  7:0] POReqCntl;
  wire          [ 31:0] POReqAdrs;
  wire          [ 31:0] POReqData;
  wire          [  3:0] POReqDataBE;
  wire          [  5:0] POReqId;
  wire          [  1:0] POReqPriority;

  wire                  PIRespValid;
  wire                  PORespRdy;
  wire          [  7:0] PIRespCntl;
  wire          [ 31:0] PIRespData;
  wire          [  5:0] PIRespId;
  wire          [  1:0] PIRespPriority;

  wire          [ 16:0] IRam0Addr;
  wire                  IRam0En;
  wire                  IRam0Wr;
  wire          [ 31:0] IRam0WrData;
  wire                  IRam0LoadStore;
  wire          [ 31:0] IRam0Data;

  wire          [ 16:0] IRam1Addr;
  wire                  IRam1En;
  wire                  IRam1Wr;
  wire          [ 31:0] IRam1WrData;
  wire                  IRam1LoadStore;
  wire          [ 31:0] IRam1Data;

  wire          [ 15:0] DRam0Addr0;
  wire                  DRam0En0;
  wire                  DRam0Wr0;
  wire          [  3:0] DRam0ByteEn0;
  wire          [ 31:0] DRam0WrData0;
  wire          [ 31:0] DRam0Data0;

  wire          [ 15:0] DRam1Addr0;
  wire                  DRam1En0;
  wire                  DRam1Wr0;
  wire          [  3:0] DRam1ByteEn0;
  wire          [ 31:0] DRam1WrData0;
  wire          [ 31:0] DRam1Data0;



  Xtensa0 core0(

    .POReqValid         (POReqValid),
    .PIReqRdy           (PIReqRdy),
    .POReqCntl          (POReqCntl),
    .POReqAdrs          (POReqAdrs),
    .POReqData          (POReqData),
    .POReqDataBE        (POReqDataBE),
    .POReqId            (POReqId),
    .POReqPriority      (POReqPriority),

    .PIRespValid        (PIRespValid),
    .PORespRdy          (PORespRdy),
    .PIRespCntl         (PIRespCntl),
    .PIRespData         (PIRespData),
    .PIRespId           (PIRespId),
    .PIRespPriority     (PIRespPriority),

    .IRam0Addr          (IRam0Addr),
    .IRam0En            (IRam0En),
    .IRam0Wr            (IRam0Wr),
    .IRam0WrData        (IRam0WrData),
    .IRam0LoadStore     (IRam0LoadStore),
    .IRam0Data          (IRam0Data),

    .IRam1Addr          (IRam1Addr),
    .IRam1En            (IRam1En),
    .IRam1Wr            (IRam1Wr),
    .IRam1WrData        (IRam1WrData),
    .IRam1LoadStore     (IRam1LoadStore),
    .IRam1Data          (IRam1Data),

    .DRam0Addr0         (DRam0Addr0),
    .DRam0En0           (DRam0En0),
    .DRam0Wr0           (DRam0Wr0),
    .DRam0ByteEn0       (DRam0ByteEn0),
    .DRam0WrData0       (DRam0WrData0),
    .DRam0Data0         (DRam0Data0),

    .DRam1Addr0         (DRam1Addr0),
    .DRam1En0           (DRam1En0),
    .DRam1Wr0           (DRam1Wr0),
    .DRam1ByteEn0       (DRam1ByteEn0),
    .DRam1WrData0       (DRam1WrData0),
    .DRam1Data0         (DRam1Data0),

    .CLK                (CLK)
  );



  peregrine_pifmem pif(
    .POReqValid         (POReqValid),
    .PIReqRdy           (PIReqRdy),
    .POReqCntl          (POReqCntl),
    .POReqAdrs          (POReqAdrs),
    .POReqData          (POReqData),
    .POReqDataBE        (POReqDataBE),
    .POReqId            (POReqId),
    .POReqPriority      (POReqPriority),
    .PIRespValid        (PIRespValid),
    .PORespRdy          (PORespRdy),
    .PIRespCntl         (PIRespCntl),
    .PIRespData         (PIRespData),
    .PIRespId           (PIRespId),
    .PIRespPriority     (PIRespPriority),
    .CLK                (CLK)
  );



  peregrine_iram0 iram0(
    .IRam0Addr          (IRam0Addr),
    .IRam0En            (IRam0En),
    .IRam0Wr            (IRam0Wr),
    .IRam0WrData        (IRam0WrData),
    .IRam0LoadStore     (IRam0LoadStore),
    .IRam0Data          (IRam0Data),
    .CLK                (CLK)
  );



  peregrine_iram1 iram1(
    .IRam1Addr          (IRam1Addr),
    .IRam1En            (IRam1En),
    .IRam1Wr            (IRam1Wr),
    .IRam1WrData        (IRam1WrData),
    .IRam1LoadStore     (IRam1LoadStore),
    .IRam1Data          (IRam1Data),
    .CLK                (CLK)
  );



  peregrine_dram0 dram0(
    .DRam0Addr0         (DRam0Addr0),
    .DRam0En0           (DRam0En0),
    .DRam0Wr0           (DRam0Wr0),
    .DRam0ByteEn0       (DRam0ByteEn0),
    .DRam0WrData0       (DRam0WrData0),
    .DRam0Data0         (DRam0Data0),
    .CLK                (CLK)
  );



  peregrine_dram1 dram1(
    .DRam1Addr0         (DRam1Addr0),
    .DRam1En0           (DRam1En0),
    .DRam1Wr0           (DRam1Wr0),
    .DRam1ByteEn0       (DRam1ByteEn0),
    .DRam1WrData0       (DRam1WrData0),
    .DRam1Data0         (DRam1Data0),
    .CLK                (CLK)
  );


endmodule   // }



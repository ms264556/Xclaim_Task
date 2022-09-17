;# Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
;# These coded instructions, statements, and computer programs are the
;# copyrighted works and confidential proprietary information of Tensilica Inc.
;# They may not be modified, copied, reproduced, distributed, or disclosed to
;# third parties in any manner, medium, or form, in whole or in part, without
;# the prior written consent of Tensilica Inc.
;# use strict;
;#
;# This script generates the buffer logic for input and output TIE
;# interfaces.
;#
;# The control parameters get passed using the -e option of tietpp.
;# The list of input parameters is described below:
;#
;#######################################################################
;#                          --- Inputs ---
;#######################################################################
;#
;my $Ten4Internal = (exists $ENV{'TEN4_INTERNAL'}) ? 1 : 0;
;if($type eq 'control') {
;   $depth = $latency + 1;
;} elsif($direction eq 'input') {
;   $depth = min($depth, $commit-$use_stage);
;} else {
;   $depth = ($def_stage < $commit) ? 1 : 2; # add 1 for staging-reg
;}
;my $ptr_width = ceillog2($depth);
// Name              : "`$name`"
// Prefix            : "`$prefix`"
// Type              : `$type`
// Direction         : `$direction`
// Width             : `$width`
// Depth             : `$depth`
;if ($direction eq 'input') {
// Use stage         : `$use_stage`
;} else {
// Def stage         : `$def_stage`
;}
;if ($type eq 'control' && $direction eq 'output') {
// Latency           : `$latency`
// Stall             : `$stall`
;}
// Commit            : `$commit`
// Reg               : `$register`
// Global Clk Gating : `$ClkGateGlobal`
// Func Clk Gating   : `$ClkGateFunc`
// Reset Flop        : `$ResetFlop`
// Async Reset Flop  : `$AsyncResetFlop`
;######################################################################
;#			  enabled flop fun
;######################################################################
;my $enflop  = $ClkGateGlobal ? '' : 'en';
;my $enport  = $ClkGateGlobal ? '' : '!GlobalStall, ';
;my $enport2 = $ClkGateGlobal ? '' : ' && !GlobalStall';
;my $async = $AsyncResetFlop ? 'a' : '';

;if($type eq 'control') {
;   ######################################################################
;   #                          control interfaces
;   #                            (a.k.a. Lookup)
;   #
;   # A lookup interface is made up of a control output interface and
;   # control input interface pair.  The output interface sends out an
;   # address to the external device, and generates a datavalid signal
;   # to the input interface a fixed number of cycles later.
;   #
;   #                +-------------------+
;   #    def_C0 ---->|                   |------> <name>_Out[]
;   #    datain_Cx ->|    output i/f     |------> <name>_Req
;   #                |                   |<------ <name>_Rdy
;   #                +--------+----------+
;   #                         | datavalid
;   #                         v
;   #                +-------------------+
;   #    use_C0 ---->|                   |<------ <name>_In[]
;   #    out_Cx <----|     input i/f     |
;   #                |                   |
;   #                +-------------------+
;   #
;   ######################################################################
;   if($direction eq 'output') {
;      my $spec = $def_stage < $commit && ($def_stage + $latency) > $commit;
;      my $simplespec = ($def_stage + $latency) <= $commit;
;      my $nonspec = $def_stage >= $commit;
;      ######################################################################
;      #                simple output buffer for lookup interface
;      ######################################################################
;      #
;      # Provides state machine for control output interface.  Must retry
;      # request until interface is ready.  Then provide datavalid signal to
;      # input control interface.
;      #
module `$prefix`TIE_outbuf_simple_`$name` (
	clk,
        fclk,
	GlobalStall,
	reset,
	Kill_E,
	killPipe_W,
	interrupt,
; if ( $Kill ) {
  	kill_C`$def_stage`,
;}
;      if ( $TMode ) {
	TMode,
;      }
;      if($trace) {
        traceout,
        traceoutvalid,
;      }
	datain_C`$def_stage`,
	dataout,
	def_C0,
        valid,
;      if($stall) {
        stall_in,
        stall_out,
;      }
        dataspec,
        datavalid,
        idle
        );
    parameter width=`$width`; // vhdl generic

    input clk;
    input fclk;
    input GlobalStall;
    input reset;
    input Kill_E;
    input killPipe_W;
    input interrupt;
; if ( $Kill ) {
    input kill_C`$def_stage`;
; }
;      if ( $TMode ) {
    input TMode;
;      }
;      if($trace) {
    output [width-1:0] traceout;
    output traceoutvalid;
;      }
    input [width-1:0] datain_C`$def_stage`;
    output [width-1:0] dataout;
    input def_C0;
    output valid;
;      if($stall) {
    input stall_in;
    output stall_out;
;      }
    output dataspec;
    output datavalid;
    output idle;

;      my $g2sclk = $ClkGateFunc ? "g2sclk" : "clk";
;      my $g2fclk = $ClkGateFunc ? "g2fclk" : "fclk";
;      if($ClkGateFunc) {
    /**********************************************************************
     *                      -- Clock Gating --
     *
     * Enable clock when buffers not empty or use in pipeline.
     **********************************************************************/

    wire g2sclk;  // stall clock
    wire g2fclk;  // free running clock (gated with waiti)
    wire en_clocks;
;         my @en = ('en_clocks', 'reset');
;         my $clkgate = ($TMode)?"xtgated_tmode_clock":"xtgated_clock";
;         my $tmode = ($TMode)?"TMode,":"";
    `$clkgate` g2sclk_gate (g2sclk,`$tmode` ` join(' || ', @en)`, clk);
    `$clkgate` g2fclk_gate (g2fclk,`$tmode` ` join(' || ', @en)`, fclk);
;      }

    /**********************************************************************
     *                      -- DEF Control Pipeline --
     **********************************************************************/

;      my $commit_or_def = $commit > $def_stage ? $commit : $def_stage;
;      foreach my $c (0..$commit_or_def) {
;         my $def = "def_C${c}";
;         $def .= " && !killPipe_W" if($c <= $commit);
;         $def .= " && !Kill_E" if ($c == 1);
;         $def .= " && !kill_C${def_stage}" if ($Kill && $c == $def_stage);
    wire odef_C`$c` = `$def`;
;         unless($c == $commit_or_def) {
    wire def_C`$c + 1`;
    xt`$async`sc`$enflop`flop #(1) def`$c`to`$c+1` (def_C`$c+1`, odef_C`$c`, `$enport`!reset, `$g2sclk`);
;         }
;      }

    /**********************************************************************
     *                 -- Request Buffer Control --
     **********************************************************************/

;      if($stall) {
    wire ready = !stall_in;
;      } else {
    wire ready = 1'b1;
;      }
    wire push = odef_C`$def_stage`;
    wire pop = valid && ready;
    wire flush = `$def_stage < $commit ? 'killPipe_W' : "1'b0"`;
    wire bypass;
    wire en_buffer;

;      if($stall) {
    /**********************************************************************
     *                     -- Request Buffer --
     **********************************************************************/

    wire [width-1:0] out_buffer;
    xt`$async`scenflop #(width) bufferreg (out_buffer, datain_C`$def_stage`, en_buffer, !reset, `$g2fclk`);

    /**********************************************************************
     *                    -- Request Buffer Bypass --
     **********************************************************************/

    assign dataout = bypass ? datain_C`$def_stage` : out_buffer;
;      } else {
    /**********************************************************************
     *                    -- Request Data --
     **********************************************************************/

    assign dataout = datain_C`$def_stage`;
;      }

    /**********************************************************************
     *                    -- Request Buffer FSM --
     **********************************************************************/

;   my $n = uc($name);
    parameter	`$n`_BYPASS = 2'b00;
    parameter	`$n`_WAIT   = 2'b01;
    parameter	`$n`_RETRY1 = 2'b10;
    parameter	`$n`_RETRY2 = 2'b11;

    wire [1:0] cs;
    reg [1:0] ns_pop;
    reg [1:0] ns_nopop;
    reg [1:0] ns;
;      my @sens = qw/cs GlobalStall pop/;
;      push @sens, qw/push interrupt flush/ if($stall);
    always @(` join ' or ', @sens`) begin
       case(cs)
          `$n`_BYPASS:
             begin
;      if($stall) {
                ns_pop   = GlobalStall        ? `$n`_WAIT   : `$n`_BYPASS;
                ns_nopop = interrupt          ? (GlobalStall ? `$n`_WAIT : `$n`_BYPASS) :
                           !push              ? `$n`_BYPASS :
                           GlobalStall        ? `$n`_RETRY1 : `$n`_RETRY2;
;      } else {
                ns_pop   = GlobalStall ? `$n`_WAIT : `$n`_BYPASS;
                ns_nopop = GlobalStall ? `$n`_WAIT : `$n`_BYPASS;
;      }
             end
          
          // Request has been accepted, but stalled due to 
          // some other cause.  Wait for stall to go away      
          `$n`_WAIT:
             begin
                ns_pop   = GlobalStall ? `$n`_WAIT : `$n`_BYPASS; 
                ns_nopop = GlobalStall ? `$n`_WAIT : `$n`_BYPASS;
             end             

;      if($stall) {
          // Retry request, but also stalled in def stage
          // for some other cause.  There will be a stall
          // this cycle unless there is a flush.
          `$n`_RETRY1:
             begin
                ns_pop   = GlobalStall        ? `$n`_WAIT : `$n`_BYPASS; 
                ns_nopop = flush || interrupt ? (GlobalStall ? `$n`_WAIT   : `$n`_BYPASS)
                                              : (GlobalStall ? `$n`_RETRY1 : `$n`_RETRY2);
             end             

          // Retry request, pipeline has advanced past the
          // def stage.
          `$n`_RETRY2:
             begin
                ns_pop   = interrupt && GlobalStall ? `$n`_WAIT   : `$n`_BYPASS;
                // interface is driving stall in RETRY2
		   ns_nopop = !flush && !interrupt     ? `$n`_RETRY2 : 
                           		GlobalStall    ? `$n`_WAIT   : `$n`_BYPASS;
             end             

;      }
`ifdef NOXPROP
          default:
             begin
                ns_pop = 2'b0;
                ns_nopop = 2'b0;
             end
`else
          default:
             begin
                ns_pop = 2'bxx;
                ns_nopop = 2'bxx;
             end
`endif
       endcase
       ns = pop ? ns_pop : ns_nopop;
    end
    xt`$async`scflop #(2) state (cs, ns, !reset, `$g2fclk`);

    wire state_`$n`_BYPASS = cs == `$n`_BYPASS;
    wire state_`$n`_WAIT   = cs == `$n`_WAIT;
    wire state_`$n`_RETRY1 = cs == `$n`_RETRY1;
    wire state_`$n`_RETRY2 = cs == `$n`_RETRY2;

;      my @valid = ("state_${n}_BYPASS && push");
;      push @valid, ("state_${n}_RETRY1 && !flush", "state_${n}_RETRY2 && !flush") if($stall);
    assign valid = ` join " || ", @valid`;
    assign bypass       = state_`$n`_BYPASS;
    assign en_buffer    = state_`$n`_BYPASS && push; 

    /**********************************************************************
     *                   -- Speculative Control --
     **********************************************************************/

;      if($simplespec) {
    assign dataspec = 1'b1;
;      } elsif($nonspec) {
    assign dataspec = 1'b0;
;      } elsif($spec) {
    // Data is speculatively pushed to the input buffer, from the external
    // agent if the request is issued before the commit stage, and there is
    // a globalstall causing the data to arrive before the commit stage, and
    // the data is normally consumed after the commit stage.
    
;         my $bits = ceillog2($def_stage + $latency + 1);
    wire [`$bits-1`:0] spec_stage0 = state_`$n`_RETRY2 ? `$bits`'d`$def_stage+1` : `$bits`'d`$def_stage`;
    wire stage0_is_killable = 1'b1;

;         foreach (1..$latency) {
    wire [`$bits-1`:0] spec_stage`$_-1`_nxt = GlobalStall ? spec_stage`$_-1` : spec_stage`$_-1` + `$bits`'d1;
    wire [`$bits-1`:0] spec_stage`$_`;
    xt`$async`scflop #(`$bits`) spec_stage`$_`_reg (spec_stage`$_`, spec_stage`$_-1`_nxt, !reset, `$g2fclk`);
    wire stage`$_`_is_killable = spec_stage`$_` <= `$bits`'d`$commit`;

;         }
    assign dataspec = spec_stage`$latency` < `$bits`'d`$commit`;
;      } else {
    assign dataspec = 1'b0;
;      }

    /**********************************************************************
     *                      -- DataValid --
     **********************************************************************/

    wire datavalid0 = ready && valid;
;      foreach my $l (1..$latency) {
    wire datavalid`$l`;
;         my $kill = $nonspec ? '' : $spec ? ' && !(stage'.($l-1).'_is_killable && killPipe_W)' : ' && !killPipe_W';
    xt`$async`scflop #(1) datavalid`$l`reg (datavalid`$l`, datavalid`$l-1``$kill`, !reset, `$g2fclk`);
;      }
;      my $kill = $nonspec ? '' : $spec ? ' && !(stage'.$latency.'_is_killable && killPipe_W)' : ' && !killPipe_W';
    assign datavalid = datavalid`$latency . $kill`;

;      if($stall) {
    /**********************************************************************
     *                      -- Stall --
     **********************************************************************/

    wire maybestall;
    wire maybestall_nxt = !interrupt && (
       state_`$n`_BYPASS && push && !GlobalStall && !ready`$def_stage <= $commit ? ' && !killPipe_W' : ''`
       || state_`$n`_RETRY1 && !ready && !GlobalStall`$def_stage <= $commit ? ' && !killPipe_W' : ''`
       || state_`$n`_RETRY2 && !ready`($def_stage + 1) <= $commit ? ' && !killPipe_W' : ''`);
    xt`$async`scflop #(1) stallreg (maybestall, maybestall_nxt, !reset, `$g2fclk`);
    assign stall_out = maybestall && !flush;    

;      }
    /**********************************************************************
     *                      -- Idle --
     **********************************************************************/

      wire active =  ` join ' || ', map("odef_C$_", 0..$def_stage), map("datavalid$_", 0..$latency-1)` ;
      assign idle = !active;

;      if($trace) {
    /**********************************************************************
     *                       -- Trace --
     **********************************************************************/

    assign traceoutvalid = odef_C`$commit`;
;         if($def_stage == $commit && !$stall) {
    assign traceout = dataout;
;         } elsif($def_stage < $commit) {
    wire [width-1:0] trace_C`$def_stage` = datain_C`$def_stage`;
;            foreach (($def_stage+1)..$commit) {
    wire [width-1:0] trace_C`$_`;
    xt`$async`sc`$enflop`flop #(width) trace_C`$_`_reg (trace_C`$_`, trace_C`$_-1`, `$enport`!reset, `$g2sclk`);
;            }
    assign traceout = trace_C`$commit`;
;         } else {
    assign traceout = {width{1'b0}};
;         }    

;      }
    /**********************************************************************
     *                      -- Clock Enable --
     **********************************************************************/
;      if($ClkGateFunc) {
;         my $clk_stage = ($stall)?($def_stage+1):$def_stage; 
;         my @en = (map("def_C$_", 0..$clk_stage), map("datavalid$_", 0..$latency));
;         push @en, map("def_C$_", 0..$commit) if($trace);
    assign en_clocks =  ` join ' || ', @en`;
;      }

endmodule
;   } elsif($direction eq 'input') {
;      my $spec = ($use_stage - $latency) < $commit && $use_stage > $commit;
;      my $simplespec = $use_stage <= $commit;
;      my $nonspec = ($use_stage - $latency) >= $commit;
;      ######################################################################
;      #			simple buffer for lookup interface
;      ######################################################################
;      #
;      # Provides buffering for lookup interfaces with depth > 0.  Buffering
;      # is required to correctly pipeline lookups across stalls.  
;      #
;      # Example 1: Stall every other cycle, depth 1
;      # Lookup      i i r r e e m m w w
;      # Lookup          i i r r e e m m w w     
;      # Address               a   b
;      # Data                    d   e
;      # defC1_C1              1   1
;      # useC2_C2                1 1 1 1
;      # out_C2                  d d e e
;      # push                    1   1
;      # pop                       1   1
;      # 
;      # Example 2: Stall every other cycle, depth 2
;      # Lookup      i i r r e e m m w w
;      # Lookup          i i r r e e m m w w
;      # Address               a   b
;      # Data                      d   e
;      # defC1_C1              1   1
;      # useC3_C3                    1 1 1 1
;      # out_C2                      d d e e
;      # 
;      # push = (def & !GlobalStall) + depth
;      # pop  = use & !GlobalStall
;      #
module `$prefix`TIE_inbuf_simple_`$name` (
	clk,
        fclk,
	GlobalStall,
	reset,
	Kill_E,
	killPipe_W,
;      if ( $TMode ) {
	TMode,
;      }
;      if($trace) {
        traceout,
        traceoutvalid,
;      }
	datain,
        dataspec,
        datavalid,
	out_C`$use_stage`,
	use_C0);
    parameter width=`$width`; // vhdl generic

    input clk;
    input fclk;
    input GlobalStall;
    input reset;
    input Kill_E;
    input killPipe_W;
;      if ( $TMode ) {
    input TMode;
;      }
;      if($trace) {
    output [width-1:0] traceout;
    output traceoutvalid;
;      }
    input [width-1:0] datain;
    input dataspec;
    input datavalid;
    output [width-1:0] out_C`$use_stage`;
    input use_C0;

;      my $g2sclk = $ClkGateFunc ? "g2sclk" : "clk";
;      my $g2fclk = $ClkGateFunc ? "g2fclk" : "fclk";
;      if($ClkGateFunc) {
    /**********************************************************************
     *                      -- Clock Gating --
     *
     * Enable clock when buffers not empty or use in pipeline.
     **********************************************************************/

    wire g2sclk;  // stall clock
    wire g2fclk;  // free running clock (gated with waiti)
    wire en_clocks;
;         my @en = ('en_clocks', 'reset');
;         my $clkgate = ($TMode)?"xtgated_tmode_clock":"xtgated_clock";
;         my $tmode = ($TMode)?"TMode,":"";
    `$clkgate` g2sclk_gate (g2sclk,`$tmode` ` join(' || ', @en)`, clk);
    `$clkgate` g2fclk_gate (g2fclk,`$tmode` ` join(' || ', @en)`, fclk);
;      }
    wire globalstall_last;
    xt`$async`scflop #(1) globalstall_reg (globalstall_last, GlobalStall, !reset, fclk);

    /**********************************************************************
     *                      -- USE Control Pipeline --
     **********************************************************************/
 
;      my $use_or_commit = !$trace ? $use_stage : $commit > $use_stage ? $commit : $use_stage;
;      foreach my $c (0..$use_or_commit) {
;         my $use = "use_C$c";
;         $use .= " && !killPipe_W" if($c <= $commit ) ;
;         $use .= " && !Kill_E" if ($c == 1);
    wire iuse_C`$c` = `$use`;
;         unless($c == $use_or_commit) {
    wire use_C`$c+1`;
    xt`$async`sc`$enflop`flop #(1) use`$c`to`$c+1` (use_C`$c+1`, iuse_C`$c`, `$enport`!reset, `$g2sclk`);
;         }
;      }

;      if($ClkGateFunc) {
    assign en_clocks = 1'b1 || ` join ' || ', map("iuse_C$_", 0..$use_stage)`;
;      }

    /**********************************************************************
     *                      -- Buffer Control --
     **********************************************************************/

    // Spec        `$spec`
    // Simple spec `$simplespec`
    // Non spec    `$nonspec`

;      if($spec) {
    wire spush = datavalid;
    wire buffer_is_spec;
    wire push = datavalid && !dataspec || buffer_is_spec && iuse_C`$commit` && !globalstall_last; 
    wire scancel = killPipe_W;
;      } else {
    wire push = datavalid;
;      }
    wire pop = iuse_C`$use_stage` && !GlobalStall;
    wire flush = killPipe_W;

    /**********************************************************************
     *                         -- Write Pointer --
     **********************************************************************/

    wire [`$ptr_width-1`:0] wr_ptr;
    wire [`$ptr_width-1`:0] wr_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_wr_ptr;

;      if($spec) {
    /**********************************************************************
     *                   -- Speculative Write Pointer --
     **********************************************************************/

    wire [`$ptr_width-1`:0] swr_ptr;
    wire [`$ptr_width-1`:0] swr_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_swr_ptr;
    assign buffer_is_spec = wr_ptr != swr_ptr;
;         my $is_pow_2 = $depth == 2**(ceillog2($depth));
;         if($is_pow_2) {
    assign swr_ptr_inc = swr_ptr + `$ptr_width`'h1;
;         } else {
    wire [`$ptr_width-1`:0] swr_ptr_inc_nq = swr_ptr + `$ptr_width`'h1;
    assign swr_ptr_inc = (swr_ptr_inc_nq == `$ptr_width`'d`$depth`) ? `$ptr_width`'h0 : swr_ptr_inc_nq;
;         }
    xt`$async`scenflop #(`$ptr_width`) swr_ptr_reg (swr_ptr, nxt_swr_ptr, spush || (scancel && buffer_is_spec), !reset, `$g2fclk`);

;      }
;      my $is_pow_2 = $depth == 2**(ceillog2($depth));
;      if($is_pow_2) {
    assign wr_ptr_inc = wr_ptr + `$ptr_width`'h1;
;      } else {
    wire [`$ptr_width-1`:0] wr_ptr_inc_nq = wr_ptr + `$ptr_width`'h1;
    assign wr_ptr_inc = (wr_ptr_inc_nq == `$ptr_width`'d`$depth`) ? `$ptr_width`'h0 : wr_ptr_inc_nq;
;      }
;      if($spec) {
    assign nxt_wr_ptr = wr_ptr_inc;
    assign nxt_swr_ptr = (scancel && buffer_is_spec) ? (push ? nxt_wr_ptr : wr_ptr) : swr_ptr_inc;
;      } elsif($use_stage > $commit) {
;         # def must be >= commit, or else would have been spec config
;         # all writes are committed
    assign nxt_wr_ptr = wr_ptr_inc;
;      } else {
;         # use is <= commit, so can use simple buffer control
    assign nxt_wr_ptr = flush ? `$ptr_width`'b0 : wr_ptr_inc;
;      }
;      my $en = $spec || $nonspec ? 'push' : 'push || flush';
    xt`$async`scenflop #(`$ptr_width`) wr_ptr_reg (wr_ptr, nxt_wr_ptr, `$en`, !reset, `$g2fclk`);

    /**********************************************************************
     *                          -- Read Pointer --
     **********************************************************************/

    wire [`$ptr_width-1`:0] rd_ptr;
    wire [`$ptr_width-1`:0] rd_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_rd_ptr;
    assign rd_ptr_inc = rd_ptr + `$ptr_width`'h1;
;      if($spec || $nonspec) {
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_rd_ptr = rd_ptr_inc;
;         } else {
    assign nxt_rd_ptr = (rd_ptr_inc == `$ptr_width`'d`$depth`)?`$ptr_width`'h0:rd_ptr_inc;
;         }
;      } else {
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_rd_ptr = flush ? `$ptr_width`'b0 : rd_ptr_inc;
;         } else {
    assign nxt_rd_ptr = flush ? `$ptr_width`'b0 :
                        (rd_ptr_inc == `$ptr_width`'d`$depth`)?`$ptr_width`'h0:rd_ptr_inc;
;         }
;      }
;      my $rden = $spec || $nonspec ? 'pop' : 'pop || flush';
    xt`$async`scenflop #(`$ptr_width`) rd_ptr_reg (rd_ptr, nxt_rd_ptr, `$rden`, !reset, `$g2sclk`);

    /**********************************************************************
     *                         -- Entry Counter --
     **********************************************************************/

    wire [`$ptr_width`:0] entries;
    wire [`$ptr_width`:0] entries_inc;
    wire [`$ptr_width`:0] pre_entries;
    wire [`$ptr_width`:0] nxt_entries;
    xtmux4e #(`$ptr_width+1`) entriesinc_mux(entries_inc,
                                `$ptr_width+1`'h0 /* 0 */,
                                `$ptr_width+1`'h1 /* 1 */,
                                `$ptr_width+1`'h` sprintf("%lx",(2**($ptr_width+1)-1))` /* 2 */,
                                `$ptr_width+1`'h0 /* 3 */,
                                {pop,push});
    xtadd #(`$ptr_width+1`) entries_add (pre_entries, entries, entries_inc);
;      if($spec || $nonspec) {
    assign nxt_entries = pre_entries;
;      } else {
    assign nxt_entries = flush ? `$ptr_width+1`'b0 : pre_entries;
;      }
;      my @entry_en = qw/pop push/;
;      push @entry_en, "flush" if($simplespec);
    xt`$async`scenflop #(`$ptr_width+1`) entries_reg (entries, nxt_entries, ` join " || ", @entry_en`,
    		!reset, `$g2fclk`);

;      if($spec) {
    /**********************************************************************
     *                   -- Speculative Entry Counter --
     **********************************************************************/

    wire [`$ptr_width`:0] sentries;
    wire [`$ptr_width`:0] sentries_inc;
    wire [`$ptr_width`:0] pre_sentries;
    wire [`$ptr_width`:0] nxt_sentries;
    xtmux4e #(`$ptr_width+1`) sentriesinc_mux(sentries_inc,
                                `$ptr_width+1`'h0 /* 0 */,
                                `$ptr_width+1`'h1 /* 1 */,
                                `$ptr_width+1`'h` sprintf("%lx",(2**($ptr_width+1)-1))` /* 2 */,
                                `$ptr_width+1`'h0 /* 3 */,
                                {pop,spush});
    xtadd #(`$ptr_width+1`) sentries_add (pre_sentries, sentries, sentries_inc);
    assign nxt_sentries = scancel ? nxt_entries : pre_sentries;
    xt`$async`scenflop #(`$ptr_width+1`) sentries_reg (sentries, nxt_sentries, pop || spush || scancel,
    		!reset, `$g2fclk`);

;      }
    /**********************************************************************
     *                           -- Empty/Full --
     **********************************************************************/

;      if($spec) {
    wire empty = sentries == `$ptr_width+1`'h0;
; if ( $Ten4Internal ) {
// synopsys translate_off
     wire internalBufFull = sentries == `$ptr_width+1`'h` sprintf("%lx",$depth)`;
// synopsys translate_on
; }
;      } else {
    wire empty = entries == `$ptr_width+1`'h0;
; if ( $Ten4Internal ) {
// synopsys translate_off
     wire internalBufFull = entries == `$ptr_width+1`'h` sprintf("%lx",$depth)`;
// synopsys translate_on
; }
;      }
    
    /**********************************************************************
     *                          -- Data Bypass --
     **********************************************************************/

    wire [width-1:0] rd_data;
    xtmux2p #(width) xtout_mux(out_C`$use_stage`,
                            datain /* 0 */,
                            rd_data /* 1 */,
                            empty /* sel */);

    /**********************************************************************
     *                            -- Regfile --
     **********************************************************************/

;      my $wr_ptr = $spec ? 'swr_ptr' : 'wr_ptr';
;      my $push = $spec ? 'spush' : 'push';
;      if($depth == 1) {
;         if ( $ResetFlop == 1 ) {
    xt`$async`scenflop #(width) icore0(rd_data, datain, `$push`, !reset, fclk);
;         } else {
    xtenflop #(width) icore0(rd_data, datain, `$push`, fclk);
;         }
;      } elsif ( $TMode ) {
    xtregfile_1R1W_`$depth`_`$width`_FF_XT`$prefix` icore0(rd_data, rd_ptr, datain, `$wr_ptr`, `$push`, TMode, reset, fclk);
;      } else {
    xtregfile_1R1W_`$depth`_`$width`_FF_XT`$prefix` icore0(rd_data, rd_ptr, datain, `$wr_ptr`, `$push`, reset, fclk);
;      }

;      if($trace) {
    /**********************************************************************
     *                       -- Trace --
     **********************************************************************/

    assign traceoutvalid = iuse_C`$commit`;
;         if($use_stage == $commit) {
    assign traceout = out_C`$use_stage`;
;         } elsif($use_stage < $commit) {
    wire [width-1:0] trace_C`$use_stage` = out_C`$use_stage`;
;            foreach (($use_stage+1)..$commit) {
    wire [width-1:0] trace_C`$_`;
    xt`$async`sc`$enflop`flop #(width) trace_C`$_`_reg (trace_C`$_`, trace_C`$_-1`, `$enport`!reset, `$g2sclk`);
;            }
    assign traceout = trace_C`$commit`;
;         } else {
    assign traceout = {width{1'b0}};
;         }    

;      }

endmodule

;   } elsif($direction eq 'output') {
;      Carp::carp "Error: Bad wire type $type and direction $direction\n";
;   } else {
;      Carp::carp "Error: Bad wire type $type and direction $direction\n";
;   }
;} elsif(1 || $type eq 'data') {
;   if ($direction eq 'input') {
;      ######################################################################
;      #			    input interface
;      ######################################################################
;      #
;      # The buffering for input interfaces is a FIFO.  New entries get
;      # pushed onto the FIFO when an instruction reads the interface and
;      # there are not available entries in the FIFO.  An entry gets popped
;      # from the FIFO when the instruction commits.  No buffering is
;      # required if the interface is read (used) after the commit stage. 
;      #
;      # The buffer looks at the stall signal before driving the
;      # acknowledgement signal, which introduces a combinational delay in
;      # the logic.  This is different from how we handle output interfaces.
;      # I think avoiding the round-trip is better but they should probably
;      # be made the same.

;      my $read_stage = ($register)?($use_stage-1):$use_stage;
module `$prefix`TIE_inbuf_`$name` (
	clk,
        fclk,
	Kill_E,
	killPipe_W,
	interrupt,
	GlobalStall,
	GlobalStall_d,
	reset,
;      if ( $TMode ) {
	TMode,
;      }
;      if($trace) {
        traceout,
        traceoutvalid,
;      }
	datain,
	out_C`$use_stage`,
;      if ( $eKill ) {
	kill_C`$read_stage`,
; }
;      if ( $Kill ) {
	kill_C`$use_stage`,
;}
	use_C0,
 	enable,
	stall_in_C`$read_stage`,
	stall_out_C`$use_stage`,
	notrdy_C`$use_stage`);
    parameter width=`$width`; // vhdl generic

    input clk;
    input fclk;
    input Kill_E;
    input killPipe_W;
    input interrupt;
    input GlobalStall;
    input GlobalStall_d;
    input reset;
;      if ( $TMode ) {
    input TMode;
;      }
;      if($trace) {
    output [width-1:0] traceout;
    output traceoutvalid;
;      }
    input use_C0;
    input stall_in_C`$read_stage`;
    input [width-1:0] datain;
;      if ( $eKill ) {
    input kill_C`$read_stage`;
; }
;      if ( $Kill ) {
    input kill_C`$use_stage`;
; }
    output [width-1:0] out_C`$use_stage`;
    output enable;
    output stall_out_C`$use_stage`;
    output notrdy_C`$use_stage`;

    /**********************************************************************
     *                      -- USE Control Pipeline --
     **********************************************************************/
 
;      for (my $i=0; $i < $use_stage; $i++) {
;         my $n = $i+1;
;         my $use = "use_C$i && !killPipe_W";
;         $use .= " && !kill_C$read_stage" if ($eKill && $i == ($read_stage) ); 
;         $use .= " && !Kill_E" if ($i == 1); # fixme, use E-stage
    wire quse_C`$i` = `$use`;
    wire use_C`$n`;
    xt`$async`sc`$enflop`flop #(1) use`$n`(use_C`$n`, quse_C`$i`, `$enport`!reset, clk);
;      }
;      my $use = "use_C$use_stage && !killPipe_W";
;      $use .= " && !kill_C$use_stage" if ($Kill);
;      $use .= " && !Kill_E" if ($use_stage == 1);
    wire quse_C`$use_stage` = `$use`;
;      for (my $i=$use_stage; $i < $commit; $i++) {
;         my $n = $i+1;
;         my $use = "use_C$n && !killPipe_W";
    wire use_C`$n`;
    xt`$async`sc`$enflop`flop #(1) use`$n`(use_C`$n`, quse_C`$i`, `$enport`!reset, clk);
    wire quse_C`$n` = `$use`;
;      }
    wire empty;

    /**********************************************************************
     *                      -- Ready/Not-Ready --
     *
     * An input queue is NOT-READY if there is no data available to 
     * a queue instruction in the 'use-stage'.  
     * 
     * - If there is no queue-read instruction in the 'use-stage', 
     *   then the NOT-READY is a simple function of the external 
     *   queue status, and internal buffering status.
     *
     * - If there is a queue-read instruction in the 'use-stage',
     *   the NOT-READY should simply consider the status of the
     *   the internal buffers.
     **********************************************************************/

    // Internal buffer status
    wire stage_empty_C`$use_stage`;
    wire spec_buffer_empty_C`$use_stage`;

    parameter NOTREADY = 1'b1;
    parameter READY    = 1'b0;

    // check to see if data buffered already
    wire status_data_already_buffered_C`$use_stage` = !stage_empty_C`$use_stage` || !spec_buffer_empty_C`$use_stage`;

    // If no use, then just look at EMPTY and internal buffer status.
    // Clocked on stall clock.
    wire stall_in_C`$use_stage`;
    xt`$async`scflop #(1) stall_in_reg (stall_in_C`$use_stage`, stall_in_C`$read_stage`, !reset, clk);
    wire status_if_no_use_C`$use_stage` = stall_in_C`$use_stage` && !status_data_already_buffered_C`$use_stage` ? NOTREADY : READY;
    
    // If there is a use, just check to see if datas is already buffered
    wire status_if_use_C`$use_stage` = status_data_already_buffered_C`$use_stage` ? READY : NOTREADY;

    // Pick status based on whether there's a use
    wire status_C`$use_stage` = use_C`$use_stage` ? status_if_use_C`$use_stage` : status_if_no_use_C`$use_stage`;
    
    // Generate the NOTREADY interface
    assign notrdy_C`$use_stage` = status_C`$use_stage` == NOTREADY;

;      if($ClkGateFunc) {
    /**********************************************************************
     *                      -- Clock Gating --
     *
     * Enable clock when buffers not empty or use in pipeline.
     **********************************************************************/

    wire g2sclk;  // stall clock
    wire g2fclk;  // free running clock (gated with waiti)
    wire en_clocks;
;         my @en = ('en_clocks');
;         my $clkgate = ($TMode)?"xtgated_tmode_clock":"xtgated_clock";
;         my $tmode = ($TMode)?"TMode,":"";
    `$clkgate` g2sclk_gate (g2sclk,`$tmode` ` join(' || ', @en)`, clk);
    `$clkgate` g2fclk_gate (g2fclk,`$tmode` ` join(' || ', @en)`, fclk);
;      }

;      if ( $register ) {
    /**********************************************************************
     *                        -- Data Staging --
     * 
     * Input queue data is registered on the processor boundary before
     * being forwarded to an instruction semantic.
     **********************************************************************/

    wire [width-1:0] staged_data;

    wire full_C`$use_stage`;
    wire pop_staging_C`$use_stage`;
    wire retry_fetch_C`$use_stage`;
    wire fetch_new_data_C`$read_stage`;

    `$prefix``$name`_buf_staging #(width) staging(
;         if($ClkGateFunc) {
        g2fclk,
;         } else {
        fclk,
;         }
        reset,
	GlobalStall,
        fetch_new_data_C`$read_stage`,
        retry_fetch_C`$use_stage`,
        pop_staging_C`$use_stage`,
        enable,
	datain,
	staged_data,
	stall_in_C`$read_stage`,
	stage_empty_C`$use_stage`
    );
;      }
    /**********************************************************************
     *                    -- Speculative Buffering --
     *
     * Reads before the commit stage are speculative and must be stored
     * in the event that the read instruction does not commit.
     **********************************************************************/
    
    wire spec_buffer_almost_empty_C`$use_stage`;
    wire use_spec_buffer = quse_C`$use_stage`
                           && !(stage_empty_C`$use_stage` && spec_buffer_empty_C`$use_stage` && interrupt);

    `$prefix``$name`_buf_speculative #(width) spec_buffer(
;      if($ClkGateFunc) {
	g2sclk,
;      } else {
	clk,
;      }
	Kill_E,
	killPipe_W,
;      if($ClkGateGlobal) {
	1'b0,
;      } else {
	GlobalStall,
;      }
	reset,
;      if($TMode) {
	TMode,
;      }
;      if($trace) {
        traceout,
;      }
;      if ( $register ) {
	staged_data,
;      } else {
	datain,
;      }
	out_C`$use_stage`,
	use_spec_buffer,
	quse_C`$commit`,
;      if ( $register ) {
	stage_empty_C`$use_stage`,
	pop_staging_C`$use_stage`,
;      } else {
	stall_in_C`$use_stage`,
	fetch_new_data_`$use_stage`,
;      }
	spec_buffer_empty_C`$use_stage`,
	spec_buffer_almost_empty_C`$use_stage`,
	full_C`$use_stage`
    );

;      if ( $register ) {
    assign stall_out_C`$use_stage` = quse_C`$use_stage` && stage_empty_C`$use_stage` && spec_buffer_empty_C`$use_stage` && !interrupt ;
    assign fetch_new_data_C`$read_stage` = quse_C`$read_stage` 
                               && (spec_buffer_empty_C`$use_stage` || (spec_buffer_almost_empty_C`$use_stage` && quse_C`$use_stage`)); 
;         #                               // fixme what about when 1 entry in fifo?
    assign retry_fetch_C`$use_stage` = quse_C`$use_stage` && spec_buffer_empty_C`$use_stage`;
;      } else {
    assign enable = fetch_new_data_`$use_stage` && ~GlobalStall;
    assign stall_out_C`$use_stage` = quse_C`$use_stage` && stall_in_C`$use_stage` && spec_buffer_empty_C`$use_stage` ;
;      }
    assign empty = (stage_empty_C`$use_stage` && spec_buffer_empty_C`$use_stage`) || 
    		   (quse_C`$use_stage` && !stage_empty_C`$use_stage` && spec_buffer_empty_C`$use_stage`) || 
		   (quse_C`$use_stage` && stage_empty_C`$use_stage` && spec_buffer_almost_empty_C`$use_stage`);
    //assign idle = stage_empty_C`$use_stage` && spec_buffer_empty_C`$use_stage`
    //              && ` join ' && ', map("!quse_C$_", (1..$commit))`;

;      if($ClkGateFunc) {
    /**********************************************************************
     *                      -- Clock Gating --
     *
     * Enable clock when buffers not empty or use in pipeline.
     **********************************************************************/

    assign en_clocks = reset || !stage_empty_C`$use_stage` || !spec_buffer_empty_C`$use_stage`
                       || ` join ' || ', map("use_C$_", (0..$commit))`;

;      }
;      if($trace) {
    /**********************************************************************
     *                      -- Trace --
     **********************************************************************/

    assign traceoutvalid = quse_C`$commit`;
;      }
; if ( $Ten4Internal ) {
    /**********************************************************************
     *                      -- Coverage Monitor --
     **********************************************************************/
// synopsys translate_off
     wire internalBufFull = full_C`$use_stage` && !stage_empty_C`$use_stage`;
// synopsys translate_on
;}
endmodule
;      if ( $register ) {
;         # input queue
module `$prefix``$name`_buf_staging (
	fclk,
	reset,
        GlobalStall,
	use_C`$read_stage`,
	retry_C`$use_stage`,
	use_C`$use_stage`,
        enable_C`$read_stage`,
	datain,
	dataout_C`$use_stage`,
	empty_in,
	empty_out_C`$use_stage`
    );
    parameter width=`$width`; // vhdl generic

    input fclk;
    input reset;
    input GlobalStall;
    input use_C`$read_stage`; // Early use 
    input retry_C`$use_stage`;
    input use_C`$use_stage`; // Actual semantic use
    output enable_C`$read_stage`; // External pop
    input [width-1:0] datain;
    output [width-1:0] dataout_C`$use_stage`;
    input empty_in;
    output empty_out_C`$use_stage`;

    /**********************************************************************
     *                         -- State Machine --
     *
     * There are 4 states:
     *
     * empty: buffer is empty
     * retry: external buffer is full
     * read:  pop'd data in <n-1>, and instruction in <n> is valid
     * spec:  pop'd data in <n-1>, but instruction killed in <n>
     **********************************************************************/

;         my $p = uc($prefix.$name.'_STAGING');
    parameter	`$p`_EMPTY = 2'b00;
    parameter	`$p`_RETRY = 2'b01;
    parameter	`$p`_READ  = 2'b11;
    parameter	`$p`_SPEC  = 2'b10;

    wire [1:0]  cs;
    wire [1:0] ns;

    wire latch_new_data;
    wire nxt_empty;
    wire pop;

   assign ns =	(cs==`$p`_EMPTY) ?	(
			use_C`$read_stage` ?	(
				empty_in ?
								`$p`_RETRY :
								`$p`_READ
						) :
								`$p`_EMPTY
					) :
		(cs==`$p`_RETRY) ?	(
			retry_C`$use_stage` ?	(
				empty_in ?
								`$p`_RETRY :
								`$p`_READ
						) :
				use_C`$read_stage` ?	(
					empty_in ?
								`$p`_RETRY :
								`$p`_READ
							) :
								`$p`_EMPTY
					) :
		(cs==`$p`_READ) ?	(
			GlobalStall ?
								`$p`_READ :
		  	use_C`$use_stage` ?
				use_C`$read_stage` ?	(
					empty_in ?
								`$p`_RETRY :
								`$p`_READ
							) :
								`$p`_EMPTY : 
			use_C`$read_stage` ?
								`$p`_READ :
								`$p`_SPEC
					) :
		(cs==`$p`_SPEC) ?	(
			use_C`$read_stage` ?
								`$p`_READ :
								`$p`_SPEC
					) :
								2'b0;

   assign latch_new_data =
		(cs==`$p`_EMPTY) ?	(
			use_C`$read_stage` ?	(
				empty_in ?
								1'b0 :
								1'b1
						) :
								1'b0
					) :
		(cs==`$p`_RETRY) ?	(
			retry_C`$use_stage` ?	(
				empty_in ?
								1'b0 :
								1'b1
						) :
				use_C`$read_stage` ?	(
					empty_in ?
								1'b0 :
								1'b1
							) :
								1'b0
					) :
		(cs==`$p`_READ) ?	(
			GlobalStall ?
								1'b0 :
		  	use_C`$use_stage` ?
				use_C`$read_stage` ?	(
					empty_in ?
								1'b0 :
								1'b1
							) :
								1'b0 : 
			use_C`$read_stage` ?
								1'b0 :
								1'b0
					) :
		(cs==`$p`_SPEC) ?	(
								1'b0
					) :
								1'b0;

   assign nxt_empty =
		(cs==`$p`_EMPTY) ?	(
			use_C`$read_stage` ?	(
				empty_in ?
								1'b1 :
								1'b0
						) :
								1'b1
					) :
		(cs==`$p`_RETRY) ?	(
			retry_C`$use_stage` ?	(
				empty_in ?
								1'b1 :
								1'b0
						) :
				use_C`$read_stage` ?	(
					empty_in ?
								1'b1 :
								1'b0
							) :
								1'b1
					) :
		(cs==`$p`_READ) ?	(
			GlobalStall ?
								empty_out_C`$use_stage` :
		  	use_C`$use_stage` ?
				use_C`$read_stage` ?	(
					empty_in ?
								1'b1 :
								1'b0
							) :
								1'b1 : 
			use_C`$read_stage` ?
								1'b0 :
								1'b0
					) :
		(cs==`$p`_SPEC) ?	(
								1'b0
					) :
								1'b1;

   assign pop =	(cs==`$p`_EMPTY) ?	(
			use_C`$read_stage` ?
							1'b1 :
							1'b0
					) :
		(cs==`$p`_RETRY) ?	(
			retry_C`$use_stage` ?
							1'b1 :
				use_C`$read_stage` ?
							1'b1 :
							1'b0
					) :
		(cs==`$p`_READ) ?	(
			GlobalStall ?
							1'b0 :
		  	use_C`$use_stage` ?
				use_C`$read_stage` ?
							1'b1 :
							1'b0 : 
							1'b0
					) :
		(cs==`$p`_SPEC) ?	(
							1'b0
					) :
							1'b0;

    /**********************************************************************
    old style FSM

    always @(cs or use_C`$use_stage` or use_C`$read_stage` or empty_in
             or empty_out_C`$use_stage` or GlobalStall or retry_C`$use_stage`) begin
       ns = 2'bx;
       latch_new_data = 1'b0;
       nxt_empty = 1'b1;
       pop = 1'b0;

       case(cs)
          `$p`_EMPTY : 
                  if(use_C`$read_stage`) begin
                     pop = 1'b1;
                     // Speculative use, but external queue is empty
                     if(empty_in) ns = `$p`_RETRY;
                     // Speculative use and queue not empty
                     else begin
                        latch_new_data = 1'b1;
                        nxt_empty = 1'b0;
                        ns = `$p`_READ;
                     end
                  end
                  else ns = `$p`_EMPTY;

          `$p`_RETRY : 
                  if(retry_C`$use_stage`) begin
                     pop = 1'b1;
                     // Queue is still empty
                     if(empty_in) ns = `$p`_RETRY;
                     // Queue is no longer empty, advance stage
                     else begin
                        latch_new_data = 1'b1;
                        nxt_empty = 1'b0;
                        ns = `$p`_READ;
                     end
                  end
                  else
                     if(use_C`$read_stage`) begin
                        pop = 1'b1;
                        // Original use is killed, but new speculative use
                        if(empty_in) ns = `$p`_RETRY;
                        // Queue is no longer empty, advance stage for
                        // new usE
                        else begin
                           latch_new_data = 1'b1;
                           nxt_empty = 1'b0;
                           ns = `$p`_READ;
                        end
                     end
                     // Speculative use is killed, and no new use, and
                     // external buffer not popped, so return to empty
                     else ns = `$p`_EMPTY;

          `$p`_READ  :
                  if(GlobalStall) begin
                     nxt_empty = empty_out_C`$use_stage`;
                     ns = `$p`_READ;
                  end
                  else if(use_C`$use_stage`)
                     if(use_C`$read_stage`) begin
                        pop = 1'b1;
                        // Original use is consumed, another use follows
                        // directly, but external queue is empty
                        if(empty_in) ns = `$p`_RETRY;
                        // Back to back reads
                        else begin
                           latch_new_data = 1'b1;
                           nxt_empty = 1'b0;
                           ns = `$p`_READ;
                        end
                     end
                     // No subsequent reads
                     else ns = `$p`_EMPTY;
                 else 
                    // Speculative use is killed, but new use follow
                    // directly. Don't read new data.
                    if(use_C`$read_stage`) begin
                        nxt_empty = 1'b0;
                        ns = `$p`_READ;
                    end
                    // Speculative use is killed, store data
                    // until next use
                    else begin
                        nxt_empty = 1'b0;
                        ns = `$p`_SPEC;
                    end

          `$p`_SPEC  :
                 begin
                    nxt_empty = 1'b0;
                    // Data already held, don't need to check external 
                    // queue for data
                    if(use_C`$read_stage`) ns = `$p`_READ;
                    // Waiting for next read
                    else ns = `$p`_SPEC;
                 end

`ifdef NOXPROP
          default: ns = 2'b0;
`else
          default: ns = 2'bx;
`endif
       endcase
    end

     **********************************************************************/

    xt`$async`scflop #(2) state (cs, ns, !reset, fclk);

    /**********************************************************************
     *                         -- Control --
     **********************************************************************/

    xt`$async`scflop #(1) empty_reg (empty_out_C`$use_stage`, nxt_empty, !reset, fclk);
    assign enable_C`$read_stage` = pop;

    /**********************************************************************
     *                        -- Datapath --
     **********************************************************************/
;  if ( $ResetFlop == 1 ) {
    xt`$async`scenflop #(width) data (dataout_C`$use_stage`, datain, latch_new_data, !reset, fclk);
;  } else {
    xtenflop #(width) data (dataout_C`$use_stage`, datain, latch_new_data, fclk);
;  }

endmodule
;      }
;      # input queue
module `$prefix``$name`_buf_speculative (
	clk,
	Kill_E,
	killPipe_W,
	myGlobalStall,
	reset,
;      if ( $TMode ) {
	TMode,
;      }
;      if($trace) {
        traceout,
;      }
	datain,
	out_C`$use_stage`,
	quse_C`$use_stage`,
	quse_C`$commit`,
	stall_C`$use_stage`,
	enable,
	empty_C`$use_stage`,
	almost_empty_C`$use_stage`,
	full_C`$use_stage`);
    parameter width=`$width`; // vhdl generic

    input clk;
    input Kill_E;
    input killPipe_W;
    input myGlobalStall;
;      if ( $TMode ) {
    input TMode;
;      }
;      if($trace) {
    output [width-1:0] traceout;
;      }
    input reset;
    input quse_C`$use_stage`;
    input quse_C`$commit`;
    input stall_C`$use_stage`;
    input [width-1:0] datain;
    output [width-1:0] out_C`$use_stage`;
    output enable;
    output empty_C`$use_stage`;
    output almost_empty_C`$use_stage`;
    output full_C`$use_stage`;


    /**********************************************************************
     *                      -- FIFO Control Signals --
     *
     * Internal FIFO control signals are assumed to be early.
     **********************************************************************/

    wire xtpush;   // Push every data which is sampled
    wire xtpop;    // Pop on every use that is committed
    wire sxtpop;   // Pop on every use
    wire scancel;  // Cancel speculation pops
    wire xtempty;  
    wire sxtempty;
    wire xtfull;
    wire sxtfull;
    wire is_spec; 

    assign xtpush  = enable;
    assign xtpop   = quse_C`$commit`;
    assign sxtpop  = quse_C`$use_stage`;
    assign scancel = killPipe_W && is_spec;


    /**********************************************************************
     *                         -- Write Pointer --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width-1`:0] wr_ptr;
    wire [`$ptr_width-1`:0] wr_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_wr_ptr;
    assign wr_ptr_inc = wr_ptr + `$ptr_width`'h1;
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_wr_ptr = wr_ptr_inc;
;         } else {
    assign nxt_wr_ptr = wr_ptr_inc == `$ptr_width`'d`$depth`;
;         }
    xt`$async`scenflop #(`$ptr_width`) wr_ptr_reg (wr_ptr, nxt_wr_ptr, xtpush && !myGlobalStall, !reset, clk);
;      } else {
    wire wr_ptr = 1'b0;
;      }
    
    /**********************************************************************
     *                          -- Read Pointer --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width-1`:0] rd_ptr;
    wire [`$ptr_width-1`:0] rd_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_rd_ptr;
    assign rd_ptr_inc = rd_ptr + `$ptr_width`'h1;
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_rd_ptr = rd_ptr_inc;
;         } else {
    assign nxt_rd_ptr = rd_ptr_inc == `$ptr_width`'d`$depth`;
;         }
    xt`$async`scenflop #(`$ptr_width`) rd_ptr_reg (rd_ptr, nxt_rd_ptr, xtpop && !myGlobalStall, !reset, clk);
;      } else {
    wire rd_ptr = 1'b0;
;      }

    /**********************************************************************
     *                    -- Speculative Read Pointer --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width-1`:0] srd_ptr;
    wire [`$ptr_width-1`:0] srd_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_srd_ptr;
    assign srd_ptr_inc = srd_ptr + `$ptr_width`'h1;
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_srd_ptr = srd_ptr_inc;
;         } else {
    assign nxt_srd_ptr = srd_ptr_inc == `$ptr_width`'d`$depth`;
;         }
    wire [`$ptr_width-1`:0] srd_nxt_ptr;
    xtmux2e #(`$ptr_width`) srdnxtptr0_mux(srd_nxt_ptr,
                                nxt_srd_ptr /* 0 */,
                                rd_ptr /* 1 */,
                                scancel);
    xt`$async`scenflop #(`$ptr_width`) srd_ptr_reg (srd_ptr, srd_nxt_ptr, (sxtpop || scancel) && !myGlobalStall, !reset, clk);
;      } else {
    wire srd_ptr = 1'b0;
;      }

    /**********************************************************************
     *                         -- Entry Counter --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width`:0] entries;
    wire [`$ptr_width`:0] entries_inc;
    wire [`$ptr_width`:0] pre_entries;
    wire [`$ptr_width`:0] nxt_entries;
    xtmux4e #(`$ptr_width+1`) entriesinc_mux(entries_inc,
                                `$ptr_width+1`'h0 /* 0 */,
                                `$ptr_width+1`'h1 /* 1 */,
                                `$ptr_width+1`'h` sprintf("%lx",(2**($ptr_width+1)-1))` /* 2 */,
                                `$ptr_width+1`'h0 /* 3 */,
                                {xtpop,xtpush});
    xtadd #(`$ptr_width+1`) entries_add (pre_entries, entries, entries_inc);
    assign nxt_entries = pre_entries;
    xt`$async`scenflop #(`$ptr_width+1`) entries_reg (entries, nxt_entries, (xtpop || xtpush) && !myGlobalStall, !reset, clk);
;      } else {
    wire entries;
    wire entries_inc;
    wire pre_entries;
    wire nxt_entries;
    xtmux4e #(1) entriesinc_mux(entries_inc,
                                1'h0 /* 0 */,
                                1'h1 /* 1 */,
                                1'h1 /* 2 */,
                                1'h0 /* 3 */,
                                {xtpop,xtpush});
   
    xtadd #(1) entries_add (pre_entries, entries, entries_inc);
    assign nxt_entries = pre_entries;
    xt`$async`scenflop #(1) entries_reg (entries, nxt_entries, (xtpop || xtpush) && !myGlobalStall, !reset, clk);   
;      } 
   
    /**********************************************************************
     *                   -- Speculative Entry Counter --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width`:0] sentries;
    wire [`$ptr_width`:0] sentries_inc;
    wire [`$ptr_width`:0] pre_sentries;
    wire [`$ptr_width`:0] nxt_sentries;
    xtmux4e #(`$ptr_width+1`) sentriesinc_mux(sentries_inc,
                                `$ptr_width+1`'h0 /* 0 */,
                                `$ptr_width+1`'h1 /* 1 */,
                                `$ptr_width+1`'h` sprintf("%lx",(2**($ptr_width+1)-1))` /* 2 */,
                                `$ptr_width+1`'h0 /* 3 */,
                                {sxtpop,xtpush});
    xtadd #(`$ptr_width+1`) sentries_add (pre_sentries, sentries, sentries_inc);
    assign nxt_sentries = scancel ? entries : pre_sentries;
    xt`$async`scenflop #(`$ptr_width+1`) spre_entries_reg (sentries, nxt_sentries, (sxtpop || xtpush || scancel) && !myGlobalStall, !reset, clk);
;      } else {
    wire sentries;
    wire sentries_inc;
    wire pre_sentries;
    wire nxt_sentries;
    xtmux4e #(1) sentriesinc_mux(sentries_inc,
                                1'h0 /* 0 */,
                                1'h1 /* 1 */,
                                1'h1 /* 2 */,
                                1'h0 /* 3 */,
                                {sxtpop,xtpush});
   
    xtadd #(1) sentries_add (pre_sentries, sentries, sentries_inc);
    assign nxt_sentries = scancel ? entries : pre_sentries;
    xt`$async`scenflop #(1) sentries_reg (sentries, nxt_sentries, (sxtpop || xtpush || scancel) && !myGlobalStall, !reset, clk);   
;      } 
    
    /**********************************************************************
     *                           -- Empty/Full --
     **********************************************************************/

    assign xtempty = entries == `$ptr_width+1`'h0;
    assign xtfull  = entries == `$ptr_width+1`'h` sprintf("%lx",$depth)`;
    
    /**********************************************************************
     *                     -- Speculative Empty/Full --
     **********************************************************************/


    assign sxtempty = sentries == `$ptr_width+1`'h0;
    assign sxtfull  = sentries == `$ptr_width+1`'h` sprintf("%lx",$depth)`;
    assign is_spec = sentries != entries;    
    
    /**********************************************************************
     *                          -- Data Bypass --
     **********************************************************************/

    wire [width-1:0] rd_data;
    xtmux2p #(width) xtout_mux(out_C`$use_stage`,
                            datain /* 0 */,
                            rd_data /* 1 */,
;      if(0 && $depth > 1) {
;         #                       // why is this different?
                            (srd_ptr == wr_ptr) /* 0 */
;      } else {
                            (sxtempty) /* 0 */
;      }
                            );
    /**********************************************************************
     *                            -- Regfile --
     **********************************************************************/

;      if($depth == 1) {
;         if ( $ResetFlop == 1 ) {
    xt`$async`scenflop #(width) icore0(rd_data, datain, xtpush && !myGlobalStall, !reset, clk);
;         } else {
    xtenflop #(width) icore0(rd_data, datain, xtpush && !myGlobalStall, clk);
;         }
;      } else {
;         my $rd = $trace ? 2 : 1;
;         my @ports = qw/rd_data srd_ptr/;
;         push @ports, qw/traceout rd_ptr/ if($trace);
;         push @ports, qw/datain wr_ptr xtpush/;
;         push @ports, 'TMode' if($TMode);
;         push @ports, qw/reset clk/;
    xtregfile_`$rd`R1W_`$depth`_`$width`_FF_XT`$prefix` icore0(` join ', ', @ports`);
;      }

    /**********************************************************************
     *                      -- TIE Wires Interface --
     **********************************************************************/

    assign enable   = sxtempty && quse_C`$use_stage`;
    assign empty_C`$use_stage` = sxtempty;
    assign almost_empty_C`$use_stage` = sentries == `$ptr_width+1`'h1;
    assign full_C`$use_stage`  = xtfull;

;      if($trace) {
    /**********************************************************************
     *                           -- Trace --
     **********************************************************************/

;         if($depth == 1) {
    assign traceout = rd_data;
;         } else {
;            die "ERROR: Trace option not implemented for non M-stage input queues\n";
;         }
;      }


endmodule
;   } else {
;      # output queue
module `$prefix`TIE_outbuf_`$name` (
        clk,
	fclk,
;      if ( $TMode ) {
	TMode,
;      }
;      if($trace) {
        traceout,
        traceoutvalid,
;      }
	GlobalStall,
	Kill_E,
	killPipe_W,
	interrupt,
	replay_M,
	except_M,
	reset,
	def_C0,
; if ( $Kill ) {
	kill_C`$def_stage`,
; }
	datain,
	out_C`$def_stage+1`,
	valid,
	stall_in_C`$def_stage`,
	stall_out_C`$def_stage`,
	notrdy_C`$def_stage`,
        idle);
    parameter width=`$width`; // vhdl generic

    input clk;
    input fclk;
;      if ( $TMode ) {
    input TMode;
;      }
;      if($trace) {
    output [width-1:0] traceout;
    output traceoutvalid;
;      }
    input GlobalStall;
    input Kill_E;
    input killPipe_W;
    input interrupt;
    input replay_M;
    input except_M;
    input reset;
    input def_C0;
; if ( $Kill ) {
    input kill_C`$def_stage`;
; }
    input [width-1:0] datain;
    output [width-1:0] out_C`$def_stage+1`;
    output valid;
    input stall_in_C`$def_stage`;
    output stall_out_C`$def_stage`;
    output notrdy_C`$def_stage`;
    output idle;

    /**********************************************************************
     *                      -- DEF Control Pipeline --
     **********************************************************************/

;      for (my $i=0; $i < $commit; $i++) {
;         my $n = $i+1;
;         my $def = "def_C$i && !killPipe_W";
;         $def .= " && !Kill_E" if ($i == 1);
;         $def .= " && !kill_C$def_stage" if ($Kill && $i == $def_stage);
    wire qdef_C`$i` = `$def`;
    wire def_C`$n`;
    xt`$async`sc`$enflop`flop #(1) def`$n`(def_C`$n`, qdef_C`$i`, `$enport`!reset, clk);
;      }
    wire qdef_C`$commit` = def_C`$commit` && !killPipe_W;

;      my $fclk = 'fclk';
;      my $clk = 'clk';
;      if($ClkGateFunc) {
    /**********************************************************************
     *                      -- Clock Gating --
     *
     * Enable clock when buffers not empty or def in pipeline.
     **********************************************************************/

    wire g2sclk;  // stall clock
    wire g2fclk;  // free running clock (gated with waiti)
    wire en_clocks;
;         my @en = ('en_clocks');
;         my $clkgate = ($TMode)?"xtgated_tmode_clock":"xtgated_clock";
;         my $tmode = ($TMode)?"TMode,":"";
    `$clkgate` g2sclk_gate (g2sclk,`$tmode` ` join(' || ', @en)`, clk);
    `$clkgate` g2fclk_gate (g2fclk,`$tmode` ` join(' || ', @en)`, fclk);
;         $fclk = 'g2fclk';
;         $clk = 'g2sclk';
;      }

    /**********************************************************************
     *                      -- FIFO Control --
     **********************************************************************/

    wire staging_empty;
    wire buffer_empty;
    wire buffer_full;
    wire buffer_almost_full;
    wire empty = staging_empty && buffer_empty;
    wire push = qdef_C`$def_stage` && !GlobalStall && !except_M && !replay_M;
    wire pop = !empty && !stall_in_C`$def_stage`;
    wire myGlobalStall = ` $ClkGateGlobal ? "1'b0" : 'GlobalStall'`;
    wire [width-1:0] buffer_data;

    /**********************************************************************
     *                        -- Staging Reg --
     **********************************************************************/

    wire en_staging = stall_in_C`$def_stage`  ? (staging_empty && (push || !buffer_empty)) :
                      buffer_empty ? push : pop;
    wire [width-1:0] nxt_staging;
;   if ( $ResetFlop == 1 ) {
    xt`$async`scenflop #(width) staging_reg (out_C`$def_stage+1`, nxt_staging, en_staging, !reset, `$fclk`);
; } else {
    xtenflop #(width) staging_reg (out_C`$def_stage+1`, nxt_staging, en_staging, `$fclk`);
; }

    /**********************************************************************
     *                          -- Buffer --
     *
     * configurable depth. need 2-entries to operate at full bandwidth 
     * w/o stall.  consider the case:
     * 
     *                0 1 2 3 4 5 6 7 8 9
     *   full         0 0 0 1 1 1 1 1 1 1
     *   qwrite       e m w x
     *   qwrite       r e m w x
     *   qwrite       i r e m w w w w w w 
     *   globalstall  0 0 0 0 1 1 1 1 1 1 
     *
     * in this example the first two write are already buffered, and there
     * must be enough buffering space available for the third write in
     * order to be able to interrupt this stall
     **********************************************************************/

    wire push_buffer = staging_empty ? 1'b0
                                     : push && !(pop && buffer_empty);
    wire pop_buffer  = pop && !buffer_empty;
    wire [`$ptr_width+1-1`:0] buffer_entries;

    `$prefix``$name`_buf_queue queue(
	`$fclk`,
	myGlobalStall,
	reset,
;      if ( $TMode ) {
	1'b0,
;      }
	datain,
	buffer_data,
	push_buffer,
	pop_buffer,
	buffer_empty,
	buffer_almost_full,
	buffer_full,
	buffer_entries);

    /**********************************************************************
     *                       -- Buffer Bypass --
     **********************************************************************/

    assign nxt_staging = !buffer_empty ? buffer_data : datain;

    /**********************************************************************
     *                      -- Staging Control --
     **********************************************************************/

    wire nxt_staging_empty = staging_empty ? !push 
                                           : pop && !push && buffer_empty;
    xt`$async`ssenflop #(1) staging_empty_reg (staging_empty, nxt_staging_empty,
       (push || pop), !reset, 1'b1, fclk);    
                            
    /**********************************************************************
     *                            -- Stall --
     **********************************************************************/

    assign stall_out_C`$def_stage` = !interrupt && 
;      if($depth > 1) {
                          ((buffer_full && qdef_C`($def_stage-1)`)
                          || (buffer_almost_full && qdef_C`($def_stage-1)` && qdef_C`$def_stage`));
;      } else {
                          qdef_C`$def_stage` && buffer_full && !staging_empty;

;      }


    /**********************************************************************
     *                             -- Valid --
     **********************************************************************/

    assign valid = !staging_empty;

    /**********************************************************************
     *                            -- Full --
     *
     * This full only indicates whether the internal buffer can hold
     * another entry.  It considers the number of free entries in the
     * buffer. This signal MUST be looked at in the same stage as the
     * push to the buffer.
     **********************************************************************/

;      {
;         my $bits = ceillog2($depth+1+1);
    wire [`$bits-1`:0] buffers = {`$bits-1`'h0, !staging_empty}
;         if(($ptr_width+1) == $bits) {
        + buffer_entries;
;         } else {
        + {`$bits-($ptr_width+1)`'h0, buffer_entries};
;         }	
    
    assign notrdy_C`$def_stage` = buffers > `$bits`'d`$depth`;
;      }

    /**********************************************************************
     *                            -- Idle --
     **********************************************************************/
  
    assign idle = staging_empty && buffer_empty 
                  && ` join ' && ', map("!qdef_C$_", (1..$commit))`;

;      if($ClkGateFunc) {
    /**********************************************************************
     *                      -- Clock Gating --
     *
     * Enable clock when buffers not empty or def in pipeline.
     **********************************************************************/
 
    assign en_clocks = reset || !staging_empty || !buffer_empty 
                       || ` join ' || ', map("def_C$_", (0..$commit))`;


;      }
;      if($trace) {
    /**********************************************************************
     *                            -- Trace --
     **********************************************************************/

    assign traceout = out_C`$def_stage+1`;
    assign traceoutvalid = qdef_C`$commit`;
;      }
; if ( $Ten4Internal ) {
    /**********************************************************************
     *                      -- Coverage Monitor --
     **********************************************************************/
// synopsys translate_off
     wire internalBufFull = buffer_full && !staging_empty;
// synopsys translate_on
; }
endmodule
;      # output queue
module `$prefix``$name`_buf_queue (
	fclk,
	myGlobalStall,
	reset,
;      if ( $TMode ) {
	TMode,
;      }
	datain,
	dataout,
	xtpush,
	xtpop,
	xtempty,
	xtalmost_full,
	xtfull,
	xtentries);
    parameter width=`$width`; // vhdl generic

    input fclk;
    input myGlobalStall;
;      if ( $TMode ) {
    input TMode;
;      }
    input reset;
    input [width-1:0] datain;
    output [width-1:0] dataout;
    input xtpush;
    input xtpop;
    output xtempty;
    output xtalmost_full;
    output xtfull;
    output [`$ptr_width+1-1`:0] xtentries;
    
    /**********************************************************************
     *                         -- Write Pointer --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width-1`:0] wr_ptr;
    wire [`$ptr_width-1`:0] wr_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_wr_ptr;
    assign wr_ptr_inc = wr_ptr + `$ptr_width`'h1;
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_wr_ptr = wr_ptr_inc;
;         } else {
    assign nxt_wr_ptr = wr_ptr_inc == `$ptr_width`'d`$depth`;
;         }
    xt`$async`scenflop #(`$ptr_width`) wr_ptr_reg (wr_ptr, nxt_wr_ptr, xtpush && !myGlobalStall, !reset, fclk);
;      } else {
    wire wr_ptr = 1'b0;
;      }
    
    /**********************************************************************
     *                          -- Read Pointer --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width-1`:0] rd_ptr;
    wire [`$ptr_width-1`:0] rd_ptr_inc;
    wire [`$ptr_width-1`:0] nxt_rd_ptr;
    assign rd_ptr_inc = rd_ptr + `$ptr_width`'h1;
;         if($depth == (2**(ceillog2($depth)))) {
    assign nxt_rd_ptr = rd_ptr_inc;
;         } else {
    assign nxt_rd_ptr = rd_ptr_inc == `$ptr_width`'d`$depth`;
;         }
    xt`$async`scenflop #(`$ptr_width`) rd_ptr_reg (rd_ptr, nxt_rd_ptr, xtpop, !reset, fclk);
;      } else {
    wire rd_ptr = 1'b0;
;      }

    /**********************************************************************
     *                         -- Entry Counter --
     **********************************************************************/

;      if($depth > 1) {
    wire [`$ptr_width`:0] entries;
    wire [`$ptr_width`:0] entries_inc;
    wire [`$ptr_width`:0] pre_entries;
    wire [`$ptr_width`:0] nxt_entries;
    xtmux4e #(`$ptr_width+1`) entriesinc_mux(entries_inc,
                                `$ptr_width+1`'h0 /* 0 */,
                                `$ptr_width+1`'h1 /* 1 */,
                                `$ptr_width+1`'h` sprintf("%lx",(2**($ptr_width+1)-1))` /* 2 */,
                                `$ptr_width+1`'h0 /* 3 */,
                                {xtpop,xtpush});
    xtadd #(`$ptr_width+1`) entries_add (pre_entries, entries, entries_inc);
    assign nxt_entries = pre_entries;
    xt`$async`scenflop #(`$ptr_width+1`) entries_reg (entries, nxt_entries, xtpop || (xtpush && !myGlobalStall), !reset, fclk);
;      } else {
    wire entries;
    wire entries_inc;
    wire pre_entries;
    wire nxt_entries;
    xtmux4e #(1) entriesinc_mux(entries_inc,
                                1'h0 /* 0 */,
                                1'h1 /* 1 */,
                                1'h1 /* 2 */,
                                1'h0 /* 3 */,
                                {xtpop,xtpush});
   
    xtadd #(1) entries_add (pre_entries, entries, entries_inc);
    assign nxt_entries = pre_entries;
    xt`$async`scenflop #(1) entries_reg (entries, nxt_entries, xtpop || (xtpush && !myGlobalStall), !reset, fclk);   
;      } 
   
    /**********************************************************************
     *                           -- Empty/Full --
     **********************************************************************/

    wire xtempty         = entries == `$ptr_width+1`'h0;
    assign xtfull        = entries == `$ptr_width+1`'h` sprintf("%lx",$depth)`;
    assign xtalmost_full = entries == `$ptr_width+1`'h` sprintf("%lx",($depth-1))`;
    assign xtentries     = entries;

    /**********************************************************************
     *                            -- Regfile --
     **********************************************************************/

;      if($depth == 1) {
;        if ( $ResetFlop == 1 ) {
    xt`$async`scenflop #(width) icore0(dataout, datain, xtpush && !myGlobalStall, !reset, fclk);
;        } else {
    xtenflop #(width) icore0(dataout, datain, xtpush && !myGlobalStall, fclk);
;        }
;      } elsif ( $TMode ) {
    xtregfile_1R1W_`$depth`_`$width`_FF_XT`$prefix` icore0(dataout, rd_ptr, datain, wr_ptr, xtpush, TMode, reset, fclk);
;      } else {
    xtregfile_1R1W_`$depth`_`$width`_FF_XT`$prefix` icore0(dataout, rd_ptr, datain, wr_ptr, xtpush, reset, fclk);
;      }

endmodule
;   }
;}

;# vim:syn=v_tpp


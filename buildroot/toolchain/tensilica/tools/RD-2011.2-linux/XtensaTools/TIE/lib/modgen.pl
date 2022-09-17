#! /usr/xtensa/tools-6.1/bin/perl -w

# Copyright (c) 2003-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

use strict;
use Carp qw(cluck confess);
use English;
use File::Basename;
use FindBin qw($RealBin);

use vars qw($tctools);
BEGIN {
  $tctools = $ENV{'TCTOOLS'};
  if (!defined($tctools)) {
    my($tctools_base, $tctools_suffix);
    ($tctools_base, $tctools, $tctools_suffix) = fileparse($RealBin);
  }
}
require "$tctools/lib/modgen_prim.pl";

use strict;

sub uniq { 
    my(%seen); 
    return grep(! $seen{$_}++, @_); 
}

sub ceil_log2 {
    my($x) = @_;
    my($n);
    for($n = 0, $x -= 1; $x > 0; $x >>= 1, $n++) {
    }
    return $n;
}

sub init_print_break {
    my($indent) = @_;
    $main::col = 0;
    $main::indent = $indent;
}

sub print_break {
    my($d) = @_;
    if ($main::col + length($d) + 1 >= 85) {
	$main::col = 4;
	print ("\n" . (' ' x $main::indent));
    }
    print "$d";
    $main::col += length($d) + 1;
}

sub temp_inst {
    return "i" . ($main::temp_inst++);
}

sub temp_wire {
    return "t" . ($main::temp_wire++);
}

# create a "selector"-style mux module with $n data inputs
sub xtsel {
    my($module, $n) = @_;

    print "module $module(o, " . join(", ", map("d$_", 0..$n-1), map("s$_", 0..$n-1)) . ");\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] o;\n";
    print "input [size-1:0] " . join(", ", map("d$_", 0..$n-1)) . ";\n";
    print "input " . join(", ", map("s$_", 0..$n-1)) . ";\n";
    print map("wire [size-1:0] tmp$_ = {size{s$_}};\n", 0..$n-1);
    print "assign o = " . join(" | ", map("tmp$_ & d$_", 0..$n-1)) . ";\n";
    print "endmodule\n\n";
    return "";
}

# create a "selector"-style mux module with $n data inputs
sub xtmux {
    my($module, $n) = @_;

    my $Ten4Internal = (exists $ENV{'TEN4_INTERNAL'}) ? 1 : 0;

    confess "xtmux: bad parameter $n" if $n < 1;

    print "module $module(o, " . join(", ", map("d$_", 0..$n-1), map("s$_", 0..$n-1)) . ");\n";
    print "parameter size = 1, check = 1; // vhdl generic\n";
    print "output [size-1:0] o;\n";
    print "reg [size-1:0] o;\n" if ($n > 1);
    print "input [size-1:0] " . join(", ", map("d$_", 0..$n-1)) . ";\n";
    print "input " . join(", ", map("s$_", 0..$n-1)) . ";\n";
    if ($n == 1) {
	print "    assign o = d0;\n";
	print "endmodule\n";
	return;
    }
    print map("     wire [size-1:0] tmp$_;\n", 0..$n-1);
    print "\n";
    print map("     assign tmp$_ = (s$_)? d$_ : {size{1'b0}};\n", 0..$n-1);
    print "\n\n// one hot check\n\n";
    print "wire [15:0] onehot;\n";
    print "wire [15:0] " . join(", ", map("a$_", 0..$n-1)) . ";\n\n";
    if ($Ten4Internal) {
    print "// synopsys translate_off\n";
    print map("assign a$_ = {15'b0, s$_};\n", 0..$n-1);
    print "assign onehot = " . join(" + ", map("a$_", 0..$n-1)) . ";\n\n";
    print "// synopsys translate_on\n";
    };
    print "// spyglass disable_block W456a\n";
    print "always @(" . join(" or ", map("tmp$_", 0..$n-1)) . " or onehot) begin\n";
    if ($Ten4Internal) {
    print "        // synopsys translate_off\n";
    print "        if (check == 1 && onehot > 1 || check == 2 && onehot != 1)\n";
    print "            o = {size{1'bx}};\n";
    print "        else\n";
    print "        // synopsys translate_on\n";
    };
    print "            o = " . join(" | ", map("tmp$_", 0..$n-1)) . ";\n";
    print "    end\n";
    print "// spyglass enable_block W456a\n";
    print "endmodule\n\n";
    return "";
}

# create a "priority"-style mux module with $n data inputs
sub xtmuxp {
    my($module, $n) = @_;
    my($inst);

    confess "xtmuxp: bad parameter $n" if $n < 2;

    print "module $module(o, " . join(", ", map("d$_", 0 .. $n-1), map("s$_", 0 .. $n-2)) . ");\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] o;\n";
    print "input [size-1:0] " . join(", ", map("d$_", 0 .. $n-1)) . ";\n";
    print "input " . join(", ", map("s$_", 0 .. $n-2)) . ";\n";

    my $ctl_width = ceil_log2($n);
    print "    wire [" . ($ctl_width-1) . ":0] s = ";
    print join(" ", map("s$_ ? " . $ctl_width . "'d$_ :", 0 .. $n-2)) . " " . 
	$ctl_width . "'d" . ($n-1) . ";\n";

    $inst = temp_inst();
    print "    xtmux${n}e #(size) $inst(o, " . join(", ", map("d$_", 0..$n-1)) . ", s);\n";
    print "endmodule\n\n";

    $module =~ s/p/e/;
    return $module;
}

# create an "encoded"-style mux module with $n data inputs
sub xtmuxe {
    my($module, $n) = @_;
    my(@data, @sel, @data1, $s, $i, $c, $out, $port);
    my($d0, $d1, $d2, $d3, $s0, $s1, $inst, @need);

    confess "xtmuxe: bad parameter $n" if $n < 2;
    return "" if $n <= 4;		# these are not generated

    $s = ceil_log2($n);
    for($i = 0; $i < $n; $i++) {
	push(@data, "d$i");
    }
    for($i = 0; $i < $s; $i++) {
	push(@sel, "s[$i]");
    }

    print "module $module(o, " .  join(", ", @data) .  ", s);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] o;\n";
    print "input [size-1:0] " . join(", ", @data) . ";\n";
    print "input [" . ($s-1) . ":0] s;\n";

    if ($n == 2) {
	print "    assign o = s ? d1 : d0;\n";
    } else {
	#  mux tree
	$c = 0;
	while (@sel > 0) {
	    @data1 = ();
	    while (@data > 1) {
		$out = temp_wire();
		print "    wire [size-1:0] $out;\n";
		$inst = temp_inst();
		if (@data == 2) {
		    $d0 = shift @data;
		    $d1 = shift @data;
		    print "    xtmux2e #(size) $inst($out, $d0, $d1, $sel[0]);\n";
		    push(@need, "xtmux2e");
		} else {		# 3 or 4
		    $d0 = shift @data;
		    $d1 = shift @data;
		    $d2 = shift @data;
		    $d3 = @data > 0 ? shift @data : "{size{1'd0}}";
		    print "    xtmux4e #(size) $inst($out, $d0, $d1, $d2, $d3, {$sel[1], $sel[0]});\n";
		    push(@need, "xtmux4e");
		}
		push(@data1, $out);
	    }
	    shift @sel;
	    shift @sel;
	    @data = (@data1, @data);
	}

	$out = shift @data;
	print "    assign o = $out;\n";
	confess "Internal consistency failure\n" if @data != 0;
    }
    print "endmodule\n\n";

    return @need;
}

sub shift_expr {
    my($di, $d1, $dir1, $dir2) = @_;
    if ($d1 > 1048576) { $d1 = 1048576; };
    return $d1 == 0 ? $di : 
	$dir2 eq "" ?
	    "$di $dir1 ($d1 * x)" :
	    "($di $dir1 ($d1 * x)) | ($di $dir2 (pin - ($d1 * x)))";
}

sub xtshift {
    my($module, $a, $dir1, $dir2, $ref) = @_;
    my($di, $do, $i, $inst, $last, $m);

    confess "xtshift: bad parameter a=$a" if $a < 1;
    # Nupur 3/24/06: Commenting this out because it prints stack trace
    # with eval strings of the whole tpp file in some cases.
    # This already generates a TC warning if the shift comes from TC
    #cluck "xtshift: unreasonable parameter a=$a" if $a >= 16;

    $m = 1 << $a;

    print "module $module(o, i, amt);\n";
    print "parameter pout=$m, pin=$m; // vhdl generic\n";
    print "parameter x=1, y=0; // vhdl generic\n";
    print "output [pout-1:0] o;\n";
    print "input [pin-1:0] i;\n";
    print "input [" . ($a-1) . ":0] amt;\n";

    if ($ref) {
      if ($dir2 eq "") {
        print "    assign o = i $dir1 (amt * x + y);\n";
      } else {
        print "    assign o = (i $dir1 (amt * x + y)) | (i $dir2 (pin - (amt * x + y)));\n";
      }
    } else {
	$di = temp_wire();
	if ($dir2 eq "") {
	    print "    wire [pin-1:0] $di = (i $dir1 y);\n";
	} else {
	    print "    wire [pin-1:0] $di = (i $dir1 y) | (i $dir2 (pin - y));\n";
	}

	$last = ($a%2) == 1 ? $a-2 : $a-1;
	if ($last == $a-2) {
	    $i = $a - 1;
	    $do = temp_wire();
	    $inst = temp_inst();
	    print "    wire [pin-1:0] $do;\n";
	    print "    xtmux2e #(pin) $inst($do,\n";
	    print "        " . shift_expr($di, 0 << $i, $dir1, $dir2) . ",\n";
	    print "        " . shift_expr($di, 1 << $i, $dir1, $dir2) . ",\n";
	    print "        amt[$i]);\n";
	    $di = $do;
	}
	for($i = $last-1; $i >= 0; $i -= 2) {
	    $do = temp_wire();
	    $inst = temp_inst();
	    print "    wire [pin-1:0] $do;\n";
	    print "    xtmux4e #(pin) $inst($do,\n";
	    print "        " . shift_expr($di, 0 << $i, $dir1, $dir2) . ",\n";
	    print "        " . shift_expr($di, 1 << $i, $dir1, $dir2) . ",\n";
	    print "        " . shift_expr($di, 2 << $i, $dir1, $dir2) . ",\n";
	    print "        " . shift_expr($di, 3 << $i, $dir1, $dir2) . ",\n";
	    print "        amt[" . ($i+1) . ":$i]);\n";
	    $di = $do;
	}
	print "    assign o = $di"."[pout-1:0];\n";
    }
    print "endmodule\n\n";
}

sub xtshift_right {
    my($module, $a) = @_;
    xtshift($module, $a, ">>", "", 0);
}

sub xtshift_left {
    my($module, $a) = @_;
    xtshift($module, $a, "<<", "", 0);
}

sub xtrotate_right {
    my($module, $a) = @_;
    xtshift($module, $a, ">>", "<<", 0);
}

sub xtrotate_left {
    my($module, $a) = @_;
    xtshift($module, $a, "<<", ">>", 0);
}

sub ref_xtrotate_right{
    my($module, $a) = @_;
    xtshift($module, $a, ">>", "<<", 1);
}

sub ref_xtrotate_left {
    my($module, $a) = @_;
    xtshift($module, $a, "<<", ">>", 1);
}

sub ref_xtshift_right {
    my($module, $a) = @_;
    xtshift($module, $a, ">>", "", 1);
}

sub ref_xtshift_left {
    my($module, $a) = @_;
    xtshift($module, $a, "<<", "", 1);
}


# the lzc generation derives from the
# lz tpp function in fp.tie.  This function is
# slightly different because it handles non-powers of
# 2 and computes an extra bit for powers of two so
# that an all-zero value will return the number of bits.
#
# lz only handled powers of 2 bits and returned the
# same result for 0 ... 001 as 0 ... 000
#
sub xtlzc_orgen {
    my ($in, $n, $i, $mid) = @_; 
    printf "    wire or_%d_%d = ", $i, $mid;
    my $start = $i;
    for(my $k = $n / 4; $k > 1; $k /= 2) {
      my $stop = $start - $k + 1;
      printf "or_%d_%d | ", $start, $stop;
      $start = $stop - 1;
    }
    printf "%s[%d] | %s[%d];\n", $in, $mid + 1, $in, $mid;
}

sub xtlzc_recur {
  my($in, $out, $i, $j, $root) = @_;
  my($me, $t1, $t2);
  my($n) = $i - $j + 1;
  confess "n must be power of 2 (n=$n, i=$i, j=$j)" if $n != (1 << ceil_log2($n));
  if ($n >= 4) {
    my($mid) = $i - $n/2 + 1;
    $me = $root ? $out : $out . "_" . $i . "_" . $j;
    my $me_out = $out . "_" . $i . "_" . $j;
    $t1 = &xtlzc_recur($in, $out, $i, $mid, 0);
    $t2 = &xtlzc_recur($in, $out, $mid-1, $j, 0);
    xtlzc_orgen($in, $n, $i, $mid);
    printf "    wire [%d:0] %s = or_%d_%d ? {1'b0,%s} : {1'b1,%s};\n",
      ceil_log2($n) - 1, $me_out, $i, $mid, $t1, $t2;
    if ($root) {
	xtlzc_orgen($in, $n*2, $i, $j);
	printf "    wire [%d:0] %s = or_%d_%d ? {1'b0,%s} : {1'b1, %d'b0};\n",
	ceil_log2($n), $out, $i, $j, $me_out, ceil_log2($n);
    }
  } else {
    $me = sprintf "~%s[%d]", $in, $i;
  }
  return $me;
}

sub xtlzc_wires {
    my ($inname, $outname, $n) = @_;
    if ($n == 1) {
	print "    wire [0:0] $outname = {~$inname};\n";
    } elsif ($n == 2) {
	printf "    wire [1:0] or_1_0 = %s[1] | %s[0];\n", $inname, $inname;
        printf "    wire [1:0] $outname = or_1_0 ? {1'b0, ~%s[1]} : {1'b1, 1'b0};\n", $inname, $inname;
    } else {
	&xtlzc_recur($inname, $outname, $n-1, 0, 1);
    }
}

sub xtlzc_simple_wires {
    my ($inname, $outname, $n) = @_;
    my $k = ceil_log2($n);
    my $k_m1 = $k+1;
    print "    wire [$k:0] $outname = ";
    for (my $i = 0; $i < $n; ++$i) {
	my $b = $n-$i-1;
	printf " %s[$b] ? %d'd%d :", $inname, ($k+1), $i;
    }
    printf " %d'd$n;\n", ($k+1);
}

sub xtlzc { 
    my($module, $n, $ref) = @_;
    my($di, $do, $i, $inst, $last, $m);

    my $k = ceil_log2($n);
    my $ibits = (1<<$k); # aligned to a power of 2.
    my $ibits_m1 = $ibits-1;
    my $obits = ($ibits == $n) ? $k+1 : $k;
    my $obits_m_1 = $obits-1;
    
    my $n_m_1 = $n-1;
    my $n_p_1 = $n+1;
    my $extended_k = ceil_log2($n+1);
    my $k_m_1 = $k-1;


    print "module $module(o, i);\n";
    print "output [$obits_m_1:0] o;\n";
    print "input [$n_m_1:0] i;\n";

    if ($main::simple) {
      print "wire [$n:0] extended_i = {i,1'b1};\n";
      print "wire [$n:0] not_needed;\n";
      print "wire [$extended_k:0] temp;\n";
      print "  DW_lzd #($n_p_1) lz1 (.a(extended_i),.dec(not_needed),.enc(temp));\n";
      print "assign o = temp[$obits_m_1:0];\n";
    } else {
      my $inname = ($ibits == $n) ? "i" : "tmp_i";
      if ($ibits != $n) {
	my $x = $ibits-$n-1;
	my $zeros = sprintf("%d'b0", $x);
	printf "    wire [$ibits_m1:0] tmp_i = { i, 1'b1 %s };\n",
		($x == 0) ? "" : ", $zeros";
      }
      if ($ref) {
	# TBD: generate a reference.
	&xtlzc_simple_wires($inname, "tmp_o", $ibits);
      } else {
	&xtlzc_wires($inname, "tmp_o", $ibits);
      }
      print "    assign o = tmp_o[$obits_m_1:0];\n";
    }
    print "endmodule\n\n";
}


sub regfile {
    my($module, $read, $write, $depth, $win, $win_size, $win_num, $use_flipflop, $reset, $async, $fpgabuild, $size ) = @_;
    my(@need, $j, $i, $k, $a);
    my $useTMode = $main::TestFullScan &&
	($main::TestLatchesTransparent || $main::ClkGateFuncUnit || $main::AsyncReset);

    my $Ten4Internal = (exists $ENV{'TEN4_INTERNAL'}) ? 1 : 0;

    $a = ceil_log2($depth);

    if ($depth <= 2) {
	$use_flipflop = 1;
    }
    #Determine if clkgate should be perword if flip-flops are used.
    my $gateperword = ($size)?($size >= 32 ):(($win==1)?1:0);


    init_print_break(2);
    print_break("module $module(");
    for($i = 0; $i < $read; $i++) {
	print_break("rd${i}_data, ");
	if ($a > 0) {
	    print_break("rd${i}_addr, ");
	}
    }
    for($i = 0; $i < $write; $i++) {
	print_break("wr${i}_data, ");
	if ($a > 0) {
	    print_break("wr${i}_addr, ");
	}
	print_break("wr${i}_we, ");
    }
    if ($useTMode || $main::forceTMode) {
	print_break("TMode, ");
    }
    if ( $win ) {
        print_break("WinBase, ");
    }
    if ( $reset ) {
        print_break("Reset, ");
    }
    print_break("clk);\n");
    unless ($size) {
      print "parameter size=32; // vhdl generic\n";
      $size = "size";
    }
    my $left;
    my $addr_left = $a - 1;

    for($i = 0; $i < $read; $i++) {
	print "output [$size-1:0] rd${i}_data;\n";
	if ($a > 0) {
	    print "input [$addr_left:0] rd${i}_addr;\n";
	}
    }
    for($i = 0; $i < $write; $i++) {
	print "input [$size-1:0] wr${i}_data;\n";
	if ($a > 0) {
	    print "input [$addr_left:0] wr${i}_addr;\n";
	}
	print "input wr${i}_we;\n";
    }
    if ($useTMode || $main::forceTMode) {
	print "input TMode;\n";
    }
    if ( $win ) {
        $left = ceil_log2($win_num) - 1;
        print "input [${left}:0] WinBase;\n";
    }
    if ( $reset ) {
	print "input Reset;\n";
    }
    print "input clk;\n";

    if ($fpgabuild && ($write <= 2) && ($a > 0) && ($depth >= 8) && ($depth <=128)){
	my $selectBits = ceil_log2($write);
	if ($write <=1){
	    for ($i = 0;$i < $read;$i++){
		print_break("\n${module}_model #($size) U${i} (\n");
		print ".Data (wr0_data),\n";
		print ".RdAddress (rd${i}_addr),\n";
		print ".WrAddress (wr0_addr),\n";
		print ".RdEn (1'b1),\n";
		print ".WrEn (wr0_we),\n";
		print ".Q (rd${i}_data),\n";
		print ".WrClock (clk),\n";
		print ".WrClken (1'b1)\n";
		print ");\n";
	    }
	}else{
	# This code is for a 2 write port regfile.  It is unclear whether
	# it is economical to support more write ports in the fpga ram
	# since there is so much replication required.
	# This is coded in a more behaviorial verilog style for simplicity
	    for ($i = 0;$i < $read;$i++){
		for ($j = 0;$j < $write;$j++){
		    print "\nwire [$size-1:0] rd${i}_${j}_data;\n";

		    print_break("\n${module}_model #($size) U${i}_${j} (\n");
		    print ".Data (wr${j}_data),\n";
		    print ".RdAddress (rd${i}_addr),\n";
		    print ".WrAddress (wr${j}_addr),\n";
		    print ".RdEn (1'b1),\n";
		    print ".WrEn (wr${j}_we),\n";
		    print ".Q (rd${i}_${j}_data),\n";
		    print ".WrClock (clk),\n";
		    print ".WrClken (1'b1)\n";
		    print ");\n";
		}
	    }
	    print "reg [".($depth-1).":0] portOwner;\n";
	    print "always @(posedge clk) begin\n";
	    print "\tif (wr0_addr == wr1_addr) begin\n";
	    print "\t\tif (wr0_we)\n";
	    print "\t\t\tportOwner[wr0_addr] <= #1 1'b0;\n";
	    print "\t\telse if (wr1_we)\n";
	    print "\t\t\tportOwner[wr1_addr] <= #1 1'b1;\n";
	    print "\tend else begin\n";
	    print "\t\tif (wr0_we)\n";
	    print "\t\t\tportOwner[wr0_addr] <= #1 1'b0;\n";
	    print "\t\tif (wr1_we)\n";
	    print "\t\t\tportOwner[wr1_addr] <= #1 1'b1;\n";
	    print "\tend\n";
	    print "end\n\n\n";

	    for ($i = 0;$i < $read;$i++){
	      print "assign rd${i}_data = portOwner[rd${i}_addr] ? rd${i}_1_data : rd${i}_0_data;\n";
	    }
	}
	print "endmodule\n";
	print_break("\nmodule ${module}_model (Data, RdAddress, WrAddress, RdEn, WrEn, Q, WrClock, WrClken);\n");
	
	print "parameter Width = 32; // vhdl generic\n";
	print "parameter WidthAd = $a; // vhdl generic\n";
	print "parameter NumWords = $depth; // vhdl generic\n";
	print "\n";
	print "input [Width-1:0] Data;\n";
	print "input [WidthAd-1:0] RdAddress;\n";
	print "input [WidthAd-1:0] WrAddress;\n";
	print "input RdEn;\n";
	print "input WrEn;\n";
	print "output [Width-1:0] Q;\n";
	print "input WrClock;\n";
	print "input WrClken;\n";
	print "\n";
	print "// internal reg\n";
	print "reg [Width-1:0] mem_data [0:NumWords-1];\n";
	print "\n";
	print "  always @(posedge WrClock)\n";
	print "     begin\n";
	print "       if (WrClken && WrEn)\n";
	print "          begin\n";
	print "            mem_data[WrAddress] <= #1 Data;\n";
	print "          end\n";
	print "     end \n";
	print "\n";
	print "  assign Q = mem_data[RdAddress];\n";
	print "\n";
	print "endmodule\n\n";

    }else{
    if ($a == 0) {
	for($i = 0; $i < $write; $i++) {
	    print "    wire wr${i}_addr = 1'b0;\n";
	}
    }

    if ($write > 1) {
	for (my $x=0; $x < $write; $x++) {
	    for (my $y=$x+1; $y < $write; $y++) {
                my $sva_string = <<"EOT";
`ifdef SVA
  ${module}_multiple_writer_check${x}_${y}: assert property (
    @(posedge clk)
      (`GLBRST == 0) |->
        !((wr${x}_addr == wr${y}_addr && wr${x}_we && wr${y}_we)))
    else \$display(\"SVA Error: Multiple writes to the same register\");
`endif

EOT
                print "$sva_string" if ($Ten4Internal);
		#DISABLE print "    /* 0in custom -fire (wr${x}_addr == wr${y}_addr && wr${x}_we && wr${y}_we)\n -name ${module}_multiple_writer_check${x}_${y} -message \"Multiple writes to the same register\" */ \n";
	    }
	}
    }
    
    ###############################################################
    # Functional clk gating for FF based register file per word
    # Always do clock gate for any size register file if $main::useGatedClocks is on.
    # Generate a single clock gate for the whole register file
    # with the write enable(s) if the width of register file is < 32.
    # If the width is > 32, generate a clock gate per word/row after
    # the address has been decoded. This is done for optimizing the
    # number of clock gates we generate.
    ###############################################################
    if ($main::ClkGateFuncUnit && $use_flipflop == 1 && !$gateperword) {
       my $clkgate = ($useTMode)?"xtgated_tmode_clock":"xtgated_clock";
       my $tmode = ($useTMode)?"TMode,":"";
       my $en = join('|', map('wr'.$_.'_we', (0..$write-1)));
       my $reset_en  = (($main::ResetFlops) && !$async) ?"|Reset": "";
       print "    wire gclk;\n";
       print "    wire clken = $en $reset_en;\n";
       print "    $clkgate xtclk(gclk, $tmode clken, clk);\n";
    }
    for($j = 0; $j < $depth; $j++) {
	my $word = "word$j";
	my $we = "word${j}_we";

	if ($write > 1) {
	    # a write-enable for each word for each port
	    for($i = 0; $i < $write; $i++) {
		if ($a > 0) {
		    print "    wire wr${i}_word${j}_we = wr${i}_we & (wr${i}_addr == ${a}'d$j);\n";
		} else {
		    print "    wire wr${i}_word${j}_we = wr${i}_we;\n";
		}
	    }
	    # write-enable for this word is the OR of the per-port write-enable
	    print "    wire $we = ";
	    print join(" | ", map("wr${_}_word${j}_we", 0 .. $write-1));
	    print ";\n";
	} else {
	    if ($a > 0) {
		print "    wire $we = wr0_we & (wr0_addr == ${a}'d$j);\n";
	    } else {
		print "    wire $we = wr0_we;\n";
	    }
	}
        ###############################################################
       	# Functional clk gating for FF based register file per word
        ###############################################################
        if ($main::ClkGateFuncUnit && $use_flipflop == 1 && $gateperword) {
            my $clkgate = ($useTMode)?"xtgated_tmode_clock":"xtgated_clock";
            my $tmode = ($useTMode)?"TMode,":"";
            my $reset_en  = (($main::ResetFlops) && !$async) ?"|Reset": "";
       	    print "    wire gclk${j};\n";
            print "    $clkgate xtclk${j}(gclk${j}, $tmode $we $reset_en, clk);\n";
        }

	# priority mux to select the data to be written
	my $wdata;
	if ($write > 1) {
	    $wdata = "wmux$j";
	    print "    wire [$size-1:0] $wdata;\n";
	    print "    xtmux${write}p #($size) i$wdata($wdata, ";
	    print join(", ", map("wr${_}_data", 0 .. $write-1));
	    print ", ";
	    print join(", ", map("wr${_}_word${j}_we", 0 .. $write-2));
	    print ");\n";
	    push(@need, "xtmux${write}p");
	} else {
	    $wdata = "wr0_data";
	}

	# connect to the latch
	print "    wire [$size-1:0] $word;\n";
	if ($use_flipflop) {
            my $clk = ($main::ClkGateFuncUnit)?($gateperword?"gclk${j}":"gclk"):"clk";
            my $enflop = ($main::ClkGateFuncUnit && $gateperword)?'':'en';
            my $clrflop = (($main::ResetFlops) && $async) ?'asc':
		          (($main::ResetFlops) && !$async) ?'sc' : '';
            my $flop = 'xt'. $clrflop . $enflop . 'flop';
            my $enable = ($main::ClkGateFuncUnit && $gateperword)?"":"${we}, ";
            my $reset = ($main::ResetFlops) ?"!Reset, ":"";
	    print "    $flop #($size) i$word($word, $wdata, $enable $reset $clk);\n";
	} else {
	    #Enable for latch, OR'ed with Reset. This reset will be a sync reset
            my $enable = ($main::ResetFlops)?"($we|Reset)":$we;
	    if ($main::UseGatedClocks) {
		if ($main::TestLatchesTransparent) {
		    print "    wire gclk${j}, gclk${j}b;\n";
		    print "    xtclock_gate_or xt_clock_gate_or$j(gclk${j}b, clk, ~$enable);\n";
		    print "    assign gclk$j = ~(~TMode & gclk${j}b);\n";
		    print "    xtRFlatch #($size) i$word($word, $wdata, gclk$j);\n";
		} else {
		    print "    wire gclk$j;\n";
		    print "    xtclock_gate_nor xt_clock_gate_nor$j(gclk$j, clk, ~$enable);\n";
		    print "    xtRFlatch #($size) i$word($word, $wdata, gclk$j);\n";
		}
	    } else {
		print "    xtRFenlatch #($size) i$word($word, $wdata, $enable, clk);\n";
	    }
	}
	print "\n";
    }
    if ($use_flipflop) {
	my $all_ens = join(', ', map('word' . $_ . '_we', 0..$depth-1));
	my $maxw = ($write > $depth) ? $depth : $write;
               my $sva_string = <<"EOT";
`ifdef SVA
  ${module}_we_check: assert property (
    @(posedge clk)
      (`GLBRST == 0) |->
        (\$countones({$all_ens}) <= $maxw))
    else \$display(\"SVA Error: >$maxw write enables asserted\");
`endif

EOT
                print "$sva_string" if ($Ten4Internal);
	#DISABLE print "    /* 0in bits_on -var ({$all_ens})\n -max $maxw -name ${module}_we_check */ \n";
    } else {
	if ($main::UseGatedClocks) {
	    my $all_ens = join(', ', map('gclk' . $_, 0..$depth-1));
	    my $all_clks = join(' | ', map('gclk' . $_, 0..$depth-1));
	    if (!$main::0inbuild) {
		if ($main::TestLatchesTransparent) {
                my $sva_string = <<"EOT";
`ifdef SVA
  wire #1 ${module}_we_check_clk = $all_clks;
  ${module}_we_check: assert property (
    @(posedge ${module}_we_check_clk)
        ((`GLBRST == 0) && ~TMode) |->
        (\$countones({$all_ens}) <= $write))
    else \$display(\"SVA Error: >$write write enables asserted\");
`endif

EOT
                print "$sva_string" if ($Ten4Internal);
		    #DISABLE print "    /* 0in bits_on -var ({$all_ens})\n -max $write -active (~TMode) -clock ($all_clks)\n -name ${module}_we_check */\n";
		} else {
                my $sva_string = <<"EOT";
`ifdef SVA
  wire #1 ${module}_we_check_clk = $all_clks;
  ${module}_we_check: assert property (
    @(posedge ${module}_we_check_clk)
      (`GLBRST == 0) |->
        (\$countones({$all_ens}) <= $write))
    else \$display(\"SVA Error: >$write write enables asserted\");
`endif

EOT
                print "$sva_string" if ($Ten4Internal);
		    #DISABLE print "    /* 0in bits_on -var ({$all_ens})\n -max $write -clock ($all_clks)\n -name ${module}_we_check */\n";
		}
	    }
	} else {
	    my $all_ens = join(', ', map('word' . $_ . '_we', 0..$depth-1));
                my $sva_string = <<"EOT";
`ifdef SVA
  ${module}_we_check: assert property (
    @(posedge clk)
      (`GLBRST == 0) |->
        (\$countones({$all_ens}) <= $write))
    else \$display(\"SVA Error: >$write write enables asserted\");
`endif

EOT
                print "$sva_string" if ($Ten4Internal);
	    #DISABLE print "    /* 0in bits_on -var ({$all_ens})\n -max $write -name ${module}_we_check */\n";
	}
    }
    
    #Read mux for windowed register file
    my $rdmuxsize = $depth;
    my $rdmuxselbits = $a;
    my $rdword = "word";
    if ( $win  && $depth > 1 && $depth > $win_size ) {
        #Generate first level mux structure
        my $nm = $win_size;		 # number of muxes in first level
	my $nmbits = ceil_log2($nm);
	my $wbits = ceil_log2($win_num);
	my $nsel = ($depth/$win_size);	 # number of inputs to each mux
	my $selbits = ceil_log2($nsel);  # number of bits in the select signal
	my $ngroups = ($depth/$win_num); # number of muxes whose selects are same
	my $ngbits =  ceil_log2($ngroups);
	my @selvaltable;		 # table wb/selects as shown in spec
	my @selval;			 # each row of table computed
	my $val;
	my $sel;

	#Computing the table 
	for ( $i = 0; $i < $ngroups ; $i ++ ) {
	    $selval[$i] = 0;
	}
	for  ( $j = 0; $j < $win_num; $j++ ) {
	    print "        wire WinBase${j} = ( WinBase == ${wbits}'d${j} );\n";
	    print "       // WinBase ${wbits}'d${j}:"; 
	    for ( $i=0 ; $i<$ngroups; $i++) {
	       $val = $selval[$i];
	       print "sel${i} = ${selbits}'d${val},\t";
            }
	    print "\n";
	    push(@selvaltable, @selval);
	    #Rotate the values
	    $val = (($j%$ngroups)==0)?(($selval[0] + 1) % $nsel):$selval[0];
	    for ( $i=$ngroups-1; $i > 0; $i --) {
	       $selval[$i] = $selval[$i-1];
            }
	    $selval[0] = $val; 
	}

	#Write out the select lines by looking at the table
	#dumb implementation but clear. for each select group
	#get that column from the table. Now for each bit of
	#the select, go through the column and see which values
	#match and pick up those index values (i.e. the values
	#of windowbase. For example,
	#sel0_0 = (WinBase == 4'd0)  | 
	#         (WinBase == 4'd13) | 
	#         (WinBase == 4'd14) | 
	#         (WinBase == 4'd15);
	for ( $i = 0; $i < $ngroups ; $i ++ ) {
	    #Get selvaltable column i
	    my @vals; 
	    for($j = 0; $j < $win_num; $j++ ) {
	        push(@vals, $selvaltable[$j*$ngroups + $i]);
	    }
	    #Look at the column to see what values of WindowBase
	    #apply for this select line
	    for( $j = 0; $j < $nsel; $j ++ ) {
	        print "    wire sel${i}_${j} = 1'b0 ";
		for( $k = 0; $k < $win_num; $k++) {
		   print " | WinBase${k} " if ( $vals[$k] ==  $j );
	        }
		print ";\n";
	    }
	}
        #Instantiate the muxes
	for ( $i=0; $i<$win_size; $i++ ) {
	    $sel = $i >> $ngbits;
	    print "    wire [$size-1:0] rd_word${i};\n";
	    print "    xtmux${nsel} #($size) win$i(rd_word${i}, ";
	    print join(", ", map("word$_", map($i+$_*$win_size, 0..($nsel-1))));
	    print ",\n        ";
	    print join(", ", map("sel${sel}_$_", 0..($nsel-1)));
	    print ");\n";
	}
	push(@need, "xtmux${nsel}");
	$rdmuxsize = $nm;
	$rdmuxselbits = $nmbits;
	$rdword = "rd_word";
    }
    # drop an instance of a read-mux for each read port
    $left = $rdmuxselbits-1;
    for($i = 0; $i < $read; $i++) {
	if ($depth > 1) {
	    print "    xtmux${rdmuxsize}e #($size) rd$i(rd${i}_data, ";
	    print join(", ", map("${rdword}$_", 0 .. $rdmuxsize-1));
	    print ", rd${i}_addr[${left}:0]);\n";
	    push(@need, "xtmux${rdmuxsize}e");
	} else {
	    print "    assign rd${i}_data = word0;\n";
	}
    }
    print "endmodule\n\n";
}
    return @need;
}


sub xtwregfile {
    my($module, $ws, $wn, $r, $w, $n) = @_;
    print STDERR "windowed regfile with window size $ws and window number $wn\n";
    return regfile($module, $r, $w, $n, 1, $ws, $wn, $main::UseFlipFlops, $main::ResetFlops, $main::AsyncReset, $main::FPGABuild);
}

sub xtregfile {
    my($module, $r, $w, $n) = @_;
    return regfile($module, $r, $w, $n, 0, 0, 0, $main::UseFlipFlops, $main::ResetFlops, $main::AsyncReset, $main::FPGABuild);
}

sub xtregfile_FF {
    my($module, $r, $w, $n) = @_;
    return regfile($module, $r, $w, $n, 0, 0, 0, 1,$main::ResetFlops, $main::AsyncReset, $main::FPGABuild);
}

sub xtwrf {
  my ($module, $ws, $wn, $r, $w, $n, $size) = @_;
  return regfile($module, $r, $w, $n, 1, $ws, $wn, $main::UseFlipFlops, $main::ResetFlops, $main::AsyncReset, $main::FPGABuild, $size);
}

sub xtrf {
  my ($module, $r, $w, $n, $size) = @_;
  return regfile($module, $r, $w, $n, 0, 0, 0, $main::UseFlipFlops, $main::ResetFlops, $main::AsyncReset, $main::FPGABuild, $size);
}

sub xtrf_FF {
    my($module, $r, $w, $n, $size) = @_;
    return regfile($module, $r, $w, $n, 0, 0, 0, 1, $main::ResetFlops, $main::AsyncReset, $main::FPGABuild, $size);
}

sub csa_debug {
    my($array, $out) = @_;
    my($i, $j);
    for($i = 0; $i < $out; $i++) {
	printf "// array[%2d]: ", $i;
	foreach $j (@{$array->[$i]}) {
	    printf "%5s ", $j;
	}
	print "\n";
    }
}

sub csa {
    my($x, $y, $z, $weight, $left_edge) = @_;
    my($key, $value, $inst, $c, $s, $magic);

    $magic = "%";
    ($x, $y, $z) = sort {$a cmp $b} ($x, $y, $z);
    $key = join($magic, $x, $y, $z);
    $value = $main::csahash{$key};
    if (defined $value) {
	($c, $s) = split($magic, $value);
    } else {
	$s = "s" . $main::cnt . "_" . $weight;
	$c = "c" . $main::cnt . "_" . ($weight+1);
	if ($x eq "1'd0") {
	    if ($left_edge) {
		print "    wire $s = $y ^ $z;\n";
	    } else {
		$inst = temp_inst();
		print "    wire $s, $c;\n";
		print "    xtha $inst($s, $c, $y, $z);\n";
		$main::csahash{$key} = join($magic, $c, $s);
	    }
	    $main::xtha++;
	} else {
	    if ($left_edge) {
		print "    wire $s = $x ^ $y ^ $z;\n";
	    } else {
		$inst = temp_inst();
		print "    wire $s, $c;\n";
		print "    xtfa $inst($s, $c, $x, $y, $z);\n";
		$main::csahash{$key} = join($magic, $c, $s);
	    }
	    $main::xtfa++;
	}
    }
    return ($c, $s);
}

sub mul_array {
    my($array, $out) = @_;
    my($i, $done, $inst, @sum1, @sum2);

    do {
	my $newarray;
	do {
	    my(@so, @co);
	    $done = 1;
	    for($i = 0; $i < $out; $i++) {
		if (@{$array->[$i]} >= 3) {
		    my $x = shift(@{$array->[$i]});
		    my $y = shift(@{$array->[$i]});
		    my $z = shift(@{$array->[$i]});
		    my($c, $s) = csa($x, $y, $z, $i, $i == $out-1);
		    $so[$i] = $s;
		    $co[$i+1] = $c;
		    $done = 0;
		} elsif (@{$array->[$i]} >= 2 && ! $done) {
		    my $x = shift(@{$array->[$i]});
		    my $y = shift(@{$array->[$i]});
		    my($c, $s) = csa($x, $y, "1'd0", $i, $i == $out-1);
		    $so[$i] = $s;
		    $co[$i+1] = $c;
		}
	    }
	    for($i = 0; $i < $out; $i++) {
		push(@{$newarray->[$i]}, $so[$i]) if defined $so[$i];
	    }
	    for($i = 0; $i < $out; $i++) {
		push(@{$newarray->[$i]}, $co[$i]) if defined $co[$i];
	    }
	    $main::cnt++;
	} while (! $done);
	$main::cnt--;

	$done = 1;
	for($i = 0; $i < $out; $i++) {
	    push(@{$array->[$i]}, @{$newarray->[$i]}) if defined @{$newarray->[$i]};
	    $done = $done && @{$array->[$i]} <= 2;
	}
    } while (! $done);
}

sub mulmac {
    my($module, $n, $m, $o, $mac_flag, $pp_flag) = @_;
    my($done, $i, $array, $a, $b, $inst, $num, $width);
    my (@s, @ports, @sum, @sum1, @sum2);

    $o = $o || $n + $m;
    $width = $n+$m > $o ? $n+$m : $o;

    # module header
    push(@ports, "xtout", "a", "b");
    push(@ports, "c") if $mac_flag;
    push(@ports, "sign");
    push(@ports, "negate") if $mac_flag || $pp_flag;
    print "module $module(" . join(", ", @ports) . ");\n";
    print "output [" . ($width+$pp_flag*$width-1) . ":0] xtout;\n";
    print "input [" . ($n-1) . ":0] a;\n";
    print "input [" . ($m-1) . ":0] b;\n";
    print "input [" . ($o-1) . ":0] c;\n" if $mac_flag;
    print "input sign;\n";
    if ($mac_flag || $pp_flag) {
	print "input negate;\n";
    } elsif ($main::simple == 0) {
	print "wire negate = 1'b0;\n";
    }

    # rename a,b so that a is the larger operand
    ($a, $b, $n, $m) = $n < $m ? ("b", "a", $m, $n) : ("a", "b", $n, $m);

    if ($main::simple) {
      if ($pp_flag) {
	print "wire [" . ($width-1) . ":0] csa_input = negate ? {{" . ($width-2) . "{1'b0}},2'b10} : ${width}'b0;\n";
	print "wire [" . ($width+1) . ":0] partial0,partial1;\n";
	print "wire [" . ($width-1) . ":0] sum0,sum1,p0,p1;\n\n";
	print "  DW02_multp #(${m},${n}," . ($width+2) . ") pp1 (.a(${b}),.b(${a}),.tc(sign),.out0(partial0),.out1(partial1));\n";
	print "  assign p0 = negate ? ~partial0[" . ($width-1) . ":0] : partial0[" . ($width-1) . ":0];\n";
	print "  assign p1 = negate ? ~partial1[" . ($width-1) . ":0] : partial1[" . ($width-1) . ":0];\n";
	print "  DW02_tree #(3,$width) wall1 (.INPUT({p0,p1,csa_input}),.OUT0(sum0),.OUT1(sum1));\n\n";
	print "assign xtout = {sum0,sum1};\n";
      } elsif ($mac_flag) {
	print "wire [" . ($width-1) . ":0] mac_result;\n";
	if (($n + $m) > $o) {
	  print "wire [" . ($width-1) . ":0] extended_c = {{" . ($n+$m-$o) . "{sign & c[" . ($o-1) . "]}},c};\n";
	} else {
	  print "wire [" . ($width-1) . ":0] extended_c = c;\n";
	}
	print "wire [" . ($width-1) . ":0] negated_c = negate ? ~extended_c : extended_c;\n\n";
	print "  DW02_prod_sum1 #(${m},${n},$width) mac1 (.A(${b}),.B(${a}),.C(negated_c),.TC(sign),.SUM(mac_result));\n\n";
	print "assign xtout = negate ? ~mac_result : mac_result;\n";
      } else {
	print "\n   wire ${b}_msb, ${a}_msb;\n";
	print "   wire signed [${m}:0] ${b}_sgn;\n";
	print "   wire signed [${n}:0] ${a}_sgn;\n\n";
	print "   assign ${b}_msb = sign & ${b}[" . ($m-1) . "];\n";
	print "   assign ${a}_msb = sign & ${a}[" . ($n-1) . "];\n";
	print "   assign ${b}_sgn = {${b}_msb,${b}};\n";
	print "   assign ${a}_sgn = {${a}_msb,${a}};\n\n";
	print "   assign xtout = ${b}_sgn * ${a}_sgn;\n";
      }
    } else {
      $num = int($m/2) + 1;
      print "    wire [" . ($n+1) . ":0] " . join(", ", map("p$_", 0 .. $num-1)) . ";\n";
      print "    wire " . join(", ", map("cin$_", 0 .. $num-1)) . ";\n";
      print "    wire ${b}se = sign & $b\[" . ($m-1) . "];\n";

      # the Booth encoders
      for($i = 0; $i < $num; $i++) {
	$s[2] = 2*$i+1 >= $m ? "${b}se" :"${b}\[" . (2*$i+1) . "]";
	$s[1] = 2*$i >= $m ? "${b}se" : "${b}\[" . (2*$i+0) . "]";
	$s[0] = 2*$i-1 < 0 ? "1'd0" : "${b}\[" . (2*$i-1) . "]";

	print "    xtbooth #($n) booth$i(p$i, cin$i, $a, {$s[2],$s[1],$s[0]}, sign, negate);\n";
	map(push(@{$array->[2*$i+$_]}, "p${i}\[$_\]"), 0 .. $n+1);
	map(push(@{$array->[2*$i+$_]}, "p${i}\[" . ($n+1) . "]"), $n+2 .. ($width - 2*$i - 1));

	push(@{$array->[2*$i]}, "cin$i");
      }

      # drop any excess columns (I was lazy in the map's above) 
      for($i = $width; $i < @{$array}; $i++) {
	@{$array->[$i]} = ( );
      }

      # compress the array once down to 2 values in each column
      %main::csahash = ();
      $main::xtfa = $main::xtha = 0;
      $main::cnt = 0;

      mul_array($array, $width);

      # add the addend (c) and compress again
      if ($mac_flag) {
	print "    wire cse = sign & c[" . ($o-1) . "];\n";
	map(push(@{$array->[$_]}, "c[$_]"), 0 .. $o-1);
	map(push(@{$array->[$_]}, "cse"), $o .. $width);
	mul_array($array, $width);
      }

      # make sure each column has exactly two values
      for($i = $width-1; $i >= 0; $i--) {
	while (@{$array->[$i]} != 2) {
	    push(@{$array->[$i]}, "1'd0");
	}
	push(@sum1, $array->[$i]->[0]);
	push(@sum2, $array->[$i]->[1]);
      }

      # final summation
      $inst = temp_inst();
      print "    wire [" . ($width-1) . ":0] sum1 = {" . join(",", @sum1) . "};\n";
      print "    wire [" . ($width-1) . ":0] sum2 = {" . join(",", @sum2) . "};\n";
      if ($pp_flag) {
	print "    assign xtout = {sum1, sum2};\n";
      } else {
	print "    wire [" . ($width-1) . ":0] sum;\n";
	print "    xtadd #($width) $inst(sum, sum1, sum2);\n";
	print "    assign xtout = sum;\n";
      }
      print "    // stats: $main::xtfa full and $main::xtha half\n";
    }
    print "endmodule\n\n";
}


sub ref_mulmac {
    my($module, $n, $m, $o, $mac_flag, $pp_flag) = @_;
    my(@ports, $width);

    sub sext {
	my($name, $w, $s) = @_;
	if ($w > $s) {
	    return "{{" . ($w - $s) . "{sign&$name\[" . ($s-1) . "]}},$name}";
	} else {
	    return $name;
	}
    }

    $o = $o || $n + $m;
    $width = $n+$m > $o ? $n+$m : $o;

    # header
    push(@ports, "xtout", "a", "b");
    push(@ports, "c") if $mac_flag;
    push(@ports, "sign");
    push(@ports, "negate") if $mac_flag;
    print "module $module(" . join(", ", @ports) . ");\n";
    print "output [" . ($width+$pp_flag*$width-1) . ":0] xtout;\n";
    print "input [" . ($n-1) . ":0] a;\n";
    print "input [" . ($m-1) . ":0] b;\n";
    print "input [" . ($o-1) . ":0] c;\n" if $mac_flag;
    print "input sign;\n";
    print "input negate;\n" if $mac_flag;
    print "    wire [" . ($o-1) . ":0] c = {".$o."{1'b0}};\n" if ! $mac_flag;
    print "    wire negate = 0;\n" if ! $mac_flag;
    print "    wire [" . ($width+$pp_flag*$width-1) . ":0] xtout_tmp;\n";

    if ($main::multiply_verify) {
	print "    assign xtout_tmp = " . sext("a", $width, $n) . " ^ " . sext("b", $width, $m) . " ^ " . sext("c", $width, $o) . " ^ sign ^ negate;\n";
    } else {
	print "    wire neg = negate & sign;\n";

	print "    wire [" . ($n+$m-1) . ":0] tmp = " . sext("a", $n+$m, $n) . " * " . sext("b", $n+$m, $m) . ";\n";
	print "    wire [" . ($width-1) . ":0] tmp1 = " . sext("tmp", $width, $n+$m) . ";\n";
	print "    wire [" . ($width-1) . ":0] tmp2 = neg ? {$width\{1'd0}} - tmp1 : tmp1;\n";
	print "    assign xtout_tmp = tmp2 + " . sext("c", $width, $o) . ";\n";
    }
    if ($pp_flag) {
	print "assign xtout = {xtout_tmp, ${width}'b0};\n";
    } else {
	print "assign xtout = xtout_tmp;\n";
    }
    print "endmodule\n\n";
}

sub xtmac {
    my($module, $n, $m, $o) = @_;
    if ($main::multiply_verify) {
	ref_mulmac($module, $n, $m, $o, 1, 0);
    } else {
	mulmac($module, $n, $m, $o, 1, 0);
    }
    return "";
}

sub xtmul {
    my($module, $n, $m, $o) = @_;
    if ($main::multiply_verify) {
	ref_mulmac($module, $n, $m, $o, 0, 0);
    } else {
	mulmac($module, $n, $m, $o, 0, 0, 0);
    }
    return "";
}

sub xtmulpp {
    my($module, $n, $m, $o) = @_;
    if ($main::multiply_verify) {
	ref_mulmac($module, $n, $m, $o, 0, 1);
    } else {
	mulmac($module, $n, $m, $o, 0, 1);
    }
    return "";
}

sub ref_xtmac {
    my($module, $n, $m, $o) = @_;
    ref_mulmac($module, $n, $m, $o, 1, 0);
    return "";
}

sub ref_xtmul {
    my($module, $n, $m, $o) = @_;
    ref_mulmac($module, $n, $m, $o, 0, 0);
    return "";
}

sub xtaddn {
    my($module, $n, $w) = @_;
    my($i, $ow, $array, @sum1, @sum2);

    $ow = $w + ceil_log2($n);
    printf "module $module(xtout";
    for ($i = 0; $i < $n; $i++) {
	printf ", xtin%d", $i;
    }
    printf ");\n";
    printf "    output [%d:0] xtout;\n", $ow-1;
    for ($i = 0; $i < $n; $i++) {
        printf "    input [%d:0] xtin%d;\n", $w-1, $i;
    }

    if ($main::simple) {
      print "\n   assign xtout = ";
      for ($i = 0; $i < $n; $i++) {
        printf "xtin%d", $i;
        print " + " unless ($i == ($n-1));
      }
      print";\n";
    } else {
      for ($i = 0; $i < $n; $i++) {
	map(push(@{$array->[$_]}, "xtin$i\[$_\]"), 0 .. $w-1);
      }
      for ($i = 0; $i < $n; $i++) {
	map(push(@{$array->[$_]}, "1'b0"), $w .. $ow-1);
      }

      %main::csahash = ();
      $main::xtfa = $main::xtha = 0;
      $main::cnt = 0;
      mul_array($array, $ow);

      # make sure each column has exactly two values
      for($i = $ow-1; $i >= 0; $i--) {
        while (@{$array->[$i]} != 2) {
            push(@{$array->[$i]}, "1'd0");
        }
        push(@sum1, $array->[$i]->[0]);
        push(@sum2, $array->[$i]->[1]);
      }

      # final summation
      my $inst = temp_inst();
      print "    wire [" . ($ow-1) . ":0] sum1 = {" . join(",", @sum1) . "};\n";
      print "    wire [" . ($ow-1) . ":0] sum2 = {" . join(",", @sum2) . "};\n";
      print "    wire [" . ($ow-1) . ":0] sum;\n";
      print "    xtadd #($ow) $inst(sum, sum1, sum2);\n";
      print "    assign xtout = sum;\n";
      print "    // stats: $main::xtfa full and $main::xtha half\n";
    }
    print "endmodule\n\n";
}


sub xtdelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtflop #(size) $inst($do, $di, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtendelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, xten, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input xten, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtenflop #(size) $inst($do, $di, xten, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtscdelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, clrb, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input clrb;\n";
    print "input clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtscflop #(size) $inst($do, $di, clrb, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtascdelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, clrb, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input clrb;\n";
    print "input clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtascflop #(size) $inst($do, $di, clrb, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtssdelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, setb, xtsval, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input [size-1:0] xtsval;\n";
    print "input setb, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtssflop #(size) $inst($do, $di, setb, xtsval, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtassdelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, setb, xtsval, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input [size-1:0] xtsval;\n";
    print "input setb, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtassflop #(size) $inst($do, $di, setb, xtsval, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtscendelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, xten, clrb, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input xten, clrb, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtscenflop #(size) $inst($do, $di, xten, clrb, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtascendelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, xten, clrb, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input xten, clrb, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtascenflop #(size) $inst($do, $di, xten, clrb, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtflop";
}

sub xtssendelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, xten, setb, xtsval, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input [size-1:0] xtsval;\n";
    print "input xten, setb, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtssenflop #(size) $inst($do, $di, xten, setb, xtsval, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtssenflop";
}

sub xtassendelay {
    my($module, $n) = @_;
    my($i, $inst, $di, $do);

    print "module $module(xtout, xtin, xten, setb, xtsval, clk);\n";
    print "parameter size = 1; // vhdl generic\n";
    print "output [size-1:0] xtout;\n";
    print "input [size-1:0] xtin;\n";
    print "input [size-1:0] xtsval;\n";
    print "input xten, setb, clk;\n";
    $di = "xtin";
    if (! $main::verify) {
	for($i = 0; $i < $n; $i++) {
	    $inst = temp_inst();
	    $do = temp_wire();
	    print "    wire [size-1:0] $do;\n";
	    print "    xtassenflop #(size) $inst($do, $di, xten, setb, xtsval, clk);\n";
	    $di = $do;
	}
    }
    print "    assign xtout = $di;\n"; 
    print "endmodule\n\n";
    return "xtssenflop";
}



sub modgen {
    my($options) = @_;
    my($all, $list, $echo, $srcptr, @srcdirs, $module, $k, $v, $key, @files, $file);
    my(%table, @todo);

    # named varargs with default values; isn't Perl5 great?
    sub check_arg {
	return exists $_[0]->{$_[1]} ? delete $_[0]->{$_[1]} : $_[2];
    }
    $all = check_arg($options, "ALL", 0);
    $list = check_arg($options, "LIST", 0);
    $echo = check_arg($options, "ECHO", 0);
    $srcptr = check_arg($options, "SRCDIRS", undef);
    @srcdirs = @$srcptr if defined $srcptr;
    $file = check_arg($options, "FILES", undef);
    @files = @$file if defined $file;
    $main::ResetFlops = check_arg($options, "RESET_FLOPS", 0);
    $main::ResetFlops = 1; #Force to be 1
    $main::AsyncReset = check_arg($options, "ASYNC_RESET", 0);
    $main::UseFlipFlops = check_arg($options, "USE_FLIP_FLOPS", 1);
    $main::FPGABuild = check_arg($options, "FPGA_BUILD", 0);
    $main::UseGatedClocks = check_arg($options, "USE_GATED_CLOCKS", 1);
    $main::ClkGateFuncUnit = check_arg($options, "CLK_GATE_FUNC_UNIT", 1);
    $main::TestLatchesTransparent = check_arg($options, "TEST_LATCHES_TRANSPARENT", 0);
    $main::TestFullScan = check_arg($options, "TEST_FULL_SCAN", 0);
    $main::forceTMode = check_arg($options, "FORCE_TMODE", 0);
    $main::verify = check_arg($options, "VERIFY", 0);
    $main::multiply_verify = check_arg($options, "MULTIPLY_VERIFY", 0);
    $main::simple = check_arg($options, "SIMPLE", 0);

    # check for extraneous parameters
    while (($k, $v) = each %$options) {
	print STDERR "modgen: unknown argument '$k' = '$v'\n";
    }

    if (! $all) {
	%main::built = (
	    'xtmux2e', 1,
	    'xtmux3e', 1,
	    'xtmux4e', 1,
	);
    }

    %table = (
	'ref_xtmac_(\d+)_(\d+)_(\d+)', \&ref_xtmac,
	'ref_xtmul_(\d+)_(\d+)', \&ref_xtmul,
	'ref_xtmulpp_(\d+)_(\d+)', \&ref_xtmulpp,
	'ref_xtrotate_left(\d+)', \&ref_xtrotate_left,
	'ref_xtrotate_right(\d+)', \&ref_xtrotate_right,
	'ref_xtshift_left(\d+)', \&ref_xtshift_left,
	'ref_xtshift_right(\d+)', \&ref_xtshift_right,
	'xtdelay(\d+)', \&xtdelay,
	'xtendelay(\d+)', \&xtendelay,
	'xtscdelay(\d+)', \&xtscdelay,
	'xtascdelay(\d+)', \&xtascdelay,
	'xtssdelay(\d+)', \&xtssdelay,
	'xtassdelay(\d+)', \&xtassdelay,
	'xtscendelay(\d+)', \&xtscendelay,
	'xtascendelay(\d+)', \&xtascendelay,
	'xtssendelay(\d+)', \&xtssendelay,
	'xtassendelay(\d+)', \&xtassendelay,
	'xtmac_(\d+)_(\d+)_(\d+)', \&xtmac,
	'xtmul_(\d+)_(\d+)', \&xtmul,
	'xtmulpp_(\d+)_(\d+)', \&xtmulpp,
	'xtsel(\d+)', \&xtsel,
	'xtmux(\d+)', \&xtmux,
	'xtmux(\d+)e', \&xtmuxe,
	'xtmux(\d+)p', \&xtmuxp,
	'xtwregfile_(\d+)D(\d+)N_(\d+)R(\d+)W_(\d+)_(\d+)_XT\w+', \&xtwrf, 
	'xtwregfile_(\d+)D(\d+)N_(\d+)R(\d+)W_(\d+)_XT\w+', \&xtwregfile, 
	'xtregfile_(\d+)R(\d+)W_(\d+)_XT\w+', \&xtregfile,
	'xtregfile_(\d+)R(\d+)W_(\d+)_FF_XT\w+', \&xtregfile_FF,
	'xtregfile_(\d+)R(\d+)W_(\d+)_(\d+)_XT\w+', \&xtrf,
	'xtregfile_(\d+)R(\d+)W_(\d+)_(\d+)_FF_XT\w+', \&xtrf_FF,
	'xtrotate_left(\d+)', \&xtrotate_left,
	'xtrotate_right(\d+)', \&xtrotate_right,
	'xtshift_left(\d+)', \&xtshift_left,
	'xtshift_right(\d+)', \&xtshift_right,
	'xtaddn(\d+)_(\d+)', \&xtaddn,
	'xtlzc_(\d+)', \&xtlzc,
    );

    if ($list) {
	print "Available primitives are:\n";
	foreach $key (sort(keys(%table))) {
	    print "    $key\n";
	}
	return;
    }

    # inputs to this method (modgen) can come either exclusively from 
    # STDIN or exclusively from arguments.  if from arguments, they can 
    # come from the SRCDIRS flag, or from a trailing list of files, or 
    # from a combination thereof.  
    if (scalar(@srcdirs)) {
	foreach my $directory (@srcdirs) {
	    opendir(DIR, $directory) || confess "Can't open $directory: $!\n";
	    foreach $k (readdir(DIR)) {
		# ugly match below means .v or .sv files, excluding
		# of course primitives.v
		if ($k =~ m/\.s?v$/ && $k !~ m/primitives/) {
		    push(@files, "$directory/$k");
		}
	    }
	    closedir(DIR);
	}
	confess("$PROGRAM_NAME has no input files\n") if @files == 0;
    } else {
	push(@files, "-") if @files == 0;
    }

    while ($file = shift @files) {
	open(FILE, $file) || confess "Can't open $file: $!\n";
	while (<FILE>) {
	    print if $echo;
	    if (m/\s*module\s*((xt|ref_)\w+)/) {
		$module = $1;
		$main::built{$module} = 1;
	    } elsif (m/\b(xt|ref_)\w+/) {
		my @t = split(' ');
		push(@todo, @t);
	    }
	}
	close FILE;
    }

    # for modgen.pl run standalone for tc
    modgen_prim();

    while (@todo > 0) {
	my @next_todo;
	foreach $module (sort(&uniq(@todo))) {
	    foreach $key (keys(%table)) {
		if ($module =~ m/^$key$/) {
		    if (! $main::built{$module}) {
			$main::built{$module} = 1;
			$main::temp_inst = $main::temp_wire = 0;
			push(@next_todo, &{$table{$key}}($module, $1, $2, $3, $4, $5));
		    }
		    last;
		}
	    }
	}
	@todo = @next_todo;
    }
}

1;

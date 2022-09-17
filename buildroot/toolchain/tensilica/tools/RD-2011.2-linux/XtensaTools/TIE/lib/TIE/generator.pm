

# Copyright (c) 2002 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

package generator;
use Exporter ();
@ISA = qw(Exporter);
@EXPORT = qw(regfile generator_help);

use strict;

BEGIN {
    %regfile::declared_interface = ();
}

sub all_help {
print <<EOF;
    regfile -- regfile generator
EOF
}

sub regfile_help {
print <<EOF;

regfile description:
    The regfile generator creates all TIE constructs for a simple
    register file.  A register file is simple if it
       - is not more than 128-bit wide
       - has 16 entries
       - can be accessed through s, t, and r instruction fields
       - load/store instructions do not modify the data
    When these restrictions are met, this regfile generate will
    automatically generate the following TIE constructions:
       - register file
       - immediate operand used as offset for load/store instructions
       - register operands using s, t, and r instruction fields
       - immediate load/store instructions with update
       - indexed load/store instructions with update
       - move instructions
       - reference descriptions for generated instructions
       - implementation descriptions for generated instructions
       - prototype definitions to enable register allocation by the compiler
    In the case where your register file is more complex, the output
    output of the generator can be used as the basis for further
    enhancement.
regfile usage:
    ; use generator;
    ; generator::regfile(pname => pvalue, ...);
regfile parameters:
    name => "string"  - regfile name, required
    width => num      - regfile bit-width, required, < 128
    sname => "string" - regfile shortname, optional, default is regfile name
    cname => "string" - ctype name, optional, default is regfile name
    iopcode => num    - starting opcode for immediate load/store instructions,
			optional, default is 0, must be in [0-12]
    xopcode => num    - starting opcode for indexed load/store instructions,
			optional, must be in [0-12].  If not specified, no 
			indexed load/store instructions will be generated.
    mopcode => num    - opcode for move instruction in CUST0 group,
			optional, default is 0, must be in [0-15].
EOF
}

sub generator_help {
    my($key) = @_;

    if ($key eq "") {
	all_help();
    } elsif ($key eq "regfile") {
	regfile_help();
    } else {
	die "Unknown help key: $key";
    }
}

sub regfile {
    my(%params) = @_;

    my ($name, $width, $sname, $cname, $iopcode, $xopcode, $mopcode, $lswidth, $depth);

    $name = $params{"name"} if (defined $params{"name"});
    $width = $params{"width"} if (defined $params{"width"});
    $sname = $params{"sname"} if (defined $params{"sname"});
    $cname = $params{"cname"} if (defined $params{"cname"});
    $iopcode = $params{"iopcode"} if (defined $params{"iopcode"});
    $xopcode = $params{"xopcode"} if (defined $params{"xopcode"});
    $mopcode = $params{"mopcode"} if (defined $params{"mopcode"});
    $lswidth = &compute_lswidth($width);
    $depth = 16;

    # set defaults for required parameters
    die "must specify name" if (! defined $name); 
    die "must specify width" if (! defined $width); 
    $iopcode = 0 if (! defined $iopcode);
    $mopcode = 0 if (! defined $mopcode);
    $sname = $name if (! defined $sname); 
    $cname = $name if (! defined $cname); 

    &check_params($name, $width, $depth, $iopcode, $xopcode, $mopcode);
    &create_interface($lswidth);
    &create_regfile($name, $width, $depth, $sname);
    &create_opcodes($name, $iopcode, $xopcode, $mopcode);
    &create_immediates($name, $width, $lswidth);
    &create_operands($name, $sname, $depth);
    &create_iclasses($name, $sname, $width, $lswidth, $xopcode, $mopcode);
    &create_references($name, $sname, $width, $lswidth, $xopcode, $mopcode);
    &create_semantics($name, $sname, $width, $lswidth, $xopcode, $mopcode);
    &create_ctypes($name, $cname, $width, $lswidth);
    &create_protos($name, $cname);
    &create_schedules($name, $sname, $xopcode);
}

sub check_params {
    my($name, $width, $depth, $iopcode, $xopcode, $mopcode) = @_;

    die "register $name is too wide" if ($width > 128);
    die "register $name must have 16 entries" if ($depth != 16);
    if ($iopcode < 0 || $iopcode > 12) {
	die "iopcode must be in range [0-12]";
    }
    if ((defined $xopcode) && ($xopcode < 0 || $xopcode > 12)) {
	die "xopcode must be in range [0-12]";
    }
    if ((defined $mopcode) && ($mopcode < 0 || $mopcode > 15)) {
	die "mopcode must be in range [0-15]";
    }
}

sub create_interface {
    my($lswidth) = @_;

    if (defined $regfile::interface_declared{$lswidth}) {
	return;
    }

    if (not defined $regfile::interface_declared{"Basic"}) {
print <<EOF;

// interface signals for all load/store instructions
interface       VAddr       	32      core    out
EOF
    $regfile::interface_declared{"Basic"} = 1;
    }

print <<EOF;

// interface signals for $lswidth bit load/store instructions
interface       MemDataOut$lswidth   $lswidth     core    out
interface       MemDataIn$lswidth    $lswidth     core    in
EOF
    $regfile::interface_declared{$lswidth} = 1;
}

sub create_regfile {
    my($name, $width, $depth, $sname) = @_;

print <<EOF;

// register file ${name}
regfile $name $width $depth $sname
EOF
}

sub create_opcodes {
    my($name, $iopcode, $xopcode, $mopcode) = @_;
    my $iop0 = $iopcode;
    my $iop1 = $iopcode + 1;
    my $iop2 = $iopcode + 2;
    my $iop3 = $iopcode + 3;

print <<EOF;

// immediate load/store opcodes for regfile $name
opcode ${name}_LI  r=$iop0 LSCI
opcode ${name}_SI  r=$iop1 LSCI
opcode ${name}_LIU r=$iop2 LSCI
opcode ${name}_SIU r=$iop3 LSCI
EOF

if (defined $xopcode) {
    my $xop0 = $xopcode;
    my $xop1 = $xopcode + 1;
    my $xop2 = $xopcode + 2;
    my $xop3 = $xopcode + 3;
print <<EOF;

opcode ${name}_LX  op2=$iop0 LSCX
opcode ${name}_SX  op2=$iop1 LSCX
opcode ${name}_LXU op2=$iop2 LSCX
opcode ${name}_SXU op2=$iop3 LSCX
EOF
}

if (defined $mopcode) {
print <<EOF;

opcode ${name}_MOVE  op2=$mopcode CUST0
EOF
}
}

sub create_immediates {
    my($name, $width, $lswidth) = @_;
    my $bytes = $lswidth / 8;
    my $num = int log($bytes) / log(2);
    my $sign = 32 - 8 - $num;
    my $findex = $num + 7;
    my $tindex = $num;

if ($num > 0) {
print <<EOF;

// offset used by $name load/store instructions
operand ${name}_LSO imm8 {{{$sign\{imm8[7]}},imm8[7:0],$num\'b0}} {${name}_LSO[$findex:$tindex]}
EOF
} else {
print <<EOF;

// offset used by $name load/store instructions
operand ${name}_LSO imm8 {{{$sign\{imm8[7]}},imm8[7:0]}} {${name}_LSO[$findex:$tindex]}
EOF
}
}

sub create_operands {
    my($name, $sname, $depth) = @_;

print <<EOF;

// operands used to access register file $name
operand ${sname}r r {${name}\[r]}
operand ${sname}s s {${name}\[s]}
operand ${sname}t t {${name}\[t]}
EOF
}

sub create_iclasses {
    my($name, $sname, $width, $lswidth, $xopcode, $mopcode) = @_;

print <<EOF;

// iclass definition for $name immediate load/store instructions
iclass ${name}_li {${name}_LI} {out ${sname}t, in ars, in ${name}_LSO} {} {
    out VAddr, in MemDataIn$lswidth
}
iclass ${name}_liu {${name}_LIU} {out ${sname}t, inout ars, in ${name}_LSO} {} {
    out VAddr, in MemDataIn$lswidth
}
iclass ${name}_si {${name}_SI} {in ${sname}t, in ars, in ${name}_LSO} {} {
    out VAddr, out MemDataOut$lswidth
}
iclass ${name}_siu {${name}_SIU} {in ${sname}t, inout ars, in ${name}_LSO} {} {
    out VAddr, out MemDataOut$lswidth
}
EOF

if (defined $xopcode) {
print <<EOF;

// iclass definition for $name indexed load/store instructions
iclass ${name}_lx {${name}_LX} {out ${sname}r, in ars, in art} {} {
    out VAddr, in MemDataIn$lswidth
}
iclass ${name}_lxu {${name}_LXU} {out ${sname}r, inout ars, in art} {} {
    out VAddr, in MemDataIn$lswidth
}
iclass ${name}_sx {${name}_SX} {in ${sname}r, in ars, in art} {} {
    out VAddr, out MemDataOut$lswidth
}
iclass ${name}_sxu {${name}_SXU} {in ${sname}r, inout ars, in art} {} {
    out VAddr, out MemDataOut$lswidth
}
EOF
}

if (defined $mopcode) {
print <<EOF;

// iclass definition for $name register-to-register move instruction
iclass ${name}_move {${name}_MOVE} {out ${sname}r, in ${sname}t} {} {}
EOF
}
}

sub create_references {
    my($name, $sname, $width, $lswidth, $xopcode, $mopcode) = @_;
    my $bytes = int $lswidth / 8;

print <<EOF;

// reference definitions for $name load/store instructions
reference ${name}_LI
{
    assign VAddr = ars + ${name}_LSO;
    assign ${sname}t = MemDataIn$lswidth;
}
reference ${name}_SI
{
    assign VAddr = ars + ${name}_LSO;
    assign MemDataOut$lswidth = ${sname}t;
}
reference ${name}_LIU
{
    wire [31:0] address = ars + ${name}_LSO;
    assign VAddr = address;
    assign ${sname}t = MemDataIn$lswidth;
    assign ars = address;
}
reference ${name}_SIU
{
    wire [31:0] address = ars + ${name}_LSO;
    assign VAddr = address;
    assign MemDataOut$lswidth = ${sname}t;
    assign ars = address;
}
EOF

if (defined $xopcode) {
print <<EOF;

// reference definitions for $name indexed load/store instructions
reference ${name}_LX
{
    assign VAddr = ars + art;
    assign ${sname}r = MemDataIn${lswidth};
}
reference ${name}_SX
{
    assign VAddr = ars + art;
    assign MemDataOut$lswidth = ${sname}r;
}
reference ${name}_LXU
{
    wire [31:0] address = ars + art;
    assign VAddr = address;
    assign ${sname}r = MemDataIn${lswidth};
    assign ars = address;
}
reference ${name}_SXU
{
    wire [31:0] address = ars + art;
    assign VAddr = address;
    assign MemDataOut$lswidth = ${sname}r;
    assign ars = address;
}
EOF
}

if (defined $mopcode) {
print <<EOF;

reference ${name}_MOVE 
{
    assign ${sname}r = ${sname}t;
}
EOF
}

}

sub create_semantics {
    my($name, $sname, $width, $lswidth, $xopcode, $mopcode) = @_;

if (defined $xopcode) {
print <<EOF;

// semantic description of all load/store instructions for $name
semantic ${name}_ls {
    ${name}_LI, ${name}_LIU, ${name}_SI, ${name}_SIU,
    ${name}_LX, ${name}_LXU, ${name}_SX, ${name}_SXU
} {
    wire immediate = ${name}_LI | ${name}_LIU | ${name}_SI | ${name}_SIU;
    wire [31:0] address = ars + (immediate ? ${name}_LSO : art);
    assign VAddr = address;
    assign ${sname}t = MemDataIn${lswidth}; // effect only for immediate loads
    assign ${sname}r = MemDataIn${lswidth}; // effect only for indexed loads
    assign MemDataOut${lswidth} = immediate ? ${sname}t : ${sname}r; // effect only stores
    assign ars = address; // effect only update load/store
}
EOF

} else {
print <<EOF;

// semantic description of all load/store instructions for $name
semantic ${name}_ls {${name}_LI, ${name}_LIU, ${name}_SI, ${name}_SIU} {
    wire [31:0] address = ars + ${name}_LSO;
    assign VAddr = address;
    assign ${sname}t = MemDataIn${lswidth}; // effect only for loads
    assign MemDataOut${lswidth} = ${sname}t; // effect only stores
    assign ars = address; // effect only update load/store
}
EOF
}

if (defined $mopcode) {
print <<EOF;

semantic ${name}_move {${name}_MOVE} {
    assign ${sname}r = ${sname}t;
}
EOF
}
}

sub create_ctypes {
    my($name, $cname, $width, $lswidth) = @_;

print <<EOF;

// C-type for data in regfile $name
ctype $cname $lswidth $lswidth $name
EOF
}

sub create_protos {
    my($name, $cname) = @_;

print <<EOF;

// prototype for register file $name load/store instructions
proto ${name}_LI {out ${cname} v, in ${cname} *p, in immediate o} {} {${name}_LI v, p, o;}
proto ${name}_SI {in ${cname} v, in ${cname} *p, in immediate o} {} {${name}_SI v, p, o;}
proto ${name}_LIU {out ${cname} v, inout ${cname} *p, in immediate o} {} {${name}_LIU v, p, o;}
proto ${name}_SIU {in ${cname} v, inout ${cname} *p, in immediate o} {} {${name}_SIU v, p, o;}

// prototypes to enable register allocation by the compiler
proto ${cname}_loadi {out $cname v, in $cname *p, in immediate o} {} {${name}_LI v, p, o;}
proto ${cname}_storei {in $cname v, in $cname *p, in immediate o} {} {${name}_SI v, p, o;}
proto ${cname}_loadiu {out $cname v, inout $cname *p, in immediate o} {} {${name}_LIU v, p, o;}
proto ${cname}_storeiu {in $cname v, inout $cname *p, in immediate o} {} {${name}_SIU v, p, o;}
proto ${cname}_move {out $cname r, in $cname t} {} {${name}_MOVE r, t;}
EOF
}

sub create_schedules {
    my($name, $sname, $xopcode) = @_;
    my $def = $::DataMemoryLatency + 1;

print <<EOF;

// load data is avaiable in second (M) stage
schedule ${name}_loadi_schedule {${name}_LI} { 
    def ${sname}t $def;
}
schedule ${name}_loadiu_schedule {${name}_LIU} { 
    def ${sname}t $def;
}
EOF

if (defined $xopcode) {
print <<EOF;
schedule ${name}_loadx_schedule {${name}_LX} { def ${sname}r $def; }
schedule ${name}_loadxi_schedule {${name}_LXU } { def ${sname}r $def; }
EOF
}
}

sub compute_lswidth {
    my($x) = @_;

    return 1024 if $x > 512;
    return 512 if $x > 256;
    return 256 if $x > 128;
    return 128 if $x > 64;
    return 64 if $x > 32;
    return 32 if $x > 16;
    return 16 if $x > 8;
    return 8;
}


# Local Variables:
# mode:perl
# perl-indent-level:4
# cperl-indent-level:4
# End:

#!/usr/xtensa/stools-6.1/bin/perl -w

# Copyright (c) 1999-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
#   These coded instructions, statements, and computer programs are the
#   copyrighted works and confidential proprietary information of Tensilica Inc.
#   They may not be modified, copied, reproduced, distributed, or disclosed to
#   third parties in any manner, medium, or form, in whole or in part, without
#   the prior written consent of Tensilica Inc.


use File::Temp qw/ tempfile tempdir /;
use Getopt::Long;
use strict;

#parse the options
my $outfilename;
my $symname;
my $verbose = 0;
my $pagesize;
my $symfile;
my $symlookup;
my $saveStrippedName = "";
my %optctl = (
	      "o=s" => \$outfilename,
	      "s=s" => \$saveStrippedName,
	      "e=s" => \$symname,
	      "v" => \$verbose,
	      "p:i" => \$pagesize,
	      "l" => \$symlookup
);

GetOptions(%optctl)
  || die "No output file specified.";
defined $outfilename 
  || die "No output file specified";
defined $symname 
  || die "No library symbol specified";

if (defined $pagesize && $pagesize == 0) {
  $pagesize = 128;
}

if (@ARGV != 1) {
  die "Extra arguments.";
}

#setup some temp files
my $tempfh;
my $emptytemp;
($tempfh, $emptytemp) = tempfile();
my $flattenedlib;
($tempfh, $flattenedlib) = tempfile();
my $strippedlib;
($tempfh, $strippedlib) = tempfile();
my $pagedlib;
($tempfh, $pagedlib) = tempfile();

#do the work
my $inputlib = $ARGV[0];

#strip out all nonloadable sections
!$verbose || print "Removing nonloadable sections...\n";
if (defined $pagesize) {
  !system("xt-strip", "--shared-pagesize=" . $pagesize, $inputlib, "-o", $strippedlib)
  || die "couldn't strip input file.";
}
else {
  !system("xt-strip", $inputlib, "-o", $strippedlib)
  || die "couldn't strip input file.";
}

#strip out empty sections and special xtensa sections that shouldn't go on the target
open OBJDUMP_OUTPUT, "xt-objdump -h " . $strippedlib . " |"
  || die "couldn't open stripped lib";

my @emptysections = ("--remove-section=.xtensa.info",
		     "--remove-section=.xt.lit",
		     "--remove-section=.xt.prop",
		     "--remove-section=.interp",
		     "--remove-section=.got.loc",
		     "--remove-section=.got",
		    );

if (!defined $symlookup) {
  push (@emptysections, "--remove-section=.hash");
  push (@emptysections, "--remove-section=.dynstr");
  push (@emptysections, "--remove-section=.dynsym");
  push (@emptysections, "--remove-section=.rela.plt");
}

while (<OBJDUMP_OUTPUT>) {
  my @entries = split ;
  if (defined $entries[2] && $entries[2] eq "00000000") {
    push @emptysections, "--remove-section=" . $entries[1];
  }
}

if (defined $pagesize) {
  !system ("xt-objcopy",  "--shared-pagesize=" . $pagesize, $strippedlib, $pagedlib, @emptysections)
  || die "couldn't remove empty sections";
}
else {
  !system ("xt-objcopy", $strippedlib, $pagedlib, @emptysections)
    || die "couldn't remove empty sections";
}

if ($saveStrippedName ne "") {
   !system ("xt-objcopy", $pagedlib, $saveStrippedName)
   || die "couldn't copy to saved stripped file";
}

#build an empty file quickly, $flattenedlib lib is an empty file
!$verbose || print "Building empty object file...\n";
!system ("xt-as",  "-o", $emptytemp, $flattenedlib)
  || die "couldn't build empty object file.";

!$verbose || print "Copying library...\n";
!system("xt-objcopy", $emptytemp, $flattenedlib,
	"--add-section", "libdata=" . $pagedlib,
	"--set-section-flags", "libdata=readonly,alloc")
  || die "couldn't convert pie file to relocatable object.";


#check for exported symbols
my %definedSyms = ();

# only do this for overlays. PIE's have their own
# symbol exporting mechanism
if (!defined $pagesize && defined $symfile) {
  open NM_OUTPUT, "xt-nm " . $inputlib . " |"
    || die "couldn't open stripped lib";
  my $symname;
  my $symval;
  my $symtype;

  while( <NM_OUTPUT> ) {

    ( $symval, $symtype, $symname ) = split ;

    $definedSyms{ $symname } = $symval;
  }

  close NM_OUTPUT;
}

my $asfile;
my $asfileh;
!$verbose || print "Writing linker script...\n";
($asfileh, $asfile) = tempfile();
printf($asfileh ".section libdata, \"a\"\n.align 4\n");
my $ofile;
my $ofileh;
($ofile, $ofileh) = tempfile();
!system("xt-as", "-o", $ofile, $asfile)
  || die "couldn't assemlbe aligned object file";

my $linkerscript;
my $scriptfile;
!$verbose || print "Writing linker script...\n";
($scriptfile, $linkerscript) = tempfile();

if (!defined $pagesize && defined $symfile) {
  my $symfileh;
  open $symfileh, $symfile || die "couldn't open symbol file: " . $symfile;

  while (<$symfileh>) {
    chomp;
    my $symval;
    if (defined $definedSyms{$_}) {
      $symval = $definedSyms{$_};
      printf($scriptfile "$_ = 0x$symval;\n");
    }
    else {
      die ("symbol \"$_\" was not defined in the library.");
    }
  }
}

printf($scriptfile
	"SECTIONS { .rodata : { "
	. $symname . " = . ; *(libdata) }}\n");

!$verbose || print "Linking into relocatable object...\n";
!system("xt-ld",  "-r", "-T",  $linkerscript,
	$ofile, $flattenedlib, "-o", $outfilename)
  || die "couldn't add section symbol.";

unlink $linkerscript;
unlink $flattenedlib;
unlink $emptytemp;
unlink $strippedlib;
unlink $pagedlib;
unlink $ofile;
unlink $asfile;

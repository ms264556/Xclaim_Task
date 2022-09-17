#  Main Program for Tensilica Preprocessor.
#
#  Copyright (c) 2005-2010 Tensilica Inc.  ALL RIGHTS RESERVED.
#  These coded instructions, statements, and computer programs are the
#  copyrighted works and confidential proprietary information of Tensilica Inc.
#  They may not be modified, copied, reproduced, distributed, or disclosed to
#  third parties in any manner, medium, or form, in whole or in part, without
#  the prior written consent of Tensilica Inc.


package tpp;
use strict;
#no warnings;
use Exporter ();
@tpp::ISA = qw(Exporter);
@tpp::EXPORT = qw(include gen);
use vars qw( @incdirs );


sub gen {
    print STDOUT (@_);
}

sub is_absolute_path {
  my ($path) = @_;
  #  Treat anything with a drive letter as absolute, even if not followed by a (back)slash.
  return 1 if $path =~ /^\w:/ and $^O =~ /MSWin32|cygwin/;
  return 1 if $path =~ m|^[/\\]|;
  return 0;
}


sub dirpath {
  my ($fname) = @_;

  #  Special case: If the file is in the root directory, use "/." for
  #  the directory name.  For Windows, if there is a drive letter in the
  #  path, use either "." or "/." after the drive letter, depending on
  #  whether it is a drive-relative path.  */

  my $drive = "";
  if ($^O =~ /MSWin32|cygwin/) {
    $drive = $1 if $fname =~ s/^([a-z]:)//i;
  }
defined($drive) or die "drive undefined ($fname)";
defined($fname) or die "fname undefined";
  $fname =~ s|[^/\\]*$||;		# remove leaf (filename) part
  $fname =~ s|\\|/|g;
  #$fname =~ s|/+|/|g;
  if ($fname eq "") {
    $fname = ".";
  } elsif ($fname eq "/") {
    $fname = "/.";
  } else {
    $fname =~ s|/$||;
  }
  return $drive.$fname;
}


sub quotem {
  my ($out) = @_;
  $out =~ s/([\"\$\@\%\\\`])/\\$1/g;		# protect quoted contents
  $out =~ s/\t/\\t/g;				# quote tabs
  $out =~ s/([\x00-\x1f])/"\\x".sprintf("%02x",ord($1))/ge;	# quote control chars
  return $out;
}


sub include {
  my ($fname) = @_;

  my $path = $fname;
  SEARCH:{
    if (!is_absolute_path ($fname)) {
      #  Search the include directories....
      foreach my $dir (@incdirs) {
	$path = $dir . "/" . $fname;
	last SEARCH if -e $path;
      }
      die "Error: could not find \"$fname\" in include path";
    }
  }
  open(IN, "<$path")
    or die "Error: could not open file \"$path\": $!";

print STDERR "****************  tpp processing '$path'\n";
  push @incdirs, dirpath ($path);
  my $line;
  my $linenum = 0;
  my $code = "#line 1 \"$path\"\n";

  while (defined($line = <IN>)) {
    $linenum ++;
#print STDERR "+++ line $linenum: $line";

    #  Check for lines beginning with ';' but not ';;'.
    #  Unlike the usual TPP, treat lines with only ';' or beginning with '; //' as code too.
    if ($line =~ s/^\s*;([^;]|$)/$1/) {
      $code .= $line;
      next;
    }
    chomp($line);

    #  Don't strip out POD.  (Why bother?)

    $code .= "print STDOUT ";

    my $out = "";
    while ($line =~ s/^([^`]*)`//) {		# everything before the next backtick...
      $out .= $1;				# ... is normal output
      if ($line =~ /^\w/) {
	$out .= "`";				# `\w  is part of output, not a perl expr
	next;
      }
      if ($line =~ s/^([^`]*)`//) {		# if we find matching backtick
	$code .= '"'.quotem($out)."\" . ($1) . ";
	$out = "";
      } else {
	die "Error: $path line $linenum: unterminated embedded perl expression: `$line";
      }
    }
    $code .= '"'.quotem($out.$line)."\\n\";\n";
  }
  close(IN);

  #print STDERR ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  EXECUTING:\n$code\n<<<<<<<<<<<<<<<<<<<<<<<<<\n";
  tppcode::execute($code);
  if ($@) {
    chomp($@);
    print STDERR "tpp: Error ($@) while preprocessing file \"$path\"\n";
    close STDOUT;
    exit 1;
  }

  pop @incdirs;
}


####################   Parse and process arguments.

my ($sconfig, $xconfig, $module, $output_file, $arg);
my (@input_files, @pre_eval_exprs, @post_eval_exprs);
@incdirs = (".");

my $xtensa_tools_root = $0;
$xtensa_tools_root =~ s|[/\\]libexec[/\\][^/\\]+$||
  or die "tpp: path to self not located in <XtensaTools>/libexec : '$0'";
#$xtensa_tools_root .= "\\libexec/..";	# to look like tpp.exe for now
$ENV{"XTENSA_TOOLS_ROOT"} = $xtensa_tools_root;
#print STDERR "****** arg0 is '$0'\n";

while (defined($_ = shift)) {
  if (/^-(?:I|o|s|x|m|e|eval|p|peval)$/) {
    defined($arg = shift) or die "tpp: missing argument for '$_' option";
    /^-I/ and push @incdirs, $arg;	# -I dir: Search for include files in directory dir.
    /^-o/ and $output_file = $arg;	# -o file: Redirect the output to file rather than stdout.
    /^-s/ and $sconfig = $arg;		# -s file: Load the system config from SLD perl file into \$sys.
    /^-x/ and $xconfig = $arg;		# -x file: Load the system config from XML file into \$sys.
    /^-m/ and $module = $arg;		# -m module: Specify module to instantiate.
    /^-e/ and push @pre_eval_exprs, $arg;	# -eval exp: Eval exp after loading config.
    /^-p/ and push @post_eval_exprs, $arg;	# -peval exp: Eval exp after processing the input file.
    next;
  }
  if (/^-/) {
    die "tpp: unrecognized option '$_'";
  }
  push @input_files, $_;
}

@input_files or die "tpp: no input files";
$xconfig and $sconfig and die "tpp: cannot specify both -s and -x";
$sconfig and !$module and die "tpp: config file specified without a component module";

if ($output_file) {
  open(STDOUT, ">$output_file")
    or die "cannot open output file '$output_file': $!";
}

push @INC, @incdirs;

print STDERR "tpp: initializing...\n";

# Read the Perl system description.
if ($sconfig) {
  tppcode::sinit($sconfig, $module);
} elsif ($xconfig) {
  tppcode::xinit($xconfig, $module);
}

foreach (@pre_eval_exprs) {
  print STDERR "tpp: preprocessing expression \"$_\"\n";
  tppcode::execute($_);
  if ($@) {
    chomp($@);
    print STDERR "tpp: Error ($@) while preprocessing expression \"$_\"\n";
    exit 1;
  }
}

foreach (@input_files) {
  #print STDERR "******************  tpp.pl '$_'\n";
  tpp::include($_);
}

foreach (@post_eval_exprs) {
  #print STDERR "tpp: postprocessing expression \"$_\"\n";
  tppcode::execute($_);
  if ($@) {
    chomp($@);
    print STDERR "tpp: Error ($@) while preprocessing expression \"$_\"\n";
    exit 1;
  }
}

exit 0;

1;



package tppcode;
#no warnings;
no strict;		# sigh

use vars qw($pr $sys $mod $dict $sysdict);

sub addinc {
    push(@INC, @_);
}

sub execute {
    my ($code) = @_;
    eval ($code);
    die $@ if $@;
}

sub sinit {
    my ($sconfig, $module) = @_;
    require SLD::System;
    require $sconfig;
    GenSystem->import();
    $sys = new SLD::System(undef);
    my $exportedVars = createSystem($sys, $module);
    $pr = $sys->componentInstances($module);
    eval($exportedVars);
    die $@ if $@;
    undef $exportedVars;
}

sub readfile {
    my ($filename) = @_;
    my $file;
    open(IN,"<$filename") or die "could not open '$filename' for reading: $!";
    defined(sysread(IN,$file,-s IN)) or die "could not read '$filename': $!";
    close(IN);
    return $file;
}

sub xinit {
    my ($xconfig, $module) = @_;
    my $socxml = readfile($xconfig);
    my $root = $ENV{'XTENSA_TOOLS_ROOT'};
    my $typefile = $root.'/src/system/systypes.xsysi';
    my $types = readfile($typefile);
    # use Stash;	# this would happen compile-time even if xinit not called
    require Stash; import Stash;	# this happens run-time only
    $sysdict = stash_dict_new('system types');
    stash_xml_read($sysdict, $types, $typefile, 'sysdoc');
    $dict = stash_dict_new('system');
    stash_dict_import($dict, $sysdict);
    my @sysobjs = stash_xml_read($dict, $socxml, $xconfig, 'sysdoc');
    # FIXME: should we look for specific top-level system name instead?:
    @sysobjs == 1
      or die "expected exactly one top-level object in $xconfig, found ".scalar(@sysobjs);
    ($sys) = @sysobjs;
    # FIXME: check that $sys is valid?
    my $err;
    ($mod,$err) = stash_lookup($dict, $module);
    die "could not lookup module '$module': $err" if defined($err);
}

1;



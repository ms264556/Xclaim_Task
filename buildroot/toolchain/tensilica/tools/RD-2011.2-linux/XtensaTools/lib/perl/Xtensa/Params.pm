package Xtensa::Params;
use Exporter ();
@ISA = qw(Exporter);
@EXPORT = qw(xtensa_params_init);
@EXPORT_OK = @EXPORT;
%EXPORT_TAGS = ();
use strict;
sub xtensa_params_init {
  my ($xtensa_system, $xtensa_core, $xtensa_params) = @_;
  $xtensa_system = $ENV{"XTENSA_SYSTEM"} unless defined($xtensa_system);
  $xtensa_system = [split(/;/, $xtensa_system)] unless ref($xtensa_system) eq "ARRAY";
  my @xtensa_systems = grep($_ ne "", @$xtensa_system);	
  map(s|[\\/]+$||, @xtensa_systems);			
  die "ERROR: Params.pm xtensa_params_init: XTENSA_SYSTEM not defined"
	unless @xtensa_systems;
  $xtensa_core = $ENV{"XTENSA_CORE"} unless defined($xtensa_core) and $xtensa_core ne "";
  $xtensa_core = "default" unless defined($xtensa_core) and $xtensa_core ne "";
  $xtensa_params = $ENV{"XTENSA_PARAMS"} unless defined($xtensa_params);
  $xtensa_params = [] unless defined($xtensa_params);
  $xtensa_params = [split(/;/, $xtensa_params)] unless ref($xtensa_params) eq "ARRAY";
  my $parmfile;
  foreach my $path (@xtensa_systems) {
    $parmfile = $path."/".$xtensa_core."-params";
    last if open(IN, "<$parmfile");
    undef $parmfile;
  }
  die "ERROR: Params.pm xtensa_params_init: could not find core $xtensa_core (file ${xtensa_core}-params) in:\n"
		.join("", map("   $_\n", @xtensa_systems))."Stopped"
	unless defined($parmfile);
  my %parms = ();
  my $line = 0;
  while (<IN>) {
    $line++;
    s/^\s+//;		
    if (!s/^([-a-zA-Z0-9_\.]+)//) {
      print STDERR "WARNING: Params.pm xtensa_params_init: Unexpected char in line $line of $parmfile: $_\n" if /^[^\#]/;
      next;
    }
    my $key = $1;
    if (!s/^\s*=//) {
      print STDERR "WARNING: Params.pm xtensa_params_init: missing '=' in line $line of $parmfile\n";
      next;
    }
    if (s/^\s*\[//) {
      my @values = ();
      while (!s/^\s*\]//) {
	my $value = parseParamsValue(\$_);
	if (defined($value)) {
	  push @values, $value;
	} elsif (/^[^\#]/) {
	  print STDERR "WARNING: Params.pm xtensa_params_init: unexpected char in line $line of $parmfile: $_\n";
	  last;
	} else {
	  $line++;
	  if (!defined($_ = <IN>)) {
	    print STDERR "WARNING: Params.pm xtensa_params_init: unexpected end of file within array of key '$key' in file $parmfile\n";
	    last;
	  }
	}
      }
      $parms{$key} = \@values;
    } else {
      my $value = parseParamsValue(\$_);
      $value = "" unless defined($value);
      $parms{$key} = $value;
    }
    s/^\s+//;		
    if (/^[^\#]/) {
      print STDERR "WARNING: Params.pm xtensa_params_init: extraneous characters at end of line $line of $parmfile: $_\n";
    }
  }
  close(IN);
  return %parms;
}
sub parseParamsValue {
  my ($ref) = @_;
  $$ref =~ s/^\s+//;
  return $1 if $$ref =~ s/^'([^']*)'//;
  return $1 if $$ref =~ s/^"([^"]*)"//;
  return undef unless $$ref =~ s/^([^\x00-\x20\'\"\#\]\x7f-\xff]+)//;
  my $value = $1;
  $value = ($value =~ /^0/ ? oct($value) : 0+$value) if $value =~ /^(0x[0-9a-f]+|0[0-7]*|[1-9][0-9]*)$/i;
  return $value;
}
1;

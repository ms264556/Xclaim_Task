; use strict;
; our $xtensa_system;		# passed via perl invocation
; our $xtensa_system_root;	# passed via perl invocation
; my $year = 1900 + (localtime)[5];
; my $headcomment = 
;  "system.xparm  -  Xtensa system-level parameter values file (xparm format)\n"
; ."\n"
; ."Copyright (c) 2007-$year Tensilica Inc.  ALL RIGHTS RESERVED.\n"
; ."These coded instructions, statements, and computer programs are the\n"
; ."copyrighted works and confidential proprietary information of Tensilica Inc.\n"
; ."They may not be modified, copied, reproduced, distributed, or disclosed to\n"
; ."third parties in any manner, medium, or form, in whole or in part, without\n"
; ."the prior written consent of Tensilica Inc.\n"
; ."\n"
; .'$Id: //depot/rel/Cottonwood/Xtensa/Software/src/system/system.xparm.tpp#3 $'."\n"
; ."\n"
; ."WARNING: This file was automatically generated.  Do not edit!\n"
; ;
; use Carp;
; use Carp::Heavy;
;#
;## Re-implement Carp's eval display to include line numbers, so error lines can be reasonably located:
;{package Carp;
; our $MaxEvalLen;
; BEGIN { undef &get_subname; }		# avoid warning about redefined get_subname()
; sub get_subname {
;   my $info = shift;
;   if (defined($info->{evaltext})) {
;     my $eval = $info->{evaltext};
;     if ($info->{is_require}) {
;       return "require $eval";
;     }
;     else {
;       $eval =~ s/([\\\'])/\\$1/g;
;       my @lines = split("\n", $eval);
;       $eval = join("\n", map(($_+1).": ".$lines[$_], 0 .. $#lines));
;       return "eval '" . str_len_trim($eval, $MaxEvalLen) . "'";
;     }
;   }
;   return ($info->{sub} eq '(eval)') ? 'eval {...}' : $info->{sub};
; }
;}
;#
;#
; use File::Path;
; use Object;
;#
;#  First create a log file for debugging:
;#
; #my $logfile = "$xtensa_system_root/misc/system.xparm.log";
; #mkpath("$xtensa_system_root/misc");
; my $logfile = "system.xparm.log";
; open(my $extralog, ">$logfile") or die "ERROR opening $logfile for write: $@";
;#
;#
;#  my $addr = $vec->vaddr($vecbase, $select);
;#
;#  Returns vector address given values of VECBASE register and vector-select pin.
;#  In the absence of the relocatable vector option, these arguments are ignored.
;#
;sub Xtensa::Vector::vaddr {
;  my ($self, $vecbase, $select) = @_;
;  my $p = $self->pconfig;
;  my $base;
;  if ($self->group eq "fixed") {
;    $base = 0;
;  } elsif ($self->group eq "dynamic") {
;    $vecbase = $p->vecbase_reset unless defined($vecbase);
;    $base = ($vecbase & ~($p->vecbase_align - 1));
;  } else {
;    $select = $p->static_vector_select_sw unless defined($select);
;    $base = $p->static_vector_base->[$select];
;  }
;  return $base + $self->offset;
;}
;# ----------------------------------------------------------------------
;# compute ceil(log2(x)), e.g. ceillog2(6) = ceillog2(8) = 3
;# it only works for <=32bit numbers
;# ----------------------------------------------------------------------
;sub ceillog2 {
;  my ($num) = @_;
;  return 32 if $num > 0x80000000;
;  my $i;
;  for ($i = 0; $num > (1 << $i); $i++) {}
;  $i;
;}
;#
;#
;#
;#######   Check / get system info   #######
; use Stash;
; use Xtensa::Params;
; stash_obj_isa($dict, $sys, "MultiCoreSystem") or die "system must be of type MultiCoreSystem";
;#######   Figure out top-level name of system object
; my ($sysname) = grep($_ ne "types" && $_ ne "modulemap", keys %{$dict->{roots}});
; print STDERR "TESTTEST system name is '$sysname'\n";
;#######   Subclass the system (to add derived information)   #######
; my $oldsys = $sys;	# save old one (just in case?)
; print STDERR "TESTTEST  creating perl package for ExpandedMultiCoreSystem ...\n";
; Stash::stash_create_hash_package_byname($dict, "ExpandedMultiCoreSystem");
; print STDERR "TESTTEST  ExpandedMultiCoreSystem::ISA is <", join(",",@ExpandedMultiCoreSystem::ISA), ">\n";
; $sys = dup_to ExpandedMultiCoreSystem $oldsys;
; $dict->{roots}{$sysname} = $sys;	# update dictionary
; #  Paranoia:
; print STDERR "TESTTEST  oldsys=$oldsys sys=$sys\n";
; stash_obj_isa($dict, $sys, "ExpandedMultiCoreSystem") or die "derived system must be of type ExpandedMultiCoreSystem";
;#######
; my $xtensa_tools_root = $ENV{"XTENSA_TOOLS_ROOT"};
;#
;#######   Expand system info   #######
;#
; my $cores = $sys->cores;
; @$cores >= 1 or die "Need at least one core";
; #
; #  Expand (subclass) core objects to include derived parameters:
; Stash::stash_create_hash_package_byname($dict, "ExpandedCoreInstance");
; @$cores = map((dup_to ExpandedCoreInstance $_), @$cores);
; #
; my %configs = ();
; my %corenames = ();		# hash of lowercased core names
; $sys->set_corenames( {} );	# hash of core names
; $sys->set_confignames( {} );	# hash of config names
;my ($coresys1, $coresys2);
; #my $shared_core = { name => "shared", config => $cores->[0]->config, vecselect => "0" };
; my $shared_core = o_new ExpandedCoreInstance;
; $shared_core->set_name("shared");
; $shared_core->set_config($cores->[0]->config);
; $shared_core->set_vecselect(0);
; my @cores_sh = (@$cores, $shared_core);
; $sys->set_cores_sh( \@cores_sh );
; my $i = 0;
; foreach my $core (@cores_sh) {
;   my $name = $core->name;
;   $sys->corenames->{$name} = $core;
;   my $lname = "\L$name";		# lowercase
;   exists($corenames{$lname}) and die "multiple core instances with same name $name (".$corenames{$lname}.", $i)";
;   $name =~ /^\d+$/ and die "core names cannot be completely numeric ($name)";
;   $corenames{$lname} = $i;
;   defined($core->index) and die "core $name ($i) index unexpectedly already defined";
;   defined($core->sys)   and die "core $name ($i) sys (Xtbench) unexpectedly already defined";
;   #  FIXME TODO: consider TDK flow for TIE (xtensa_params)
;   $core->set_index($i);
;   my $config = $core->config;
;   my $cfgseen = $configs{"\L$config"};
;   if (defined($cfgseen)) {
;     $core->set_sys( $cfgseen->[0] );
;     $config eq $cfgseen->[1]
;	or die "config names cannot differ only by case (".$cfgseen->[1].", $config)";
;   } else {
;     my %params = xtensa_params_init($xtensa_system, $config);
;     my $xtensa_root = $params{'config-prefix'} or die "missing config-prefix for core $i ($name)";
;     my $path_xparm = $xtensa_root."/config/core.xparm";
;     my $core_xparm = readfile($path_xparm);
;     my $core_dict = stash_dict_new("config $config");
;     stash_dict_import($core_dict, $sysdict);
;     my ($coresys) = stash_xml_read($core_dict, $core_xparm, $path_xparm, "sysdoc");
;     defined($coresys) or die "core $name ($i): no core info from $path_xparm";
;#print STDERR "TESTTEST type of coresys is ".ref($coresys).".\n";
;     #  Superclass to Xtbench...
;$coresys1 = $coresys;
;#print STDERR "*** coresys1 is ", Stash::dumpit($coresys1), ".\n";
;     $coresys = dup_to Xtbench $coresys;
;#print STDERR "*** coresys1 is ", Stash::dumpit($coresys1), ".\n";
;#print STDERR "TESTTEST shrunk coresys is ".ref($coresys).".\n";
;     #  So we can subclass to our own Xtbench derivative ...
;     Stash::stash_create_hash_package($dict,
;		Stash::dict_lookup_type($dict, "ExpandedXtbench"),
;		"ExpandedXtbench type");
;$coresys2 = $coresys;
;     $coresys = dup_to ExpandedXtbench $coresys;
;#print STDERR "TESTTEST expanded coresys is ".ref($coresys).".\n";
;#print STDERR "*** coresys1 is ", Stash::dumpit($coresys1), ".\n";
;     #
;     $coresys->set_xtensa_root($xtensa_root);		# FIXME: probably useful
;     $core->set_sys( $coresys );
;     $configs{"\L$config"} = [$coresys, $config];
;     $sys->confignames->{$config} = $coresys;
;     stash_dict_import($dict, $core_dict, 0, 1);	# merge core_dict into dict, so it gets output as well
;   }
;   $i++;
; }
; #  Now ensure core names and config names are mutually distinct,
; #  so we can mix them in the same registry directory (xtensa_system):
; foreach my $config (keys %configs) {
;   die "config names and core instance names must be distinct ($config is not)"
;	if exists($corenames{$config});
;   die "config names cannot be completely numeric ($config)"
;	if $config =~ /^\d+$/;
; }
;#
;#
;#######   Process system memory map
;#
; use Xtensa::AddressLayout;
; my $toplayout = new Xtensa::AddressLayout;
; #  Create CC (coherency controller's outgoing memory space):
; my $cc = Xtensa::System::AddressSpace->new(undef, 0xFFFFFFFF, {delay=>2});
; $toplayout->add_addressable("cc", $cc);
; $sys->set_cc( $cc );
; #  Connect each core's PIF to the CC:
; foreach my $core (@cores_sh) {
;     my $coresys = $core->sys;
;     my $xtensa_root = $coresys->xtensa_root;
;     my $layout = new Xtensa::AddressLayout;
;     my $emsg;
;     my $diags = 0;	# or 1, or per INCLUDE_DV_SECTIONS in genldscripts
;     $emsg = $layout->read_addressables(1, "$xtensa_root/lib/".($diags?"diag-":"")."core.xmap")
;	and die "could not read core xmap: $emsg";
;     #  No config-specific system map:
;     #my $sysxmap = defined($alt_system_xmap) ? $alt_system_xmap : "$xtensa_root/lib/system.xmap";
;     #$emsg = $layout->read_addressables(0, $sysxmap)
;	#and die "could not read system xmap: $emsg";
;     #  No config-specific diag map either, need something different for MP:
;     #$diags and $emsg = $layout->read_addressables(0, "$xtensa_root/lib/diag.xmap")
;	#and die "could not read diag xmap: $emsg";
;     #check_diag_hack($pconfig, $paramOptionsRef);
;     #  Connect to CC:
;     my $core_layer = $layout->lookup_layer("core");
;     my $corename = "core ".$core->name;
;     my $pif = $core_layer->find_addressable("PIF")
;		or die "no PIF for $corename (all MX cores need PIF!)";
;     @{$pif->ref_map} and die "$corename PIF should not have mappings:\n".$pif->dump;
;     $pif->add_mapping(0, [[$cc, 0, 0xFFFFFFFF]], {});
;     $toplayout->add_layout($core->name, $core_layer);
;#
;#
;     #  Compute pconfig style config info needed by initLayout():
;#
;     #  NOTE:  we maintain this config info *PER CORE* not per config,
;     #  because some of the parameters are per-core (vecselect, vecbase).
;     #  FIXME:  Eventually we can put per-config info in the config,
;     #  and per-core info in the core...
;#
;    BaseStruct ('Xtensa::ProcessorConfig',
;	'$vectors',
;	'$vectors_reloc',
;	'$xea',
;	'$fast_imem_l32r',
;	'$litbase_l32r',		# "extended L32R"
;	'$lsunits_cnt',
;	'$targhw_min_vnum',
;	'$targhw_max_vnum',
;	'$regionprot_xlt',
;       '$relocatable_vectors',
;       '$static_vector_select_sw',
;       '$static_vector_base',
;       '$vecbase_reset',
;       '$vecbase_align',
;       '$vecbase_size',
;    );
;    BaseStruct ('Xtensa::Vector',
;       '$name',
;       '$offset',
;       '$size',
;       '$group',
;       '$pconfig',
;    );
;#print STDERR "*** coresys1 is ", Stash::dumpit($coresys1), ".\n";
;#print STDERR "*** coresys2 is ", Stash::dumpit($coresys2), ".\n";
;#print STDERR "*** coresys is ", Stash::dumpit($coresys), ".\n";
;##print STDERR "*** coresys->pr ref is ", Stash::dumpit($coresys->ref_pr), ".\n";
;#print STDERR "*** ok1\n";
;    my $testpr = $coresys->pr();
;#print STDERR "*** ok2\n";
;#print STDERR "*** coresys->pr is ", Stash::dumpit($coresys->pr), ".\n";
;#print STDERR "*** ok3\n";
;    my $relvec = $coresys->pr->xt->vectors->relocatableVectorOption;
;#print STDERR "*** after relvec set\n";
;    my $pconfig = Xtensa::ProcessorConfig->new;
;        #  For initLayout:
;	#vectors => [$pr->vectors->swVectorList],
;    $pconfig->set_vectors_reloc([]);	# [$pr->vectors->swRelocVectorList],
;    $pconfig->set_xea($coresys->pr->xt->core->newExceptionArch ? "XEA2"
;		     : $coresys->pr->xt->core->externalExceptionArch ? "XEAX" : "XEA1");
;    my $mmu = $coresys->pr->xt->mmu;
;#print STDERR "#####  mmu->itlb->ways is ", Stash::dumpit($mmu->itlb->ways), ".\n";
;#    my $tmpiways = $mmu->itlb->ways;
;#print STDERR "#####  mmu->itlb->ways (scalar) is ", Stash::dumpit($tmpiways), ".\n";
;#print STDERR "#####  mmu->itlb->ways->[0] is ", Stash::dumpit($mmu->itlb->ways(0)), ".\n";
;    $mmu = undef unless $mmu and scalar($mmu->itlb->ways) == 1;
;    my $iway = $mmu ? (scalar($mmu->itlb->ways) == 1 ? $mmu->itlb->ways(0) : undef) : undef;
;    my $dway = $mmu ? (scalar($mmu->dtlb->ways) == 1 ? $mmu->dtlb->ways(0) : undef) : undef;
;    $pconfig->set_regionprot_xlt( ($coresys->pr->xt->mmuOption
;		and $mmu and $mmu->asidBits == 0 and $mmu->ringCount == 1
;		and $iway and $dway and $iway->assoc->numOfEntries == 8 and $dway->assoc->numOfEntries == 8
;		and defined($dway->assoc->PPNResetValues(0))) ? 1 : 0 );
;		# and ! $iway->isPpnConstant()
;		# and ! $dway->isPpnConstant()
;		# and $iway->assoc->PPNMsb() == 0 and $iway->assoc->PPNLsb() == 0
;		# and $dway->assoc->PPNMsb() == 0 and $dway->assoc->PPNLsb() == 0
;		# and grep($_ != 0, $iway->storedPpns()) == 0
;		# and grep($_ != 0, $dway->storedPpns()) == 0
;#print STDERR "############  is xlt = ", $pconfig->regionprot_xlt, ".\n";
;    #$pconfig->set_fast_imem_l32r($coresys->pr->xt->core->L32RnoReplay);
;    $pconfig->set_fast_imem_l32r(($dway and $dway->assoc->autoRefill
;		or $coresys->pr->xt->core->ExtL32R or $coresys->pr->xt->smallCore) ? 0 : 1);
;    $pconfig->set_litbase_l32r($coresys->pr->xt->core->ExtL32R);
;    $pconfig->set_lsunits_cnt($coresys->pr->xt->loadStoreUnitsCount);
;    # FIXME FIXME: should be derived from $coresys->pr->xt->TargetHWEarliestVersion by Version.pm:
;    $pconfig->set_targhw_min_vnum(240000);	# FIXME - need to use Version.pm - currently this parm only used by AddressLayout.pm to check for dual loadstore units that can't loadstore to IRAM/IROM in FLIX
;    # FIXME FIXME: should be derived from $coresys->pr->xt->TargetHWVersion by Version.pm:
;    $pconfig->set_targhw_max_vnum(240000);	# FIXME
;        #  For vec->vaddr():
;    $pconfig->set_relocatable_vectors($relvec);
;	# $params{'RelocatableVectors'},
;    $pconfig->set_static_vector_select_sw( defined($core->vecselect) ? $core->vecselect
;				  : $coresys->pr->xt->vectors->SW_stationaryVectorBaseSelect );
;	# $params{'StaticVectorSelect'},
;    $pconfig->set_static_vector_base( [ $coresys->pr->xt->vectors->stationaryVectorBase0,
;					 $coresys->pr->xt->vectors->stationaryVectorBase1 ] );
;	# [$params{'StaticVectorBase0'}, $params{'StaticVectorBase1'}],
;    $pconfig->set_vecbase_reset( $coresys->pr->xt->vectors->relocatableVectorBaseResetVal );
;	# $params{'DynVecBaseReset'},
;    #$pconfig->set_vecbase_align($params{'DynVecBaseAlignBits'} < 32 ? (1 << $params{'DynVecBaseAlignBits'}) : 0);
;    #
;    my @vectors = ();
;    my $maxofs = 0;
;    my $maxsize = 0;
;    foreach my $vec ($coresys->pr->xt->vectors->vectorList) {
;        my $group = ($relvec ? ($vec->group eq "stationary" ? "static" : "dynamic") : "fixed");
;	 my $xvec = Xtensa::Vector->new;
;	 $xvec->set_name( $vec->name );
;	 $xvec->set_offset( $relvec ? $vec->offset : $vec->baseAddress );
;	 $xvec->set_size( $vec->size );
;	 $xvec->set_group( $group );
;	 $xvec->set_pconfig( $pconfig );
;        push @vectors, $xvec;
;	 if ($group eq "dynamic") {
;	     $maxofs = $vec->offset if $maxofs < $vec->offset;
;	     $maxsize = $vec->offset + $vec->size if $maxsize < $vec->offset + $vec->size;
;	 }
;    }
;    if (0) {
;      printf $extralog "************************ Vectors...\n";
;      foreach my $vec (@vectors) {
;        printf $extralog "*** VEC vaddr=0x%08x ofs=0x%08x size=%-3d group=%s %s\n",
;                $vec->vaddr, $vec->offset, $vec->size, $vec->group, $vec->name;
;    }}
;    $pconfig->set_vectors(\@vectors);
;    $pconfig->set_vecbase_align(ceillog2($maxofs+1));
;    $pconfig->set_vecbase_size($maxsize);
;    $core->set_pconfig( $pconfig );
;#
;#
;     ## No point in trying this without providing some system memories...
;     #my $noSysSegs = 0;
;     #$core_layer->initLayout($pconfig, $noSysSegs);
;#
;     ###  And create the classical memory map:
;     #my $map = $layout->linkmap->build_classical_memmap;
; }
; #
; #  Connect system memories to the CC:
; $sys->set_memories({}) unless defined($sys->memories);
; foreach my $memname (keys %{$sys->memories}) {
;   my $pmem = $sys->memories->{$memname};
;   my $writable = ($pmem->writable || 0);
;   my $device = ($pmem->device || 0);
;   print $extralog ">>>>>  pmem '$memname' size=",$pmem->size," (",$pmem->size+0,") paddr=",$pmem->paddr," (",$pmem->paddr+0,") writable=${writable} device=${device}.\n";
;   my $attrs = {};
;   my $offset = 0;
;   my $mem;
;   my $target = $pmem->target;
;   if (defined($target)) {
;     $offset = ($pmem->offset || 0);
;     $attrs->{device} = 1 if $device;
;     $attrs->{writable} = 0 if ! $writable;
;     $mem = $toplayout->find_addressable($target);
;     defined($mem)
;       or die "system memory '$memname': memory/device/port '$target' does not exist (try CORENAME.inbound ?)";
;     $target =~ /\.inbound$/
;       or print STDERR "WARNING: system memory '$memname': memory/device/port '$target' is not an inbound PIF port (which are named CORENAME.inbound)";
;     $pmem->size-1 <= $mem->sizem1
;       or die sprintf("system memory '%s': %s port offset 0x%x mapped at paddr 0x%x size 0x%x but is only 0x%x bytes in size",
;			$memname, $target, $offset, $pmem->paddr, $pmem->size, $mem->sizem1+1);
;   } else {
;     defined($pmem->offset) and die "new memory '$memname': unexpected offset parameter";
;     $mem = Xtensa::System::Addressable->new($pmem->size-1, undef,
;		{'device'=>$device, 'writable'=>$writable});
;     $toplayout->add_addressable($memname, $mem);
;   }
;   $cc->add_mapping($pmem->paddr, [[$mem, $offset, $pmem->size-1]], $attrs);
; }
; print $extralog "==========  Global Layout ============\n";
; print $extralog scalar($toplayout->format_addressables), "\n\n";
; print STDERR "==========  Invoking initMulticoreLayout ...\n";
; $toplayout->initMulticoreLayout($sys, createlocmems=>1, extralog => $extralog);
; #
; foreach my $core (@cores_sh) {
;   #  Output linkmap in some .xmap file:
;   my $corename = $core->name;
;   my $filename = "$xtensa_system_root/cores/$corename/xtensa-elf/lib/allcore.xmap";
;print STDERR "WRITING $filename ... (core is $core, layer is ".$core->layer.")\n";
;   mkpath("$xtensa_system_root/cores/$corename/xtensa-elf/lib");
;   open(OUT,">$filename") or die "Could not open $filename for writing: $!";
;   print OUT $core->layer->linkmap->dump;
;   close(OUT) or die "Could not close $filename after writing: $!";
;   #  Done with this; does its presence work fully with stash_xml_write()?:
;   #delete $core->{pconfig};
; }
;#
; $sys->set_layout( $toplayout );
; #
; my $bigxmap = "$xtensa_system_root/system.xmap";
; #mkpath($xtensa_system_root);
; open(OUT,">$bigxmap") or die "Could not open $bigxmap for writing: $!";
; print OUT scalar($toplayout->format_addressables);
; close(OUT) or die "Could not close $bigxmap after writing: $!";
;#
;#
;#
;#######   Output expanded system info   #######
; my $xml = stash_xml_write($dict, "sysdoc", $headcomment);
; #  FIXME CHEAT HACK: call it twice, to get new types known: HACK HACK HACK
; $xml = stash_xml_write($dict, "sysdoc", $headcomment);
`$xml`

;#  Paranoia test:
; #my $foo = stash_lookup($dict,"...");
; my $foo = $sys->cores->[0]->sys->pr->xt;
; my $foodict = $dict->{objs}{$foo}{dict};
; $foodict eq $dict or die "oops, core 0 xt is dict ".$foodict->{name}." instead of ".$dict->{name};
; my $foo2 = $sys->cores->[0]->sys->pr->xt->vectors->vectorList(0)->baseAddress;
; #printf STDERR "+++++ core 0 reset vaddr = 0x%x\n", $foo2;
;#
;#  And close the debug log file:
; close($extralog) or die "ERROR closing $logfile: $@";
;print STDERR "Done system.xparm.tpp\n";

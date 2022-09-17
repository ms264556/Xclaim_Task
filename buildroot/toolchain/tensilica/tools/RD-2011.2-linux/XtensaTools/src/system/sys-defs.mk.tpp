#  sys-defs.mk - generated Makefile information about the (sub)system
#
; my $year = 1900 + (localtime)[5];
# Copyright (c) 2007-`$year` by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
;#
; stash_obj_isa($sys, "MultiCoreSystem") or die "system must be of type MultiCoreSystem";
;# FOR TESTING:
;#  sys is ` defined($sys) ? "" : "not "`defined
;#  dict is ` defined($dict) ? "" : "not "`defined
;#  sys is of type `$dict->{objs}{$sys}{type}{name}`.
;#  which is ` stash_obj_isa($sys,"MultiCoreSystem") ? "okay" : "not okay"`.

#  Core instances, in PRID order (starting with 0, no gaps).
; my @cores = @{$sys->cores};
NCORES = ` scalar(@cores)`
CORES = ` join(' ',map($_->name, @cores))`

#  Core instances that share their reset vector with another (not in any order):
; my @shared_reset = grep($_->[0] eq 'ResetVector', @{$sys->shared_sets});
SHARED_RESET_CORES = ` join(' ',map($_->name, map(@{$_->[3]}, @shared_reset)))`

#  Core configs, in alphabetical order.
; my %configs = ();
; foreach my $core (@cores) {$configs{$core->config} = 1;}
NCONFIGS = ` scalar(keys %configs)`
CONFIGS = ` join(' ',sort keys %configs)`

#  Core to use when working with shared elements

SHARED_CONFIG = `$cores[0]->name`

#  Configs for each core.
; foreach my $core (@cores) {
CORE_`$core->name`_CONFIG = `$core->config`
; }
;#
;#  core_config = $(CORE_$(1)_CONFIG)
;#  coren_config = $(CORE_$(word $(1),$(CORES))_CONFIG)
;#  $(call core_config,<name>)
;#  $(call coren_config,<number>)

#  Per-core derived/constructed/system info:
; foreach my $core (@cores) {
;   my $p = $core->pconfig;
CORE_`$core->name`_VECBASE   = `$p->relocatable_vectors ? sprintf("0x%08x",$p->vecbase_reset) : ""`
CORE_`$core->name`_VECSELECT = `$p->relocatable_vectors ? $p->static_vector_select_sw : ""`
CORE_`$core->name`_RESET_TABLE_VADDR = ` defined($core->shared_reset_table_vaddr) ? sprintf("0x%08x", $core->shared_reset_table_vaddr) : ""`
; }

#  The system contains:
; use Stash;
; my $xml = stash_xml_write($dict, "sysdoc");
; $xml =~ s/^/#\t/gm;
; print $xml;
#  end.

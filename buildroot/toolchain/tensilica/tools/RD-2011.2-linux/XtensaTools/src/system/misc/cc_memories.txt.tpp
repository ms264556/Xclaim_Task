#  cc_memories.txt - generated list of memories connected to the CC
#
; my $year = 1900 + (localtime)[5];
# Copyright (c) 2007-`$year` by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.
; stash_obj_isa($sys, "MultiCoreSystem") or die "system must be of type MultiCoreSystem";

#  Memories connected to CC:
; foreach my $seg (@{$sys->{cc}{map}}) {
` sprintf("%s_addr = 0x%x", $seg->{mems}[0]{mem}{name}, $seg->{addr})` 
` sprintf("%s_size = 0x%x", $seg->{mems}[0]{mem}{name}, $seg->{sizem1}+1)` 
;#` sprintf("%s_flags = %s", $seg->{mems}[0]{mem}{name}, $seg->{...})` 
; }


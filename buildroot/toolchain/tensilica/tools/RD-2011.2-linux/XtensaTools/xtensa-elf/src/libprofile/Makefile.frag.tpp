# Copyright (c) 2003, 2004, 2005, 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.

; use strict;
; my @timerints = (0, 0, 0, 0);
; if ($pr->configured('Timers')) {
;    for my $t ($pr->timers()->timer()) {
;       my $level = $pr->interrupts()->intrNumber($t->interrupt())->level();
;       if ($level >= 2) {
;         $timerints[$t->number()] = 1;
;       }
;     }
; }
; my $targets = "";
; for my $i (0 .. 3) {
;   if($timerints[$i] == 1) {
;      $targets .= "libhwprofile" . $i . ".a " . "hw_profile_interrupt." . $i . ".o "
;   }
; }
TARGETS = `$targets`

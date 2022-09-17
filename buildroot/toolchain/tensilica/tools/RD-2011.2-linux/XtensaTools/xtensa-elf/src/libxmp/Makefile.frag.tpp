#  Copyright (C) 2007--2009 Tensilica, Inc.  
; use strict;
; my $objects = "";
; my $dobjects = "";
; if ($pr->configured('ConditionalStore') && $pr->configured('Synchronization')) {
;    $objects = "\$(OBJS) \$(MALLOC_OBJS)";
;    $dobjects = "\$(DEBUG_OBJS) \$(MALLOC_OBJS)";
; }

OBJECTS = `$objects`
DEBUG_OBJECTS = `$dobjects`


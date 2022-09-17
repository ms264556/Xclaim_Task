; # Copyright (c) 2003-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
; # These coded instructions, statements, and computer programs are the
; # copyrighted works and confidential proprietary information of Tensilica Inc.
; # They may not be modified, copied, reproduced, distributed, or disclosed to
; # third parties in any manner, medium, or form, in whole or in part, without
; # the prior written consent of Tensilica Inc.
; #####################################################
; # helper subroutine for printing mapfile symbols
; sub mapfile_add_sym {
;   my ($sym) = @_;
;   # print the name, with a trailing semicolon for Unix
;   if ($^O eq 'MSWin32') {
;     print "  $sym\n";
;   } else {
;     print "  $sym;\n";
;   }
; }
; #####################################################
; sub hx { sprintf("0x%x",shift @_); }
; #####################################################
; ##### inputs #####
; # $gen_data: 0 => .c file, 1 => .h file, 2 => .map file
; $name = $names{1};
; $iname = $name;
; $orig_iname = $names{-1};
; $words = int(($bits + 31) / 32);
; $num_wports = scalar(@wports);
; #####################################################
; $max_width = 0;
; foreach $i (keys %names) {
;    $max_width = $max_width < $i ? $i : $max_width;
; }
; #####################################################
; @bit = ();
; if ($max_width == 1) {
;   push(@bit, "");
; } else {
;   for ($w = 0; $w < $max_width; $w++) {
;     push(@bit, $w);
;   }
; }
; #####################################################
; if ($max_stages == 32) {
;   $pipeline_type = 'unsigned';
;   $one = '1U';
; } else {
;   $pipeline_type = 'unsigned long long';
;   $one = '1ULL';
; }
; #####################################################
; @stage_name = ();
; $r_stage = 0;
; $stage_name[$r_stage] = "R_STAGE";
; for ($i = $r_stage+1; $i < $e_stage; $i++) {
;     $after_r = $i - $r_stage;
;     $stage_name[$i] = "(R_STAGE+" . "$after_r)";
; }
; $stage_name[$e_stage] = "E_STAGE";
; for ($i = $e_stage+1; $i < $m_stage; $i++) {
;     $after_e = $i - $e_stage;
;     $stage_name[$i] = "(E_STAGE+" . "$after_e)";
; }
; $stage_name[$m_stage] = "M_STAGE";
; for ($i = $m_stage+1; $i < $w_stage; $i++) {
;     $after_m = $i - $m_stage;
;     $stage_name[$i] = "(M_STAGE+" . "$after_m)";
; }
; $stage_name[$w_stage] = "W_STAGE";
; for ($i = $w_stage+1; $i <= $max_stages; $i++) {
;     $after_w = $i - $w_stage;
;     $stage_name[$i] = "(W_STAGE+" . "$after_w)";
; }
; #####################################################
; # Exported symbols for the mapfile (gen_data == 2)
; #####################################################
; if ($gen_data == 2) {
;   $name = "my_" . $iname;
;   for ($w = 1; $w <= $max_width; $w = $w << 1) {
;     if (defined $names{$w}) {
;       mapfile_add_sym('my_' . $names{$w} . '_stall');
;       foreach $rport (sort keys %rport_map) {
;         mapfile_add_sym('my_' . $names{$w} . '_' . $rport . '_set_use');
;         mapfile_add_sym('my_' . $names{$w} . '_' . $rport . '_use');
;       }
;     }
;   }
;   foreach $wport (sort keys %wport_map) {
;     for ($w = 1; $w <= $max_width; $w = $w << 1) {
;       if (defined $names{$w}) {
;         mapfile_add_sym('my_' . $names{$w} . '_' . $wport . '_set_def');
;         mapfile_add_sym('my_' . $names{$w} . '_' . $wport . '_def');
;         mapfile_add_sym('my_' . $names{$w} . '_' . $wport . '_kill_def');
;       }
;     }
;   }
; } else { # ($gen_data != 2)
;     $name = "my_" . $iname;
;     if ($gen_data == 1) {

   /***********************************************************************
    * Regfile `$iname` fields 
; if ($gen_data == 1) {
;  foreach $i (sort keys %rport_map) {
    * `$i` => `$rport_map{$i}`
;  }
;  foreach $i (sort keys %wport_map) {
    * `$i` => `$wport_map{$i}`
;  }
; }
    ***********************************************************************/
    unsigned `$name`_committed_data[`$depth`][`$words`];
    `$pipeline_type` `$name`_def_pipeline;
    `$pipeline_type` `$name`_use_pipeline;
    unsigned `$name`_def_array_start;
    unsigned `$name`_use_array_start;
#define `$iname`_ARRAY_SIZE `$max_stages` 
#define `$iname`_ARRAY_MASK `$max_stages-1` 
; foreach $wport (@wports) {
;  for ($w = 0; $w < $max_width; $w++) {
    unsigned `$name``$bit[$w]`_`$wport`_def_data[`$iname`_ARRAY_SIZE][`$words`];
;  }
    int `$name`_`$wport`_def_addr[`$iname`_ARRAY_SIZE];
    int `$name`_`$wport`_def_valid[`$iname`_ARRAY_SIZE];
    int `$name`_`$wport`_def_width[`$iname`_ARRAY_SIZE];
    int `$name`_`$wport`_def_stage[`$iname`_ARRAY_SIZE];
; }
; foreach $rport (@rports) {
    int `$name`_`$rport`_use_addr[`$iname`_ARRAY_SIZE];
; }
;     } else { # ($gen_data == 0)

;       $prefix = "info->" . $name;
/************************************************************************
    Regfile `$iname` functions
    Last def stage:  `$def_stage`  `$stage_name[$def_stage]`
    Last stage:      `$last_stage`  `$stage_name[$last_stage]`
************************************************************************/

#define `$iname`_DEF_STAGE_INDEX(stage) \
  ((stage + `$prefix`_def_array_start) & `$iname`_ARRAY_MASK)

#define `$iname`_USE_STAGE_INDEX(stage) \
  ((stage + `$prefix`_use_array_start) & `$iname`_ARRAY_MASK)

static void
`$name`_cycle(dll_data_ptr_t info)
{
  int addr;
  if (`$prefix`_def_pipeline) {

; foreach $wport (@wports) {
    if (`$prefix`_`$wport`_def_valid[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)]) {
        addr = `$prefix`_`$wport`_def_addr[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)];
        if (info->events)
            iss4_regfile_event(info->core, `$stage_name[$last_stage]`, "`$orig_iname`", addr, `$bits`, `$prefix``$bit[0]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)], `$prefix`_committed_data[addr]);
; if ($words > 4) {
        { int j;
            for (j = 0; j < `$words`; j++) 
	        `$prefix`_committed_data[addr][j] = `$prefix``$bit[0]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)][j];
        }
; } else {
;  for ($i = 0; $i < $words; $i++) {
	`$prefix`_committed_data[addr][`$i`] = `$prefix``$bit[0]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)][`$i`];
;  }
; }
;  for ($w = 2; $w <= $max_width; $w = $w << 1) {
	if (`$prefix`_`$wport`_def_width[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)] >= `$w`) {
;   for ($e = $w >> 1; $e < $w; $e++) {
            if (info->events)
                iss4_regfile_event(info->core, `$stage_name[$last_stage]`, "`$orig_iname`", addr+`$e`, `$bits`, `$prefix``$bit[$e]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)], `$prefix`_committed_data[addr+`$e`]);
; if ($words > 4) {
        { int j;
            for (j = 0; j < `$words`; j++)
	        `$prefix`_committed_data[addr+`$e`][j] = `$prefix``$bit[$e]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)][j];
        }
; } else {
;    for ($i = 0; $i < $words; $i++) {
	    `$prefix`_committed_data[addr+`$e`][`$i`] = `$prefix``$bit[$e]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(`$stage_name[$last_stage]`)][`$i`];
;    }
; }
;   }
	}
;  }
    } 

; }
    `$prefix`_def_array_start = `$iname`_DEF_STAGE_INDEX(-1);

; foreach $wport (@wports) {
    `$prefix`_`$wport`_def_valid[`$iname`_DEF_STAGE_INDEX(`$stage_name[0]`)] = 0;
    `$prefix`_`$wport`_def_width[`$iname`_DEF_STAGE_INDEX(`$stage_name[0]`)] = 0;
; }

    `$prefix`_def_pipeline >>= 1;
  }

  if (`$prefix`_use_pipeline) {

    `$prefix`_use_array_start = `$iname`_USE_STAGE_INDEX(-1);

; foreach $rport (@rports) {
    `$prefix`_`$rport`_use_addr[`$iname`_USE_STAGE_INDEX(`$stage_name[0]`)] = 0;
; }

    `$prefix`_use_pipeline >>= 1;
  }
}

int
`$name`_stall(dll_data_ptr_t info, int use_stage, int addr)
{
    int i, has_def, addr_match;

    for (i = `$stage_name[1]`; i <= `$stage_name[$last_stage]`; i++) {
        int index = `$iname`_DEF_STAGE_INDEX(i);
; for ($wp = 0; $wp < $num_wports; $wp++) {
;  $wport = $wports[$wp];
;  $wport_depth = $wport_depths[$wp];
	has_def = `$prefix`_`$wport`_def_width[index];
; if ($max_width > 1) {
	addr_match = `$prefix`_`$wport`_def_addr[index] == (addr & ~(`$prefix`_`$wport`_def_width[index] - 1));
; } else {
	addr_match = `$prefix`_`$wport`_def_addr[index] == addr;
; }
	if (has_def && addr_match) {
	    if (`$prefix`_`$wport`_def_stage[index] - i >= use_stage)
		return 1;
; if ($num_wports > 1 && $wport_depth <= $last_stage) {
	    if (i >= `$stage_name[$wport_depth]`) break;
; }
; if ($split_pipe) {
            if (i >= `$stage_name[$w_stage]` && 
                `$prefix`_`$wport`_def_stage[index] <= `$stage_name[$w_stage]`)
                break;
; }
	}
; }
    }
    return 0;
}

; for ($w = 2; $w <= $max_width; $w = $w << 1) {
;  if (defined $names{$w}) {
int
my_`$names{$w}`_stall(dll_data_ptr_t info, int use_stage, int addr) 
{
    int stall = 0;
;   for ($e = 0; $e < $w; $e++) {
    stall |= `$name`_stall(info, use_stage, addr + `$e`);
;   }
    return stall;
}

;  }
; }
;#
; foreach $rport (sort keys %rport_map) {
;  for ($w = 1; $w <= $max_width; $w = $w << 1) {
;   if (defined $names{$w}) {
void
my_`$names{$w}`_`$rport`_set_use(dll_data_ptr_t info, int use_stage, int addr)
{
    `$prefix`_`$rport_map{$rport}`_use_addr[`$iname`_USE_STAGE_INDEX(`$stage_name[0]`)] = addr;
    `$prefix`_use_pipeline |= (`$one` << use_stage);
}

;   }
;  }
; }
;#
; foreach $wport (@wports) {
static void
`$name`_set_`$wport`_def_width(dll_data_ptr_t info, int def_stage, int addr, int width)
{
    `$prefix`_`$wport`_def_width[`$iname`_DEF_STAGE_INDEX(`$stage_name[0]`)] = width;
    `$prefix`_`$wport`_def_stage[`$iname`_DEF_STAGE_INDEX(`$stage_name[0]`)] = def_stage;
    `$prefix`_`$wport`_def_addr[`$iname`_DEF_STAGE_INDEX(`$stage_name[0]`)] = addr;
    `$prefix`_def_pipeline |= (`$one` << `$stage_name[$last_stage]`);
}

; }
; #
; foreach $wport (sort keys %wport_map) {
;  for ($w = 1; $w <= $max_width; $w = $w << 1) {
;   if (defined $names{$w}) {
void
my_`$names{$w}`_`$wport`_set_def(dll_data_ptr_t info, int def_stage, int addr)
{
    `$name`_set_`$wport_map{$wport}`_def_width(info, def_stage, addr, `$w`);
}

;   }
;  }
; }
;#
static void
`$name`_use(dll_data_ptr_t info, int use_stage, int addr, unsigned *v)
{
    if (`$prefix`_def_pipeline) {
        int i;
        for (i = use_stage + 1; i <= `$stage_name[$last_stage]`; i++) {
            int valid, match;
            int index = `$iname`_DEF_STAGE_INDEX(i);
; if ($max_width > 1) { 
            int mask;
; }
; foreach $wport (@wports) {
            valid = `$prefix`_`$wport`_def_valid[index];
; if ($max_width > 1) { 
            mask = `$prefix`_`$wport`_def_width[index] - 1;
            match = `$prefix`_`$wport`_def_addr[index] == (addr & ~mask);
; } else {
            match = `$prefix`_`$wport`_def_addr[index] == addr;
; }
            if (valid && match) {
; if ($max_width > 1) {
                switch (addr & mask) {
; }
;  for ($w = 0; $w < $max_width; $w++) {
; if ($max_width > 1) {
                case `$w`:
; }
; if ($words > 4) {
                    { int j;
                      for (j = 0; j < `$words`; j++) {
                        v[j] = `$prefix``$bit[$w]`_`$wport`_def_data[index][j];
                      }
                      if (info->events)
                          iss4_regfile_event(info->core, use_stage, "`$orig_iname`", addr, `$bits`, `$prefix``$bit[$w]`_`$wport`_def_data[index], 0);
                    }
; } else {
;   for ($i = 0; $i < $words; $i++) {
                    v[`$i`] = `$prefix``$bit[$w]`_`$wport`_def_data[index][`$i`];
;   }
                    if (info->events)
                        iss4_regfile_event(info->core, use_stage, "`$orig_iname`", addr, `$bits`, `$prefix``$bit[$w]`_`$wport`_def_data[index], 0);
; }
		    return;
;  }
; if ($max_width > 1) {
	        }
; }
	    }
; }
        }
    }

; if ($words > 4) {
    { int j;
      for (j = 0; j < `$words`; j++) {
        v[j] = `$prefix`_committed_data[addr][j];
      }
    }
; } else {
;   for ($i = 0; $i < $words; $i++) {
    v[`$i`] = `$prefix`_committed_data[addr][`$i`];
;   }
; }
    if (info->events)
        iss4_regfile_event(info->core, use_stage, "`$orig_iname`", addr, `$bits`, `$prefix`_committed_data[addr], 0);
}

; foreach $rport (sort keys %rport_map) {
void
`$name`_`$rport`_use(dll_data_ptr_t info, int use_stage, unsigned *v)
{
    int addr = `$prefix`_`$rport_map{$rport}`_use_addr[`$iname`_USE_STAGE_INDEX(use_stage)];
    `$name`_use(info, use_stage, addr, v);
}

; }
; #
; for ($w = 2; $w <= $max_width; $w = $w << 1) {
;  if (defined $names{$w}) {
;   $widenum = int (($bits * $w + 31) / 32);
; #  @wideval = ();
; #  @passwideval = ();
; #  for ($i = 0; $i < $widenum; $i++) {
; #    push(@wideval, "unsigned *v$i");
; #    push(@passwideval, "v$i");
; #  }
; #  $wideval = join(", ", @wideval);
; #  $passwideval = join(", ", @passwideval);
;  $wideval = "unsigned *v";
;  $passwideval = "v"; 
void
my_`$names{$w}`_use(dll_data_ptr_t info, int use_stage, int addr, `$wideval`) 
{
; $narrownum = int (($bits + 31) / 32);
; for ($i = 0; $i < $w; $i++) {
; # @narrowval = ();
; # for ($j = 0; $j < $narrownum; $j++) {
; #   push(@narrowval, "u" . "$i" . "_" . "v" . "$j");
; # }
; # $narrowval = join(", ", @narrowval);
; #  unsigned `$narrowval`;
    unsigned u`$i`_v[`$narrownum`];
; }
; $narrownum = int (($bits + 31) / 32);
; for ($i = 0; $i < $w; $i++) {
; # @narrowval = ();
; # for ($j = 0; $j < $narrownum; $j++) {
; #   push(@narrowval, "&u" . "$i" . "_" . "v" . "$j");
; # }
; # $narrowval = join(", ", @narrowval);
; #  `$name`_use(info, use_stage, addr + `$i`, `$narrowval`);
    `$name`_use(info, use_stage, addr + `$i`, u`$i`_v);
; }

; $wi = 0;
; $bi = 0;
; for ($ui = 0; $ui < $w; $ui++) {
;     for ($vi = 0; $vi < $words; $vi++) {
;         $si = ($vi < ($words-1)) ? 32 : ($bits - ($words-1) * 32);
;         if ($bi == 0) {
    v[`$wi`] = u`$ui`_v[`$vi`];
;         } else {
    v[`$wi`] |= u`$ui`_v[`$vi`] << `$bi`;
;         }
;         if ((32 - $bi) < $si) {
    v[`$wi + 1`] = u`$ui`_v[`$vi`] >> (`$si - 32 + $bi`);
;         }
;         $nbi = $bi + $si;
;         $wi += $nbi >= 32;
;         $bi = $nbi % 32;
;     }
; }
}

; foreach $rport (sort keys %rport_map) {
void
my_`$names{$w}`_`$rport`_use(dll_data_ptr_t info, int use_stage, `$wideval`)
{
    int addr = `$prefix`_`$rport_map{$rport}`_use_addr[`$iname`_USE_STAGE_INDEX(use_stage)];
    my_`$names{$w}`_use(info, use_stage, addr, `$passwideval`);
}

; }
;  }
; }
; #
; foreach $wport (sort keys %wport_map) {
;  for ($w = 1; $w <= $max_width; $w = $w << 1) {
;   if (defined $names{$w}) {
;    $widenum = int (($bits * $w + 31) / 32);
; #   @wideval = ();
; #   for ($i = 0; $i < $widenum; $i++) {
; #    push(@wideval, "unsigned v$i");
; #   }
; #   $wideval = join(", ", @wideval);
;    $wideval = "unsigned *v";
void
my_`$names{$w}`_`$wport`_def(dll_data_ptr_t info, int def_stage, `$wideval`)
{
; $narrownum = int (($bits + 31) / 32);
; for ($i = 0; $i < $w; $i++) {
; # @narrowval = ();
; # for ($j = 0; $j < $narrownum; $j++) {
; #   push(@narrowval, "u" . "$i" . "_" . "v" . "$j");
; # }
; # $narrowval = join(", ", @narrowval);
    unsigned u`$i`_v[`$narrownum`];
; }
    if (`$prefix`_`$wport_map{$wport}`_def_width[`$iname`_DEF_STAGE_INDEX(def_stage)]) {
	`$prefix`_`$wport_map{$wport}`_def_valid[`$iname`_DEF_STAGE_INDEX(def_stage)] = 1;
; $wi = 0;
; $bi = 0;
; for ($ui = 0; $ui < $w; $ui++) {
;     for ($vi = 0; $vi < $words; $vi++) {
;         $si = ($vi < ($words-1)) ? 32 : ($bits - ($words-1) * 32);
;         if ($bi == 0) {
;             if ($si == 32) {
	u`$ui`_v[`$vi`] = v[`$wi`];
;             } else {
	u`$ui`_v[`$vi`] = v[`$wi`] & ((1 << `$si`) - 1);
;             }
;         } else {
;             if ($si == 32) {
	u`$ui`_v[`$vi`] = v[`$wi`] >> `$bi`;
;             } else {
	u`$ui`_v[`$vi`] = (v[`$wi`] >> `$bi`) & ((1 << `$si`) - 1);
;             }
;         }
;         if ((32 - $bi) < $si) {
;             if ($si == 32) {
	u`$ui`_v[`$vi`] |= v[`$wi + 1`] << `(32 - $bi)`;
;             } else {
	u`$ui`_v[`$vi`] = (u`$ui`_v[`$vi`] | (v[`$wi + 1`] << `(32 - $bi)`)) & ((1 << `$si`) - 1);
;             }
;         }
;         $nbi = $bi + $si;
;         $wi += $nbi >= 32;
;         $bi = $nbi % 32;
;     }
; }
; for ($j = 0; $j < $w; $j++) {
;   if ($words > 4) {
        { int j;
            for (j = 0; j < `$words`; j++) {
                `$prefix``$bit[$j]`_`$wport_map{$wport}`_def_data[`$iname`_DEF_STAGE_INDEX(def_stage)][j] = u`$j`_v[j];
            }
        }
;   } else {
;     for ($i = 0; $i < $words; $i++) {
	`$prefix``$bit[$j]`_`$wport_map{$wport}`_def_data[`$iname`_DEF_STAGE_INDEX(def_stage)][`$i`] = u`$j`_v[`$i`];
;     }
;   }
; }
    }
}

;   }
;  }
; }
;#
; foreach $wport (@wports) {
static void
`$name`_`$wport`_kill_def_internal(dll_data_ptr_t info, int stage)
{
    `$prefix`_`$wport`_def_valid[`$iname`_DEF_STAGE_INDEX(stage)] = 0;
    `$prefix`_`$wport`_def_width[`$iname`_DEF_STAGE_INDEX(stage)] = 0;
}

; }
; foreach $wport (sort keys %wport_map) {
;  for ($w = 1; $w <= $max_width; $w = $w << 1) {
;   if (defined $names{$w}) {
void
my_`$names{$w}`_`$wport`_kill_def(dll_data_ptr_t info, int stage)
{
    `$name`_`$wport_map{$wport}`_kill_def_internal(info, stage);
}
;   }

;  }
; }
;#
static void 
`$name`_kill_stage_internal(dll_data_ptr_t info, int fs, int ts)
{
    int i;

    if (`$prefix`_def_pipeline) {
        for (i = fs; i <= ts; i++) {
; foreach $wport (@wports) {
            `$prefix`_`$wport`_def_valid[`$iname`_DEF_STAGE_INDEX(i)] = 0;
	    `$prefix`_`$wport`_def_width[`$iname`_DEF_STAGE_INDEX(i)] = 0;
; }
            `$prefix`_def_pipeline &= ~(`$one` << (`$stage_name[$last_stage]` - i));
        }
    }
}

; for ($w = 1; $w <= $max_width; $w = $w << 1) {
;  if (defined $names{$w}) {
static void 
my_`$names{$w}`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
    `$name`_kill_stage_internal(info, fs, ts);
}

;  }
; }
;#
/* Interfaces for ISS (see dll_regfile_table below) */
; #@lbuf = ();
; #for ($i = 0; $i < $words; $i++) {
; #   push(@lbuf, "&buf[$i]");
; #}
; #$lbuf = join(", ", @lbuf);
; $lbuf = "buf";
int
`$name`_stage_value(dll_data_ptr_t info, int stage, int addr, unsigned *buf)
{
    `$name`_use(info, stage, addr, `$lbuf`);
    return !`$name`_stall(info, stage, addr);
}

int
`$name`_set_stage_value(dll_data_ptr_t info, int stage, int addr, unsigned *buf)
{
    /* Fail if there is a definition coming down the pipe. */
    if (`$name`_stall(info, stage, addr))
        return 0;

; foreach $wport (@wports) {
    if (`$prefix`_`$wport`_def_width[`$iname`_DEF_STAGE_INDEX(stage)] &&
        addr == `$prefix`_`$wport`_def_addr[`$iname`_DEF_STAGE_INDEX(stage)]) {
;  for ($i = 0; $i < $words; $i++) {
;    my $wd = (($bits % 32) && ($i == ($words -1))) ? " & ". hx((1 << ($bits % 32)) - 1) : "";
	`$prefix``$bit[0]`_`$wport`_def_data[`$iname`_DEF_STAGE_INDEX(stage)][`$i`] = buf[`$i`]`$wd`;
;  }
        return 1;
    }
; }

; for ($i = 0; $i < $words; $i++) {
;  my $wd = (($bits % 32) && ($i == ($words -1))) ? " & ". hx((1 << ($bits % 32)) - 1) : "";
    `$prefix`_committed_data[addr][`$i`] = buf[`$i`]`$wd`;
; }
    return 1;
}

void
`$name`_commit_value(dll_data_ptr_t info, int addr, unsigned *buf)
{
; if ($def_stage < $last_stage) {
     `$name`_use(info, `$stage_name[$def_stage]`, addr, `$lbuf`);
; } else {
; if ($words > 4) {
    { int j;
        for (j = 0; j < `$words`; j++) {
	    buf[j] = `$prefix`_committed_data[addr][j];
        }
    }
;   } else {
;  for ($i = 0; $i < $words; $i++) {
    buf[`$i`] = `$prefix`_committed_data[addr][`$i`];
;  }
; }
; }
}

void
`$name`_set_commit_value(dll_data_ptr_t info, int addr, unsigned *buf)
{
; for ($i = 0; $i < $words; $i++) {
;  my $wd = (($bits % 32) && ($i == ($words -1))) ? " & ". hx((1 << ($bits % 32)) - 1) : "";
    `$prefix`_committed_data[addr][`$i`] = buf[`$i`]`$wd`;
; }
}

;     } # end of ($gen_data == 0)
; } # end of ($gen_data != 2)

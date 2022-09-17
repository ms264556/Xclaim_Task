; # Copyright (c) 2002-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
; # These coded instructions, statements, and computer programs are the
; # copyrighted works and confidential proprietary information of Tensilica Inc.
; # They may not be modified, copied, reproduced, distributed, or disclosed to
; # third parties in any manner, medium, or form, in whole or in part, without
; # the prior written consent of Tensilica Inc.
; ### inputs ###
; # $name = "mul32x4";
; # $last_stage = 6;
; #####################################################
; die "Error: must specify a name\n" if not defined $name;
; die "Error: must specify last_stage\n" if not defined $last_stage;
; die "Error: last_stage cannot be negative\n" if $last_stage < 0;
; die "Error: More than $max_stages stages are not supported\n" if $last_stage >= $max_stages;
; #####################################################
; @dinit = ();
; for ($i = 0; $i <= $last_stage; $i++) {
;    push(@dinit, "0");
; }
; $dinit = join(", ", @dinit);
; $stagesp1 = $last_stage + 1; 
; #
; if ($max_stages == 32) {
;   $pipeline_type = 'unsigned';
;   $one = '1U';
; } else {
;   $pipeline_type = 'unsigned long long';
;   $one = '1ULL';
; }
; #####################################################

; if ($gen_data) {
   /************************************************************************
    * Shared function `$name` modelling
    ************************************************************************/
    `$pipeline_type` mysf_`$name`_usages[`$stagesp1`];
    unsigned mysf_`$name`_any_use;
; } else {
/***************************************************************************
 *   Shared function `$name` modelling: functions
 ***************************************************************************/
; $prefix = "info->mysf_" . $name;

int
mysf_`$name`_stall(dll_data_ptr_t info, `$pipeline_type` usage)
{
    int i;
    if (`$prefix`_any_use) {
      for (i = 1; i <= `$last_stage`; i++) {
        if (usage & `$prefix`_usages[i]) return 1;
      }
    }
    return 0;
}

void
mysf_`$name`_set_usage(dll_data_ptr_t info, `$pipeline_type` usage)
{
    `$prefix`_usages[0] |= usage;
    `$prefix`_any_use = 1;
}

static void
mysf_`$name`_cycle(dll_data_ptr_t info)
{
    int i;
    if (`$prefix`_any_use) {
      int no_use = 1;
      for (i = `$last_stage`; i > 0; i--) {
        if ((`$prefix`_usages[i] = `$prefix`_usages[i-1] >> 1))
          no_use = 0;
      }
      `$prefix`_usages[0] = 0;
      if (no_use)
        `$prefix`_any_use = 0;
    }
}

static void
mysf_`$name`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
    int i;
    if (`$prefix`_any_use) {
      for (i = fs; i <= ts && i <= `$last_stage`; i++) {
        `$prefix`_usages[i] = 0;
      }
    }
}
; } # if ($gen_data)

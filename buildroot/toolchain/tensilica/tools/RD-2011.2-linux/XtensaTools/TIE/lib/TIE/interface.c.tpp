; # Copyright (c) 2003-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
; # These coded instructions, statements, and computer programs are the
; # copyrighted works and confidential proprietary information of Tensilica Inc.
; # They may not be modified, copied, reproduced, distributed, or disclosed to
; # third parties in any manner, medium, or form, in whole or in part, without
; # the prior written consent of Tensilica Inc.
; #
; #
; ### inputs ###
; #include <assert.h>
; # $name = "EPC1";
; # $words = 2;
; # $dir = 'in';
; # $type = undef;
; # $stage = 0;
; # $commit = 3;
; # $depth = 4;
; # $reg = 0;
; # $enable = undef; 
; # $defval  = undef;
; # $reg = 0;
; # $stall = 0;
; # $kill = undef;
; # $ekill = undef;
; # $rdwrfunc = undef;
; # $target = undef;
; # $clkgate = undef;
; ###########################################
;# Turn this on to make it debuggable
; $debug = 0;
; ##########################################
; my $bdepth;
; my $buf = 0;
; my $read_stage;
; my $use_stage;
; my $stall_stage;
; #####################################################
; my $iname = $name;
; my $pdepth = $commit + 1; # R stage to W stage
; if ($dir eq 'in') {
;   # Input
;   if ($type eq 'D') {
;     # Data
;     $buf = ($stage < $commit);
;     $use_stage = $stage;
;     $read_stage = $reg ? ($stage - 1) : $stage;
;     $stall_stage = $stage;
;     $bdepth = $commit - $read_stage + 1;
;   } else {
;     # Control
;     $bdepth = ($depth > 0) ? ($depth + 1) : 0; #depth is the latency
;     $buf = ($bdepth > 0);
;     $read_stage = $depth > 0 ? ($stage-$depth) : ($reg ? ($stage-1) : $stage);
;  }
; } else { 
;   # Output
;   $bdepth = ($type eq 'D' && $depth > 0) ? ($depth + $reg) : 0;
;   $buf = ($depth > 0);
;   $stall_stage = $stage;
; }
; # NOTE: right now read/write stalls after W stage cannot be interrupted
; #####################################################
; # implementation
; #####################################################
; #
; my $kill_name;
; if (defined $kill) {
;   $kill_name  = $kill.'_interface';
;   $kill = 1;
; } else {
;   $kill_name = '';
;   $kill = 0;
; } 
; #
; my $ekill_name;
; if (defined $ekill) {
;   $ekill_name  = $ekill.'_interface';
;   $ekill = 1;
; } else {
;   $ekill_name = '';
;   $ekill = 0;
; } 
; #
; $name = $iname . '_interface';
; $link_name = $name . '_link';
; $core_name = $name . '_core';
; $device_name = $name . '_device';
; #
; if ($gen_data) {
; if ($dir eq 'in') {
; ######################################################
; # INPUT PORT DATA FIELDS (this goes into libcas-*.h) #
; ######################################################
; if ($reg || $buf) {

   /***********************************************************************
; if ($reg) {
    * `$name`_staging_data:    Data used by semantic stage function
;   if ($type eq 'D') {
    * `$name`_staging_valid:   Does staging buffer have valid data?
;   }
; }
; if ($buf) {
;   if ($type eq 'D') {
    * `$name`_fifo[`$bdepth`]:         FIFO of data read from external port
    * `$name`_pipe[`$pdepth`]:         Data index for each pipe stage
    * `$name`_use_pipeline:    Track uses in pipe stages
    * `$name`_rptr:            Read pointer, reads the oldest entry
    * `$name`_srptr:           Speculative read pointer 
    * `$name`_wptr:            Write pointer, where new entry is added
;   } else {
    * `$name`_buf[`$bdepth`]:          Data sampled during globalstall
    * `$name`_rptr:            Read pointer (for the semantic)
    * `$name`_wptr:            Write pointer (for sampled data)
    * `$name`_sample_pipeline: Sampling pipeline for lookup
    * `$name`_read_pipeline:   Read pipeline for lookup
;   } #endif type
; } #endif buf
    ***********************************************************************/
; } #endif ($reg || $buf)
; if ($type eq 'D') {
    int `$name`_use_C`$read_stage`;
; }
; if ($reg) {
    unsigned `$name`_staging_data[`$words`];
;   if ($type eq 'D') {
    int `$name`_staging_valid;
    int `$name`_staging_killed;
    int `$name`_reads;
;     if ($stall) {
    int `$name`_notrdy;
    int `$name`_ext_stall;
    int `$name`_stall;
;     } #endif stall
;     if ($kill || $ekill) {
    int `$name`_cancel_read;
;     } #endif kill
;   } #endif type
; } #endif reg
; if ($buf) {
; ###############################
; #speculative buffer 
; ###############################
;   if ($type eq 'D') {
    unsigned `$name`_fifo[`$bdepth`][`$words`];
    int `$name`_pipe[`$pdepth`];
    unsigned `$name`_use_pipeline;
    unsigned `$name`_rptr;
    unsigned `$name`_srptr;
    unsigned `$name`_wptr;
;     if ($reg) {
    int `$name`_read_last_sentry;
;     }
;     if ($kill) {
    int `$name`_kill_C`$use_stage`;
;     }
;     if ($ekill) {
    int `$name`_kill_C`$read_stage`;
;     }
; ##################################################
; #simple  buffer to separate read and use of input
; ##################################################
;   } else { #type
    unsigned `$name`_buf[`$bdepth`][`$words`];
    unsigned `$name`_rptr;
    unsigned `$name`_wptr;
    unsigned `$name`_sample_pipeline;
    unsigned `$name`_read_pipeline;
;     if ($read_stage < $commit && $stage > $commit) {
    unsigned `$name`_use_pipeline;
    unsigned `$name`_reads;
;     }
;   } #endif type
; } #end if buf
; } else { #direction
; ###########################
; # OUTPUT PORT DATA FIELDS #
; ###########################

   /***********************************************************************
    * `$name`_semantic_data:  Data from the semantic
    * `$name`_write_pipeline: Keep track of writes in the pipe
    * `$name`_def_pipeline:   Keep track of defs in the pipe
;   if ($bdepth == 0) {
             write_pipeline is updated on the free clock
             def_pipeline is updated on the stall clock
;   }
;   if ($stall) {
    * `$name`_stall:          Stall due to this interface?
;   }
;   if ($bdepth > 0) {
    * `$name`_rptr:           Read pointer, reads the oldest entry
    * `$name`_wptr:           Write pointer, where new entry is added
    * `$name`_entries:        Number of entries available
    * `$name`_pipe:           Fake pipeline so that data written in
             one cycle can only get pushed out in the following cycle
;   }
    ***********************************************************************/
    unsigned `$name`_semantic_data[`$words`];
    unsigned `$name`_write_pipeline;
    unsigned `$name`_def_pipeline; 
;   if ($stall) {
    int `$name`_stall;
;   } #endif stall
;   if ($type eq 'D') {
    int `$name`_def_C`$stage`;
;   } #endif type
;   if ($kill) {
    int `$name`_kill_C`$stage`;
;   } #endif kill
;   if ($bdepth > 0) {
    unsigned `$name`_fifo[`$bdepth`][`$words`];
    int `$name`_pipe[`$bdepth`];
    unsigned `$name`_rptr;
    unsigned `$name`_wptr;
    int `$name`_entries;
;   } #endif bdepth
; } #endif direction
; } else { #gen_data
; #
; ###########################################################
; # IMPLEMENTATION FUNCTIONS (this goes into libcas-*.c)
; ###########################################################

; if ( $debug ) {
#ifndef _BINARY_PIPE_
#define _BINARY_PIPE_
static const char *
binvec(unsigned val)
{
  static char b_string[33];
  int len = 1, i;
  for (i = 31; i > 0; i--) {
    if (val & (1 << i)) {
      len = i + 1;
      break;
    }
  }
  for (i = 0; i < len; i++)
    b_string[len-i-1] = (val & (1 << i)) ? '1' : '0';
  b_string[len] = '\0';
  return b_string;
}
#endif

; }
/****************************************************************************
    NAME:       `$iname`
    DIRECTION:  `$dir eq 'in' ? "In" : "Out"`
    TYPE:       `$type eq 'D' ? "Data" : "Control"`
    BITWIDTH:   `$width`
    STAGE:      `$stage`
; if ($bdepth > 0) {
    BUF.DEPTH:  `$bdepth`
; }
; if (defined($enable)) {
    ENABLE:     `$enable`
; }
; if ($stall) {
    HAS STALL:  Yes
; }
    REGISTERED: `$reg ? "Yes" : "No"`
; if ($kill) {
    HAS KILL:   Yes
; }
; if ($ekill) {
    HAS EKILL:  Yes
; }

; #
;    if ( $dir eq 'out' ) {
;        if ( $stall ) {
    `$name`_stall: scheduled by the issue function at stage `$stage`
;           if ( $bdepth > 0 ) {
        If the internal buffer is full, return 1, else return 0.
;           } else {
        Check the external stall condition and return 0 or 1.
;           } #endif bdepth
;        } #endif stall
    `$name`_write : called by <instr>_stage<`$stage`> 
;       if ( $bdepth > 0 ) {
	Write the entry into the internal buffer, update the write
	pointer and number of entries and schedule the cycle
	function which will push it out to the external wire
;       } else {
	Write data to the external logic. 
;       } 
;       if ( $bdepth > 0 ) {
    `$name`_init : called by dll_initialize
        Initialize the buffer pointers
    `$name`_cycle : scheduled on the free clk from the write function
        Check if the external stall function is 1; if so, reschedule
	itself. If stall condition is 0, it writes the external wire
	and decrements the number of entries and updates the read 
	pointer. If number of entries is still >0, reschedules itself
	for the next cycle.
;       }
; # end ($dir eq 'out')
;     } else { 
; # begin ($dir eq 'in')
;         if ( $buf ) {
;#    The purpose of the buffer for tie wires is to keep speculatively read
;#    values inside Xtensa if the instr reading it gets killed. If an instr
;#    reads a data type tie wire before the commit stage, the data is sampled
;#    and an acknowledge sent to the external logic that data has been read.
;#    However, if the instr is killed, we keep the data in a buffer, and any
;#    subsequent instr reading the tie wire gets the data in the buffer instead
;#    of a new data being sampled. The depth of the buffer is (commit stage - N)
;#    where N is the read stage of the wire. However, due to the behavior of
;#    ISS scheduler, an extra entry is added in the fifo. This is because the
;#    the first call to cycle happens in the same cycle when it is scheduled.
;#    If the depth is 0, i.e. the tie wire is read in W stage, there should
;#    really be no buffer.
;#
;#    The buffer implementation works this way: 
;#                   
;#      rptr -----> --------------      ---  pipeline of index
;#	           |              |    |   |
;#		   |              |    |---|
;#	srptr ---->|              |    |   |
;#	           |              |    |---|
;#      wptr ----->|              |    |   |
;#	           |              |    
;#		    --------------
;#
;#	You can think of rptr pointing to where valid data is in buffer.
;#	srptr points to data that is now being speculatively read by instr
;#	that can get killed. wptr is the pointer where you can add new 
;#	entries. When a semantic stage function calls the wire_read 
;#	function for a tie wire, if the buffer is empty, data is sampled
;#	sampled data gets written to fifo at wptr and wptr  gets put into the 
;#	pipe to be cycled thru the pipeline, when it reaches the end of the
;#	pipe, i.e. the instr commits rptr gets updated. If the instruction
;#	is killed, the srptr (speculative rptr) gets updated. 
;#
    `$name`_init: initialize all data structures
;#
;         } #endif buf
    `$name`_read: called from the semantic stage function
;        if ( $stall ) {
    `$name`_stall: scheduled by the issue function
;             if ( $reg ) { 
	stall if staging buffer has no data and speculative buffer is empty
;             } else {
	stall if speculative buffer is empty and stall condition is true
;             } #endif reg
;        } #endif stall
;        if ( $buf ) {
    `$name`_kill_stage: called when pipeline stages are killed
;       } #endif buf
;    }# end ($dir eq 'in')          
****************************************************************************/
; my $prefix = 'info->' . $name;
; my $prefix_link = 'info->' . $link_name;
; my $prefix_core = 'info->' . $core_name;
; my $prefix_device = 'info->' . $device_name;
; my $prefix_space = " " x (length($prefix));
; # ########################################################################
; # OUTPUT BUFFER
; # ########################################################################
; if ( $dir eq 'out' ) {

; # Declare an extra function that is triggered when a write happens
;     if ( $rdwrfunc ) {
static int `$rdwrfunc`(dll_data_ptr_t info);
; ;     }#endif rdwrfunc

; # ########################################################################
; #THIS SECTION IS FOR OUTPUT INTERFACES THAT HAVE A BUFFER THAT RUNS ON A
; # STALL CLOCK AND CAN STALL THE PIPELINE IF THE BUFFER IS FULL.
; # ########################################################################
; if ( $bdepth > 0 ) {
; ################################################################
; #init code
; ################################################################
static void
`$name`_init(dll_data_ptr_t info)
{
    int i;
    for (i = `$bdepth - 1`; i >= 0; i--) {
      `$prefix`_pipe[i] = -1;
    }
    for (i = `$bdepth - 1`; i >= 0; i--) {
;   for ($i = 0; $i < $words; $i++) {
      `$prefix`_fifo[i][`$i`] = 0;
;   }
    }
    `$prefix`_wptr = 0;
    `$prefix`_entries = `$bdepth`;
    `$prefix`_write_pipeline = 0;
    `$prefix`_def_pipeline = 0;
    `$prefix`_def_C`$stage` = 0;
}

; ################################################################
; #Functions related to stalling for buffered outputs, where the stall
; #depends on external stall condition and buffer status
; ################################################################
;   if ( $stall ) {
static void `$name`_push_external(dll_data_ptr_t info);

static void
`$name`_check_stall(dll_data_ptr_t info)
{
    int i;
    int stall = `$name`_stall_expr(info);
    if (stall) {
; if ( $debug ) {
      fprintf(stdout, "TIEWIRE: `$name`_push_external: can't push data, "
              "pipe %d { ", `$prefix`_write_pipeline);
      for (i = 0; i < `$bdepth`; i++) {
        fprintf(stdout, "%d ", `$prefix`_pipe[i]);
      }
      fprintf(stdout, "}\n");
; }
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_push_external,
                                          info);
      return;
    }
; if ( $debug ) {
   { int rptr = `$prefix`_pipe[`$bdepth - 1`];
    fprintf(stdout, "TIEWIRE: `$name`_check_stall: Push succeeded with the following data : ");
;   for ($i = 0; $i < $words; $i++) {
        fprintf(stdout, "%x ", `$prefix`_fifo[rptr][`$i`]);
;   }
    fprintf(stdout, "\n");
   }
; } #if debug

; # assert(`$prefix`_entries != `$bdepth`);
    `$prefix`_entries ++;
    /* Cycle the index pipe */
    for (i = `$bdepth - 1`; i > 0; i--) {
        `$prefix`_pipe[i] = `$prefix`_pipe[i-1];
    }
    `$prefix`_pipe[0] = -1;
    `$prefix`_write_pipeline >>= 1;
; if ( $debug ) {
    fprintf(stdout, "writepipe %d { ", `$prefix`_write_pipeline);
    for (i = 0; i < `$bdepth`; i++) {
      fprintf(stdout, "%d ", `$prefix`_pipe[i]);
    }
    fprintf(stdout, "}\n");
; }

    if (`$prefix`_write_pipeline)
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_push_external,
                                          info);
; if (!defined($enable)) {
    if (`$prefix`_write_pipeline == 0)
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_default,
                                          info);
; }
}
;   } # end of if ( $stall )

; ################################################################
; # A def cycles thru the pipeline
; ################################################################
static void 
`$name`_def_cycle(dll_data_ptr_t info)
{
    if (`$prefix`_def_pipeline) {
      /* Shift right, make sure the R stage bit is zero */
      `$prefix`_def_pipeline >>= 1;
      if (`$prefix`_def_pipeline)
        iss4_schedule_for_end_of_stall_cycle(info->core,
                                             `$name`_def_cycle,
                                             info);
; if ($debug) {
      printf("%s %u: def_pipe %s\n", __func__, __LINE__,
             binvec(`$prefix`_def_pipeline));
; }
    }
}

; ################################################################
; # set the def when instruction is issued
; ################################################################
static void
`$name`_set_def(dll_data_ptr_t info)
{
; my $cond = $kill ? "old == 0 && !${prefix}_kill_C${stage}" : "old == 0";
    unsigned old = `$prefix`_def_pipeline;
    `$prefix`_def_pipeline |= (1 << `$pdepth-1`);
    if (`$cond`) 
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_def_cycle,
                                           info);
; if ($debug) {
    fprintf(stdout, "TIEWIRE: `$name`_set_def: def_pipeline %x reschedule %d",
            `$prefix`_def_pipeline, `$cond`);
; }
}

; ################################################################
; # kill the def_pipeline and write_pipeline when instructio is killed
; ################################################################
static void 
`$name`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
    int i;
    if (`$prefix`_def_pipeline) {
      for (i = fs; i <= ts; i++) {
        `$prefix`_def_pipeline &= (~(1 << (W_STAGE - i)));
      }
; if ($debug) {
      fprintf(stdout, "TIEWIRE: `$name`_kill_stage(%d): def_pipeline %x\n",
    	      fs, `$prefix`_def_pipeline);
; }
    }
}

; ################################################################
; # Function to generate NOTRDY signal
; ################################################################
static int
`$iname`_NOTRDY_external_stall(dll_data_ptr_t info)
{
   return 0;
}
/* Returns 1 if the output buffer is full.
 */
static void 
`$iname`_NOTRDY_interface_read(dll_data_ptr_t info, unsigned *result)
{
; if ($target =~ "LX1") {
    int i, defs = 0;
    /* count how many current defs are in the pipeline */
    for (i = 0; i < `$pdepth`; i++) {
      defs += (`$prefix`_def_pipeline & (1 << (W_STAGE-i))) != 0;
    }
    *result = (`$prefix`_entries == 0  || 
               `$prefix`_entries <= defs); 
; if ($debug) {
    printf("%s %u: `$iname`_notrdy = %d (def_pipe %x, %d defs, %d entries)\n",
           __func__, __LINE__, *result,
           `$prefix`_def_pipeline, defs, `$prefix`_entries);
; }
; } else {
    *result = (`$prefix`_entries == 0); 
; if ($debug) {
    printf("%s %u: `$iname`_notrdy = %d\n",
           __func__, __LINE__, *result);
; }
; }
      if ( info->events ) 
	iss4_tieport_event(info->core, `$stage`, "`$iname`_NOTRDY", "->", 1, result);
}

; ################################################################
; # Function to generate idle signal based on state of buffer
; ################################################################
static int
`$name`_idle(dll_data_ptr_t info)
{
    if (`$prefix`_entries == `$bdepth`) {
      int i, def, defs = 0;
      /* count how many current defs are in the pipeline */
      for (i = 0; i < `$pdepth`; i++) {
        def = (`$prefix`_def_pipeline & (1 << (W_STAGE-i))) != 0;
; if ($kill) {
        if (def && (i == `$stage`) && `$prefix`_kill_C`$stage`) 
          def = 0;
; }
        defs += def;
      }
; if ($debug) {
      fprintf(stdout, "TIEWIRE: `$name`_idle: %d defs, return %d\n", 
              defs, defs == 0);
; }
      return (defs == 0);
    }
; if ($debug) {
    fprintf(stdout, "TIEWIRE: `$name`_idle: %d entries, return 0\n", 
            `$prefix`_entries);
; }
    return 0;
}

; ################################################################
; #Functions to write data to external logic from an internal buffer
; # Note that we assume that the "enable" signal is always
; # a request and is not gated by stall condition.
; ################################################################
static void
`$name`_push_external(dll_data_ptr_t info)
{
    int i;
    unsigned data[`$words`];

    if (`$prefix`_write_pipeline == 0) return;

    /* Is there something to write? */
    if (`$prefix`_pipe[`$bdepth - 1`] != -1) {
      /* There is some data to be pushed out */
      int rptr = `$prefix`_pipe[`$bdepth - 1`]; 
; for ($i = 0; $i < $words; $i++) {
      data[`$i`] = `$prefix`_fifo[rptr][`$i`];
; }
      `$prefix`(`$prefix_link`,
      `$prefix_space` `$prefix_core`,
      `$prefix_space` `$prefix_device`,
      `$prefix_space` data);
; if ($stall) {
      iss4_schedule_for_end_of_global_cycle(info->core,
                                            `$name`_check_stall,
                                            info);
      return;
; } else {
      `$prefix`_entries ++;
; }
    }

    /* Cycle the index pipe */
    for (i = `$bdepth - 1`; i > 0; i--) {
      `$prefix`_pipe[i] = `$prefix`_pipe[i-1];
    }
    `$prefix`_pipe[0] = -1;
    `$prefix`_write_pipeline >>= 1;
; if ( $debug ) {
    fprintf(stdout, "write_pipe %d { ", `$prefix`_write_pipeline);
    for (i = 0; i < `$bdepth`; i++) {
      fprintf(stdout, "%d ", `$prefix`_pipe[i]);
    }
    fprintf(stdout, "}\n");
; }

    if (`$prefix`_write_pipeline)
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_push_external,
                                          info);
}

; ################################################################
; # Push semantic data into internal buffer after checking conditions
; # May have to invoke stage semantic again to evaluate kill
; ################################################################
static void
`$name`_push_internal(dll_data_ptr_t info)
{
    int i;
    unsigned pipeline = `$prefix`_write_pipeline;
; if ( $bdepth > 0 ) {
    unsigned wptr = `$prefix`_wptr;
    unsigned entries = `$prefix`_entries;
    int stage_killed = iss4_stage_killed(info->core, `$stage`);
    int killpipe_w = iss4_killpipe_w(info->core);
    if (stage_killed || killpipe_w) {
; if ($debug) {
      fprintf(stdout, "TIEWIRE: `$name`_push_internal : stage_killed %d "
              "killpipe_w %d, don't push data in internal buffer\n",
              stage_killed, killpipe_w);
; }
      `$prefix`_def_C`$stage` = 0;
      return;
    }
; for ($i = 0; $i < $words; $i++) {
    `$prefix`_fifo[wptr][`$i`] = `$prefix`_semantic_data[`$i`];
; }
    `$prefix`_wptr = ( wptr + 1 ) % `$bdepth`;
    `$prefix`_entries = entries - 1;
    /* Insert this entry in the pipe */
    for (i = `$bdepth-2`; i >= 0; i--) {
      if (`$prefix`_pipe[i] == -1) {
    	`$prefix`_pipe[i] = wptr;
    	`$prefix`_write_pipeline = 1 <<  (`$bdepth-1` - i);
	break;
      }
    }
; if ($debug) {
    fprintf(stdout, "TIEWIRE: `$name`_push_internal : "
            "Push data %x to buffer, entries = %d, wptr = %d\t", 
    	    `$prefix`_semantic_data[0], `$prefix`_entries, `$prefix`_wptr);
    fprintf(stdout, "write_pipeline old = %d new = %d { ", 
            pipeline, `$prefix`_write_pipeline);
; for ($i = 0; $i < $bdepth; $i++) {
    fprintf(stdout, "%d ", `$prefix`_pipe[`$i`]);
; }
    fprintf(stdout, "}\n");
; }

    if (pipeline == 0)
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_push_external,
                                          info);
; } else {
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` data);
; if ($type eq 'C') {
    `$prefix`_write_pipeline = (1 << 1);
    if (pipeline == 0)
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_push_external,
                                          info);
; }
; }
; if ($type eq 'D') {
    `$prefix`_def_C`$stage` = 0;
; }
    if ( info->events ) 
	iss4_tieport_event(info->core, `$stage`, "`$iname`", "<-", `$width`, `$prefix`_semantic_data);
}

;   #This function is needed just to flop the NOTRDY signal
static unsigned
`$iname`_internal_buffer_maxcount(dll_data_ptr_t info)
{
    return `$bdepth`;
}

; ################################################################
; # Push semantic data into internal buffer after checking conditions
; # May have to invoke stage semantic again to evaluate kill
; ################################################################
static int
`$iname`_internal_buffer_push(dll_data_ptr_t info, const unsigned *src)
{
; if ($bdepth > 0) {
    unsigned entries = `$prefix`_entries;
    if (entries) {
      unsigned wptr = `$prefix`_wptr;
; for ($i = 0; $i < $words; $i++) {
      `$prefix`_fifo[wptr][`$i`] = src[`$i`];
; }
      `$prefix`_wptr = (wptr + 1) % `$bdepth`;
      `$prefix`_entries = entries - 1;
; if ($debug) {
      fprintf(stdout, "TIEWIRE: `$iname`_internal_buffer_push: "
              "Push data %x to buffer, entries = %d, wptr = %d\t", 
    	      src[0], `$prefix`_entries, `$prefix`_wptr);
; }
      return 1;
    }
    return 0;
; } else { # !(bdepth > 0)
    return 0;
; }
}

static int
`$iname`_internal_buffer_pop(dll_data_ptr_t info, unsigned *dst)
{
; if ($bdepth > 0) {
    unsigned entries = `$prefix`_entries;
    if (entries != `$bdepth`) {
      unsigned rptr = (`$prefix`_wptr + `$bdepth` - entries) % `$bdepth`;
; for ($i = 0; $i < $words; $i++) {
      dst[`$i`] = `$prefix`_fifo[rptr][`$i`];
; }
      `$prefix`_entries = entries + 1;
;# We only clean out the write_pipeline when there are not more
;# entries.  This implies that we need to pop until the queue is
;# empty to maintain the write_pipeline invariant.
      if (`$prefix`_entries == `$bdepth`) {
        int i;
        `$prefix`_write_pipeline = 0;
        for (i = 1; i > 0; i--) {
          `$prefix`_pipe[i] = -1;
        }
      }
; if ( $debug ) {
      fprintf(stdout, "TIEWIRE: `$iname`_internal_buffer_pop: "
              "pop data %x to buffer, entries = %d, rptr = %d\t", 
    	      dst[0], `$prefix`_entries, rptr);
; }
      return 1;
    }
    return 0;
; } else { # !(bdepth > 0)
    return 0;
; }
}

; ################################################################
; # Called from semantic to write interface
; ################################################################
static void
`$name`_write(dll_data_ptr_t info, unsigned *data)
{
    if (iss4_global_stall(info->core) || info->tieportstall) return;
; if ($kill) {

    if (`$prefix`_kill_C`$stage`) return; /* don't push data if killed */
; }

    if (`$prefix`_def_C`$stage`) { 
      /* Someone else has written data in this cycle, just OR data*/
; for ($i = 0; $i < $words; $i++) {
      `$prefix`_semantic_data[`$i`] |= data[`$i`];
; }
    }
    else {
      `$prefix`_def_C`$stage` = 1;
; for ($i = 0; $i < $words; $i++) {
      `$prefix`_semantic_data[`$i`] = data[`$i`];
; }
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_push_internal,
                                           info);
    }
}
; if ( $kill ) {
; ##################################################################
; #Something to kill writes
; ##################################################################
static void
`$kill_name`_reset(dll_data_ptr_t info)
{
    `$prefix`_kill_C`$stage` = 0;
; if ( $debug ) {
    fprintf(stdout, "TIEWIRE: `$kill_name`_reset: kill = %d, def = %x\n",
            `$prefix`_kill_C`$stage`, `$prefix`_def_pipeline);
;}
}

static void
`$kill_name`_write(dll_data_ptr_t info, unsigned *data)
{
    unsigned globalstall = iss4_global_stall(info->core);
    `$prefix`_kill_C`$stage` = data[0];
    if (!globalstall && 
        !info->tieportstall &&
	`$prefix`_kill_C`$stage`) /* reset the def pipe */
      `$prefix`_def_pipeline &= (~(1 << `$commit-$stage`));
; if ( $debug ) {
    fprintf(stdout, "TIEWIRE: `$kill_name`_write: kill = %d, def = %x\n",
            `$prefix`_kill_C`$stage`, `$prefix`_def_pipeline);
;}

    if ( info->events && !globalstall && !info->tieportstall ) 
	iss4_tieport_event(info->core, `$stage`, "`$iname`_KILL", "<-", 1, &`$prefix`_kill_C`$stage`);
    iss4_schedule_for_start_of_free_cycle(info->core,
                                          `$kill_name`_reset,
                                          info);
}
;}#endif kill

; if ( $stall ) {
; ################################################################
; # Stall function called from iss stall function to check 
; # if this interface is causing a globalstall
; ################################################################
static int 
`$name`_stall(dll_data_ptr_t info)
{
    int stall;
; if ($bdepth > 0) {
    int i;
    unsigned defs = 0;

; if ($stall_stage < $commit) {
    /* If there is an interrupt in this cycle, release the stall */
    if (iss4_interrupt_stall_m(info->core)) {
; if ($debug) {
      printf("%s %u: InterruptStall_M, no stall\n", __func__, __LINE__);
; }
      return 0;
    }
; }    
    /* count how many current defs are in the pipeline */
    for (i = `$stage`; i >= `$stall_stage`; i--) {
      defs += (`$prefix`_def_pipeline & (1 << (W_STAGE-i))) != 0;
    }
    stall = (defs > `$prefix`_entries);
; if ($debug) {
    printf("%s %u: def_pipe %s, defs %d, entries %d, stall %d\n",
    	   __func__, __LINE__,
           binvec(`$prefix`_def_pipeline),
           defs, `$prefix`_entries, stall);
; }
; # end ( $bdepth > 0 )
; } else {
    stall = `$name`_stall_expr(info);
; }
; if ($debug) {
    if (stall)
      printf("%s %u: possible `$iname` stall\n", __func__, __LINE__);
; }
; if ($kill) {
    if (stall) { /* need to invoke the semantic to evaluate kill */
; if ($debug) {
      printf("%s %u: evaluate `$iname` kill condition\n", __func__, __LINE__);
; }
      iss4_set_tie_stall_eval(info->core, 1);
      iss4_process_stage(info->core, `$stage`);
      if (`$prefix`_kill_C`$stage`) {
        stall = 0;
; if ($debug) {
        printf("%s %u: `$iname` stall is killed\n", __func__, __LINE__);
; }
      }
      iss4_set_tie_stall_eval(info->core, 0);
    }
; }
    return stall;
}
; } #endif stall
; # end ( $bdepth > 0)
; } elsif ( $type eq 'C' ) {
; # ########################################################################
; # THIS SECTION IS FOR OUTPUT INTERFACES THAT HAVE A SIMPLE BUFFER FOR
; # CONTROL. IN THIS CASE WE HAVE A COUNTER THAT COUNTS WRITE REQUESTS.
; # THIS IS INCREMENTED WHEN INSTR ISSUED AND DECREMENTED WHEN WRITE GOES
; # OUT. IF THERE IS NO HANDSHAKE SIGNAL, THEN DEFAULT VALUE IS WRITTEN
; # WHEN THERE ARE NO MORE WRITES.
; # ########################################################################
; if ( $buf ) {
; ################################################################
; # Initialize write counter
; ################################################################
static void
`$name`_init(dll_data_ptr_t info)
{
  `$prefix`_write_pipeline = 0;
  `$prefix`_def_pipeline = 0;
; if ( $stall ) {
  `$prefix`_stall = 0;
; }
}
; ################################################################
; #If no handshake, generate functions to drive default value 
; ################################################################
; if (!defined($enable)) {
static unsigned `$name`_defval[`$words`] = `$defval`;

static void
`$name`_default(dll_data_ptr_t info)
{
  if (!`$prefix`_write_pipeline)
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`, 
    `$prefix_space` &`$name`_defval[0]);
}
; } #endif enable

; ################################################################
; # idle if there are no writes pending
; ################################################################
static int
`$name`_idle(dll_data_ptr_t info)
{
; if ($debug) {
  printf("%s %u: %s\n", __func__, __LINE__,
	 (`$prefix`_write_pipeline || 
          `$prefix`_def_pipeline) ? "No" : "Yes");
; }
  return !`$prefix`_write_pipeline &&
         !`$prefix`_def_pipeline;
}

static void 
`$name`_def_cycle(dll_data_ptr_t info)
{
  if (`$prefix`_def_pipeline) {
; if ($debug) {
    printf("%s %u: def_pipe %u => %u\n", __func__, __LINE__,
           `$prefix`_def_pipeline,
           `$prefix`_def_pipeline >> 1);
; }
    `$prefix`_def_pipeline >>= 1;
    if (`$prefix`_def_pipeline) {
; if ($debug) {
      printf("%s %u: reschedule for next cycle\n", __func__, __LINE__);
; }
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_def_cycle,
                                           info);
    }
  }
}

; ################################################################
; # A write cycles thru the pipeline until it reaches the write 
; # stage, i.e. pipeline is 1. The transition from 1 to 0 is done
; # when you send the write out.
; ################################################################
static void 
`$name`_write_cycle(dll_data_ptr_t info)
{
  if (`$prefix`_write_pipeline) {
    if (`$prefix`_write_pipeline & 1) {
      /* an instruction is waiting to write */
; if ($debug) {
      printf("%s %u: write pending, no reschedule\n", __func__, __LINE__);
; }
      return;
    }
; if ($debug) {
    printf("%s %u: write_pipe %u => %u\n", __func__, __LINE__,
           `$prefix`_write_pipeline,
           `$prefix`_write_pipeline >> 1);
; }
    `$prefix`_write_pipeline >>= 1;
    if (`$prefix`_write_pipeline > 1) {
; if ($debug) {
      printf("%s %u: reschedule for next cycle\n", __func__, __LINE__);
; }
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_write_cycle,
                                           info);
    }
; if ($debug) {
  } else {
      printf("%s %u: no more writes\n", __func__, __LINE__);
; }
  }
}

; ################################################################
; # Set write pipeline, called set_def so tc does not need to know
; # the difference 
; ################################################################
static void
`$name`_set_def(dll_data_ptr_t info)
{
  unsigned write_pipe = `$prefix`_write_pipeline;
  unsigned def_pipe = `$prefix`_def_pipeline;
  `$prefix`_write_pipeline |= (1 << `$stage`);
  `$prefix`_def_pipeline |= (1 << `$stage`);
; if ($debug) {
  printf("%s %u: write_pipe %u => %u, def_pipe %u => %u\n", __func__, __LINE__,
          write_pipe, `$prefix`_write_pipeline,
          def_pipe, `$prefix`_def_pipeline);
; }
  if (write_pipe <= 1) { /* otherwise, write_cycle is already active */
    iss4_schedule_for_end_of_stall_cycle(info->core,
                                         `$name`_write_cycle,
                                         info);
; if ($debug) {
    printf("%s %u: schedule write_cycle\n", __func__, __LINE__);
; }
  }
  if (def_pipe == 0) { /* otherwise, def_cycle is already active */
    iss4_schedule_for_end_of_stall_cycle(info->core,
                                         `$name`_def_cycle,
                                         info);
; if ($debug) {
    printf("%s %u: schedule def_cycle\n", __func__, __LINE__);
; }
  }
}

; ################################################################
; # Kill writes, assuming fs will never be > commit stage
; ################################################################
static void 
`$name`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
  int i;
  if (`$prefix`_write_pipeline && fs <= `$stage`) {
    for (i = fs; i <= ts && i<= `$stage`; i++) {
      `$prefix`_write_pipeline &= (~(1 << (`$stage` - i)));
      `$prefix`_def_pipeline &= (~(1 << (`$stage` - i)));
    }
; if ($debug) {
    printf("%s %u: (%d..%d) write_pipe = %u, def_pipe = %u\n",
           __func__, __LINE__, fs, ts,
          `$prefix`_write_pipeline,
          `$prefix`_def_pipeline);
; }
  }
}

; if ( $kill ) {
static void
`$kill_name`_write(dll_data_ptr_t info, unsigned *data)
{
  if (!info->tieportstall && (`$prefix`_write_pipeline & 0x1)) {
    unsigned kill = data[0];
    if (kill) 
      `$prefix`_write_pipeline &= ~(0x1);
; if ( $debug ) {
    printf("%s %u: kill = %d, write_pipeline = %u\n", __func__, __LINE__,
            kill, `$prefix`_write_pipeline);
; }
  }
}
; }

; ################################################################
; # If there is a stall condition, we have to check if it is true.
; # If 1, then the write was not accepted. Call yourself next cyc
; # If 0, write accepted, decrement writes counter.
; ################################################################
; if ( $stall ) {
#ifndef _STALL_STATE
#define NOSTALL 0
#define STALLNEXTSTAGE 1
#define STALLNEXTCYC 2
#define STALLED 3
#define INTERRUPTSTALL 4
#endif
; ################################################################
; # Function that returns the stall computation done at the end
; # of the previous cycle in check write. This is called from
; # ISS stall function.
; ################################################################
static int 
`$name`_stall(dll_data_ptr_t info)
{
  int interrupt = iss4_interrupt_stall_m(info->core);
  int stall = (`$prefix`_stall != NOSTALL &&
               `$prefix`_stall != STALLNEXTSTAGE &&
               `$prefix`_stall != INTERRUPTSTALL);
  if (`$prefix`_stall == STALLNEXTCYC) {
    `$prefix`_stall = STALLED;
  } 
  /* Check if stall is being interrupted, for timing reasons the hardware does
     not interrupt the stall in the same cycle. This is different from Q. */
  if (stall && interrupt) {
    `$prefix`_stall = INTERRUPTSTALL;
  } 
; if ($debug ) {
  printf("%s %u: stall %d => %d, interrupt %d\n", __func__, __LINE__,
         stall, `$prefix`_stall, interrupt);
; }
  return stall;
}

static void `$name`_check_write(dll_data_ptr_t info);

static void
`$name`_write_external(dll_data_ptr_t info)
{
; if ( $stage < $commit ) {
  if (iss4_killpipe_w(info->core)) { 
    `$prefix`_stall = NOSTALL;
    `$prefix`_write_pipeline  = 0; 
; if ($debug) {
    printf("%s %u: KillPipe_W, write_pipe = 0\n", __func__, __LINE__);
; }
    return;
  }
; }
  /* Send the write */
  if (`$prefix`_write_pipeline & 0x1) {
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` &`$prefix`_semantic_data[0]);
; if ($debug) {
    printf("%s %u: sending data 0x%x\n", __func__, __LINE__,
           `$prefix`_semantic_data[0]);
  } else {
    printf("%s %u: request dropped, not sending data\n", __func__, __LINE__);
; }
  }
  iss4_schedule_for_end_of_global_cycle(info->core,
                                        `$name`_check_write,
                                        info);
}

; ################################################################
; # Function that is called at the end of a cycle to check if
; # the write sent earlier in the cycle was accepted.
; # If not, set the stall var in the info struct to 1, indicating
; # next cycle will be a GlobalStall.
; ################################################################
static void
`$name`_check_write(dll_data_ptr_t info)
{
  int globalstall =  iss4_global_stall(info->core);
  unsigned stall_state = `$prefix`_stall;
  int earlystall`$stage` = globalstall && (stall_state != STALLED) ;
  int stall = `$name`_stall_expr(info);
; if ($stage < $commit) {
  int killpipe_w = iss4_killpipe_w(info->core);
  int interrupt_m = iss4_interrupt_stall_m(info->core);
  if (killpipe_w) { 
    `$prefix`_stall = NOSTALL;
; if ($debug) {
    printf("%s %u: killed by KillPipe_W\n", __func__, __LINE__);
; }
    return;
  }
  if (interrupt_m) {
    `$prefix`_write_pipeline &= ~(0x1);
    /* drop req next cycle */
  }
; } #endif (stage < commmit)
  if (stall) {
    if (stall_state == NOSTALL) {
      `$prefix`_stall =
        globalstall ? STALLNEXTSTAGE : STALLNEXTCYC;
    } 
    else if (stall_state == STALLNEXTSTAGE) {
      `$prefix`_stall =
        globalstall ? STALLNEXTSTAGE : STALLNEXTCYC;
; if ($stage < $commit) {
      if (interrupt_m)
        `$prefix`_stall = NOSTALL;
; }
    } 
    else if (`$prefix`_stall == STALLNEXTCYC) {
; if ($stage < $commit) {
      if (interrupt_m)
        `$prefix`_stall = NOSTALL;
; }
    }
    else if (`$prefix`_stall == INTERRUPTSTALL) {
      `$prefix`_stall = NOSTALL;
      `$prefix`_write_pipeline &= ~(0x1);
      /* drop req next cycle */
    }
    if (`$prefix`_stall != NOSTALL) {
      iss4_schedule_for_start_of_free_cycle(info->core,
                                            `$name`_write_external,
                                            info);
    }
; if ( $debug ) {
;   my $interrupt = ($stage < $commit) ? "interrupt_m" : "0";
    printf("%s %u: write !OK, interrupt %d, stall %d => %d, "
           "write_pipe %d\n", __func__, __LINE__, `$interrupt`, stall_state,
           `$prefix`_stall,
           `$prefix`_write_pipeline);
; } #endif debug
  }
  else {
    `$prefix`_stall = NOSTALL;
    `$prefix`_write_pipeline &= ~(0x1);
    /* This part is tricky. We are trying to emulate a state machine
     * Normally, the write not being accepted results in a stall in
     * `($stage+1)` cycles in which case the stall state old will be STALLED. 
     * If it is the case that we are still stuck in the `$stage` due to another 
     * globalstall, then stall state will be STALLNEXTSTAGE, or STALLNEXTCYC
     * in the last globalstall cycle. In this case, although this write 
     * is successful, we cannot rotate the write pipeline to let the next write 
     * request move forward, because the pipeline has not advanced yet. So wait
     * until the defcycle clocks in the next stall clock to rotate the pieline.
     */
    if (earlystall`$stage` == 0) {
      `$prefix`_write_pipeline >>= 1;
    }
; if ($rdwrfunc) {
    /* call the function that triggers some action based on the write */
    `$rdwrfunc`(info);
; }
    if (`$prefix`_write_pipeline > 1) {
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_write_cycle,
                                           info);
; if ($debug) {
      printf("%s %u: write OK, stall %d, write_pipe %d, "
             "reschedule `$name`_write_cycle\n", 
             __func__, __LINE__,
             `$prefix`_stall,
             `$prefix`_write_pipeline);
; } #endif debug
    }
; if ($debug) {
    else {
      printf("%s %u: write OK, stall %d, write_pipe %d\n",
             __func__, __LINE__,
            `$prefix`_stall, 
            `$prefix`_write_pipeline);
    }
; } #endif debug
  }
}
;}

; ################################################################
; # Semantic is sending a write. This could be under globalstall.
; # If write has already been sent, do nothing. This is what
; # makes a lookup write go out in the first globalstall cycle.
; # If write is > 1, someone ahead of you hasn't written yet,
; # so ignore and return;
; # If there is exactly one write, copy the semantic data
; # send out the write.
; # If no stall, decerement #writes and return
; # If stall, check if write was accepted, if not keep
; # repeating the write.
; ################################################################
static void
`$name`_write(dll_data_ptr_t info, unsigned *data)
{
; if ( $stage <= $commit ) {
  if (iss4_killpipe_w(info->core)) { 
; if ($stall) {
    `$prefix`_stall = NOSTALL;
; }
; if ($debug) {
    printf("%s %u: no-op due to KillPipe_W\n", __func__, __LINE__);
; }
    return;
  }
; }
  if (info->tieportstall) {
; if ($debug) {
    printf("%s %u: no-op due to tieportstall\n", __func__, __LINE__);
; }
    return;
  }
; if ($stall) {
  /* Check if the write already happened (during GlobalStall) */
  /* or if there is an older write still waiting */
  if (!(`$prefix`_write_pipeline & 0x1) || 
      `$prefix`_stall != NOSTALL)
; } else {
  /* Check if the write already happened (during GlobalStall) */
  if (!(`$prefix`_write_pipeline & 0x1))
; }
  {
; if ($debug) {
    printf("%s %u: no-op due to %s\n", __func__, __LINE__,
            ((`$prefix`_write_pipeline & 1) ? 
            "older write still waiting" : 
            "write already done (GlobalStall)"));
; }
    return;
  }

; for ($i = 0; $i < $words; $i++) {
  `$prefix`_semantic_data[`$i`] = data[`$i`];
; }
; if ($debug) {
  printf("%s %u: latch semantic data 0x%x\n", __func__, __LINE__,
         `$prefix`_semantic_data[0]);

; }
; if ($stall) {
  `$name`_write_external(info);
; } else {
  `$prefix`(`$prefix_link`,
  `$prefix_space` `$prefix_core`,
  `$prefix_space` `$prefix_device`, 
  `$prefix_space` &`$prefix`_semantic_data[0]);
  `$prefix`_write_pipeline &= ~(0x1); /* clear stage bit */
; if ($debug) {
  printf("%s %u: writing data 0x%x, write_pipe %u\n", __func__, __LINE__,
         `$prefix`_semantic_data[0],
         `$prefix`_write_pipeline);


; }
; if ($rdwrfunc) {
  `$rdwrfunc`(info);
; }
; } #endif stall
   if ( info->events ) 
	iss4_tieport_event(info->core, `$stage`, "`$iname`", "<-", `$width`, `$prefix`_semantic_data);
} 
; } else { #simple control interface
; # ########################################################################
; # THIS SECTION IS FOR SIMPLE CONTROL OUTPUT INTERFACES
; # ########################################################################
static unsigned `$name`_defval[`$words`] = `$defval`;

static void `$name`_default(dll_data_ptr_t info)
{
    if (`$prefix`_write_pipeline == 0)
      `$prefix`(`$prefix_link`,
      `$prefix_space` `$prefix_core`,
      `$prefix_space` `$prefix_device`,
      `$prefix_space` &`$name`_defval[0]);
    }
}

static void
`$name`_init(dll_data_ptr_t info)
{
    `$prefix`_write_pipeline = 0;
}

static void
`$name`_push_external(dll_data_ptr_t info)
{
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` &`$prefix`_semantic_data[0]);
    `$prefix`_write_pipeline >>= 1;
; if ( $debug ) {
    fprintf(stdout, "In `$name`_push_external, writing data %x, write_pipeline = %d\n",
        `$prefix`_semantic_data[0], `$prefix`_write_pipeline);
; }
     iss4_schedule_for_end_of_free_cycle(info->core,
                                         `$name`_default,
                                         info);
}

static void
`$name`_write(dll_data_ptr_t info, unsigned *data)
{
    unsigned pipeline = `$prefix`_write_pipeline;
    if ( info->tieportstall ) return;
; if ( $stage <= $commit ) {
    /* now that the semantic is called during globalstall,
     * the interface function should check that there is
     * no killpipe.
     */
    if ( iss4_killpipe_w(info->core) ) return;
; }
;   for ($i = 0; $i < $words; $i++) {
       `$prefix`_semantic_data[`$i`] = data[`$i`];
;   }
    `$prefix`_write_pipeline = 1;
; if ( $debug ) {
    fprintf(stdout, "In `$name`_write, writing data %x, write_pipeline old = %d, new %d\n",
        `$prefix`_semantic_data[0], pipeline, `$prefix`_write_pipeline);
; }
    if ( pipeline == 0 ) {
        iss4_schedule_for_end_of_stall_cycle(info->core,
                                             `$name`_push_external,
                                             info);
    }
}

; } #endd $buf
; } # end ( $bdepth == 0 ) 
; } #end out
; ##########################################################################
; if ( $dir eq 'in' ) {
; # ########################################################################
; # Input Interface code
; # This code is not very general, for example the stall functions assume
; # that stall happens to only Data type interfaces with speculative buffers.
; # ########################################################################
; if ( $kill || $ekill ) {
; #/* ****************************************************************************
; #   Major ugliness to match RTL because of weird behavior that a "killed"
; #   Q access can become unkilled if there was some instruction behind it
; #   which was trying to pop data.
; #   Lower 4 bits are used for the status of the read in the M stage.
; #   There are 4 states 
; #   0 - no reads cancelled
; #   1 - read cancelled due to kill
; #   2 - read cancelled due to kill but globalstall in `$stage` and PopReq still high 
; #       so you may have data anyway, don't decrement the "read" counter
; #   3 - read cancelled due to kill but there is/was globalstall in `$stage` and the
; #       instruction behind has popped data, so this instruction either uses it or
; #	  sets staging killed to be 1.
; #   4 - Request was killed in the E stage, so there should be no valid read.
; #  The next 4 bits are used for the state of the read in the E stage
; #   16 - request cancelled with early kill to kill pop in the E stage
; #**********************************************************************************/

#ifndef _CANCEL_STATE
#define _CANCEL_STATE
#define NO_CANCEL        0
#define CANCEL           1
#define CANCEL_RETRY     2
#define CANCEL_DATA      3
#define CANCEL_REQ_READ  4
#define CANCEL_REQUEST  16
#endif
; }
;  if ( $stall ) {

static int
`$name`_stall(dll_data_ptr_t info)
{
    int stall;
; my $nocancelrequest;
; $nocancelrequest = $ekill ? " && cancel != CANCEL_REQUEST " : "";
; if ($ekill || ($kill && $debug)) {
    int cancel = `$prefix`_cancel_read;
; }
; if ($buf) {
    if (`$prefix`_srptr != `$prefix`_wptr`$nocancelrequest`) {
      /* Buffer has valid data */
      int sentries = (`$prefix`_wptr > `$prefix`_srptr) ? 
                     (`$prefix`_wptr - `$prefix`_srptr) :
                     (`$bdepth`+(`$prefix`_wptr - `$prefix`_srptr));
;   if ($debug) {
      printf("%s %u: specbuf has data, no stall\n", __func__, __LINE__);
;   }
      if (sentries == 1) {
        `$prefix`_read_last_sentry = 1;
;   if ($debug) {
        printf("%s %u: reading last specbuf entry\n", __func__, __LINE__);
;   }
      }
      return 0;
    }
; } # if ( $buf )

; if ( $reg ) { # registered case, stall only when no staging data 
    stall = !`$prefix`_staging_valid;
;   if ($debug) {
    if (stall)
      printf("%s %u: no data in staging buf, possible stall\n",
             __func__, __LINE__);
;   }
; } else {
    stall = `$name`_stall_expr(info);
; }
; if ($ekill) {
    if (`$prefix`_cancel_read == CANCEL_REQUEST) {
      `$prefix`_cancel_read = CANCEL_REQ_READ;
;   if ($debug) {
      printf("%s %u: request was e-killed, no stall, cancel %d => %d\n", 
             __func__, __LINE__, cancel, `$prefix`_cancel_read);
;   }
      return 0;
    } else if ( `$prefix`_cancel_read == CANCEL_REQ_READ ) {
;   if ($debug) {
      printf("%s %u: reads canceled, no stall, cancel %d\n", 
             __func__, __LINE__, `$prefix`_cancel_read);
;   }
      return 0;
    }
; }
; if ($kill) {
    `$prefix`_cancel_read = NO_CANCEL;

    if (stall`$nocancelrequest`) { /* invoke stage function to evaluate kill */
;   if ($debug) {
      printf("%s %u: evaluate `$iname` kill condition\n", __func__, __LINE__);
;   }
      iss4_set_tie_stall_eval(info->core, 1);
      iss4_process_stage(info->core, `$use_stage`);
      if (`$prefix`_kill_C`$use_stage`) {
        stall = 0;
        if (`$prefix`_reads > 0)
          `$prefix`_cancel_read = CANCEL;
;   if ($debug) {
	printf("%s %u: `$iname` stall is killed, read count %u, "
               "cancel %d => %d\n", __func__, __LINE__,
               `$prefix`_reads, cancel, `$prefix`_cancel_read);
;   }
      }
      iss4_set_tie_stall_eval(info->core, 0);
    }
;}

    /* If there is an interrupt in this cycle, release the stall */
    if (stall && iss4_interrupt_stall_m(info->core)) {
;   if ($debug) {
      printf("%s %u: InterruptStall_M, no stall\n", __func__, __LINE__);
;   }
      stall = 0;

      /* During InterruptStall_M, if there is a GlobalStall for other reasons,
       * we need to revisit `$name`_stall in case kill conditions change.
       */
      if (iss4_global_stall(info->core))
        iss4_schedule_for_start_of_free_cycle(info->core,
                                              `$name`_stall,
                                              info);
    }

    `$prefix`_stall = stall;
    return stall;
}
; } # endif stall
; if ( $buf ) {
;  #############################################################################
;  # Speculative buffer implementation for data type interfaces
;  #############################################################################
; if ( $type eq 'D' ) {

#ifndef _STAGING_KILL_STATE
#define _STAGING_KILL_STATE
#define STAGING_NOTKILLED 0
#define STAGING_KILLPIPE 1
#define STAGING_CONDKILL 2

#define STAGING_KILLED_COND(m) ((m) & 0x2)
#define STAGING_KILLED(m)       (m)
#endif

static void
`$name`_init(dll_data_ptr_t info)
{
    int i;
    for (i = `$pdepth - 1`; i >= 0; i--) {
      `$prefix`_pipe[i] = -1;
    }
    for (i = `$bdepth - 1`; i >= 0; i--) {
;   for ($i = 0; $i < $words; $i++) {
      `$prefix`_fifo[i][`$i`] = 0;
;   }
    }
;   if ( $reg ) {
;    for ($i = 0; $i < $words; $i++) {
    `$prefix`_staging_data[`$i`] = 0;
;    }
    `$prefix`_staging_valid = 0;
    `$prefix`_reads = 0;
    `$prefix`_staging_killed = 0;
    `$prefix`_read_last_sentry = 0;
;   }
    `$prefix`_rptr = 0;
    `$prefix`_srptr = 0;
    `$prefix`_wptr = 0;
    `$prefix`_use_pipeline = 0;
;   if ( $stall ) {
    `$prefix`_notrdy = 0;
    `$prefix`_stall = 0;
;   }
;   if ( $kill || $ekill ) {
    `$prefix`_cancel_read = NO_CANCEL;
;   }
}

static void
`$name`_cycle(dll_data_ptr_t info)
{
    int i;

    if (`$prefix`_use_pipeline) {
      /* Take out the one that is committing, update rptr */
      int commit_ptr = `$prefix`_pipe[W_STAGE];
      if (commit_ptr != -1) {
	unsigned new_rptr = (commit_ptr + 1) % `$bdepth`;
;   if ($debug) {
        printf("%s %u: commit index %d [ %x ] rptr %d => %d\n",
               __func__, __LINE__, commit_ptr,
               `$prefix`_fifo[commit_ptr][0], 
               `$prefix`_rptr, new_rptr);
;   }
	`$prefix`_rptr = new_rptr;
      }
      /* Cycle the index pipe */
      for (i = `$pdepth - 1`; i > 0; i--) {
        `$prefix`_pipe[i] = `$prefix`_pipe[i-1];
      }
      `$prefix`_pipe[0] = -1;
      `$prefix`_use_pipeline >>= 1;

      if (`$prefix`_use_pipeline)
        iss4_schedule_for_end_of_stall_cycle(info->core,
                                             `$name`_cycle,
                                             info);
;   if ($debug) {
      printf("%s %u: use_pipe = %s  { ", __func__, __LINE__,
              binvec(`$prefix`_use_pipeline));
      for (i = 0; i < `$pdepth`; i++)
        printf("%d ", `$prefix`_pipe[i] );
      printf("}\n");
;   }
    }
}

static void
`$name`_set_use(dll_data_ptr_t info)
{
    unsigned old = `$prefix`_use_pipeline;
    `$prefix`_use_pipeline |= (1 << `$pdepth-1`);
; if ($debug) {
    printf("%s %u: use in stage %u, use_pipe = %s\n", __func__, __LINE__,
           `$pdepth-1`, binvec(`$prefix`_use_pipeline));
; }
    if (old == 0)
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_cycle,
                                           info);
}

/* Called when pipeline stages [fs..ts] are killed.
 * Update srptr to pipe[fs]. Make pipe invalid from fs to ts.
 */
static void
`$name`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
    int i;

; if ( $reg ) {
    if (fs > `$use_stage`) {
; if ( $debug ) {
      if (`$prefix`_reads)
        printf("%s %u: stages %d..%d, read count = 0\n",
               __func__, __LINE__, fs, ts);
; }
      `$prefix`_reads = 0; 
      if (`$prefix`_staging_valid) { 
        /* Hold this data, clear bit for cond kill if present */
        `$prefix`_staging_killed = STAGING_KILLPIPE; 
      }
; if ( $kill || $ekill ) {
      `$prefix`_cancel_read = NO_CANCEL;
; }
    }
    if (fs == `$use_stage` && `$prefix`_read_last_sentry) {
      `$prefix`_read_last_sentry = 0;
    }
; } # if ( $reg )
    /* If there is an instr in the pipeline in the stage   */
    /* that is being killed, set the srptr back correctly. */
    if (`$prefix`_use_pipeline && 
        `$prefix`_pipe[fs] != -1) {
      if (fs > `$use_stage`)
        `$prefix`_srptr = `$prefix`_pipe[fs];
      for (i = fs; i <= ts; i++) {
        `$prefix`_use_pipeline &= ~(1 << (W_STAGE - i)); 
        `$prefix`_pipe[i] = -1;
      }
; if ( $debug ) {
      printf("%s %u: stages %d..%d, staging_killed %d, srptr %d, use_pipe %d {",
             __func__, __LINE__, fs, ts, `$prefix`_staging_killed,
             `$prefix`_srptr, `$prefix`_use_pipeline);
      for (i = 0; i < `$pdepth`; i++)
        printf("%d ", `$prefix`_pipe[i] );
      printf("}\n");
; }
    }
}

;   #This function is needed just to flop the NOTRDY signal
static int
`$iname`_internal_buffer_pop(dll_data_ptr_t info, unsigned *dst)
{
    unsigned srptr = `$prefix`_srptr;
    if (srptr == `$prefix`_wptr) {
      if (!`$prefix`_staging_valid) {
        return 0;
      } 
      else {
;   for ($i = 0; $i < $words; $i++) {
        dst[`$i`] = `$prefix`_staging_data[`$i`];
;   }
        `$prefix`_staging_valid = 0;
      }
    } 
    else {
      /* There is valid data in the buffer, read it */
;  for ($i = 0; $i < $words; $i++) {
      dst[`$i`] = `$prefix`_fifo[srptr][`$i`];
;  }
      `$prefix`_srptr = (srptr + 1) % `$bdepth`;
    }
    return 1;
}

;   #this function returns the max number of buffers that could be full at
;   # a time.
static unsigned
`$iname`_internal_buffer_maxcount(dll_data_ptr_t info)
{
    return `$bdepth`;
}

;   #This function is needed to functionally push data into the internal buffer
static int
`$iname`_internal_buffer_push(dll_data_ptr_t info, const unsigned *src)
{
    unsigned wptr = `$prefix`_wptr;
    if ((wptr + 1) % `$bdepth` == `$prefix`_srptr) {
      if (`$prefix`_staging_valid) {
        return 0;
      }
;   for ($i = 0; $i < $words; $i++) {
      `$prefix`_staging_data[`$i`] = src[`$i`];
;   }
      `$prefix`_staging_valid = 1;
    } 
    else {
;   for ($i = 0; $i < $words; $i++) {
      `$prefix`_fifo[wptr][`$i`] = src[`$i`];
;   }
      `$prefix`_wptr = (wptr + 1) % `$bdepth`;
    }
    return 1;
}

;   if ( $stall ) {
/* This function is to flop the external stall to generate NOTRDY
   to match RTL. Otherwise there is a comb path in RTL between
   Empty->NOTRDY->KILL->GlobalStall-> all fanouts
*/
static int
`$iname`_NOTRDY_external_stall(dll_data_ptr_t info) 
{
  `$prefix`_ext_stall = `$name`_stall_expr(info);
; if ($debug){
    printf("%s %u: `$iname`_external_stall = %d\n",
           __func__, __LINE__, `$prefix`_ext_stall);
; }
; if ( $clkgate == 0 ) {
  //This is to handle the case where G1SCLK is going under GlobalStall
  //i.e. the case of ClkGateGlobal 0.
  if (iss4_global_stall(info->core))
   iss4_schedule_for_end_of_free_cycle(info->core, `$iname`_NOTRDY_external_stall, info);
; }
  return 0;
}

static int
`$iname`_NOTRDY_interface_generate(dll_data_ptr_t info) 
{
    int notrdy = (`$prefix`_srptr == `$prefix`_wptr) &&
                 !`$prefix`_staging_valid;

; if ( $ekill ) {
    if ( (`$prefix`_use_pipeline & (1 << `$commit-$use_stage`)) && 
	 (`$prefix`_cancel_read != CANCEL_REQUEST ) ) 
; } else {
    if (`$prefix`_use_pipeline & (1 << `$commit-$use_stage`))
; }
      `$prefix`_notrdy = notrdy;
    else
      `$prefix`_notrdy = notrdy && `$prefix`_ext_stall; /* use flopped `$name`_stall_expr(info); */

; if ($debug){
    printf("%s %u: `$iname`_notrdy = %d\n",
           __func__, __LINE__, `$prefix`_notrdy);
; }

    return 0;
}

static void
`$iname`_NOTRDY_interface_read(dll_data_ptr_t info, unsigned *result)
{
; if ($debug) {
    printf("%s %u: `$iname`_notrdy %d\n",
           __func__, __LINE__, `$prefix`_notrdy);
; }
    *result = `$prefix`_notrdy;

    if (iss4_global_stall(info->core)) {
      //We need to generate the NOTRDY interface every free clk cycle because in the case
      //where NOTRDY is accompanied by a read of the Q, the value MAY change if we 
      //read data while waiting. However, flopping of the Empty signal (which will be
      //used by NOTRDY ONLY in the case where there is no accompanying Q read should
      //be done at stall clk, NOT free clk. Note that the flopping of the Empty on stall
      //clk under global stall might seem un-necessary but we do it for no global clkgate case
      iss4_schedule_for_start_of_free_cycle(info->core, `$iname`_NOTRDY_interface_generate, info);
; if ( $clkgate == 0 ) {
      iss4_schedule_for_end_of_free_cycle(info->core, `$iname`_NOTRDY_external_stall, info);
;}
    } else { 
      if ( info->events && !info->tieportstall ) 
	iss4_tieport_event(info->core, `$use_stage`, "`$iname`_NOTRDY", "->", 1, &`$prefix`_notrdy);
    }
}

;} # end of if ( $stall )
; if ( $kill ) {
static void 
`$kill_name`_reset(dll_data_ptr_t info)
{
    `$prefix`_kill_C`$use_stage` = 0;
; if ( $debug ) {
    fprintf(stdout, "TIEWIRE: `$kill_name`_reset: kill = %d\n",
            `$prefix`_kill_C`$use_stage`);
;}
}
     
static void
`$kill_name`_write(dll_data_ptr_t info, unsigned *data)
{
    `$prefix`_kill_C`$use_stage` = data[0];
    /* Do not change any other state when just evaluating the stall */
    if (info->tieportstall) return;

    if (`$prefix`_srptr != `$prefix`_wptr) {
      /* Kill the specbuf read */
      if (`$prefix`_kill_C`$use_stage` && `$prefix`_read_last_sentry) {
        `$prefix`_read_last_sentry = 0;
; if ($debug) {
    	printf("%s %u: `$iname` kill last specbuf entry read, read count %d\n",
    	       __func__, __LINE__, `$prefix`_reads);
; }
        /* Also may need to kill staging buffer data if valid */
        if (`$prefix`_staging_valid) {
          if (`$prefix`_cancel_read == CANCEL_DATA)
            `$prefix`_cancel_read = NO_CANCEL;
          if (`$prefix`_cancel_read == NO_CANCEL) {
            `$prefix`_staging_killed |= STAGING_CONDKILL; 
; if ($debug) {
    	    printf("%s %u: `$iname` cond-kill staging buffer data\n",
    	           __func__, __LINE__);
; }
          }
        }
      }
    } 
    else {
      /* Kill/Un-kill staging buffer data */
      if (`$prefix`_kill_C`$use_stage` &&
          `$prefix`_staging_valid &&
          `$prefix`_cancel_read == NO_CANCEL) {
        `$prefix`_staging_killed |= STAGING_CONDKILL; 
      }
      else if (`$prefix`_kill_C`$use_stage` &&
               `$prefix`_staging_valid &&
               `$prefix`_cancel_read == CANCEL_DATA) {
        `$prefix`_staging_killed |= STAGING_CONDKILL; 
        `$prefix`_cancel_read = NO_CANCEL;
      }
      else if (!`$prefix`_kill_C`$use_stage` &&
               `$prefix`_staging_valid && 
               STAGING_KILLED_COND(`$prefix`_staging_killed)) {
        `$prefix`_staging_killed = STAGING_NOTKILLED; 
      }
      else if (!`$prefix`_kill_C`$use_stage` &&
               `$prefix`_staging_valid && 
               `$prefix`_cancel_read == CANCEL) {
        /* read was cancelled but is now valid */
        `$prefix`_cancel_read = NO_CANCEL ; 
      }
; if ( $debug ) {
      printf("%s %u: `$iname`_kill = %d, %s staging data, "
             "cancel = %d, staging_killed = %d\n", 
             __func__, __LINE__, data[0],
             `$prefix`_staging_valid ? "has" : "no",
             `$prefix`_cancel_read, `$prefix`_staging_killed);
; }
    }

    if ( info->events ) 
	iss4_tieport_event(info->core, `$use_stage`, "`$iname`_KILL", "<-", 1, &`$prefix`_kill_C`$use_stage`);
    iss4_schedule_for_start_of_free_cycle(info->core,
                                          `$kill_name`_reset,
                                          info);
}
;}
; if ( $ekill ) {
static void 
`$ekill_name`_reset(dll_data_ptr_t info)
{
    `$prefix`_kill_C`$read_stage` = 0;
; if ( $debug ) {
    	fprintf(stdout, "TIEWIRE: `$ekill_name`_reset : kill = %d, cancel = %d\n", 
		`$prefix`_kill_C`$read_stage`, `$prefix`_cancel_read);
; }
}
     
static void
`$ekill_name`_write(dll_data_ptr_t info, unsigned *data)
{
    `$prefix`_kill_C`$read_stage` = data[0];
    /* Don't change any other state when you are evaluating stalls */
    if ( info->tieportstall ) return;
    iss4_schedule_for_start_of_free_cycle(info->core,
                                          `$ekill_name`_reset,
                                          info);
; if ( $debug ) {
    	fprintf(stdout, "TIEWIRE: `$ekill_name`_write : kill = %d, cancel = %d\n", 
		`$prefix`_kill_C`$read_stage`, `$prefix`_cancel_read);
; }
}
;}
; } else { #type 
; #################################################################################
; # Control type interfaces have plain lookup buffers
; # If the buffereda data is also registered, that WILL NOT WORK currently.
; ################################################################################
static void
`$name`_init(dll_data_ptr_t info)
{
    int i;
    `$prefix`_sample_pipeline = 0;
    `$prefix`_read_pipeline = 0;
    `$prefix`_rptr = 0;
    `$prefix`_wptr = 0;
    for (i = `$bdepth - 1`; i >= 0; i--) {
;   for ($i = 0; $i < $words; $i++) {
	`$prefix`_buf[i][`$i`] = 0;
;   }
    }
; if ( $read_stage < $commit && $stage > $commit ) {
    `$prefix`_use_pipeline = 0;
    `$prefix`_reads = 0;
;}
}

static void
`$name`_sample(dll_data_ptr_t info)
{
    unsigned wptr = `$prefix`_wptr;
    unsigned data[`$words`];
   
    if (`$prefix`_sample_pipeline & 0x1) { /* read new data */
      `$prefix`(`$prefix_link`,
      `$prefix_space` `$prefix_core`,
      `$prefix_space` `$prefix_device`,
      `$prefix_space` &data[0]);
      if (`$prefix`_read_pipeline & 0x1) { 
        /* write the data into the write pointer */
; for ($i = 0; $i < $words; $i ++) {
        `$prefix`_buf[wptr][`$i`] = data[`$i`];
; }
        `$prefix`_wptr = (wptr + 1) % `$bdepth`;
; if ($read_stage < $commit && $stage > $commit) {
        `$prefix`_reads ++;
;}
; if ($debug) {
        printf("%s %u: read new data 0x%x to %d, wptr = %d\n",
               __func__, __LINE__,
    	       `$prefix`_buf[wptr][0], wptr, `$prefix`_wptr);
; if ($read_stage < $commit && $stage > $commit) {
        printf("%s %u: read count = %u\t", __func__, __LINE__,
               `$prefix`_reads);
; }
; }
; if ($debug) {
      } else {
        printf("%s %u: read data & discard, request is no longer valid\n",
               __func__, __LINE__);
; }
      }
    }
    `$prefix`_read_pipeline >>= 1;
    `$prefix`_sample_pipeline >>= 1;
; if ($debug) {
    printf("%s %u: sample_pipe = %u, read_pipe = %u\n", __func__, __LINE__,
	   `$prefix`_sample_pipeline,
           `$prefix`_read_pipeline);
; }
    if (`$prefix`_sample_pipeline)
      iss4_schedule_for_start_of_free_cycle(info->core,
                                            `$name`_sample,
                                            info);
}

; # Wrapper function that simply schedules the read function for the end of a cycle 
static int
`$name`_external_read(dll_data_ptr_t info)
{
    unsigned old_sample = `$prefix`_sample_pipeline;
    unsigned old_read = `$prefix`_read_pipeline;
    `$prefix`_sample_pipeline |= (1<<`$depth-1`);
    `$prefix`_read_pipeline |= (1<<`$depth-1`);
; if ( $debug ) {
    printf("%s %u: sample_pipe: old %x, new %x; read_pipe: old %x, new %x\n",
           __func__, __LINE__,
     	   old_sample, `$prefix`_sample_pipeline,
           old_read, `$prefix`_read_pipeline);
; }
    if (old_sample == 0)
      iss4_schedule_for_start_of_free_cycle(info->core,
                                            `$name`_sample,
                                            info);
    return 0;
}
; ################################################################
; # idle if there are no reads expected back from the external world
; ################################################################
static int
`$name`_idle(dll_data_ptr_t info)
{
; if ( $debug ) {
    fprintf(stdout, "TIEWIRE: In `$name`_idle: read_pipeline %x\n",
            `$prefix`_read_pipeline);
; }
    return (`$prefix`_read_pipeline == 0);
}

; if ( $bdepth > 1 && $read_stage < $commit) {
; if ( $stage > $commit ) {
static void 
`$name`_use_cycle(dll_data_ptr_t info)
{
    if ( `$prefix`_use_pipeline ) {
	`$prefix`_use_pipeline >>= 1;
	if ( `$prefix`_use_pipeline ) {
            iss4_schedule_for_end_of_stall_cycle(info->core,
                                                 `$name`_use_cycle,
                                                 info);
	}
    }
}

static void
`$name`_set_use(dll_data_ptr_t info)
{
    unsigned old = `$prefix`_use_pipeline;
    `$prefix`_use_pipeline |= (1 << `$stage`);
; if ($debug) {
    printf("%s %u: use_pipe %u => %u\n", __func__, __LINE__,
           old, `$prefix`_use_pipeline);
;}
    if (old == 0)
      iss4_schedule_for_end_of_stall_cycle(info->core,
                                           `$name`_use_cycle,
                                           info);
}
; } else {
static void
`$name`_set_use(dll_data_ptr_t info)
{
}
; } 

static void 
`$name`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
; if ( $read_stage < $commit && $stage > $commit ) {
    unsigned i, k, reads = 0, uses = 0;
    for (i = fs; i <= ts; i++) {
      `$prefix`_use_pipeline &= (~(1 << (`$stage` - i)));
      `$prefix`_read_pipeline &= (~(1 << (`$stage` - i)));
    }
; if ( $debug ) {
    printf("%s %u: (%d, %d) read_pipe %u, use_pipe %u\n",
           __func__, __LINE__, fs, ts,
           `$prefix`_read_pipeline,
           `$prefix`_use_pipeline);
; }
    if (fs == W_STAGE) {
      for (i = 0; i < `$stage-$commit`; i++) 
        reads += ((`$prefix`_read_pipeline & (1 << i)) != 0);
      /* we need to skip the bit 0. this stage is executed this cycle
       * but the use pipeline won't shift until the end of the cycle */
      for (i = 1; i < `$stage-$commit`; i++) 
	uses += ((`$prefix`_use_pipeline & (1 <<i )) != 0);
; if ( $debug ) {
      printf("%s %u: already read %u, committed uses %d\n", __func__, __LINE__,
             `$prefix`_reads, uses);
; }
      /* If we have already read some more data than we will use */
      if (`$prefix`_reads > uses) {
        k = (`$prefix`_reads - uses );
	`$prefix`_wptr = (`$prefix`_rptr + uses) % `$bdepth`;
	`$prefix`_reads -= k;
	uses = 0; /* just for next computation*/
; if ( $debug ){
        printf("%s %u: remove %u reads, fix wptr\n", __func__, __LINE__,
               `$prefix`_reads);
; }
      } 
      else {
	uses -= `$prefix`_reads; /* just for next computation*/
      }
; if ( $debug ){
      printf("%s %u: rptr %d, wptr %d, pending reads %d, "
             "committed uses not read %d\n", __func__, __LINE__,
	     `$prefix`_rptr,
             `$prefix`_wptr,
             reads, uses);
; }
      /* we need to remove some pending reads that will not be used */
      if (reads > uses) {
        k = 0;
        for (i = 0; i < `$stage-$commit`; i++) {
	  if ((`$prefix`_read_pipeline & (1 <<i )) != 0) {
            k++;
            if (k > uses && k <= reads) {
              `$prefix`_read_pipeline &= (~(1 << i));
            }
          }
        }
      }
; if ( $debug ){
      printf("%s %u: new read pipeline %i\n", __func__, __LINE__,
             `$prefix`_read_pipeline);
; }
    }
; } else {
    int i;
    for (i = fs; i <= ts; i++) {
      `$prefix`_read_pipeline &= (~(1 << (`$stage` - i)));
    }
    if (fs == W_STAGE) {
      `$prefix`_wptr = `$prefix`_rptr;
      `$prefix`_read_pipeline = 0;
; if ( $debug ){
      printf("%s %u: (%d,%d) rptr = wptr = %d, read_pipeline = %d\n", 
             __func__, __LINE__, fs, ts,
            `$prefix`_wptr,
            `$prefix`_read_pipeline);
; }
    }
;}
}
;} else { #dummmy functions
static void 
`$name`_kill_stage(dll_data_ptr_t info, int fs, int ts)
{
}
static void
`$name`_set_use(dll_data_ptr_t info)
{
}
;}
; } #end of if type
; } # end of if ( $buf )
; #############################################################################
; if ( $reg ) {
;  ############################################################################
;  # Registering for data type interfaces
;  ############################################################################
;   if ( $type eq 'D' ) {

/* This function is scheduled as a stall function but it never actually
   causes a GlobalStall (always returns 0). The stall is generated from
   semantic_read function if the staging buffer is empty. This function
   writes the staging buffer and sets the flag staging_valid to 1.  If
   the stall condition is true it schedules itself again and returns.
   This would cause a stall in the next cycle since staging buffer has
   no data (staging_valid = 0).

   Because this function is called even during globalstall, there may
   be multiple instructions waiting in the pipeline to read from a 
   wire that is stalled. When the stall condition goes away, only one 
   of them should pop. The other one should find that there is already 
   valid data in the staging buffer and schedule itself for the next 
   cycle and return.

   If we were trying to read this wire and an exception killed the
   instruction, this function should stop getting called every cycle.
   It would be called again when the instr gets issued. 
*/

static void `$name`_request(dll_data_ptr_t info);

static void
`$name`_sample(dll_data_ptr_t info)
{
; if ( $kill || $ekill ) {
    /* read request may have been removed but the sample is still scheduled */
    if (`$prefix`_reads == 0) return;
; }
; if ( $stall ) {

    if (`$name`_stall_expr(info)) {
;   if ($debug) {
      printf("%s %u: `$iname` empty, read count %u\n", 
             __func__, __LINE__, `$prefix`_reads);
;   }
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_request,
                                          info);
      return;
    }
; } # end of if ( $stall )

    /* No stall means we are reading valid data */
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` &`$prefix`_staging_data[0]);

    `$prefix`_staging_valid = 1;
    `$prefix`_reads --;
; if ( $kill ) {
    if (`$prefix`_cancel_read == CANCEL_RETRY)
      `$prefix`_cancel_read = CANCEL_DATA;
; }
; if ($debug) {
    printf("%s %u: `$iname` got new data [", __func__, __LINE__);
;   for ($i = 0; $i < $words; $i++) {
    printf(" 0x%x", `$prefix`_staging_data[`$i`]); 
;   }
    printf(" ], read count %u-- => %u\n",
           `$prefix`_reads+1, `$prefix`_reads);
; }

    /* Reschedule until there are no more reads */
    if (`$prefix`_reads > 0)
      iss4_schedule_for_end_of_free_cycle(info->core,
                                          `$name`_request,
                                          info);
}

static void
`$name`_request(dll_data_ptr_t info)
{
    int globalstall =  iss4_global_stall(info->core);
; if ( $kill || $ekill ) {
    int cancel = `$prefix`_cancel_read;
; if ( $ekill ) {

    if (`$prefix`_kill_C`$read_stage`) {
      `$prefix`_reads --;
       cancel = CANCEL_REQUEST;
; if ( $debug ) {
       printf("%s %u: request killed, read count %u-- => %u, cancel = %d\t", 
              __func__, __LINE__,
              `$prefix`_reads+1, `$prefix`_reads, cancel);
; }
    }
; } #end ekill
; if ( $kill ) {

    if (`$prefix`_cancel_read == CANCEL) {
      /* This is how the RTL behaves hence this weirdness to match. If
       * there is no other Q access, drop the Request by decrementing
       * read count. BUT, if there is another one, then don't decrement
       * read count, because then the canceled read can be uncanceled.
       */
      if (`$prefix`_reads > 1 && globalstall) {
	cancel = CANCEL_RETRY;
; if ( $debug ) {
        printf("%s %u: read killed, read count %u, cancel = %d\n", 
               __func__, __LINE__, `$prefix`_reads, cancel);
; }
      }
      else {
        `$prefix`_reads --;
	cancel &= (CANCEL_REQUEST | NO_CANCEL);
; if ( $debug ) {
        printf("%s %u: read killed, read count %u-- => %u, cancel = %d\n", 
               __func__, __LINE__,
               `$prefix`_reads+1, `$prefix`_reads, cancel);
; }
      }
    }
; }

    `$prefix`_cancel_read = cancel;
; if ( $debug ) {
    if (cancel)
      printf("%s %u: cancel %d\n", __func__, __LINE__, cancel);
; }
; }

    `$prefix`_use_C`$read_stage` = 0;

    /* Read requests have been killed */
    if (`$prefix`_reads == 0) {
; if ($debug) {
      printf("%s %u: all reads killed, remove from scheduler\n",
             __func__, __LINE__);
;    }
      return;
    }

    /* There is a KillPipe during globalstall, stop the reads */
    if (iss4_killpipe_w(info->core)) {
      /* There is data in the staging buffer that needs to be used later */
      if (`$prefix`_staging_valid) { 
        `$prefix`_staging_killed = STAGING_KILLPIPE; 
      }
; if ($debug) {
      printf("%s %u: KillPipe_W, read count = 0, staging_killed = %d\n",
             __func__, __LINE__, `$prefix`_staging_killed);
; }
      `$prefix`_reads = 0;
      return;
    }

; if ( $buf ) {
    if (`$prefix`_srptr != `$prefix`_wptr && 
        !`$prefix`_read_last_sentry) {
      /* If there are multiple reads (there could be one or two)
       * then come back to pop. Don't decrement the read count here;
       * it should be decremented when the spec buffer is read
       */
; if ($debug) {
      printf("%s %u: specbuf has data, read count %u\n",
             __func__, __LINE__, `$prefix`_reads);
; }
      if (`$prefix`_reads > 1)
        iss4_schedule_for_end_of_free_cycle(info->core,
                                            `$name`_request,
                                            info);
      return;
    }

; } # end of if ( $buf )
    if (`$prefix`_staging_valid) { 
      /* staging data is being killed by an instruction during globalstall,
       * but the state might change during globalstall so wait */
      unsigned wait_use_staging = 
        STAGING_KILLED_COND(`$prefix`_staging_killed) && globalstall;
; if ($debug) {
      printf("%s %u: valid data in staging buf can %sbe used\n",
             __func__, __LINE__, wait_use_staging ? "not yet " : "");
; }
      /* If killed data in staging buf can be used */
      if (STAGING_KILLED(`$prefix`_staging_killed) && !wait_use_staging) {
        `$prefix`_staging_killed = STAGING_NOTKILLED;
        `$prefix`_reads --;
; if ($debug) {
        printf("%s %u: staging buf has killed data, read count %u-- => %u\n",
               __func__, __LINE__,
               `$prefix`_reads+1, `$prefix`_reads);
; }
      } 
      else if (`$prefix`_reads > 0) {
        /* Someone else reading check next cycle */
        iss4_schedule_for_end_of_free_cycle(info->core,
                                            `$name`_request,
                                            info);
; if ($debug) {
        printf("%s %u: staging buf has data but read count %u, reschedule\n", 
               __func__, __LINE__, `$prefix`_reads);
; }
      }
      return;
    }

; if ($debug) {
    printf("%s %u: invoke `$enable`_interface\n", __func__, __LINE__);
; }
    info->`$enable`_interface(info->`$enable`_interface_link, 
           `$prefix_space` info->`$enable`_interface_core,
           `$prefix_space` info->`$enable`_interface_device, 0);
    
    iss4_schedule_for_end_of_global_cycle(info->core,
                                          `$name`_sample,
                                          info);
}

; # Wrapper function that simply schedules the read function for the end of a cycle 
static int
`$name`_external_read(dll_data_ptr_t info)
{
    if (!`$prefix`_use_C`$read_stage`) { /* once per instruction */
      unsigned old = `$prefix`_reads ++;
;   if ( $buf ) {
      int specbuf_has_data = 
        (`$prefix`_srptr != `$prefix`_wptr);
;   }
      `$prefix`_use_C`$read_stage` = 1;

;   if ($buf) {
      if (old == 0 || (old == 1 && specbuf_has_data))
;   } else {
      if (old == 0)
;   }
        iss4_schedule_for_end_of_free_cycle(info->core,
                                            `$name`_request,
                                            info);
;   if ( $debug ) {
      printf("%s %u: read count %u++ => %u, specbuf has %sdata\n", 
             __func__, __LINE__,
             old, `$prefix`_reads, specbuf_has_data ? "" : "no ");
;   }
    }
    return 0;
}
; # end of if ( type eq 'D' )
; } else {
; #############################################################################
; # Control interfaces that are registered. Different from "buffered"
; # control interfaces which are for lookup interfaces.
; #
; # if read stage is 0, just declare a external read function 
; # this will be scheduled from the issue function for the end of R stage.
; # else declare an external_read function that in turn schedules a sample
; # function for the end of the same cycle.
; #############################################################################
; if ( $read_stage > 0 ) {
static void
`$name`_sample(dll_data_ptr_t info)
{
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` &`$prefix`_staging_data[0]);
; if ( $debug ) {
    printf("%s %u: read new data [ ", __func__, __LINE__);
;  for ($i = 0; $i < $words; $i++) {
    printf("0x%x ", `$prefix`_staging_data[`$i`]); 
;  }
    printf("]\n");
; }
}

static int
`$name`_external_read(dll_data_ptr_t info)
{
    iss4_schedule_for_end_of_global_cycle(info->core,
                                          `$name`_sample,
                                          info);
; if ( $debug ) {
    fprintf(stdout, "TIEWIRE: `$name`_external_read; ");
; }
    return 0;
}
;    # end of if ( $read_stage > 0 )
; } else { # read stage == 0
int `$name`_external_read(dll_data_ptr_t info)
{
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` &`$prefix`_staging_data[0]);
; if ( $debug ) {
    fprintf(stdout, "TIEWIRE: `$name`_external_read : read new data: ");
;  for ($i = 0; $i < $words; $i++) {
    fprintf(stdout, "%x ", `$prefix`_staging_data[`$i`]); 
;  }
    fprintf(stdout, "\n");
; }
    return 0;
}
;    # end of ($read_stage == 0)
;    } 
; } # end of ($type eq 'C')
; } # end of ($reg)
; #############################################################################

; ############################################################################
; # The read function is valid for all types of input interfaces. This is the
; # function the semantic calls to get data. Must accomodate all types of inputs
; # the simplest being a control interface that is not registered, in which 
; # case it just calls the external read function.
; ############################################################################
static void
`$name`_read(dll_data_ptr_t info, unsigned *data)
{
; if ( $buf ) {
; if ( $type eq 'D') { #data
    unsigned srptr = `$prefix`_srptr;
    unsigned wptr = `$prefix`_wptr;
    unsigned globalstall = iss4_global_stall(info->core);

    if (info->tieportstall) return;

    if (globalstall) {
      /* Un-kill the staging buffer data when the kill goes away during
       * GlobalStall. Otherwise, the next instruction would think that it
       * can use the killed staging buffer data and not make a Pop request.
       */
; if ( $kill ) {
      if (srptr == wptr &&
          `$prefix`_staging_valid &&
          STAGING_KILLED_COND(`$prefix`_staging_killed) && 
          !`$prefix`_kill_C`$stage` &&
          `$prefix`_reads > 0) {
; if ($debug) {
	printf("%s %u: globalstall, reset staging_killed, "
               "read count %u, staging (valid %d, killed %d), kill %d\n", 
               __func__, __LINE__,
	       `$prefix`_reads, `$prefix`_staging_valid,
               `$prefix`_staging_killed, `$prefix`_kill_C`$stage`);
; }
;  } else {
      /* Add (srptr == wptr) test here like in the $kill case? */
      if (`$prefix`_staging_valid &&
          STAGING_KILLED_COND(`$prefix`_staging_killed) &&
          `$prefix`_reads > 0) {
; if ($debug) {
	printf("%s %u: globalstall, reset staging_killed, "
               "read count %u, staging (valid %d, killed %d)\n", 
               __func__, __LINE__,
	       `$prefix`_reads, `$prefix`_staging_valid,
               `$prefix`_staging_killed);
; }
; } #endif kill
        `$prefix`_staging_killed = STAGING_NOTKILLED;
      }
; if ( $kill ) {

      if (`$prefix`_kill_C`$stage` &&
          srptr != wptr &&
          `$prefix`_read_last_sentry) {
        `$prefix`_read_last_sentry = 0;
; if ( $debug ) {
        printf("%s %u: globalstall, last spec buffer "
               "entry read killed, reset `$prefix`_read_last_sentry\n",
               __func__, __LINE__);
; }
      }
; } #endif kill
      return;
   }
; if ( $ekill ) {

    if (`$prefix`_cancel_read == CANCEL_REQ_READ || `$prefix`_cancel_read == CANCEL_REQUEST ) {
      `$prefix`_cancel_read = NO_CANCEL;
;   if ( $debug ) {
      printf("%s %u: request was killed, "
             "read bogus data, cancel = %d\n",
             __func__, __LINE__, `$prefix`_cancel_read);
; }
      return;
    }
; }
; if ( $kill ) {

    if (`$prefix`_kill_C`$stage`)  {
      if (srptr == wptr) {
        int decr = (`$prefix`_cancel_read == CANCEL_RETRY &&
                    `$prefix`_reads > 1);
;   for ($i = 0; $i < $words; $i++) {
	data[`$i`] = `$prefix`_staging_data[`$i`];
;   }
;   if ( $debug ) {
	printf("%s %u: peek from staging buf, read count %u => %u\n",
               __func__, __LINE__,
               `$prefix`_reads, `$prefix`_reads - decr);
;   }
        if (decr)
          `$prefix`_reads --;
      }
      else {
	/* Do not decrement the read count if there is only
         * one read left and the staging buffer has valid data.
         */
        int decr = (`$prefix`_reads > 1 ||
                    !`$prefix`_staging_valid ||
                    (`$prefix`_reads > 0 &&
                     STAGING_KILLED_COND(`$prefix`_staging_killed)));
;  for ($i = 0; $i < $words; $i++) {
        data[`$i`] = `$prefix`_fifo[`$prefix`_srptr][`$i`];
;  }
;   if ( $debug ) {
	printf("%s %u: peek from spec buf, read count %u => %u\n",
               __func__, __LINE__,
               `$prefix`_reads, `$prefix`_reads - decr);
;   }
        if (decr)
	  `$prefix`_reads --;
	`$prefix`_read_last_sentry = 0;
      }
      return;
    }
;}

    if (srptr == wptr) { /* specbuf is empty, read new data */
      /* If InterruptStall_M, don't push junk data into specbuf */
; if ($reg) {
      if (!`$prefix`_staging_valid &&
          iss4_interrupt_stall_m(info->core)) {
;  if ($debug) {
        printf("%s %u: No staging data, ignore due to InterruptStall_M\n",
               __func__, __LINE__);
;  }
        return;
      }

;  if ($debug) {
      printf("%s %u: Staging data valid, read it\n",
             __func__, __LINE__);
;  }
      /* Data comes from the staging buffer */
;   for ($i = 0; $i < $words; $i++) {
      `$prefix`_fifo[wptr][`$i`] = `$prefix`_staging_data[`$i`];
      data[`$i`] = `$prefix`_staging_data[`$i`];
;   }
;  if ($kill) {
      /* Handle conditional kills which left data in staging buffer */
      if (`$prefix`_staging_valid &&
          STAGING_KILLED_COND(`$prefix`_staging_killed)) {
        `$prefix`_staging_killed = STAGING_NOTKILLED;
      }
;  }
      `$prefix`_staging_valid = 0;
; } else { # (!$reg)
      if (iss4_interrupt_stall_m(info->core)) {
;  if ($debug) {
        printf("%s %u: Ignore data due to InterruptStall_M\n",
               __func__, __LINE__);
;  }
        return;
      }

      /* Read new data */
      `$prefix`(`$prefix_link`,
      `$prefix_space` `$prefix_core`,
      `$prefix_space` `$prefix_device`,
      `$prefix_space` &data[0]);
;   for ($i = 0; $i < $words; $i++) {
      `$prefix`_fifo[wptr][`$i`] = data[`$i`];
;   }

; if ( $debug ) {
      printf("%s %u: read new data: ", __func__, __LINE__);
;   for ($i = 0; $i < $words; $i++) {
      printf("0x%x ", data[`$i`]); 
;   }
      printf("\n");

; }        
; } # end of (!$reg)
      `$prefix`_pipe[`$use_stage`] = wptr;
      `$prefix`_wptr = (wptr + 1) % `$bdepth`;
      `$prefix`_srptr = (srptr + 1) % `$bdepth`;
;  if ( $debug ) {

      printf("%s %u: fifo = { ", __func__, __LINE__);
;   for ($i = $bdepth - 1; $i >= 0; $i--) {
      printf("0x%x ", `$prefix`_fifo[`$i`][0]);
;   }
      printf("}");
      printf(" wptr %d, srptr %d, use_pipe %s  { ",
	     `$prefix`_wptr, `$prefix`_srptr,
             binvec(`$prefix`_use_pipeline));
;   for ($i=0; $i<$pdepth; $i++) {
      printf("%d ", `$prefix`_pipe[`$i`]);
;   }
      printf("}\n");
;  }
    } 
    else { 
      /* specbuf has valid data, read it */
;  for ($i = 0; $i < $words; $i++) {
      data[`$i`] = `$prefix`_fifo[`$prefix`_srptr][`$i`];
;  }
;  if ( $debug ) {
      printf("%s %u: specbuf has data, read from srptr %d\n",
             __func__, __LINE__, `$prefix`_srptr);
;  }
;  if ( $reg ) {
      if (`$prefix`_reads == 0) {
; if ( $debug ) {
         printf("%s %u: read count already 0, do not decrement!!!!\n",
                __func__, __LINE__);
; }
      } else {
        `$prefix`_reads --;
;   if ( $debug ) {
         printf("%s %d: read count %u-- => %u: ", __func__, __LINE__, 
		`$prefix`_reads+1, `$prefix`_reads);
;   }
      }
      `$prefix`_read_last_sentry = 0;
;  }
;  if ( $debug ) {
;   for ($i = 0; $i < $words; $i++) {
      printf(" 0x%x", `$prefix`_fifo[`$prefix`_srptr][`$i`]);
;   }
      printf("\n");

;  }
      `$prefix`_pipe[`$use_stage`] = srptr;
      `$prefix`_srptr = (srptr + 1) % `$bdepth`;
;  if ( $debug ) {

      printf("%s %u: srptr = %u, pipe { ",
             __func__, __LINE__, `$prefix`_srptr);
;   for ($i=0; $i< $pdepth; $i++) {
      printf("%d ", `$prefix`_pipe[`$i`]);
;   }
      printf(" }\n");
;  }
    }
; } else { #type control
; #########################################################################
; #lookup inputs
    unsigned rptr = `$prefix`_rptr;
    if (iss4_global_stall(info->core) || info->tieportstall) return;
;   for ($i = 0; $i < $words; $i++) {
    data[`$i`] = `$prefix`_buf[rptr][`$i`];
;   }
    `$prefix`_rptr = (rptr + 1) % `$bdepth`;
; if ( $read_stage < $commit && $stage > $commit ) {
    `$prefix`_reads --;
; }
; if ( $debug ) {
    printf("%s %u: read data 0x%x from %d, new rptr %d\n",
    	   __func__, __LINE__,
           data[0], rptr, `$prefix`_rptr);
; if ( $read_stage < $commit && $stage > $commit ) {
    printf("%s %u: read count %u\n",
           __func__, __LINE__, `$prefix`_reads);
; }
;}
; }
; # end of if ( $buf )
; } else { # no buffer just call the external interface function
    if (iss4_global_stall(info->core) || info->tieportstall) return;
;  if ( $reg  ) {
;   for ($i = 0; $i < $words; $i++) {
    data[`$i`] = `$prefix`_staging_data[`$i`];
;   }
;   if ( $type eq 'D' ) {
    `$prefix`_staging_valid = 0;
;   }
;  } else {
    /* call external interface function */
    `$prefix`(`$prefix_link`,
    `$prefix_space` `$prefix_core`,
    `$prefix_space` `$prefix_device`,
    `$prefix_space` data);
;  }
; }
    if ( info->events ) 
	iss4_tieport_event(info->core, `$stage`, "`$iname`", "->", `$width`, data);
}
; } #$dir eq in
;} #gen data

/* Assembly portions of profiling  */

/*
 * Copyright (c) 2005-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <xtensa/config/core.h>
#include "xt_profiling.h"
#include "hw_profiler_internal.h"
	
#if XCHAL_HAVE_WINDOWED && !__XTENSA_CALL0_ABI__
#define ENTRY(x) entry a1, x
#define CALL call8	
#define CALLX callx8
#define PARAM_REG a10
#define NORM_RETURN retw
#else
#define ENTRY(x)
#define CALL call0	
#define CALLX callx0
#define PARAM_REG a2
#define NORM_RETURN ret
#endif	
	
// This is just the default. You can use xt_profile_set_frequency
// to override it. It is measured in cycles
// It should be a multiple of 1024 to keep labels correct in gmon (but that would
// be the only damage
#ifndef SAMPLE_FREQUENCY
#define SAMPLE_FREQUENCY 0x4000
#endif

	.struct 0
	sample_freq:	.space 4
	enabled:	.space 4
	a3_save:	.space 4
	a4_save:	.space 4
	a5_save:	.space 4
	a6_save:	.space 4
	a7_save:	.space 4
	first_record:	.space 4
	free_mem_start:	.space 4
	free_mem_end:	.space 4
	add_mem_start:	.space 4
	add_mem_end:	.space 4
	errors:	        .space 4
	globals_size:

	
	.section ".bss", "wa"
	.global __hw_profiling_globals
	.align 4
__hw_profiling_globals:
	.word 0 //sample_freq
	.word 0 //enabled
	.word 0 //a3_save
	.word 0 //a4_save
	.word 0 //a5_save
	.word 0 //a6_save
	.word 0 //a7_save
.global xt_profile_base_record
xt_profile_base_record:
	.word 0 //first_record
	.word 0 //free_mem_start
	.word 0 //free_mem_end
	.word 0 //add_mem_start
	.word 0 //add_mem_end
.global xt_profile_errors
xt_profile_errors:
	.word 0 //errors
	.org globals_size

.weak _text_start
.weak _text_end
.weak _sram_text_start
.weak _sram_text_end
.weak _srom_text_start
.weak _srom_text_end
.weak _ResetVector_text_start
.weak _ResetVector_text_end
.weak _iram0_text_start
.weak _iram0_text_end
.weak _iram1_text_start
.weak _iram1_text_end
.weak _WindowVectors_text_start
.weak _WindowVectors_text_end
.weak _KernelExceptionVector_text_start
.weak _KernelExceptionVector_text_end
.weak _UserExceptionVector_text_start
.weak _UserExceptionVector_text_end
.weak _Level2InterruptVector_text_start
.weak _Level2InterruptVector_text_end
.weak _Level3InterruptVector_text_start
.weak _Level3InterruptVector_text_end
.weak _Level4InterruptVector_text_start
.weak _Level4InterruptVector_text_end
.weak _Level5InterruptVector_text_start
.weak _Level5InterruptVector_text_end
.weak _Level6InterruptVector_text_start
.weak _Level6InterruptVector_text_end
.weak _Level7InterruptVector_text_start
.weak _Level7InterruptVector_text_end
.weak _Level8InterruptVector_text_start
.weak _Level8InterruptVector_text_end
.weak _Level9InterruptVector_text_start
.weak _Level9InterruptVector_text_end
.weak _DebugExceptionVector_text_start
.weak _DebugExceptionVector_text_end
.weak _NMIExceptionVector_text_start
.weak _NMIExceptionVector_text_end
.weak _DoubleExceptionVector_text_start
.weak _DoubleExceptionVector_text_end

.global _text_start
.global _text_end
.global _sram_text_start
.global _sram_text_end
.global _srom_text_start
.global _srom_text_end
.global _ResetVector_text_start
.global _ResetVector_text_end
.global _iram0_text_start
.global _iram0_text_end
.global _iram1_text_start
.global _iram1_text_end
.global _WindowVectors_text_start
.global _WindowVectors_text_end
.global _KernelExceptionVector_text_start
.global _KernelExceptionVector_text_end
.global _UserExceptionVector_text_start
.global _UserExceptionVector_text_end
.global _Level2InterruptVector_text_start
.global _Level2InterruptVector_text_end
.global _Level3InterruptVector_text_start
.global _Level3InterruptVector_text_end
.global _Level4InterruptVector_text_start
.global _Level4InterruptVector_text_end
.global _Level5InterruptVector_text_start
.global _Level5InterruptVector_text_end
.global _Level6InterruptVector_text_start
.global _Level6InterruptVector_text_end
.global _Level7InterruptVector_text_start
.global _Level7InterruptVector_text_end
.global _Level8InterruptVector_text_start
.global _Level8InterruptVector_text_end
.global _Level9InterruptVector_text_start
.global _Level9InterruptVector_text_end
.global _DebugExceptionVector_text_start
.global _DebugExceptionVector_text_end
.global _NMIExceptionVector_text_start
.global _NMIExceptionVector_text_end
.global _DoubleExceptionVector_text_start
.global _DoubleExceptionVector_text_end	

	.data
	.align 4
.Ltext_size_table:
	.word _text_start
	.word _text_end
	.word _sram_text_start
	.word _sram_text_end
	.word _srom_text_start
	.word _srom_text_end
	.word _ResetVector_text_start
	.word _ResetVector_text_end
	.word _iram0_text_start
	.word _iram0_text_end
	.word _iram1_text_start
	.word _iram1_text_end
	.word _WindowVectors_text_start
	.word _WindowVectors_text_end
	.word _KernelExceptionVector_text_start
	.word _KernelExceptionVector_text_end
	.word _UserExceptionVector_text_start
	.word _UserExceptionVector_text_end
	.word _Level2InterruptVector_text_start
	.word _Level2InterruptVector_text_end
	.word _Level3InterruptVector_text_start
	.word _Level3InterruptVector_text_end
	.word _Level4InterruptVector_text_start
	.word _Level4InterruptVector_text_end
	.word _Level5InterruptVector_text_start
	.word _Level5InterruptVector_text_end
	.word _Level6InterruptVector_text_start
	.word _Level6InterruptVector_text_end
	.word _Level7InterruptVector_text_start
	.word _Level7InterruptVector_text_end
	.word _Level8InterruptVector_text_start
	.word _Level8InterruptVector_text_end
	.word _Level9InterruptVector_text_start
	.word _Level9InterruptVector_text_end
	.word _DebugExceptionVector_text_start
	.word _DebugExceptionVector_text_end
	.word _NMIExceptionVector_text_start
	.word _NMIExceptionVector_text_end
	.word _DoubleExceptionVector_text_start
	.word _DoubleExceptionVector_text_end
	.word 0
	.word 0

	// trashes a3, a6
.macro switch_memory_pools alabel
	//switch memory pools
	l32i a3, a2, add_mem_start
	l32i a6, a2, add_mem_end
	beqz a3, .Lout_of_memory
	s32i a3, a2, free_mem_start
	s32i a6, a2, free_mem_end
	movi a6, 0
	s32i a6, a2, add_mem_start
	s32i a6, a2, add_mem_end
	j \alabel
.endm
	
	.struct 0
	base_pc:	.skip 4
	left_child:     .skip 4
        right_child:	.skip 4
	bins:	        .skip 4
	record_size:	

/* the bins are simply arrays of shorts, each short keeping a count
   of how many times we have executed at that pc.

   Their size
		=ADDRESSES_PER_RECORD * counter_size / sizeof(short)
		=ADDRESSES_PER_RECORD * 2 / 2
		=ADDRESSES_PER_RECORD

		+ next pointer
*/
	
	.struct 0
	next: .skip 4
	bin:  .skip ADDRESSES_PER_RECORD
	bin_size:	
		
	.section .init, "ax"
	CALL xt_profile_init
	CALL xt_profile_enable
	movi PARAM_REG, SAMPLE_FREQUENCY
	CALL xt_profile_set_frequency
	
	.section .fini, "ax"
	CALL xt_profile_disable

	.section .text, "ax"
	#.weak atexit
	.global atexit
	.global xt_dbfs_sbrk
	
	.global xt_profile_init
	.type xt_profile_init, @function
	.align 4	
xt_profile_init:	
	ENTRY(32)
	movi a7, __hw_profiling_globals

	movi a2, .Ltext_size_table
	movi a3, 0
.Lanother_section:	
	l32i a4, a2, 0
	l32i a5, a2, 4
	sub a6, a5, a4
	add a3, a6, a3
	addi a2, a2, 8
	bnez a5, .Lanother_section

	// keep it 16 byte aligned
	movi a4, 0xFFFFFFF0
	and a3, a4, a3
	
	// add some slop because there is some overhead and fragmentation
	addi a3, a3, 4096
		
	// temporarily save the size
	s32i a3, a7, free_mem_end
	
	mov PARAM_REG, a3
	CALL xt_dbfs_sbrk
	bnei PARAM_REG, -1, 1f
	l32i a3, a2, errors
	addi a3, a3, 1
	s32i a3, a2, errors
	NORM_RETURN
	
1:	
	l32i a3, a7, free_mem_end
	s32i PARAM_REG, a7, free_mem_start
	add a3, PARAM_REG, a3
	s32i a3, a7, free_mem_end

	movi a7, 0
2:	
	s32i a7, PARAM_REG, 0
	s32i a7, PARAM_REG, 4
	s32i a7, PARAM_REG, 8
	s32i a7, PARAM_REG, 12
	addi PARAM_REG, PARAM_REG, 16
	bne PARAM_REG, a3, 2b

	movi a3, atexit
	movi PARAM_REG, xt_profile_save_and_reset
	beqz a3, .Lskipatexit
	CALLX a3
.Lskipatexit:
	NORM_RETURN
		
	.global xt_profile_add_memory
	.type xt_profile_add_memory, @function
	.align 4	
xt_profile_add_memory:	
	ENTRY(16)
	movi a7, __hw_profiling_globals
	// if they passed, NULL, we'll just store null
	s32i a2, a7, add_mem_start
	add a2, a2, a3
	s32i a2, a7, add_mem_end
	NORM_RETURN

	
	.global xt_profile_enable
	.type xt_profile_init, @function
	.align 4
xt_profile_enable:
	ENTRY(32)
	movi a2, __hw_profiling_globals
	movi a3, 1
	s32i a3, a2, enabled
	movi a3, .Ltake_sample
	rsr.ccount a5
	l32i a4, a2, sample_freq
	wsr_excsave  a3
	add a5, a5, a4
	wsr_ccompare a5

	// turn on our interrupt
	movi PARAM_REG, (1 << TIMER_INTERRUPT)
	CALL xt_profile_interrupt_on
	NORM_RETURN

	
	.global xt_profile_disable
	.type xt_profile_init, @function
	.align 4
xt_profile_disable:
	ENTRY(32)
	movi a2, __hw_profiling_globals
	movi a3, 0
	s32i a3, a2, enabled

	// turn off our interrupt
	movi PARAM_REG, (1 << TIMER_INTERRUPT)
	CALL xt_profile_interrupt_off
	NORM_RETURN

	
	.global xt_profile_set_frequency
	.type xt_profile_init, @function
	.align 4
xt_profile_set_frequency:
	ENTRY(16)
	movi a3, __hw_profiling_globals
	s32i a2, a3, sample_freq
	rsr.ccount a5
	add a5, a5, a2
	wsr_ccompare a5
	NORM_RETURN
	
	.global xt_profile_get_frequency
	.type xt_profile_get_frequency, @function
	.align 4
xt_profile_get_frequency:
	ENTRY(16)
	movi a3, __hw_profiling_globals
	l32i a2, a3, sample_freq
	NORM_RETURN	

	
#define pc_high_bitmask (0xFFFFFFFF) & ~(ADDRESSES_PER_RECORD - 1)
#define pc_low_bitmask	ADDRESSES_PER_RECORD - 1
#define cant_sample_now_increment 256
		
	.global xt_profile_take_sample
xt_profile_take_sample:	
.Ltake_sample:
	movi a2, __hw_profiling_globals
	s32i a3, a2, a3_save
	l32i a3, a2, enabled
	s32i a4, a2, a4_save
	s32i a5, a2, a5_save
	s32i a6, a2, a6_save
	s32i a7, a2, a7_save
	l32i a4, a2, sample_freq
	beqz a3, .Ldone

	l32i a3, a2, errors
	bnez a3, .Ldone
	
	// we don't want to sample mcount, or the window vectors, so sample a little later
	movi a4, pc_high_bitmask
	movi a5, _mcount
	movi a6, _mcount_end
	rsr_epc a3
	bltu a3, a5, 1f
	bltu a6, a3, 1f
	movi a4, cant_sample_now_increment
	j .Ldone

1:	
#if !__XTENSA_CALL0_ABI__
	movi a5, _WindowVectors_text_start
	movi a6, _WindowVectors_text_end 
	bltu a3, a5, 1f
	bltu a6, a3, 1f
#endif
	movi a4, cant_sample_now_increment
	j .Ldone

1:	
	and a4, a4, a3 // save the high bits of the pc

	movi a5, xt_profile_base_record

xt_profile_find_record:	
.Lfind_record:
	l32i a3, a5, 0
	beqz a3, .Lnew_record // if a5 is null, get a new record
	l32i a6, a3, base_pc //get base address of current record
	beq  a6, a4, .Lfound_record
	sub a6, a4, a6
	addi a7, a3, left_child
	addi a5, a3, right_child
	movltz a5, a7, a6
	j .Lfind_record

	// a5 = address of place to store address of new record
	// a4 = high bits of pc for new record
.Lnew_record:
	.global xt_profile_new_record
xt_profile_new_record:	
	l32i a3, a2, free_mem_start
	l32i a6, a2, free_mem_end
	addi a7, a3, record_size

	bgeu a6, a7, 1f
	switch_memory_pools .Lnew_record
1:
	s32i a7, a2, free_mem_start
	// a3 = new record
.Lhave_new_record:	
	s32i a3, a5, 0
	s32i a4, a3, base_pc
	// a3 = some record that needs new bins
.Lnew_bins:
	l32i a5, a2, free_mem_start
	l32i a6, a2, free_mem_end
	addi a7, a5, bin_size

	bgeu a6, a7, 1f
	switch_memory_pools .Lnew_bins
1:
	s32i a7, a2, free_mem_start
	// a5 = new bins
	l32i a6, a3, bins
	s32i a5, a3, bins
	s32i a6, a5, next
	
	// a3 = some record that needs some bins incremented

.global xt_profile_found_record	
xt_profile_found_record:	
.Lfound_record:
	movi a6, pc_low_bitmask
	rsr_epc a7
	and a6, a6, a7
	//index	= floor(pc_low_bits / ADDRESSES_PER_BUCKET * sizeof(short))
	//      =  ~1 & ( pc_low_bits / 2 * 2)
	//      =  ~1 & pc_low_bits
	movi a7, -2
	and a7, a7, a6

	l32i a5, a3, bins
	add a5, a5, a7
	l16ui a4, a5, bin // increment its value
	addi a4, a4, 1
	beqz a4, .Lnew_bins
	s16i a4, a5, bin

	l32i a4, a2, sample_freq

	// a2 = __hw_profiling_globals
	// a4 = increment to next sample cycle 	
.Ldone:	
	l32i a7, a2, a7_save
	l32i a6, a2, a6_save
	l32i a5, a2, a5_save
	rsr.ccount a3
	add a3, a3, a4
	wsr_ccompare a3
	l32i a4, a2, a4_save
	l32i a3, a2, a3_save	
	movi a2, .Ltake_sample
	xsr_excsave a2
	rfi LEVEL
	
	.global xt_profile_out_of_memory
xt_profile_out_of_memory:	
.Lout_of_memory:	
	l32i a3, a2, errors
	addi a3, a3, 1
	s32i a3, a2, errors
	j .Ldone
	

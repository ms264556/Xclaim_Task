/*
 * Copyright (c) 2005-2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#ifndef __XT_PROFILER_H__
#define __XT_PROFILER_H__

#ifndef __ASSEMBLER__
#ifdef __cplusplus
extern "C" {
#endif

/* This file defines an interface that allows a program being profiled
   to control when and how it is profiled, whether it is running under
   the instruction set simulator or under the hardware profiler.

   Both ISS and HWP implement this interface, although in different
   ways. Both also do the right thing if you don't call any of these
   functions.
*/


/* 
xt_profile_init

   ISS: a no op.

   HWP: Initialize the profiler.  Ordinarily, this function is called 
   automatically via the .init section. If your environment does not 
   support the .init section, you will need to call this function 
   by hand.
*/
extern void xt_profile_init(void);

/* 
xt_profile_add_memory

   ISS: a no op.

   HWP:
   Makes "buf_size" bytes at "buf" available to the hardware profiler.
   This buffer should be initialized to zeros prior to this call.

   The hardware profiler has already estimated the amount of memory it needs,
   but under certain circumstances may still run out of memory.  If so, you can
   provide more memory with this routine.

*/
extern void xt_profile_add_memory(void * buf, unsigned int buf_size);


/* xt_profile_enable

   Turn on the profiler. Ordinarily, profiling is on by default. 
   If you turn off profiling using xt_profile_disable, You can turn 
   it on again via this function.
*/
extern void xt_profile_enable(void);

/* xt_profile_disable

   Turn off the profiler. If you don't want to profile a portion of your code,
   use this function and then xt_profile_enable when you want to start again.
*/
extern void xt_profile_disable(void);

/* xt_profile_save_and_reset

   Save and reset the profiler's data.
   If there were errors, either during profiling or while attempting to
   write the data, no data will be written and this function will
   return non-zero.
   
*/
extern int xt_profile_save_and_reset(void);

/* xt_profile_get_frequency

   ISS: always returns 1.

   HWP:
   Returns the number of cycles between samples.
*/
extern unsigned int xt_profile_get_frequency(void);

/* xt_profile_set_frequency

   ISS: a no op.

   HWP:
   Set the number of cycles between samples.

   sample frequency is the number of cycles to wait between samples.  It should
   be a multiple of 1024.
   
   If you set the sample frequency to a different value than was passed in xt_profile_init,
   then the labels in the output will reflect the later frequency, even though some samples may
   have been taken at the earlier frequency.  Typically this does not make a significant difference
   in the results if this function is called early enough.
*/
extern void xt_profile_set_frequency(unsigned int sample_frequency);

/* xt_profile_num_errors

   ISS: always returns 0

   HWP:
   Returns the number of errors that occured while taking samples.  Typically these
   are out of memory errors and you need to pass a bigger buffer to 
   xt_profile_add_memory
*/
extern int xt_profile_num_errors(void);

#ifdef __cplusplus
}
#endif
#endif

#endif

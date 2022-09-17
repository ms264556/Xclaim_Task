//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: profile_interface.cxx
// $Revision: 1.11 $
// $Date: 2001/03/12 18:39:30 $
// $Author: mtibuild $
// $Source: /isms/cmplrs.src/osprey1.0/instrumentation/libinstr/RCS/profile_interface.cxx,v $
//
// ====================================================================
//
// Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it would be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Further, this software is distributed without any warranty that it
// is free of the rightful claim of any third person regarding
// infringement  or the like.  Any license provided herein, whether
// implied or otherwise, applies only to this software file.  Patent
// licenses, if any, provided herein do not apply to combinations of
// this program with other software, or any other product whatsoever.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
//
// Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
// Mountain View, CA 94043, or:
//
// http://www.sgi.com
//
// For further information regarding this notice, see:
//
// http://oss.sgi.com/projects/GenInfo/NoticeExplan
//
// ====================================================================
//
// Description:
//
// During instrumentation, calls to the following procedures are
// inserted into the WHIRL code.  When invoked, these procedures
// initialize, perform, and finalize frequency counts.
//
// ====================================================================
// ====================================================================


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "profile.h"
#include "profile_interface.h"
#include "profile_errors.h"
#include "dump.h"

#ifdef BUILD_FOR_HARDWARE
#define REPLACE_FS_WITH_DBFS
#include <xtensa/debugfs.h>
#endif

// ====================================================================


static int
my_mkstemp(char *path)
{
  char *start, *p;
  int fd, i;

  for (p = path; *p; ++p) ; // go to the end of path

  while (*--p == 'X')       // replace X's with 0's
    *p = '0';

  start = p + 1;

#ifdef __ERRNO_FIXED__
  for (;;) {
#else
  // errno is broken with simcalls -- prevent infinite looping
  for (i = 0; i < 1000; i++) {
#endif

    fd = open(path, O_CREAT | O_EXCL | O_RDWR | O_BINARY, 0666);
    if (fd >= 0)
      return fd;

#ifdef __ERRNO_FIXED__
    if (errno != EEXIST)
      return -1;
#endif

    for (p = start;;) {
      if (!*p)
        return -1;
      if (*p == 'z')
        *p++ = 'a';
      else {
        if (isdigit (*p))
          *p = 'a';
        else
          ++ * p;
        break;
      }
    }
  }

  return -1;
}


// ====================================================================


static char *base_filename = NULL;
static char *output_filename = NULL;
static int fd = -1;
static BOOL unique_output_filename = FALSE; 
static BOOL profile_table_inited = FALSE;
static bool needs_initialization = true;

// ====================================================================


void __profile_init(char *fname, unsigned int flags, int phase_num, BOOL unique_name)
{
  try {
    static bool one_time_init_completed = false;
    if (!one_time_init_completed) {
      one_time_init_completed = true;
      
      base_filename = fname;
      void (*pf)() = __profile_finish;
      atexit(pf);
      
      PU_Profile_Handle_Table = new HASH_MAP(1024);
      profile_table_inited = TRUE;
      Set_Instrumentation_Flags(flags);
      unique_output_filename = unique_name;
    }
    
    PROFILE_PHASE curr_phase_num = Instrumentation_Phase_Num();
    if (curr_phase_num == PROFILE_PHASE_NONE) {
      Set_Instrumentation_Phase_Num((PROFILE_PHASE) phase_num);
    } 
    else {
      if(curr_phase_num != (PROFILE_PHASE) phase_num) {
	profile_warn("Phase Number already set to a different value in ", 
		     base_filename);
      }
    }
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// PU level initialization to gather profile information for the PU.
// We call atexit during the first call to this routine to ensure
// that at exit we remember to destroy the data structures and dump
// profile information.
// Also, during the first call, a profile handle for the PU is created.
// During subsequent calls, teh PC address of the PU is used to access
// a hash table and return the profile handle that was created during the 
// first call.

void *
#if FB_64BIT_COUNTERS
__profile_pu_init_new_64
#else
__profile_pu_init_new_32
#endif
                     (char *file_name, 
		      char *puname, 
		      long  current_pc,
		      INT32 checksum,
		      INT32 num_invokes,
		      INT32 num_branches,
		      INT32 num_loops,
		      INT32 num_short_circuit_ops,
		      INT32 num_calls,
		      char *output_filebasename,
		      INT32 flags)
{
  try {
    if (!profile_table_inited)
      __profile_init(output_filebasename, flags);
    
    PU_PROFILE_HANDLE pu_handle
      = Get_PU_Handle(file_name, puname, current_pc, checksum, flags);
    
    if (num_invokes > 0)
      pu_handle->Get_Invoke_Table().reserve_and_init(num_invokes);
    
    if (num_branches > 0)
      pu_handle->Get_Branch_Table().reserve_and_init(num_branches);
    
    if (num_loops > 0)
      pu_handle->Get_Loop_Table().reserve_and_init(num_loops);
    
    if (num_short_circuit_ops > 0)
      pu_handle->Get_Short_Circuit_Table().reserve_and_init(num_short_circuit_ops);
    
    if (num_calls > 0)
      pu_handle->Get_Call_Table().reserve_and_init(num_calls);
    
    return (void *) pu_handle;
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
  return NULL;
}




// For a PU, initialize the data structures that maintain 
// invokation profile information.

void
__profile_invoke_init(void *pu_handle, INT32 num_invokes)
{
  try {
    Profile_Invoke_Init((PU_PROFILE_HANDLE) pu_handle, num_invokes);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information for a conditional invoke

void
__profile_invoke(void *pu_handle, INT32 invoke_id)
{
  try {
    Profile_Invoke((PU_PROFILE_HANDLE) pu_handle, invoke_id);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// For a PU, initialize the data structures that maintain 
// conditional branch profile information.

void
__profile_branch_init(void *pu_handle, INT32 num_branches)
{
  try {
    Profile_Branch_Init((PU_PROFILE_HANDLE) pu_handle, num_branches);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information for a conditional branch

void
__profile_branch(void *pu_handle, INT32 branch_id, bool taken)
{
  try {
    Profile_Branch((PU_PROFILE_HANDLE) pu_handle, branch_id, taken);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// For a PU, initialize the data structures that maintain 
// switch profile information.

void
__profile_switch_init(void *pu_handle,
		      INT32 num_switches,    INT32 *switch_num_targets,
		      INT32 num_case_values, INT64 *case_values)
{
  try {
    Profile_Switch_Init((PU_PROFILE_HANDLE) pu_handle,
		      num_switches,    switch_num_targets,
		      num_case_values, case_values);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information for an Switch

void
__profile_switch(void *pu_handle, INT32 switch_id, INT64 case_value,
		   INT32 num_targets)
{
  try {
    Profile_Switch((PU_PROFILE_HANDLE) pu_handle, switch_id, case_value,
		   num_targets);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// For a PU, initialize the data structures that maintain 
// compgoto profile information.

void
__profile_compgoto_init(void *pu_handle, INT32 num_compgotos,
			INT32 *compgoto_num_targets)
{
  try {
    Profile_Compgoto_Init((PU_PROFILE_HANDLE) pu_handle, num_compgotos,
			compgoto_num_targets);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information for an Compgoto

void
__profile_compgoto(void *pu_handle, INT32 compgoto_id, INT32 target,
		   INT32 num_targets)
{
  try {
    Profile_Compgoto((PU_PROFILE_HANDLE) pu_handle, compgoto_id, target,
		   num_targets);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// For a PU, initialize the data structures that maintain 
// loop profile information.

void 
__profile_loop_init(void *pu_handle, INT32 num_loops)
{
  try {
    Profile_Loop_Init((PU_PROFILE_HANDLE) pu_handle, num_loops);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information for a loop entry

void
__profile_loop(void *pu_handle, INT32 loop_id)
{
  try {
    Profile_Loop((PU_PROFILE_HANDLE) pu_handle, loop_id);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information from a Loop Iteration

void
__profile_loop_iter(void *pu_handle, INT32 loop_id) 
{
  try {
    Profile_Loop_Iter((PU_PROFILE_HANDLE) pu_handle, loop_id);  
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// For a PU, initialize the data structures that maintain
// short circuit profile information.

void 
__profile_short_circuit_init(void *pu_handle, INT32 num_short_circuit_ops)
{
  try {
    Profile_Short_Circuit_Init((PU_PROFILE_HANDLE) pu_handle,
			       num_short_circuit_ops);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather profile information for the right operand of a short circuit op

void 
__profile_short_circuit(void *pu_handle, INT32 short_circuit_id, bool taken)
{
  try {
    Profile_Short_Circuit((PU_PROFILE_HANDLE) pu_handle,
			  short_circuit_id, taken);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// For a PU, initialize the data structures that maintain
// call profiles.

void 
__profile_call_init(void *pu_handle, int num_calls)
{
  try {
    Profile_Call_Init((PU_PROFILE_HANDLE) pu_handle, num_calls);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather the entry count for this call id

void 
__profile_call_entry(void *pu_handle, int call_id)
{
  try {
    Profile_Call_Entry((PU_PROFILE_HANDLE) pu_handle, call_id);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


// Gather the exit count for this call id

void 
__profile_call_exit(void *pu_handle, int call_id)
{
  try {
    Profile_Call_Exit((PU_PROFILE_HANDLE) pu_handle, call_id);
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}



void profile_cleanup_and_exit(void)
{
  profile_warn("Cannot write profiling information\n", "");

  close(fd);
  fd = -1;
  unlink(output_filename);
  _exit(1);
}


// At exit processing to destroy data structures and dump profile
// information.

void xt_feedback_save_and_reset(void)
{
  try {
    __profile_finish();
    
    long sig;
    PU_PROFILE_HANDLE pu_handle;
    HASH_MAP_ITER i(PU_Profile_Handle_Table);
    
    while (i.Step(&sig, &pu_handle)) {
      pu_handle->Get_Invoke_Table().Bzero_array();
      pu_handle->Get_Branch_Table().Bzero_array();
      pu_handle->Get_Loop_Table().Bzero_array();
      pu_handle->Get_Short_Circuit_Table().Bzero_array();
      pu_handle->Get_Call_Table().Bzero_array();
    }
    
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}


void __profile_finish(void)
{
  try {
    if (!profile_table_inited) {
      // nothing to save
      return;
    }

    char * output_filename = base_filename;
    
    if (unique_output_filename) {
      output_filename = (char * ) alloca(strlen(base_filename) + 7 + 1);
      strcpy(output_filename, base_filename);
      strcat (output_filename, ".XXXXXX");
      
      fd = my_mkstemp (output_filename);
      if (fd < 0) { 
	// We can't use profile_error, because it calls exit, which
	// then calls __profile_finish, and we end up looping forever
	profile_warn("Feedback Error: couldn't create feedback file ", output_filename);
	_exit(1); // otherwise we end up back in __profile_finish
      }
    }
    
    Dump_all(fd, output_filename);
    
    output_filename = NULL;
    
    close(fd);  
    fd = -1;
  }
  catch (...) {
    profile_error("Out of memory.", "");
  }
}

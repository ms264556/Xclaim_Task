//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: profile.h
// $Revision: 1.7 $
// $Date: 2001/03/12 18:39:21 $
// $Author: mtibuild $
// $Source: /isms/cmplrs.src/osprey1.0/instrumentation/libinstr/RCS/profile.h,v $
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


#ifndef profile_INCLUDED
#define profile_INCLUDED

#ifndef profile_com_INCLUDED
#include "profile_com.h"
#endif

#ifndef profile_aux_INCLUDED
#include "profile_aux.h"
#endif 


// ====================================================================
#define INLINE_EVERYTHING
#ifdef INLINE_EVERYTHING
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


extern PROFILE_PHASE instrumentation_phase_num;
extern unsigned int instrumentation_flags;

// ====================================================================

// Given the PC address of a PU, index into a hash table and 
// retrieve the profile handle for the PU. If the handle is
// NULL, create it.

static inline PU_PROFILE_HANDLE
Get_PU_Handle(char *file_name, char* pu_name, long current_pc, INT32 checksum, INT32 opt_flags)
{
  PU_PROFILE_HANDLE pu_handle = PU_Profile_Handle_Table->Find(current_pc);

  if (pu_handle == NULL) {
    pu_handle = new PU_Profile_Handle(file_name, pu_name, checksum, opt_flags); 
    PU_Profile_Handle_Table->Enter(current_pc, pu_handle);
  }

  return pu_handle;
}

// Given a PU handle, set the file name in which the PU appears

inline void
PU_Profile_Handle::Set_file_name(char *s)
{
  this->file_name = new char[strlen(s) + 1];
  strcpy(this->file_name, s);
}

// Given a PU handle, set the name of the PU

inline void
PU_Profile_Handle::Set_pu_name( char *s)
{
  this->pu_name = new char[strlen(s) + 1];
  strcpy(this->pu_name, s);
}

// ====================================================================

// For each PU, we want to a one-time initialization of the
// tables that maintain profile information of invoke nodes.
// Given a pu_handle and the number of invoke nodes in that PU, we
// initialize the Call_Profile_Table in the pu handle with the appropriate
// number of entries. This routine may be invoked multiple times from
// a PU, but the initialization is done only the first time.

static inline void 
Profile_Invoke_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_invokes)
{
  Invoke_Profile_Vector& Inv_Table = pu_handle->Get_Invoke_Table();
  
  if (Inv_Table.empty()) {
    Inv_Table.reserve_and_init(num_invokes);
  }
}

// Update entry count for a invoke

static inline void 
Profile_Invoke(PU_PROFILE_HANDLE pu_handle, INT32 invoke_id)
{
  Invoke_Profile_Vector& Inv_Table = pu_handle->Get_Invoke_Table();

  Inv_Table[invoke_id].invoke_count++;
}

// ====================================================================

// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of branch nodes. 
// Given a pu_handle and the number of conditional branches
// in that PU, we initialize the Br_Table in the pu handle
// with the appropriate number of entries. 
// This routine may be invoked multiple times from a PU, but the
// initialization is done only the first time.

static inline void
Profile_Branch_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_branches)
{
  Branch_Profile_Vector& Br_Table = pu_handle->Get_Branch_Table();
  
  if (Br_Table.empty()) {
    Br_Table.reserve_and_init(num_branches);
  }
}

// Given a PU handle, retrieve the branch table and increment
// taken count for id.

static inline void Incr_Branch_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id) 
{
  Branch_Profile_Vector& Br_Table = pu_handle->Get_Branch_Table();
  Br_Table[id].taken++;
}

// Given a PU handle, retrieve the branch table and increment
// not-taken count for id.

static inline void Incr_Branch_Not_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id) 
{
  Branch_Profile_Vector& Br_Table = pu_handle->Get_Branch_Table();
  Br_Table[id].not_taken++;
}

// Update appropriate profile information for a branch.

static inline void
Profile_Branch(PU_PROFILE_HANDLE pu_handle, INT32 branch_id, bool taken)
{
  if (taken)
       Incr_Branch_Taken(pu_handle, branch_id);
  else 
       Incr_Branch_Not_Taken(pu_handle, branch_id);
}

// ====================================================================

// For each PU, we want a one-time initialization of the 
// tables that maintain profile information of Switch nodes. 
// Given a pu_handle, the number of Switches in that PU and an
// array that reprents the number of targets for each Switch in the PU,
// we initialize the Switch_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from a 
// PU, but the initialization is done only the first time.

// switch_num_targets[i] gives the number of targets for the ith Switch
// in the PU.

static inline void
Profile_Switch_Init(PU_PROFILE_HANDLE pu_handle,
		    INT32 num_switches, INT32 *switch_num_targets,
		    INT32 num_case_values, INT64 *case_values)
{
  Switch_Profile_Vector& Switch_Table = pu_handle->Get_Switch_Table();

  if (Switch_Table.empty()) {

    Switch_Table.reserve(num_switches);

    for (int i = 0; i < num_switches; i++ ) {

	Switch_Profile& sp = Switch_Table[i];
	
	INT32 num_targets = *switch_num_targets;

	sp.targets_profile.reserve_and_init(num_targets + 1);

	sp.targets_case_value.reserve(num_targets);
	for (int j = 0; j < num_targets; j++) 
	  sp.targets_case_value[j] = case_values[j];

	case_values += num_targets;

	++switch_num_targets;
    }
    // Should now be true:  case_index == num_case_values
  }
}

// Update appropriate profile information for an Switch
// Given a PU handle, rtrieve the Switch table; use this table 
// and the switch_id to retrieve the vector representing the
// possible targets for this Switch; target takes a 
// value between 0 and n-1 where n is the number of targets
// for the Switch. Increment the profile information for 
// the appropriate target represented by 'target'.

static inline void
Profile_Switch(PU_PROFILE_HANDLE pu_handle, INT32 switch_id, INT64 case_value, 
	       INT32 num_targets)
{
  Switch_Profile_Vector& switch_table = pu_handle->Get_Switch_Table();
  Switch_Profile& switch_entry  = switch_table[switch_id];
  Switch_Profile::value_type& targets_profile =
      switch_entry.Get_Targets_Profile();
  Switch_Profile::case_value_type& targets_case_value =
      switch_entry.Get_Targets_Case_Value();

  // Which branch corresponds to case_value?
  INT32 t, target = -1;  // default branch
  for (t = 0; t < num_targets; t++) {
    if (targets_case_value[t] == case_value) {
      target = t;
    }
  }

  targets_profile[target + 1]++;  // 0 is default
}

// ====================================================================

// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of Compgoto nodes. 
// Given a pu_handle, the number of Compgotos in that PU and an
// array that reprents the number of targets for each Compgoto in the PU,
// we initialize the Compgoto_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from a 
// PU, but the initialization is done only the first time.

// compgoto_num_targets[i] gives the number of targets for the ith Compgoto
// in the PU.

static inline void
Profile_Compgoto_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_compgotos,
		      INT32 *compgoto_num_targets)
{
  Compgoto_Profile_Vector& Compgoto_Table = pu_handle->Get_Compgoto_Table();

  if (Compgoto_Table.empty()) {

     Compgoto_Table.reserve(num_compgotos);

     for (INT32 i = 0; i < num_compgotos; i++) {

	 INT32 num_targets = *compgoto_num_targets + 1;

	 Compgoto_Table[i].Get_Targets_Profile().reserve_and_init(num_targets);

	 ++compgoto_num_targets;
     }
  }
}

// Update appropriate profile information for an Compgoto
// Given a PU handle, rtrieve the Compgoto table; use this table 
// and the compgoto_id to retrieve the vector representing the
// possible targets for this Compgoto; target takes a 
// value between 0 and n-1 where n is the number of targets
// for the Compgoto. Increment the profile information for 
// the appropriate target represented by 'target'.

static inline void
Profile_Compgoto(PU_PROFILE_HANDLE pu_handle, INT32 compgoto_id, INT32 target, 
		 INT32 num_targets)
{
 if (target < 0 || target >= num_targets)
    target = -1;

 Compgoto_Profile_Vector& compgoto_table = pu_handle->Get_Compgoto_Table();

 Compgoto_Profile& cgoto = compgoto_table[compgoto_id];

 ++(cgoto.Get_Targets_Profile ()[target + 1]);  // 0 is default
}

// ====================================================================

// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of loop nodes. 
// Given a pu_handle and the number of loops in that PU, we 
// initialize the Loop_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from 
// a PU, but the initialization is done only the first time.

static inline void
Profile_Loop_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_loops)
{

  Loop_Profile_Vector& Loop_Table = pu_handle->Get_Loop_Table();

  if (Loop_Table.empty()) {
     Loop_Table.reserve_and_init(num_loops);
  }
}

// Update appropriate profile information at a loop entry.

static inline void
Profile_Loop(PU_PROFILE_HANDLE pu_handle, INT32 loop_id)
{
  Loop_Profile_Vector& Loop_Table = pu_handle->Get_Loop_Table();
  Loop_Profile& loop_info = Loop_Table[loop_id];

#ifdef __NOT_USED__
  if (loop_info.invocation_count == 1) {
     loop_info.min_trip_count = loop_info.last_trip_count;
     loop_info.max_trip_count = loop_info.last_trip_count;
  } else if (loop_info.invocation_count != 0) {
     loop_info.min_trip_count = min(loop_info.min_trip_count,
				    loop_info.last_trip_count);
     loop_info.max_trip_count = max(loop_info.max_trip_count,
				    loop_info.last_trip_count);
  }
#endif

  // Count num_zero_trips -- NOTE: The code does not check whether or not
  // the very last trip through the loop is a zero trip.  Instead, code
  // in procedure Convert_Loop_Profile of the file dump.cxx handles that
  // responsibility.
  if (loop_info.invocation_count > 0 && loop_info.last_trip_count == 0) {
     loop_info.num_zero_trips++;
  }

  loop_info.invocation_count++;
  loop_info.last_trip_count = 0;
}

// Update appropriate profile information at a loop iteration.

static inline void
Profile_Loop_Iter(PU_PROFILE_HANDLE pu_handle, INT32 loop_id)
{
  Loop_Profile_Vector& Loop_Table = pu_handle->Get_Loop_Table();
  Loop_Profile& loop_info = Loop_Table[loop_id];

  loop_info.last_trip_count++;
  loop_info.total_trip_count++;
}

// ====================================================================

// For each PU, we want to a one-time initialization of the 
// tables that maintain profile information of CAND/COR nodes. 
// Given a pu_handle and the number of CAND/COR in that PU, we 
// initialize the Short_Circuit_Table in the pu handle with the appropriate 
// number of entries. This routine may be invoked multiple times from 
// a PU, but the initialization is done only the first time.

static inline void
Profile_Short_Circuit_Init(PU_PROFILE_HANDLE pu_handle,
			   INT32 num_short_circuit_ops)
{
  Short_Circuit_Profile_Vector& Short_Circuit_Table
    = pu_handle->Get_Short_Circuit_Table();

  if (Short_Circuit_Table.empty()) {
     Short_Circuit_Table.reserve_and_init(num_short_circuit_ops);
  }
}

// Given a PU handle, retrieve the CAND/COR table and increment
// right_taken_count for id.

static inline void Incr_Right_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id)
{
  Short_Circuit_Profile_Vector& Short_Circuit_Table
    = pu_handle->Get_Short_Circuit_Table();
  Short_Circuit_Table[id].right_taken_count++;
}

// Given a PU handle, retrieve the CAND/COR table and increment
// neither_taken_count for id.

static inline void Incr_Neither_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id)
{
  Short_Circuit_Profile_Vector& Short_Circuit_Table
    = pu_handle->Get_Short_Circuit_Table();
  Short_Circuit_Table[id].neither_taken_count++;
}

// Update appropriate profile information for right operand of a CAND/COR

static inline void
Profile_Short_Circuit(PU_PROFILE_HANDLE pu_handle, INT32 short_circuit_id,
		      bool taken)
{
  if (taken) 
       Incr_Right_Taken(pu_handle, short_circuit_id);
  else 
       Incr_Neither_Taken(pu_handle, short_circuit_id);
}

// ====================================================================

// For each PU, we want to a one-time initialization of the
// tables that maintain profile information of CALL nodes.
// Given a pu_handle and the number of CALL nodes in that PU, we
// initialize the Call_Profile_Table in the pu handle with the appropriate
// number of entries. This routine may be invoked multiple times from
// a PU, but the initialization is done only the first time.

static inline void 
Profile_Call_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_calls)
{
  Call_Profile_Vector& Call_Table = pu_handle->Get_Call_Table();

  if (Call_Table.empty()) {
     Call_Table.reserve_and_init(num_calls);
  }
}

// Update entry count for a call

static inline void 
Profile_Call_Entry(PU_PROFILE_HANDLE pu_handle, INT32 call_id)
{
  Call_Profile_Vector& Call_Table = pu_handle->Get_Call_Table();

  Call_Table[call_id].entry_count++;
}

// Update exit count for a call

static inline void 
Profile_Call_Exit(PU_PROFILE_HANDLE pu_handle, INT32 call_id)
{
  Call_Profile_Vector& Call_Table = pu_handle->Get_Call_Table();

  Call_Table[call_id].exit_count++;
}

static inline void
Set_Instrumentation_Phase_Num(PROFILE_PHASE phase_num)
{
  instrumentation_phase_num = phase_num;
}

static inline void
Set_Instrumentation_Flags(unsigned int flags)
{
  instrumentation_flags = flags;
}

// ====================================================================

static inline PROFILE_PHASE
Instrumentation_Phase_Num()
{
  return instrumentation_phase_num;
}

static inline unsigned int
Instrumentation_Flags()
{
  return instrumentation_flags;
}

// ====================================================================

#else

extern PU_PROFILE_HANDLE Get_PU_Handle(char *file_name, char* pu_name,
				       long current_pc, INT32 checksum);


extern void Profile_Invoke_Init(PU_PROFILE_HANDLE pu_handle,
				INT32 num_invokes);

extern void Profile_Invoke(PU_PROFILE_HANDLE pu_handle, INT32 invoke_id);


extern void Profile_Branch_Init(PU_PROFILE_HANDLE pu_handle,
				INT32 num_branches);

extern void Profile_Branch(PU_PROFILE_HANDLE pu_handle, INT32 id, bool taken);

extern void Incr_Branch_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);
extern void Incr_Branch_Not_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);


extern void Profile_Switch_Init(PU_PROFILE_HANDLE pu_handle,
				INT32 num_switches, INT32 *switch_num_targets,
				INT32 num_case_values, INT64 *case_values);

extern void Profile_Switch(PU_PROFILE_HANDLE pu_handle, INT32 switch_id,
			   INT64 case_value, INT32 num_targets);


extern void Profile_Compgoto_Init(PU_PROFILE_HANDLE pu_handle,
				  INT32 num_compgotos,
				  INT32 *compgoto_num_targets);

extern void Profile_Compgoto(PU_PROFILE_HANDLE pu_handle, INT32 compgoto_id,
			     INT32 target, INT32 num_targets);


extern void Profile_Loop_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_loops);

extern void Profile_Loop(PU_PROFILE_HANDLE pu_handle, INT32 id);

extern void Profile_Loop_Iter(PU_PROFILE_HANDLE pu_handle, INT32 id);


extern void Profile_Short_Circuit_Init(PU_PROFILE_HANDLE pu_handle,
				       INT32 num_short_circuit_ops);

extern void Profile_Short_Circuit(PU_PROFILE_HANDLE pu_handle,
				  INT32 short_circuit_id, bool taken);

extern void Incr_Right_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);
extern void Incr_Neither_Taken(PU_PROFILE_HANDLE pu_handle, INT32 id);


extern void Profile_Call_Init(PU_PROFILE_HANDLE pu_handle, INT32 num_calls);

extern void Profile_Call_Entry(PU_PROFILE_HANDLE pu_handle, INT32 call_id);

extern void Profile_Call_Exit(PU_PROFILE_HANDLE pu_handle, INT32 call_id);


extern void Set_Instrumentation_Phase_Num(PROFILE_PHASE phase_num);

extern PROFILE_PHASE Instrumentation_Phase_Num(void);

extern void Set_Instrumentation_Flags(unsigned int flags);

extern unsigned int Instrumentation_Flags(void);

// ====================================================================

#endif /* INLINE_EVERYTHING */
#endif /* profile_INCLUDED */

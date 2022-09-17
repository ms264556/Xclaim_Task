//-*-c++-*-
// ====================================================================
// ====================================================================
//
// Module: dump.cxx
// $Revision: 1.15 $
// $Date: 2001/03/12 18:39:07 $
// $Author: mtibuild $
// $Source: /isms/cmplrs.src/osprey1.0/instrumentation/libinstr/RCS/dump.cxx,v $
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
// Write to a binary file all of the frequency counts obtained during
// this run of this program.
//
// ====================================================================
// ====================================================================


#include <stdio.h>
#include <unistd.h>

#include "profile.h"
#include "profile_errors.h"

#define ERRORS_INCLUDED			// force errors.h not included by
					// fb_info.h
#define DevWarn(x)			// DevWarn not available at runtime
#include "fb_info.h"

#include "dump.h"

#ifdef BUILD_FOR_HARDWARE
#include <xtensa/gdbio.h>
#endif

void FD_WRITE(const void * buffer, 
	      size_t size, 
	      size_t nitems, 
	      int fd, 
	      char * error_message, 
	      char * error_arg) 
{
  size_t bytes_to_write = size * nitems;
  size_t bytes_written = 0;
  bytes_written = write(fd, (void *) buffer, bytes_to_write);
  if (bytes_to_write != bytes_written)
    profile_error(error_message, error_arg);
}


void FD_LSEEK(int fd, 
	      off_t position, 
	      int whence, 
	      char * error_msg,
	      char * error_arg)
{
  size_t retval;
  retval = lseek (fd, position, whence);
  if (retval == -1)
    profile_error(error_msg, error_arg);
}


off_t FD_LSEEK_NO_ERR(int fd, 
		      off_t position, 
		      int whence)
{
  return lseek (fd, position, whence);
}

// ====================================================================


static char* ERR_POS = (char *) "Error in positioning within ";
static char* ERR_WRITE = (char *) "Error in writing to ";


// round up "offset" as specified by "alignment"
static inline mUINT32
align (mUINT32 offset, UINT32 alignment)
{
    --alignment;
    return ((offset + alignment) & ~alignment);
}


static inline void
realign_file (int fd, mUINT32& offset, UINT32 alignment, char* fname)
{
    if (offset % alignment == 0)
	return;

    mUINT32 new_offset = align (offset, alignment);
    FD_LSEEK (fd, new_offset - offset, SEEK_CUR, ERR_POS, fname);
    offset = new_offset;
}


namespace {

    // local templates used only by Dump_PU_Profile

    static void
    Convert_Invoke_Profile (DYN_ARRAY<FB_Info_Invoke>& dest,
			    const Invoke_Profile_Vector& src) {
	dest.reserve (src.size ());
	for (int i = 0; i < src.size(); i++) {
	    dest[i] = FB_Info_Invoke (FB_FREQ (src[i].invoke_count));
	}
    }


    static void
    Convert_Branch_Profile (DYN_ARRAY<FB_Info_Branch>& dest,
			    const Branch_Profile_Vector& src) {
	dest.reserve (src.size ());
	
	for (int i = 0; i < src.size(); i++) {
	    dest[i] = FB_Info_Branch (FB_FREQ (src[i].taken),
				      FB_FREQ (src[i].not_taken));
	}
    }


    template <class T>
    static void
    Convert_Switch_Profile (DYN_ARRAY<FB_FREQ>& freq_targets, T& src) {

	freq_targets.reserve (src.size ());

	for (int i = 0; i < src.Get_Targets_Profile().size(); i++) {
	    freq_targets[i] = FB_FREQ (src.Get_Targets_Profile()[i]);
	}

    }


    static void
    Convert_Loop_Profile (DYN_ARRAY<FB_Info_Loop>& dest,
			  const Loop_Profile_Vector& src) {
	dest.reserve (src.size ());

	for (int i = 0; i < src.size(); i++) {

	  // Did the very last pass through the loop have zero trip count?
	  INT64 zero_trips = src[i].num_zero_trips;
	  if ( src[i].invocation_count > 0 && src[i].last_trip_count == 0 ) {
	    zero_trips++;
	  }

	  int tmp = src[i].invocation_count - zero_trips;
	  int tmp1 = src[i].total_trip_count - src[i].invocation_count + zero_trips;
#if 0
	  FB_Info_Loop loop (FB_FREQ (zero_trips),
			     FB_FREQ ( tmp),
			     FB_FREQ (FB_FREQ_TYPE_UNKNOWN),
			     FB_FREQ (tmp1));

	   dest[i] = loop;
#endif
           dest[i] = FB_Info_Loop(FB_FREQ (zero_trips),
				  FB_FREQ ( tmp),
				  FB_FREQ (FB_FREQ_TYPE_UNKNOWN),
				  FB_FREQ (tmp1));
      }
    }

    static void
    Convert_Short_Circuit_Profile (DYN_ARRAY<FB_Info_Circuit>& dest,
				   const Short_Circuit_Profile_Vector& src) {
	dest.reserve (src.size ());

	for (int i = 0; i < src.size(); i++) {
	    dest[i] = 
		FB_Info_Circuit (FB_FREQ (FB_FREQ_TYPE_UNKNOWN),
				 FB_FREQ (src[i].right_taken_count),
				 FB_FREQ (src[i].neither_taken_count));
	}
    }

    static void
    Convert_Call_Profile (DYN_ARRAY<FB_Info_Call>& dest,
			  const Call_Profile_Vector& src) {
	dest.reserve (src.size ());
	for (int i = 0; i < src.size(); i++) {
	    dest[i] = 
	      FB_Info_Call (FB_FREQ (src[i].entry_count),
			    FB_FREQ (src[i].exit_count),
			    src[i].entry_count == src[i].exit_count);
	}
    }

    static void
    Convert_Return_Profile (DYN_ARRAY<FB_Info_Return>& dest,
                          const Return_Profile_Vector& src) {
        dest.reserve (src.size ());
        for (int i = 0; i < src.size(); i++) {
            dest[i] =
              FB_Info_Return (FB_FREQ (src[i].return_count));
        }
    }


    struct POSITION {
	mUINT32 offset;
	INT32 num_entries;

	POSITION () {}
	POSITION (mUINT32 ofst, INT32 num) : offset (ofst), num_entries (num) {}
    };

    template <class T>
    POSITION
    Dump_PU_Profile (int fd, mUINT32& offset, const T& profile, char *fname) {
	realign_file (fd, offset, sizeof(mINT64), fname);

	POSITION pos (offset, profile.size ());

	FD_WRITE (&(profile[0]), sizeof(typename T::value_type),
                   profile.size (), fd, ERR_WRITE, fname);
        
	offset += profile.size () * sizeof(typename T::value_type);
	return pos;
    }


    template <class T>
    POSITION
    Dump_PU_Switch_Profile (int fd, mUINT32& offset, T& profile,
			    mUINT32& target_offset, char *fname) {

	realign_file (fd, offset, sizeof(mINT64), fname);

	POSITION pos (offset, profile.size ());

	for (int i = 0; i < profile.size(); i++) {
	    DYN_ARRAY<FB_FREQ> freq_targets;
	    Convert_Switch_Profile (freq_targets, profile[i]);

	    FD_WRITE (&(freq_targets[0]), sizeof(FB_FREQ),
                       freq_targets.size (), fd, ERR_WRITE, fname);

	    offset += sizeof(INT64) * freq_targets.size ();
	}

	target_offset = offset;

	for (int i = 0; i < profile.size(); i++) {
	    INT32 num_targs = profile[i].Get_Targets_Profile ().size ();
	    FD_WRITE (&num_targs, sizeof(INT32), 1, fd, ERR_WRITE, fname);
	}

	offset += profile.size () * sizeof(INT32);
	return pos;
    }

}  // end namespace


// Given a PU handle, dump all the profile information that
// has been gathered for that PU.
static void
Dump_PU_Profile(int fd, PU_PROFILE_HANDLE pu_handle, char * fname,
		DYN_ARRAY<Pu_Hdr> & Pu_Hdr_Table, 
                DYN_ARRAY<char *> & Str_Table)
{
  static mUINT32 Str_Offset;
  static mUINT32 PU_Offset;

  // Update the string Table
  Str_Table.push_back(pu_handle->pu_name);

  // double word align the PU_Offset
  realign_file (fd, PU_Offset, sizeof(mINT64), fname);
  
  Pu_Hdr pu_hdr;

  pu_hdr.pu_checksum = pu_handle->checksum;
  pu_hdr.pu_low_pc = pu_handle->low_pc;
  pu_hdr.pu_opt_flags = pu_handle->opt_flags;
  pu_hdr.pu_name_index = Str_Offset;
  pu_hdr.pu_file_offset = PU_Offset;
  pu_hdr.pu_cycle_count = pu_handle->cycle_count;

  mUINT32 offset = 0;

  POSITION pos;

  {
      DYN_ARRAY<FB_Info_Invoke> fb_info;
      Convert_Invoke_Profile (fb_info, pu_handle->Get_Invoke_Table ());
      pos = Dump_PU_Profile (fd, offset, fb_info, fname);
      pu_hdr.pu_inv_offset = pos.offset;
      pu_hdr.pu_num_inv_entries = pos.num_entries;
  }

  {
      DYN_ARRAY<FB_Info_Branch> fb_info;
      Convert_Branch_Profile (fb_info, pu_handle->Get_Branch_Table ());
      pos = Dump_PU_Profile (fd, offset, fb_info, fname);
      pu_hdr.pu_br_offset = pos.offset;
      pu_hdr.pu_num_br_entries = pos.num_entries;
  }

  pos = Dump_PU_Switch_Profile (fd, offset, pu_handle->Get_Switch_Table (),
				pu_hdr.pu_switch_target_offset, fname);
  pu_hdr.pu_switch_offset = pos.offset;
  pu_hdr.pu_num_switch_entries = pos.num_entries;

  pos = Dump_PU_Switch_Profile (fd, offset, pu_handle->Get_Compgoto_Table (),
				pu_hdr.pu_cgoto_target_offset, fname);
  pu_hdr.pu_cgoto_offset = pos.offset;
  pu_hdr.pu_num_cgoto_entries = pos.num_entries;

  {
      DYN_ARRAY<FB_Info_Loop> fb_info;
      Convert_Loop_Profile (fb_info, pu_handle->Get_Loop_Table ());
      pos = Dump_PU_Profile (fd, offset, fb_info, fname);
      pu_hdr.pu_loop_offset = pos.offset;
      pu_hdr.pu_num_loop_entries = pos.num_entries;
  }

  {
      DYN_ARRAY<FB_Info_Circuit> fb_info;
      Convert_Short_Circuit_Profile (fb_info,
				     pu_handle->Get_Short_Circuit_Table ());
      pos = Dump_PU_Profile (fd, offset, fb_info, fname); 
      pu_hdr.pu_scircuit_offset = pos.offset;
      pu_hdr.pu_num_scircuit_entries = pos.num_entries;
  }

  {
    DYN_ARRAY<FB_Info_Call> fb_info;
    Convert_Call_Profile (fb_info, pu_handle->Get_Call_Table ());
    pos = Dump_PU_Profile (fd, offset, fb_info, fname);
    pu_hdr.pu_call_offset = pos.offset;
    pu_hdr.pu_num_call_entries = pos.num_entries;
  }

  {
    DYN_ARRAY<FB_Info_Return> fb_info;
    Convert_Return_Profile (fb_info, pu_handle->Get_Return_Table ());
    pos = Dump_PU_Profile (fd, offset, fb_info, fname);
    pu_hdr.pu_return_offset = pos.offset;
    pu_hdr.pu_num_return_entries = pos.num_entries;
  }

  // Enter the PU header
  Pu_Hdr_Table.push_back(pu_hdr);

  Str_Offset += strlen(pu_handle->pu_name) + 1;
  PU_Offset += (offset - pu_hdr.pu_inv_offset);
}


// Write out the PU header table
void 
Dump_Fb_File_Pu_Table(int fd, char *fname, 
                      DYN_ARRAY<Pu_Hdr>& Pu_Hdr_Table,
		      Fb_Hdr& fb_hdr)
{
  mUINT32 offset = FD_LSEEK_NO_ERR(fd, 0, SEEK_CUR);
  realign_file (fd, offset, 
#if defined(__GNUC__)
  __alignof__(Pu_Hdr),
#else
  __builtin_alignof(Pu_Hdr), 
#endif
  fname);

  fb_hdr.fb_pu_hdr_offset = offset;
  fb_hdr.fb_pu_hdr_num = Pu_Hdr_Table.size();
 
  for (size_t i = 0; i < Pu_Hdr_Table.size(); i++) {
    Pu_Hdr & pu_hdr_entry = Pu_Hdr_Table[i];
    fb_hdr.total_cycle_count += pu_hdr_entry.pu_cycle_count;
    FD_WRITE(&pu_hdr_entry, sizeof(Pu_Hdr), 1, fd, ERR_WRITE, fname);
  }
}


// Write out the string table
static void 
Dump_Fb_File_Str_Table(int fd, char *fname, 
                       DYN_ARRAY<char *>& Str_Table,
		       Fb_Hdr& fb_hdr)
{
  mUINT32 table_size = 0;

  fb_hdr.fb_str_table_offset = FD_LSEEK_NO_ERR(fd, 0, SEEK_CUR);

  for (size_t i = 0; i < Str_Table.size(); i++) {
    char *pu_name = Str_Table[i];
    FD_WRITE(pu_name, strlen(pu_name) + 1, 1, fd, ERR_WRITE, fname);

    table_size += strlen(pu_name) + 1;
  }

  fb_hdr.fb_str_table_size = table_size;
}


static void
Dump_Fb_File_Header(int fd, char *output_filename, Fb_Hdr& fb_hdr) 
{
  // Rewind the file
  FD_LSEEK(fd,0,SEEK_SET, ERR_POS, output_filename);
  
  // Write the feedback header
  FD_WRITE(&fb_hdr, sizeof(Fb_Hdr), 1, fd, ERR_WRITE, output_filename);

  // Reset the file position to EOF
  //FD_LSEEK(fd,0,SEEK_END, ERR_POS, output_filename);
}


// Just before finish, dump all the profile information
void 
Dump_all(int fd, char *output_filename)
{
  long sig;
  PU_PROFILE_HANDLE pu_handle;
  HASH_MAP_ITER i(PU_Profile_Handle_Table);

  DYN_ARRAY<Pu_Hdr> Pu_Hdr_Table;
  Pu_Hdr_Table.reserve(PU_Profile_Handle_Table->Num_Entries());
  Pu_Hdr_Table.Initidx(0xFFFFFFFF);

  DYN_ARRAY<char *> Str_Table;
  Str_Table.reserve(PU_Profile_Handle_Table->Num_Entries());
  Str_Table.Initidx(0xFFFFFFFF);

  Fb_Hdr fb_hdr;

  strcpy ((char *) fb_hdr.fb_ident, INSTR_MAG);
  fb_hdr.fb_version = INSTR_CURRENT;
  fb_hdr.fb_pu_hdr_ent_size = sizeof(Pu_Hdr);
  fb_hdr.phase_num = Instrumentation_Phase_Num();
  fb_hdr.opt_flags = Instrumentation_Flags();
  fb_hdr.fb_profile_offset = align (sizeof(Fb_Hdr), sizeof(mUINT64));
  // Leave space for Fb_Hdr

  FD_LSEEK(fd, fb_hdr.fb_profile_offset, SEEK_SET, ERR_POS, output_filename);

  // Dump profile info for all PU

  while (i.Step(&sig, &pu_handle)) 
    Dump_PU_Profile(fd, pu_handle, output_filename, Pu_Hdr_Table, Str_Table);
  // Now attach the PU header table

  Dump_Fb_File_Pu_Table(fd, output_filename, Pu_Hdr_Table, fb_hdr);

  // Attach the string table 

  Dump_Fb_File_Str_Table(fd, output_filename, Str_Table, fb_hdr);

  // Go put the Fb_Hdr at the top of the feedback file

  Dump_Fb_File_Header(fd, output_filename,fb_hdr);
}

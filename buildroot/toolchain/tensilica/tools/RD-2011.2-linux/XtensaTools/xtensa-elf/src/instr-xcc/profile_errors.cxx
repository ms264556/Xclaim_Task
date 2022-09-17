
/* 
   Copyright (C) 2002-2009 Tensilica, Inc.  All Rights Reserved.
   Revised to support Tensilica processors and to improve overall performance
 */

/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


// ====================================================================
// ====================================================================
//
// Module: profile_errors.cxx
// $Revision: 1.5 $
// $Date: 2000/04/06 02:30:30 $
// $Author: mtibuild $
// $Source: /isms/cmplrs.src/osprey1.0/common/instrument/RCS/profile_errors.cxx,v $
//
// Revision history:
//  24-Jul-98 - Original Version
//
// Description:
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

#include "profile_errors.h"

#ifdef BUILD_FOR_HARDWARE
#include <xtensa/gdbio.h>
#endif

#include "profile_errors.h"

int __profile_errors = 0;

void message(const char * msg)
{
  write(2, msg, strlen(msg));
}

void profile_error(const char *str0, const char *str1)
{
  __profile_errors ++;
  message("XCC Feedback: ");
  message(str0);
  message(str1);
  write(2, "\n", 1);
  exit(1);
} 

void profile_warn(const char *str0, const char *str1)
{
  __profile_errors ++;
  message("XCC Feedback: ");
  message(str0);
  message(str1);
  write(2, "\n", 1);
}

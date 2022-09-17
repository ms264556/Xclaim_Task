/*
 * Copyright (c) 2006-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <sys/time.h>
#include <string.h>
#include "gdbio.h"

int _gettimeofday_r (struct _reent * reent, struct timeval *tv, struct timezone * tz)
{
  gdbio_timeval_struct gtv;
  gdbio_ret_struct ret = _gdbio_syscall(GDBIO_TARGET_SYSCALL_GETTIMEOFDAY, 
					NULL, 0, 0, &gtv);

  if (tz)
    memset(tz, 0, sizeof(struct timezone));
  
  gtv.tv_sec = _fix_endian_4(gtv.tv_sec);
  gtv.tv_usec = _fix_endian_8(gtv.tv_usec);
  
  tv->tv_sec = gtv.tv_sec;
  tv->tv_usec = gtv.tv_usec;

  reent->_errno = _gdbio_to_xtensa_errno(ret._errno);
  return ret.retval;
}

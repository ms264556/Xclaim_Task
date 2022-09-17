/*
 * Copyright (c) 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <unistd.h>
#include "gdbio.h"
	
int _unlink_r(struct _reent * reent, const char * path)
{
  gdbio_ret_struct ret;
  int len;
  const char * end = path;
  while (*end++ != 0)
    ;

  len = end - path;

  ret = _gdbio_syscall(GDBIO_TARGET_SYSCALL_UNLINK, 
		       len, 0, 0, path);
  reent->_errno = _gdbio_to_xtensa_errno(ret._errno);
  return ret.retval;
}

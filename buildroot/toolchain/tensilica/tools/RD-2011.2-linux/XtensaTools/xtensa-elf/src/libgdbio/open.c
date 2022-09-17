/*
 * Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "gdbio.h"
	
int _open_r(struct _reent * reent, const char * path, int flags, int mode)
{
  gdbio_ret_struct ret;
  int len, fd;
  const char * end = path;
  while (*end++ != 0)
    ;
  len = end - path;

  if ((fd = _gdbio_new_fd(reent)) < 0)
    return -1;

  flags = _xtensa_to_gdbio_flags(flags);
  mode = _xtensa_to_gdbio_mode(mode);
  ret = _gdbio_syscall(GDBIO_TARGET_SYSCALL_OPEN, flags, mode, len, path);
  reent->_errno = _gdbio_to_xtensa_errno(ret._errno);
  if (ret.retval >= 0) {
    _gdbio_set_fd(fd, ret.retval);
    return fd;
  }
  return ret.retval;
}

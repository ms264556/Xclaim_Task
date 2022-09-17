/*
 * Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */


#include <sys/types.h>
#include <unistd.h>
#include "gdbio.h"

off_t _lseek_r(struct _reent * reent, int fd, off_t offset, int whence)
{
  gdbio_ret_struct ret;

  if ((fd = _gdbio_map_fd(reent, fd)) < 0)
    return -1;
  whence = _xtensa_to_gdbio_whence(whence);
  ret = _gdbio_syscall(GDBIO_TARGET_SYSCALL_LSEEK, offset, whence, 0, fd);
  reent->_errno = _gdbio_to_xtensa_errno(ret._errno);
  return ret.retval;
}

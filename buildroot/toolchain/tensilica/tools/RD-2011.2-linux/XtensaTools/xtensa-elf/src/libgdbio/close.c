/*  
 * Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include "gdbio.h"
#include <errno.h>

int _close_r (struct _reent * reent, int fd)
{
  gdbio_ret_struct ret;

  if ((fd = _gdbio_close_fd(reent, fd)) < 0)
    return -1;
  if (fd >= 0 && fd <= 2)	/* never close stdin,stdout,stderr (PR 15796) */
    return 0;
  ret = _gdbio_syscall(GDBIO_TARGET_SYSCALL_CLOSE, 0, 0, 0, fd);
  reent->_errno = _gdbio_to_xtensa_errno(ret._errno);
  return ret.retval;
}
	

/* kill.c -- remove a process
 *
 * Copyright (c) 2004, 2005, 2006 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <sys/reent.h>
#include "gdbio_syscalls.h"

int _kill_r (struct _reent *ptr, int pid, int sig)
{
  if (pid == __MYPID)
    _exit (sig);
  return 0;
}

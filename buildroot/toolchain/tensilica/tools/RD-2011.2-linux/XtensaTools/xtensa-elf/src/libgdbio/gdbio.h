/*  
 * Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of Tensilica Inc.
 * They may not be modified, copied, reproduced, distributed, or disclosed to
 * third parties in any manner, medium, or form, in whole or in part, without
 * the prior written consent of Tensilica Inc.
 */

#include <unistd.h>
#include "sys/reent.h"
#include "gdbio_syscalls.h"

typedef struct gdbio_ret_struct
{
  int retval;
  int _errno;
} gdbio_ret_struct;

typedef struct gdbio_stat_struct
{
  unsigned long st_dev;
  unsigned long st_ino;
           long st_mode;
  unsigned long st_nlink;
  unsigned long st_uid;
  unsigned long st_gid;
  unsigned long st_rdev;
  unsigned long long st_size;
  unsigned long long st_blksize;
  unsigned long long st_blocks;
  unsigned long st_atime;
  unsigned long st_mtime;
  unsigned long st_ctime;
} gdbio_stat_struct;

typedef struct gdbio_timeval_struct
{
  unsigned long tv_sec;
  unsigned long long tv_usec;
} gdbio_timeval_struct;

extern int  _gdbio_to_xtensa_errno(int gdbio_errno);
extern int  _xtensa_to_gdbio_flags(int xtensa_flags);
extern int  _xtensa_to_gdbio_mode(int xtensa_mode);
extern int  _xtensa_to_gdbio_whence(int xtensa_mode);
extern int  _gdbio_map_fd(struct _reent * reent, int fd);
extern int  _gdbio_close_fd(struct _reent * reent, int fd);
extern int  _gdbio_new_fd(struct _reent * reent);
extern void _gdbio_set_fd(int fd, int gdb_fd);
extern unsigned long      _fix_endian_4(unsigned long val);
extern unsigned long long _fix_endian_8(unsigned long long val);
extern gdbio_ret_struct   _gdbio_syscall(int syscall_code, ...);

